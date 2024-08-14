/******************************************************************************
* Copyright (C) 2023 - 2024 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite_shim_aie4.h
* @{
*
* This header file defines a lite shim interface for AIE4 type devices.
*
** <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Ramakant   27/12/2023  Initial creation
* </pre>
*
******************************************************************************/

#ifndef XAIE_LITE_SHIM_AIE4_H_
#define XAIE_LITE_SHIM_AIE4_H_

/***************************** Include Files *********************************/
#include "xaie_lite_hwcfg.h"
#include "xaiegbl_defs.h"
#include "xaiegbl.h"

/************************** Constant Definitions *****************************/
#define XAIE_MAX_NUM_NOC_INTR				3U
#define IS_TILE_NOC_TILE(Loc)				1
#define UPDT_NEXT_NOC_TILE_LOC(Loc)			(Loc).Col++

/************************** Function Prototypes  *****************************/
/*****************************************************************************/
/**
*
* This is API returns the shim tile type for a given device instance and tile
* location.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
*
* @return	TileType SHIMPL/SHIMNOC./
*
* @note		Internal only.
*
******************************************************************************/
static inline u8 _XAie_LGetShimTTypefromLoc(XAie_DevInst *DevInst,
			XAie_LocType Loc)
{
	(void) DevInst;
	(void) Loc;

	return XAIEGBL_TILE_TYPE_SHIMNOC;
}

/*****************************************************************************/
/**
*
* This is API returns the range of columns programmed to generate interrupt on
* the given IRQ channel.
*
* @param	IrqId: L2 IRQ ID.
*
* @return	Range of columns.
*
* @note		Internal only.
*
******************************************************************************/
static inline XAie_Range _XAie_MapIrqIdToCols(u8 IrqId)
{
	XAie_Range _MapIrqIdToCols[] = {
#if XAIE_DEV_SINGLEGEN == XAIE_DEV_GEN_AIE2P_STRIX_B0
		{.Start = 0, .Num = 2},
		{.Start = 2, .Num = 2},
		{.Start = 4, .Num = 2},
		{.Start = 6, .Num = 2},
#else
		{.Start = 0, .Num = 1},
		{.Start = 1, .Num = 1},
		{.Start = 2, .Num = 1},
		{.Start = 3, .Num = 1},
#endif
	};

	return _MapIrqIdToCols[IrqId];
}

/*****************************************************************************/
/**
*
* This is API returns the L2 IRQ ID for a given column.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
*
* @return	L2 IRQ ID.
*
* @note		Internal only.
*
******************************************************************************/
static inline u8 _XAie_MapColToIrqId(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 AbsCol = DevInst->StartCol + Loc.Col;

	return AbsCol / (XAIE_NUM_COLS / XAIE_MAX_NUM_NOC_INTR);
}

/*****************************************************************************/
/**
*
* This is API returns the HW Err IRQ ID for a given column.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
*
* @return	HW Err IRQ ID.
*
* @note		Internal only.
*
******************************************************************************/
static inline u8 _XAie_MapColToHWErrIrqId(XAie_DevInst *DevInst,
		XAie_LocType Loc)
{
	(void)DevInst;
	(void)Loc;

	/**
         * As per spec recomendation.
         * The HW error should be triggering NPI IRQ 1.
         */
	return 1;
}

/*****************************************************************************/
/**
* This API modifies(enable or disable) the clock control register for given shim.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE SHIM tile
* @param        Enable: XAIE_ENABLE to enable shim clock buffer,
*                       XAIE_DISABLE to disable.

* @note         It is internal function to this file
*
******************************************************************************/
static inline void _XAie_PrivilegeSetShimClk(XAie_DevInst *DevInst,
					     XAie_LocType Loc, u8 Enable)
{
	u64 RegAddr;
	u32 FldVal;

	RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
		XAIE_SHIM_TILE_MOD_CLOCK_CONTROL_0_REGOFF;
	FldVal = XAie_SetField(Enable,
			XAIE_SHIM_TILE_MOD_CLOCK_CONTROL_0_CTE_CLOCK_ENABLE_LSB,
			XAIE_SHIM_TILE_MOD_CLOCK_CONTROL_0_CTE_CLOCK_ENABLE_MASK);
	FldVal |= XAie_SetField(Enable,
			XAIE_SHIM_TILE_MOD_CLOCK_CONTROL_0_PL_INTERFACE_CLOCK_ENABLE_LSB,
			XAIE_SHIM_TILE_MOD_CLOCK_CONTROL_0_PL_INTERFACE_CLOCK_ENABLE_MASK);

	_XAie_LPartMaskWrite32(DevInst, RegAddr,
		XAIE_SHIM_TILE_MOD_CLOCK_CONTROL_0_MASK, FldVal);

	RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
		XAIE_SHIM_TILE_MOD_CLOCK_CONTROL_1_REGOFF;
	FldVal = XAie_SetField(Enable,
			XAIE_SHIM_TILE_NOC_MOD_CLOCK_CONTROL_1_CLOCK_ENABLE_LSB,
			XAIE_SHIM_TILE_NOC_MOD_CLOCK_CONTROL_1_CLOCK_ENABLE_MASK);

	_XAie_LPartMaskWrite32(DevInst, RegAddr,
		XAIE_SHIM_TILE_MOD_CLOCK_CONTROL_1_MASK, FldVal);

}

/*****************************************************************************/
/**
* This API modifies(enable or disable) the uc memory_privileged register for all
* cols in the partition.
*
* @param        DevInst: Device Instance
* @param        Enable: XAIE_ENABLE to enable the uc memory_privileged,
*                       XAIE_DISABLE to disable.
* @note         Modifying the uc.memory_privileged register while the uc core is
* 		enabled results in undefined behaviour.
* 		It is job of the caller to make sure the UC core is in sleep.
*
******************************************************************************/
static inline void _XAie_PrivilegeSetUCMemoryPrivileged(XAie_DevInst *DevInst,
							u8 Enable)
{
	u64 RegAddr;
	u32 Val;
	int i;

	for (i = 0; i < DevInst->NumCols; i++) {
		RegAddr = _XAie_LGetTileAddr(0, i) +
				XAIE_UC_MODULE_MEMORY_PRIVILEGED;
		Val = XAie_SetField(Enable,
				XAIE_UC_MODULE_MEMORY_PRIVILEGED_MEMORY_PRIVILEGED_LSB,
				XAIEGBL_UC_MODULE_MEMORY_PRIVILEGED_MEMORY_PRIVILEGED_MASK);
		_XAie_LPartMaskWrite32(DevInst, RegAddr,
				XAIEGBL_UC_MODULE_MEMORY_PRIVILEGED_MEMORY_PRIVILEGED_MASK,
				       Val);
	}
}

#endif /* XAIE_LITE_SHIM_AIE4_H_ */
