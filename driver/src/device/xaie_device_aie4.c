/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_device_aie4.c
* @{
*
* This file contains the apis for device specific operations of aie4.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Ramakant   11/26/2022  Initial creation
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_helper.h"
#include "xaie_helper_internal.h"
#include "xaie_reset_aie.h"
#include "xaie_tilectrl.h"

/***************************** Macro Definitions *****************************/
/* Set the timeout to maximum zeroization cycles required for Memtile DM zeroization in Me-Simulator.
   If polling timeout is less driver will return an error before zeroization is complete */
#ifdef __AIESOCKET__
	#define XAIE4_MEMZERO_POLL_TIMEOUT		100000
#else
	#define XAIE4_MEMZERO_POLL_TIMEOUT		1000
#endif
#define XAIE_CORE_DUAL_APP_A_VAL        0x1U
#define XAIE_CORE_DUAL_APP_A_TOP_VAL    0x2U
#define XAIE_CORE_DUAL_APP_B_BOTTOM_VAL 0x3U
#define XAIE_CORE_DUAL_APP_B_VAL        0x4U
#define XAIE_MIN_AIE_TILE_REQUEST       1U
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is the function used to get the tile type for a given device instance
* and tile location.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @return	TileType (AIETILE/MEMTILE/SHIMPL/SHIMNOC on success and MAX on
*		error)
*
* @note		Internal API only. This API returns tile type based on all
*		tiles on row 0 being shim noc tiles.
*
******************************************************************************/
u8 _XAie4_GetTTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	if(Loc.Col >= DevInst->NumCols) {
		XAIE_ERROR("Invalid column: %d\n", Loc.Col);
		return XAIEGBL_TILE_TYPE_MAX;
	}

	if(Loc.Row == DevInst->ShimRow) {
		return XAIEGBL_TILE_TYPE_SHIMNOC;
	} else if(Loc.Row >= DevInst->MemTileRowStart &&
			(Loc.Row < (DevInst->MemTileRowStart +
				     DevInst->MemTileNumRows))) {
		return XAIEGBL_TILE_TYPE_MEMTILE;
	} else if (Loc.Row >= DevInst->AieTileRowStart &&
			(Loc.Row < (DevInst->AieTileRowStart +
				     DevInst->AieTileNumRows))) {
		return XAIEGBL_TILE_TYPE_AIETILE;
	}

	XAIE_ERROR("Cannot find Tile Type\n");

	return XAIEGBL_TILE_TYPE_MAX;
}

/*****************************************************************************/
/**
*
* This API initialize the uC modules of the column to zero.
*
* @param	DevInst: Device Instance
*
* @return       XAIE_OK on success, error code on failure
*
* @note		It is not required to check the DevInst as the caller function
*		should provide the correct value.
*		Internal API only.
*
******************************************************************************/

AieRC _XAie4_ZeroInitUcMemory(XAie_DevInst *DevInst) {
	AieRC RC = XAIE_OK;
	u8 TileType = XAIEGBL_TILE_TYPE_SHIMNOC;
	const XAie_MemCtrlMod *MemCtrlUcMod, *MemCtrlUcMod_B;
	u64 RegAddr;
	u32 FldVal;
	XAie_LocType Loc;

	for(u8 C = 0; C < DevInst->NumCols; C++) {
		MemCtrlUcMod = DevInst->DevProp.DevMod[TileType].MemCtrlUcMod;
		MemCtrlUcMod_B = DevInst->DevProp.DevMod[TileType].MemCtrlUcMod_B;
		Loc = XAie_TileLoc(C, 0U);
		if(DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) {
			RegAddr = MemCtrlUcMod->MemZeroisationCtrlRegOff + XAie_GetTileAddr(DevInst, 0U, C);
			FldVal =  XAie_SetField(XAIE_ENABLE, MemCtrlUcMod->MemZeroisation.Lsb,MemCtrlUcMod->MemZeroisation.Mask);
			RC = XAie_MaskWrite32(DevInst, RegAddr,
					MemCtrlUcMod->MemZeroisation.Mask,
					FldVal);
			if(RC != XAIE_OK) {
				return RC;
			}
			RegAddr = MemCtrlUcMod_B->MemZeroisationCtrlRegOff + XAie_GetTileAddr(DevInst, 0U, C);
			FldVal =  XAie_SetField(XAIE_ENABLE, MemCtrlUcMod_B->MemZeroisation.Lsb,MemCtrlUcMod_B->MemZeroisation.Mask);
			RC = XAie_MaskWrite32(DevInst, RegAddr,
					MemCtrlUcMod->MemZeroisation.Mask,
					FldVal);
			if(RC != XAIE_OK) {
				return RC;
			}
		} else if (DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_A) {
			RegAddr = MemCtrlUcMod->MemZeroisationCtrlRegOff + XAie_GetTileAddr(DevInst, 0U, C);
			FldVal =  XAie_SetField(XAIE_ENABLE, MemCtrlUcMod->MemZeroisation.Lsb,MemCtrlUcMod->MemZeroisation.Mask);
			RC = XAie_MaskWrite32(DevInst, RegAddr,
					MemCtrlUcMod->MemZeroisation.Mask,
					FldVal);
			if(RC != XAIE_OK) {
				return RC;
			}
		} else {
			RegAddr = MemCtrlUcMod_B->MemZeroisationCtrlRegOff + XAie_GetTileAddr(DevInst, 0U, C);
			FldVal =  XAie_SetField(XAIE_ENABLE, MemCtrlUcMod_B->MemZeroisation.Lsb,MemCtrlUcMod_B->MemZeroisation.Mask);
			RC = XAie_MaskWrite32(DevInst, RegAddr,
					MemCtrlUcMod_B->MemZeroisation.Mask,
					FldVal);
			if(RC != XAIE_OK) {
				return RC;
			}
		}
		/* Read last col Reg to make sure all the writes went though and
		*  Zeroization is complete.
		*/
		if(C == DevInst->NumCols - 1U){
			if(DevInst->AppMode != XAIE_DEVICE_DUAL_APP_MODE_B) {
				RegAddr = MemCtrlUcMod->MemZeroisationCtrlRegOff + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
				return XAie_MaskPoll(DevInst, RegAddr,
						MemCtrlUcMod->MemZeroisation.Mask,
						0, XAIE4_MEMZERO_POLL_TIMEOUT);
			} else {
				RegAddr = MemCtrlUcMod_B->MemZeroisationCtrlRegOff + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
				return XAie_MaskPoll(DevInst, RegAddr,
						MemCtrlUcMod_B->MemZeroisation.Mask,
						0, XAIE4_MEMZERO_POLL_TIMEOUT);
			}
		}
	}

	/* Code should never reach here. */
    return XAIE_ERR;
}

/*****************************************************************************/
/**
* This API sets the column clock control register. Its configuration affects
* (enable or disable) all tile's clock above the Shim tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE SHIM tile
* @param        Enable: XAIE_ENABLE to enable column global clock buffer,
*                       XAIE_DISABLE to disable.
*
* @return       XAIE_OK for success, and error code for failure.
*
* @note         It is not required to check the DevInst and the Loc tile type
*               as the caller function should provide the correct value.
*               It is internal function to this file
*
******************************************************************************/
static AieRC _XAie4_PmSetColumnClockBuffer(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Enable)
{
	u8 TileType;
	u32 FldVal;
	u64 RegAddr;
	XAie_LocType ShimLoc = XAie_TileLoc(Loc.Col, 0U);
	const XAie_PlIfMod *PlIfMod;
	const XAie_ShimClkBufCntr *ClkBufCntr;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, ShimLoc);
	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
	ClkBufCntr = PlIfMod->ClkBufCntr;

	RegAddr = ClkBufCntr->RegOff +
			XAie_GetTileAddr(DevInst, 0U, Loc.Col);
	FldVal = XAie_SetField(Enable, ClkBufCntr->ClkBufEnable.Lsb,
			ClkBufCntr->ClkBufEnable.Mask);

	return XAie_MaskWrite32(DevInst, RegAddr, ClkBufCntr->ClkBufEnable.Mask,
			FldVal);
}

/*****************************************************************************/
/**
* This API enables the uc memory_privileged register for all
* cols in the partition.
*
* @param        DevInst: Device Instance
* @param        Enable: XAIE_ENABLE to enable the uc memory_privileged,
*
* @note         Modifying the uc.memory_privileged register while the uc core is
* 		enabled results in undefined behaviour.
* 		It is job of the caller to make sure the UC core is in sleep.
*
******************************************************************************/
AieRC _XAie4_SetUCMemoryPrivileged(XAie_DevInst *DevInst, u8 Enable) {
	AieRC RC = XAIE_OK;
	const XAie_MemCtrlMod *MemCtrlUcMod;
	u64 RegAddr;
	u8 TileType = XAIEGBL_TILE_TYPE_SHIMNOC;
	u32 FldVal;

	for(u8 C = 0; C < DevInst->NumCols; C++) {
		MemCtrlUcMod = DevInst->DevProp.DevMod[TileType].MemCtrlUcMod;
		RegAddr = MemCtrlUcMod->MemPrivilegeCtrlRegOff + XAie_GetTileAddr(DevInst, 0U, C);
		FldVal =  XAie_SetField(Enable, MemCtrlUcMod->MemPrivilegeCtrl.Lsb,MemCtrlUcMod->MemPrivilegeCtrl.Mask);
		RC = XAie_MaskWrite32(DevInst, RegAddr,
							MemCtrlUcMod->MemPrivilegeCtrl.Mask,
							FldVal);
		if(RC != XAIE_OK) {
			return RC;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API set/unset dual app mode for all the requested tiles in all the
* cols in the partition.
*
* @param        DevInst: Device Instance
* @param        XAie_BackendTilesArray: Tile arguments
*
*
******************************************************************************/
AieRC _XAie4_SetDualAppModePrivileged(XAie_DevInst *DevInst, XAie_BackendTilesArray *Args) {
	AieRC RC = XAIE_OK;
	const XAie_TileCtrlMod *TCtrlMod;
	u32 FldVal, Mask;
	u64 RegAddr;

	if(Args->Locs == NULL && DevInst->AppMode != XAIE_DEVICE_SINGLE_APP_MODE) {
		XAIE_ERROR("Tile location cannot be empty in Dual App mode\n");
		return XAIE_INVALID_ARGS;
	}

	for(u32 i = 0; i < Args->NumTiles; i++) {
		if (Args->Locs[i].Row == DevInst->ShimRow) {
			TCtrlMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_SHIMNOC].TileCtrlMod;
			RegAddr = TCtrlMod->DualAppModeRegOff +
						XAie_GetTileAddr(DevInst, Args->Locs[i].Row, Args->Locs[i].Col);
			Mask = TCtrlMod->DualAppControl.Mask;
			if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_A || DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B ) {
				FldVal = XAie_SetField(XAIE_ENABLE,
							TCtrlMod->DualAppControl.Lsb, Mask);
			} else {
				FldVal = XAie_SetField(XAIE_DISABLE,
							TCtrlMod->DualAppControl.Lsb, Mask);
			}

			RC = XAie_Write32(DevInst, RegAddr, FldVal);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to set Shim Tile Dual/Single App Mode\n");
				return RC;
			}
		} else if(Args->Locs[i].Row == DevInst->MemTileRowStart) {
			TCtrlMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_MEMTILE].TileCtrlMod;
			RegAddr = TCtrlMod->DualAppModeRegOff +
						XAie_GetTileAddr(DevInst, Args->Locs[i].Row, Args->Locs[i].Col);
			Mask = TCtrlMod->DualAppControl.Mask;
			if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_A || DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {
				FldVal = XAie_SetField(XAIE_ENABLE,
							TCtrlMod->DualAppControl.Lsb, Mask);
			} else {
				FldVal = XAie_SetField(XAIE_DISABLE,
							TCtrlMod->DualAppControl.Lsb, Mask);
			}

			RC = XAie_Write32(DevInst, RegAddr, FldVal);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to set Mem Tile Dual/Single App Mode\n");
				return RC;
			}
		} else if (Args->Locs[i].Row >= DevInst->AieTileRowStart ) {
			TCtrlMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE].TileCtrlMod;
			RegAddr = TCtrlMod->DualAppModeRegOff +
						XAie_GetTileAddr(DevInst, Args->Locs[i].Row, Args->Locs[i].Col);
			Mask = TCtrlMod->DualAppControl.Mask;
			if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_A) {
				if(Args->Locs[i].Row == ((DevInst->AieTileRowStart + DevInst->AieTileNumRows) - XAIE_MIN_AIE_TILE_REQUEST)) {
					FldVal = XAie_SetField(XAIE_CORE_DUAL_APP_A_TOP_VAL,
								TCtrlMod->DualAppControl.Lsb, Mask);
					RC = XAie_Write32(DevInst, RegAddr, FldVal);
				} else {
					FldVal = XAie_SetField(XAIE_CORE_DUAL_APP_A_VAL,
								TCtrlMod->DualAppControl.Lsb, Mask);
					RC = XAie_Write32(DevInst, RegAddr, FldVal);
				}
			} else if (DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {
				if(Args->Locs[i].Row == DevInst->AieTileRowStart) {
					FldVal = XAie_SetField(XAIE_CORE_DUAL_APP_B_BOTTOM_VAL,
								TCtrlMod->DualAppControl.Lsb, Mask);
					RC = XAie_Write32(DevInst, RegAddr, FldVal);
				} else {
					FldVal = XAie_SetField(XAIE_CORE_DUAL_APP_B_VAL,
							TCtrlMod->DualAppControl.Lsb, Mask);
					RC = XAie_Write32(DevInst, RegAddr, FldVal);
				}
			} else {
				FldVal = XAie_SetField(XAIE_DEVICE_SINGLE_APP_MODE,
							TCtrlMod->DualAppControl.Lsb, Mask);
				RC = XAie_Write32(DevInst, RegAddr, FldVal);
			}

			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to set AIE Tile Dual/Single App Mode\n");
				return RC;
			}
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API initialize the memories of the partition to zero.
*
* @param	DevInst: Device Instance
*
* @return       XAIE_OK on success, error code on failure
*
* @note		It is not required to check the DevInst as the caller function
*		should provide the correct value.
*		Internal API only.
*
******************************************************************************/
AieRC _XAie4_PartMemZeroInit(XAie_DevInst *DevInst)
{
	AieRC RC = XAIE_OK;
	const XAie_MemCtrlMod *MCtrlMod;
	u64 RegAddr;
	XAie_LocType Loc;

	for(u8 C = 0; C < DevInst->NumCols; C++) {
		for(u8 R = 1; R < DevInst->NumRows; R++) {
			u32 FldVal;
			u8 TileType, NumMods;

			Loc = XAie_TileLoc(C, R);
			TileType = DevInst->DevOps->GetTTypefromLoc(DevInst,
					Loc);
			NumMods = DevInst->DevProp.DevMod[TileType].NumModules;
			if(TileType == XAIEGBL_TILE_TYPE_MEMTILE) {
				if(DevInst->AppMode != XAIE_DEVICE_DUAL_APP_MODE_B) {
					MCtrlMod = DevInst->DevProp.DevMod[TileType].MemCtrlMod_A;
				} else {
					MCtrlMod = DevInst->DevProp.DevMod[TileType].MemCtrlMod_B;
				}
			} else {
				MCtrlMod = DevInst->DevProp.DevMod[TileType].MemCtrlMod;
			}
			for (u8 M = 0; M < NumMods; M++) {
				RegAddr = MCtrlMod[M].MemZeroisationCtrlRegOff +
					XAie_GetTileAddr(DevInst, R, C);
				FldVal = XAie_SetField(XAIE_ENABLE,
					MCtrlMod[M].MemZeroisation.Lsb,
					MCtrlMod[M].MemZeroisation.Mask);
				RC = XAie_MaskWrite32(DevInst, RegAddr,
					MCtrlMod[M].MemZeroisation.Mask,
					FldVal);
				if(RC != XAIE_OK) {
					XAIE_ERROR("Failed to zeroize partition mems.\n");
					return RC;
				}

				if((C == DevInst->NumCols - 1U) &&
						(R == DevInst->NumRows - 1U) &&
						(M == NumMods - 1U)) {
					RegAddr = MCtrlMod[M].MemZeroisationCtrlRegOff +
						XAie_GetTileAddr(DevInst,
								Loc.Row,
								Loc.Col);
					return XAie_MaskPoll(DevInst, RegAddr,
							MCtrlMod[M].MemZeroisation.Mask,
							0, XAIE4_MEMZERO_POLL_TIMEOUT);
				}
			}
		}
	}

	/* Code should never reach here. */
	return XAIE_ERR;
}

/*****************************************************************************/
/**
* This API enables clock for all the tiles passed as argument to this API.
*
* @param	DevInst: AI engine partition device instance pointer
* @param	Args: Backend tile args
*
* @return       XAIE_OK on success, error code on failure
** @note		Internal only.
*
*******************************************************************************/
AieRC _XAie4_RequestTiles(XAie_DevInst *DevInst, XAie_BackendTilesArray *Args)
{
	AieRC RC;
	u32 SetTileStatus;

	if(Args->Locs == NULL) {
		u32 NumTiles;

		XAie_LocType TileLoc = XAie_TileLoc(0, 1);
		NumTiles = (u32)((DevInst->NumRows - 1U) * (DevInst->NumCols));

		SetTileStatus = _XAie_GetTileBitPosFromLoc(DevInst, TileLoc);
		_XAie_SetBitInBitmap(DevInst->DevOps->TilesInUse, SetTileStatus,
				NumTiles);

		return DevInst->DevOps->SetPartColClockAfterRst(DevInst,
				XAIE_ENABLE);
	}

	/* Disable all the column clock and enable only the requested column clock */
	RC = _XAie_PmSetPartitionClock(DevInst, XAIE_DISABLE);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to set partition clock buffers.\n");
		return RC;
	}

	/* Clear the TilesInuse bitmap to reflect the current status */
	for(u32 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc;
		u32 ColClockStatus;

		Loc = XAie_TileLoc((u8)C, 1U);
		ColClockStatus = _XAie_GetTileBitPosFromLoc(DevInst, Loc);

		_XAie_ClrBitInBitmap(DevInst->DevOps->TilesInUse,
				ColClockStatus, (u32)(DevInst->NumRows - 1U));
	}

	for(u32 i = 0; i < Args->NumTiles; i++) {
		u32 ColClockStatus;

		if(Args->Locs[i].Col >= DevInst->NumCols || Args->Locs[i].Row >= DevInst->NumRows) {
			XAIE_ERROR("Invalid Tile Location \n");
			return XAIE_INVALID_TILE;
		}

		/*
		 * Shim rows are enabled by default
		 * Check if column clock buffer is already enabled and continue
		 * Get bitmap position from first row after shim
		 */
		ColClockStatus = _XAie_GetTileBitPosFromLoc(DevInst,
				XAie_TileLoc(Args->Locs[i].Col, 1));
		if (CheckBit(DevInst->DevOps->TilesInUse, ColClockStatus)) {
			continue;
		}

		RC = _XAie4_PmSetColumnClockBuffer(DevInst, Args->Locs[i],
				XAIE_ENABLE);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable clock for column: %d\n",
					Args->Locs[i].Col);
			return RC;
		}

		/*
		 * Set bitmap for entire column, row 1 to last row.
		 * Shim row is already set, so use NumRows-1
		 */
		_XAie_SetBitInBitmap(DevInst->DevOps->TilesInUse,
				ColClockStatus, (u32)(DevInst->NumRows - 1U));
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API sets isolation boundary of an AI engine partition after reset
*
* @param	DevInst: Device Instance
*
* @return       XAIE_OK on success, error code on failure
*
* @note	It is not required to check the DevInst as the caller function
*		should provide the correct value.
*		Internal API only.
*
******************************************************************************/
AieRC _XAie4_SetPartIsolationAfterRst(XAie_DevInst *DevInst)
{
	AieRC RC = XAIE_OK;
	u64 RegAddr;
	u32 FldVal, Mask;
	const XAie_TileCtrlMod *TCtrlMod;

	for(u8 C = 0; C < DevInst->NumCols; C++) {
		u8 Dir = 0;

		if (DevInst->AppMode != XAIE_DEVICE_SINGLE_APP_MODE) {
			Dir |= XAIE_ISOLATE_WEST_MASK | XAIE_ISOLATE_EAST_MASK ;
		} else {
			if(C == 0) {
				Dir |= XAIE_ISOLATE_WEST_MASK;
			}
			if (C == (u8)(DevInst->NumCols - 1U)) {
				Dir |= XAIE_ISOLATE_EAST_MASK;
			}
		}

		for(u8 R = 0; R < DevInst->NumRows; R++) {
			if(DevInst->AppMode != XAIE_DEVICE_SINGLE_APP_MODE && R == DevInst->ShimRow) {
				TCtrlMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_SHIMNOC].TileCtrlMod;
				RegAddr = TCtrlMod->TileCtrlAxiRegOff +
							XAie_GetTileAddr(DevInst, R, C);
				Mask = TCtrlMod->IsolateAxiEast.Mask | TCtrlMod->IsolateAxiWest.Mask;
				FldVal = XAie_SetField(Dir,
							TCtrlMod->IsolateAxiWest.Lsb, Mask);
				RC = XAie_Write32(DevInst, RegAddr, FldVal);
				if(RC != XAIE_OK) {
					XAIE_ERROR("Failed to set Shim Tile AXI-MM isolation.\n");
					return RC;
				}
			}

			RC = _XAie_TileCtrlSetIsolation(DevInst,
					XAie_TileLoc(C, R), Dir);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to set partition isolation.\n");
				return RC;
			}
		}
	}

	return RC;
}

/*****************************************************************************/
/**
*
* This API sets the Application reset bit for the specified partition.
*
* @param	DevInst: Device Instance
* @param	Enable: Indicate if to enable or disable Application SHIM reset
*			XAIE_ENABLE to enable Application reset, XAIE_DISABLE to
*			disable reset.
*
* @return	XAIE_OK for success, and error code for failure
*
* @note		Internal API only.
*
******************************************************************************/
AieRC _XAie4_SetPartApplicationReset(XAie_DevInst *DevInst, u8 Enable)
{
	u8 TileType;
	u32 FldVal;
	u64 RegAddr;
	AieRC RC;
	const XAie_PlIfMod *PlIfMod;
	const XAie_ShimRstMod *ShimTileRst;
	XAie_LocType Loc;

	for(u8 C = 0; C < DevInst->NumCols; C++) {
		Loc = XAie_TileLoc(C, 0);
		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
		PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
		ShimTileRst = PlIfMod->ShimTileRst;

		RegAddr = ShimTileRst->RegOff +
				XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
		if(DevInst->AppMode != XAIE_DEVICE_DUAL_APP_MODE_B) {
			FldVal = XAie_SetField(Enable,
					ShimTileRst->RstCntr.Lsb, ShimTileRst->RstCntr.Mask);
		} else {
			FldVal = XAie_SetField(Enable,
					ShimTileRst->RstCntr_B.Lsb, ShimTileRst->RstCntr_B.Mask);
		}

		RC = XAie_Write32(DevInst, RegAddr, FldVal);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to set Application resets.\n");
			return RC;
		}
	}
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API sets the L2 Split register bit for the specified partition.
* Based on L2Split value MemTile Data memory is divided between APP A and B
* Max supported L2 Split Value is 0x1F
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK for success, and error code for failure
*
* @note		Internal API only.
*
******************************************************************************/
AieRC _XAie4_PartInitL2Split(XAie_DevInst *DevInst) {
	AieRC RC = XAIE_OK;
	const XAie_TileCtrlMod *TCtrlMod;
	u32 FldVal, Mask;
	u64 RegAddr;

	for(u8 C = 0; C < DevInst->NumCols; C++) {
		for(u8 R = DevInst->MemTileRowStart; R <= DevInst->MemTileNumRows; R++) {
			TCtrlMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_MEMTILE].TileCtrlMod;
			RegAddr = TCtrlMod->L2SplitRegOff + XAie_GetTileAddr(DevInst, R, C);
			Mask = TCtrlMod->L2SplitControl.Mask;
			FldVal = XAie_SetField(DevInst->L2Split, TCtrlMod->DualAppControl.Lsb, Mask);
			RC = XAie_Write32(DevInst, RegAddr, FldVal);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to set L2 Split in Dual App Mode\n");
				return RC;
			}
		}
	}
	return RC;
}

/** @} */
