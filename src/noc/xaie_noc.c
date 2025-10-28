/******************************************************************************
* Copyright (C) 2023-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_plif.c
* @{
*
* This file contains routines for AIE tile control.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Sandip  11/12/2023  Initial creation. Functions are taken from the xaie_plif.c
* 1.1   Jignesh  7/7/2025  Added support for AIE2PS Shim NoC.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_feature_config.h"
#include "xaie_noc.h"
#include "xaiegbl_defs.h"
#include "xaie_helper.h"
#include "xaie_helper_internal.h"

#ifdef XAIE_FEATURE_PL_ENABLE

/************************** Constant Definitions *****************************/
#define XAIE_MUX_DEMUX_CONFIG_TYPE_PL	0x0
#define XAIE_MUX_DEMUX_CONFIG_TYPE_DMA	0x1
#define XAIE_MUX_DEMUX_CONFIG_TYPE_NOC	0x2

#define XAIE_STREAM_SOUTH_PORT_1	1U
#define XAIE_STREAM_SOUTH_PORT_2	2U
#define XAIE_STREAM_SOUTH_PORT_3	3U
#define XAIE_STREAM_SOUTH_PORT_4	4U
#define XAIE_STREAM_SOUTH_PORT_5	5U
#define XAIE_STREAM_SOUTH_PORT_6	6U
#define XAIE_STREAM_SOUTH_PORT_7	7U

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API configures the Mux registers in the AIE Shim NoC tiles. The input
* stream switch ports for incoming data from PL, NoC or DMA can be enabled using
* this API.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (2, 3, 6, 7)
* @param	InputConnectionType: XAIE_MUX_DEMUX_CONFIG_TYPE_PL,
*		XAIE_MUX_DEMUX_CONFIG_TYPE_DMA or XAIE_MUX_DEMUX_CONFIG_TYPE_NOC
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal API Only.
*
******************************************************************************/
static AieRC _XAie_ConfigShimNocMux(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, u8 InputConnectionType)
{
	u8 TileType;
	u32 FldVal;
	u32 FldMask;
	u64 RegAddr;
	const XAie_NocMod *NocMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	if(DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE2PS) {
		if((PortNum != XAIE_STREAM_SOUTH_PORT_1) &&
				(PortNum != XAIE_STREAM_SOUTH_PORT_3) &&
				(PortNum != XAIE_STREAM_SOUTH_PORT_5) &&
				(PortNum != XAIE_STREAM_SOUTH_PORT_7)) {
			XAIE_ERROR("Invalid port number for Mux\n");
			return XAIE_ERR_STREAM_PORT;
		}
	} else {
		if((PortNum != XAIE_STREAM_SOUTH_PORT_2) &&
				(PortNum != XAIE_STREAM_SOUTH_PORT_3) &&
				(PortNum != XAIE_STREAM_SOUTH_PORT_6) &&
				(PortNum != XAIE_STREAM_SOUTH_PORT_7)) {
			XAIE_ERROR("Invalid port number for Mux\n");
			return XAIE_ERR_STREAM_PORT;
		}
		/* Map the port numbers to 0, 1, 2, 3 */
		if(PortNum > 3U) {
			PortNum -= 4U;
		} else {
		PortNum -= 2U;
		}
	}

	NocMod = DevInst->DevProp.DevMod[TileType].NocMod;
	if (_XAie_CheckPrecisionExceeds(NocMod->ShimNocMux[PortNum].Lsb,
			_XAie_MaxBitsNeeded(InputConnectionType), MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	FldVal = (u32)(InputConnectionType << NocMod->ShimNocMux[PortNum].Lsb);
	FldMask = NocMod->ShimNocMux[PortNum].Mask;

	RegAddr = NocMod->ShimNocMuxOff +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* Mask write to the Mux register */
	return XAie_MaskWrite32(DevInst, RegAddr, FldMask, FldVal);
}

/*****************************************************************************/
/**
*
* This API configures the DeMux registers in the AIE Shim NoC tiles. The output
* stream switch ports for outgoing data to PL, NoC or DMA can be enabled using
* this API.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (2, 3, 4, 5)
* @param	OutputConnectionType: XAIE_MUX_DEMUX_CONFIG_TYPE_PL,
*		XAIE_MUX_DEMUX_CONFIG_TYPE_DMA or XAIE_MUX_DEMUX_CONFIG_TYPE_NOC
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal API Only.
*
******************************************************************************/
static AieRC _XAie_ConfigShimNocDeMux(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, u8 OutputConnectionType)
{
	u8 TileType;
	u32 FldVal;
	u32 FldMask;
	u64 RegAddr;
	const XAie_NocMod *NocMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}
	if(DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE2PS) {
			if((PortNum != XAIE_STREAM_SOUTH_PORT_1) &&
				(PortNum != XAIE_STREAM_SOUTH_PORT_2) &&
				(PortNum != XAIE_STREAM_SOUTH_PORT_3) &&
				(PortNum != XAIE_STREAM_SOUTH_PORT_5)) {
			XAIE_ERROR("Invalid port number\n");
			return XAIE_ERR_STREAM_PORT;
		}
	} else {
		if((PortNum != XAIE_STREAM_SOUTH_PORT_2) &&
				(PortNum != XAIE_STREAM_SOUTH_PORT_3) &&
				(PortNum != XAIE_STREAM_SOUTH_PORT_4) &&
				(PortNum != XAIE_STREAM_SOUTH_PORT_5)) {
			XAIE_ERROR("Invalid port number\n");
			return XAIE_ERR_STREAM_PORT;
		}
		/* Map the port numbers to 0, 1, 2, 3 */
		PortNum -= 2U;
	}

	NocMod = DevInst->DevProp.DevMod[TileType].NocMod;
	if (_XAie_CheckPrecisionExceeds(NocMod->ShimNocDeMux[PortNum].Lsb,
				_XAie_MaxBitsNeeded(OutputConnectionType), MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	FldVal = (u32)(OutputConnectionType << NocMod->ShimNocDeMux[PortNum].Lsb);
	FldMask = NocMod->ShimNocDeMux[PortNum].Mask;

	RegAddr = NocMod->ShimNocDeMuxOff +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* Mask write to the Mux register */
	return XAie_MaskWrite32(DevInst, RegAddr, FldMask, FldVal);
}

/*****************************************************************************/
/**
*
* This API enables the Shim DMA to input stream switch portconnection in the Mux
* configuration register for a given AIE Tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (3, 7)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EnableShimDmaToAieStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
	/* In AIE4, there no mux available in Noc module. As each interface
	have dedicated stream port available */
	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
		XAIE_ERROR("AIE4 Doesn't have Noc Mux\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}
	if((PortNum != XAIE_STREAM_SOUTH_PORT_3) &&
			(PortNum != XAIE_STREAM_SOUTH_PORT_7)) {
		XAIE_ERROR("Invalid port number\n");
		return XAIE_ERR_STREAM_PORT;
	}

	return _XAie_ConfigShimNocMux(DevInst, Loc, PortNum,
			XAIE_MUX_DEMUX_CONFIG_TYPE_DMA);
}

/*****************************************************************************/
/**
*
* This API enables the stream switch port to Shim DMA connection (for data
* going outside of AIE) in the DeMux configuration register for a given AIE
* tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (2, 3)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EnableAieToShimDmaStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
	/* In AIE4, there no mux available in Noc module. As each interface
	have dedicated stream port available */
	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
		XAIE_ERROR("AIE4 Doesn't have Noc Mux\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}
	
	if(DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE2PS) {
		if((PortNum != XAIE_STREAM_SOUTH_PORT_1) &&
			(PortNum != XAIE_STREAM_SOUTH_PORT_3)) {
				XAIE_ERROR("Invalid port number %d\n", PortNum);
		return XAIE_ERR_STREAM_PORT;
		}
	} else {
		if((PortNum != XAIE_STREAM_SOUTH_PORT_2) &&
				(PortNum != XAIE_STREAM_SOUTH_PORT_3)) {
					XAIE_ERROR("Invalid port number %d\n", PortNum);
			return XAIE_ERR_STREAM_PORT;
		}
	}

	return _XAie_ConfigShimNocDeMux(DevInst, Loc, PortNum,
			XAIE_MUX_DEMUX_CONFIG_TYPE_DMA);
}

/*****************************************************************************/
/**
*
* This API enables the NoC to input stream switch portconnection in the Mux
* configuration register for a given AIE Tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (2, 3, 6, 7)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EnableNoCToAieStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
        /* In AIE4, NOC streams are defeatured, so no need MUX
	registers are available to configure. So if Device
	gen is AIE4 or higher, just return with Invalid argument */
	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
		XAIE_ERROR("AIE4 Doesn't have Noc Mux\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	return _XAie_ConfigShimNocMux(DevInst, Loc, PortNum,
			XAIE_MUX_DEMUX_CONFIG_TYPE_NOC);
}

/*****************************************************************************/
/**
*
* This API enables the stream switch port to NoC connection (for data
* going outside of AIE) in the DeMux configuration register for a given AIE
* tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param        PortNum: Stream Port Number (2, 3, 4, 5)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EnableAieToNoCStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
        /* In AIE4, NOC streams are defeatured, so no need MUX
	registers are available to configure. So if Device
	gen is AIE4 or higher, just return with Invalid argument */
        if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
                XAIE_ERROR("AIE4 Doesn't have Noc Mux\n");
                return XAIE_FEATURE_NOT_SUPPORTED;
        }
	return _XAie_ConfigShimNocDeMux(DevInst, Loc, PortNum,
			XAIE_MUX_DEMUX_CONFIG_TYPE_NOC);
}

/*****************************************************************************/
/**
*
* This API enables the PL to input stream switch portconnection in the Mux
* configuration register for a given AIE Tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (2, 3, 6, 7)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		After a device reset, AIE<->PL connections are enabled by
*		default. This API has to be called only if AIE<->SHIMDMA or
*		AIE<->NOC connections have been enabled after a device reset.
*
******************************************************************************/
AieRC XAie_EnablePlToAieStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
        /* In AIE4, NOC streams are defeatured, so no need MUX
	registers are available to configure. So if Device
	gen is AIE4 or higher, just return with Invalid argument */
        if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
                XAIE_ERROR("AIE4 Doesn't have Noc Mux\n");
                return XAIE_FEATURE_NOT_SUPPORTED;
        }
	return _XAie_ConfigShimNocMux(DevInst, Loc, PortNum,
			XAIE_MUX_DEMUX_CONFIG_TYPE_PL);
}

/*****************************************************************************/
/**
*
* This API enables the stream switch port to PL connection (for data
* going outside of AIE) in the DeMux configuration register for a given AIE
* tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param        PortNum: Stream Port Number (2, 3, 4, 5)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		After a device reset, AIE<->PL connections are enabled by
*		default. This API has to be called only if AIE<->SHIMDMA or
*		AIE<->NOC connections have been enabled after a device reset.
*
******************************************************************************/
AieRC XAie_EnableAieToPlStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
        /* In AIE4, NOC streams are defeatured, so no need MUX
	registers are available to configure. So if Device
	gen is AIE4 or higher, just return with Invalid argument */
        if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
                XAIE_ERROR("AIE4 Doesn't have Noc Mux\n");
                return XAIE_FEATURE_NOT_SUPPORTED;
        }
	return _XAie_ConfigShimNocDeMux(DevInst, Loc, PortNum,
			XAIE_MUX_DEMUX_CONFIG_TYPE_PL);
}

#endif /* XAIE_FEATURE_PL_ENABLE */
/** @} */
