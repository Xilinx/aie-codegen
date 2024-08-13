/******************************************************************************
* Copyright (C) 2023 - 2024 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite_aie4.h
* @{
*
* This header file defines a lightweight version of AIE4 specific register
* operations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Ramakant   27/12/2023  Initial creation
* </pre>
*
******************************************************************************/

#ifndef XAIE_LITE_AIE4_H_
#define XAIE_LITE_AIE4_H_

/***************************** Include Files *********************************/
#include "xaie_lite_hwcfg.h"
#include "xaie_lite_io.h"
#include "xaie_lite_npi.h"
#include "xaie_lite_regdef_aie4.h"
#include "xaiegbl_defs.h"
#include "xaiegbl.h"
#include "xaie_lite_util.h"

/************************** Constant Definitions *****************************/
#define XAIE_PL_MOD_UC_MEMORY_IDX		4U
#define XAIE_PL_MOD_UC_MEMORY_COUNT 	2U

#define XAIE_CORE_MOD_DUALAPP_A         0X1
#define XAIE_CORE_MOD_DUALAPP_A_TOP     0X2
#define XAIE_CORE_MOD_DUALAPP_B_BOTTOM  0X3
#define XAIE_CORE_MOD_DUALAPP_B         0X4

#define XAIE_PL_MOD_AXIMM_APP_A_PENDING_TRANSACTIONS_MASK	0x00000007
#define XAIE_PL_MOD_AXIMM_APP_B_PENDING_TRANSACTIONS_MASK	0x00000038

/* Set the timeout to maximum zeroization cycles required for Memtile DM zeroization for Sim backend.
   If polling timeout is less driver will return an error before zeroization is complete */
#ifdef __AIESIM__
	#define XAIE4_MEMZERO_POLL_TIMEOUT		100000
#else
	#define XAIE4_MEMZERO_POLL_TIMEOUT		1000
#endif

#define XAIE4_PENDING_AXIMM_TRANSACTION_POLL_TIMEOUT	1000
/************************** Function Prototypes  *****************************/
#if defined(XAIE_FEATURE_LITE_UTIL)
/*****************************************************************************/
/**
*
* This API returns the Aie Tile Core status for a particular column and row.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Col: Column number.
* @param	Row: Row number.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
__FORCE_INLINE__
static inline void _XAie_LCoreStatus(XAie_DevInst *DevInst, XAie_Col_Status *Status, u32 Col, u32 Row)
{
	u64 RegAddr;

	/* core status addr */
	RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col)
		+ XAIE_AIE_TILE_CORE_STATUS_REGOFF;

	/* core status */
	Status[Col].CoreTile[Row].CoreStatus =
		(_XAie_LPartRead32(DevInst, RegAddr)
		 & XAIE_AIE_TILE_CORE_STATUS_MASK);

	/* core program counter addr */
	RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col)
		+ XAIE_AIE_TILE_CORE_PC_REGOFF;

	/* program counter */
	Status[Col].CoreTile[Row].ProgramCounter =
		(_XAie_LPartRead32(DevInst, RegAddr)
		 & XAIE_AIE_TILE_CORE_PC_MASK);

	/* core stack pointer addr */
	RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col)
		+ XAIE_AIE_TILE_CORE_SP_REGOFF;

	/* stack pointer */
	Status[Col].CoreTile[Row].StackPtr =
		(_XAie_LPartRead32(DevInst, RegAddr)
		 & XAIE_AIE_TILE_CORE_SP_MASK);

	/* core link addr */
	RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col)
		+ XAIE_AIE_TILE_CORE_LR_REGOFF;

	/* link register */
	Status[Col].CoreTile[Row].LinkReg =
		(_XAie_LPartRead32(DevInst, RegAddr)
		 & XAIE_AIE_TILE_CORE_LR_MASK);
}

/*****************************************************************************/
/**
*
* This API returns the Aie Tile DMA status for a particular column and row.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Col: Column number.
* @param	Row: Row number.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
__FORCE_INLINE__
static inline void _XAie_LCoreDMAStatus(XAie_DevInst *DevInst, XAie_Col_Status *Status, u32 Col, u32 Row)
{
	u64 RegAddr;

	/* iterate all tile dma channels */
	for (u32 Chan = 0; Chan < XAIE_TILE_DMA_NUM_CH; Chan++) {

		/* s2mm channel address */
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col)
			+ Chan * XAIE_TILE_DMA_S2MM_CHANNEL_STATUS_IDX + XAIE_TILE_DMA_S2MM_CHANNEL_STATUS_REGOFF;

		/* read s2mm channel status */
		Status[Col].CoreTile[Row].dma[Chan].S2MMStatus =
			(_XAie_LPartRead32(DevInst, RegAddr) & XAIE_TILE_DMA_S2MM_CHANNEL_VALID_BITS_MASK);

		/* mm2s channel address */
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col)
			+ Chan * XAIE_TILE_DMA_MM2S_CHANNEL_STATUS_IDX + XAIE_TILE_DMA_MM2S_CHANNEL_STATUS_REGOFF;

		/* read mm2s channel status */
		Status[Col].CoreTile[Row].dma[Chan].MM2SStatus =
			(_XAie_LPartRead32(DevInst, RegAddr) & XAIE_TILE_DMA_MM2S_CHANNEL_VALID_BITS_MASK);
	}

}

/*****************************************************************************/
/**
*
* This API returns the Aie Lock status for a particular column and row.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Col: Column number.
* @param    Row: Row number.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
__FORCE_INLINE__
static inline void _XAie_LCoreLockValue(XAie_DevInst *DevInst, XAie_Col_Status *Status, u32 Col, u32 Row)
{
	u64 RegAddr;

	/* iterate all lock value registers */
	for(u32 Lock = 0; Lock < XAIE_TILE_NUM_LOCKS; Lock++) {

		/* lock value address */
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col)
			+ Lock * XAIE_AIE_TILE_LOCK_VALUE_IDX + XAIE_AIE_TILE_LOCK_VALUE_REGOFF;

		/* read lock value */
		Status[Col].CoreTile[Row].LockValue[Lock] =
			(u8)(_XAie_LPartRead32(DevInst, RegAddr)
			& XAIE_AIE_TILE_LOCK_VALUE_MASK);
	}
}

/*****************************************************************************/
/**
*
* This API returns the Core Tile Event Status for a particular column and row.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Col: Column number.
* @param    Row: Row number.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
__FORCE_INLINE__
static inline void _XAie_LCoreEventStatus(XAie_DevInst *DevInst, XAie_Col_Status *Status, u32 Col, u32 Row)
{
	u64 RegAddr;

	/* iterate all event status registers */
	for(u32 EventReg = 0; EventReg < XAIE_CORE_TILE_NUM_EVENT_STATUS_REGS; EventReg++) {

		/* event status address */
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col)
			+ EventReg * XAIE_AIE_TILE_CORE_MOD_EVENT_STATUS_IDX + XAIE_AIE_TILE_CORE_MOD_EVENT_STATUS_REGOFF;

		/* read event status register and store in output buffer */
		Status[Col].CoreTile[Row].EventCoreModStatus[EventReg] =
			(u32)(_XAie_LPartRead32(DevInst, RegAddr) & XAIE_AIE_TILE_CORE_MOD_EVENT_STATUS_MASK);
	}
}

/*****************************************************************************/
/**
*
* This API returns the Mem Tile DMA status for a particular column and row.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Col: Column number.
* @param	Row: Row number.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
__FORCE_INLINE__
static inline void _XAie_LMemDMAStatus(XAie_DevInst *DevInst, XAie_Col_Status *Status, u32 Col, u32 Row)
{
	u64 RegAddr;

	/* mem tile dma status */
	for(u32 Chan = 0; Chan < XAIE_MEM_TILE_DMA_NUM_CH; Chan++) {

		/* s2mm channel address */
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_MEM_TILE_ROW_START, Col)
			+ Chan * XAIE_MEM_TILE_DMA_S2MM_CHANNEL_STATUS_IDX + XAIE_MEM_TILE_DMA_S2MM_CHANNEL_STATUS_REGOFF;

		/* read s2mm channel status */
		Status[Col].MemTile[Row].dma[Chan].S2MMStatus =
			(_XAie_LPartRead32(DevInst, RegAddr) & XAIE_MEM_TILE_DMA_S2MM_CHANNEL_VALID_BITS_MASK);

		/* mm2s channel address */
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_MEM_TILE_ROW_START, Col)
			+ Chan * XAIE_MEM_TILE_DMA_MM2S_CHANNEL_STATUS_IDX + XAIE_MEM_TILE_DMA_MM2S_CHANNEL_STATUS_REGOFF;

		/* read s2mm channel status */
		Status[Col].MemTile[Row].dma[Chan].MM2SStatus =
			(_XAie_LPartRead32(DevInst, RegAddr) & XAIE_MEM_TILE_DMA_MM2S_CHANNEL_VALID_BITS_MASK);
	}
}

/*****************************************************************************/
/**
*
* This API returns the Mem Lock status for a particular column and row.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Col: Column number.
* @param    Row: Row number.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
__FORCE_INLINE__
static inline void _XAie_LMemLockValue(XAie_DevInst *DevInst, XAie_Col_Status *Status, u32 Col, u32 Row)
{
	u64 RegAddr;

	/* iterate all lock value registers */
	for(u32 Lock = 0; Lock < XAIE_MEM_TILE_NUM_LOCKS; Lock++) {

		/* lock value address */
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_MEM_TILE_ROW_START, Col)
			+ Lock * XAIE_MEM_TILE_LOCK_VALUE_IDX + XAIE_MEM_TILE_LOCK_VALUE_REGOFF;

		/* read lock value */
		Status[Col].MemTile[Row].LockValue[Lock] =
			(u8)(_XAie_LPartRead32(DevInst, RegAddr)
			& XAIE_MEM_TILE_LOCK_VALUE_MASK);
	}
}

/*****************************************************************************/
/**
*
* This API returns the Mem Event status for a particular column and row.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Col: Column number.
* @param    Row: Row number.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
__FORCE_INLINE__
static inline void _XAie_LMemEventStatus(XAie_DevInst *DevInst, XAie_Col_Status *Status, u32 Col, u32 Row)
{
	u64 RegAddr;

	/* iterate all Event Status registers */
	for(u32 EventReg = 0; EventReg < XAIE_MEM_TILE_NUM_EVENT_STATUS_REGS; EventReg++)
	{
		/* Event Status address */
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_MEM_TILE_ROW_START, Col)
			+ EventReg * XAIE_MEM_TILE_EVENT_STATUS_IDX + XAIE_MEM_TILE_EVENT_STATUS_REGOFF;

		/* read Event Status register */
		Status[Col].MemTile[Row].EventStatus[EventReg] =
			(u32)(_XAie_LPartRead32(DevInst, RegAddr) & XAIE_MEM_TILE_EVENT_STATUS_MASK);
	}
}

/*****************************************************************************/
/**
*
* This API calls the Mem Tile and Core Tile sub-modules.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Col: Column number.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
__FORCE_INLINE__
static inline void _XAie_LTileStatus(XAie_DevInst *DevInst, XAie_Col_Status *Status, u32 Col)
{
	/* iterate all mem tile rows */
	for(u32 Row = 0; Row < XAIE_MEM_TILE_NUM_ROWS; Row++) {
		_XAie_LMemDMAStatus(DevInst, Status, Col, Row);
		_XAie_LMemLockValue(DevInst, Status, Col, Row);
		_XAie_LMemEventStatus(DevInst, Status, Col, Row);
	}

	/* iterate all aie tile rows */
	for(u32 Row = 0; Row < XAIE_AIE_TILE_NUM_ROWS; Row++) {
		_XAie_LCoreStatus(DevInst, Status, Col, Row);
		_XAie_LCoreDMAStatus(DevInst, Status, Col, Row);
		_XAie_LCoreLockValue(DevInst, Status, Col, Row);
		_XAie_LCoreEventStatus(DevInst, Status, Col, Row);
	}
}

/*****************************************************************************/
/**
*
* This API returns the Shim Lock status for a particular column.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Col: Column number.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
__FORCE_INLINE__
static inline void _XAie_LShimLockValue(XAie_DevInst *DevInst, XAie_Col_Status *Status, u32 Col)
{
	u64 RegAddr;

	/* iterate all lock value registers for shim tile*/
	for(u32 Lock = 0; Lock < XAIE_SHIM_NUM_LOCKS; Lock++) {

		/* lock value address */
		RegAddr = _XAie_LGetTileAddr(XAIE_SHIM_ROW, Col)
			+ Lock * XAIE_SHIM_TILE_LOCK_VALUE_IDX + XAIE_SHIM_TILE_LOCK_VALUE_REGOFF;

		/* read lock value */
		Status[Col].ShimTile[XAIE_SHIM_ROW].LockValue[Lock] =
			(u8)(_XAie_LPartRead32(DevInst, RegAddr)
			& XAIE_SHIM_TILE_LOCK_VALUE_MASK);
	}
}

/*****************************************************************************/
/**
*
* This API returns the Shim DMA status for a particular column.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Col: Column number.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
__FORCE_INLINE__
static inline void _XAie_LShimDMAStatus(XAie_DevInst *DevInst, XAie_Col_Status *Status, u32 Col)
{
	u64 RegAddr;

	/* shim dma status - fixed at row XAIE_SHIM_ROW */
	for(u32 Chan = 0; Chan < XAIE_SHIM_DMA_NUM_CH; Chan++) {
		RegAddr = _XAie_LGetTileAddr(XAIE_SHIM_ROW, Col) + Chan * XAIE_SHIM_DMA_S2MM_CHANNEL_STATUS_IDX +
			XAIE_SHIM_DMA_S2MM_CHANNEL_STATUS_REGOFF;

		/* read s2mm channel status */
		Status[Col].ShimTile[XAIE_SHIM_ROW].dma[Chan].S2MMStatus =
			(_XAie_LPartRead32(DevInst, RegAddr) & XAIE_SHIM_DMA_S2MM_CHANNEL_VALID_BITS_MASK);

		/* mm2s channel address */
		RegAddr = _XAie_LGetTileAddr(XAIE_SHIM_ROW, Col) + Chan * XAIE_SHIM_DMA_MM2S_CHANNEL_STATUS_IDX +
			XAIE_SHIM_DMA_MM2S_CHANNEL_STATUS_REGOFF;

		/* read mm2s channel status */
		Status[Col].ShimTile[XAIE_SHIM_ROW].dma[Chan].MM2SStatus =
			(_XAie_LPartRead32(DevInst, RegAddr) & XAIE_SHIM_DMA_MM2S_CHANNEL_VALID_BITS_MASK);
	}
}

/*****************************************************************************/
/**
*
* This API returns the Shim Event status for a particular column.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Col: Column number.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
__FORCE_INLINE__
static inline void _XAie_LShimEventStatus(XAie_DevInst *DevInst, XAie_Col_Status *Status, u32 Col)
{
	u64 RegAddr;

	/* iterate all event status registers for shim tile */
	for (u32 EventReg = 0; EventReg < XAIE_SHIM_TILE_NUM_EVENT_STATUS_REGS; EventReg++) {

		/* event status register address */
		RegAddr = _XAie_LGetTileAddr(XAIE_SHIM_ROW, Col+DevInst->StartCol)
			+ EventReg * XAIE_SHIM_TILE_EVENT_STATUS_IDX + XAIE_SHIM_TILE_EVENT_STATUS_REGOFF;

		/* read event status value */
		Status[Col].ShimTile[XAIE_SHIM_ROW].EventStatus[EventReg] =
			(u32)(_XAie_LPartRead32(DevInst, RegAddr) & XAIE_SHIM_TILE_EVENT_STATUS_MASK);
	}
}


/*****************************************************************************/
/**
*
* This API returns the Shim Tile status for a particular column. It calls
* further sub-apis.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Col: Column number.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
__FORCE_INLINE__
static inline void _XAie_LShimTileStatus(XAie_DevInst *DevInst, XAie_Col_Status *Status, u32 Col)
{
    _XAie_LShimDMAStatus(DevInst, Status, Col);
    _XAie_LShimLockValue(DevInst, Status, Col);
    _XAie_LShimEventStatus(DevInst, Status, Col);
}

/*****************************************************************************/
/**
*
* This API returns the column status for N number of columns.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
__FORCE_INLINE__
static inline void XAie_LGetColRangeStatus(XAie_DevInst *DevInst, XAie_Col_Status *Status)
{
	u32 StartCol = (u32)(DevInst->StartCol);
	u32 NumCols  = (u32)(DevInst->NumCols);

	/* iterate specified columns */
	for(u32 Col = 0; Col < NumCols; Col++) {
		_XAie_LShimTileStatus(DevInst, Status, Col);
		_XAie_LTileStatus(DevInst, Status, Col);
	}
}

#endif      /* end of #if defined(XAIE_FEATURE_LITE_UTIL) */

/*****************************************************************************/
/**
 *
 * This API Clears Core register as a part of Clear context.
 *
 * @param       DevInst: Device Instance
 *
 * @return      None.
 *
 * @note        None.
 *
 *****************************************************************************/
static inline void _XAie_ClearCoreReg(XAie_DevInst *DevInst)
{
	(void)DevInst;
}

/*****************************************************************************/
/**
*
* This API sets Single App or Dual App mode.
*
* @param	DevInst: Device Instance
* @param	XAie_PartInitOpts: Partition init options
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
static inline AieRC _XAie_LSetDualAppModePrivileged(XAie_DevInst *DevInst, XAie_PartInitOpts *Opts)
{
	u64 RegAddr;
	for(u32 i = 0 ; i < Opts->NumUseTiles; i++) {
		XAie_LocType Loc = XAie_TileLoc(Opts->Locs[i].Col, Opts->Locs[i].Row);

		if(Loc.Row >= XAIE_NUM_ROWS){
			XAIE_ERROR("Row number is greater then Total number of Rows\n");
			return XAIE_INVALID_ARGS;
		}

		if (Loc.Row == XAIE_SHIM_ROW) {
				RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + XAIE_PL_MOD_DUAL_APP_MODE;
				_XAie_LPartWrite32(DevInst, RegAddr, XAIE_ENABLE);

		} else if (Loc.Row >= XAIE_MEM_TILE_ROW_START && Loc.Row < XAIE_AIE_TILE_ROW_START ) {
				RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + XAIE_MEM_TILE_DUAL_APP_MODE;
				_XAie_LPartWrite32(DevInst, RegAddr, XAIE_ENABLE);

		} else if (Loc.Row >= XAIE_AIE_TILE_ROW_START) {
			/* APP A requested tiles should not be >= to total no of AIE Tiles*/
			u32 AieTiles = Opts->NumUseTiles - (XAIE_SHIM_NUM_ROWS + XAIE_MEM_TILE_NUM_ROWS);

			if(AieTiles >= XAIE_AIE_TILE_NUM_ROWS) {
				XAIE_ERROR("In Dual App Mode at-least 1 compute tile should be alloted to App B\n");
				return XAIE_INVALID_RANGE;
			}

			/* Configure App A Top compute Tile */
			if(Loc.Row == ((AieTiles  + XAIE_AIE_TILE_ROW_START) - 1)) {
				RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + XAIE_CORE_MOD_DUAL_APP_MODE;
				_XAie_LPartWrite32(DevInst, RegAddr, XAIE_CORE_MOD_DUALAPP_A_TOP);
			}
			else if(Loc.Row < ((AieTiles  + XAIE_AIE_TILE_ROW_START) - 1) &&
										Loc.Row >= XAIE_AIE_TILE_ROW_START) {
				/* Configure App A compute Tiles */
				RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + XAIE_CORE_MOD_DUAL_APP_MODE;
				_XAie_LPartWrite32(DevInst, RegAddr, XAIE_CORE_MOD_DUALAPP_A);
			}
			/* Configure App B compute Tiles */
			else if(Loc.Row == ((AieTiles  + XAIE_AIE_TILE_ROW_START))) {
				RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + XAIE_CORE_MOD_DUAL_APP_MODE;
				_XAie_LPartWrite32(DevInst, RegAddr, XAIE_CORE_MOD_DUALAPP_B_BOTTOM);
			}
			/* Configure App B Bottom compute Tile */
			else {
				RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + XAIE_CORE_MOD_DUAL_APP_MODE;
				_XAie_LPartWrite32(DevInst, RegAddr, XAIE_CORE_MOD_DUALAPP_B);
			}
		}
	}
	return XAIE_OK;
}
/*****************************************************************************/
/**
*
* This API check if an AI engine array tile is in use.
*
* @param	DevInst: Device Instance.
* @param	Loc: Tile location.
*
* @return	XAIE_ENABLE if a tile is in use, otherwise XAIE_DISABLE.
*
* @note		Internal only.
*
******************************************************************************/
static inline u8 _XAie_LPmIsArrayTileRequested(XAie_DevInst *DevInst,
			XAie_LocType Loc)
{
	(void) DevInst;
	(void) Loc.Col;
	(void) Loc.Row;

	return XAIE_ENABLE;
}

/*****************************************************************************/
/**
*
* This API pauses DMA in the AI engine partition
*
* @param	DevInst: Device Instance
* @param	Loc: SHIM tile location
*
*
* @note		This function is internal.
*
******************************************************************************/

static inline void _XAie_LSetPartDmaPause(XAie_DevInst *DevInst,
		XAie_LocType Loc)
{
	u64 RegAddr, RegAddr_uC_A, RegAddr_uC_B;
	u32 FldVal, FldVal_uC_A, FldVal_uC_B;

	RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + XAIE_PL_MOD_DMA_PAUSE_REGOFF;
	RegAddr_uC_A = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + XAIE_PL_MOD_UC_DMA_A_PAUSE_REGOFF;
	RegAddr_uC_B = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + XAIE_PL_MOD_UC_DMA_B_PAUSE_REGOFF;
	FldVal_uC_A = XAie_SetField(XAIE_PL_MOD_UC_DMA_A_PAUSE_MASK, XAIE_PL_MOD_UC_DMA_A_PAUSE_LSB, XAIE_PL_MOD_UC_DMA_A_PAUSE_MASK);
	FldVal_uC_B = XAie_SetField(XAIE_PL_MOD_UC_DMA_B_PAUSE_MASK, XAIE_PL_MOD_UC_DMA_B_PAUSE_LSB, XAIE_PL_MOD_UC_DMA_B_PAUSE_MASK);

	if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_A) {
		/* Application A DMA Pause */
		FldVal = XAie_SetField(XAIE_PL_MOD_DMA_PAUSE_APP_A_MASK, XAIE_PL_MOD_DMA_PAUSE_LSB, XAIE_PL_MOD_DMA_PAUSE_APP_A_MASK);
		/* Pause uC A DMA */
		_XAie_LPartWrite32(DevInst, RegAddr_uC_A, FldVal_uC_A);
	} else if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B){
		/* Application B DMA Pause */
		FldVal = XAie_SetField(XAIE_PL_MOD_DMA_PAUSE_APP_B_MASK, XAIE_PL_MOD_DMA_PAUSE_LSB, XAIE_PL_MOD_DMA_PAUSE_APP_B_MASK);
		/* Pause uC B DMA */
		_XAie_LPartWrite32(DevInst, RegAddr_uC_B, FldVal_uC_B);
	} else if(DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE){
		/* Single Application mode DMA Pause */
		FldVal = XAie_SetField(XAIE_PL_MOD_DMA_PAUSE_MASK, XAIE_PL_MOD_DMA_PAUSE_LSB, XAIE_PL_MOD_DMA_PAUSE_MASK);

		/* Pause uC A DMA */
		FldVal_uC_A = XAie_SetField(XAIE_PL_MOD_UC_DMA_A_PAUSE_MASK, XAIE_PL_MOD_UC_DMA_A_PAUSE_LSB, XAIE_PL_MOD_UC_DMA_A_PAUSE_MASK);
		_XAie_LPartWrite32(DevInst, RegAddr_uC_A, FldVal_uC_A);

		/* Pause uC B DMA */
		FldVal_uC_B = XAie_SetField(XAIE_PL_MOD_UC_DMA_B_PAUSE_MASK, XAIE_PL_MOD_UC_DMA_B_PAUSE_LSB, XAIE_PL_MOD_UC_DMA_B_PAUSE_MASK);
		_XAie_LPartWrite32(DevInst, RegAddr_uC_B, FldVal_uC_B);
	}
	_XAie_LPartWrite32(DevInst, RegAddr, FldVal);
}

/*****************************************************************************/
/**
*
* This API polls for pending AXI-MM transactions in the AI engine partition
*
* @param	DevInst: Device Instance
* @param	Loc: SHIM tile location
*
* @note		This function is internal.
*
*
******************************************************************************/
static inline void _XAie_LPollAximmTransactions(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u64 RegAddr;

	RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + XAIE_NOC_MOD_AXI_MM_OUTSTANDING_TRANSACTIONS_REGOFF;
	if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_A) {
		/* Application A Pending AXI-MM Transaction polling */
		_XAie_LPartPoll32(DevInst, RegAddr,
				XAIE_PL_MOD_AXIMM_APP_A_PENDING_TRANSACTIONS_MASK, 0, XAIE4_PENDING_AXIMM_TRANSACTION_POLL_TIMEOUT);
	} else if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B){
		/* Application B Pending AXI-MM Transaction polling */
		_XAie_LPartPoll32(DevInst, RegAddr,
				XAIE_PL_MOD_AXIMM_APP_B_PENDING_TRANSACTIONS_MASK, 0, XAIE4_PENDING_AXIMM_TRANSACTION_POLL_TIMEOUT);
	} else if(DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE){
		/* Single Application Pending AXI-MM Transaction polling */
		_XAie_LPartPoll32(DevInst, RegAddr,
				XAIE_NOC_MOD_AXI_MM_OUTSTANDING_TRANSACTIONS_MASK, 0, XAIE4_PENDING_AXIMM_TRANSACTION_POLL_TIMEOUT);
	}
}

/*****************************************************************************/
/**
*
* This API set SHIM reset in the AI engine partition
*
* @param	DevInst: Device Instance
* @param	Loc: SHIM tile location
* @param	Reset: XAIE_ENABLE to enable reset,
* 			XAIE_DISABLE to disable reset
*
* @note		This function is internal.
*
*
******************************************************************************/
static inline void _XAie_LSetPartColShimReset(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Reset)
{
	u64 RegAddr;
	u32 FldVal;

	/* Pause DMA */
	_XAie_LSetPartDmaPause(DevInst, Loc);
	/* Poll pending AXI-MM transactions before Application reset */
	_XAie_LPollAximmTransactions(DevInst, Loc);

	RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + XAIE_PL_MOD_COL_RST_REGOFF;
	if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_A) {
		/* Application A reset */
		FldVal = XAie_SetField(Reset, XAIE_PL_MOD_COL_RST_APP_A_LSB, XAIE_PL_MOD_COL_RST_APP_A_MASK);
	} else if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B){
		/* Application B reset */
		FldVal = XAie_SetField(Reset, XAIE_PL_MOD_COL_RST_APP_B_LSB, XAIE_PL_MOD_COL_RST_APP_B_MASK);
	} else if(DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE){
		/* Reset both Application A and B */
		FldVal = XAie_SetField(Reset, (XAIE_PL_MOD_COL_RST_APP_A_LSB | XAIE_PL_MOD_COL_RST_APP_B_LSB),
			(XAIE_PL_MOD_COL_RST_APP_A_MASK | XAIE_PL_MOD_COL_RST_APP_B_MASK));
	}
	_XAie_LPartWrite32(DevInst, RegAddr, FldVal);
}

/*****************************************************************************/
/**
*
* This API sets isolation boundry of an AI engine partition after reset
*
* @param	DevInst: Device Instance
*
* @note		Internal API only.
*
******************************************************************************/
static inline void _XAie_LSetPartIsolationAfterRst(XAie_DevInst *DevInst)
{
	for(u8 C = 0; C < DevInst->NumCols; C++) {
		u64 RegAddr;
		u32 RegVal = 0;

		if (DevInst->AppMode != XAIE_DEVICE_SINGLE_APP_MODE) {
			RegVal |= XAIE_TILE_CNTR_ISOLATE_WEST_MASK | XAIE_TILE_CNTR_ISOLATE_EAST_MASK ;
		} else {
			if(C == 0) {
				RegVal |= XAIE_TILE_CNTR_ISOLATE_WEST_MASK;
			}
			if(C == (u8)(DevInst->NumCols - 1)) {
				RegVal |= XAIE_TILE_CNTR_ISOLATE_EAST_MASK;
			}
		}

		/* Isolate boundrary of SHIM tiles */
		RegAddr = _XAie_LGetTileAddr(0, C) +
			XAIE_PL_MOD_TILE_CNTR_REGOFF;
		_XAie_LPartWrite32(DevInst, RegAddr, RegVal);
		if (DevInst->AppMode != XAIE_DEVICE_SINGLE_APP_MODE) {
			/* Isolate East-West AXI-MM Transactions in Dual App Mode */
			RegAddr = _XAie_LGetTileAddr(0, C) +
					XAIE_PL_MOD_TILE_CNTR_AXIMM_REGOFF;
			_XAie_LPartWrite32(DevInst, RegAddr, RegVal);
		}

		/* Isolate boundrary of MEM tiles */
		for (u8 R = XAIE_MEM_TILE_ROW_START;
			R < XAIE_AIE_TILE_ROW_START; R++) {
			RegAddr = _XAie_LGetTileAddr(R, C) +
				XAIE_MEM_TILE_MOD_TILE_CNTR_REGOFF;
			_XAie_LPartWrite32(DevInst, RegAddr, RegVal);
		}

		/* Isolate boundrary of CORE tiles */
		for (u8 R = XAIE_AIE_TILE_ROW_START;
			R < XAIE_NUM_ROWS; R++) {
			RegAddr = _XAie_LGetTileAddr(R, C) +
				XAIE_CORE_MOD_TILE_CNTR_REGOFF;
			_XAie_LPartWrite32(DevInst, RegAddr, RegVal);
		}
	}
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
* @note		Internal API only.
*
******************************************************************************/
static inline void  _XAie_LPartMemZeroInit(XAie_DevInst *DevInst)
{
	u64 RegAddr;

	for(u8 C = 0; C < DevInst->NumCols; C++) {
		for (u8 R = XAIE_MEM_TILE_ROW_START;
			R < XAIE_AIE_TILE_ROW_START; R++) {
			/* Zeroize App A Memtile DM */
				RegAddr = _XAie_LGetTileAddr(R, C) +
							XAIE_MEM_TILE_MOD_MEM_CNTR_REGOFF;
				_XAie_LPartMaskWrite32(DevInst, RegAddr,
							XAIE_MEM_TILE_MEM_CNTR_ZEROISATION_MASK,
							XAIE_MEM_TILE_MEM_CNTR_ZEROISATION_MASK);
				/* Zeroize App B Memtile DM */
				RegAddr = _XAie_LGetTileAddr(R, C) +
						XAIE_MEM_TILE_MOD_MEM_CNTR_REGOFF_B;
				_XAie_LPartMaskWrite32(DevInst, RegAddr,
							XAIE_MEM_TILE_MEM_CNTR_ZEROISATION_MASK,
							XAIE_MEM_TILE_MEM_CNTR_ZEROISATION_MASK);
		}

		/* Isolate boundrary of CORE tiles */
		for (u8 R = XAIE_AIE_TILE_ROW_START;
			R < XAIE_NUM_ROWS; R++) {
			RegAddr = _XAie_LGetTileAddr(R, C) +
				XAIE_CORE_MOD_MEM_CNTR_REGOFF;
			_XAie_LPartMaskWrite32(DevInst, RegAddr,
				XAIE_CORE_MOD_MEM_CNTR_ZEROISATION_MASK,
				XAIE_CORE_MOD_MEM_CNTR_ZEROISATION_MASK);
			RegAddr = _XAie_LGetTileAddr(R, C) +
				XAIE_MEM_MOD_MEM_CNTR_REGOFF;
			_XAie_LPartMaskWrite32(DevInst, RegAddr,
				XAIE_MEM_MOD_MEM_CNTR_ZEROISATION_MASK,
				XAIE_MEM_MOD_MEM_CNTR_ZEROISATION_MASK);
		}
	}

	/* Poll last mem module and last mem tile mem module */
	RegAddr = _XAie_LGetTileAddr(XAIE_NUM_ROWS - 1,
			DevInst->NumCols - 1) +
			XAIE_MEM_MOD_MEM_CNTR_REGOFF;
	_XAie_LPartPoll32(DevInst, RegAddr,
			XAIE_MEM_MOD_MEM_CNTR_ZEROISATION_MASK, 0, 800);

	RegAddr = _XAie_LGetTileAddr(XAIE_AIE_TILE_ROW_START - 1,
			DevInst->NumCols - 1) +
			XAIE_MEM_TILE_MOD_MEM_CNTR_REGOFF;
	_XAie_LPartPoll32(DevInst, RegAddr,
			XAIE_MEM_TILE_MEM_CNTR_ZEROISATION_MASK, 0, XAIE4_MEMZERO_POLL_TIMEOUT);
}

/*****************************************************************************/
/**
*
* This API initialize the memories of the uC module to zero.
*
* @param	DevInst: Device Instance
*
* @return       XAIE_OK on success, error code on failure
*
* @note		Internal API only.
*
******************************************************************************/
static inline void _XAie_LZeroInitUcMemory(XAie_DevInst *DevInst)
{
	u64 RegAddr;
	for(u8 C = 0; C < DevInst->NumCols; C++) {
		for(u8 U = 0; U < XAIE_PL_MOD_UC_MEMORY_COUNT; U++) {
			RegAddr = _XAie_LGetTileAddr(0, C) + ((U * XAIE_PL_MOD_UC_MEMORY_IDX) + XAIE_PL_MODULE_MEMORY_ZEROIZATION);
			_XAie_LPartMaskWrite32(DevInst, RegAddr,
					XAIE_PL_MODULE_MEMORY_ZEROIZATION_MASK,
					XAIE_PL_MODULE_MEMORY_ZEROIZATION_MASK);
		}
	}

	/* Read last uC to make sure all the writes went though and
	* Zeroization is complete.
	*/
	RegAddr = _XAie_LGetTileAddr(0, DevInst->NumCols - 1) + XAIE_PL_MODULE_MEMORY_ZEROIZATION_B;
	_XAie_LPartPoll32(DevInst, RegAddr, XAIE_PL_MODULE_MEMORY_ZEROIZATION_MASK, 0, 1000);
}
/*****************************************************************************/
/**
*
* This API checks if all the Tile DMA and Mem Tile DMA channels in a partition
* are idle.
*
* @param	DevInst: Device Instance
*
* @return       XAIE_OK if all channels are idle, XAIE_ERR otherwise.
*
* @note		Internal API only. Checks for AIE Tile DMAs and Mem Tile DMAs
*
******************************************************************************/
static inline AieRC _XAie_LPartIsDmaIdle(XAie_DevInst *DevInst)
{
	for (u8 C = 0U; C < DevInst->NumCols; C++) {
		u64 RegAddr;
		u32 RegVal;

		/* AIE TILE DMAs */
		for(u8 R = XAIE_AIE_TILE_ROW_START; R < XAIE_NUM_ROWS; R++) {
			for (u32 Ch = 0; Ch < XAIE_TILE_DMA_NUM_CH; Ch++) {
				/* S2MM Channel */
				RegAddr = _XAie_LGetTileAddr(R, C) + Ch * 4 +
					XAIE_TILE_DMA_S2MM_CHANNEL_STATUS_REGOFF;
				RegVal = _XAie_LPartRead32(DevInst, RegAddr);
				if(RegVal &
				   (XAIE_TILE_DMA_S2MM_CHANNEL_STATUS_MASK |
				    XAIE_TILE_DMA_S2MM_CHANNEL_RUNNING_MASK))
					return XAIE_ERR;

				/* MM2S Channel */
				RegAddr = _XAie_LGetTileAddr(R, C) + Ch * 4 +
					XAIE_TILE_DMA_MM2S_CHANNEL_STATUS_REGOFF;
				RegVal = _XAie_LPartRead32(DevInst, RegAddr);
				if(RegVal &
				   (XAIE_TILE_DMA_MM2S_CHANNEL_STATUS_MASK |
				    XAIE_TILE_DMA_MM2S_CHANNEL_RUNNING_MASK))
					return XAIE_ERR;
			}

		}

		/* MEM TILE DMAs */
		for(u8 R = XAIE_MEM_TILE_ROW_START; R < XAIE_AIE_TILE_ROW_START;
				R++) {
			for(u32 Ch = 0; Ch < XAIE_MEM_TILE_DMA_NUM_CH; Ch++) {
				/* S2MM Channel */
				RegAddr = _XAie_LGetTileAddr(R, C) + Ch * 4 +
					XAIE_MEM_TILE_DMA_S2MM_CHANNEL_STATUS_REGOFF;
				RegVal = _XAie_LPartRead32(DevInst, RegAddr);
				if(RegVal &
				   (XAIE_MEM_TILE_DMA_S2MM_CHANNEL_STATUS_MASK |
				    XAIE_MEM_TILE_DMA_S2MM_CHANNEL_RUNNING_MASK))
					return XAIE_ERR;

				/* MM2S Channel */
				RegAddr = _XAie_LGetTileAddr(R, C) + Ch * 4 +
					XAIE_MEM_TILE_DMA_MM2S_CHANNEL_STATUS_REGOFF;
				RegVal = _XAie_LPartRead32(DevInst, RegAddr);
				if(RegVal &
				   (XAIE_MEM_TILE_DMA_MM2S_CHANNEL_STATUS_MASK |
				    XAIE_MEM_TILE_DMA_MM2S_CHANNEL_RUNNING_MASK))
					return XAIE_ERR;
			}
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API checks if all the DMA channels in a SHIM NOC tile are idle.
*
* @param	DevInst: Device Instance
* @param	Loc: ShimDma location
*
* @return       XAIE_OK if all channels are idle, XAIE_ERR otherwise.
*
* @note		Internal API only. Checks for AIE Tile DMAs and Mem Tile DMAs
*
******************************************************************************/
static inline AieRC _XAie_LIsShimDmaIdle(XAie_DevInst *DevInst,
		XAie_LocType Loc)
{
	u64 RegAddr;
	u32 RegVal;

	for(u32 Ch = 0; Ch < XAIE_SHIM_DMA_NUM_CH; Ch++) {
		/* S2MM Channel */
		RegAddr = _XAie_LGetTileAddr(0, Loc.Col) + Ch * 4 +
			XAIE_SHIM_DMA_S2MM_CHANNEL_STATUS_REGOFF;
		RegVal = _XAie_LPartRead32(DevInst, RegAddr);
		if(RegVal & XAIE_SHIM_DMA_S2MM_CHANNEL_STATUS_MASK)
			return XAIE_ERR;

		/* MM2S Channel */
		RegAddr = _XAie_LGetTileAddr(0, Loc.Col) + Ch * 4 +
			XAIE_SHIM_DMA_MM2S_CHANNEL_STATUS_REGOFF;
		RegVal = _XAie_LPartRead32(DevInst, RegAddr);
		if(RegVal & XAIE_SHIM_DMA_MM2S_CHANNEL_STATUS_MASK)
			return XAIE_ERR;
	}

	return XAIE_OK;
}

static inline int _XAie_LMemBarrier(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u64 RegAddr;
	u32 Value = 0xBEEF;
	int Ret;

	RegAddr = _XAie_LGetTileAddr(0, Loc.Col) + XAIE_PL_MODULE_SPARE_REG;
	_XAie_LPartWrite32(DevInst, RegAddr, Value);
	Ret = _XAie_LPartPoll32(DevInst, RegAddr, 0x1, Value, 800);
	if (Ret < 0)
		return Ret;

	_XAie_LPartWrite32(DevInst, RegAddr, 0U);
	return 0;
}

/*****************************************************************************/
/**
*
* This API zeroizes all the DMs of the Partition for Application
* context switch
*
* @param	DevInst: Device Instance
*
* @return   XAIE_OK if all channels are idle, XAIE_ERR otherwise.
*
* @note		Internal API only.
*
******************************************************************************/
static inline AieRC _XAie_LPartDataMemZeroInit(XAie_DevInst *DevInst)
{
	u64 RegAddr;
	int Ret;
	u32 AppModVal = 0;

	for(u8 C = 0; C < DevInst->NumCols; C++) {
		if(DevInst->L2PreserveMem == 0) {
			for (u8 R = XAIE_MEM_TILE_ROW_START;
					R < XAIE_AIE_TILE_ROW_START; R++) {
				if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_A ||
						DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE ) {
					RegAddr = _XAie_LGetTileAddr(R, C) +
						XAIE_MEM_TILE_MOD_MEM_CNTR_REGOFF;
				} else if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {
					RegAddr = _XAie_LGetTileAddr(R, C) +
						XAIE_MEM_TILE_MOD_MEM_CNTR_REGOFF_B;
				} else {
					XAIE_ERROR("Invalid App Mode\n");
					return XAIE_INVALID_APP_MODE;
				}
				_XAie_LPartMaskWrite32(DevInst, RegAddr,
						XAIE_MEM_TILE_MEM_CNTR_ZEROISATION_MASK,
						XAIE_MEM_TILE_MEM_CNTR_ZEROISATION_MASK);
			}
		}

		for (u8 R = XAIE_AIE_TILE_ROW_START;
						R < XAIE_NUM_ROWS; R++) {
			if (DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_A) {
				RegAddr = _XAie_LGetTileAddr(R, C) + XAIE_CORE_MOD_MEM_CNTR_REGOFF;
				AppModVal = _XAie_LPartRead32(DevInst, RegAddr);
				if ((AppModVal & XAIE_CORE_MOD_DUALAPP_A_TOP) || (AppModVal & XAIE_CORE_MOD_DUALAPP_A)) {
					_XAie_LPartMaskWrite32(DevInst, RegAddr,
								XAIE_CORE_MOD_MEM_CNTR_ZEROISATION_MASK,
							XAIE_CORE_MOD_MEM_CNTR_ZEROISATION_MASK);
					RegAddr = _XAie_LGetTileAddr(R, C) + XAIE_MEM_MOD_MEM_CNTR_REGOFF;
					_XAie_LPartMaskWrite32(DevInst, RegAddr,
							XAIE_MEM_MOD_MEM_CNTR_ZEROISATION_MASK,
							XAIE_MEM_MOD_MEM_CNTR_ZEROISATION_MASK);
				}
			} else if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B){
				RegAddr = _XAie_LGetTileAddr(R, C) + XAIE_CORE_MOD_DUAL_APP_MODE;
				AppModVal = _XAie_LPartRead32(DevInst, RegAddr);
				if ((AppModVal & XAIE_CORE_MOD_DUALAPP_B_BOTTOM) || (AppModVal & XAIE_CORE_MOD_DUALAPP_B)) {
					_XAie_LPartMaskWrite32(DevInst, RegAddr,
							XAIE_CORE_MOD_MEM_CNTR_ZEROISATION_MASK,
							XAIE_CORE_MOD_MEM_CNTR_ZEROISATION_MASK);
					RegAddr = _XAie_LGetTileAddr(R, C) + XAIE_MEM_MOD_MEM_CNTR_REGOFF;
					_XAie_LPartMaskWrite32(DevInst, RegAddr,
							XAIE_MEM_MOD_MEM_CNTR_ZEROISATION_MASK,
							XAIE_MEM_MOD_MEM_CNTR_ZEROISATION_MASK);
				}
			} else if(DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) {
				RegAddr = _XAie_LGetTileAddr(R, C) + XAIE_CORE_MOD_MEM_CNTR_REGOFF;
				_XAie_LPartMaskWrite32(DevInst, RegAddr,
							XAIE_CORE_MOD_MEM_CNTR_ZEROISATION_MASK,
							XAIE_CORE_MOD_MEM_CNTR_ZEROISATION_MASK);
				RegAddr = _XAie_LGetTileAddr(R, C) + XAIE_MEM_MOD_MEM_CNTR_REGOFF;
				_XAie_LPartMaskWrite32(DevInst, RegAddr,
							XAIE_MEM_MOD_MEM_CNTR_ZEROISATION_MASK,
							XAIE_MEM_MOD_MEM_CNTR_ZEROISATION_MASK);
			} else {
				XAIE_ERROR("Invalid App Mode\n");
				return XAIE_INVALID_APP_MODE;
			}
		}

		/* Zeroize uC Shared Memory */
		if (DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_A) {
			RegAddr = _XAie_LGetTileAddr(0, C) + XAIE_PL_MODULE_MEMORY_ZEROIZATION;
			_XAie_LPartMaskWrite32(DevInst, RegAddr,
					XAIE_PL_MODULE_MEMORY_A_DM_ZEROIZATION_MASK,
					XAIE_PL_MODULE_MEMORY_A_DM_ZEROIZATION_LSB);
		} else if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B){
			RegAddr = _XAie_LGetTileAddr(0, C) + XAIE_PL_MODULE_MEMORY_ZEROIZATION_B;
			_XAie_LPartMaskWrite32(DevInst, RegAddr,
					XAIE_PL_MODULE_MEMORY_B_DM_ZEROIZATION_MASK,
					XAIE_PL_MODULE_MEMORY_B_DM_ZEROIZATION_LSB);
		} else if(DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) {
			for(u8 U = 0; U < XAIE_PL_MOD_UC_MEMORY_COUNT; U++) {
				RegAddr = _XAie_LGetTileAddr(0, C) + ((U * XAIE_PL_MOD_UC_MEMORY_IDX)
							+ XAIE_PL_MODULE_MEMORY_ZEROIZATION);
				_XAie_LPartMaskWrite32(DevInst, RegAddr,
						XAIE_PL_MODULE_MEMORY_A_DM_ZEROIZATION_MASK,
						XAIE_PL_MODULE_MEMORY_A_DM_ZEROIZATION_LSB);
			}
		} else {
			XAIE_ERROR("Invalid App Mode\n");
			return XAIE_INVALID_APP_MODE;
		}
	}

	for(u8 C = 0; C < DevInst->NumCols; C++) {
		/* Ramakant : Accessing Spare Register causes hang in Simnow. Need to be debugged */
		/*Ret = _XAie_LMemBarrier(DevInst, XAie_TileLoc(C, 0));
		if (Ret < 0)
			return XAIE_ERR;
		*/
		/* Poll last mem module and last mem tile mem module */
		RegAddr = _XAie_LGetTileAddr(XAIE_NUM_ROWS - 1, C) +
				XAIE_MEM_MOD_MEM_CNTR_REGOFF;
		Ret = _XAie_LPartPoll32(DevInst, RegAddr,
				XAIE_MEM_MOD_MEM_CNTR_ZEROISATION_MASK, 0, 800);
		if (Ret < 0)
			return XAIE_ERR;

		RegAddr = _XAie_LGetTileAddr(XAIE_AIE_TILE_ROW_START - 1, C) +
				XAIE_MEM_TILE_MOD_MEM_CNTR_REGOFF;
		Ret = _XAie_LPartPoll32(DevInst, RegAddr,
				XAIE_MEM_TILE_MEM_CNTR_ZEROISATION_MASK, 0, XAIE4_MEMZERO_POLL_TIMEOUT);
		if (Ret < 0)
			return XAIE_ERR;
	}
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is function to setup the protected register configuration value.
*
* @param	DevInst : AI engine partition device pointer
* @param	Enable: Enable partition
*
* @note		None
*
*******************************************************************************/
static inline void _XAie_LNpiSetPartProtectedReg(XAie_DevInst *DevInst,
		u8 Enable)
{
	u32 StartCol, EndCol, RegVal;

	StartCol = DevInst->StartCol;
	EndCol = DevInst->NumCols + StartCol - 1;

	RegVal = XAie_SetField(Enable, XAIE_NPI_PROT_REG_CNTR_EN_LSB,
			       XAIE_NPI_PROT_REG_CNTR_EN_MSK);

	RegVal |= XAie_SetField(StartCol, XAIE_NPI_PROT_REG_CNTR_FIRSTCOL_LSB,
				XAIE_NPI_PROT_REG_CNTR_FIRSTCOL_MSK);
	RegVal |= XAie_SetField(EndCol, XAIE_NPI_PROT_REG_CNTR_LASTCOL_LSB,
				XAIE_NPI_PROT_REG_CNTR_LASTCOL_MSK);

	_XAie_LNpiSetLock(XAIE_DISABLE);
	_XAie_LNpiWriteCheck32(XAIE_NPI_PROT_REG_CNTR_REG, RegVal);
	_XAie_LNpiSetLock(XAIE_ENABLE);
}

/*****************************************************************************/
/**
 *
 * This API Disable TLAST Error Enable Field in Module Clock Control register.
 * By disabling this bit, control packet can be processed without the need
 * for TLAST to be present after each packet.
 *
 * @param	DevInst: Device Instance
 *
 * @return	None.
 *
 * @note	Internal only. Not Applicable for AIE4 architecture
 *
 *****************************************************************************/
static inline void _XAie_DisableTlast(XAie_DevInst *DevInst)
{
}

/*****************************************************************************/
/**
 *
 * This API sets the L2 Memory split in Dual Application Mode
 * By setting L2 Split Control Register MemTile memory is divided between
 * Application A and B. Max supported L2 Split Value is 0x1F
 *
 * @param	DevInst: Device Instance
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
static inline void _XAie_LSetPartL2Split(XAie_DevInst *DevInst)
{
	u64 RegAddr;
	u32 Mask;
	for(u8 Col = 0; Col < DevInst->NumCols; Col++) {
		for(u8 Row = XAIE_MEM_TILE_ROW_START; Row <= XAIE_MEM_TILE_NUM_ROWS; Row++) {
			RegAddr = XAIE_MEM_TILE_L2_SPLIT_CONTROL + _XAie_LGetTileAddr(Row, Col);
			Mask = XAIE_MEM_TILE_L2_SPLIT_CONTROL_MASK;
			_XAie_LPartMaskWrite32(DevInst, RegAddr, Mask, DevInst->L2Split);
		}
	}
}

/*****************************************************************************/
/**
 *
 * This API configures the registers following POR sequence. It Unlock ME PCSR,
 * Sets ME_IPOR, Sets top row and Row Offset, Releases ARRAY resets, Configures
 * NPI registers, un-gates column clock and disable isolation in entire array.
 *
 * @param	DevInst: Device Instance
 * @param	PorOptions: contains options for ME_TOP_ROW and ROW_OFFSET
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
static inline AieRC _XAie_LAiePorConfiguration(XAie_DevInst *DevInst, XAie_PartPorOpts *PorOptions) {

	u64 RegAddr;
	u32 StartCol,EndCol,RegVal, FldVal;;

	/* Unlock ME PCSR */
	_XAie_LNpiSetLock(XAIE_DISABLE);

	/* Release ME_IPOR & Release Array Reset*/
	_XAie_LNpiSetMeIporReset(XAIE_ENABLE);

	/* Program NPI TOP_ROW and ROW_OFFSET */
	_XAie_LNpiWrite32(XAIE_NPI_PROT_REG_ME_TOP_ROW, PorOptions->ME_TOP_ROW);
	_XAie_LNpiWrite32(XAIE_NPI_PROT_REG_ME_TOP_ROW, PorOptions->ROW_OFFSET);

	/* Configure other NPI registers */
	_XAie_LNpiWrite32(XAIE_NPI_PROT_REG_ME_SECURE_REG,XAIE_DISABLE);
	_XAie_LNpiWrite32(XAIE_NPI_PROT_REG_ME_RAM_EMA_CTRL,XAIE_DISABLE);
	_XAie_LNpiWrite32(XAIE_NPI_PROT_REG_ME_RAM_EMA_CTRL,0x7fff);
	_XAie_LNpiWrite32(XAIE_NPI_PROT_REG_ME_RAM_EMA_CTRL,XAIE_DISABLE);
	_XAie_LNpiWrite32(XAIE_NPI_PROT_REG_ME_RAM_EMA_CTRL,XAIE_NPI_PROT_REG_ME_RAM_EMA_RESET_VALUE);

	/* Update Start Col and Last Col of array in NPI register.
	 * Enable Protection on entire array */
	StartCol = DevInst->StartCol;
	EndCol = StartCol + DevInst->NumCols - 1;

	RegVal = XAie_SetField(XAIE_ENABLE, XAIE_NPI_PROT_REG_CNTR_EN_LSB,
					XAIE_NPI_PROT_REG_CNTR_EN_MSK);

	RegVal |= XAie_SetField(StartCol, XAIE_NPI_PROT_REG_CNTR_FIRSTCOL_LSB,
					XAIE_NPI_PROT_REG_CNTR_FIRSTCOL_MSK);
	RegVal |= XAie_SetField(EndCol, XAIE_NPI_PROT_REG_CNTR_LASTCOL_LSB,
					XAIE_NPI_PROT_REG_CNTR_LASTCOL_MSK);
	_XAie_LNpiWriteCheck32(XAIE_NPI_PROT_REG_CNTR_REG, RegVal);

	/* Un-gate all Columns */
	for(u32 C = 0; C < DevInst->NumCols; C++) {
		RegAddr = _XAie_LGetTileAddr(0,C) + XAIE_PL_MOD_COL_CLKCNTR_REGOFF;
		FldVal = XAie_SetField(XAIE_ENABLE,
					XAIE_PL_MOD_COL_CLKCNTR_CLKBUF_ENABLE_LSB,
					XAIE_PL_MOD_COL_CLKCNTR_CLKBUF_ENABLE_MASK);
		_XAie_LPartMaskWrite32(DevInst, RegAddr,
				XAIE_PL_MOD_COL_CLKCNTR_CLKBUF_ENABLE_MASK, FldVal);
	}

	/* To-Do:Check with Architecture team if Application Reset and Zeroization is required in POR sequence
	 * or can be done during Partition Initialization
	 */

	/* Remove Isolation from all tiles on all sides */
	for(u32 C=0; C < DevInst->NumCols; C++) {
		for(u32 R=0; R < DevInst->NumCols; R++) {
			if(R == XAIE_SHIM_ROW) {
				RegAddr = _XAie_LGetTileAddr(R, C) + XAIE_PL_MOD_TILE_CNTR_REGOFF;
				_XAie_LPartWrite32(DevInst, RegAddr, XAIE_DISABLE);
			} else if (R >= XAIE_MEM_TILE_ROW_START && R < XAIE_AIE_TILE_ROW_START) {
				RegAddr = _XAie_LGetTileAddr(R, C) + XAIE_MEM_TILE_MOD_TILE_CNTR_REGOFF;
				_XAie_LPartWrite32(DevInst, RegAddr, XAIE_DISABLE);
			} else {
				RegAddr = _XAie_LGetTileAddr(R, C) + XAIE_CORE_MOD_TILE_CNTR_REGOFF;
				_XAie_LPartWrite32(DevInst, RegAddr, XAIE_DISABLE);
			}
		}
	}

	/* Zeroize All uC modules */
	_XAie_LZeroInitUcMemory(DevInst);
	/* Enable NPI accept only Secure data */
	_XAie_LNpiWrite32(XAIE_NPI_PROT_REG_ME_SECURE_REG,XAIE_ENABLE);
	/* Lock NPI PCSR */
	_XAie_LNpiSetLock(XAIE_ENABLE);
	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API Enables/Disables clock in Individual AIE Tiles
*
* @param	DevInst: AI engine partition device instance pointer
* @param	Locs: Locations of Tiles to Enable/Disable Module Clock
* @param	NumTiles: Number of tiles for clock Gate/Ungate
* @param	Enable: Enable/Disable Tile Module clock
*
* @return   XAIE_OK on success, error code on failure
*
* @note		Initial support for this API is only for AIE4 devices. It doesn't support
* 			Legacy devices.
*
*******************************************************************************/
static inline AieRC _XAie_LTileClockControl(XAie_DevInst *DevInst, XAie_LocType *Loc,u8 NumTiles, u8 Enable) {

	u32 FldVal;
	u64 RegAddr;
	for(u32 i = 0; i < NumTiles; i++) {
	if(Loc[i].Col >= DevInst->NumCols || Loc[i].Row >= DevInst->NumRows) {
		XAIE_ERROR("Invalid Tile Location\n");
		return XAIE_INVALID_TILE;
	}

	if (Loc[i].Row == XAIE_SHIM_ROW) {
		/* Disable/Enable Column Clock buffer */
		RegAddr = _XAie_LGetTileAddr(Loc[i].Row, Loc[i].Col) + XAIE_PL_MOD_COL_CLKCNTR_REGOFF;
		FldVal = XAie_SetField(Enable,
					XAIE_PL_MOD_COL_CLKCNTR_CLKBUF_ENABLE_LSB,
					XAIE_PL_MOD_COL_CLKCNTR_CLKBUF_ENABLE_MASK);
		_XAie_LPartMaskWrite32(DevInst, RegAddr,
					XAIE_PL_MOD_COL_CLKCNTR_CLKBUF_ENABLE_MASK, FldVal);

		/* Disable/Enable column CERT Clock */
		RegAddr = _XAie_LGetTileAddr(Loc[i].Row, Loc[i].Col) + XAIE_PL_MOD_CTE_CLKCNTR_0_REGOFF;
		FldVal = XAie_SetField(Enable,
					XAIE_PL_MOD_CTE_CLKCNTR_0_CLKBUF_ENABLE_LSB,
						XAIE_PL_MOD_CTE_CLKCNTR_0_CLKBUF_ENABLE_MASK);
		_XAie_LPartMaskWrite32(DevInst, RegAddr,
					XAIE_PL_MOD_CTE_CLKCNTR_0_CLKBUF_ENABLE_MASK, FldVal);
		} else if (Loc[i].Row >= XAIE_AIE_TILE_ROW_START) {
			/* Disable/Enable Compute Tile Module Clock */
			RegAddr = _XAie_LGetTileAddr(Loc[i].Row, Loc[i].Col) + XAIE_AIE_TILE_MODULE_CLOCKCTR_REGOFF;
			FldVal = XAie_SetField(Enable,
						XAIE_AIE_TILE_MODULE_CLOCKCTR_LSB,
						XAIE_AIE_TILE_MODULE_CLOCKCTR_MASK);
			_XAie_LPartMaskWrite32(DevInst, RegAddr,
					XAIE_AIE_TILE_MODULE_CLOCKCTR_MASK, FldVal);
		}
	}
	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API Sets SMID, AxUSER_A or B and Trusted_Keys registers in Shim Tile for each
* application
*
* @param	DevInst: AI engine partition device instance pointer
* @param	XAie_ShimOpts: Values for SMID, AxUSER_A or B, Trusted_Keys
*
* @return   XAIE_OK on success, error code on failure
*
* @note		Initial support for this API is only for AIE4 devices. It doesn't support
* 			Legacy devices.
*
*******************************************************************************/
static inline AieRC _XAie_LConfigureShimDmaRegisters(XAie_DevInst *DevInst, XAie_ShimOpts *ShimOptions){

	u32 FldVal;
	u64 RegAddr;

	for(u8 col=DevInst->StartCol; col< DevInst->NumCols; col++) {
		if(DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE){
			/* In Single App mode both uCA and uCB are configured with same SMID and Trusted keys value */
			/* Configure SMID A register */
			RegAddr = XAIE_PL_MOD_NOC_SMID_A_REGOFF +
						_XAie_LGetTileAddr(XAIE_SHIM_ROW, col);
			FldVal = XAie_SetField(ShimOptions->SMID,
						XAIE_PL_MOD_NOC_SMID_A_LSB,
						XAIE_PL_MOD_NOC_SMID_A_MASK);
			_XAie_LPartWrite32(DevInst, RegAddr, FldVal);

			/* Configure Trusted_Keys A register */
			RegAddr = XAIE_PL_MOD_NOC_TRUSTED_KEYS_A_REGOFF +
						_XAie_LGetTileAddr(XAIE_SHIM_ROW, col);
			FldVal = XAie_SetField(ShimOptions->Trusted_Keys,
						XAIE_PL_MOD_NOC_TRUSTED_KEYS_A_LSB,
						XAIE_PL_MOD_NOC_TRUSTED_KEYS_A_MASK);
			_XAie_LPartWrite32(DevInst, RegAddr, FldVal);

			/* Configure SMID B register */
			RegAddr = XAIE_PL_MOD_NOC_SMID_B_REGOFF +
						_XAie_LGetTileAddr(XAIE_SHIM_ROW, col);
			FldVal = XAie_SetField(ShimOptions->SMID,
						XAIE_PL_MOD_NOC_SMID_B_LSB,
						XAIE_PL_MOD_NOC_SMID_B_MASK);
			_XAie_LPartWrite32(DevInst, RegAddr, FldVal);

			/* Configure Trusted_Keys B register */
			RegAddr = XAIE_PL_MOD_NOC_TRUSTED_KEYS_B_REGOFF +
						_XAie_LGetTileAddr(XAIE_SHIM_ROW, col);
			FldVal = XAie_SetField(ShimOptions->Trusted_Keys,
						XAIE_PL_MOD_NOC_TRUSTED_KEYS_B_LSB,
						XAIE_PL_MOD_NOC_TRUSTED_KEYS_B_MASK);
			_XAie_LPartWrite32(DevInst, RegAddr, FldVal);
		} else if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_A) {
			/* Configure SMID A register */
			RegAddr = XAIE_PL_MOD_NOC_SMID_A_REGOFF +
					_XAie_LGetTileAddr(XAIE_SHIM_ROW, col);
			FldVal = XAie_SetField(ShimOptions->SMID,
						XAIE_PL_MOD_NOC_SMID_A_LSB,
						XAIE_PL_MOD_NOC_SMID_A_MASK);
			_XAie_LPartWrite32(DevInst, RegAddr, FldVal);

			/* Configure Trusted_Keys A register */
			RegAddr = XAIE_PL_MOD_NOC_TRUSTED_KEYS_A_REGOFF +
					_XAie_LGetTileAddr(XAIE_SHIM_ROW, col);
			FldVal = XAie_SetField(ShimOptions->Trusted_Keys,
						XAIE_PL_MOD_NOC_TRUSTED_KEYS_A_LSB,
						XAIE_PL_MOD_NOC_TRUSTED_KEYS_A_MASK);
			_XAie_LPartWrite32(DevInst, RegAddr, FldVal);
		} else {
			/* Configure SMID B register */
			RegAddr = XAIE_PL_MOD_NOC_SMID_B_REGOFF +
					_XAie_LGetTileAddr(XAIE_SHIM_ROW, col);
			FldVal = XAie_SetField(ShimOptions->SMID,
						XAIE_PL_MOD_NOC_SMID_B_LSB,
						XAIE_PL_MOD_NOC_SMID_B_MASK);
			_XAie_LPartWrite32(DevInst, RegAddr, FldVal);

			/* Configure Trusted_Keys B register */
			RegAddr = XAIE_PL_MOD_NOC_TRUSTED_KEYS_B_REGOFF +
					_XAie_LGetTileAddr(XAIE_SHIM_ROW, col);
			FldVal = XAie_SetField(ShimOptions->Trusted_Keys,
						XAIE_PL_MOD_NOC_TRUSTED_KEYS_B_LSB,
						XAIE_PL_MOD_NOC_TRUSTED_KEYS_B_MASK);
			_XAie_LPartWrite32(DevInst, RegAddr, FldVal);
		}
	}
	return XAIE_OK;
}

#endif /* XAIE_LITE_AIE4_H_ */
