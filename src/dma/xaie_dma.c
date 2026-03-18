/******************************************************************************
* Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_dma.c
* @{
*
* This file contains routines for AIE DMA Controls.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus	01/04/2020  Cleanup error messages
* 1.2   Tejus   03/22/2020  Remove initial dma api implementation
* 1.3   Tejus   03/22/2020  Dma api implementation
* 1.4   Tejus   04/09/2020  Remove unused argument from interleave enable api
* 1.5   Tejus   04/13/2020  Remove use of range in apis
* 1.6   Tejus   06/05/2020  Add api to enable fifo mode.
* 1.7   Tejus   06/10/2020  Switch to new io backend apis.
* 1.8   Nishad  09/15/2020  Add checks to validate XAie_DmaChReset, and
*			    XAie_DmaFifoCounter values in
*			    XAie_DmaChannelResetAll, and XAie_DmaConfigFifoMode,
*			    respectively.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <string.h>
#include "xaie_dma.h"
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_helper_internal.h"
#include "xaiegbl_regdef.h"
#include "xaie_dma_aie4.h"

#ifdef XAIE_FEATURE_DMA_ENABLE

/************************** Constant Definitions *****************************/
#define XAIE_DMA_32BIT_TXFER_LEN			2U
#define XAIE_DMA_MAX_QUEUE_SIZE				4U

#define XAIE_DMA_CHCTRL_NUM_WORDS			2U
#define XAIE_DMA_WAITFORDONE_DEF_WAIT_TIME_US		1000000U


/************************** Function Definitions *****************************/
static inline u8 _XAie_DmaTileAndChannelDirSupportsPvtBuffPoolBds(u8 DevGen,  u8 TileType,
										XAie_DmaDirection Dir)
{
	u8 private_pool = 0;
	if (_XAie_IsDeviceGenAIE4(DevGen)) {
		if (TileType == XAIEGBL_TILE_TYPE_MEMTILE) 
			private_pool = 1;
		else if (TileType == XAIEGBL_TILE_TYPE_SHIMNOC) {
			if (Dir == DMA_MM2S_CTRL)
				private_pool = 1;
			if (Dir == DMA_S2MM_TRACE)
				private_pool = 1;
		}		
	}
	return private_pool;
}
static inline u8 _XAie_DmaGetSharedMaxNumBds(const XAie_DmaMod *DmaMod, u8 DevGen, u8 TileType,
									u8 AppMode)
{
	u16 MaxNumBds = 0;
	if (_XAie_IsDeviceGenAIE4(DevGen)) {
		MaxNumBds = _XAie_GetMaxElementValue(DevGen, TileType, AppMode, (u16)DmaMod->NumBds);
		if(MaxNumBds > UINT8_MAX) {
			XAIE_ERROR("Invalid MaxNumBds \n");
			return XAIE_ERR;
		}
		return (u8)MaxNumBds;
	} else {
		/* For Legacy Specs (<AIE4) */
		return DmaMod->NumBds;
	}
}

static inline u8 _XAie_DmaGetPrivateMaxNumBds(const XAie_DmaMod *DmaMod, u8 DevGen, u8 TileType,
							XAie_DmaDirection Dir)
{
	u8 MaxNumBds = 0;
	if (_XAie_IsDeviceGenAIE4(DevGen)) {
		if (TileType == XAIEGBL_TILE_TYPE_MEMTILE) 
			MaxNumBds = DmaMod->NumBds;
		else if (TileType == XAIEGBL_TILE_TYPE_SHIMNOC) {
			if (Dir == DMA_MM2S_CTRL)
				MaxNumBds =  DmaMod->CtrlMm2sProp->NumBds;
			if (Dir == DMA_S2MM_TRACE)
				MaxNumBds =  DmaMod->TraceS2mmProp->NumBds;
		}
		return MaxNumBds;
	} else {
		XAIE_ERROR("Private buffer pool not supported in AIE versions below AIE4 \n");
		return 0;
	}	
}

/*****************************************************************************/
/**
 * _XAie_DmaGetMaxNumChannels - Get the maximum number of DMA channels.
 * @DevInst: Pointer to the device instance.
 * @DmaMod: Pointer to the DMA module.
 * @TileType: Type of the tile.
 * @Dir: Direction of the DMA transfer.
 *
 * This function returns the maximum number of DMA channels available for the
 * specified device instance, DMA module, tile type, and direction.
 *
 * Return: Maximum number of DMA channels.
*****************************************************************************/
u8 _XAie_DmaGetMaxNumChannels(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
				    u8 TileType, XAie_DmaDirection Dir)
{
	u8 NumChannels = 0;
	u16 NumChannels_loc = 0;

	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
		switch (Dir) {
			case DMA_S2MM:
				NumChannels =  DmaMod->NumChannels;
				break;
			case DMA_MM2S:
				NumChannels =  DmaMod->NumMm2sChannels;
				break;
			case DMA_MM2S_CTRL:
				NumChannels =  DmaMod->CtrlMm2sProp->NumChannels;
				break;
			case DMA_S2MM_TRACE:
				NumChannels =  DmaMod->TraceS2mmProp->NumChannels;
				break;
			default:
				XAIE_ERROR("Invalid Channel direction \n");
				break;
		}
		/* Trace S2MM is not suppored in DUAL App mode B*/
		if(Dir != DMA_S2MM_TRACE) {
			NumChannels_loc = _XAie_GetMaxElementValue(DevInst->DevProp.DevGen, TileType, DevInst->AppMode, NumChannels);
			if(NumChannels_loc > UINT8_MAX) {
				XAIE_ERROR("Invalid MaxNumChannels \n");
				return XAIE_ERR;
			}
			return (u8)NumChannels_loc;
		}	
		else
			return NumChannels;
	} else {
		/* For Legacy Specs (<AIE4) */
		return DmaMod->NumChannels;
	}
}

/*****************************************************************************/
/**
 * _XAie_DmaValidateChannelNumber - Validates the DMA channel number.
 * @param	DevInst: Pointer to the device instance.
 * @param	TileType: Type of the tile.
 * @param	Dir: Direction of the DMA transfer.
 * @param	ChNum: Channel number to be validated.
 * @param	MaxChannels: Maximum number of channels available.
 *
 * This function checks if the provided DMA channel number is valid
 * based on the device instance, tile type, direction of transfer, and
 * the maximum number of channels available.
 *
 * Return: AieRC indicating success or failure of the validation.
 */
/*****************************************************************************/
static AieRC _XAie_DmaValidateChannelNumber(XAie_DevInst* DevInst, u8 TileType,
	XAie_DmaDirection Dir, u8 ChNum, u8 MaxChannels)
{
	if (MaxChannels == 0)
		return XAIE_INVALID_CHANNEL_NUM;

	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
		/* Trace S2MM is not suppored in DUAL App mode B*/
		if (((TileType == XAIEGBL_TILE_TYPE_SHIMNOC) && (Dir == DMA_S2MM_TRACE)) &&
			(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B)) {
				XAIE_ERROR("Trace S2MM is not suported in application B dual app mode\n");
				return XAIE_INVALID_CHANNEL_NUM;
		}
	}

	if (ChNum >= MaxChannels)
		return XAIE_INVALID_CHANNEL_NUM;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API initializes the Dma Descriptor for AIE DMAs for a given range of
* tiles.
*
* @param	DevInst: Device Instance.
* @param	DmaDesc: Pointer to the user allocated dma descriptor.
* @param	Loc: Location of AIE Tile
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaDescInit(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc)
{
	u8 TileType;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) || (DmaDesc == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;

	memset((void *)DmaDesc, 0, sizeof(XAie_DmaDesc));

	DmaMod->DmaBdInit(DmaDesc);
	DmaDesc->TileType = TileType;
	DmaDesc->IsReady = XAIE_COMPONENT_IS_READY;
	DmaDesc->DmaMod = DmaMod;
	DmaDesc->LockMod = DevInst->DevProp.DevMod[TileType].LockMod;
	DmaDesc->DevGen = DevInst->DevProp.DevGen;
	DmaDesc->AppMode = DevInst->AppMode;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API initializes the Acquire and Release Locks for a a Dma descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Acq: Lock object with acquire lock ID and lock value.
* @param	Rel: Lock object with release lock ID and lock value.
* @param 	AcqEn: XAIE_ENABLE/XAIE_DISABLE for enable or disable acquire
*		lock.
* @param 	RelEn: XAIE_ENABLE/XAIE_DISABLE for enable or disable release
*		lock.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal Only.
*
******************************************************************************/
static AieRC _XAie_DmaLockConfig(XAie_DmaDesc *DmaDesc, XAie_Lock Acq,
		XAie_Lock Rel, u8 AcqEn, u8 RelEn)
{
	const XAie_DmaMod *DmaMod;

	DmaMod = DmaDesc->DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	return DmaMod->SetLock(DmaDesc, Acq, Rel, AcqEn, RelEn);
}

/*****************************************************************************/
/**
*
* This API initializes the Acquire and Release Locks for a a Dma descriptor and
* enables the lock in the dma descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Acq: Lock object with acquire lock ID and lock value.
* @param	Rel: Lock object with release lock ID and lock value.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetLock(XAie_DmaDesc *DmaDesc, XAie_Lock Acq, XAie_Lock Rel)
{
	return _XAie_DmaLockConfig(DmaDesc, Acq, Rel, XAIE_ENABLE, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API initializes the Acquire and Release Locks for a a Dma descriptor and
* enables the lock in the dma descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Acq: Lock object with acquire lock ID and lock value.
* @param	Rel: Lock object with release lock ID and lock value.
* @param 	AcqEn: XAIE_ENABLE/XAIE_DISABLE for enable or disable acquire lock.
* @param 	RelEn: XAIE_ENABLE/XAIE_DISABLE for enable or disable release lock.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaLockControl(XAie_DmaDesc *DmaDesc, XAie_Lock Acq,
		XAie_Lock Rel, u8 AcqEn, u8 RelEn)
{
	return _XAie_DmaLockConfig(DmaDesc, Acq, Rel, AcqEn, RelEn);
}

/*****************************************************************************/
/**
*
* This api sets up the packet id and packet type for a dma descriptor.
*
* @param	DmaDesc: Initialized dma descriptor.
* @param	Pkt: Pkt object with acquire packet id and packet type.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetPkt(XAie_DmaDesc *DmaDesc, XAie_Packet Pkt)
{
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaDesc->PktDesc.PktEn = XAIE_ENABLE;
	DmaDesc->PktDesc.PktId = Pkt.PktId;
	DmaDesc->PktDesc.PktType = Pkt.PktType;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API writes the packet type configuration to the DMA channel control
* register for AIE4 MEM and SHIM tiles.
*
* @param	DevInst: Device Instance.
* @param	DmaDesc: Initialized dma descriptor containing packet type.
* @param	Loc: Location of AIE Tile.
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		This API configures the packet type field in the hardware DMA
*		channel control register. For AIE4, packet type is configured
*		per-DMA-channel for MEM and SHIM tiles. This API is only
*		supported for AIE4 generation and has no effect on earlier
*		AIE versions.
*
******************************************************************************/
AieRC XAie_DmaChannelWritePktType(XAie_DevInst *DevInst,
		XAie_DmaDesc *DmaDesc, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir)
{
	u64 Addr;
	u32 Val;
	u8 MaxNumChannels;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) || (DmaDesc == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	if((DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Dma Channel descriptor not initilized\n");
		return XAIE_INVALID_DMA_DESC;
	}

	if(DmaDesc->TileType != DevInst->DevOps->GetTTypefromLoc(DevInst, Loc)) {
		XAIE_ERROR("Tile type mismatch\n");
		return XAIE_INVALID_TILE;
	}

	/* For MEM and Shim, Packet_Type is now per-DMA-channel and is set via DMA channel control register*/
	if (!(_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) ||
		((DmaDesc->TileType != XAIEGBL_TILE_TYPE_MEMTILE) && 
		 (DmaDesc->TileType != XAIEGBL_TILE_TYPE_SHIMNOC))) {
		XAIE_ERROR("Pkt type configuration in channel is only supported for AIE4 MEM and SHIM tiles\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if(DmaDesc->PktDesc.PktEn != XAIE_ENABLE) {
		XAIE_ERROR("Enable the packet descriptor for packet type\n");
		return XAIE_INVALID_DMA_DESC;
	}

	DmaMod = DevInst->DevProp.DevMod[DmaDesc->TileType].DmaMod;
	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod,
			DmaDesc->TileType, Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, DmaDesc->TileType,
		Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	if (!(_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)))
		Addr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			DmaMod->ChCtrlBase + ChNum * (u32)DmaMod->ChIdxOffset +
			(u32)((u8)Dir * (u32)DmaMod->ChIdxOffset * DmaMod->NumChannels);
	else
		Addr = _XAie4_DmaGetChannelCtrlAddr(DevInst, DmaMod, Loc, Dir, ChNum);
	
	if (_XAie_CheckPrecisionExceeds(DmaMod->BdProp->Pkt->PktType.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PktDesc.PktType), MAX_VALID_AIE_REG_BIT_INDEX)){
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	Val = XAie_SetField(DmaDesc->PktDesc.PktType, DmaMod->BdProp->Pkt->PktType.Lsb,
			DmaMod->BdProp->Pkt->PktType.Mask);

	return XAie_MaskWrite32(DevInst, Addr, DmaMod->BdProp->Pkt->PktType.Mask, Val);
}

/*****************************************************************************/
/**
*
* This api sets up the value of inserted out of order ID field in packet header.
*
* @param	DmaDesc: Initialized dma descriptor.
* @param	OutofOrderBdId: Value of out of order ID field in packet header.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The api sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware. This API
*		works only for AIEML and has no effect on AIE. The buffer
*		descriptor with this feature enabled can work only for MM2S
*		channels.
*
******************************************************************************/
AieRC XAie_DmaSetOutofOrderBdId(XAie_DmaDesc *DmaDesc, u8 OutofOrderBdId)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->OutofOrderBdId == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaDesc->BdEnDesc.OutofOrderBdId = OutofOrderBdId;
	DmaDesc->EnOutofOrderBdId = XAIE_ENABLE;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Buffer starting address, Acquire lock and release lock for
* DMA Descriptor and enables the double buffering mode. This API is supported
* for AIE only.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Addr: Buffer Address for the 2nd buffer in double buffer mode.
* @param	Acq: Lock object with acquire lock ID and lock value.
* @param	Rel: Lock object with release lock ID and lock value.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The Acquire and release locks for the double buffers are enabled
*		by default. The API sets up the value in the dma descriptor and
*		does not configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetDoubleBuffer(XAie_DmaDesc *DmaDesc, u64 Addr, XAie_Lock Acq,
		XAie_Lock Rel)
{
	const XAie_DmaMod *DmaMod;
	const XAie_LockMod *LockMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->DoubleBuffering == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if(((Addr & DmaMod->BdProp->AddrAlignMask) != 0U) ||
			((Addr + DmaDesc->AddrDesc.Length) >
			 DmaMod->BdProp->AddrMax)) {
		XAIE_ERROR("Invalid Address\n");
		return XAIE_INVALID_ADDRESS;
	}

	LockMod = DmaDesc->LockMod;
	if((Acq.LockId > LockMod->NumLocks) || (Acq.LockId != Rel.LockId)) {
		XAIE_ERROR("Invalid Lock\n");
		return XAIE_INVALID_LOCK_ID;
	}

	if ((DmaMod->BdProp->AddrAlignShift > _XAie_MaxBitsNeeded(Addr)) &&
		(Addr >= (1ULL << DmaMod->BdProp->AddrAlignShift))) {
		XAIE_ERROR("Loss of precision for Rightshift\n");
		return XAIE_INVALID_ADDRESS;
	}

	DmaDesc->AddrDesc_2.Address = Addr >> DmaMod->BdProp->AddrAlignShift;
	DmaDesc->EnDoubleBuff = XAIE_ENABLE;

	/* For AIE, Acquire and Release Lock IDs must be the same */
	DmaDesc->LockDesc_2.LockAcqId = Acq.LockId;
	DmaDesc->LockDesc_2.LockRelId = Rel.LockId;
	DmaDesc->LockDesc_2.LockAcqEn = XAIE_ENABLE;
	DmaDesc->LockDesc_2.LockRelEn = XAIE_ENABLE;

	if(Rel.LockVal != XAIE_LOCK_WITH_NO_VALUE) {
		DmaDesc->LockDesc_2.LockRelValEn = XAIE_ENABLE;
		DmaDesc->LockDesc_2.LockRelVal = Rel.LockVal;
	}

	if(Acq.LockVal != XAIE_LOCK_WITH_NO_VALUE) {
		DmaDesc->LockDesc_2.LockAcqValEn = XAIE_ENABLE;
		DmaDesc->LockDesc_2.LockAcqVal = Acq.LockVal;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Buffer starting address and the buffer length of the DMA
* Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Addr: Buffer address.
* @param	Len: Length of the buffer in bytes.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		If the dma is being used in double buffer mode, the Len argument
*		passed to this API should include the size for both buffers.
*		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetAddrLen(XAie_DmaDesc *DmaDesc, u64 Addr, u32 Len)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(((Addr & DmaMod->BdProp->AddrAlignMask) != 0U) ||
			((Addr + Len) > DmaMod->BdProp->AddrMax)) {
		XAIE_ERROR("Invalid Address\n");
		return XAIE_INVALID_ADDRESS;
	}

	if ((DmaMod->BdProp->AddrAlignShift > _XAie_MaxBitsNeeded(Addr)) &&
		(Addr >= (1ULL << DmaMod->BdProp->AddrAlignShift))) {
		XAIE_ERROR("Loss of precision for Rightshift\n");
		return XAIE_INVALID_ADDRESS;
	}

	DmaDesc->AddrDesc.Address = Addr >> DmaMod->BdProp->AddrAlignShift;
	DmaDesc->AddrDesc.Length = (Len >> XAIE_DMA_32BIT_TXFER_LEN) -
		DmaMod->BdProp->LenActualOffset;

	/*
	 * Make sure the MemInst is not set as the Addr is not the offset
	 * to the memory object
	 */
	DmaDesc->MemInst = XAIE_NULL;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Buffer starting offset to a memory object and the buffer
* length of the DMA Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	MemInst: Memory object instance
* @param	Offset: Buffer address offset to the specified memory object.
* @param	Len: Length of the buffer in bytes.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The memory object is the shared memory object between the
*		appication and the AI engine partition, this API is used to
*		setup the DMA descriptor which be used for SHIM DMA as it is
*		the one to transfer memory to/from external external memory.
******************************************************************************/
AieRC XAie_DmaSetAddrOffsetLen(XAie_DmaDesc *DmaDesc, XAie_MemInst *MemInst,
		u64 Offset, u32 Len)
{
	const XAie_DmaMod *DmaMod;
	u64 Addr;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("DMA set address offset failed, Invalid DmaDesc\n");
		return XAIE_INVALID_ARGS;
	}

	if (MemInst == XAIE_NULL) {
		XAIE_ERROR("DMA set address offset failed, Invalid MemInst\n");
		return XAIE_INVALID_ARGS;
	}

	if (Offset >= MemInst->Size || Offset + Len > MemInst->Size) {
		XAIE_ERROR("DMA set address offset failed, Invalid Offset, Len\n");
		return XAIE_INVALID_ARGS;
	}

	Addr = Offset + MemInst->DevAddr;
	DmaMod = DmaDesc->DmaMod;
	if(((Addr & DmaMod->BdProp->AddrAlignMask) != 0U) ||
			((Offset + Len) > DmaMod->BdProp->AddrMax)) {
		XAIE_ERROR("DMA Set Address Offset failed, Invalid Address Offset\n");
		return XAIE_INVALID_ADDRESS;
	}

	if ((DmaMod->BdProp->AddrAlignShift > _XAie_MaxBitsNeeded(Addr)) &&
		(Addr >= (1ULL << DmaMod->BdProp->AddrAlignShift))) {
		XAIE_ERROR("Loss of precision for Rightshift\n");
		return XAIE_INVALID_ADDRESS;
	}

	DmaDesc->AddrDesc.Address = Addr >> DmaMod->BdProp->AddrAlignShift;
	DmaDesc->AddrDesc.Length = (Len >> XAIE_DMA_32BIT_TXFER_LEN) -
		DmaMod->BdProp->LenActualOffset;
	DmaDesc->MemInst = MemInst;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Buffer starting address, the buffer length and the address
* mode of the DMA Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Tensor: Dma Tensor describing the address mode of dma. The user
*		should setup the tensor based on the hardware generation.
* @param	Addr: Buffer address.
* @param	Len: Length of the buffer in bytes.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. The stepsize and wrap parameters operate at 32 bit
*		granularity. The address is the absolute address of the buffer
*		which is 32 bit aligned. The driver will configure the BD
*		register with necessary bits(<32 bits) as required by the
*		hardware.
*
******************************************************************************/
AieRC XAie_DmaSetMultiDimAddr(XAie_DmaDesc *DmaDesc, XAie_DmaTensor *Tensor,
		u64 Addr, u32 Len)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) || (Tensor == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(((Addr & DmaMod->BdProp->AddrAlignMask) != 0U) ) {
		/* TODO : Removed the boundary check as the multi dim address
		 * calculation is not linear. Have to add a logic to make sure
		 * we are not accessing above the address range.
		 */
		XAIE_ERROR("Invalid Address\n");
		return XAIE_INVALID_ADDRESS;
	}

	if(Tensor->NumDim > DmaMod->NumAddrDim) {
		XAIE_ERROR("Tensor dimension not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if ((DmaMod->BdProp->AddrAlignShift > _XAie_MaxBitsNeeded(Addr)) &&
		(Addr >= (1ULL << DmaMod->BdProp->AddrAlignShift))) {
		XAIE_ERROR("Loss of precision for Rightshift\n");
		return XAIE_INVALID_ADDRESS;
	}

	DmaDesc->AddrDesc.Address = Addr >> DmaMod->BdProp->AddrAlignShift;
	DmaDesc->AddrDesc.Length = (Len >> XAIE_DMA_32BIT_TXFER_LEN) -
		DmaMod->BdProp->LenActualOffset;

	return	DmaMod->SetMultiDim(DmaDesc, Tensor);
}

/*****************************************************************************/
/**
*
* This API setups the iteration parameters for a Buffer descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	StepSize: Offset applied at each execution of the BD.
* @param	Wrap: Iteration Wrap.
* @param	IterCurr: Current iteration step. This field is incremented by
*		the hardware after BD is loaded.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None. The stepsize and wrap parameters operate at 32 bit
*		granularity. The address is the absolute address of the buffer
*		which is 32 bit aligned. The driver will configure the BD
*		register with necessary bits(<32 bits) as required by the
*		hardware.
*
******************************************************************************/
AieRC XAie_DmaSetBdIteration(XAie_DmaDesc *DmaDesc, u32 StepSize, u16 Wrap,
		u8 IterCurr)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;

	return DmaMod->SetBdIter(DmaDesc, StepSize, Wrap, IterCurr);
}

/*****************************************************************************/
/**
*
* This API enables the compression bit in the DMA Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaEnableCompression(XAie_DmaDesc *DmaDesc)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->Compression == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaDesc->EnCompression = XAIE_ENABLE;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API configures the FIFO mode of the DMA Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Counter: Fifo counter to use. XAIE_DMA_FIFO_COUNTER_NONE to
* 		disable.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaConfigFifoMode(XAie_DmaDesc *DmaDesc, XAie_DmaFifoCounter Counter)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->FifoMode == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if(Counter > XAIE_DMA_FIFO_COUNTER_1) {
		XAIE_ERROR("Invalid DMA FIFO counter\n");
		return XAIE_INVALID_ARGS;
	}

	DmaDesc->FifoMode = (u8)Counter;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the number of buffer descriptors per tile.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of tile
* @param	NumBds: Integer to write number of bds too
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaGetNumBds(XAie_DevInst *DevInst, XAie_LocType Loc, u8 *NumBds)
{
	const XAie_DmaMod *DmaMod;
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	*NumBds = _XAie_DmaGetSharedMaxNumBds(DmaMod, DevInst->DevProp.DevGen, TileType, DevInst->AppMode);

	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen) &&
			(TileType == XAIEGBL_TILE_TYPE_SHIMNOC))
		XAIE_WARN("This API currently returns NumBds for S2MM & MM2S\n");

	return XAIE_OK;
}

/*****************************************************************************/
/**
 * XAie_DmaGetNumBdsPvtBuffPool - Retrieves the number of buffer descriptors
 * in the private buffer pool for a given DMA channel and direction.
 *
 * @DevInst: Pointer to the device instance.
 * @Loc: Location type specifying the tile location.
 * @ChNum: DMA channel number.
 * @Dir: Direction of the DMA transfer.
 * @NumBds: Pointer to a variable where the number of buffer descriptors will be stored.
 *
 * This function queries the number of buffer descriptors available in the private
 * buffer pool for the specified DMA channel and direction. The result is stored
 * in the variable pointed to by NumBds.
 *
 * Return: AieRC - Return code indicating success or failure of the operation.
 *****************************************************************************/
AieRC XAie_DmaGetNumBdsPvtBuffPool(XAie_DevInst *DevInst, XAie_LocType Loc,
			u8 ChNum, XAie_DmaDirection Dir, u8 *NumBds)
{
	const XAie_DmaMod *DmaMod;
	u8 TileType;
	(void)ChNum;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	*NumBds = _XAie_DmaGetPrivateMaxNumBds(DmaMod, DevInst->DevProp.DevGen, TileType, Dir);

	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen) &&
			(TileType == XAIEGBL_TILE_TYPE_SHIMNOC))
		XAIE_WARN("This API currently returns NumBds for S2MM & MM2S\n");

	return XAIE_OK;
}

/*****************************************************************************/
/**
 * XAie_DmaGetNumBdsGeneric - Retrieves the number of buffer descriptors (BDs) for a given DMA channel.
 * @DevInst: Pointer to the AI engine device instance.
 * @Loc: Location type of the AI engine.
 * @ChNum: DMA channel number.
 * @Dir: Direction of the DMA transfer.
 * @NumBds: Pointer to a variable where the number of BDs will be stored.
 *
 * This function retrieves the number of buffer descriptors (BDs) associated with a specific
 * DMA channel in the AI engine. The direction of the DMA transfer is specified by the Dir parameter.
 *
 * Return: AieRC - Status code indicating success or failure of the operation.
*****************************************************************************/
AieRC XAie_DmaGetNumBdsGeneric(XAie_DevInst *DevInst, XAie_LocType Loc,
			u8 ChNum, XAie_DmaDirection Dir, u8 *NumBds)
{
	u8 TileType;
	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
		
	if(!_XAie_DmaTileAndChannelDirSupportsPvtBuffPoolBds(DevInst->DevProp.DevGen, TileType, Dir))
		return XAie_DmaGetNumBds(DevInst, Loc, NumBds);		
	else
		return XAie_DmaGetNumBdsPvtBuffPool(DevInst, Loc, ChNum, Dir, NumBds);
}

/*****************************************************************************/
/**
*
* This API setups the Enable Bd, Next Bd and UseNxtBd fields in the DMA
* Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	NextBd: NextBd to run after the current Bd finishes execution..
* @param	EnableNextBd: XAIE_ENABLE/XAIE_DISABLE to enable or disable
*		the next BD in DMA Descriptor.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetNextBd(XAie_DmaDesc *DmaDesc, u16 NextBd, u8 EnableNextBd)
{
	const XAie_DmaMod *DmaMod;
	u8 MaxNumBds;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	
	MaxNumBds = _XAie_DmaGetSharedMaxNumBds(DmaMod, DmaDesc->DevGen, DmaDesc->TileType, DmaDesc->AppMode);
	if(NextBd >= MaxNumBds) {
		XAIE_ERROR("Invalid Next Bd\n");
		return XAIE_INVALID_BD_NUM;
	}

	DmaDesc->BdEnDesc.NxtBd = NextBd;
	DmaDesc->BdEnDesc.UseNxtBd = EnableNextBd;

	return XAIE_OK;
}

/*****************************************************************************/
/**
 * XAie_DmaSetNextBdPvtBuffPool - Sets the next buffer descriptor in the private buffer pool.
 * @param	DmaDesc: Pointer to the DMA descriptor.
 * @param	ChNum: Channel number.
 * @param	Dir: Direction of the DMA transfer.
 * @param	NextBd: Next buffer descriptor.
 * @param	EnableNextBd: Flag to enable the next buffer descriptor.
 *
 * This function sets the next buffer descriptor in the private buffer pool for the specified
 * DMA channel and direction. It also enables or disables the next buffer descriptor based on
 * the provided flag.
 *
 * Return: AieRC - Status of the operation.
*****************************************************************************/
AieRC XAie_DmaSetNextBdPvtBuffPool(XAie_DmaDesc *DmaDesc, u8 ChNum, XAie_DmaDirection Dir,
					u16 NextBd, u8 EnableNextBd)
{
	const XAie_DmaMod *DmaMod;
	u8 MaxNumBds;
	(void)ChNum;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;

	if (!_XAie_DmaTileAndChannelDirSupportsPvtBuffPoolBds(DmaDesc->DevGen, DmaDesc->TileType, Dir)) {
		XAIE_ERROR("Invalid DMA direction for private buffer pool\n");
		return XAIE_INVALID_DMA_DIRECTION;
	}
	
	MaxNumBds = _XAie_DmaGetPrivateMaxNumBds(DmaMod, DmaDesc->DevGen, DmaDesc->TileType, Dir);
	if(NextBd >= MaxNumBds) {
		XAIE_ERROR("Invalid Next Bd\n");
		return XAIE_INVALID_BD_NUM;
	}

	DmaDesc->BdEnDesc.NxtBd = NextBd;
	DmaDesc->BdEnDesc.UseNxtBd = EnableNextBd;

	return XAIE_OK;
}

/*****************************************************************************/
/**
 * XAie_DmaSetNextBdGeneric - Sets the next buffer descriptor for DMA operations.
 * 
 * @DmaDesc: Pointer to the DMA descriptor.
 * @ChNum: Channel number for the DMA operation.
 * @Dir: Direction of the DMA transfer.
 * @NextBd: The next buffer descriptor to be set.
 * @EnableNextBd: Flag to enable the next buffer descriptor.
 * 
 * This function configures the next buffer descriptor for a DMA channel, 
 * specifying the direction of the transfer and whether the next buffer 
 * descriptor should be enabled.
 * 
 * Return: AieRC - Status of the operation.
*****************************************************************************/
AieRC XAie_DmaSetNextBdGeneric(XAie_DmaDesc *DmaDesc, u8 ChNum, XAie_DmaDirection Dir,
					u16 NextBd, u8 EnableNextBd)
{		
	if(!_XAie_DmaTileAndChannelDirSupportsPvtBuffPoolBds(DmaDesc->DevGen, DmaDesc->TileType, Dir))
		return XAie_DmaSetNextBd(DmaDesc, NextBd, EnableNextBd);
	else
		return XAie_DmaSetNextBdPvtBuffPool(DmaDesc, ChNum, Dir, NextBd, EnableNextBd);		
}

/*****************************************************************************/
/**
*
* This API enables the Enable Bd bit in the DMA Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaEnableBd(XAie_DmaDesc *DmaDesc)
{
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaDesc->BdEnDesc.ValidBd = XAIE_ENABLE;

	if (_XAie_IsDeviceGenAIE4(DmaDesc->DevGen))
		XAIE_WARN("In AIE4, Valid_BD is assumed always Enabled\n");

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API disables the Enable Bd bit in the DMA Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaDisableBd(XAie_DmaDesc *DmaDesc)
{
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaDesc->BdEnDesc.ValidBd = XAIE_DISABLE;

	if (_XAie_IsDeviceGenAIE4(DmaDesc->DevGen))
		XAIE_WARN("In AIE4, Valid_BD is assumed always Enabled\n");

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Enable Bd, Next Bd and UseNxtBd fields in the DMA
* Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Smid: SMID for the buffer descriptor.
* @param	BurstLen: BurstLength for the buffer descriptor (4, 8 or 16).
* @param	Qos: AXI Qos bits for the AXI-MM transfer.
* @param	Cache: AxCACHE bits for AXI-MM transfer.
* @param	Secure: Secure status of the transfer(1-Secure, 0-Non Secure).
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetAxi(XAie_DmaDesc *DmaDesc, u8 Smid, u8 BurstLen, u8 Qos,
		u8 Cache, u8 Secure)
{
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(DmaDesc->TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAIE_ERROR("No Axi properties for tile type\n");
		return XAIE_INVALID_TILE;
	}

	if (!DmaDesc->DmaMod->AxiBurstLenCheck) {
		XAIE_ERROR("Invalid AxiBurstLenCheck pointer\n");
		return XAIE_INVALID_API_POINTER;
	}
        if (DmaDesc->DmaMod->AxiBurstLenCheck(BurstLen,
                                              &DmaDesc->AxiDesc.BurstLen)) {
                XAIE_ERROR("Invalid Burst length\n");
                return XAIE_INVALID_BURST_LENGTH;
        }

	DmaDesc->AxiDesc.SMID = Smid;
	DmaDesc->AxiDesc.AxQos = Qos;
	DmaDesc->AxiDesc.AxCache = Cache;
	DmaDesc->AxiDesc.SecureAccess = Secure;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Enable Bd, Next Bd and UseNxtBd fields in the DMA
* Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	AxUser: AxUSER for future extension.
* @param	IOCoherence: IO coherent data for AxUSER.
* @param	KeyIdx: Trusted memory key index for AxUSER.
* @param	DataReuse: Data reuse for AxUSER.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetAxi_AxUser(XAie_DmaDesc *DmaDesc, u8 AxUser, u8 IOCoherence, u8 KeyIdx,
		u8 DataReuse)
{
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(DmaDesc->TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAIE_ERROR("No Axi properties for tile type\n");
		return XAIE_INVALID_TILE;
	}

	if (!_XAie_IsDeviceGenAIE4(DmaDesc->DevGen)) {
		XAIE_ERROR("Properties valid for Aie4 only\n");
		return XAIE_INVALID_DEVICE;
	}

	DmaDesc->AxiDesc.AxUser = AxUser;
	DmaDesc->AxiDesc.IOCoherence = IOCoherence;
	DmaDesc->AxiDesc.KeyIdx = KeyIdx;
	DmaDesc->AxiDesc.DataReuse = DataReuse;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups DmaDesc with parameters to run dma in interleave mode.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	DoubleBuff: Double buffer to use(0 - A, 1-B)
* @param	IntrleaveCount: Interleaved count to use(to be 32b word aligned)
* @param	IntrleaveCurr: Interleave current pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetInterleaveEnable(XAie_DmaDesc *DmaDesc, u8 DoubleBuff,
		u8 IntrleaveCount, u16 IntrleaveCurr)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->InterleaveMode == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	return DmaMod->SetIntrleave(DmaDesc, DoubleBuff, IntrleaveCount,
			IntrleaveCurr);
}

/*****************************************************************************/
/**
 *
 * This API writes a Dma Descriptor which is initialized and setup by other APIs
 * into the corresponding registers and register fields in the hardware.
 *
 * @param	DevInst: Device Instance
 * @param	DmaDesc: Initialized Dma Descriptor.
 * @param	Loc: Location of AIE Tile
 * @param	BdNum: Hardware BD number to be written to.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		None.
 *
 ******************************************************************************/
AieRC XAie_DmaWriteBd(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u16 BdNum)
{
	const XAie_DmaMod *DmaMod;
	u8 MaxNumBds;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(DmaDesc->TileType != DevInst->DevOps->GetTTypefromLoc(DevInst, Loc)) {
		XAIE_ERROR("Tile type mismatch\n");
		return XAIE_INVALID_TILE;
	}
	
	DmaMod = DmaDesc->DmaMod;

	MaxNumBds = _XAie_DmaGetSharedMaxNumBds(DmaMod, DmaDesc->DevGen, DmaDesc->TileType, DmaDesc->AppMode);
	if(BdNum >= MaxNumBds) {
		XAIE_ERROR("Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}
	
	return DmaMod->WriteBd(DevInst, DmaDesc, Loc, BdNum);
}

/*****************************************************************************/
/**
 *
 * This API reads the data from the buffer descriptor registers to fill the
 * dma descriptor structure.
 *
 * @param	DevInst: Device Instance
 * @param	DmaDesc: Dma Descriptor to be filled.
 * @param	Loc: Location of AIE Tile
 * @param	BdNum: Hardware BD number to be written to.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note	None.
 *
 ******************************************************************************/
AieRC XAie_DmaReadBd(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u16 BdNum)
{
	const XAie_DmaMod *DmaMod;
	u8 MaxNumBds;

	if((DevInst == XAIE_NULL) || (DmaDesc == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(DmaDesc->TileType != DevInst->DevOps->GetTTypefromLoc(DevInst, Loc)) {
		XAIE_ERROR("Tile type mismatch\n");
		return XAIE_INVALID_TILE;
	}
	
	DmaMod = DmaDesc->DmaMod;

	MaxNumBds = _XAie_DmaGetSharedMaxNumBds(DmaMod, DmaDesc->DevGen, DmaDesc->TileType, DmaDesc->AppMode);
	if (BdNum >= MaxNumBds) {
		XAIE_ERROR("Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}	

	memset((void *)DmaDesc, 0, sizeof(XAie_DmaDesc));
	DmaDesc->DmaMod = DmaMod;

	return DmaMod->ReadBd(DevInst, DmaDesc, Loc, BdNum);
}

/*****************************************************************************/
/**
 * XAie_DmaWriteBdPvtBuffPool - Writes a DMA buffer descriptor to the private buffer pool.
 * @DevInst: Pointer to the device instance.
 * @DmaDesc: Pointer to the DMA descriptor.
 * @Loc: Location type.
 * @ChNum: Channel number.
 * @Dir: DMA direction.
 * @BdNum: Buffer descriptor number.
 *
 * This function writes a DMA buffer descriptor to the private buffer pool
 * for the specified device instance, location, channel, and direction.
 *
 * Return: AieRC - Status of the operation.
*****************************************************************************/
AieRC XAie_DmaWriteBdPvtBuffPool(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc, XAie_LocType Loc,
			u8 ChNum, XAie_DmaDirection Dir, u16 BdNum)
{
	const XAie_DmaMod *DmaMod;
	u8 MaxNumChannels, MaxNumBds;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(DmaDesc->TileType != DevInst->DevOps->GetTTypefromLoc(DevInst, Loc)) {
		XAIE_ERROR("Tile type mismatch\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DmaDesc->DmaMod;

	if (!_XAie_DmaTileAndChannelDirSupportsPvtBuffPoolBds(DmaDesc->DevGen, DmaDesc->TileType, Dir)) {
		XAIE_ERROR("Invalid DMA direction for private buffer pool\n");
		return XAIE_INVALID_DMA_DIRECTION;
	}

	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, DmaDesc->TileType, Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, DmaDesc->TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	MaxNumBds = _XAie_DmaGetPrivateMaxNumBds(DmaMod, DmaDesc->DevGen, DmaDesc->TileType, Dir);
	if (BdNum >= MaxNumBds) {
		XAIE_ERROR("Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}

	return DmaMod->WriteBdPvtBuffPool(DevInst, DmaDesc, Loc, ChNum, Dir, BdNum);
}

/*****************************************************************************/
/**
 * XAie_DmaWriteBdGeneric - Writes a DMA buffer descriptor to the specified location.
 * @DevInst: Pointer to the device instance.
 * @DmaDesc: Pointer to the DMA descriptor.
 * @Loc: Location type where the descriptor should be written.
 * @ChNum: Channel number to which the descriptor belongs.
 * @Dir: Direction of the DMA transfer.
 * @BdNum: Buffer descriptor number.
 *
 * This function writes a DMA buffer descriptor to the specified location in the device.
 * It configures the DMA channel with the provided descriptor details.
 *
 * Return: AieRC - Status of the operation.
*****************************************************************************/
AieRC XAie_DmaWriteBdGeneric(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc, XAie_LocType Loc,
			u8 ChNum, XAie_DmaDirection Dir, u16 BdNum)
{
	u8 TileType;
	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
		
	if(!_XAie_DmaTileAndChannelDirSupportsPvtBuffPoolBds(DevInst->DevProp.DevGen, TileType, Dir))
		return XAie_DmaWriteBd(DevInst, DmaDesc, Loc, BdNum);
	else
		return XAie_DmaWriteBdPvtBuffPool(DevInst, DmaDesc, Loc, ChNum, Dir, BdNum);
}

/*****************************************************************************/
/**
 * XAie_DmaReadBdPvtBuffPool - Reads a DMA buffer descriptor from the private buffer pool.
 * @DevInst: Pointer to the device instance.
 * @DmaDesc: Pointer to the DMA descriptor.
 * @Loc: Location type.
 * @ChNum: Channel number.
 * @Dir: DMA direction.
 * @BdNum: Buffer descriptor number.
 *
 * This function reads a DMA buffer descriptor from the private buffer pool
 * based on the provided parameters.
 *
 * Return: AieRC - Return code indicating success or failure.
*****************************************************************************/
AieRC XAie_DmaReadBdPvtBuffPool(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc, XAie_LocType Loc,
			u8 ChNum, XAie_DmaDirection Dir, u16 BdNum)
{
	const XAie_DmaMod *DmaMod;
	u8 MaxNumBds, MaxNumChannels;

	if((DevInst == XAIE_NULL) || (DmaDesc == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(DmaDesc->TileType != DevInst->DevOps->GetTTypefromLoc(DevInst, Loc)) {
		XAIE_ERROR("Tile type mismatch\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DmaDesc->DmaMod;
	
	if (!_XAie_DmaTileAndChannelDirSupportsPvtBuffPoolBds(DmaDesc->DevGen, DmaDesc->TileType, Dir)) {
		XAIE_ERROR("Invalid DMA direction for private buffer pool\n");
		return XAIE_INVALID_DMA_DIRECTION;
	}

	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, DmaDesc->TileType, Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, DmaDesc->TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	MaxNumBds = _XAie_DmaGetPrivateMaxNumBds(DmaMod, DmaDesc->DevGen, DmaDesc->TileType, Dir);
	if (BdNum >= MaxNumBds) {
		XAIE_ERROR("Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}

	memset((void *)DmaDesc, 0, sizeof(XAie_DmaDesc));
	DmaDesc->DmaMod = DmaMod;

	return DmaMod->ReadBdPvtBuffPool(DevInst, DmaDesc, Loc, ChNum, Dir, BdNum);
}

/*****************************************************************************/
/**
 * XAie_DmaReadBdGeneric - Reads a DMA buffer descriptor from the private buffer pool.
 * @DevInst: Pointer to the AI engine device instance.
 * @DmaDesc: Pointer to the DMA descriptor.
 * @Loc: Location type of the AI engine.
 * @ChNum: Channel number.
 * @Dir: Direction of the DMA transfer.
 * @BdNum: Buffer descriptor number.
 *
 * This function reads a DMA buffer descriptor from the private buffer pool
 * for the specified AI engine device instance, location, channel, direction,
 * and buffer descriptor number.
 *
 * Return: AieRC - Return code indicating success or failure of the operation.
 */
AieRC XAie_DmaReadBdGeneric(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc, XAie_LocType Loc,
			u8 ChNum, XAie_DmaDirection Dir, u16 BdNum)
{
	if(!_XAie_DmaTileAndChannelDirSupportsPvtBuffPoolBds(DmaDesc->DevGen, DmaDesc->TileType, Dir))
		 return XAie_DmaReadBd(DevInst, DmaDesc, Loc, BdNum);
	else
		return XAie_DmaReadBdPvtBuffPool(DevInst, DmaDesc, Loc, ChNum, Dir, BdNum);
}
/*****************************************************************************/
/**
*
* This API resets a DMA Channel for a given AIE Tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	Reset: DMA_CHANNEL_UNRESET/RESET
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Returns error for SHIMNOC tiles.
*
******************************************************************************/
AieRC XAie_DmaChannelReset(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir, XAie_DmaChReset Reset)
{
	u8 TileType;
	u8 MaxNumChannels;
	u64 Addr;
	u32 Val;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	if(Reset > DMA_CHANNEL_RESET) {
		XAIE_ERROR("Invalid DMA channel reset value\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if((TileType == XAIEGBL_TILE_TYPE_SHIMPL) ||
			(TileType == XAIEGBL_TILE_TYPE_SHIMNOC)) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	if (!_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen))
		/* Calculate Channel base address */
		Addr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
				DmaMod->ChCtrlBase + ChNum * (u32)DmaMod->ChIdxOffset +
				(u32)((u8)Dir * (u32)DmaMod->ChIdxOffset * DmaMod->NumChannels);
	else
		Addr = _XAie4_DmaGetChannelCtrlAddr(DevInst, DmaMod, Loc, Dir, ChNum);

	if (_XAie_CheckPrecisionExceeds(DmaMod->ChProp->Reset.Lsb,
			_XAie_MaxBitsNeeded((u32)Reset), MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	Val = XAie_SetField(Reset, DmaMod->ChProp->Reset.Lsb,
			DmaMod->ChProp->Reset.Mask);

	return XAie_MaskWrite32(DevInst, Addr, DmaMod->ChProp->Reset.Mask, Val);
}

/*****************************************************************************/
/**
*
* This API resets all the Dma Channels of an AIE Tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Reset: DMA_CHANNEL_UNRESET/RESET
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Returns error for SHIMNOC tiles.
*
******************************************************************************/
AieRC XAie_DmaChannelResetAll(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_DmaChReset Reset)
{
	u8 TileType;
	u8 MaxNumChannels;
	AieRC RC;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Reset > DMA_CHANNEL_RESET) {
		XAIE_ERROR("Invalid DMA channel reset value\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;

	/* Reset MM2S */
	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, DMA_MM2S);
	for(u8 i = 0U; i < MaxNumChannels; i++) {		
		RC = XAie_DmaChannelReset(DevInst, Loc, i, DMA_MM2S, Reset);
		if (RC != XAIE_OK) {
			return RC;
		}
	}
	
	/* Reset S2MM */
	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, DMA_S2MM);
	for(u8 i = 0U; i < MaxNumChannels; i++) {	
		RC = XAie_DmaChannelReset(DevInst, Loc, i, DMA_S2MM, Reset);
		if (RC != XAIE_OK) {
			return RC;
		}
	}
	return  XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API pauses the stream via SHIM Dma for Shim NoC tiles.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the SHIM DMA Channel. (MM2S or S2MM)
* @param	Pause: XAIE_ENABLE to Pause or XAIE_DISABLE to un-pause.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		For AIE Shim Noc Tiles only.
*
******************************************************************************/
AieRC XAie_DmaChannelPauseStream(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 Pause)
{
	u8 TileType;
	u8 MaxNumChannels;
	u32 Value;
	u64 Addr;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	if(DevInst->DevProp.DevGen != XAIE_DEV_GEN_AIE) {
		XAIE_ERROR("Shim stream pause not supported\n");
		return XAIE_ERR;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	if (_XAie_CheckPrecisionExceeds(DmaMod->ChProp->PauseStream.Lsb,
			_XAie_MaxBitsNeeded(Pause), MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	Value = XAie_SetField(Pause, DmaMod->ChProp->PauseStream.Lsb,
			DmaMod->ChProp->PauseStream.Mask);

	if (!(_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)))
		/* Calculate Channel base address */
		Addr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			DmaMod->ChCtrlBase + ChNum * (u32)DmaMod->ChIdxOffset +
			(u32)((u8)Dir * (u32)DmaMod->ChIdxOffset * DmaMod->NumChannels);
	else
		Addr = _XAie4_DmaGetChannelCtrlAddr(DevInst, DmaMod, Loc, Dir, ChNum);

	return XAie_MaskWrite32(DevInst, Addr, DmaMod->ChProp->PauseStream.Mask,
			Value);
}

/*****************************************************************************/
/**
*
* This API pauses the AXI-MM transactions on SHIM Dma for Shim NoC tiles.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the SHIM DMA Channel. (MM2S or S2MM)
* @param	Pause: XAIE_ENABLE to Pause or XAIE_DISABLE to un-pause.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		For AIE Shim Noc Tiles only.
*
******************************************************************************/
AieRC XAie_DmaChannelPauseMem(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir, u8 Pause)
{
	u8 TileType;
	u8 MaxNumChannels;
	u32 Value;
	u64 Addr;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	if(DevInst->DevProp.DevGen != XAIE_DEV_GEN_AIE) {
		XAIE_ERROR("Shim stream pause not supported\n");
		return XAIE_ERR;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	if (_XAie_CheckPrecisionExceeds(DmaMod->ChProp->PauseMem.Lsb,
			_XAie_MaxBitsNeeded(Pause), MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	Value = XAie_SetField(Pause, DmaMod->ChProp->PauseMem.Lsb,
			DmaMod->ChProp->PauseMem.Mask);

	if (!(_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)))
		/* Calculate Channel base address */
		Addr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			DmaMod->ChCtrlBase + ChNum * (u32)DmaMod->ChIdxOffset +
			(u32)((u8)Dir * (u32)DmaMod->ChIdxOffset * DmaMod->NumChannels);
	else
		Addr = _XAie4_DmaGetChannelCtrlAddr(DevInst, DmaMod, Loc, Dir, ChNum);

	return XAie_MaskWrite32(DevInst, Addr, DmaMod->ChProp->PauseMem.Mask,
			Value);
}

/*****************************************************************************/
/**
*
* This API pushes a Buffer Descriptor onto the MM2S or S2MM Channel queue.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	BdNum: Bd number to be pushed to the queue.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		If there is no bit to enable the channel in AIE DMAs,
*		pushing a Buffer descriptor number onto the queue starts the
*		channel.
*
******************************************************************************/
AieRC XAie_DmaChannelPushBdToQueue(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u16 BdNum)
{
	AieRC RC;
	u8 TileType;
	u8 MaxNumBds, MaxNumChannels;
	u64 Addr;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	if(!_XAie_DmaTileAndChannelDirSupportsPvtBuffPoolBds(DevInst->DevProp.DevGen, TileType, Dir))
		MaxNumBds = _XAie_DmaGetSharedMaxNumBds(DmaMod, DevInst->DevProp.DevGen, TileType, DevInst->AppMode);
	else
		MaxNumBds = _XAie_DmaGetPrivateMaxNumBds(DmaMod, DevInst->DevProp.DevGen, TileType, Dir);
	if(BdNum >= MaxNumBds) {
		XAIE_ERROR("Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}

	RC = DmaMod->BdChValidity(DmaMod, Dir, BdNum, ChNum);
	if (RC != XAIE_OK) {
		return RC;
	}
	
	/* AIE4 single-app mode BD adjustment for ShimNOC tiles:
	 * For channels accessing BDs 16-31, subtract 16 to get the correct BD index. */
	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen) &&
			(DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) &&
			(TileType == XAIEGBL_TILE_TYPE_SHIMNOC) &&
			((Dir == DMA_S2MM) || (Dir == DMA_MM2S)) &&
			(BdNum >= DmaMod->NumBds)) {
		BdNum -= DmaMod->NumBds;
	}

	if (!(_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen))) {
		/* Calculate Channel base address */
		Addr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			DmaMod->ChCtrlBase + ChNum * (u32)DmaMod->ChIdxOffset +
			(u32)((u8)Dir * (u32)DmaMod->ChIdxOffset * DmaMod->NumChannels);
	} else {
		Addr = _XAie4_DmaGetChannelCtrlAddr(DevInst, DmaMod, Loc, Dir, ChNum);
	}

	return XAie_Write32(DevInst, Addr + (u64)(DmaMod->ChProp->StartBd.Idx * 4U),
			BdNum);
}

/*****************************************************************************/
/**
*
* This API Enables or Disables a S2MM or MM2S channel of AIE DMAs.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	Enable: XAIE_ENABLE/XAIE_DISABLE to enable/disable.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
static AieRC _XAie_DmaChannelControl(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 Enable)
{
	u8 TileType;
	u8 MaxNumChannels;
	u64 Addr;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}
	/* Calculate Channel base address */
	Addr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChCtrlBase + ChNum * (u32)DmaMod->ChIdxOffset +
		(u32)((u8)Dir * (u32)DmaMod->ChIdxOffset * DmaMod->NumChannels);

	return XAie_MaskWrite32(DevInst,
			Addr + (u64)(DmaMod->ChProp->Enable.Idx * 4U),
			DmaMod->ChProp->Enable.Mask, Enable);
}

/*****************************************************************************/
/**
*
* This API Enables a S2MM or MM2S channel of AIE DMAs.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		From AIE4 there is no channel enable configuration in RegDB
*
******************************************************************************/
AieRC XAie_DmaChannelEnable(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir)
{
	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen))
		return XAIE_NOT_SUPPORTED;
	else
		return _XAie_DmaChannelControl(DevInst, Loc, ChNum, Dir, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API Disables a S2MM or MM2S channel of AIE DMAs.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		From AIE4 there is no channel enable configuration in RegDB
*
******************************************************************************/
AieRC XAie_DmaChannelDisable(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir)
{
	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen))
		return XAIE_NOT_SUPPORTED;
	else
		return _XAie_DmaChannelControl(DevInst, Loc, ChNum, Dir, XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API is used to get the count of scheduled BDs in pending.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	PendingBd: Pointer to store the number of pending BDs.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		This function checks the number of pending BDs in the queue
* as well as if there's any BD that the channel is currently operating on.
* If multiple BDs are chained, it's counted as one BD.
*
******************************************************************************/
AieRC XAie_DmaGetPendingBdCount(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 *PendingBd)
{
	u8 TileType;
	u8 MaxNumChannels;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	return DmaMod->PendingBd(DevInst, Loc, DmaMod, ChNum, Dir, PendingBd);
}

/*****************************************************************************/
/**
*
* This API is used to Get the Channel Status Value.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param        Status - Value of Channel Status Register
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaGetChannelStatus(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u32 *Status)
{
	u8 TileType;
	u8 MaxNumChannels;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	return DmaMod->GetChannelStatus(DevInst, Loc, DmaMod, ChNum,
			Dir, Status);
}

/*****************************************************************************/
/**
*
* This API is used to wait on Shim DMA channel to be completed.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param        TimeOutUs - Minimum timeout value in micro seconds.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		This API in context of TXN flow will be a yeilded poll wait.
*
******************************************************************************/
AieRC XAie_DmaWaitForDone(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir, u32 TimeOutUs)
{
	u8 TileType;
	u8 MaxNumChannels;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, (u8)Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	if(TimeOutUs == 0U) {
		TimeOutUs = XAIE_DMA_WAITFORDONE_DEF_WAIT_TIME_US;
	}

	return DmaMod->WaitforDone(DevInst, Loc, DmaMod, ChNum, Dir, TimeOutUs,
		XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API is used to wait on Shim DMA channel to be completed.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param        TimeOutUs - Minimum timeout value in micro seconds.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		This API in context of TXN flow will be a busy poll wait.
*
******************************************************************************/
AieRC XAie_DmaWaitForDoneBusy(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir, u32 TimeOutUs)
{
	u8 TileType;
	u8 MaxNumChannels;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if (TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	if(TimeOutUs == 0U) {
		TimeOutUs = XAIE_DMA_WAITFORDONE_DEF_WAIT_TIME_US;
	}

	return DmaMod->WaitforDone(DevInst, Loc, DmaMod, ChNum, Dir, TimeOutUs,
		XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API is used to wait on DMA channel task queue till its free with atleast
* one task or till the timeout.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param    TimeOutUs - Minimum timeout value in micro seconds.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note     This API in context of TXN flow will be a yeilded poll wait.
*
******************************************************************************/
AieRC XAie_DmaWaitForBdTaskQueue(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u32 TimeOutUs)
{
	u8 TileType;
	u8 MaxNumChannels;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, (u8)Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	if(TimeOutUs == 0U) {
		TimeOutUs = XAIE_DMA_WAITFORDONE_DEF_WAIT_TIME_US;
	}

	if (DmaMod->WaitforBdTaskQueue) {
		return DmaMod->WaitforBdTaskQueue(DevInst, Loc, DmaMod, ChNum, Dir,
			TimeOutUs, XAIE_DISABLE);
	} else {
		XAIE_ERROR("WaitForBdTaskQueue is not supported/implemented\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}
}

/*****************************************************************************/
/**
*
* This API is used to wait on DMA channel task queue till its free with atleast
* one task or till the timeout.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param    TimeOutUs - Minimum timeout value in micro seconds.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note     This API in context of TXN flow will be a busy poll wait.
*
******************************************************************************/
AieRC XAie_DmaWaitForBdTaskQueueBusy(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u32 TimeOutUs)
{
	u8 TileType;
	u8 MaxNumChannels;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, (u8)Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	if(TimeOutUs == 0U) {
		TimeOutUs = XAIE_DMA_WAITFORDONE_DEF_WAIT_TIME_US;
	}

	if (DmaMod->WaitforBdTaskQueue) {
		return DmaMod->WaitforBdTaskQueue(DevInst, Loc, DmaMod, ChNum, Dir,
			TimeOutUs, XAIE_ENABLE);
	} else {
		XAIE_ERROR("WaitForBdTaskQueue is not supported/implemented\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}
}

/*****************************************************************************/
/**
*
* This API return the maximum queue size of the dma given a tile location.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	QueueSize: Pointer to store the queue size.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaGetMaxQueueSize(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 *QueueSize)
{
	u8 TileType;

	if((DevInst == XAIE_NULL) || (QueueSize == NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* TODO: If the request is for MM2S_Control channel,
	 * how to get the MaxQueue Size
	 */

	*QueueSize = DevInst->DevProp.DevMod[TileType].DmaMod->ChProp->StartQSizeMax;
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API pushes a Buffer Descriptor number, configures repeat count and token
* status to start channel queue.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	BdNum: Bd number to be pushed to the queue.
* 		RepeatCount: Number of times the task is to be repeated.
* 		EnTokenIssue: Determines whether or not issue a token when task
* 			     is completed
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		This feature is not supported for AIE. For AIE-ML the enable
*		token issue can be XAIE_ENABLE or XAIE_DISABLE.
*		This API doesn't support out of order.
*
******************************************************************************/
AieRC XAie_DmaChannelSetStartQueue(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u16 BdNum, u32 RepeatCount,
		u8 EnTokenIssue)
{
	XAie_DmaDeclareQueueConfig(DmaQueueDesc, BdNum, RepeatCount,
			EnTokenIssue, XAIE_DISABLE);

	return XAie_DmaChannelSetStartQueueGeneric(DevInst, Loc, ChNum, Dir,
			&DmaQueueDesc);
}

/*****************************************************************************/
/**
*
* This API pushes a Buffer Descriptor number, configures repeat count and token
* status to start channel queue.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	DmaQueueDesc: Pointer of DMA queue descriptor
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		This feature is not supported for AIE.
*
******************************************************************************/
AieRC XAie_DmaChannelSetStartQueueGeneric(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir,
		XAie_DmaQueueDesc *DmaQueueDesc)
{
	AieRC RC;
	u8 TileType;
	u16 StartBd;
	u8 MaxNumChannels;
	u8 MaxNumBds;
	u32 Val = 0;
	u64 Addr;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance to start queue\n");
		return XAIE_INVALID_ARGS;
	}

	if(DmaQueueDesc == XAIE_NULL) {
		XAIE_ERROR("Invalid Dma queue description pointer to start queue.\n");
		return XAIE_INVALID_ARGS;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL ||
		TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type to start queue\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(DmaMod->RepeatCount == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Repeat count feature in start queue is not supported for this device generation\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if(DmaMod->EnTokenIssue == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Enable token issue feature in start queue is not supported for this device generation\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	if((DmaQueueDesc->RepeatCount < 1U) ||
		(DmaQueueDesc->RepeatCount > DmaMod->ChProp->MaxRepeatCount)) {
		XAIE_ERROR("Invalid Repeat Count: %d\n",
			DmaQueueDesc->RepeatCount);
		return XAIE_INVALID_ARGS;
	}

	if(DmaQueueDesc->OutOfOrder != XAIE_ENABLE) {
		StartBd = DmaQueueDesc->StartBd;

		if(!_XAie_DmaTileAndChannelDirSupportsPvtBuffPoolBds(DevInst->DevProp.DevGen, TileType, Dir))
			MaxNumBds = _XAie_DmaGetSharedMaxNumBds(DmaMod, DevInst->DevProp.DevGen, TileType, DevInst->AppMode);
		else
			MaxNumBds = _XAie_DmaGetPrivateMaxNumBds(DmaMod, DevInst->DevProp.DevGen, TileType, Dir);
		if(StartBd >= MaxNumBds) {
			XAIE_ERROR("Invalid BD number\n");
			return XAIE_INVALID_BD_NUM;
		}

		RC = DmaMod->BdChValidity(DmaMod, Dir, StartBd, ChNum);
		if(RC != XAIE_OK) {
			return RC;
		}
		/* AIE4 single-app mode BD adjustment for ShimNOC tiles:
		* For channels accessing BDs 16-31, subtract 16 to get the correct BD index. */
		if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen) &&
				(DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) &&
				(TileType == XAIEGBL_TILE_TYPE_SHIMNOC) &&
				((Dir == DMA_S2MM) || (Dir == DMA_MM2S)) &&
				(StartBd >= DmaMod->NumBds)) {
			StartBd -= DmaMod->NumBds;
		}
	} else {
		StartBd = 0;
	}

	if (!(_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen))) {
		Addr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			DmaMod->StartQueueBase + ChNum * (u32)DmaMod->ChIdxOffset +
			(u32)((u8)Dir * (u32)DmaMod->ChIdxOffset * DmaMod->NumChannels);
	} else {
		Addr = _XAie4_DmaGetChannelCtrlAddr(DevInst, DmaMod, Loc, Dir, ChNum);
		/* start queue Register = Ctrl register addr + 0x04 */
		Addr += 0x04;
	}

	if (_XAie_CheckPrecisionExceeds(DmaMod->ChProp->StartBd.Lsb,
			_XAie_MaxBitsNeeded(StartBd), MAX_VALID_AIE_REG_BIT_INDEX)  ||
		_XAie_CheckPrecisionExceeds(DmaMod->ChProp->RptCount.Lsb,
			_XAie_MaxBitsNeeded(DmaQueueDesc->RepeatCount - 1U), MAX_VALID_AIE_REG_BIT_INDEX) ||
		_XAie_CheckPrecisionExceeds(DmaMod->ChProp->EnToken.Lsb,
			_XAie_MaxBitsNeeded(DmaQueueDesc->EnTokenIssue), MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	Val = XAie_SetField(StartBd, DmaMod->ChProp->StartBd.Lsb,
			DmaMod->ChProp->StartBd.Mask) |
		XAie_SetField((DmaQueueDesc->RepeatCount - 1U),
			DmaMod->ChProp->RptCount.Lsb,
			DmaMod->ChProp->RptCount.Mask) |
		XAie_SetField(DmaQueueDesc->EnTokenIssue,
			DmaMod->ChProp->EnToken.Lsb,
			DmaMod->ChProp->EnToken.Mask);

	return XAie_Write32(DevInst, Addr, Val);
}

/*****************************************************************************/
/**
*
* This API initializes the Dma Channel Descriptor for AIE DMAs for a given
* location.
*
* @param	DevInst: Device Instance.
* @param	DmaChannelDesc: Pointer to user allocated dma channel descriptor
* @param	Loc: Location of AIE-ML Tile
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		This API should be called before setting individual fields for
* 		Dma Channel Descriptor. This API works only for AIE-ML and has
* 		no effect on AIE.
*
******************************************************************************/
AieRC XAie_DmaChannelDescInit(XAie_DevInst *DevInst,
		XAie_DmaChannelDesc *DmaChannelDesc, XAie_LocType Loc)
{
	u8 TileType;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) || (DmaChannelDesc == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;

	memset((void *)DmaChannelDesc, 0, sizeof(XAie_DmaChannelDesc));

	DmaChannelDesc->TileType = TileType;
	DmaChannelDesc->IsReady = XAIE_COMPONENT_IS_READY;
	DmaChannelDesc->DmaMod = DmaMod;

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API enables/disables the Encompression bit in the DMA Channel Descriptor.
*
* @param	DmaChannelDesc: Initialized Dma channel Descriptor.
* @param	EnCompression: XAIE_ENABLE or XAIE_DISABLE for
* 				Compression / Decompression.
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets the enable/disable choice for
* 		Compression / Decompression in dma channel descriptor and does
* 		not configure the descriptor field in the hardware.
* 		This API works only for AIE-ML and has no effect on AIE.
*
******************************************************************************/
AieRC XAie_DmaChannelEnCompression(XAie_DmaChannelDesc *DmaChannelDesc,
		u8 EnCompression)
{
	const XAie_DmaMod *DmaMod;

	if((DmaChannelDesc == XAIE_NULL) ||
			(DmaChannelDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaChannelDesc->DmaMod;
	if(DmaMod->ChProp->HasEnCompression == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaChannelDesc->EnCompression = EnCompression;

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API enables/disables out of order mode in the DMA Channel Descriptor.
*
* @param	DmaChannelDesc: Initialized Dma Channel Descriptor.
* @param	EnOutofOrder: XAIE_ENABLE or XAIE_DISABLE for out of order mode.
* 				XAIE_DISABLE = in-order mode.
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets the enable/disable choice for
* 		out of order mode in dma channel descriptor and does
* 		not configure the descriptor field in the hardware.
* 		This API works only for AIE-ML and has no effect on AIE.
*
******************************************************************************/
AieRC XAie_DmaChannelEnOutofOrder(XAie_DmaChannelDesc *DmaChannelDesc,
		u8 EnOutofOrder)
{
	const XAie_DmaMod *DmaMod;

	if((DmaChannelDesc == XAIE_NULL) ||
			(DmaChannelDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaChannelDesc->DmaMod;
	if(DmaMod->ChProp->HasEnOutOfOrder == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaChannelDesc->EnOutofOrderId = EnOutofOrder;

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This api sets the task complete token controller id field in the DMA Channel
* Descriptor.
*
* @param	DmaChannelDesc: Initialized dma channel descriptor.
* @param	ControllerId: Task-complete-token controller ID field
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The api sets the controller id in the dma channel descriptor and
*		does not configure the descriptor field in the hardware. This
*		API works only for AIE-ML and has no effect on AIE.
*
******************************************************************************/
AieRC XAie_DmaChannelSetControllerId(XAie_DmaChannelDesc *DmaChannelDesc,
		u32 ControllerId)
{
	const XAie_DmaMod *DmaMod;

	if((DmaChannelDesc == XAIE_NULL) ||
			(DmaChannelDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaChannelDesc->DmaMod;
	if(DmaMod->ChProp->HasControllerId == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if (_XAie_CheckPrecisionExceedsForRightShift(DmaMod->ChProp->ControllerId.Lsb,
			DmaMod->ChProp->ControllerId.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	if(ControllerId > DmaMod->ChProp->ControllerId.Mask >>
			DmaMod->ChProp->ControllerId.Lsb) {
		XAIE_ERROR("Invalid ControllerId: %d\n", ControllerId);
		return XAIE_INVALID_ARGS;
	}

	DmaChannelDesc->ControllerId = ControllerId;

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This api sets the task complete token controller id field in the DMA Channel
* Descriptor.
*
* @param	DmaChannelDesc: Initialized dma channel descriptor.
* @param	FoTMode: Finish on TLAST mode (DMA_FoT_DISABLED,
* 			DMA_FoT_NO_COUNTS, DMA_FoT_COUNTS_WITH_TASK_TOKENS or
*			DMA_FoT_COUNTS_FROM_MM_REG)
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The api sets the FoT Mode in the dma channel descriptor and
*		does not configure the descriptor field in the hardware. This
*		API works only for AIE-ML and has no effect on AIE.
*
******************************************************************************/
AieRC XAie_DmaChannelSetFoTMode(XAie_DmaChannelDesc *DmaChannelDesc,
		XAie_DmaChannelFoTMode FoTMode)
{
	const XAie_DmaMod *DmaMod;

	if((DmaChannelDesc == XAIE_NULL) ||
			(DmaChannelDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaChannelDesc->DmaMod;
	if(DmaMod->ChProp->HasFoTMode == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if((u8)FoTMode > DmaMod->ChProp->MaxFoTMode) {
		XAIE_ERROR("Invalid FoTMode: %d\n", FoTMode);
		return XAIE_INVALID_ARGS;
	}

	DmaChannelDesc->FoTMode = (u8)FoTMode;

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API configures the Dma channel descriptor fields in the hardware for a
* particular tile location. This includes FoT mode, Controller id, out of order
* and Compression/Decompression.
*
*
* @param	DevInst: Device Instance.
* @param	DmaChannelDesc: Initialized Dma Channel Descriptor.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		This API works only for AIE-ML and has no effect on AIE.
*
******************************************************************************/
AieRC XAie_DmaWriteChannel(XAie_DevInst *DevInst,
		XAie_DmaChannelDesc *DmaChannelDesc, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir)
{
	u64 Addr;
	u32 Val;
	u8 MaxNumChannels;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) || (DmaChannelDesc == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE) {
		XAIE_ERROR("Feature not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if(Dir >= DMA_MAX) {
		XAIE_ERROR("Invalid DMA direction\n");
		return XAIE_INVALID_ARGS;
	}

	if((DmaChannelDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Dma Channel descriptor not initilized\n");
		return XAIE_INVALID_DMA_DESC;
	}

	if(DmaChannelDesc->TileType != DevInst->DevOps->GetTTypefromLoc(DevInst, Loc)) {
		XAIE_ERROR("Tile type mismatch\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[DmaChannelDesc->TileType].DmaMod;
	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod,
			DmaChannelDesc->TileType, Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, DmaChannelDesc->TileType,
		Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	if (!(_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)))
		Addr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			DmaMod->ChCtrlBase + ChNum * (u32)DmaMod->ChIdxOffset +
			(u32)((u8)Dir * (u32)DmaMod->ChIdxOffset * DmaMod->NumChannels);
	else
		Addr = _XAie4_DmaGetChannelCtrlAddr(DevInst, DmaMod, Loc, Dir, ChNum);
	
	if (_XAie_CheckPrecisionExceeds(DmaMod->ChProp->ControllerId.Lsb,
			_XAie_MaxBitsNeeded(DmaChannelDesc->ControllerId), MAX_VALID_AIE_REG_BIT_INDEX)  ||
		_XAie_CheckPrecisionExceeds(DmaMod->ChProp->FoTMode.Lsb,
			_XAie_MaxBitsNeeded(DmaChannelDesc->FoTMode), MAX_VALID_AIE_REG_BIT_INDEX) ||
		_XAie_CheckPrecisionExceeds(DmaMod->ChProp->EnOutofOrder.Lsb,
			_XAie_MaxBitsNeeded(DmaChannelDesc->EnOutofOrderId), MAX_VALID_AIE_REG_BIT_INDEX)  ||
		_XAie_CheckPrecisionExceeds(DmaMod->ChProp->EnCompression.Lsb,
			_XAie_MaxBitsNeeded(DmaChannelDesc->EnCompression), MAX_VALID_AIE_REG_BIT_INDEX)){
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	Val = XAie_SetField(DmaChannelDesc->EnOutofOrderId, (DmaMod->ChProp->EnOutofOrder.Lsb),
			(DmaMod->ChProp->EnOutofOrder.Mask)) |
		XAie_SetField(DmaChannelDesc->EnCompression, (DmaMod->ChProp->EnCompression.Lsb),
			(DmaMod->ChProp->EnCompression.Mask)) |
		XAie_SetField(DmaChannelDesc->ControllerId, (DmaMod->ChProp->ControllerId.Lsb),
			(DmaMod->ChProp->ControllerId.Mask)) |
		XAie_SetField(DmaChannelDesc->FoTMode, (DmaMod->ChProp->FoTMode.Lsb),
			(DmaMod->ChProp->FoTMode.Mask));

	return XAie_Write32(DevInst, Addr, Val);
}

/******************************************************************************/
/**
* This api sets the number of 32 bit words to be added before and after each
* dimenstion for a DMA BD descriptor.
*
* @param	DmaDesc: Initialized dma descriptor.
* @param	PadTensor: Padding tensor with After and Before values for each
*		dimension.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
*******************************************************************************/
AieRC XAie_DmaSetPadding(XAie_DmaDesc *DmaDesc, XAie_DmaPadTensor *PadTensor)
{
	const XAie_DmaMod *DmaMod;
	if((DmaDesc == XAIE_NULL) || (PadTensor == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->Padding == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if ( PadTensor->NumDim > (sizeof(DmaDesc->PadDesc)/sizeof(DmaDesc->PadDesc[0])) )
	{
		XAIE_ERROR("Max dimension supported %d, but requested %d\n", (sizeof(DmaDesc->PadDesc)/sizeof(DmaDesc->PadDesc[0])), PadTensor->NumDim);
		return XAIE_INVALID_ARGS;
	}

	for(u8 i = 0U; i < (sizeof(DmaDesc->PadDesc)/sizeof(DmaDesc->PadDesc[0])); i++) {
		if (i < PadTensor->NumDim) {
			DmaDesc->PadDesc[i].After = PadTensor->PadDesc[i].After;
			DmaDesc->PadDesc[i].Before = PadTensor->PadDesc[i].Before;
		} else {
			DmaDesc->PadDesc[i].After = 0;
			DmaDesc->PadDesc[i].Before = 0;
		}
	}

	return XAIE_OK;
}

/******************************************************************************/
/**
* This api sets the number of zeros to be added before or after  the given
* dimension for a dma descriptor.
*
* @param	DmaDesc: Initialized dma descriptor.
* @param	NumZeros: No. of zeros to be added
* @param	Dim: Dimension - 0, 1 or 2.
* @param	Pos: Position for zeros padding i.e.
* 			DMA_ZERO_PADDING_BEFORE or DMA_ZERO_PADDING_AFTER
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
*******************************************************************************/
AieRC XAie_DmaSetZeroPadding(XAie_DmaDesc *DmaDesc, u8 Dim,
		XAie_DmaZeroPaddingPos Pos, u8 NumZeros)
{
	const XAie_DmaMod *DmaMod;
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	/* TODO: Raman  zero-padding is not supported in AIE2P and AIE4 */
	if (_XAie_IsDeviceGenAIE4(DmaDesc->DevGen)) {
		XAIE_ERROR("DevGen = %d, doesn't support Zeropadding, use XAie_DmaSetPadValue\n", DmaDesc->DevGen);
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->Padding == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	switch(Pos){
	case DMA_ZERO_PADDING_BEFORE: {
		switch(Dim){
		case 0U:
		case 1U:
		case 2U:
		{
			DmaDesc->PadDesc[Dim].Before = NumZeros;
			break;
		}
		default:
			XAIE_ERROR("Invalid Dimension: %d\n", Dim);
			return XAIE_INVALID_ARGS;
		}
		break;
	}

	case DMA_ZERO_PADDING_AFTER: {
		switch(Dim){
		case 0U:
		case 1U:
		case 2U:
		{
			DmaDesc->PadDesc[Dim].After = NumZeros;
			break;
		}
		default:
			XAIE_ERROR("Invalid Dimension: %d\n", Dim);
			return XAIE_INVALID_ARGS;
		}
		break;
	}
	default:
		XAIE_ERROR("Invalid Position: %d\n", Pos);
		return XAIE_INVALID_ARGS;
	}

	return XAIE_OK;
}

/******************************************************************************/
/**
* This api enables the DMA to assert TLAST signal after the DMA transfer is
* complete. By default, the TLAST assertion is enabled in the DmaDesc during
* dma descriptor initialization.
*
* @param	DmaDesc: Initialized dma descriptor.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
*******************************************************************************/
AieRC XAie_DmaTlastEnable(XAie_DmaDesc *DmaDesc)
{
	const XAie_DmaMod *DmaMod;
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->TlastSuppress == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaDesc->TlastSuppress = XAIE_DISABLE;

	return XAIE_OK;
}

/******************************************************************************/
/**
* This api configures the DMA descriptor to not assert the TLAST signal after
* the DMA transfer is complete. By default, the TLAST assertion is enabled in
* the DmaDesc during dma descriptor initialization.
*
* @param	DmaDesc: Initialized dma descriptor.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
*******************************************************************************/
AieRC XAie_DmaTlastDisable(XAie_DmaDesc *DmaDesc)
{
	const XAie_DmaMod *DmaMod;
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->TlastSuppress == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaDesc->TlastSuppress = XAIE_ENABLE;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API reads the length of the buffer descriptor in the dma module.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	Len: Stores the Length of all the BDs in bytes.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Returns 0 if the Buffer Descriptor Valid bit is 0.
*
******************************************************************************/
AieRC XAie_DmaGetBdLen(XAie_DevInst *DevInst, XAie_LocType Loc, u32 *Len,
		u16 BdNum)
{
	u8 TileType;
	u64 RegAddr;
	u32 RegVal, Valid;
	AieRC RC;
	u8 MaxNumBds;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) || (Len == NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;

	MaxNumBds = _XAie_DmaGetSharedMaxNumBds(DmaMod, DevInst->DevProp.DevGen, TileType, DevInst->AppMode);
	if (BdNum >= MaxNumBds) {
		XAIE_ERROR("Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}

	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
		RC = DmaMod->GetBdLen(DevInst, DmaMod, Loc, Len, BdNum);
		if (RC != XAIE_OK) {
			*Len = 0;
			return RC;
		}

		return XAIE_OK;
	} else {		

		RegAddr = (u64)(DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset) +
			XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			DmaMod->BdProp->BdEn->ValidBd.Idx * 4U;
		RC = XAie_Read32(DevInst, RegAddr, &RegVal);
		if(RC != XAIE_OK) {
			return RC;
		}

		if (_XAie_CheckPrecisionExceedsForRightShift(DmaMod->BdProp->BdEn->ValidBd.Lsb,
				DmaMod->BdProp->BdEn->ValidBd.Mask)) {
			XAIE_ERROR("Check Precision Exceeds Failed\n");
			return XAIE_ERR;
		}
		Valid = XAie_GetField(RegVal, DmaMod->BdProp->BdEn->ValidBd.Lsb,
				DmaMod->BdProp->BdEn->ValidBd.Mask);
		if(Valid == 1U) {
			RegAddr = (u64)(DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset)
				+ XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
				DmaMod->BdProp->BufferLen.Idx * 4U;
			RC = XAie_Read32(DevInst, RegAddr, &RegVal);
			if(RC != XAIE_OK) {
				return RC;
			}

			if (_XAie_CheckPrecisionExceedsForRightShift(DmaMod->BdProp->BufferLen.Lsb,
					DmaMod->BdProp->BufferLen.Mask)) {
				XAIE_ERROR("Check Precision Exceeds Failed\n");
				return XAIE_ERR;
			}

			*Len = (XAie_GetField(RegVal, DmaMod->BdProp->BufferLen.Lsb,
				DmaMod->BdProp->BufferLen.Mask) +
				DmaMod->BdProp->LenActualOffset)
				<< XAIE_DMA_32BIT_TXFER_LEN;
		} else {
			*Len = 0;
		}
	}
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API updates the length of the buffer descriptor in the dma module.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	Len: Length of BD in bytes.
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		This API accesses the hardware directly and does not operate
*		on software descriptor.
******************************************************************************/
AieRC XAie_DmaUpdateBdLen(XAie_DevInst *DevInst, XAie_LocType Loc, u32 Len,
		u16 BdNum)
{
	const XAie_DmaMod *DmaMod;
	u32 AdjustedLen;
	u8 TileType;
	u8 MaxNumBds;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;

	MaxNumBds = _XAie_DmaGetSharedMaxNumBds(DmaMod, DevInst->DevProp.DevGen, TileType, DevInst->AppMode);
	if (BdNum >= MaxNumBds) {
		XAIE_ERROR("Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}

	AdjustedLen = (Len >> XAIE_DMA_32BIT_TXFER_LEN) -
		DmaMod->BdProp->LenActualOffset;

	return DmaMod->UpdateBdLen(DevInst, DmaMod, Loc, AdjustedLen, BdNum);
}

/*****************************************************************************/
/**
*
* This API updates the address of the buffer descriptor in the dma module.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	Addr: Buffer address
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		This API accesses the hardware directly and does not operate
*		on software descriptor.
******************************************************************************/
AieRC XAie_DmaUpdateBdAddr(XAie_DevInst *DevInst, XAie_LocType Loc, u64 Addr,
		u16 BdNum)
{
	const XAie_DmaMod *DmaMod;
	u8 TileType;
	u8 MaxNumBds;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;

	MaxNumBds = _XAie_DmaGetSharedMaxNumBds(DmaMod, DevInst->DevProp.DevGen, TileType, DevInst->AppMode);
	if (BdNum >= MaxNumBds) {
		XAIE_ERROR("Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}

	if(((Addr & DmaMod->BdProp->AddrAlignMask) != 0U) ||
			(Addr > DmaMod->BdProp->AddrMax)) {
		XAIE_ERROR("Invalid Address\n");
		return XAIE_INVALID_ADDRESS;
	}

	if ((DmaMod->BdProp->AddrAlignShift > _XAie_MaxBitsNeeded(Addr)) &&
		(Addr > ((u64)1U << DmaMod->BdProp->AddrAlignShift))) {
		XAIE_ERROR("Address is not aligned\n");
		return XAIE_INVALID_ADDRESS;
	}
	Addr = Addr >> DmaMod->BdProp->AddrAlignShift;

	return DmaMod->UpdateBdAddr(DevInst, DmaMod, Loc, Addr, BdNum);
}

/*****************************************************************************/
/**
 * XAie_DmaGetBdLenPvtBuffPool - Retrieves the length of a buffer descriptor
 *                                from the private buffer pool.
 *
 * @DevInst: Device instance pointer.
 * @Loc: Location type.
 * @ChNum: Channel number.
 * @Dir: DMA direction.
 * @Len: Pointer to store the length of the buffer descriptor.
 * @BdNum: Buffer descriptor number.
 *
 * This function retrieves the length of a specific buffer descriptor from the
 * private buffer pool based on the provided channel number, DMA direction, 
 * location type, and buffer descriptor number. The length is stored in the 
 * memory location pointed to by @Len.
 *
 * Return: AieRC - Status of the operation.
*****************************************************************************/
AieRC XAie_DmaGetBdLenPvtBuffPool(XAie_DevInst *DevInst, XAie_LocType Loc,
			u8 ChNum, XAie_DmaDirection Dir, u32 *Len, u16 BdNum)
{
	u8 TileType;	
	AieRC RC;
	u8 MaxNumChannels, MaxNumBds;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) || (Len == NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;

	if (!_XAie_DmaTileAndChannelDirSupportsPvtBuffPoolBds(DevInst->DevProp.DevGen, TileType, Dir)) {
		XAIE_ERROR("Invalid DMA direction for private buffer pool\n");
		return XAIE_INVALID_DMA_DIRECTION;
	}

	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	MaxNumBds = _XAie_DmaGetPrivateMaxNumBds(DmaMod, DevInst->DevProp.DevGen, TileType, Dir);
	if (BdNum >= MaxNumBds) {
		XAIE_ERROR("Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}

	RC = DmaMod->GetBdLenPvtBuffPool(DevInst, DmaMod, Loc, ChNum, Dir,  Len, BdNum);
	if (RC != XAIE_OK) {
		*Len = 0;
		return RC;
	}

	return XAIE_OK;
	
}

/*****************************************************************************/
/**
 * XAie_DmaUpdateBdLenPvtBuffPool - Updates the buffer descriptor length in the private buffer pool.
 * @DevInst: Pointer to the device instance.
 * @Loc: Location type.
 * @ChNum: Channel number.
 * @Dir: DMA direction.
 * @Len: Length to be updated.
 * @BdNum: Buffer descriptor number.
 *
 * This function updates the length of a buffer descriptor in the private buffer pool
 * for a given channel and direction.
 *
 * Return: AieRC - Status of the operation.
*****************************************************************************/
AieRC XAie_DmaUpdateBdLenPvtBuffPool(XAie_DevInst *DevInst, XAie_LocType Loc,
			u8 ChNum, XAie_DmaDirection Dir, u32 Len, u16 BdNum)
{
	const XAie_DmaMod *DmaMod;
	u32 AdjustedLen;
	u8 TileType;
	u8 MaxNumChannels, MaxNumBds;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;

	if (!_XAie_DmaTileAndChannelDirSupportsPvtBuffPoolBds(DevInst->DevProp.DevGen, TileType, Dir)) {
		XAIE_ERROR("Invalid DMA direction for private buffer pool\n");
		return XAIE_INVALID_DMA_DIRECTION;
	}

	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	MaxNumBds = _XAie_DmaGetPrivateMaxNumBds(DmaMod, DevInst->DevProp.DevGen, TileType, Dir);
	if (BdNum >= MaxNumBds) {
		XAIE_ERROR("Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}

	AdjustedLen = (Len >> XAIE_DMA_32BIT_TXFER_LEN) -
		DmaMod->BdProp->LenActualOffset;

	return DmaMod->UpdateBdLenPvtBuffPool(DevInst, DmaMod, Loc, ChNum, Dir, AdjustedLen, BdNum);
}

/*****************************************************************************/
/**
 * XAie_DmaUpdateBdAddrPvtBuffPool - Updates the buffer descriptor address in the private buffer pool.
 * @DevInst: Pointer to the device instance.
 * @Loc: Location type.
 * @ChNum: Channel number.
 * @Dir: DMA direction.
 * @Addr: Address to be updated.
 * @BdNum: Buffer descriptor number.
 *
 * This function updates the address of a buffer descriptor in the private buffer pool
 * for a specified channel and direction.
 *
 * Return: AieRC - Return code indicating success or failure.
*****************************************************************************/
AieRC XAie_DmaUpdateBdAddrPvtBuffPool(XAie_DevInst *DevInst, XAie_LocType Loc,
			u8 ChNum, XAie_DmaDirection Dir, u64 Addr, u16 BdNum)
{
	const XAie_DmaMod *DmaMod;
	u8 TileType;
	u8 MaxNumChannels, MaxNumBds;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;

	if (!_XAie_DmaTileAndChannelDirSupportsPvtBuffPoolBds(DevInst->DevProp.DevGen, TileType, Dir)) {
		XAIE_ERROR("Invalid DMA direction for private buffer pool\n");
		return XAIE_INVALID_DMA_DIRECTION;
	}

	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod, TileType, Dir);
	if (_XAie_DmaValidateChannelNumber(DevInst, TileType, Dir, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	MaxNumBds = _XAie_DmaGetPrivateMaxNumBds(DmaMod, DevInst->DevProp.DevGen, TileType, Dir);
	if (BdNum >= MaxNumBds) {
		XAIE_ERROR("Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}

	if(((Addr & DmaMod->BdProp->AddrAlignMask) != 0U) ||
			(Addr > DmaMod->BdProp->AddrMax)) {
		XAIE_ERROR("Invalid Address\n");
		return XAIE_INVALID_ADDRESS;
	}

	if ((DmaMod->BdProp->AddrAlignShift > _XAie_MaxBitsNeeded(Addr)) &&
		(Addr > ((u64)1U << DmaMod->BdProp->AddrAlignShift))) {
		XAIE_ERROR("Address is not aligned\n");
		return XAIE_INVALID_ADDRESS;
	}
	Addr = Addr >> DmaMod->BdProp->AddrAlignShift;

	return DmaMod->UpdateBdAddrPvtBuffPool(DevInst, DmaMod, Loc, ChNum, Dir, Addr, BdNum);
}

/*****************************************************************************/
/**
 * XAie_DmaGetBdLenGeneric - Retrieves the length of a DMA buffer descriptor.
 * @DevInst: Pointer to the device instance.
 * @Loc: Location type of the DMA engine.
 * @ChNum: Channel number of the DMA engine.
 * @Dir: Direction of the DMA transfer.
 * @Len: Pointer to store the length of the buffer descriptor.
 * @BdNum: Buffer descriptor number.
 *
 * This function retrieves the length of a specified buffer descriptor for a
 * given DMA engine channel and direction.
 *
 * Return: AieRC - Status of the operation.
*****************************************************************************/
AieRC XAie_DmaGetBdLenGeneric(XAie_DevInst *DevInst, XAie_LocType Loc, 
						u8 ChNum, XAie_DmaDirection Dir, u32 *Len,	u16 BdNum)
{
	u8 TileType;
	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
		
	if(!_XAie_DmaTileAndChannelDirSupportsPvtBuffPoolBds(DevInst->DevProp.DevGen, TileType, Dir))
		return XAie_DmaGetBdLen(DevInst, Loc, Len, BdNum);
	else
		return XAie_DmaGetBdLenPvtBuffPool(DevInst, Loc, ChNum, Dir, Len, BdNum);
}

/*****************************************************************************/
/**
 * XAie_DmaUpdateBdLenGeneric - Updates the length of a DMA buffer descriptor.
 * @DevInst: Pointer to the device instance.
 * @Loc: Location type of the DMA.
 * @ChNum: Channel number of the DMA.
 * @Dir: Direction of the DMA transfer.
 * @Len: Length to be updated in the buffer descriptor.
 * @BdNum: Buffer descriptor number to be updated.
 *
 * This function updates the length of a specified buffer descriptor for a
 * given DMA channel and direction.
 *
 * Return: AieRC - Return code indicating success or failure of the operation.
*****************************************************************************/
AieRC XAie_DmaUpdateBdLenGeneric(XAie_DevInst *DevInst, XAie_LocType Loc,
						u8 ChNum, XAie_DmaDirection Dir, u32 Len, u16 BdNum)
{
	u8 TileType;
	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
		
	if(!_XAie_DmaTileAndChannelDirSupportsPvtBuffPoolBds(DevInst->DevProp.DevGen, TileType, Dir))
		return XAie_DmaUpdateBdLen(DevInst, Loc, Len, BdNum);
	else
		return XAie_DmaUpdateBdLenPvtBuffPool(DevInst, Loc, ChNum, Dir, Len, BdNum);
}

/*****************************************************************************/
/**
 * XAie_DmaUpdateBdAddrGeneric - Updates the buffer descriptor address for a given DMA channel.
 * 
 * @DevInst: Pointer to the device instance.
 * @Loc: Location type of the AI engine.
 * @ChNum: Channel number for which the buffer descriptor address needs to be updated.
 * @Dir: Direction of the DMA transfer.
 * @Addr: New address to be set in the buffer descriptor.
 * @BdNum: Buffer descriptor number to be updated.
 * 
 * This function updates the address of a buffer descriptor for a specified DMA channel
 * in the AI engine. The direction of the DMA transfer and the buffer descriptor number
 * are also specified as parameters.
 * 
 * Return: AieRC - Status of the operation.
*****************************************************************************/
AieRC XAie_DmaUpdateBdAddrGeneric(XAie_DevInst *DevInst, XAie_LocType Loc,
						u8 ChNum, XAie_DmaDirection Dir, u64 Addr,	u16 BdNum)
{
	u8 TileType;
	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
		
	if(!_XAie_DmaTileAndChannelDirSupportsPvtBuffPoolBds(DevInst->DevProp.DevGen, TileType, Dir))
		return XAie_DmaUpdateBdAddr(DevInst, Loc, Addr, BdNum);
	else
		return XAie_DmaUpdateBdAddrPvtBuffPool(DevInst, Loc, ChNum, Dir, Addr, BdNum);
}

/*****************************************************************************/
/**
*
* This API configure the pad value for DMA MM2S Channel.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile.
* @param	ChNum: DMA MM2S Channel number.
* @param	PadValue: 32-bit pad value.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaSetPadValue(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		u32 PadValue)
{
	const XAie_DmaMod *DmaMod;
	u8 TileType;
	u8 MaxNumChannels;
	u64 Addr = 0;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(DmaMod->PadValueBase == XAIE_FEATURE_UNAVAILABLE) {
		XAIE_ERROR("Cannot configure pad value for this"
				" architecture\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	MaxNumChannels = _XAie_DmaGetMaxNumChannels(DevInst, DmaMod,
			TileType, DMA_MM2S);
	if (_XAie_DmaValidateChannelNumber(DevInst, TileType, DMA_MM2S, ChNum, MaxNumChannels)) {
		XAIE_ERROR("Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}
		
	if(_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen) &&
			(DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) ) {
		if (ChNum >= DmaMod->NumMm2sChannels) {
			Addr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, Addr);	
			ChNum -= DmaMod->NumMm2sChannels;			
		}
	}

	Addr |= XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
					 DmaMod->PadValueBase + ChNum * 0x4U;

	return XAie_Write32(DevInst, Addr, PadValue);
}

#endif /* XAIE_FEATURE_DMA_ENABLE */
/** @} */
