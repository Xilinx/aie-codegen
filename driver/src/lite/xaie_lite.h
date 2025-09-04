/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite.h
* @{
*
* This header file defines a lightweight version of AIE driver APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Nishad  08/30/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_LITE_H
#define XAIE_LITE_H

#ifdef XAIE_FEATURE_LITE

#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaie_helper.h"

#define XAie_LDeclareDevInst(DevInst, _BaseAddr, _StartCol, _NumCols) \
	XAie_DevInst DevInst = { \
		.BaseAddr = (_BaseAddr), \
		.StartCol = (_StartCol), \
		.NumCols = (_NumCols), \
		.NumRows = (XAIE_NUM_ROWS), \
		.L2PreserveMem = 0, \
		.HostddrBuffAddr = 0, \
		.HostddrBuffSize = 0, \
		.HostddrBuff_SMID = 0, \
	}

/* Macro for Initialization of AIE4 Device Instance */
#define XAie_LDeclareAie4DevInst(DevInst, _BaseAddr, _StartCol, _NumCols, _AppMode) \
	XAie_DevInst DevInst = { \
		.BaseAddr = (_BaseAddr), \
		.StartCol = (_StartCol), \
		.NumCols = (_NumCols), \
		.NumRows = (XAIE_NUM_ROWS), \
		.AppMode = (_AppMode), \
		.HostddrBuffAddr = 0, \
		.HostddrBuffSize = 0, \
		.HostddrBuff_SMID = 0, \
		.HostddrBuff_AxUSER = 0 \
	}

#define XAie_A2SBuffWorkaroundInfo(DevInst, _HostddrBuffAddr, _HostddrBuffSize, _HostddrBuff_SMID, _HostddrBuff_AxUSER) \
        DevInst.HostddrBuffAddr = _HostddrBuffAddr, \
        DevInst.HostddrBuffSize = _HostddrBuffSize, \
        DevInst.HostddrBuff_SMID = _HostddrBuff_SMID, \
        DevInst.HostddrBuff_AxUSER = _HostddrBuff_AxUSER

#if XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE
#include "xaie_lite_aie.h"
#include "xaie_lite_shim_aie.h"
#elif XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIEML
#include "xaie_lite_aieml.h"
#include "xaie_lite_shim_aie.h"
#include "xaie_lite_shim_aieml.h"
#elif XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE2IPU
#include "xaie_lite_aieml.h"
#include "xaie_lite_shim_aie2ipu.h"
#elif ((XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE2P) || \
		(XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE2P_STRIX_A0) || \
		(XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE2P_STRIX_B0))
#include "xaie_lite_aieml.h"
#include "xaie_lite_shim_aie2p.h"
#elif ((XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE4_GENERIC) || \
	 	(XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE4) || \
		(XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE4_A))
#include "xaie_lite_aie4.h"
#include "xaie_lite_shim_aie4.h"
#else
#include <xaie_custom_device.h>
#endif

#define XAIE_ERROR_MSG(...)						\
	"[AIE ERROR] %s():%d: %s", __func__, __LINE__, __VA_ARGS__

#ifdef XAIE_ENABLE_INPUT_CHECK
#ifdef _ENABLE_IPU_LX6_
#include <printf.h>
#endif
#define XAIE_ERROR_RETURN(ERRCON, RET, ...) {	\
	if (ERRCON) {				\
		printf(__VA_ARGS__);		\
		return (RET);			\
	}					\
}
#else
#define XAIE_ERROR_RETURN(...)
#endif

/***************************** Macro Definitions *****************************/
#define XAIE_MEM_WORD_ALIGN_SHIFT       2U
#define XAIE_MEM_WORD_ALIGN_MASK        ((1U << XAIE_MEM_WORD_ALIGN_SHIFT) - 1U)
#define XAIE_MEM_WORD_ALIGN_SIZE        (1U << XAIE_MEM_WORD_ALIGN_SHIFT)
#define XAIE_MEM_WORD_LAST_BYTE 8U
#define XAIE_MEM_WORD_LAST_BYTE_MASK    ((1U << XAIE_MEM_WORD_LAST_BYTE) - 1U)

#define XAIE_MEM_WORD_ROUND_UP(Addr)    (((Addr) + XAIE_MEM_WORD_ALIGN_MASK) & \
                                                ~XAIE_MEM_WORD_ALIGN_MASK)
#define XAIE_MEM_WORD_ROUND_DOWN(Addr)  ((Addr) & (~XAIE_MEM_WORD_ALIGN_MASK))

/************************** Function Prototypes  *****************************/


/************************** Variable Definitions *****************************/
/************************** Function Prototypes  *****************************/
XAIE_AIG_EXPORT AieRC XAie_IsPartitionIdle(XAie_DevInst *DevInst);
XAIE_AIG_EXPORT AieRC XAie_ClearPartitionContext(XAie_DevInst *DevInst);
XAIE_AIG_EXPORT AieRC XAie_SetColumnClk(XAie_DevInst *DevInst, u8 Enable);
XAIE_AIG_EXPORT AieRC XAie_ClearCoreReg(XAie_DevInst *DevInst);
XAIE_AIG_EXPORT AieRC XAie_NpiSetPartProtectedReg(XAie_DevInst *DevInst, u8 enable);
XAIE_AIG_EXPORT AieRC XAie_PowerOnReset(XAie_DevInst *DevInst, XAie_PartPorOpts *PorOptions);
XAIE_AIG_EXPORT AieRC XAie_TileClockControl(XAie_DevInst *DevInst, XAie_LocType *Locs,u8 NumTiles, u8 Enable);
XAIE_AIG_EXPORT AieRC XAie_ConfigureShimDmaRegisters(XAie_DevInst *DevInst, XAie_ShimOpts *ShimOptions);
XAIE_AIG_EXPORT AieRC XAie_TrigColIntr(XAie_DevInst *DevInst, u8 BcChan);
XAIE_AIG_EXPORT AieRC XAie_PauseMem(XAie_DevInst *DevInst);
XAIE_AIG_EXPORT AieRC XAie_CfgPrivilegeHwErrIrq(XAie_DevInst *DevInst, XAie_HwErrCfg HwErrCf);
XAIE_AIG_EXPORT AieRC XAie_WakeupShimUc(XAie_DevInst *DevInst, u8 ColNum);

XAIE_AIG_EXPORT AieRC XAie_LMemBlockWrite(XAie_DevInst *DevInst, XAie_LocType Loc, u32 Addr,
                const void *Src, u32 Size);
XAIE_AIG_EXPORT AieRC XAie_LMemBlockRead(XAie_DevInst *DevInst, XAie_LocType Loc, u32 Addr,
                void *Dst, u32 Size);

/* Custom API for Reading/Writing to registers directly for Self-test from IPU FW*/
XAIE_AIG_EXPORT u64 XAie_GenRead(u64 RegAddr);
XAIE_AIG_EXPORT void XAie_GenWrite(u64 RegAddr, u32 Value);
XAIE_AIG_EXPORT AieRC XAie_GenNPIInterrupt(XAie_DevInst *DevInst, u8 IntLine);
XAIE_AIG_EXPORT AieRC XAie_ClearNPIInterrupt(XAie_DevInst *DevInst, u8 IntLine);
XAIE_AIG_EXPORT AieRC XAie_SetupErrorNetwork(XAie_DevInst *DevInst);

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is API returns the location next NoC tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
*
* @note		Internal only.
*
******************************************************************************/
__FORCE_INLINE__
static inline XAie_LocType XAie_LPartGetNextNocTile(XAie_DevInst *DevInst,
		XAie_LocType Loc)
{
	if((UINT8_MAX-Loc.Col) > DevInst->StartCol){
		XAIE_ERROR("Colum is out of range \n");
		return XAie_TileLoc(255,255);
	}
	XAie_LocType lLoc = XAie_TileLoc((Loc.Col + DevInst->StartCol),
			Loc.Row);

	UPDT_NEXT_NOC_TILE_LOC(lLoc);
	return lLoc;
}

#endif /* XAIE_FEATURE_LITE */

#endif /* XAIE_LITE_H */

/** @} */
