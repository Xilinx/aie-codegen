/******************************************************************************
* Copyright (C) 2023-2025 Advanced Micro Devices, Inc. All rights reserved.
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
#define XAIE_AIE4_ASYNC_ERROR_NPI_IRQ       1U
#define XAIE_MAX_NUM_NOC_INTR               3U
// Fixed the macro to return true only if Loc is SHIM tile.
#define IS_TILE_NOC_TILE(Loc)               (((Loc).Row == 0) ? 1 : 0)
#define UPDT_NEXT_NOC_TILE_LOC(Loc)         (Loc).Col++

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
	if(Loc.Col > (DevInst->StartCol + DevInst->NumCols)){
		XAIE_ERROR("Colum is out of range\n");
		return XAIE_INVALID_RANGE;
	}

	/**
	 * As per the latest agreement with MPNPU Firmware team, Since spatial
	 * sharing and dual app mode is not supported for AIE4 at this point of
	 * time, hence they want to use only NPI IRQ 1 for reporting Async Errors
	 * to MPNPU firmware from each column.
	 */
	return XAIE_AIE4_ASYNC_ERROR_NPI_IRQ;
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
	 * As per spec recomendation. The HW error should be triggering NPI IRQ 1.
	 * If AIE4 derived devices have any specific requiremnet then that needs
	 * to be handled here.
	 */
	return XAIE_AIE4_ASYNC_ERROR_NPI_IRQ;
}

/*****************************************************************************/
/**
* This API modifies(enable or disable) the clock control register for given shim.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE SHIM tile
* @param        Enable: XAIE_ENABLE to enable shim clock buffer,
*                       XAIE_DISABLE to disable.

* @note         Not Applicable for AIE4 architecture
*
******************************************************************************/
static inline void _XAie_PrivilegeSetShimClk(XAie_DevInst *DevInst,
					     XAie_LocType Loc, u8 Enable)
{
	(void)DevInst;
	(void)Loc;
	(void)Enable;

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
