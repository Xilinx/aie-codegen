/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_ss.c
* @{
*
* This file contains routines for AIE stream switch
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   10/21/2019  Optimize stream switch data structures
* 1.2   Tejus	01/04/2020  Cleanup error messages
* 1.3   Tejus   03/20/2020  Make internal function static
* 1.4   Tejus   03/21/2020  Fix slave port configuration bug
* 1.5   Tejus   03/21/2020  Add stream switch packet switch mode apis
* 1.6   Tejus   04/13/2020  Remove range apis and change to single tile apis
* 1.7   Nishad  06/19/2020  Move XAIE_PACKETID_MAX to xaiegbl.h
* 1.8   Tejus   06/10/2020  Switch to new io backend apis.
* 2.0   Nishad  09/15/2020  Add check to validate XAie_StrmSwPktHeader value in
*			    _XAie_StrmPktSwMstrPortConfig()
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_events.h"
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_helper_internal.h"
#include "xaie_ss.h"

#ifdef XAIE_FEATURE_SS_ENABLE

/************************** Constant Definitions *****************************/
#define XAIE_SS_MASTER_PORT_ARBITOR_LSB		0U
#define XAIE_SS_MASTER_PORT_ARBITOR_MASK	0x7U
#define XAIE_SS_MASTER_PORT_MSELEN_LSB		0x3U
#define XAIE_SS_MASTER_PORT_MSELEN_MASK		0x78U

#define XAIE_SS_ARBITOR_MAX			0x7U
#define XAIE_SS_MSEL_MAX			0x3U
#define XAIE_SS_MASK				0x1FU
#define XAIE_SS_MSELEN_MAX			0xFU

#define XAIE_SS_DETERMINISTIC_MERGE_MAX_PKT_CNT (64U - 1U) /* 6 bits */

/************************** Function Definitions *****************************/
/*
 * This API is to give maximum number of stream switch ports
 * for a given tiletype, based on application mode (dual/single)
 */
static AieRC _GetMaxNumSsPorts(XAie_DevInst *DevInst, u8 TileType,
		const XAie_StrmPort *PortPtr, StrmSwPortType PortType, u8 *MaxNumPorts)
{
	if (!(_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen))) {
		*MaxNumPorts = PortPtr->NumPorts;
	} else {
		if (DevInst->AppMode != XAIE_DEVICE_SINGLE_APP_MODE) {
			/* No streams to East and West side in dual app mode */
			if ((PortType == EAST) || (PortType == WEST) ||
				(PortType == _32B_EAST) || (PortType == _32B_WEST)) {
				return XAIE_ERR_STREAM_PORT;
			}
			if (TileType == XAIEGBL_TILE_TYPE_AIETILE) {
				if ((PortType == NORTH) || (PortType == SOUTH) ||
					(PortType == _32B_NORTH) || (PortType == _32B_SOUTH)) {
					/* In Dual app mode for AIE tile, only half of the
					 * north and south tiles will be available, as the
					 * remaining will be used for shadow logic to bypass
					 * the streams to App_B tiles
					 */
					*MaxNumPorts = PortPtr->NumPorts / 2;
				} else {
					*MaxNumPorts = PortPtr->NumPorts;
				}
			} else {
				*MaxNumPorts = PortPtr->NumPorts;
			}
		} else {
			/* For AIE tile PortPtr will have full number of ports
			 * For SHIM tiles WEST & EAST port PortPtr will have full number of ports
			 * Below check verifies the same
			 */
			if ((TileType == XAIEGBL_TILE_TYPE_AIETILE) ||
				((TileType == XAIEGBL_TILE_TYPE_SHIMNOC) &&
				 ((PortType == EAST) || (PortType == WEST) ||
				  (PortType == _32B_EAST) || (PortType == _32B_WEST)))){
				*MaxNumPorts = PortPtr->NumPorts;
			} else {
				if((PortPtr->NumPorts * 2 ) > UINT8_MAX){
					XAIE_ERROR("MaxNumPorts Exceeds U8 MAX value \n");
					return XAIE_ERR;
				}else {
					*MaxNumPorts = PortPtr->NumPorts * 2;
				}
			}
			/* The trace S2MM port is supported only in APP A as oppurtunistic feature.*/
			if ((DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) &&
					(TileType == XAIEGBL_TILE_TYPE_SHIMNOC) &&
					(PortType == DMA_Trace)) {
				XAIE_ERROR(" Trace S2MM port is supported only in APP A \n");
				return XAIE_ERR;
			}
		}
	}

	return XAIE_OK;
}

static inline const XAie_StrmMod * _GetStreamMod(XAie_DevInst *DevInst, u8 TileType, u8 PortType)
{
	if (PortType <= _512B_PORT_END)
		return DevInst->DevProp.DevMod[TileType].StrmSw;
	else if ((PortType >= _32B_PORT_START) && (PortType <= _32B_PORT_END))
		return DevInst->DevProp.DevMod[TileType].StrmSw32b;

	return NULL;
}

/*
 * This is an internal API
 */
static AieRC _XAie_GetPortIdxLegacy(const XAie_StrmMod *StrmMod,
		StrmSwPortType PortType, u8 PortNum, u8 *PortIdx,
		XAie_StrmPortIntf Port)
{
	u32 BaseAddr;
	u32 RegAddr;
	u32 DiffAdd;
	const XAie_StrmPort *PortPtr;

	/* Get Base Addr of the slave/master tile from Stream Switch Module */
	if (Port == XAIE_STRMSW_SLAVE) {
			PortPtr = &StrmMod->SlvConfig[PortType];
			BaseAddr = StrmMod->SlvConfigBaseAddr;
	} else {
			PortPtr = &StrmMod->MstrConfig[PortType];
			BaseAddr = StrmMod->MstrConfigBaseAddr;
	}

	RegAddr = PortPtr->PortBaseAddr + StrmMod->PortOffset * PortNum;
	DiffAdd = ((RegAddr - BaseAddr)/4U);
	if(DiffAdd > UINT8_MAX){
		XAIE_ERROR("PortId Exceeds U8 MAX value \n");
		return XAIE_ERR;
	}else {
		*PortIdx = (u8)(DiffAdd);
	}

	return XAIE_OK;
}

/*
 * This is an internal API, this returns Port ID which should be configured in
 * master port configuration register.
 */
static AieRC _XAie_GetPortIdxAie4Plus(XAie_DevInst *DevInst, u8 TileType,
	StrmSwPortType PortType, const XAie_StrmPort *PortPtr, u8 PortNum,
	u8 *PortIdx, XAie_StrmPortIntf Port)
{
	u8 AddPlaceHolderPortPhyIds = 0;
	u32 TempPortIdx;
	if (DevInst->AppMode != XAIE_DEVICE_SINGLE_APP_MODE) {
		if((UINT8_MAX - PortPtr->PortLogicalId) > PortNum){
			*PortIdx = PortPtr->PortLogicalId + PortNum;
		}else {
			XAIE_ERROR("PortNum Exceeds U8 MAX value \n");
			return XAIE_ERR;
		}
	} else {
		/* For North and South ports, there are one/two extra port which
		 * has been provided as a placeholder for future devices,
		 * so if the PortNum is greater than half of the Maxports,
		 * It requires to add correspoding number of physicalIDs (of placeholder Ports)
		 * to match with the value provided in the spec.
		 */
		switch (TileType) {
		case XAIEGBL_TILE_TYPE_AIETILE:
			if (((Port == XAIE_STRMSW_SLAVE) && (PortType == NORTH)) ||
			    ((Port == XAIE_STRMSW_MASTER) && (PortType == SOUTH))) {
				if (PortNum >= (PortPtr->NumPorts / 2))
					AddPlaceHolderPortPhyIds = 1;
			}
			break;
		case XAIEGBL_TILE_TYPE_MEMTILE:
			if (((Port == XAIE_STRMSW_SLAVE) && (PortType == NORTH)) ||
			    ((Port == XAIE_STRMSW_MASTER) && (PortType == SOUTH))) {
				if (PortNum >= (PortPtr->NumPorts))
					AddPlaceHolderPortPhyIds = 1;
				} else if ((Port == XAIE_STRMSW_SLAVE) && (PortType == SOUTH)) {
					if (PortNum >= PortPtr->NumPorts)
						AddPlaceHolderPortPhyIds = 2;
			}
			break;
		case XAIEGBL_TILE_TYPE_SHIMNOC:
			if ((Port == XAIE_STRMSW_MASTER) && (PortType == NORTH)) {
				if (PortNum >= PortPtr->NumPorts)
					AddPlaceHolderPortPhyIds = 1;
			}
			break;
		default:
			return XAIE_INVALID_TILE;
		}

		TempPortIdx = PortPtr->PortPhysicalId + PortNum + AddPlaceHolderPortPhyIds;
		if (TempPortIdx > UINT8_MAX){
			XAIE_ERROR("PortIdx calculation Exceeds U8 MAX value \n");
			return XAIE_ERR;
		}else {
			*PortIdx = (u8)TempPortIdx;
		}
	}

	return XAIE_OK;
}
/*
 * This API is to validate Port number.
 * 
 * In V1.2 spec, for memtile (MM2S5 & MM2S11) and shimtile(S2MM1 & S2MM3) are
 * removed. But the port numbers not adjusted sequentially.
 * So this API checks for the same and validates. 
 */
static AieRC _XAie_ValidatePortNumber(XAie_DevInst* DevInst, u8 TileType,
	StrmSwPortType PortType, XAie_StrmPortIntf Port, u8 PortNum, u8 MaxPorts)
{
	if (MaxPorts == 0)
		return XAIE_ERR_STREAM_PORT;

	if (!_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
		if (PortNum >= MaxPorts)
			return XAIE_ERR_STREAM_PORT;

		return XAIE_OK;
	}

	if (DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) {
		if ((PortType == DMA) &&
			(((TileType == XAIEGBL_TILE_TYPE_MEMTILE) && (Port == XAIE_STRMSW_SLAVE)) ||
			 ((TileType == XAIEGBL_TILE_TYPE_SHIMNOC) && (Port == XAIE_STRMSW_MASTER)))) {
			if (PortNum == MaxPorts / 2)
				return XAIE_ERR_STREAM_PORT;
			if (PortNum > MaxPorts)
				return XAIE_ERR_STREAM_PORT;
		}
	} else {
		if (PortNum >= MaxPorts)
			return XAIE_ERR_STREAM_PORT;
	}

	return XAIE_OK;
}
/*****************************************************************************/
/**
*
* To configure stream switch master registers, slave index has to be calculated
* from the internal data structure. The routine calculates the slave index for
* any tile type.
*
* @param	DevInst: Device Instance
* @param	TileType: Tile Type
* @param	StrmMod: Stream Module pointer
* @param	Slave: Stream switch port type
* @param	PortNum: Slave port number
* @param	SlaveIdx: Place holder for the routine to store the slave idx
* @param	Port: Port interface type
*
* @return	XAIE_OK on success and XAIE_ERR_STREAM_PORT on failure
*
* @note		Internal API only.
*
******************************************************************************/
AieRC _XAie_GetPortIdx(XAie_DevInst *DevInst, u8 TileType,
		const XAie_StrmMod *StrmMod, StrmSwPortType PortType,
		u8 PortNum, u8 *PortIdx, XAie_StrmPortIntf Port)
{
	AieRC RC;
	const XAie_StrmPort *PortPtr;
	u8 MaxNumPorts;

	if (Port == XAIE_STRMSW_SLAVE)
		PortPtr = &StrmMod->SlvConfig[PortType];
	else
		PortPtr = &StrmMod->MstrConfig[PortType];

	RC = _GetMaxNumSsPorts(DevInst, TileType, PortPtr, PortType, &MaxNumPorts);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Invalid stream port\n");
		return RC;
	}

	RC = _XAie_ValidatePortNumber(DevInst, TileType, PortType, Port, PortNum, MaxNumPorts);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Invalid stream port\n");
		return RC;
	}

	if (!(_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)))
		_XAie_GetPortIdxLegacy(StrmMod, PortType, PortNum, PortIdx, Port);
	else
		_XAie_GetPortIdxAie4Plus(DevInst, TileType, PortType, PortPtr,
			PortNum, PortIdx, Port);

	return XAIE_OK;
}
/*****************************************************************************/
/**
*
* This API is used to get the register offset and value required to configure
* the selected slave port of the stream switch in the corresponding tile.
*
* @param	PortPtr - Pointer to the internal port data structure.
* @param	PortNum - Port Number.
* @param	Enable - Enable/Disable the slave port (1-Enable,0-Disable).
* @param	PktEnable - Enable/Disable the packet switching mode
*		(1-Enable,0-Disable).
* @param	RegVal - pointer to store the register value.
* @param	RegOff - pointer to store the regster offset.
*
* @return	XAIE_OK on success and error code on failure.
*
* @note		Internal API. When PortType is TRACE and there are more than one
*		TRACE ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
static AieRC _XAie_StrmConfigSlv(XAie_DevInst *DevInst, const XAie_StrmMod *StrmMod,
		u8 TileType, StrmSwPortType PortType, u8 PortNum, u8 Enable, u8 PktEnable,
		u32 *RegVal, u32 *RegOff)
{
	AieRC RC;
	u8 MaxNumPorts = 0;
	u8 AddPlaceHolderPort = 0;
	*RegVal = 0U;
	*RegOff = 0U;
	const XAie_StrmPort  *PortPtr;

	/* Get the slave port pointer from stream module */
	PortPtr = &StrmMod->SlvConfig[PortType];

	RC = _GetMaxNumSsPorts(DevInst, TileType, PortPtr, PortType, &MaxNumPorts);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Invalid stream port\n");
		return RC;
	}

	RC = _XAie_ValidatePortNumber(DevInst, TileType, PortType, XAIE_STRMSW_SLAVE,
		PortNum, MaxNumPorts);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Invalid stream port\n");
		return RC;
	}

	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
		if (TileType == XAIEGBL_TILE_TYPE_AIETILE) {
			if ((DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) &&
				(PortType == NORTH) &&
				(PortNum >= (PortPtr->NumPorts / 2))) {
				AddPlaceHolderPort = 1;
			}
		} else {
			if (PortNum >= PortPtr->NumPorts) {
				*RegOff = XAIE4_MASK_VALUE_APP_B;
				PortNum -= PortPtr->NumPorts;
				if ((TileType == XAIEGBL_TILE_TYPE_MEMTILE) && (PortType == DMA))
					PortNum--;
			}
		}
	}

	*RegOff |= PortPtr->PortBaseAddr + StrmMod->PortOffset * (PortNum + AddPlaceHolderPort);

	if (Enable != XAIE_ENABLE) {
		return XAIE_OK;
	}

	if ((_XAie_CheckPrecisionExceeds(StrmMod->SlvEn.Lsb,
			_XAie_MaxBitsNeeded(Enable), MAX_VALID_AIE_REG_BIT_INDEX)) ||
			(_XAie_CheckPrecisionExceeds(StrmMod->SlvPktEn.Lsb,
					_XAie_MaxBitsNeeded(PktEnable), MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	/* Frame the 32-bit reg value */
	*RegVal = XAie_SetField(Enable, StrmMod->SlvEn.Lsb,
			StrmMod->SlvEn.Mask) |
		XAie_SetField(PktEnable,
				StrmMod->SlvPktEn.Lsb, StrmMod->SlvPktEn.Mask);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to get the register offset and value required to configure
* the selected master port of the stream switch in the corresponding tile.
*
* @param	PortPtr - Pointer to the internal port data structure.
* @param	PortNum - Port Number.
* @param	Enable - Enable/Disable the slave port (1-Enable,0-Disable).
* @param	PktEnable - Enable/Disable the packet switching mode
*		(1-Enable,0-Disable).
* @param	RegVal - pointer to store the register value.
* @param	RegOff - pointer to store the regster offset.
*
* @return	XAIE_OK on success and error code on failure.
*
* @note		Internal API.
*
*******************************************************************************/
static AieRC _StrmConfigMstr(XAie_DevInst *DevInst, const XAie_StrmMod *StrmMod,
		u8 TileType, StrmSwPortType PortType, u8 PortNum, u8 Enable, u8 PktEnable,
		u8 Config, u32 *RegVal, u32 *RegOff)
{
	AieRC RC;
	u8 DropHdr;
	u8 MaxNumPorts = 0;
	u8 AddPlaceHolderPort = 0;
	u32 TempDropHdr;
	*RegVal = 0U;
	*RegOff = 0U;
	const XAie_StrmPort *PortPtr;

	/* Get Port pointer from stream switch module */
	PortPtr = &StrmMod->MstrConfig[PortType];

	RC = _GetMaxNumSsPorts(DevInst, TileType, PortPtr, PortType, &MaxNumPorts);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Invalid stream port\n");
		return RC;
	}

	RC = _XAie_ValidatePortNumber(DevInst, TileType, PortType, XAIE_STRMSW_MASTER,
		PortNum, MaxNumPorts);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Invalid stream port\n");
		return RC;
	}

	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
		if (TileType == XAIEGBL_TILE_TYPE_AIETILE) {
			if ((DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) &&
				(PortType == SOUTH) &&
				(PortNum >= (PortPtr->NumPorts / 2))) {
				AddPlaceHolderPort = 1;
			}
		} else {
			if (PortNum >= PortPtr->NumPorts) {
				*RegOff = XAIE4_MASK_VALUE_APP_B;
				PortNum -= PortPtr->NumPorts;
				if ((TileType == XAIEGBL_TILE_TYPE_SHIMNOC) && (PortType == DMA))
					PortNum--;
			}
		}
	}

	*RegOff |= PortPtr->PortBaseAddr + StrmMod->PortOffset * (PortNum + AddPlaceHolderPort);

	if (Enable != XAIE_ENABLE) {
		return XAIE_OK;
	}

	/* Extract the drop header field */
	TempDropHdr = XAie_GetField(Config, StrmMod->DrpHdr.Lsb,
			StrmMod->DrpHdr.Mask);

	if (TempDropHdr > UINT8_MAX){
		XAIE_ERROR("DropHdr calculation Exceeds U8 MAX value \n");
		return XAIE_ERR;
	} else {
		DropHdr = (u8)TempDropHdr;
	}


	if ((_XAie_CheckPrecisionExceeds(StrmMod->MstrEn.Lsb,
			_XAie_MaxBitsNeeded(Enable), MAX_VALID_AIE_REG_BIT_INDEX)) ||
			(_XAie_CheckPrecisionExceeds(StrmMod->MstrPktEn.Lsb,
			_XAie_MaxBitsNeeded(PktEnable), MAX_VALID_AIE_REG_BIT_INDEX)) ||
			(_XAie_CheckPrecisionExceeds(StrmMod->DrpHdr.Lsb,
			_XAie_MaxBitsNeeded(DropHdr), MAX_VALID_AIE_REG_BIT_INDEX)) ||
			(_XAie_CheckPrecisionExceeds(StrmMod->Config.Lsb,
			_XAie_MaxBitsNeeded(Config), MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	/* Frame 32-bit reg value */
	*RegVal = XAie_SetField(Enable, StrmMod->MstrEn.Lsb,
			StrmMod->MstrEn.Mask) |
		XAie_SetField(PktEnable, StrmMod->MstrPktEn.Lsb,
				StrmMod->MstrPktEn.Mask) |
		XAie_SetField(DropHdr, StrmMod->DrpHdr.Lsb,
				StrmMod->DrpHdr.Mask) |
		XAie_SetField(Config, StrmMod->Config.Lsb,
				StrmMod->Config.Mask);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to connect the selected master port to the specified slave
* port of the stream switch switch in ciruit switch mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave - Slave port type.
* @param	SlvPortNum- Slave port number.
* @param	Master - Master port type.
* @param	MstrPortNum- Master port number.
* @param	SlvEnable - Enable/Disable the slave port (1-Enable,0-Disable).
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal API. When PortType is TRACE and there are more than one
*		TRACE ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
static AieRC _XAie_StreamSwitchConfigureCct(XAie_DevInst *DevInst,
		XAie_LocType Loc, StrmSwPortType Slave, u8 SlvPortNum,
		StrmSwPortType Master, u8 MstrPortNum, u8 Enable)
{
	AieRC RC;
	u64 MstrAddr;
	u64 SlvAddr;
	u32 MstrOff;
	u32 MstrVal;
	u32 SlvOff;
	u32 SlvVal;
	u8 SlaveIdx;
	u8 TileType;
	const XAie_StrmMod *StrmMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = _GetStreamMod(DevInst, TileType, (u8)Master);
	if (StrmMod == NULL) {
		XAIE_ERROR("Invalid Stream Switch Ports\n");
		return XAIE_ERR_STREAM_PORT;
	}

	RC = _XAie_GetPortIdx(DevInst, TileType, StrmMod, Slave,
			SlvPortNum, &SlaveIdx, XAIE_STRMSW_SLAVE);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to compute Slave Index\n");
		return RC;
	}

	RC = StrmMod->PortVerify(DevInst, Slave, SlvPortNum, Master, MstrPortNum);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Slave port(Type: %d, Number: %d) can't connect to Master port(Type: %d, Number: %d) on the AIE tile.\n",
		Slave, SlvPortNum, Master, MstrPortNum);
		return RC;
	}

	/* Compute the register value and register address for the master port*/
	RC = _StrmConfigMstr(DevInst, StrmMod, TileType, Master, MstrPortNum,
			Enable, XAIE_DISABLE, SlaveIdx, &MstrVal, &MstrOff);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Master config error\n");
		return RC;
	}

	/* Compute the register value and register address for slave port */
	RC = _XAie_StrmConfigSlv(DevInst, StrmMod, TileType, Slave, SlvPortNum, Enable,
			XAIE_DISABLE, &SlvVal, &SlvOff);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Slave config error\n");
		return RC;
	}

	/* Compute absolute address and write to register */
	MstrAddr = MstrOff + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	SlvAddr = SlvOff + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	RC = XAie_Write32(DevInst, MstrAddr, MstrVal);
	if(RC != XAIE_OK) {
		return RC;
	}

	return XAie_Write32(DevInst, SlvAddr, SlvVal);
}

/*****************************************************************************/
/**
*
* This API is used to enable the connection between the selected master port
* to the specified slave port of the stream switch switch in ciruit switch mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type.
* @param	SlvPortNum: Slave port number.
* @param	Master: Master port type.
* @param	MstrPortNum: Master port number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmConnCctEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum)
{
	return _XAie_StreamSwitchConfigureCct(DevInst, Loc, Slave, SlvPortNum,
			Master, MstrPortNum, XAIE_ENABLE);

}

/*****************************************************************************/
/**
*
* This API is used to disable the connection between the selected master port
* to the specified slave port of the stream switch in ciruit switch mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type.
* @param	SlvPortNum: Slave port number.
* @param	Master: Master port type.
* @param	MstrPortNum: Master port number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmConnCctDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum)
{
	return _XAie_StreamSwitchConfigureCct(DevInst, Loc, Slave, SlvPortNum,
			Master, MstrPortNum, XAIE_DISABLE);

}

/*****************************************************************************/
/**
*
* This API is used to configure the slave port of a stream switch.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type.
* @param	SlvPortNum: Slave port number.
* @param	PktEn: XAIE_ENABLE/XAIE_DISABLE to Enable/Disable packet switch
*		mode.
* @param	Enable: XAIE_ENABLE/XAIE_DISABLE to Enable/Disable slave port.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. When PortType is TRACE or _32B_TRACE and there are
* 		more than one TRACE ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT
*		and PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
static AieRC _XAie_StrmSlavePortConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, u8 EnPkt, u8 Enable)
{
	AieRC RC;
	const XAie_StrmMod *StrmMod;
	u64 Addr;
	u32 RegOff;
	u32 RegVal = 0U;
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}
	
	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = _GetStreamMod(DevInst, TileType, (u8)Slave);
	if (StrmMod == NULL) {
		XAIE_ERROR("Invalid Stream Switch Ports\n");
		return XAIE_ERR_STREAM_PORT;
	}

	/* Compute the register value and register address for slave port */
	RC = _XAie_StrmConfigSlv(DevInst, StrmMod, TileType, Slave, SlvPortNum,
			EnPkt, Enable, &RegVal, &RegOff);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Slave config error\n");
		return RC;
	}

	Addr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOff;

	return XAie_Write32(DevInst, Addr, RegVal);
}

/*****************************************************************************/
/**
*
* This API is used to Enable the slave port of a stream switch in packet switch
* mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type.
* @param	SlvPortNum: Slave port number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmPktSwSlavePortEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum)
{
	return _XAie_StrmSlavePortConfig(DevInst, Loc, Slave, SlvPortNum,
			XAIE_ENABLE, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API is used to Disable the slave port of a stream switch in packet switch
* mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type.
* @param	SlvPortNum: Slave port number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmPktSwSlavePortDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum)
{
	return _XAie_StrmSlavePortConfig(DevInst, Loc, Slave, SlvPortNum,
			XAIE_DISABLE, XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API is used to configure the register fields of Master ports for
* configuration in packet switch mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Master: Master port type.
* @param	MstrPortNum: Master port number.
* @param	DropHeader: Enable or disable the drop header bit
* @param	Arbitor: Arbitor to use for this packet switch connection
* @param	MselEn: MselEn field in the Master port register field
* @param	PktEn: XAIE_ENABLE/XAIE_DISABLE to Enable/Disable packet switch
*		mode.
* @param	Enable: XAIE_ENABLE/XAIE_DISABLE to Enable/Disable master port.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. When Enable is XAIE_DISABLE, the API configures
*		Master port register to reset value.
*
*
*******************************************************************************/
static AieRC _XAie_StrmPktSwMstrPortConfig(XAie_DevInst *DevInst,
		XAie_LocType Loc, StrmSwPortType Master, u8 MstrPortNum,
		XAie_StrmSwPktHeader DropHeader, u8 Arbitor, u8 MSelEn,
		u8 PktEn, u8 Enable)
{
	AieRC RC;
	u64 Addr;
	u32 RegOff;
	u32 RegVal;
	u8 TileType;
	const XAie_StrmMod *StrmMod;
	u32 Config = 0U;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(DropHeader > XAIE_SS_PKT_DROP_HEADER) {
		XAIE_ERROR("Invalid stream switch packet drop header value\n");
		return XAIE_INVALID_ARGS;
	}

	if((Arbitor > XAIE_SS_ARBITOR_MAX) || (MSelEn > XAIE_SS_MSELEN_MAX)) {
		XAIE_ERROR("Invalid Arbitor or MSel Enable\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = _GetStreamMod(DevInst, TileType, (u8)Master);
	if (StrmMod == NULL) {
		XAIE_ERROR("Invalid Stream Switch Ports\n");
		return XAIE_ERR_STREAM_PORT;
	}

	if ((_XAie_CheckPrecisionExceeds(StrmMod->DrpHdr.Lsb,
			_XAie_MaxBitsNeeded((u8)DropHeader), MAX_VALID_AIE_REG_BIT_INDEX)) ||
			(_XAie_CheckPrecisionExceeds(XAIE_SS_MASTER_PORT_ARBITOR_LSB,
			_XAie_MaxBitsNeeded(Arbitor), MAX_VALID_AIE_REG_BIT_INDEX)) ||
			(_XAie_CheckPrecisionExceeds(XAIE_SS_MASTER_PORT_MSELEN_LSB,
			_XAie_MaxBitsNeeded(MSelEn), MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}


	/* Construct Config and Drop header register fields */
	if(Enable == XAIE_ENABLE) {
		Config = XAie_SetField(DropHeader, StrmMod->DrpHdr.Lsb,
				StrmMod->DrpHdr.Mask) |
			XAie_SetField(Arbitor, XAIE_SS_MASTER_PORT_ARBITOR_LSB,
					XAIE_SS_MASTER_PORT_ARBITOR_MASK) |
			XAie_SetField(MSelEn, XAIE_SS_MASTER_PORT_MSELEN_LSB,
					XAIE_SS_MASTER_PORT_MSELEN_MASK);
	}

	if (Config > UINT8_MAX){
		XAIE_ERROR("Config Exceeds U8 MAX value \n");
		return XAIE_ERR;
	}
	/* Compute the register value and register address for the master port*/
	RC = _StrmConfigMstr(DevInst, StrmMod, TileType, Master, MstrPortNum,
			Enable, PktEn, (u8)Config, &RegVal, &RegOff);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Master config error\n");
		return RC;
	}

	Addr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOff;

	return XAie_Write32(DevInst, Addr, RegVal);
}

/*****************************************************************************/
/**
*
* This API is used to Enable the Master ports with configuration for packet
* switch mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Master: Master port type.
* @param	MstrPortNum: Master port number.
* @param	DropHeader: Enable or disable the drop header bit
* @param	Arbitor: Arbitor to use for this packet switch connection
* @param	MSelEn: MselEn field in the Master port register field
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
*
*******************************************************************************/
AieRC XAie_StrmPktSwMstrPortEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Master, u8 MstrPortNum,
		XAie_StrmSwPktHeader DropHeader, u8 Arbitor, u8 MSelEn)
{
	return _XAie_StrmPktSwMstrPortConfig(DevInst, Loc, Master, MstrPortNum,
			DropHeader, Arbitor, MSelEn, XAIE_ENABLE, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API is used to configure the register fields of Master ports for
* configuration in packet switch mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Master: Master port type.
* @param	MstrPortNum: Master port number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Configures Master port register to reset value.
*
*
*******************************************************************************/
AieRC XAie_StrmPktSwMstrPortDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Master, u8 MstrPortNum)
{
	return _XAie_StrmPktSwMstrPortConfig(DevInst, Loc, Master, MstrPortNum,
			XAIE_SS_PKT_DONOT_DROP_HEADER, 0U, 0U, XAIE_DISABLE,
			XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API is used to configure the stream switch slave slot configuration
* registers. This API should be used in combination with other APIs to
* first configure the master and slave ports in packet switch mode. Disabling
* the slave port slot writes reset values to the registers.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type
* @param	SlvPortNum: Slave port number
* @param	SlotNum: Slot number for the slave port
* @param	Pkt: Packet with initialized packet id and packet type
* @param	Mask: Mask field in the slot register
* @param	Msel: Msel register field in the slave slot register
* @param	Arbitor: Arbitor to use for this packet switch connection
* @param	Enable: XAIE_ENABLE/XAIE_DISABLE to Enable or disable
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. When PortType is TRACE and there are more than
*		one TRACE ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT
*		and PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
static AieRC _XAie_StrmSlaveSlotConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, u8 SlotNum,
		XAie_Packet Pkt, u8 Mask, u8 MSel, u8 Arbitor, u8 Enable)
{
	AieRC RC;
	u8 MaxNumPorts;
	u8 TileType;
	u8 AddPlaceHolderPort = 0;
	u64 RegAddr = 0U;
	u32 RegVal = 0U;
	const XAie_StrmMod *StrmMod;
	const XAie_StrmPort *PortPtr;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if((Arbitor > XAIE_SS_ARBITOR_MAX) || (MSel > XAIE_SS_MSEL_MAX) ||
			((Mask & ~XAIE_SS_MASK) != 0U) ||
			(Pkt.PktId > XAIE_PACKET_ID_MAX)) {
		XAIE_ERROR("Invalid Arbitor, MSel, PktId or Mask\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = _GetStreamMod(DevInst, TileType, (u8)Slave);
	if (!StrmMod) {
		XAIE_ERROR("Invalid Stream Switch Ports\n");
		return XAIE_ERR_STREAM_PORT;
	}

	/* Get Port pointer from stream switch module */
	PortPtr = &StrmMod->SlvConfig[Slave];

	RC = _GetMaxNumSsPorts(DevInst, TileType, PortPtr, Slave, &MaxNumPorts);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Invalid stream port\n");
		return RC;
	}

	RC = _XAie_ValidatePortNumber(DevInst, TileType, Slave, XAIE_STRMSW_SLAVE,
		SlvPortNum, MaxNumPorts);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Invalid stream port\n");
		return RC;
	}

	if (SlotNum >= StrmMod->NumSlaveSlots) {
		XAIE_ERROR("Invalid slot\n");
		return XAIE_ERR_STREAM_PORT;
	}

	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
		if (TileType == XAIEGBL_TILE_TYPE_AIETILE) {
			if ((DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) &&
				(Slave == NORTH) &&
				(SlvPortNum >= (PortPtr->NumPorts / 2))) {
				AddPlaceHolderPort = 1;
			}
		} else {
			if (SlvPortNum >= PortPtr->NumPorts) {
				RegAddr = XAIE4_MASK_VALUE_APP_B;
				SlvPortNum -= PortPtr->NumPorts;
				if ((TileType == XAIEGBL_TILE_TYPE_MEMTILE) && (Slave == DMA))
					SlvPortNum--;
			}
		}
	}

	RegAddr |= XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
				StrmMod->SlvSlotConfig[Slave].PortBaseAddr +
				(SlvPortNum + AddPlaceHolderPort) * StrmMod->SlotOffsetPerPort +
				SlotNum * StrmMod->SlotOffset;

	if ((_XAie_CheckPrecisionExceeds(StrmMod->SlotPktId.Lsb,
			_XAie_MaxBitsNeeded(Pkt.PktId), MAX_VALID_AIE_REG_BIT_INDEX)) ||
			(_XAie_CheckPrecisionExceeds(StrmMod->SlotMask.Lsb,
			_XAie_MaxBitsNeeded(Mask), MAX_VALID_AIE_REG_BIT_INDEX)) ||
			(_XAie_CheckPrecisionExceeds(StrmMod->SlotEn.Lsb,
			_XAie_MaxBitsNeeded(XAIE_ENABLE), MAX_VALID_AIE_REG_BIT_INDEX)) ||
			(_XAie_CheckPrecisionExceeds(StrmMod->SlotMsel.Lsb,
			_XAie_MaxBitsNeeded(MSel), MAX_VALID_AIE_REG_BIT_INDEX)) ||
			(_XAie_CheckPrecisionExceeds(StrmMod->SlotArbitor.Lsb,
			_XAie_MaxBitsNeeded(Arbitor), MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	if(Enable == XAIE_ENABLE) {
		RegVal = XAie_SetField(Pkt.PktId, StrmMod->SlotPktId.Lsb,
				StrmMod->SlotPktId.Mask) |
			XAie_SetField(Mask, StrmMod->SlotMask.Lsb,
					StrmMod->SlotMask.Mask) |
			XAie_SetField(XAIE_ENABLE, StrmMod->SlotEn.Lsb,
					StrmMod->SlotEn.Mask) |
			XAie_SetField(MSel, StrmMod->SlotMsel.Lsb,
					StrmMod->SlotMsel.Mask) |
			XAie_SetField(Arbitor, StrmMod->SlotArbitor.Lsb,
					StrmMod->SlotArbitor.Mask);
	}

	return XAie_Write32(DevInst, RegAddr, RegVal);
}

/*****************************************************************************/
/**
*
* This API is used to configure the stream switch slave slot configuration
* registers. This API should be used in combination with other APIs to
* first configure the master and slave ports in packet switch mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type
* @param	SlvPortNum: Slave port number
* @param	SlotNum: Slot number for the slave port
* @param	Pkt: Packet with initialized packet id and packet type
* @param	Mask: Mask field in the slot register
* @param	MSel: Msel register field in the slave slot register
* @param	Arbitor: Arbitor to use for this packet switch connection
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmPktSwSlaveSlotEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, u8 SlotNum,
		XAie_Packet Pkt, u8 Mask, u8 MSel, u8 Arbitor)
{
	return _XAie_StrmSlaveSlotConfig(DevInst, Loc, Slave, SlvPortNum,
			SlotNum, Pkt, Mask, MSel, Arbitor, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API is used to disable the stream switch slave port slots. The API
* disables the slot and writes reset values to all other fields.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type
* @param	SlvPortNum: Slave port number
* @param	SlotNum: Slot number for the slave port
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmPktSwSlaveSlotDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, u8 SlotNum)
{
	XAie_Packet Pkt = XAie_PacketInit(0U, 0U);
	return _XAie_StrmSlaveSlotConfig(DevInst, Loc, Slave, SlvPortNum,
			SlotNum, Pkt, 0U, 0U, 0U, XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API is used to get the physical port id of the stream switch for a given
* tile location, logical port type and port number.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Port: XAIE_STRMSW_SLAVE/MASTER for Slave or Master ports
* @param	PortType: Logical port type of the stream switch
* @param	PortNum: Logical port number
* @param	PhyPortId: Pointer to store the physical port id.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmSwLogicalToPhysicalPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_StrmPortIntf Port, StrmSwPortType PortType, u8 PortNum,
		u8 *PhyPortId)
{
	u8 TileType;
	const XAie_StrmMod *StrmMod;

	if((DevInst == XAIE_NULL) || (PhyPortId == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(Port > XAIE_STRMSW_MASTER) {
		XAIE_ERROR("Invalid Stream Switch Port Interface\n");
		return XAIE_ERR_STREAM_PORT;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		return XAIE_INVALID_TILE;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = _GetStreamMod(DevInst, TileType, (u8)PortType);
	if (!StrmMod) {
		XAIE_ERROR("Invalid Stream Switch Port Type\n");
		return XAIE_ERR_STREAM_PORT;
	}

	return _XAie_GetPortIdx(DevInst, TileType, StrmMod, PortType,
			PortNum, PhyPortId, Port);
}

static AieRC _XAie_StrmSwGetPhysicalToLogicalPort(const XAie_StrmMod *StrmMod,
	XAie_StrmPortIntf Port, u8 PhyPortId, StrmSwPortType *PortType, u8 *PortNum)
{
	u8 MaxPhyPorts;
	const XAie_StrmSwPortMap *PortMap;

	if(Port > XAIE_STRMSW_MASTER) {
		XAIE_ERROR("Invalid Stream Switch port interface\n");
		return XAIE_ERR_STREAM_PORT;
	}

	if(Port == XAIE_STRMSW_SLAVE) {
		PortMap = StrmMod->SlavePortMap;
		MaxPhyPorts = StrmMod->MaxSlavePhyPortId;
	} else {
		PortMap = StrmMod->MasterPortMap;
		MaxPhyPorts = StrmMod->MaxMasterPhyPortId;
	}

	if(PhyPortId > MaxPhyPorts) {
		XAIE_ERROR("Invalid physical port id\n");
		return XAIE_ERR_STREAM_PORT;
	}

	*PortType = PortMap[PhyPortId].PortType;
	*PortNum = PortMap[PhyPortId].PortNum;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to get logical port id and port number for a given tile
* location and physical port id.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Port: XAIE_STRMSW_SLAVE/MASTER for Slave or Master ports
* @param	PhyPortId: Physical port id
* @param	PortType: Pointer to store the logical port type of the stream
*		switch
* @param	PortNum: Pointer to store the logical port number
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmSwPhysicalToLogicalPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_StrmPortIntf Port, u8 PhyPortId, StrmSwPortType *PortType,
		u8 *PortNum)
{
	const XAie_StrmMod *StrmMod;
	u8 TileType;

	if((DevInst == XAIE_NULL) || (PortType == XAIE_NULL) ||
			(PortNum == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX)
		return XAIE_INVALID_TILE;

	/* Get stream switch module pointer from device instance */
	StrmMod = _GetStreamMod(DevInst, TileType, _512B_PORT_START);
	if (!StrmMod) {
		XAIE_ERROR("Invalid Stream Switch Port Type\n");
		return XAIE_ERR_STREAM_PORT;
	}

	return _XAie_StrmSwGetPhysicalToLogicalPort(StrmMod, Port, PhyPortId,
		PortType, PortNum);
}

/*****************************************************************************/
/**
*
* This API is used to get logical port id and port number for a given tile
* location and physical port id for 32bit switch.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Port: XAIE_STRMSW_SLAVE/MASTER for Slave or Master ports
* @param	PhyPortId: Physical port id
* @param	PortType: Pointer to store the logical port type of the stream
*		switch
* @param	PortNum: Pointer to store the logical port number
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. When PortType is _32B_TRACE and there are more than one TRACE
*		ports in the Tile, PortNum 0 maps to CORE_TRACE_PORT and
*		PortNum 1 maps to MEM_TRACE_PORT.
*
*******************************************************************************/
AieRC XAie_StrmSw32bPhysicalToLogicalPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_StrmPortIntf Port, u8 PhyPortId, StrmSwPortType *PortType,
		u8 *PortNum)
{
	const XAie_StrmMod *StrmMod;
	u8 TileType;

	if((DevInst == XAIE_NULL) || (PortType == XAIE_NULL) ||
			(PortNum == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX)
		return XAIE_INVALID_TILE;

	/* Get stream switch module pointer from device instance */
	StrmMod = _GetStreamMod(DevInst, TileType, _32B_PORT_START);
	if (!StrmMod) {
		XAIE_ERROR("Invalid Stream Switch Port Type\n");
		return XAIE_ERR_STREAM_PORT;
	}

	return _XAie_StrmSwGetPhysicalToLogicalPort(StrmMod, Port, PhyPortId,
			PortType, PortNum);
}

/*****************************************************************************/
/**
*
* This API is used to configure the stream switch module for deterministic
* merge of packets from its ports.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Arbitor: Arbitor number.
* @param	Slave: Slave port type.
* @param	PortNum: Slave port number.
* @param	PktCount: Number of packets to merge from Slave and PortNum.
* @param	Position: Position of the packets arriving from Slave & PortNum.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_StrmSwDeterministicMergeConfig(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Arbitor, StrmSwPortType Slave, u8 PortNum,
		u8 PktCount, u8 Position)
{
	AieRC RC;
	u8 TileType, SlvIdx;
	u8 MaxNumPorts, MaxArbitors;
	u32 RegVal, Mask;
	u64 RegAddr = 0;
	const XAie_StrmMod *StrmMod;
	const XAie_StrmPort *PortPtr;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	/* There is no deterministic feature support for 32bit switch ports */
	if (Slave < _512B_PORT_START || Slave > _512B_PORT_END) {
		XAIE_ERROR("Invalid Port Type\n");
		return XAIE_ERR_STREAM_PORT;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = DevInst->DevProp.DevMod[TileType].StrmSw;
	if(StrmMod->DetMergeFeature == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Deterministic merge feature is not available\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	/* Get Port pointer from stream switch module */
	PortPtr = &StrmMod->SlvConfig[Slave];

	RC = _GetMaxNumSsPorts(DevInst, TileType, PortPtr, Slave, &MaxNumPorts);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Invalid stream port\n");
		return RC;
	}

	RC = _XAie_ValidatePortNumber(DevInst, TileType, Slave, XAIE_STRMSW_SLAVE,
		PortNum, MaxNumPorts);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Invalid stream port\n");
		return RC;
	}

	if (Slave >= SS_PORT_TYPE_MAX) {
		XAIE_ERROR("Invalid stream port type or port number\n");
		return XAIE_ERR_STREAM_PORT;
	}

	MaxArbitors = _XAie_GetMaxElementValue(DevInst->DevProp.DevGen, TileType, DevInst->AppMode,
			StrmMod->DetMerge->NumArbitors);
	if((Arbitor > MaxArbitors) ||
			(Position >= StrmMod->DetMerge->NumPositions) ||
			(PktCount > XAIE_SS_DETERMINISTIC_MERGE_MAX_PKT_CNT)) {
		XAIE_ERROR("Invalid Arbitor/Position or PktCount\n");
		return XAIE_INVALID_ARGS;
	}

	RC = _XAie_GetPortIdx(DevInst, TileType, StrmMod, Slave,
			PortNum, &SlvIdx, XAIE_STRMSW_SLAVE);
	if(RC != XAIE_OK) {
		return RC;
	}

	if (Arbitor > StrmMod->DetMerge->NumArbitors) {
		RegAddr = XAIE4_MASK_VALUE_APP_B;
		Arbitor -= StrmMod->DetMerge->NumArbitors;
	}

	RegAddr |= (u64)(StrmMod->DetMerge->ConfigBase +
		(u64)StrmMod->DetMerge->ArbConfigOffset * (u64)Arbitor) +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	if(Position > 1U) {
		RegAddr += 0x4U;
	}

	if((Position % 2U) == 0U) {
		if ((_XAie_CheckPrecisionExceeds(StrmMod->DetMerge->SlvId0.Lsb,
				_XAie_MaxBitsNeeded(SlvIdx), MAX_VALID_AIE_REG_BIT_INDEX)) ||
				(_XAie_CheckPrecisionExceeds(StrmMod->DetMerge->PktCount0.Lsb,
				_XAie_MaxBitsNeeded(PktCount), MAX_VALID_AIE_REG_BIT_INDEX))) {
			XAIE_ERROR("Check Precision Exceeds Failed\n");
			return XAIE_ERR;
		}
		RegVal = XAie_SetField(SlvIdx, StrmMod->DetMerge->SlvId0.Lsb,
				StrmMod->DetMerge->SlvId0.Mask) |
			XAie_SetField(PktCount, StrmMod->DetMerge->PktCount0.Lsb,
					StrmMod->DetMerge->PktCount0.Mask);
		Mask = StrmMod->DetMerge->SlvId0.Mask |
			StrmMod->DetMerge->PktCount0.Mask;
	} else {
		if ((_XAie_CheckPrecisionExceeds(StrmMod->DetMerge->SlvId1.Lsb,
				_XAie_MaxBitsNeeded(SlvIdx), MAX_VALID_AIE_REG_BIT_INDEX)) ||
				(_XAie_CheckPrecisionExceeds(StrmMod->DetMerge->PktCount1.Lsb,
				_XAie_MaxBitsNeeded(PktCount), MAX_VALID_AIE_REG_BIT_INDEX))) {
			XAIE_ERROR("Check Precision Exceeds Failed\n");
			return XAIE_ERR;
		}
		RegVal = XAie_SetField(SlvIdx, StrmMod->DetMerge->SlvId1.Lsb,
				StrmMod->DetMerge->SlvId1.Mask) |
			XAie_SetField(PktCount, StrmMod->DetMerge->PktCount1.Lsb,
					StrmMod->DetMerge->PktCount1.Mask);
		Mask = StrmMod->DetMerge->SlvId1.Mask |
			StrmMod->DetMerge->PktCount1.Mask;
	}

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, RegVal);
}

/*****************************************************************************/
/**
*
* This API is used to enable/disable the deterministic merge feature of stream
* switch modules.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Arbitor: Arbitor number.
* @param	Enable: XAIE_ENABLE to enable. XAIE_DISABLE to disable.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_StrmSwDeterministicMergeCtrl(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Arbitor, u8 Enable)
{
	u8 TileType;
	u8 MaxArbitors;
	u32 RegVal;
	u64 RegAddr = 0;
	const XAie_StrmMod *StrmMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = DevInst->DevProp.DevMod[TileType].StrmSw;
	if(StrmMod->DetMergeFeature == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Deterministic merge feature is not available\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	MaxArbitors = _XAie_GetMaxElementValue(DevInst->DevProp.DevGen, TileType, DevInst->AppMode,
			StrmMod->DetMerge->NumArbitors);

	if(Arbitor > MaxArbitors) {
		XAIE_ERROR("Invalid Arbitor number\n");
		return XAIE_INVALID_ARGS;
	}

	if(Arbitor > StrmMod->DetMerge->NumArbitors) {
		RegAddr = XAIE4_MASK_VALUE_APP_B;
		Arbitor -= StrmMod->DetMerge->NumArbitors;
	}

	RegAddr |= (u64)(StrmMod->DetMerge->EnableBase +
		(u64)StrmMod->DetMerge->ArbConfigOffset * (u64) Arbitor) +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	if (_XAie_CheckPrecisionExceeds(StrmMod->DetMerge->Enable.Lsb,
			_XAie_MaxBitsNeeded(Enable), MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RegVal = XAie_SetField(Enable, StrmMod->DetMerge->Enable.Lsb,
			StrmMod->DetMerge->Enable.Mask);

	return XAie_Write32(DevInst, RegAddr, RegVal);
}

/*****************************************************************************/
/**
*
* This API is used to enable the deterministic merge feature of stream switch
* modules.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Arbitor: Arbitor number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_StrmSwDeterministicMergeEnable(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Arbitor)
{
	return _XAie_StrmSwDeterministicMergeCtrl(DevInst, Loc, Arbitor,
			XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API is used to disable the deterministic merge feature of stream switch
* modules.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Arbitor: Arbitor number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_StrmSwDeterministicMergeDisable(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Arbitor)
{
	return _XAie_StrmSwDeterministicMergeCtrl(DevInst, Loc, Arbitor,
			XAIE_DISABLE);
}

#endif /* XAIE_FEATURE_SS_ENABLE */
/** @} */
