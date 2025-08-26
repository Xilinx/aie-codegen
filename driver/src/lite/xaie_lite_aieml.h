/******************************************************************************
* Copyright (C) 2023-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite_aieml.h
* @{
*
* This header file defines a lightweight version of AIEML specific register
* operations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Wendy   09/06/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_LITE_AIEML_H
#define XAIE_LITE_AIEML_H

/***************************** Include Files *********************************/
#include "xaie_lite_hwcfg.h"
#include "xaie_lite_io.h"
#include "xaie_lite_npi.h"
#if ((XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE2P) || \
	(XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE2P_STRIX_B0) || \
	(XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE2P_STRIX_A0))
#include "xaie_lite_regdef_aie2p.h"
#else
#include "xaie_lite_regdef_aieml.h"
#endif
#include "xaiegbl_defs.h"
#include "xaiegbl.h"
#include "xaie_lite_util.h"
#include "xaie_lite_aie2p_ssw_and_dma.h"

/* AIE core registers step size */
#define AIE_CORE_REGS_STEP              0x10
#define MAX_DMA_CHAN			2
#define MAX_DMA_DIR			2


/************************** Constant Definitions *****************************/
/************************** Function Prototypes  *****************************/
/* Set the timeout to maximum zeroization cycles required for Memtile DM zeroization for Sim backend.
   If polling timeout is less driver will return an error before zeroization is complete */
#ifdef __AIESIM__
	#define XAIEML_MEMZERO_POLL_TIMEOUT		150000
	#define A2S_BUFFER_SIZE (1 * 1024)
#else
	#define XAIEML_MEMZERO_POLL_TIMEOUT		1000
	#define A2S_BUFFER_SIZE (64 * 1024)
#endif
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
	RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col + DevInst->StartCol)
		+ XAIE_AIE_TILE_CORE_STATUS_REGOFF;

	/* core status */
	Status[Col].CoreTile[Row].CoreStatus =
		(_XAie_LPartRead32(DevInst, RegAddr)
		 & XAIE_AIE_TILE_CORE_STATUS_MASK);

	/* core program counter addr */
	RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col + DevInst->StartCol)
		+ XAIE_AIE_TILE_CORE_PC_REGOFF;

	/* program counter */
	Status[Col].CoreTile[Row].ProgramCounter =
		(_XAie_LPartRead32(DevInst, RegAddr)
		 & XAIE_AIE_TILE_CORE_PC_MASK);

	/* core stack pointer addr */
	RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col + DevInst->StartCol)
		+ XAIE_AIE_TILE_CORE_SP_REGOFF;

	/* stack pointer */
	Status[Col].CoreTile[Row].StackPtr =
		(_XAie_LPartRead32(DevInst, RegAddr)
		 & XAIE_AIE_TILE_CORE_SP_MASK);

	/* core link addr */
	RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col + DevInst->StartCol)
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
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col + DevInst->StartCol)
			+ Chan * XAIE_TILE_DMA_S2MM_CHANNEL_STATUS_IDX + XAIE_TILE_DMA_S2MM_CHANNEL_STATUS_REGOFF;

		/* read s2mm channel status */
		Status[Col].CoreTile[Row].dma[Chan].S2MMStatus =
			(_XAie_LPartRead32(DevInst, RegAddr) & XAIE_TILE_DMA_S2MM_CHANNEL_VALID_BITS_MASK);

		/* mm2s channel address */
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col + DevInst->StartCol)
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
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col + DevInst->StartCol)
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
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col + DevInst->StartCol)
			+ EventReg * XAIE_AIE_TILE_CORE_MOD_EVENT_STATUS_IDX + XAIE_AIE_TILE_CORE_MOD_EVENT_STATUS_REGOFF;

		/* read event status register and store in output buffer */
		Status[Col].CoreTile[Row].EventCoreModStatus[EventReg] =
			(u32)(_XAie_LPartRead32(DevInst, RegAddr) & XAIE_AIE_TILE_CORE_MOD_EVENT_STATUS_MASK);

		/* event status address */
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_AIE_TILE_ROW_START, Col + DevInst->StartCol)
			+ EventReg * XAIE_AIE_TILE_MEM_MOD_EVENT_STATUS_IDX + XAIE_AIE_TILE_MEM_MOD_EVENT_STATUS_REGOFF;

		/* read event status register and store in output buffer */
		Status[Col].CoreTile[Row].EventMemModStatus[EventReg] =
			(u32)(_XAie_LPartRead32(DevInst, RegAddr) & XAIE_AIE_TILE_MEM_MOD_EVENT_STATUS_MASK);
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
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_MEM_TILE_ROW_START, Col + DevInst->StartCol)
			+ Chan * XAIE_MEM_TILE_DMA_S2MM_CHANNEL_STATUS_IDX + XAIE_MEM_TILE_DMA_S2MM_CHANNEL_STATUS_REGOFF;

		/* read s2mm channel status */
		Status[Col].MemTile[Row].dma[Chan].S2MMStatus =
			(_XAie_LPartRead32(DevInst, RegAddr) & XAIE_MEM_TILE_DMA_S2MM_CHANNEL_VALID_BITS_MASK);

		/* mm2s channel address */
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_MEM_TILE_ROW_START, Col + DevInst->StartCol)
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
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_MEM_TILE_ROW_START, Col + DevInst->StartCol)
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
		RegAddr = _XAie_LGetTileAddr(Row + XAIE_MEM_TILE_ROW_START, Col + DevInst->StartCol)
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
		RegAddr = _XAie_LGetTileAddr(XAIE_SHIM_ROW, Col + DevInst->StartCol)
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
		RegAddr = _XAie_LGetTileAddr(XAIE_SHIM_ROW, Col + DevInst->StartCol) + Chan * XAIE_SHIM_DMA_S2MM_CHANNEL_STATUS_IDX +
			XAIE_SHIM_DMA_S2MM_CHANNEL_STATUS_REGOFF;

		/* read s2mm channel status */
		Status[Col].ShimTile[XAIE_SHIM_ROW].dma[Chan].S2MMStatus =
			(_XAie_LPartRead32(DevInst, RegAddr) & XAIE_SHIM_DMA_S2MM_CHANNEL_VALID_BITS_MASK);

		/* mm2s channel address */
		RegAddr = _XAie_LGetTileAddr(XAIE_SHIM_ROW, Col + DevInst->StartCol) + Chan * XAIE_SHIM_DMA_MM2S_CHANNEL_STATUS_IDX +
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
* This API returns the column status for N number of colums.
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
* This API checks if an AI engine array tile is in use.
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
* This API set SHIM reset in the AI engine partition
*
* @param	DevInst: Device Instance
* @param	Loc: SHIM tile location
* @param	Reset: XAIE_ENABLE to enable reset,
* 			XAIE_DISABLE to disable reset
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		This function is internal.
*		This function does nothing in AIEML.
*
******************************************************************************/
static inline void _XAie_LSetPartColShimReset(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Reset)
{
	(void)DevInst;
	(void)Loc;
	(void)Reset;
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

		if(C == 0) {
			RegVal = XAIE_TILE_CNTR_ISOLATE_WEST_MASK;
		} else if(C == (u8)(DevInst->NumCols - 1)) {
			RegVal = XAIE_TILE_CNTR_ISOLATE_EAST_MASK;
		}

		/* Isolate boundrary of SHIM tiles */
		RegAddr = _XAie_LGetTileAddr(0, C) +
			XAIE_PL_MOD_TILE_CNTR_REGOFF;
		_XAie_LPartWrite32(DevInst, RegAddr, RegVal);

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
    int Ret;
	
	u8 clearA2SBuffer = 0;
	if ((DevInst->HostddrBuffAddr != 0) && (DevInst->HostddrBuffSize > 0)) {
		clearA2SBuffer = 1;
	}

	DMA_SHIM_BD_t mm2s_bd_0 = {0};
	DMA_SHIM_BD_t s2mm_bd_1 = {0};

	if (clearA2SBuffer) {
		//A2S work-around : (1) Stream switch configuration
		SSwitch_Slave_Port_Config_t switch_sub_port = {0};
		SSwitch_Master_Port_Config_t switch_mgr_port = {0};
		switch_sub_port.bits.Slave_Enable = 1;
		switch_mgr_port.bits.Master_Enable = 1;
		switch_mgr_port.bits.Configuration = 0;
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_3), switch_sub_port.value);
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_2), switch_mgr_port.value);

		//A2S work-around : (2) Connect South port with DMA
		Mux_Config_t mux_config = {0};
		Demux_Config_t demux_config = {0};
		mux_config.bits.South3 = 0x1;   //values: PL (0x0), DMA(0x1) or NoC(0x2)
		demux_config.bits.South2 = 0x1; //values: PL (0x0), DMA(0x1) or NoC(0x2)
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_MUX_CONFIG), mux_config.value);
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DEMUX_CONFIG), demux_config.value);

		//A2S work-around : (2) DMA configuration
		DMA_Task_Queue_t mm2s_task_queue = {0};
		DMA_Task_Queue_t s2mm_task_queue = {0};

		//A2S work-around : Configure S2MM BD1
		s2mm_bd_1.reg2.bits.Base_Address_High = (u32)((u64)(DevInst->HostddrBuffAddr + ((size_t)DevInst->HostddrBuffSize/2)) >> 32);
		s2mm_bd_1.reg1.bits.Base_Address_Low = (u32)((u64)(DevInst->HostddrBuffAddr + ((size_t)DevInst->HostddrBuffSize/2)) & 0xFFFFFFFF);
		s2mm_bd_1.reg0.bits.Buffer_Length = ((size_t)DevInst->HostddrBuffSize/2) >> 2;
		s2mm_bd_1.reg5.bits.SMID = DevInst->HostddrBuff_SMID;

		for (int i = 0; i < sizeof(s2mm_bd_1)/sizeof(s2mm_bd_1.reg0) ; i++)
		{
			RegAddr = _XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DMA_BD1_0 + (i * 4);
			_XAie_LPartWrite32(DevInst, RegAddr, *((u32 *)&s2mm_bd_1.reg0 + i));
		}
		s2mm_task_queue.bits.Start_BD_ID = 1;
		s2mm_task_queue.bits.Repeat_Count = ((A2S_BUFFER_SIZE << 1 ) / ((size_t)DevInst->HostddrBuffSize));
		if (s2mm_task_queue.bits.Repeat_Count > 0)
				s2mm_task_queue.bits.Repeat_Count -= 1; // Repeat count is one less than the number of times to repeat
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE), s2mm_task_queue.value);

		//A2S work-around : Configure MM2S BD0
		mm2s_bd_0.reg2.bits.Base_Address_High = (u32)((u64)(DevInst->HostddrBuffAddr) >> 32);
		mm2s_bd_0.reg1.bits.Base_Address_Low = (u32)((u64)(DevInst->HostddrBuffAddr) & 0xFFFFFFFF);
		mm2s_bd_0.reg0.bits.Buffer_Length = ((size_t)DevInst->HostddrBuffSize/2) >> 2;
		mm2s_bd_0.reg5.bits.SMID = DevInst->HostddrBuff_SMID;

		for (int i = 0; i < sizeof(mm2s_bd_0)/sizeof(mm2s_bd_0.reg0) ; i++)
		{
			RegAddr = _XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DMA_BD0_0 + (i * 4);
			_XAie_LPartWrite32(DevInst, RegAddr, *((u32 *)&mm2s_bd_0.reg0 + i));
		}
		mm2s_task_queue.bits.Start_BD_ID = 0;
		mm2s_task_queue.bits.Repeat_Count = (A2S_BUFFER_SIZE << 1 ) / ((size_t)DevInst->HostddrBuffSize);
		if (mm2s_task_queue.bits.Repeat_Count > 0)
				mm2s_task_queue.bits.Repeat_Count -= 1; // Repeat count is one less than the number of times to repeat
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DMA_MM2S_0_TASK_QUEUE), mm2s_task_queue.value);
	}

	for(u8 C = 0; C < DevInst->NumCols; C++) {
		for (u8 R = XAIE_MEM_TILE_ROW_START;
			R < XAIE_AIE_TILE_ROW_START; R++) {
			RegAddr = _XAie_LGetTileAddr(R, C) +
				XAIE_MEM_TILE_MOD_MEM_CNTR_REGOFF;
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
			XAIE_MEM_TILE_MEM_CNTR_ZEROISATION_MASK, 0, 800);

	if (clearA2SBuffer) {

		/* A2S work-around : Poll for A2S buffer written with Dummy data*/
		//Note: After testing remove XAIEML_MEMZERO_POLL_TIMEOUT as A2S DMA should be done in parallel of mem-zeroisation
		Ret = _XAie_LPartPoll32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DMA_S2MM_STATUS_0),
			(XAIE_NOC_MODULE_DMA_S2MM_STATUS_0_MASK | XAIE_NOC_MODULE_DMA_S2MM_STATUS_0_CHANNEL_RUNNIG_MASK), 0, XAIEML_MEMZERO_POLL_TIMEOUT);
		if (Ret < 0)
		{
			XAIE_ERROR("A2S buffer DMA poll time out");
			return;
		}

		//A2S work-around : Reset used fields
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_3), 0x00);
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_2), 0x00);
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_MUX_CONFIG), 0x00);
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DEMUX_CONFIG), 0x00);

		for (int i = 0; i < sizeof(s2mm_bd_1)/sizeof(s2mm_bd_1.reg0) ; i++)
		{
			RegAddr = _XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DMA_BD0_0 + (i * 4);
			_XAie_LPartWrite32(DevInst, RegAddr, 0x0);

			RegAddr = _XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DMA_BD1_0 + (i * 4);
			_XAie_LPartWrite32(DevInst, RegAddr, 0x0);
		}
	}
}

static inline void _XAie_LCertMemZeroInit(XAie_DevInst *DevInst)
{
	(void)DevInst;
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
	Ret = _XAie_LPartPoll32(DevInst, RegAddr, 0xFFFF, Value, 800);
	if (Ret < 0)
		return Ret;

	_XAie_LPartWrite32(DevInst, RegAddr, 0U);
	return 0;
}

static inline AieRC _XAie_LPartDataMemZeroInit(XAie_DevInst *DevInst)
{
	u64 RegAddr;
	int Ret;

	u8 clearA2SBuffer = 0;
	if ((DevInst->HostddrBuffAddr != 0) && (DevInst->HostddrBuffSize > 0)) {
		clearA2SBuffer = 1;
	}

	DMA_SHIM_BD_t mm2s_bd_0 = {0};
	DMA_SHIM_BD_t s2mm_bd_1 = {0};

	if (clearA2SBuffer) {
		//A2S work-around : (1) Stream switch configuration
		SSwitch_Slave_Port_Config_t switch_sub_port = {0};
		SSwitch_Master_Port_Config_t switch_mgr_port = {0};
		switch_sub_port.bits.Slave_Enable = 1;
		switch_mgr_port.bits.Master_Enable = 1;
		switch_mgr_port.bits.Configuration = 0;
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_3), switch_sub_port.value);
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_2), switch_mgr_port.value);

		//A2S work-around : (2) Connect South port with DMA
		Mux_Config_t mux_config = {0};
		Demux_Config_t demux_config = {0};
		mux_config.bits.South3 = 0x1;	//values: PL (0x0), DMA(0x1) or NoC(0x2)
		demux_config.bits.South2 = 0x1;	//values: PL (0x0), DMA(0x1) or NoC(0x2)
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_MUX_CONFIG), mux_config.value);
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DEMUX_CONFIG), demux_config.value);

		//A2S work-around : (2) DMA configuration
		DMA_Task_Queue_t mm2s_task_queue = {0};
		DMA_Task_Queue_t s2mm_task_queue = {0};

		//A2S work-around : Configure S2MM BD1
		s2mm_bd_1.reg2.bits.Base_Address_High = (u32)((u64)(DevInst->HostddrBuffAddr + ((size_t)DevInst->HostddrBuffSize/2)) >> 32);
		s2mm_bd_1.reg1.bits.Base_Address_Low = (u32)((u64)(DevInst->HostddrBuffAddr + ((size_t)DevInst->HostddrBuffSize/2)) & 0xFFFFFFFF);
		s2mm_bd_1.reg0.bits.Buffer_Length = ((size_t)DevInst->HostddrBuffSize/2) >> 2;
		s2mm_bd_1.reg5.bits.SMID = DevInst->HostddrBuff_SMID;

		for (int i = 0; i < sizeof(s2mm_bd_1)/sizeof(s2mm_bd_1.reg0) ; i++)
		{
			RegAddr = _XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DMA_BD1_0 + (i * 4);
			_XAie_LPartWrite32(DevInst, RegAddr, *((u32 *)&s2mm_bd_1.reg0 + i));
		}
		s2mm_task_queue.bits.Start_BD_ID = 1;
		s2mm_task_queue.bits.Repeat_Count = ((A2S_BUFFER_SIZE << 1 ) / ((size_t)DevInst->HostddrBuffSize));
		if (s2mm_task_queue.bits.Repeat_Count > 0)
			s2mm_task_queue.bits.Repeat_Count -= 1; // Repeat count is one less than the number of times to repeat
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE), s2mm_task_queue.value);

		//A2S work-around : Configure MM2S BD0
		mm2s_bd_0.reg2.bits.Base_Address_High = (u32)((u64)(DevInst->HostddrBuffAddr) >> 32);
		mm2s_bd_0.reg1.bits.Base_Address_Low = (u32)((u64)(DevInst->HostddrBuffAddr) & 0xFFFFFFFF);
		mm2s_bd_0.reg0.bits.Buffer_Length = ((size_t)DevInst->HostddrBuffSize/2) >> 2;
		mm2s_bd_0.reg5.bits.SMID = DevInst->HostddrBuff_SMID;

		for (int i = 0; i < sizeof(mm2s_bd_0)/sizeof(mm2s_bd_0.reg0) ; i++)
		{
			RegAddr = _XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DMA_BD0_0 + (i * 4);
			_XAie_LPartWrite32(DevInst, RegAddr, *((u32 *)&mm2s_bd_0.reg0 + i));
		}
		mm2s_task_queue.bits.Start_BD_ID = 0;
		mm2s_task_queue.bits.Repeat_Count = (A2S_BUFFER_SIZE << 1 ) / ((size_t)DevInst->HostddrBuffSize);
		if (mm2s_task_queue.bits.Repeat_Count > 0)
			mm2s_task_queue.bits.Repeat_Count -= 1; // Repeat count is one less than the number of times to repeat
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DMA_MM2S_0_TASK_QUEUE), mm2s_task_queue.value);
	}

	if(DevInst->L2PreserveMem == 0) {
		for(u8 C = 0; C < DevInst->NumCols; C++) {
			for (u8 R = XAIE_MEM_TILE_ROW_START;
					R < XAIE_AIE_TILE_ROW_START; R++) {
				RegAddr = _XAie_LGetTileAddr(R, C) +
					XAIE_MEM_TILE_MOD_MEM_CNTR_REGOFF;
				_XAie_LPartMaskWrite32(DevInst, RegAddr,
						XAIE_MEM_TILE_MEM_CNTR_ZEROISATION_MASK,
						XAIE_MEM_TILE_MEM_CNTR_ZEROISATION_MASK);
			}
		}
	}
	for(u8 C = 0; C < DevInst->NumCols; C++) {
		for (u8 R = XAIE_AIE_TILE_ROW_START;
			R < XAIE_NUM_ROWS; R++) {
			RegAddr = _XAie_LGetTileAddr(R, C) +
				XAIE_MEM_MOD_MEM_CNTR_REGOFF;
			_XAie_LPartMaskWrite32(DevInst, RegAddr,
				XAIE_MEM_MOD_MEM_CNTR_ZEROISATION_MASK,
				XAIE_MEM_MOD_MEM_CNTR_ZEROISATION_MASK);
		}
	}

	for(u8 C = 0; C < DevInst->NumCols; C++) {
		Ret = _XAie_LMemBarrier(DevInst, XAie_TileLoc(C, 0));
		if (Ret < 0)
			return XAIE_ERR;

		/* Poll last mem module and last mem tile mem module */
		RegAddr = _XAie_LGetTileAddr(XAIE_NUM_ROWS - 1, C) +
				XAIE_MEM_MOD_MEM_CNTR_REGOFF;
		Ret = _XAie_LPartPoll32(DevInst, RegAddr,
				XAIE_MEM_MOD_MEM_CNTR_ZEROISATION_MASK, 0, 800);
		if (Ret < 0)
			return XAIE_ERR;
		if(DevInst->L2PreserveMem == 0) {
			RegAddr = _XAie_LGetTileAddr(XAIE_AIE_TILE_ROW_START - 1, C) +
				XAIE_MEM_TILE_MOD_MEM_CNTR_REGOFF;
		Ret = _XAie_LPartPoll32(DevInst, RegAddr,
				XAIE_MEM_TILE_MEM_CNTR_ZEROISATION_MASK, 0, XAIEML_MEMZERO_POLL_TIMEOUT);
		if (Ret < 0)
			return XAIE_ERR;
		}
	}

	if (clearA2SBuffer) {
		/* A2S work-around : Poll for A2S buffer written with Dummy data*/
		//Note: After testing remove XAIEML_MEMZERO_POLL_TIMEOUT as A2S DMA should be done in parallel of mem-zeroisation
		Ret = _XAie_LPartPoll32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DMA_S2MM_STATUS_0),
			(XAIE_NOC_MODULE_DMA_S2MM_STATUS_0_MASK | XAIE_NOC_MODULE_DMA_S2MM_STATUS_0_CHANNEL_RUNNIG_MASK), 0, XAIEML_MEMZERO_POLL_TIMEOUT);
		if (Ret < 0) {
			XAIE_ERROR("A2S buffer DMA poll time out");
			return XAIE_ERR;
		}

		//A2S work-around : Reset used fields
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_3), 0x00);
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_2), 0x00);
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_MUX_CONFIG), 0x00);
		_XAie_LPartWrite32(DevInst, (_XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DEMUX_CONFIG), 0x00);

		for (int i = 0; i < sizeof(s2mm_bd_1)/sizeof(s2mm_bd_1.reg0) ; i++)
		{
			RegAddr = _XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DMA_BD0_0 + (i * 4);
			_XAie_LPartWrite32(DevInst, RegAddr, 0x0);

			RegAddr = _XAie_LGetTileAddr(0, 0) + XAIE_NOC_MODULE_DMA_BD1_0 + (i * 4);
			_XAie_LPartWrite32(DevInst, RegAddr, 0x0);
		}
	}
	return XAIE_OK;
}

/*****************************************************************************/
/**
 *
 * This API sets the L2 Memory split in Dual Application Mode
 * By setting L2 Split Control Register MemTile memory is divided between
 * Application A and B
 *
 * @param	DevInst: Device Instance
 *
 * @return	None.
 *
 * @note	None.
 *
 * @note	Internal only. Not Applicable for AIEML architecture
 *
 *****************************************************************************/
static inline void _XAie_LSetPartL2Split(XAie_DevInst *DevInst)
{
	(void)DevInst;
}

/*****************************************************************************/
/**
 *
 * This API configures the registers following POR sequence. It Unlock ME PCSR,
 * Sets ME_IPOR, Sets top row and Row Offset, Releases ARRAY resets, Configures
 * NPI registers, un-gates column clock and reset/un-reset Application reset for each column,
 * disable isolation in entire array.
 *
 * @param	DevInst: Device Instance
 * @param	PorOptions: contains options for MeTopRow and RowOffset
 *
 * @return	None.
 *
 * @note	None.
 *
 * @note	Internal only. Not Applicable for AIEML architecture
 *
 *****************************************************************************/
static inline AieRC _XAie_LAiePorConfiguration(XAie_DevInst *DevInst, XAie_PartPorOpts *PorOptions)
{
	(void)DevInst;
	(void)PorOptions;

	return XAIE_NOT_SUPPORTED;
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
* @note		Not Supported in AIE2 devices
*
******************************************************************************/

static inline void _XAie_LSetPartDmaPause(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 AppMode, u8 Enable) {
		(void)DevInst;
		(void)Loc;
		(void)AppMode;
		(void)Enable;
}

/*****************************************************************************/
/**
*
* This API set Application reset in the AI engine partition
*
* @param	DevInst: Device Instance
* @param	Loc: SHIM tile location
* @param	Reset: XAIE_ENABLE to enable reset,
* 			XAIE_DISABLE to disable reset
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		Not Supported in AIE2 devices
*
******************************************************************************/
static inline void _XAie_LSetPartColAppReset(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Reset)
{
	(void)DevInst;
	(void)Loc;
	(void)Reset;
}

/*****************************************************************************/
/**
*
* This API polls for pending AXI-MM transactions in the AI engine partition
*
* @param	DevInst: Device Instance
* @param	Loc: SHIM tile location
*
* @note		Not supported in AIE2 devices
*
*
******************************************************************************/
static inline AieRC _XAie_LPollAximmTransactions(XAie_DevInst *DevInst, u8 AppMode, XAie_LocType Loc)
{
	(void)DevInst;
	(void)AppMode;
	(void)Loc;
	return XAIE_NOT_SUPPORTED;
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
* @note		Internal only. Not Applicable for AIEML architecture
*
*******************************************************************************/
static inline AieRC _XAie_LTileClockControl(XAie_DevInst *DevInst, XAie_LocType *Loc,u8 NumTiles, u8 Enable)
{
	(void)DevInst;
	(void)Loc;
	(void)NumTiles;
	(void)Enable;

	return XAIE_NOT_SUPPORTED;
}

/*****************************************************************************/
/**
* This API Enables/Disables column clocks in AIE Columns
*
* @param        DevInst: AI engine partition device instance pointer
* @param        Locs: Locations of Tiles to Enable/Disable Module Clock
* @param        Enable: Enable/Disable Tile Module clock
*
* @return   XAIE_OK on success, error code on failure
*
* @note         Initial support for this API is only for AIE4 devices. It doesn't support
*                       Legacy devices.
*
*******************************************************************************/
static inline AieRC _XAie_LColumnClkControl(XAie_DevInst *DevInst, XAie_LocType Loc, u8 Enable)
{
        (void)DevInst;
        (void)Loc;
        (void)Enable;
        return XAIE_NOT_SUPPORTED;
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
* @note		Internal only. Not Applicable for AIEML architecture
*
*******************************************************************************/
static inline AieRC _XAie_LConfigureShimDmaRegisters(XAie_DevInst *DevInst, XAie_ShimOpts *ShimOptions)
{
	(void)DevInst;
	(void)ShimOptions;

	return XAIE_NOT_SUPPORTED;
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
* This API sets Single App or Dual App mode.
*
* @param	DevInst: Device Instance
* @param	XAie_PartInitOpts: Partition init options
*
* @return	None.
*
* @note		Internal only. Not Applicable for AIEML architecture
*
******************************************************************************/
static inline AieRC _XAie_LSetDualAppModePrivileged(XAie_DevInst *DevInst, XAie_PartInitOpts *Opts)
{
	(void)DevInst;
	(void)Opts;

	return XAIE_OK;
}

#if ((XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE) || \
     (XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIEML) || \
     (XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE2IPU))

/*****************************************************************************/
/**
 *
 * Disable TLAST is not supported on AIEML
 *
 * @param	DevInst: Device Instance
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
static inline void _XAie_DisableTlast(XAie_DevInst *DevInst)
{
	(void)DevInst;
}
#else
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
 * @note	None.
 *
 *****************************************************************************/
static inline void _XAie_DisableTlast(XAie_DevInst *DevInst)
{
	u8 MemTileStart, MemTileEnd, AieRowStart, AieRowEnd;
	u64 RegAddr;
	u32 Mask;

	MemTileStart = XAIE_MEM_TILE_ROW_START;
	MemTileEnd = XAIE_MEM_TILE_ROW_START + XAIE_MEM_TILE_NUM_ROWS;
	AieRowStart = XAIE_AIE_TILE_ROW_START;
	AieRowEnd = XAIE_AIE_TILE_ROW_START + XAIE_AIE_TILE_NUM_ROWS;

	for(u8 Col = 0; Col < DevInst->NumCols; Col++) {
		for(u8 Row = AieRowStart; Row < AieRowEnd; Row++) {
			RegAddr = XAIE_AIE_TILE_CLOCK_CONTROL_REGOFF + _XAie_LGetTileAddr(Row, Col);
			Mask = XAIE_AIE_TILE_CLOCK_CONTROL_CTRL_PKT_TLAST_ERROR_ENABLE_MASK;
			_XAie_LPartMaskWrite32(DevInst, RegAddr, Mask, XAIE_DISABLE);
		}
		for(u8 MemRow = MemTileStart; MemRow < MemTileEnd; MemRow++) {
			RegAddr = XAIE_MEM_TILE_CLOCK_CONTROL_REGOFF + _XAie_LGetTileAddr(MemRow, Col);
			Mask = XAIE_MEM_TILE_CLOCK_CONTROL_CTRL_PKT_TLAST_ERROR_ENABLE_MASK;
			_XAie_LPartMaskWrite32(DevInst, RegAddr, Mask, XAIE_DISABLE);
		}
		/*
		 * Shim tile Clock Control TLAST Error disabled
		 */
		RegAddr = XAIE_SHIM_TILE_MOD_CLOCK_CONTROL_0_REGOFF + _XAie_LGetTileAddr(0, Col);
		Mask = XAIE_SHIM_TILE_MOD_CLOCK_CONTROL_0_CTRL_PKT_TLAST_ERROR_ENABLE_MASK;
		_XAie_LPartMaskWrite32(DevInst, RegAddr, Mask, XAIE_DISABLE);

	}
}

#endif /* GEN == AIE || AIEML || AIE2IPU */

/*****************************************************************************/
/**
 *
 * This API runs loop through all start and end of core registers
 *
 * @param	DevInst: Device Instance
 *		soff   : Start Address of Core Register
 *		eoff   : End Address of Core Register
 *		Row   :  Row number of Tile
 *		Col   :  Column Number
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
static inline void _XAie_WriteCoreReg(XAie_DevInst *DevInst, u32 soff, u32 eoff,
				      u8 Row, u8 Col, u8 width)
{
	u32 RegAddr;
	u8 j;
	for(u32 reg = soff; reg <= eoff; reg+= AIE_CORE_REGS_STEP) {
		for(j = 0; j < width; j++) {
			RegAddr = reg + _XAie_LGetTileAddr(Row, Col);
			_XAie_LPartWrite32(DevInst, (RegAddr + j * 4), 0);
		}
	}
}

/*****************************************************************************/
/**
 *
 * This API Clears Core register as a part of Clear context.
 *
 * @param	DevInst: Device Instance
 *
 * @return	XAIE_OK
 *
 * @note	None.
 *
 *****************************************************************************/
static inline AieRC _XAie_ClearCoreReg(XAie_DevInst *DevInst)
{
	u32 soff, eoff;
	u8 Row, Col;

	for(Row = XAIE_AIE_TILE_ROW_START; Row < DevInst->NumRows; Row++) {
		for(Col = DevInst->StartCol; Col < DevInst->NumCols; Col++) {

                        /* Below registers are 128-bit. So need to write in 4 cycles */
			soff = XAIE_AIE_TILE_CORE_LL_REGOFF;
			eoff = XAIE_AIE_TILE_CORE_HH_REGOFF;
			_XAie_WriteCoreReg(DevInst, soff, eoff, Row, Col, 4);

			soff = XAIE_CORE_MODULE_CORE_WL0_PART1_REGOFF;
			eoff = XAIE_CORE_MODULE_CORE_WH11_PART2_REGOFF;
			_XAie_WriteCoreReg(DevInst, soff, eoff, Row, Col, 4);

#if ((XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE2P) || \
        (XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE2P_STRIX_B0) || \
        (XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE2P_STRIX_A0))

			soff = XAIE_CORE_MODULE_CORE_Q0_REGOFF;
			eoff = XAIE_CORE_MODULE_CORE_LDFIFOH1_PART4_REGOFF;
			_XAie_WriteCoreReg(DevInst, soff, eoff, Row, Col, 4);

			soff = XAIE_CORE_MODULE_CORE_E0_REGOFF;
			eoff = XAIE_CORE_MODULE_CORE_E11_REGOFF;
			/* Below registers are 64-bit. So need to write in 2 cycles */
			_XAie_WriteCoreReg(DevInst, soff, eoff, Row, Col, 2);

#else
                        soff = XAIE_CORE_MODULE_CORE_Q0_REGOFF;
                        eoff = XAIE_CORE_MODULE_CORE_Q3_REGOFF;
                        /* Below registers are 128-bit. So need to write in 4 cycles */
                        _XAie_WriteCoreReg(DevInst, soff, eoff, Row, Col, 4);

#endif
                        soff = XAIE_CORE_MODULE_CORE_R0_REGOFF;
                        eoff = XAIE_CORE_MODULE_CORE_S3_REGOFF;
                        /* Below registers are 32-bit. So need to write in 1 cycles */
                        _XAie_WriteCoreReg(DevInst, soff, eoff, Row, Col, 1);
	
                        soff = XAIE_CORE_MODULE_CORE_PC_START_REGOFF;
                        eoff = XAIE_CORE_MODULE_CORE_PC_END_REGOFF;
                        /* Below registers are 32-bit. So need to write in 1 cycles */
                        _XAie_WriteCoreReg(DevInst, soff, eoff, Row, Col, 1);

		}
	}
	/* Return XAIE_OK to resolve CERT C issues*/
	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API Trigger the Broadcast Interrupt
*
* @param    DevInst: AI engine partition device instance pointer
* @param    BcChan: Broadcast Channel number to be written
*
* @return   XAIE_OK on success, error code on failure
*
*******************************************************************************/
static inline AieRC _XAie_LTrigColIntr(XAie_DevInst *DevInst, u8 BcChan)
{
	(void)DevInst;
	(void)BcChan;

	return XAIE_NOT_SUPPORTED;
}

/*****************************************************************************/
/**
* This API Clears the Broadcast Interrupt
*
* @param    DevInst: AI engine partition device instance pointer
* @param    BcChan: Broadcast Channel number to be written
*
* @return   XAIE_OK on success, error code on failure
*
*******************************************************************************/
static inline AieRC _XAie_LClearBCPort(XAie_DevInst *DevInst, u8 BcChan)
{
	(void)DevInst;
	(void)BcChan;

	return XAIE_NOT_SUPPORTED;
}

/*****************************************************************************/
/**
 *
 * This API Stops issueing new AXI-MM commands before context switch
 * Before context switching Needs to stop new AXI-MM transactions. So
 * that context switch can be done and no data corruption can happen.
 *
 * @param       DevInst: Device Instance
 *
 * @return      None.
 *
 * @note        None.
 *
 *****************************************************************************/
static inline void _XAie_PauseMem(XAie_DevInst *DevInst)
{
	u64 RegAddr;
	u32 RegVal;
	u8 Col, Dir, ChNum;

	/* Here column value will be relative. So Column number will always start from 0 */
	for(Col = 0; Col < DevInst->NumCols; Col++) {
		for(Dir = 0; Dir < MAX_DMA_DIR; Dir++) {
			for(ChNum = 0; ChNum < MAX_DMA_CHAN; ChNum++) {
				RegAddr = _XAie_LGetTileAddr(XAIE_SHIM_ROW, Col) +
					XAIE_SHIM_DMA_MM2S_CHANNEL_CTRL_REGOFF +
					ChNum * XAIE_SHIM_DMA_CHANNEL_CTRL_IDX +
					(u32)Dir * XAIE_SHIM_DMA_CHANNEL_CTRL_IDX_OFFSET;
				RegVal = _XAie_LPartRead32(DevInst, RegAddr);
				RegVal = (RegVal | XAIE_SHIM_DMA_CHANNEL_CTRL_PAUSE_MEM_MASK);
				_XAie_LPartWrite32(DevInst, RegAddr, RegVal);
			}
		}
	}
	usleep(10000);
}

/*****************************************************************************/
/**
*
* This API is used to wakeup the micro controller(s) in shim tile by trigger
* XAIE_EVENT_USER_EVENT_0_PL (USER_EVENT_O) to givem column. Supported for
* AIE2PS and above device generations only.
*
* @param	DevInst: Device Instance
* @param	ColNum: Column Number
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
static inline AieRC _XAie_WakeupShimUc(XAie_DevInst *DevInst, u8 ColNum)
{
	(void)DevInst;
	(void)ColNum;

	return XAIE_NOT_SUPPORTED;
}

#endif		/* end of protection macro */
/** @} */
