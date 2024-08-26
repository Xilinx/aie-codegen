/******************************************************************************
* Copyright (C) 2023 - 2024 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xai4medusagbl_reginit.c
* @{
*
* This file contains the instances of the register bit field definitions for the
* Core, Memory, NoC and PL module registers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Ramakant  02/12/2024  Initial creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaie_core_aieml.h"
#include "xaie_device_aieml.h"
#include "xaie_device_aie.h"
#include "xaie_device_aie4.h"
#include "xaie_dma_aieml.h"
#include "xaie_dma_aie2p.h"
#include "xaie_dma_aie4.h"
#include "xaie_events.h"
#include "xaie_events_aie4.h"
#include "xaie_feature_config.h"
#include "xaie_interrupt_aieml.h"
#include "xaie_locks_aie4.h"
#include "xaie_reset_aieml.h"
#include "xaie_ss_aieml.h"
#include "xaie_ss_aie4.h"
#include "xaiegbl_regdef.h"
#include "xaiemlgbl_params.h"
#include "xaie4gbl_params.h"
#include "xaie_uc.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Macro Definitions ******************************/
#define XAIE4_TILES_BITMAPSIZE	32U

/************************** Variable Definitions *****************************/
/* bitmaps to capture modules being used by the application */
static u32 Aie4TilesInUse[XAIE4_TILES_BITMAPSIZE];
static u32 Aie4MemInUse[XAIE4_TILES_BITMAPSIZE];
static u32 Aie4CoreInUse[XAIE4_TILES_BITMAPSIZE];

#ifdef XAIE_FEATURE_CORE_ENABLE
/*
 * Global instance for Core module Core_Control register.
 */
static const  XAie_RegCoreCtrl Aie4CoreCtrlReg =
{
	XAIE4GBL_CORE_MODULE_CORE_CONTROL,
	{XAIE4GBL_CORE_MODULE_CORE_CONTROL_ENABLE_LSB, XAIE4GBL_CORE_MODULE_CORE_CONTROL_ENABLE_MASK},
	{XAIE4GBL_CORE_MODULE_CORE_CONTROL_RESET_LSB, XAIE4GBL_CORE_MODULE_CORE_CONTROL_RESET_MASK}
};

/*
 * Global instance for Core module Core_Status register.
 */
static const  XAie_RegCoreSts Aie4CoreStsReg =
{
	.RegOff = XAIE4GBL_CORE_MODULE_CORE_STATUS,
	.Mask = XAIE4GBL_CORE_MODULE_CORE_STATUS_CORE_PROCESSOR_BUS_STALL_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_CORE_DONE_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_ERROR_HALT_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_ECC_SCRUBBING_STALL_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_ECC_ERROR_STALL_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_DEBUG_HALT_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_CASCADE_STALL_MCD_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_CASCADE_STALL_SCD_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_STREAM_STALL_MS0_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_STREAM_STALL_SS0_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_STREAM_STALL_SS0_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_LOCK_STALL_E_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_LOCK_STALL_N_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_LOCK_STALL_W_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_LOCK_STALL_S_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_MEMORY_STALL_E_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_MEMORY_STALL_N_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_MEMORY_STALL_W_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_MEMORY_STALL_S_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_RESET_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_RESET_MASK |
		XAIE4GBL_CORE_MODULE_CORE_STATUS_ENABLE_MASK,
	.Done = {XAIE4GBL_CORE_MODULE_CORE_STATUS_CORE_DONE_LSB,
		XAIE4GBL_CORE_MODULE_CORE_STATUS_CORE_DONE_MASK},
	.Rst = {XAIE4GBL_CORE_MODULE_CORE_STATUS_RESET_LSB,
		XAIE4GBL_CORE_MODULE_CORE_STATUS_RESET_MASK},
	.En = {XAIE4GBL_CORE_MODULE_CORE_STATUS_ENABLE_LSB,
		XAIE4GBL_CORE_MODULE_CORE_STATUS_ENABLE_MASK}
};

/*
 * Global instance for Core module for core debug registers.
 */
static const XAie_RegCoreDebug Aie4CoreDebugReg =
{
	.RegOff = XAIE4GBL_CORE_MODULE_DEBUG_CONTROL0,
	.DebugCtrl1Offset = XAIE4GBL_CORE_MODULE_DEBUG_CONTROL1,
	.DebugHaltCoreEvent1.Lsb = XAIE4GBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_HALT_CORE_EVENT1_LSB,
	.DebugHaltCoreEvent1.Mask = XAIE4GBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_HALT_CORE_EVENT1_MASK,
	.DebugHaltCoreEvent0.Lsb = XAIE4GBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_HALT_CORE_EVENT0_LSB,
	.DebugHaltCoreEvent0.Mask = XAIE4GBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_HALT_CORE_EVENT0_MASK,
	.DebugSStepCoreEvent.Lsb = XAIE4GBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_SINGLESTEP_CORE_EVENT_LSB,
	.DebugSStepCoreEvent.Mask = XAIE4GBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_SINGLESTEP_CORE_EVENT_MASK,
	.DebugResumeCoreEvent.Lsb = XAIE4GBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_RESUME_CORE_EVENT_LSB,
	.DebugResumeCoreEvent.Mask = XAIE4GBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_RESUME_CORE_EVENT_MASK,
	.DebugHalt.Lsb = XAIE4GBL_CORE_MODULE_DEBUG_CONTROL0_DEBUG_HALT_BIT_LSB,
	.DebugHalt.Mask = XAIE4GBL_CORE_MODULE_DEBUG_CONTROL0_DEBUG_HALT_BIT_MASK
};

static const XAie_RegCoreDebugStatus Aie4CoreDebugStatus =
{
	.RegOff = XAIE4GBL_CORE_MODULE_DEBUG_STATUS,
	.DbgEvent1Halt.Lsb = XAIE4GBL_CORE_MODULE_DEBUG_STATUS_DEBUG_EVENT1_HALTED_LSB,
	.DbgEvent1Halt.Mask = XAIE4GBL_CORE_MODULE_DEBUG_STATUS_DEBUG_EVENT1_HALTED_MASK,
	.DbgEvent0Halt.Lsb = XAIE4GBL_CORE_MODULE_DEBUG_STATUS_DEBUG_EVENT0_HALTED_LSB,
	.DbgEvent0Halt.Mask = XAIE4GBL_CORE_MODULE_DEBUG_STATUS_DEBUG_EVENT0_HALTED_MASK,
	.DbgStrmStallHalt.Lsb = XAIE4GBL_CORE_MODULE_DEBUG_STATUS_STREAM_STALL_HALTED_LSB,
	.DbgStrmStallHalt.Mask = XAIE4GBL_CORE_MODULE_DEBUG_STATUS_STREAM_STALL_HALTED_MASK,
	.DbgLockStallHalt.Lsb = XAIE4GBL_CORE_MODULE_DEBUG_STATUS_LOCK_STALL_HALTED_LSB,
	.DbgLockStallHalt.Mask = XAIE4GBL_CORE_MODULE_DEBUG_STATUS_LOCK_STALL_HALTED_MASK,
	.DbgMemStallHalt.Lsb = XAIE4GBL_CORE_MODULE_DEBUG_STATUS_MEMORY_STALL_HALTED_LSB,
	.DbgMemStallHalt.Mask = XAIE4GBL_CORE_MODULE_DEBUG_STATUS_MEMORY_STALL_HALTED_MASK,
	.DbgPCEventHalt.Lsb = XAIE4GBL_CORE_MODULE_DEBUG_STATUS_PC_EVENT_HALTED_LSB,
	.DbgPCEventHalt.Mask = XAIE4GBL_CORE_MODULE_DEBUG_STATUS_PC_EVENT_HALTED_MASK,
	.DbgHalt.Lsb = XAIE4GBL_CORE_MODULE_DEBUG_STATUS_DEBUG_HALTED_LSB,
	.DbgHalt.Mask = XAIE4GBL_CORE_MODULE_DEBUG_STATUS_DEBUG_HALTED_MASK,
};

/*
 * Global instance for core event registers in the core module.
 */
static const XAie_RegCoreEvents Aie4CoreEventReg =
{
	.EnableEventOff = XAIE4GBL_CORE_MODULE_ENABLE_EVENTS,
	.DisableEventOccurred.Lsb = XAIE4GBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_OCCURRED_LSB,
	.DisableEventOccurred.Mask = XAIE4GBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_OCCURRED_MASK,
	.DisableEvent.Lsb = XAIE4GBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_LSB,
	.DisableEvent.Mask = XAIE4GBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_MASK,
	.EnableEvent.Lsb = XAIE4GBL_CORE_MODULE_ENABLE_EVENTS_ENABLE_EVENT_LSB,
	.EnableEvent.Mask = XAIE4GBL_CORE_MODULE_ENABLE_EVENTS_ENABLE_EVENT_MASK,
};

/*
 * Global instance for core accumulator control register.
 */
static const XAie_RegCoreAccumCtrl Aie4CoreAccumCtrlReg =
{
	.RegOff = XAIE4GBL_CORE_MODULE_ACCUMULATOR_CONTROL,
	.CascadeInput.Lsb = XAIE4GBL_CORE_MODULE_ACCUMULATOR_CONTROL_INPUT_LSB,
	.CascadeInput.Mask = XAIE4GBL_CORE_MODULE_ACCUMULATOR_CONTROL_INPUT_MASK,
	.CascadeOutput.Lsb = XAIE4GBL_CORE_MODULE_ACCUMULATOR_CONTROL_OUTPUT_LSB,
	.CascadeOutput.Mask = XAIE4GBL_CORE_MODULE_ACCUMULATOR_CONTROL_OUTPUT_MASK,
};
#endif /* XAIE_FEATURE_CORE_ENABLE */

#ifdef XAIE_FEATURE_UC_ENABLE
/*
 * Global instance for uC module Core_Control register.
 */
static const  XAie_RegUcCoreCtrl Aie4UcCoreCtrlReg =
{
	XAIE4GBL_UC_MODULE_CORE_CONTROL,
	{XAIE4GBL_UC_MODULE_CORE_CONTROL_WAKEUP_LSB, XAIE4GBL_UC_MODULE_CORE_CONTROL_WAKEUP_MASK},
	{XAIE4GBL_UC_MODULE_CORE_CONTROL_GO_TO_SLEEP_LSB, XAIE4GBL_UC_MODULE_CORE_CONTROL_GO_TO_SLEEP_MASK}
};

static const  XAie_RegUcCoreSts Aie4UcCoreStsReg =
{
	.RegOff = XAIE4GBL_UC_MODULE_CORE_STATUS,
	.Mask = XAIE4GBL_UC_MODULE_CORE_STATUS_MASK,
	.Intr = {XAIE4GBL_UC_MODULE_CORE_STATUS_INTERRUPT_LSB,
		XAIE4GBL_UC_MODULE_CORE_STATUS_INTERRUPT_MASK},
	.Sleep = {XAIE4GBL_UC_MODULE_CORE_STATUS_SLEEP_LSB,
		XAIE4GBL_UC_MODULE_CORE_STATUS_SLEEP_MASK}
};

static const XAie_UcMod Aie4UcMod =
{
	.IsCheckerBoard = 0U,
	.BaseAddress = 0,
	.ProgMemAddr = 0x0,
	.ProgMemSize = 32 * 1024,
	.ProgMemHostOffset = XAIE4GBL_UC_MODULE_CORE_PROGRAM_MEMORY,
	.PrivDataMemAddr = XAIE4GBL_UC_MODULE_CORE_PRIVATE_DATA_MEMORY,
	.PrivDataMemSize = 16 * 1024,
	.DataMemAddr = XAIE4GBL_UC_MODULE_MODULE_DATA_MEMORY,
	.DataMemSize = 32 * 1024,
	.DataMemUcOffset = 0x20000,
	.UcModuleEventSelect = XAIE4GBL_PL_MODULE_UC_MODULE_EVENT_SELECT,
	.CoreCtrl = &Aie4UcCoreCtrlReg,
	.CoreSts = &Aie4UcCoreStsReg,
	.Wakeup = &_XAie_UcCoreWakeup,
	.Sleep = &_XAie_UcCoreSleep,
	.GetCoreStatus = &_XAie_UcCoreGetStatus
};
#endif /* XAIE_FEATURE_UC_ENABLE */


#ifdef XAIE_FEATURE_DMA_ENABLE
static const  XAie_DmaBdEnProp Aie4MemTileDmaBdEnProp =
{
        .NxtBd.Idx = 10U,
        .NxtBd.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_10_NEXT_BD_LSB,
        .NxtBd.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_10_NEXT_BD_MASK,
        .UseNxtBd.Idx = 10U,
        .UseNxtBd.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_10_USE_NEXT_BD_LSB,
        .UseNxtBd.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_10_USE_NEXT_BD_MASK,
        .OutofOrderBdId.Idx = 9U,
        .OutofOrderBdId.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_9_OUT_OF_ORDER_BD_ID_LSB,
        .OutofOrderBdId.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_9_OUT_OF_ORDER_BD_ID_MASK,
        .TlastSuppress.Idx = 6U,
        .TlastSuppress.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_6_TLAST_SUPPRESS_LSB,
        .TlastSuppress.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_6_TLAST_SUPPRESS_MASK,
};

static const  XAie_DmaBdPkt Aie4MemTileDmaBdPktProp =
{
        .EnPkt.Idx = 7U,
        .EnPkt.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_7_ENABLE_PACKET_LSB,
        .EnPkt.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_7_ENABLE_PACKET_MASK,
        .PktId.Idx = 7U,
        .PktId.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_7_PACKET_ID_LSB,
        .PktId.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_7_PACKET_ID_MASK,
		/* PacketType is per channel for MemTile
        .PktType.Idx = 0U,
        .PktType.Lsb = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_TYPE_LSB,
        .PktType.Mask = XAIEMLGBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_TYPE_MASK,
        */
};

static const  XAie_DmaBdLock Aie4MemTileDmaLockProp =
{
        .AieMlDmaLock.LckRelVal.Idx = 2U,
        .AieMlDmaLock.LckRelVal.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_2_LOCK_REL_VALUE_LSB,
        .AieMlDmaLock.LckRelVal.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_2_LOCK_REL_VALUE_MASK,
        .AieMlDmaLock.LckRelId.Idx = 2U,
        .AieMlDmaLock.LckRelId.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_2_LOCK_REL_ID_LSB,
        .AieMlDmaLock.LckRelId.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_2_LOCK_REL_ID_MASK,
        .AieMlDmaLock.LckAcqEn.Idx = 2U,
        .AieMlDmaLock.LckAcqEn.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_2_LOCK_ACQ_ENABLE_LSB,
        .AieMlDmaLock.LckAcqEn.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_2_LOCK_ACQ_ENABLE_MASK,
        .AieMlDmaLock.LckAcqVal.Idx = 1U,
        .AieMlDmaLock.LckAcqVal.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_1_LOCK_ACQ_VALUE_LSB,
        .AieMlDmaLock.LckAcqVal.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_1_LOCK_ACQ_VALUE_MASK,
        .AieMlDmaLock.LckAcqId.Idx = 2U,
        .AieMlDmaLock.LckAcqId.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_2_LOCK_ACQ_ID_LSB,
        .AieMlDmaLock.LckAcqId.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_2_LOCK_ACQ_ID_MASK,
};

static const  XAie_DmaBdBuffer Aie4MemTileBufferProp =
{
        .TileDmaBuff.BaseAddr.Idx = 0U,
        .TileDmaBuff.BaseAddr.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_0_BASE_ADDRESS_LSB,
        .TileDmaBuff.BaseAddr.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_0_BASE_ADDRESS_MASK,
};

static const  XAie_DmaBdMultiDimAddr Aie4MemTileMultiDimProp =
{
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Idx = 9U,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_9_D0_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_9_D0_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_3_D1_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_3_D1_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Idx = 9U,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_9_D1_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_9_D1_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Idx = 4U,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_4_D2_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_4_D2_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Idx = 10U,
	.AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_10_D2_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_10_D2_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Idx = 5U,
	.AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_5_D3_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_5_D3_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize = {0U},
	.AieMlMultiDimAddr.DmaDimProp[3U].Wrap = {0U},
	.AieMlMultiDimAddr.IterCurr.Idx = 7U,
	.AieMlMultiDimAddr.IterCurr.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_7_ITERATION_CURRENT_LSB,
	.AieMlMultiDimAddr.IterCurr.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_7_ITERATION_CURRENT_MASK,
	.AieMlMultiDimAddr.Iter.Wrap.Idx = 10U,
	.AieMlMultiDimAddr.Iter.Wrap.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_10_ITERATION_WRAP_LSB,
	.AieMlMultiDimAddr.Iter.Wrap.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_10_ITERATION_WRAP_MASK,
	.AieMlMultiDimAddr.Iter.StepSize.Idx = 6U,
	.AieMlMultiDimAddr.Iter.StepSize.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_6_ITERATION_STEPSIZE_LSB,
	.AieMlMultiDimAddr.Iter.StepSize.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_6_ITERATION_STEPSIZE_MASK,
};

static const  XAie_DmaBdPad Aie4MemTilePadProp =
{
        .D0_PadBefore.Idx = 7U,
        .D0_PadBefore.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_7_D0_PAD_BEFORE_LSB,
        .D0_PadBefore.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_7_D0_PAD_BEFORE_MASK,
        .D1_PadBefore.Idx = 8U,
        .D1_PadBefore.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_8_D1_PAD_BEFORE_LSB,
        .D1_PadBefore.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_8_D1_PAD_BEFORE_MASK,
        .D2_PadBefore.Idx = 8U,
        .D2_PadBefore.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_8_D2_PAD_BEFORE_LSB,
        .D2_PadBefore.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_8_D2_PAD_BEFORE_MASK,
        .D0_PadAfter.Idx = 7U,
        .D0_PadAfter.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_7_D0_PAD_AFTER_LSB,
        .D0_PadAfter.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_7_D0_PAD_AFTER_MASK,
        .D1_PadAfter.Idx = 8U,
        .D1_PadAfter.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_8_D1_PAD_AFTER_LSB,
        .D1_PadAfter.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_8_D1_PAD_AFTER_MASK,
        .D2_PadAfter.Idx = 8U,
        .D2_PadAfter.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_8_D2_PAD_AFTER_LSB,
        .D2_PadAfter.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_8_D2_PAD_AFTER_MASK,
};

/* Data structure to capture register offsets and masks for Mem Tile Dma */
static const  XAie_DmaBdProp Aie4MemTileDmaProp =
{
	.AddrAlignMask = 0x3,
	.AddrAlignShift = 0x2,
	.AddrMax = 0x3800000, /* -7,local,+7 */
	.LenActualOffset = 0U,
	.StepSizeMax = (1U << 23) - 1U,
	.WrapMax = (1U << 12U) - 1U,
	.IterStepSizeMax = (1U << 23) - 1U,
	.IterWrapMax = (1U << 8U) - 1U,
	.IterCurrMax = (1U << 8) - 1U,
	.BufferLen.Idx = 1U,
	.BufferLen.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_1_BUFFER_LENGTH_LSB,
	.BufferLen.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_1_BUFFER_LENGTH_MASK,
	.Buffer = &Aie4MemTileBufferProp,
	.DoubleBuffer = NULL,
	.Lock = &Aie4MemTileDmaLockProp,
	.Pkt = &Aie4MemTileDmaBdPktProp,
	.BdEn = &Aie4MemTileDmaBdEnProp,
	.AddrMode = &Aie4MemTileMultiDimProp,
	.Pad = &Aie4MemTilePadProp,
	.Compression = NULL,
	.SysProp = NULL
};

static const XAie_DmaChStatus Aie4MemTileDmaChStatus =
{
	/* This database is common for mm2s and s2mm channels */
	.AieMlDmaChStatus.Status.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STATUS_LSB,
	.AieMlDmaChStatus.Status.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STATUS_MASK,
	.AieMlDmaChStatus.TaskQSize.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_LSB,
	.AieMlDmaChStatus.TaskQSize.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_MASK,
	.AieMlDmaChStatus.ChannelRunning.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_CHANNEL_RUNNING_LSB,
	.AieMlDmaChStatus.ChannelRunning.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_CHANNEL_RUNNING_MASK,
	.AieMlDmaChStatus.StalledLockAcq.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_LSB,
	.AieMlDmaChStatus.StalledLockAcq.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_MASK,
	.AieMlDmaChStatus.StalledLockRel.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_LSB,
	.AieMlDmaChStatus.StalledLockRel.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_MASK,
	.AieMlDmaChStatus.StalledStreamStarve.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_LSB,
	.AieMlDmaChStatus.StalledStreamStarve.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_MASK,
	.AieMlDmaChStatus.StalledTCT.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_LSB,
	.AieMlDmaChStatus.StalledTCT.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_MASK,
};

static const  XAie_DmaChProp Aie4MemTileDmaChProp =
{
	.HasFoTMode = XAIE_FEATURE_AVAILABLE,
	.HasControllerId = XAIE_FEATURE_AVAILABLE,
	.HasEnCompression = XAIE_FEATURE_UNAVAILABLE,
	.HasEnOutOfOrder = XAIE_FEATURE_AVAILABLE,
	.MaxFoTMode = DMA_FoT_COUNTS_FROM_MM_REG,
	.MaxRepeatCount = 4096U,
	.ControllerId.Idx = 0,
	.ControllerId.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_LSB,
	.ControllerId.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_MASK,
	.EnOutofOrder.Idx = 0,
	.EnOutofOrder.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_LSB,
	.EnOutofOrder.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_MASK,
	.FoTMode.Idx = 0,
	.FoTMode.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_LSB,
	.FoTMode.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_MASK ,
	.Reset.Idx = 0,
	.Reset.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_RESET_LSB,
	.Reset.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_RESET_MASK,
	.EnToken.Idx = 1,
	.EnToken.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_LSB,
	.EnToken.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_MASK,
	.RptCount.Idx = 1,
	.RptCount.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_LSB,
	.RptCount.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_MASK,
	.StartBd.Idx = 1,
	.StartBd.Lsb = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_LSB,
	.StartBd.Mask = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_MASK,
	.PauseStream = {0U},
	.PauseMem = {0U},
	.Enable = {0U},
	.StartQSizeMax = 16U,
	.DmaChStatus = &Aie4MemTileDmaChStatus,
};

/* Mem Tile Dma Module */
static const  XAie_DmaMod Aie4MemTileDmaMod =
{
	.BaseAddr = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_BD0_0,
	.IdxOffset = 0x30,  /* This is the offset between each BD */
	.NumBds = 16,		/* Number of BDs for each channel in AIE4 MemTile DMA */
	.NumLocks = 480U,
	.NumAddrDim = 4U,
	.DoubleBuffering = XAIE_FEATURE_UNAVAILABLE,	/* Same as AIE2PS */
	.Compression = XAIE_FEATURE_UNAVAILABLE,	/* Removed in AIE4 */
	.Padding = XAIE_FEATURE_AVAILABLE,		/* Same as AIE2PS */
	.OutofOrderBdId = XAIE_FEATURE_AVAILABLE,	/* Same as AIE2PS */
	.InterleaveMode = XAIE_FEATURE_UNAVAILABLE,	/* Same as AIE2PS */
	.FifoMode = XAIE_FEATURE_UNAVAILABLE,		/* Same as AIE2PS */
	.EnTokenIssue = XAIE_FEATURE_AVAILABLE,		/* Same as AIE2PS */
	.RepeatCount = XAIE_FEATURE_AVAILABLE,		/* Same as AIE2PS */
	.TlastSuppress = XAIE_FEATURE_AVAILABLE,	/* Same as AIE2PS */
	.StartQueueBase = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE,
	.ChCtrlBase = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL,
	.ChCtrlOffset = 0x60,
	.NumChannels = 4,	/* number of s2mm channels */
	.NumMm2sChannels = 6,	/* number of mm2s channels */
	.ChIdxOffset = 0x8,	/* This is the offset between each channel */
	.ChStatusBase = XAIE4GBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0,
	.ChStatusOffset = 0x30,
	.PadValueBase = XAIE4GBL_MEM_TILE_MODULE_DMA_MM2S_0_CONSTANT_PAD_VALUE,
	.BdProp = &Aie4MemTileDmaProp,
	.ChProp = &Aie4MemTileDmaChProp,
	.DmaBdInit = &_XAie4_MemTileDmaInit,
	.SetLock = &_XAieMl_DmaSetLock,
	.SetIntrleave = NULL,
	.SetMultiDim = &_XAie4_DmaSetMultiDim,
	.SetBdIter = &_XAie4_DmaSetBdIteration,
	.WriteBd = &_XAie4_MemTileDmaWriteBd,
	.ReadBd = &_XAie4_MemTileDmaReadBd,
	.PendingBd = &_XAie4_DmaGetPendingBdCount,
	.WaitforDone = &_XAie4_DmaWaitForDone,
	.BdChValidity = &_XAieMl_DmaCheckBdChValidity,
	.UpdateBdLen = &_XAie4_MemTileDmaUpdateBdLen,
	.GetBdLen = &_XAie4_MemTileDmaGetBdLen,
	.UpdateBdAddr = &_XAie4_MemTileDmaUpdateBdAddr,
	.GetChannelStatus = &_XAie4_DmaGetChannelStatus,
	.AxiBurstLenCheck = NULL,
};

static const  XAie_DmaBdEnProp Aie4TileDmaBdEnProp =
{
        .NxtBd.Idx = 5U,
        .NxtBd.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_5_NEXT_BD_LSB,
        .NxtBd.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_5_NEXT_BD_MASK,
        .UseNxtBd.Idx = 5U,
        .UseNxtBd.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_5_USE_NEXT_BD_LSB,
        .UseNxtBd.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_5_USE_NEXT_BD_MASK,
        .OutofOrderBdId.Idx = 1U,
        .OutofOrderBdId.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_1_OUT_OF_ORDER_BD_ID_LSB,
        .OutofOrderBdId.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_1_OUT_OF_ORDER_BD_ID_MASK,
        .TlastSuppress.Idx = 5U,
        .TlastSuppress.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_5_TLAST_SUPPRESS_LSB,
        .TlastSuppress.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_5_TLAST_SUPPRESS_MASK,
};


static const  XAie_DmaBdPkt Aie4TileDmaBdPktProp =
{
        .EnPkt.Idx = 1U,
        .EnPkt.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_PACKET_LSB,
        .EnPkt.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_PACKET_MASK,
        .PktId.Idx = 1U,
        .PktId.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_1_PACKET_ID_LSB,
        .PktId.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_1_PACKET_ID_MASK,
        .PktType.Idx = 1U,
        .PktType.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_1_PACKET_TYPE_LSB,
        .PktType.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_1_PACKET_TYPE_MASK,
};

static const  XAie_DmaBdLock Aie4TileDmaLockProp =
{
        .AieMlDmaLock.LckRelVal.Idx = 5U,
        .AieMlDmaLock.LckRelVal.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_VALUE_LSB,
        .AieMlDmaLock.LckRelVal.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_VALUE_MASK,
        .AieMlDmaLock.LckRelId.Idx = 5U,
        .AieMlDmaLock.LckRelId.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_ID_LSB,
        .AieMlDmaLock.LckRelId.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_ID_MASK,
        .AieMlDmaLock.LckAcqEn.Idx = 5U,
        .AieMlDmaLock.LckAcqEn.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ENABLE_LSB,
        .AieMlDmaLock.LckAcqEn.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ENABLE_MASK,
        .AieMlDmaLock.LckAcqVal.Idx = 5U,
        .AieMlDmaLock.LckAcqVal.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_VALUE_LSB,
        .AieMlDmaLock.LckAcqVal.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_VALUE_MASK,
        .AieMlDmaLock.LckAcqId.Idx = 5U,
        .AieMlDmaLock.LckAcqId.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ID_LSB,
        .AieMlDmaLock.LckAcqId.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ID_MASK,
};

static const  XAie_DmaBdBuffer Aie4TileDmaBufferProp =
{
        .TileDmaBuff.BaseAddr.Idx = 0U,
        .TileDmaBuff.BaseAddr.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_0_BASE_ADDRESS_LSB,
        .TileDmaBuff.BaseAddr.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_0_BASE_ADDRESS_MASK,
};

static const  XAie_DmaBdMultiDimAddr Aie4TileDmaMultiDimProp =
{
		.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Idx = 3U,
		.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_3_D0_WRAP_LSB,
		.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_3_D0_WRAP_MASK,
		.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Idx = 2U,
		.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_2_D1_STEPSIZE_LSB,
		.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_2_D1_STEPSIZE_MASK,
		.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Idx = 4U,
		.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_4_D1_WRAP_LSB,
		.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_4_D1_WRAP_MASK,
		.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Idx = 3U,
		.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_3_D2_STEPSIZE_LSB,
		.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_3_D2_STEPSIZE_MASK,
		.AieMlMultiDimAddr.IterCurr.Idx = 3U,
		.AieMlMultiDimAddr.IterCurr.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_3_ITERATION_CURRENT_LSB,
		.AieMlMultiDimAddr.IterCurr.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_3_ITERATION_CURRENT_MASK,
		.AieMlMultiDimAddr.Iter.Wrap.Idx = 4U,
		.AieMlMultiDimAddr.Iter.Wrap.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_WRAP_LSB,
		.AieMlMultiDimAddr.Iter.Wrap.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_WRAP_MASK,
		.AieMlMultiDimAddr.Iter.StepSize.Idx = 4U,
		.AieMlMultiDimAddr.Iter.StepSize.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_STEPSIZE_LSB,
		.AieMlMultiDimAddr.Iter.StepSize.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_STEPSIZE_MASK,
		.AieMlMultiDimAddr.DmaDimProp[0U].StepSize = {0U},
		.AieMlMultiDimAddr.DmaDimProp[2U].Wrap = {0U},
		.AieMlMultiDimAddr.DmaDimProp[3U].Wrap = {0U},
		.AieMlMultiDimAddr.DmaDimProp[3U].StepSize = {0U}
};

/* Data structure to capture register offsets and masks for Tile Dma */
static const  XAie_DmaBdProp Aie4TileDmaProp =
{
		.AddrAlignMask = 0x3,
		.AddrAlignShift = 0x2,
		.AddrMax = 0x40000,
		.LenActualOffset = 0U,
		.StepSizeMax = (1U << 15) - 1U,
		.WrapMax = (1U << 9U) - 1U,
		.IterStepSizeMax = (1U << 15U) - 1U,
		.IterWrapMax = (1U << 6U) - 1U,
		.IterCurrMax = (1U << 6U) - 1U,
		.BufferLen.Idx = 0U,
		.BufferLen.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_BD0_0_BUFFER_LENGTH_LSB,
		.BufferLen.Mask = XAIE4GBL_MEMORY_MODULE_DMA_BD0_0_BUFFER_LENGTH_MASK,
		.Buffer = &Aie4TileDmaBufferProp,
		.DoubleBuffer = NULL,
		.Lock = &Aie4TileDmaLockProp,
		.Pkt = &Aie4TileDmaBdPktProp,
		.BdEn = &Aie4TileDmaBdEnProp,
		.AddrMode = &Aie4TileDmaMultiDimProp,
		.Pad = NULL,
		.Compression = NULL,
		.SysProp = NULL
};

static const XAie_DmaChStatus Aie4TileDmaChStatus =
{
		/* This database is common for mm2s and s2mm channels */
		.AieMlDmaChStatus.Status.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STATUS_LSB,
		.AieMlDmaChStatus.Status.Mask = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STATUS_MASK,
		.AieMlDmaChStatus.TaskQSize.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_LSB,
		.AieMlDmaChStatus.TaskQSize.Mask = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_MASK,
		.AieMlDmaChStatus.ChannelRunning.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_CHANNEL_RUNNING_LSB,
		.AieMlDmaChStatus.ChannelRunning.Mask = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_CHANNEL_RUNNING_MASK,
		.AieMlDmaChStatus.StalledLockAcq.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_LSB,
		.AieMlDmaChStatus.StalledLockAcq.Mask = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_MASK,
		.AieMlDmaChStatus.StalledLockRel.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_LSB,
		.AieMlDmaChStatus.StalledLockRel.Mask = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_MASK,
		.AieMlDmaChStatus.StalledStreamStarve.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_LSB,
		.AieMlDmaChStatus.StalledStreamStarve.Mask = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_MASK,
		.AieMlDmaChStatus.StalledTCT.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_LSB,
		.AieMlDmaChStatus.StalledTCT.Mask = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_MASK,
};

/* Data structure to capture register offsets and masks for Mem Tile and
 * Tile Dma Channels
 */
static const  XAie_DmaChProp Aie4DmaChProp =
{
		.HasFoTMode = XAIE_FEATURE_AVAILABLE,
		.HasControllerId = XAIE_FEATURE_AVAILABLE,
		.HasEnCompression = XAIE_FEATURE_UNAVAILABLE,
		.HasEnOutOfOrder = XAIE_FEATURE_AVAILABLE,
		.MaxFoTMode = DMA_FoT_COUNTS_FROM_MM_REG,
		.MaxRepeatCount = 4096U,
		.ControllerId.Idx = 0,
		.ControllerId.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_LSB,
		.ControllerId.Mask =XAIE4GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_MASK ,
		.EnOutofOrder.Idx = 0,
		.EnOutofOrder.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_LSB,
		.EnOutofOrder.Mask = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_MASK,
		.FoTMode.Idx = 0,
		.FoTMode.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_LSB,
		.FoTMode.Mask = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_MASK ,
		.Reset.Idx = 0,
		.Reset.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_RESET_LSB,
		.Reset.Mask = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_RESET_MASK,
		.EnToken.Idx = 1,
		.EnToken.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_LSB,
		.EnToken.Mask = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_MASK,
		.RptCount.Idx = 1,
		.RptCount.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_LSB,
		.RptCount.Mask = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_MASK,
		.StartBd.Idx = 1,
		.StartBd.Lsb = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_LSB,
		.StartBd.Mask = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_MASK,
		.PauseStream = {0U},
		.PauseMem = {0U},
		.Enable = {0U},
		.StartQSizeMax = 16U,
		.DmaChStatus = &Aie4TileDmaChStatus,
};

/* Tile Dma Module */
static const  XAie_DmaMod Aie4TileDmaMod =
{
	.BaseAddr = XAIE4GBL_MEMORY_MODULE_DMA_BD0_0,
	.IdxOffset = 0x20,  	/* This is the offset between each BD */
	.NumBds = 16U,			/* Number of BDs for AIE4 Tile DMA */
	.NumLocks = 16U,
	.NumAddrDim = 3U,
	.DoubleBuffering = XAIE_FEATURE_UNAVAILABLE,
	.Compression = XAIE_FEATURE_UNAVAILABLE,
	.Padding = XAIE_FEATURE_UNAVAILABLE,
	.OutofOrderBdId = XAIE_FEATURE_AVAILABLE,
	.InterleaveMode = XAIE_FEATURE_UNAVAILABLE,
	.FifoMode = XAIE_FEATURE_UNAVAILABLE,
	.EnTokenIssue = XAIE_FEATURE_AVAILABLE,
	.RepeatCount = XAIE_FEATURE_AVAILABLE,
	.TlastSuppress = XAIE_FEATURE_AVAILABLE,
	.StartQueueBase = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE,
	.ChCtrlBase = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_0_CTRL,
	.ChCtrlOffset = 0x10,
	.NumChannels = 2U,  /* Number of s2mm channels */
	.NumMm2sChannels = 1U, /* Number of mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.ChStatusBase = XAIE4GBL_MEMORY_MODULE_DMA_S2MM_STATUS_0,
	.ChStatusOffset = 0x10,
	.PadValueBase = XAIE_FEATURE_UNAVAILABLE,
	.BdProp = &Aie4TileDmaProp,
	.ChProp = &Aie4DmaChProp,
	.DmaBdInit = &_XAie4_TileDmaInit,
	.SetLock = &_XAieMl_DmaSetLock,
	.SetIntrleave = NULL,
	.SetMultiDim = &_XAie4_DmaSetMultiDim,
	.SetBdIter = &_XAie4_DmaSetBdIteration,
	.WriteBd = &_XAie4_TileDmaWriteBd,
	.ReadBd = &_XAie4_TileDmaReadBd,
	.PendingBd = &_XAie4_DmaGetPendingBdCount,
	.WaitforDone = &_XAie4_DmaWaitForDone,
	.BdChValidity = &_XAieMl_DmaCheckBdChValidity,
	.UpdateBdLen = &_XAie4_TileDmaUpdateBdLen,
	.GetBdLen = &_XAie4_TileDmaGetBdLen,
	.UpdateBdAddr = &_XAie4_TileDmaUpdateBdAddr,
	.GetChannelStatus = &_XAie4_DmaGetChannelStatus,
	.AxiBurstLenCheck = NULL,
};

static const  XAie_DmaBdEnProp Aie4ShimDmaBdEnProp =
{
        .NxtBd.Idx = 5U,
        .NxtBd.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_5_NEXT_BD_LSB,
        .NxtBd.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_5_NEXT_BD_MASK,
        .UseNxtBd.Idx = 5U,
        .UseNxtBd.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_5_USE_NEXT_BD_LSB,
        .UseNxtBd.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_5_USE_NEXT_BD_MASK,
        .OutofOrderBdId.Idx = 8U,
        .OutofOrderBdId.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_8_OUT_OF_ORDER_BD_ID_LSB,
        .OutofOrderBdId.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_8_OUT_OF_ORDER_BD_ID_MASK,
        .TlastSuppress.Idx = 6U,
        .TlastSuppress.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_6_TLAST_SUPPRESS_LSB,
        .TlastSuppress.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_6_TLAST_SUPPRESS_MASK,
};

static const  XAie_DmaBdPkt Aie4ShimDmaBdPktProp =
{
	.EnPkt.Idx = 5U,
	.EnPkt.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_5_ENABLE_PACKET_LSB,
	.EnPkt.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_5_ENABLE_PACKET_MASK,
	.PktId.Idx = 8U,
	.PktId.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_8_PACKET_ID_LSB,
	.PktId.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_8_PACKET_ID_MASK,
	/* Why Packet ID is removed in AIE4 BD....Need to check?
	.PktType.Idx = 2U,
	.PktType.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_2_PACKET_TYPE_LSB,
	.PktType.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_2_PACKET_TYPE_MASK,*/
};

static const  XAie_DmaBdLock Aie4ShimDmaLockProp =
{
	.AieMlDmaLock.LckRelVal.Idx = 4U,
	.AieMlDmaLock.LckRelVal.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_4_LOCK_REL_VALUE_LSB,
	.AieMlDmaLock.LckRelVal.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_4_LOCK_REL_VALUE_MASK,
	.AieMlDmaLock.LckRelId.Idx = 4U,
	.AieMlDmaLock.LckRelId.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_4_LOCK_REL_ID_LSB,
	.AieMlDmaLock.LckRelId.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_4_LOCK_REL_ID_MASK,
	.AieMlDmaLock.LckAcqEn.Idx = 3U,
	.AieMlDmaLock.LckAcqEn.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_3_LOCK_ACQ_ENABLE_LSB,
	.AieMlDmaLock.LckAcqEn.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_3_LOCK_ACQ_ENABLE_MASK,
	.AieMlDmaLock.LckAcqVal.Idx = 4U,
	.AieMlDmaLock.LckAcqVal.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_4_LOCK_ACQ_VALUE_LSB,
	.AieMlDmaLock.LckAcqVal.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_4_LOCK_ACQ_VALUE_MASK,
	.AieMlDmaLock.LckAcqId.Idx = 3U,
	.AieMlDmaLock.LckAcqId.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_3_LOCK_ACQ_ID_LSB,
	.AieMlDmaLock.LckAcqId.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_3_LOCK_ACQ_ID_MASK,
};

static const  XAie_DmaBdBuffer Aie4ShimDmaBufferProp =
{
	.ShimDmaBuff.AddrLow.Idx = 1U,
	.ShimDmaBuff.AddrLow.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_1_BASE_ADDRESS_LOW_LSB,
	.ShimDmaBuff.AddrLow.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_1_BASE_ADDRESS_LOW_MASK,
	.ShimDmaBuff.AddrHigh.Idx = 0U,
	.ShimDmaBuff.AddrHigh.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_0_BASE_ADDRESS_HIGH_LSB,
	.ShimDmaBuff.AddrHigh.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_0_BASE_ADDRESS_HIGH_MASK,
};

static const  XAie_DmaBdMultiDimAddr Aie4ShimDmaMultiDimProp =
{
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Idx = 4U,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_4_D0_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_4_D0_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Idx = 5U ,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_5_D1_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_5_D1_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_3_D1_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_3_D1_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Idx = 6U,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_6_D2_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_6_D2_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_3_D2_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_3_D2_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Idx = 8U,
	.AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_8_D3_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_8_D3_STEPSIZE_MASK,
	.AieMlMultiDimAddr.IterCurr.Idx = 6U,
	.AieMlMultiDimAddr.IterCurr.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_6_ITERATION_CURRENT_LSB,
	.AieMlMultiDimAddr.IterCurr.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_6_ITERATION_CURRENT_MASK,
	.AieMlMultiDimAddr.Iter.Wrap.Idx = 7U,
	.AieMlMultiDimAddr.Iter.Wrap.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_7_ITERATION_WRAP_LSB,
	.AieMlMultiDimAddr.Iter.Wrap.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_7_ITERATION_WRAP_MASK,
	.AieMlMultiDimAddr.Iter.StepSize.Idx = 7U,
	.AieMlMultiDimAddr.Iter.StepSize.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_7_ITERATION_STEPSIZE_LSB,
	.AieMlMultiDimAddr.Iter.StepSize.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_7_ITERATION_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize = {0U},
	.AieMlMultiDimAddr.DmaDimProp[3U].Wrap = {0U}
};

static const  XAie_DmaSysProp Aie4ShimDmaSysProp =
{
	/* TODO: Check why SMID is removed in RegDB...?
	.SMID.Idx = 5U,
	.SMID.Lsb = XAIEMLGBL_NOC_MODULE_DMA_BD0_5_SMID_LSB,
	.SMID.Mask = XAIEMLGBL_NOC_MODULE_DMA_BD0_5_SMID_MASK,*/
	.BurstLen.Idx = 3U,
	.BurstLen.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_3_BURST_LENGTH_LSB,
	.BurstLen.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_3_BURST_LENGTH_MASK,
	.AxQos.Idx = 5U,
	.AxQos.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_5_AXQOS_LSB,
	.AxQos.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_5_AXQOS_MASK,
	/* secure_access is defeatured in AIE4 */
	.SecureAccess.Idx = 0,
	.SecureAccess.Lsb = 0,
	.SecureAccess.Mask = 0,
	.AxCache.Idx = 7U,
	.AxCache.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_7_AXCACHE_LSB,
	.AxCache.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_7_AXCACHE_MASK,
};

static const  XAie_DmaBdCompression Aie4ShimDmaCompressionProp =
{
	.EnCompression.Idx = 1U,
	.EnCompression.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_6_ENABLE_COMPRESSION_LSB,
	.EnCompression.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_6_ENABLE_COMPRESSION_MASK,
};

/* Data structure to capture register offsets and masks for Tile Dma */
static const  XAie_DmaBdProp Aie4ShimDmaProp =
{
	.AddrAlignMask = 0x3,
	.AddrAlignShift = 0U,
	.AddrMax = 0x200000000000000, /* 57bit max value */
	.LenActualOffset = 0U,
	.StepSizeMax = (1U << 22) - 1U,
	.WrapMax = (1U << 12U) - 1U,
	.IterStepSizeMax = (1U << 22) - 1U,
	.IterWrapMax = (1U << 6U) - 1U,
	.IterCurrMax = (1U << 6) - 1U,
	.BufferLen.Idx = 2U,
	.BufferLen.Lsb = XAIE4GBL_NOC_MODULE_DMA_BD0_2_BUFFER_LENGTH_LSB,
	.BufferLen.Mask = XAIE4GBL_NOC_MODULE_DMA_BD0_2_BUFFER_LENGTH_MASK,
	.Buffer = &Aie4ShimDmaBufferProp,
	.DoubleBuffer = NULL,
	.Lock = &Aie4ShimDmaLockProp,
	.Pkt = &Aie4ShimDmaBdPktProp,
	.BdEn = &Aie4ShimDmaBdEnProp,
	.AddrMode = &Aie4ShimDmaMultiDimProp,
	.Pad = NULL,
	.Compression = &Aie4ShimDmaCompressionProp,
	.SysProp = &Aie4ShimDmaSysProp
};

static const XAie_DmaChStatus Aie4ShimDmaChStatus =
{
	/* This database is common for mm2s and s2mm channels */
	.AieMlDmaChStatus.Status.Lsb = XAIE4GBL_NOC_MODULE_DMA_S2MM_STATUS_0_STATUS_LSB,
	.AieMlDmaChStatus.Status.Mask = XAIE4GBL_NOC_MODULE_DMA_S2MM_STATUS_0_STATUS_MASK,
	.AieMlDmaChStatus.TaskQSize.Lsb = XAIE4GBL_NOC_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_LSB,
	.AieMlDmaChStatus.TaskQSize.Mask = XAIE4GBL_NOC_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_MASK,
	.AieMlDmaChStatus.ChannelRunning.Lsb = XAIE4GBL_NOC_MODULE_DMA_S2MM_STATUS_0_CHANNEL_RUNNING_LSB,
	.AieMlDmaChStatus.ChannelRunning.Mask = XAIE4GBL_NOC_MODULE_DMA_S2MM_STATUS_0_CHANNEL_RUNNING_MASK,
	.AieMlDmaChStatus.StalledLockAcq.Lsb = XAIE4GBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_LSB,
	.AieMlDmaChStatus.StalledLockAcq.Mask = XAIE4GBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_MASK,
	.AieMlDmaChStatus.StalledLockRel.Lsb = XAIE4GBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_LSB,
	.AieMlDmaChStatus.StalledLockRel.Mask = XAIE4GBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_MASK,
	.AieMlDmaChStatus.StalledStreamStarve.Lsb = XAIE4GBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_LSB,
	.AieMlDmaChStatus.StalledStreamStarve.Mask = XAIE4GBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_MASK,
	.AieMlDmaChStatus.StalledTCT.Lsb = XAIE4GBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_LSB,
	.AieMlDmaChStatus.StalledTCT.Mask = XAIE4GBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_MASK,
};

/* Data structure to capture register offsets and masks for Mem Tile and
 * Tile Dma Channels
 */
static const  XAie_DmaChProp Aie4ShimDmaChProp =
{
	.HasFoTMode = XAIE_FEATURE_AVAILABLE,
	.HasControllerId = XAIE_FEATURE_AVAILABLE,
	.HasEnCompression = XAIE_FEATURE_AVAILABLE,
	.HasEnOutOfOrder = XAIE_FEATURE_AVAILABLE,
	.MaxFoTMode = DMA_FoT_COUNTS_FROM_MM_REG,
	.MaxRepeatCount = 4096U,
	.ControllerId.Idx = 0U,
	.ControllerId.Lsb = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_LSB ,
	.ControllerId.Mask = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_MASK ,
	.EnOutofOrder.Idx = 0U,
	.EnOutofOrder.Lsb = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_LSB,
	.EnOutofOrder.Mask = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_MASK,
	.FoTMode.Idx = 0,
	.FoTMode.Lsb = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_LSB,
	.FoTMode.Mask = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_MASK ,
	.PauseStream.Idx = 0,
	.PauseStream.Lsb = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_CTRL_PAUSE_STREAM_LSB,
	.PauseStream.Mask = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_CTRL_PAUSE_STREAM_MASK,
	.EnToken.Idx = 1U,
	.EnToken.Lsb = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_ENABLE_TOKEN_ISSUE_LSB,
	.EnToken.Mask = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_ENABLE_TOKEN_ISSUE_MASK,
	.RptCount.Idx = 1U,
	.RptCount.Lsb = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_REPEAT_COUNT_LSB,
	.RptCount.Mask = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_REPEAT_COUNT_MASK,
	.StartBd.Idx = 1U,
	.StartBd.Lsb = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_START_BD_ID_LSB,
	.StartBd.Mask = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_START_BD_ID_MASK,
	.PauseMem = {0U},
	.Enable = {0U},
	.StartQSizeMax = 16U,
	.DmaChStatus = &Aie4ShimDmaChStatus,
	.EnCompression.Idx = 0U,
	.EnCompression.Lsb = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_CTRL_COMPRESSION_ENABLE_LSB,
	.EnCompression.Mask = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_CTRL_COMPRESSION_ENABLE_MASK,
};

/* Shim Dma Module */
static const  XAie_DmaMod Aie4ShimDmaMod =
{
	.BaseAddr = XAIE4GBL_NOC_MODULE_DMA_BD0_0,
	.IdxOffset = 0x30,		/* This is the offset between each BD */
	.NumBds = 16U,			/* Number of BDs for AIE4 ShimTile DMA */
	.NumMm2sCtrlBds = 8,	/* Number Bds for each control mm2s channel */
	.NumLocks = 16U,
	.NumAddrDim = 4U,
	.DoubleBuffering = XAIE_FEATURE_UNAVAILABLE,
	.Compression = XAIE_FEATURE_AVAILABLE,	/* Added for AIE4 */
	.Padding = XAIE_FEATURE_UNAVAILABLE,
	.OutofOrderBdId = XAIE_FEATURE_AVAILABLE,
	.InterleaveMode = XAIE_FEATURE_UNAVAILABLE,
	.FifoMode = XAIE_FEATURE_UNAVAILABLE,
	.EnTokenIssue = XAIE_FEATURE_AVAILABLE,
	.RepeatCount = XAIE_FEATURE_AVAILABLE,
	.TlastSuppress = XAIE_FEATURE_AVAILABLE,
	.StartQueueBase = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE,
	.ChCtrlBase = XAIE4GBL_NOC_MODULE_DMA_S2MM_0_CTRL,
	.ChCtrlOffset = 0x10,
	.NumChannels = 2U,	/* Number of s2mm channels */
	.NumMm2sChannels = 2U, /* Number of mm2s channels */
	.NumMm2sCtrlChannels = 1, /* Number of control MM2S channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.ChStatusBase = XAIE4GBL_NOC_MODULE_DMA_S2MM_STATUS_0,
	.ChStatusOffset = 0x10,
	.PadValueBase = XAIE_FEATURE_UNAVAILABLE,
	.BdProp = &Aie4ShimDmaProp,
	.ChProp = &Aie4ShimDmaChProp,
	.DmaBdInit = &_XAie4_ShimDmaInit,
	.SetLock = &_XAieMl_DmaSetLock,
	.SetIntrleave = NULL,
	.SetMultiDim = &_XAie4_DmaSetMultiDim,
	.SetBdIter = &_XAie4_DmaSetBdIteration,
	.WriteBd = &_XAie4_ShimDmaWriteBd,
	.ReadBd = &_XAie4_ShimDmaReadBd,
	.PendingBd = &_XAie4_DmaGetPendingBdCount,
	.WaitforDone = &_XAie4_DmaWaitForDone,
	.BdChValidity = &_XAie4_ShimTileDmaCheckBdChValidity,
	.UpdateBdLen = &_XAie4_ShimTileDmaUpdateBdLen,
	.GetBdLen = &_XAie4_ShimTileDmaGetBdLen,
	.UpdateBdAddr = &_XAie4_ShimTileDmaUpdateBdAddr,
	.GetChannelStatus = &_XAie4_DmaGetChannelStatus,
	.AxiBurstLenCheck = &_XAie2P_AxiBurstLenCheck,
};
#endif /* XAIE_FEATURE_DMA_ENABLE */

#ifdef XAIE_FEATURE_SS_ENABLE
/*
 * Array of all Tile Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie4TileStrmMstr[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_AIE_CORE_0,
		.PortPhysicalId = 0,
		.PortLogicalId = 0,
	},
	{	/* DMA_0 */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0,
		.PortPhysicalId = 2,
		.PortLogicalId = 2,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_TILE_CTRL_0,
		.PortPhysicalId = 30,
		.PortLogicalId = 14,
	},
	{	/* Fifo */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* South */
		.NumPorts = 4,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_SOUTH_0,
		.PortPhysicalId = 6,
		.PortLogicalId = 6,
	},
	{	/* West */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_WEST_0,
		.PortPhysicalId = 12,
		.PortLogicalId = 0xFF, /*in dual app mode West ports not available, so no logical ID*/
	},
	{	/* North */
		.NumPorts = 8,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_NORTH_0,
		.PortPhysicalId = 16,
		.PortLogicalId = 9,
	},
	{	/* East */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_EAST_0,
		.PortPhysicalId = 24,
		.PortLogicalId = 0xFF, /*in dual app mode East ports not available, so no logical ID*/
	},
	{	/* Trace */
		/* No Trace ports in AIE4 for 512B switch */
		.NumPorts = 0,
		.PortBaseAddr = 0
	},
	{	/* UCTRLR */
		/* No UCTRLR ports in AIE4 for 512B switch */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Nort_Control */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_NORTH_CONTROL_0,
		.PortPhysicalId = 28,
		.PortLogicalId = 13,
	},
	{	/* South_Control */
		/* No South control ports for master for 512B switch */
		.NumPorts = 0,
		.PortBaseAddr = 0
	},
	{	/* 32b switch */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_32B_SWITCH_0,
		.PortPhysicalId = 31,
		.PortLogicalId = 15,
	},
};

/*
 * Array of all Tile Stream Switch Slave Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie4TileStrmSlv[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_AIE_CORE_0,
		.PortPhysicalId = 0,
		.PortLogicalId = 0,
	},
	{	/* DMA */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_DMA_0,
		.PortPhysicalId = 1,
		.PortLogicalId = 1,
	},
	{	/* Ctrl */
		/* No slave control port for 512B switch */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Fifo */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* South */
		.NumPorts = 8,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_SOUTH_0,
		.PortPhysicalId = 5,
		.PortLogicalId = 5,
	},
	{	/* West */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_WEST_0,
		.PortPhysicalId = 13,
		.PortLogicalId = 0xFF, /* in dual app mode West ports not available, so no logical ID */
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_NORTH_0,
		.PortPhysicalId = 17,
		.PortLogicalId = 9,
	},
	{	/* East */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_EAST_0,
		.PortPhysicalId = 23,
		.PortLogicalId = 0xFF, /* in dual app mode East ports not available, so no logical ID */
	},
	{	/* Trace */
		/* No Trace ports in AIE4 for 512B switch */
		.NumPorts = 0,
		.PortBaseAddr = 0
	},
	{	/* UCTRLR */
		/* No UCTRLR ports in AIE4 for 512B switch */
		.NumPorts = 0,
		.PortBaseAddr = 0
	},
	{	/* Nort_Control */
		/* No South control ports for master for 512B switch */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* South_Control */
		/* No South control ports for master for 512B switch */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_SOUTH_CONTROL_0,
		.PortPhysicalId = 27,
		.PortLogicalId = 12,
	},
	{	/* 32b switch */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_32B_SWITCH_0,
		.PortPhysicalId = 29,
		.PortLogicalId = 13,
	},
};

/*
 * Array of all Tile 32bit Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie4TileStrmMstr32b[SS_PORT_TYPE_MAX] =
{
	{/*Core*/.NumPorts = 0,}, {/*DMA_0*/.NumPorts = 0,}, {/*Ctrl*/.NumPorts = 0,},
	{/*Fifo*/.NumPorts = 0,}, {/*South*/.NumPorts = 0,}, {/* West */.NumPorts = 0,},
	{/*North*/.NumPorts = 0,}, {/* East */.NumPorts = 0,}, {/*Trace*/.NumPorts = 0,},
	{/*UCtrlr*/.NumPorts = 0,}, {/*North_control*/.NumPorts = 0,}, {/*South_control*/.NumPorts = 0,},
	{/*To 32b switch*/.NumPorts = 0,}, {/*DMA control*/.NumPorts = 0,}, {/*PL*/.NumPorts = 0,},

	{	/* _32B_control */
		.NumPorts = 0,
	},
	{	/* _32B_Trace */
		.NumPorts = 0,
	},
	{	/* _32B_South */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0,
		.PortPhysicalId = 0,
		.PortLogicalId = 0,
	},
	{	/* _32B_West */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_WEST_0,
		.PortPhysicalId = 2,
		.PortLogicalId = 0xFF, /*in dual app mode West ports not available, so no logical ID*/
	},
	{	/* 32B_North */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_NORTH_0,
		.PortPhysicalId = 3,
		.PortLogicalId = 1,
	},
	{	/* _32B_East */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_EAST_0,
		.PortPhysicalId = 5,
		.PortLogicalId = 0xFF, /*in dual app mode East ports not available, so no logical ID*/
	},
	{	/* _32B_Switch_512b */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_512B_SWITCH_0,
		.PortPhysicalId = 6,
		.PortLogicalId = 2,
	}
};

/*
 * Array of all Tile Stream Switch Slave Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie4TileStrmSlv32b[SS_PORT_TYPE_MAX] =
{
	{/*Core*/.NumPorts = 0,}, {/*DMA_0*/.NumPorts = 0,}, {/*Ctrl*/.NumPorts = 0,},
	{/*Fifo*/.NumPorts = 0,}, {/*South*/.NumPorts = 0,}, {/* West */.NumPorts = 0,},
	{/*North*/.NumPorts = 0,}, {/* East */.NumPorts = 0,}, {/*Trace*/.NumPorts = 0,},
	{/*UCtrlr*/.NumPorts = 0,}, {/*North_control*/.NumPorts = 0,}, {/*South_control*/.NumPorts = 0,},
	{/*To 32b switch*/.NumPorts = 0,}, {/*DMA control*/.NumPorts = 0,}, {/*PL*/.NumPorts = 0,},

	{	/* _32B_control */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0,
		.PortPhysicalId = 0,
		.PortLogicalId = 0,
	},
	{	/* _32B_Trace */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_AIE_TRACE_0,
		.PortPhysicalId = 1,
		.PortLogicalId = 1,
	},
	{	/* _32B_South */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_SOUTH_0,
		.PortPhysicalId = 3,
		.PortLogicalId = 3,
	},
	{	/* _32B_West */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_WEST_0,
		.PortPhysicalId = 5,
		.PortLogicalId = 0xFF, /*in dual app mode West ports not available, so no logical ID*/
	},
	{	/* 32B_North */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_NORTH_0,
		.PortPhysicalId = 6,
		.PortLogicalId = 4,
	},
	{	/* _32B_East */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_EAST_0,
		.PortPhysicalId = 8,
		.PortLogicalId = 0xFF, /*in dual app mode East ports not available, so no logical ID*/
	},
	{	/* _32B_Switch_512b */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_512B_SWITCH_0,
		.PortPhysicalId = 9,
		.PortLogicalId = 5,
	},
};

/*
 * Array of all Shim NOC Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie4ShimStrmMstr[SS_PORT_TYPE_MAX] =
{
	{       /* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{       /* DMA */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0,
		.PortPhysicalId = 0,
		.PortLogicalId = 0,
	},
	{       /* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_TILE_CTRL_0,
		.PortPhysicalId = 28,
		.PortLogicalId = 6,
	},
	{       /* Fifo */
		/* Spec (0.9.4) mentioned TBD for FIFo in ShimTile, so considering no fifos */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{       /* South */
		/* In ME & AIE2*, the south ports of the stream switch are shared between
		 * PL and DMA streams using muxes and demuxes. In AIE4, PL and DMA streams
		 * are connected to dedicated manager/subordinate stream switch ports.
		 * So, No south ports in AIE4.
		 */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{       /* West */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_WEST_0,
		.PortPhysicalId = 12,
		.PortLogicalId = 0xFF, /* will not be available in dualapp mode */
	},
	{       /* North */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_NORTH_0,
		.PortPhysicalId = 16,
		.PortLogicalId = 2,
	},
	{       /* East */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_EAST_0,
		.PortPhysicalId = 22,
		.PortLogicalId = 0xFF, /* will not be available in dualapp mode */
	},
	{       /* Trace */
		.NumPorts = 0,
		.PortBaseAddr = 0
	},
	{       /* Uctrls */
		.NumPorts = 0,
		.PortBaseAddr = 0
	},
	{       /* North control */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_NORTH_CONTROL_0,
		.PortPhysicalId = 26,
		.PortLogicalId = 5,
	},
	{       /* South control */
		.NumPorts = 0,
		.PortBaseAddr = 0
	},
	{       /* 32b switch */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_32B_SWITCH_0,
		.PortPhysicalId = 30,
		.PortLogicalId = 7,
	},
	{       /* DMA control */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{       /* PL */
		/* Spec (0.9.4) mentioned TBD for PL in ShimTile, so considering no PL ports */
		.NumPorts = 0,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_PL_0,
	}
};

/*
 * Array of all Shim NOC Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie4ShimStrmMstr32b[SS_PORT_TYPE_MAX] =
{
	{/*Core*/.NumPorts = 0,}, {/*DMA_0*/.NumPorts = 0,}, {/*Ctrl*/.NumPorts = 0,},
	{/*Fifo*/.NumPorts = 0,}, {/*South*/.NumPorts = 0,}, {/* West */.NumPorts = 0,},
	{/*North*/.NumPorts = 0,}, {/* East */.NumPorts = 0,}, {/*Trace*/.NumPorts = 0,},
	{/*UCtrlr*/.NumPorts = 0,}, {/*North_control*/.NumPorts = 0,}, {/*South_control*/.NumPorts = 0,},
	{/*To 32b switch*/.NumPorts = 0,}, {/*DMA control*/.NumPorts = 0,}, {/*PL*/.NumPorts = 0,},

	{	/* _32B_control */
		.NumPorts = 0,
	},
	{	/* _32B_Trace */
		.NumPorts = 0,
	},
	{	/* _32B_South */
		.NumPorts = 0,
	},
	{	/* _32B_West */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_WEST_0,
		.PortPhysicalId = 2,
		.PortLogicalId = 0xFF, /*in dual app mode West ports not available, so no logical ID*/
	},
	{	/* 32B_North */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_NORTH_0,
		.PortPhysicalId = 3,
		.PortLogicalId = 1,
	},
	{	/* _32B_East */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_EAST_0,
		.PortPhysicalId = 5,
		.PortLogicalId = 0xFF, /*in dual app mode East ports not available, so no logical ID*/
	},
	{	/* _32B_Switch_512b */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_512B_SWITCH_0,
		.PortPhysicalId = 6,
		.PortLogicalId = 2,
	},
	{	/* _32B_Uctrlr */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_UCONTROLLER_0,
		.PortPhysicalId = 0,
		.PortLogicalId = 0,
	},
	{	/* _32B_PL */
		/* Spec (0.9.4) mentioned TBD for PL in ShimTile, so considering no PL ports */
		.NumPorts = 0,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_PL_0,
	}
};

/*
 * Array of all Shim NOC/PL Stream Switch Slave Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie4ShimStrmSlv[SS_PORT_TYPE_MAX] =
{
	{       /* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{       /* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_DMA_0,
		.PortPhysicalId = 0,
		.PortLogicalId = 0,
	},
	{       /* Ctrl */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{       /* Fifo */
		/* Spec (0.9.4) mentioned TBD for FIFo in ShimTile, so considering no fifos */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{       /* South */
		/* In ME & AIE2*, the south ports of the stream switch are shared between
		 * PL and DMA streams using muxes and demuxes. In AIE4, PL and DMA streams
		 * are connected to dedicated manager/subordinate stream switch ports.
		 * So, No south ports in AIE4.
		 */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{       /* West */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_WEST_0,
		.PortPhysicalId = 14,
		.PortLogicalId = 0xFF, /* will not be available in dualapp mode */
	},
	{       /* North */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_NORTH_0,
		.PortPhysicalId = 18,
		.PortLogicalId = 2,
	},
	{       /* East */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_EAST_0,
		.PortPhysicalId = 22,
		.PortLogicalId = 0xFF, /* will not be available in dualapp mode */
	},
	{       /* Trace */
		.NumPorts = 0,
		.PortBaseAddr = 0
	},
	{       /* Uctrl */
		.NumPorts = 0,
		.PortBaseAddr = 0
	},
	{       /* Nort control */
		.NumPorts = 0,
		.PortBaseAddr = 0
	},
	{       /* south control */
		.NumPorts = 0,
		.PortBaseAddr = 0
	},
	{       /* 32b switch */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_32B_SWITCH_0,
		.PortPhysicalId = 28,
		.PortLogicalId = 5
	},
	{       /* DMA control */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_DMA_CONTROL_0,
		.PortPhysicalId = 26,
		.PortLogicalId = 4
	},
	{       /* PL */
		/* Spec (0.9.4) mentioned TBD for PL in ShimTile, so considering no PL ports */
		.NumPorts = 0,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_PL_0,
	},
};

/*
 * Array of all Shim NOC/PL Stream Switch Slave Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie4ShimStrmSlv32b[SS_PORT_TYPE_MAX] =
{
	{/*Core*/.NumPorts = 0,}, {/*DMA_0*/.NumPorts = 0,}, {/*Ctrl*/.NumPorts = 0,},
	{/*Fifo*/.NumPorts = 0,}, {/*South*/.NumPorts = 0,}, {/* West */.NumPorts = 0,},
	{/*North*/.NumPorts = 0,}, {/* East */.NumPorts = 0,}, {/*Trace*/.NumPorts = 0,},
	{/*UCtrlr*/.NumPorts = 0,}, {/*North_control*/.NumPorts = 0,}, {/*South_control*/.NumPorts = 0,},
	{/*To 32b switch*/.NumPorts = 0,}, {/*DMA control*/.NumPorts = 0,}, {/*PL*/.NumPorts = 0,},

	{	/* _32B_control */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0,
		.PortPhysicalId = 0,
		.PortLogicalId = 0,
	},
	{	/* _32B_Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TRACE_0,
		.PortPhysicalId = 2,
		.PortLogicalId = 1,
	},
	{	/* _32B_South */
		.NumPorts = 0,
	},
	{	/* _32B_West */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_WEST_0,
		.PortPhysicalId = 6,
		.PortLogicalId = 0xFF, /*in dual app mode West ports not available, so no logical ID*/
	},
	{	/* 32B_North */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_NORTH_0,
		.PortPhysicalId = 7,
		.PortLogicalId = 3,
	},
	{	/* _32B_East */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_EAST_0,
		.PortPhysicalId = 9,
		.PortLogicalId = 0xFF, /*in dual app mode East ports not available, so no logical ID*/
	},
	{	/* _32B_Switch_512b */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_512B_SWITCH_0,
		.PortPhysicalId = 10,
		.PortLogicalId = 4,
	},
	{	/* _32B_Uctrlr */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_UCONTROLLER_0,
		.PortPhysicalId = 4,
		.PortLogicalId = 2,
	},
	{	/* _32B_PL */
		/* Spec (0.9.4) mentioned TBD for PL in ShimTile, so considering no PL ports */
		.NumPorts = 0,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_PL_0,
	}
};


/*
 * Array of all Mem Tile Stream Switch Manager Config registers
 * The data structure contains number of ports and the register offsets.
 * This is having half of the resource, user of this array needs
 * to check and double the resource count if its in single app mode.
 */
static const  XAie_StrmPort Aie4MemTileStrmMstr[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		/* No core ports in MEM tile */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 4,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0,
		.PortPhysicalId = 0,
		.PortLogicalId = 0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_TILE_CTRL_0,
		.PortPhysicalId = 26,
		.PortLogicalId = 12,
	},
	{	/* Fifo */
		/* Spec (0.9.4) mentioned TBD for FIFo in memtile, so considering no fifos */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* South */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_SOUTH_0,
		.PortPhysicalId = 10,
		.PortLogicalId = 4,
	},
	{	/* West */
		/* No west ports for MEM tile */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_NORTH_0,
		.PortPhysicalId = 16,
		.PortLogicalId = 7,
	},
	{	/* East */
		/* No East ports in MEM tile */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Trace */
		/* No Trace ports in mem tile */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* UCtrl */
		/* No Uctrl ports in mem tile */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* North Control */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_NORTH_CONTROL_0,
		.PortPhysicalId = 24,
		.PortLogicalId = 11,
	},
	{	/* South control */
		/* No South control ports in mem tile */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* 32b switch */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_32B_SWITCH_0,
		.PortPhysicalId = 28,
		.PortLogicalId = 13,
	}
};


/*
 * Array of all Mem Tile Stream Switch Slave Config registers
 * The data structure contains number of ports and the register offsets
 * This is having half of the resource, user of this array needs
 * to check and double the resource count if its in single app mode.
 */
static const  XAie_StrmPort Aie4MemTileStrmSlv[SS_PORT_TYPE_MAX] =
{
	{   /* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{   /* DMA */
		.NumPorts = 5,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_DMA_0,
		.PortPhysicalId = 0,
		.PortLogicalId = 0,
	},
	{   /* Ctrl */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{   /* Fifo */
		/* Spec (0.9.4) mentioned TBD for FIFo in memtile, so considering no fifos */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{   /* South */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_SOUTH_0,
		.PortPhysicalId = 14,
		.PortLogicalId = 6,
	},
	{   /* West */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{   /* North */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_NORTH_0,
		.PortPhysicalId = 22,
		.PortLogicalId = 10,
	},
	{   /* East */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{   /* Trace */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{   /* Uctrl */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{   /* Nort control */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{   /* Soutch Control */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_SOUTH_CONTROL_0,
		.PortPhysicalId = 28,
		.PortLogicalId = 13,
	},
	{   /* 32b Switch */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_32B_SWITCH_0,
		.PortPhysicalId = 31,
		.PortLogicalId = 14,
	}
};

/*
 * Array of all Mem Tile 32bit Stream Switch Manager Config registers
 * The data structure contains number of ports and the register offsets
 * This is having half of the resource, user of this array needs
 * to check and double the resource count if its in single app mode.
 */
static const  XAie_StrmPort Aie4MemTileStrmMstr32b[SS_PORT_TYPE_MAX] =
{
	{/*Core*/.NumPorts = 0,}, {/*DMA_0*/.NumPorts = 0,}, {/*Ctrl*/.NumPorts = 0,},
	{/*Fifo*/.NumPorts = 0,}, {/*South*/.NumPorts = 0,}, {/* West */.NumPorts = 0,},
	{/*North*/.NumPorts = 0,}, {/* East */.NumPorts = 0,}, {/*Trace*/.NumPorts = 0,},
	{/*UCtrlr*/.NumPorts = 0,}, {/*North_control*/.NumPorts = 0,}, {/*South_control*/.NumPorts = 0,},
	{/*To 32b switch*/.NumPorts = 0,}, {/*DMA control*/.NumPorts = 0,}, {/*PL*/.NumPorts = 0,},

	{	/* 32bit Ctrl */
		.NumPorts = 0,
	},
	{	/* 32bit trace */
		.NumPorts = 0,
	},
	{	/* 32bit South */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0,
		.PortPhysicalId = 0,
		.PortLogicalId = 0,
	},
	{	/* 32bit West */
		.NumPorts = 0,
	},
	{	/* 32bit North */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_NORTH_0,
		.PortPhysicalId = 3,
		.PortLogicalId = 1,
	},
	{	/* 32bit East */
		.NumPorts = 0,
	},
	{	/* 32bit switch 512b */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_512B_SWITCH_0,
		.PortPhysicalId = 6,
		.PortLogicalId = 2,
	},
};


/*
 * Array of all Mem Tile Stream Switch Slave Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie4MemTileStrmSlv32b[SS_PORT_TYPE_MAX] =
{
	{/*Core*/.NumPorts = 0,}, {/*DMA_0*/.NumPorts = 0,}, {/*Ctrl*/.NumPorts = 0,},
	{/*Fifo*/.NumPorts = 0,}, {/*South*/.NumPorts = 0,}, {/* West */.NumPorts = 0,},
	{/*North*/.NumPorts = 0,}, {/* East */.NumPorts = 0,}, {/*Trace*/.NumPorts = 0,},
	{/*UCtrlr*/.NumPorts = 0,}, {/*North_control*/.NumPorts = 0,}, {/*South_control*/.NumPorts = 0,},
	{/*To 32b switch*/.NumPorts = 0,}, {/*DMA control*/.NumPorts = 0,}, {/*PL*/.NumPorts = 0,},

	{	/* 32bit Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0,
		.PortPhysicalId = 0,
		.PortLogicalId = 0,
	},
	{	/* 32bit trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TRACE_0,
		.PortPhysicalId = 2,
		.PortLogicalId = 1,
	},
	{	/* 32bit South */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_SOUTH_0,
		.PortPhysicalId = 4,
		.PortLogicalId = 2,
	},
	{	/* 32bit West */
		.NumPorts = 0,
	},
	{	/* 32bit North */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_NORTH_0,
		.PortPhysicalId = 7,
		.PortLogicalId = 3,
	},
	{	/* 32bit East */
		.NumPorts = 0,
	},
	{	/* 32bit switch 512b */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_512B_SWITCH_0,
		.PortPhysicalId = 10,
		.PortLogicalId = 4,
	},
};

/*
 * Array of all Shim Stream Switch Slave Slot Config registers of AIE4.
 * The data structure contains number of ports and the register base address.
 */
static const  XAie_StrmPort Aie4ShimStrmSlaveSlot[SS_PORT_TYPE_MAX] =
{
        {       /* Core */
                .NumPorts = 0,
                .PortBaseAddr = 0,
        },
        {       /* DMA */
                .NumPorts = 2,
                .PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0,
        },
        {       /* Ctrl */
                .NumPorts = 0,
                .PortBaseAddr = 0,
        },
        {       /* Fifo */
                .NumPorts = 0,
                .PortBaseAddr = 0,
        },
        {       /* South */
                .NumPorts = 0,
                .PortBaseAddr = 0,
        },
        {       /* West */
                .NumPorts = 1,
                .PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_WEST_0_SLOT0,
        },
        {       /* North */
                .NumPorts = 2,
                .PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_NORTH_0_SLOT0,
        },
        {       /* East */
                .NumPorts = 1,
                .PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_EAST_0_SLOT0,
        },
        {       /* Trace */
                .NumPorts = 0,
                .PortBaseAddr = 0,
        },
        {       /* UCtrl */
                .NumPorts = 0,
                .PortBaseAddr = 0,
        },
        {       /* Nort control */
                .NumPorts = 0,
                .PortBaseAddr = 0,
        },
        {       /* south control */
                .NumPorts = 0,
                .PortBaseAddr = 0,
        },
        {       /* 32b switch control */
                .NumPorts = 1,
                .PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_32B_SWITCH_0_SLOT0,
        },
        {       /* DMA control */
                .NumPorts = 1,
                .PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_CONTROL_0_SLOT0,
        },
        {       /* PL */
                .NumPorts = 0,
                .PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_PL_0_SLOT3,
        }
};

/*
 * Array of all Shim Stream Switch Slave Slot Config registers of AIE4.
 * The data structure contains number of ports and the register base address.
 */
static const  XAie_StrmPort Aie4ShimStrmSlaveSlot32b[SS_PORT_TYPE_MAX] =
{
	{/*Core*/.NumPorts = 0,}, {/*DMA_0*/.NumPorts = 0,}, {/*Ctrl*/.NumPorts = 0,},
	{/*Fifo*/.NumPorts = 0,}, {/*South*/.NumPorts = 0,}, {/* West */.NumPorts = 0,},
	{/*North*/.NumPorts = 0,}, {/* East */.NumPorts = 0,}, {/*Trace*/.NumPorts = 0,},
	{/*UCtrlr*/.NumPorts = 0,}, {/*North_control*/.NumPorts = 0,}, {/*South_control*/.NumPorts = 0,},
	{/*To 32b switch*/.NumPorts = 0,}, {/*DMA control*/.NumPorts = 0,}, {/*PL*/.NumPorts = 0,},

	{	/* _32B_control */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0,
	},
	{	/* _32B_Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TRACE_0_SLOT0,
	},
	{	/* _32B_South */
		.NumPorts = 0,
	},
	{	/* _32B_West */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_WEST_0_SLOT0,
	},
	{	/* 32B_North */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_NORTH_0_SLOT0,
		.PortPhysicalId = 7,
		.PortLogicalId = 3,
	},
	{	/* _32B_East */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_EAST_0_SLOT0,
	},
	{	/* _32B_Switch_512b */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_512B_SWITCH_0_SLOT0,
	},
	{	/* _32B_Uctrlr */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_UCONTROLLER_0_SLOT0,
	},
	{	/* _32B_PL */
		/* Spec (0.9.4) mentioned TBD for PL in ShimTile, so considering no PL ports */
		.NumPorts = 0,
		.PortBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_PL_0_SLOT0,
	}
};

/*
 * Array of all AIE4 Tile Stream Switch Slave Slot Config registers.
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie4AieTileStrmSlaveSlot[SS_PORT_TYPE_MAX] =
{
        {       /* Core */
                .NumPorts = 1,
                .PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_AIE_CORE_0_SLOT0,
        },
        {       /* DMA */
                .NumPorts = 1,
                .PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0,
        },
        {       /* Ctrl */
                .NumPorts = 0,
                .PortBaseAddr = 0,
        },
        {       /* Fifo */
                .NumPorts = 0,
                .PortBaseAddr = 0,
        },
        {       /* South */
                .NumPorts = 8,
                .PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_SOUTH_0_SLOT0,
        },
        {       /* West */
                .NumPorts = 2,
                .PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_WEST_0_SLOT0,
        },
        {       /* North */
                .NumPorts = 4,
                .PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_NORTH_0_SLOT0,
        },
        {       /* East */
                .NumPorts = 2,
                .PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_EAST_0_SLOT0,
        },
        {       /* Trace */
                .NumPorts = 0,
                .PortBaseAddr = 0
        },
        {       /* UCtrl */
                .NumPorts = 0,
                .PortBaseAddr = 0
        },
        {       /* Nort control */
                .NumPorts = 0,
                .PortBaseAddr = 0
        },
        {       /* South control */
                .NumPorts = 2,
                .PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_SOUTH_CONTROL_0_SLOT0
        },
        {       /* 32b switch */
                .NumPorts = 1,
                .PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_32B_SWITCH_0_SLOT0
        }
};

/*
 * Array of all AIE4 Tile 32bit Stream Switch Slave Slot Config registers.
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie4AieTileStrmSlaveSlot32b[SS_PORT_TYPE_MAX] =
{
	{/*Core*/.NumPorts = 0,}, {/*DMA_0*/.NumPorts = 0,}, {/*Ctrl*/.NumPorts = 0,},
	{/*Fifo*/.NumPorts = 0,}, {/*South*/.NumPorts = 0,}, {/* West */.NumPorts = 0,},
	{/*North*/.NumPorts = 0,}, {/* East */.NumPorts = 0,}, {/*Trace*/.NumPorts = 0,},
	{/*UCtrlr*/.NumPorts = 0,}, {/*North_control*/.NumPorts = 0,}, {/*South_control*/.NumPorts = 0,},
	{/*To 32b switch*/.NumPorts = 0,}, {/*DMA control*/.NumPorts = 0,}, {/*PL*/.NumPorts = 0,},

	{       /* 32b Core */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0,
	},
        {       /* 32b Trace */
                .NumPorts = 2,
                .PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_AIE_TRACE_0_SLOT0,
        },
        {       /* 32b South */
                .NumPorts = 2,
                .PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_SOUTH_0_SLOT0,
        },
        {       /* 32b West */
                .NumPorts = 1,
                .PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_WEST_0_SLOT0,
        },
        {       /* 32b North */
                .NumPorts = 2,
                .PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_NORTH_0_SLOT0,
        },
        {       /* 32b East */
                .NumPorts = 1,
                .PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_EAST_0_SLOT0,
        },
        {       /* 32b switch_512b*/
                .NumPorts = 1,
                .PortBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_512B_SWITCH_0_SLOT0
        }
};

/*
 * Array of all AIE4 Mem Tile Stream Switch Slave Slot Config registers
 * The data structure contains number of ports and the register offsets
 * This is having half of the resource, user of this array needs
 * to check and double the resource count if its in single app mode.
 */
static const  XAie_StrmPort Aie4MemTileStrmSlaveSlot[SS_PORT_TYPE_MAX] =
{
	{       /* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{       /* DMA */
		.NumPorts = 5,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0,
	},
	{       /* Ctrl */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{       /* Fifo */
		/* Spec (0.9.4) mentioned TBD for FIFo in memtile, so considering no fifos */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{       /* South */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_SOUTH_0_SLOT0,
	},
	{       /* West */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{       /* North */
		.NumPorts = 2,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_NORTH_0_SLOT0,
	},
	{       /* East */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{       /* Trace */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{       /* Uctrl */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{       /* North contol */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{       /* South Control */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_SOUTH_CONTROL_0_SLOT0,
	},
	{       /* 32b Switch */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_32B_SWITCH_0_SLOT0,
	}
};

/*
 * Array of all AIE4 Mem Tile 32bit Stream Switch Slave Slot Config registers
 * The data structure contains number of ports and the register offsets
 * This is having half of the resource, user of this array needs
 * to check and double the resource count if its in single app mode.
 */
static const  XAie_StrmPort Aie4MemTileStrmSlaveSlot32b[SS_PORT_TYPE_MAX] =
{
	{/*Core*/.NumPorts = 0,}, {/*DMA_0*/.NumPorts = 0,}, {/*Ctrl*/.NumPorts = 0,},
	{/*Fifo*/.NumPorts = 0,}, {/*South*/.NumPorts = 0,}, {/* West */.NumPorts = 0,},
	{/*North*/.NumPorts = 0,}, {/* East */.NumPorts = 0,}, {/*Trace*/.NumPorts = 0,},
	{/*UCtrlr*/.NumPorts = 0,}, {/*North_control*/.NumPorts = 0,}, {/*South_control*/.NumPorts = 0,},
	{/*To 32b switch*/.NumPorts = 0,}, {/*DMA control*/.NumPorts = 0,}, {/*PL*/.NumPorts = 0,},

	{	/* 32bit Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0,
	},
	{	/* 32bit trace */
		.NumPorts = 0,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TRACE_0_SLOT0,
	},
	{	/* 32bit South */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_SOUTH_0_SLOT0,
	},
	{	/* 32bit West */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_WEST_0_SLOT0,
	},
	{	/* 32bit North */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_NORTH_0_SLOT0,
	},
	{	/* 32bit East */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_EAST_0_SLOT0,
	},
	{	/* 32bit switch 512b */
		.NumPorts = 1,
		.PortBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_512B_SWITCH_0_SLOT0,
	},
};


static const XAie_StrmSwPortMap Aie4TileStrmSwMasterPortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = CORE,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = CORE,
		.PortNum = 1,
	},
	{
		/* PhyPort 2 */
		.PortType = DMA,
		.PortNum = 0,
	},
	{
		/* PhyPort 3 */
		.PortType = DMA,
		.PortNum = 1,
	},
	{
		/* PhyPort 4 */
		.PortType = FIFO,
		.PortNum = 0,
	},
	{
		/* PhyPort 5 */
		.PortType = FIFO,
		.PortNum = 1,
	},
	{
		/* PhyPort 6 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 7 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 8, as per spec its place holder for future devices */
		.PortType = SOUTH,
		.PortNum = 'X',
	},
	{
		/* PhyPort 9 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 10 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 11, as per spec its place holder for future devices */
		.PortType = SOUTH,
		.PortNum = 'Y',
	},
	{
		/* PhyPort 12 */
		.PortType = WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 13 */
		.PortType = WEST,
		.PortNum = 1,
	},
	{
		/* PhyPort 14 */
		.PortType = WEST,
		.PortNum = 2,
	},
	{
		/* PhyPort 15 */
		.PortType = WEST,
		.PortNum = 3,
	},
	{
		/* PhyPort 16 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 17 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 18 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 19 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 20 */
		.PortType = NORTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 21 */
		.PortType = NORTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 22 */
		.PortType = NORTH,
		.PortNum = 6,
	},
	{
		/* PhyPort 23 */
		.PortType = NORTH,
		.PortNum = 7,
	},
	{
		/* PhyPort 24 */
		.PortType = EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 25 */
		.PortType = EAST,
		.PortNum = 1,
	},
	{
		/* PhyPort 26 */
		.PortType = EAST,
		.PortNum = 2,
	},
	{
		/* PhyPort 27 */
		.PortType = EAST,
		.PortNum = 3,
	},
	{
		/* PhyPort 28 */
		.PortType = NORTH_CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 29 */
		.PortType = NORTH_CTRL,
		.PortNum = 1,
	},
	{
		/* PhyPort 30 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 31 */
		.PortType = SWITCH_32b,
		.PortNum = 0,
	},

};

static const XAie_StrmSwPortMap Aie4TileStrmSwSlavePortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = CORE,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = DMA,
		.PortNum = 0,
	},
	{
		/* PhyPort 2 */
		.PortType = DMA,
		.PortNum = 1,
	},
	{
		/* PhyPort 3 */
		.PortType = FIFO,
		.PortNum = 0,
	},
	{
		/* PhyPort 4 */
		.PortType = FIFO,
		.PortNum = 1,
	},
	{
		/* PhyPort 5 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 6 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 7 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 8 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 9 */
		.PortType = SOUTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 10 */
		.PortType = SOUTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 11 */
		.PortType = SOUTH,
		.PortNum = 6,
	},
	{
		/* PhyPort 12 */
		.PortType = SOUTH,
		.PortNum = 7,
	},
	{
		/* PhyPort 13 */
		.PortType = WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 14 */
		.PortType = WEST,
		.PortNum = 1,
	},
	{
		/* PhyPort 15 */
		.PortType = WEST,
		.PortNum = 2,
	},
	{
		/* PhyPort 16 */
		.PortType = WEST,
		.PortNum = 3,
	},
	{
		/* PhyPort 17 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 18 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 19, as per spec its place holder for future devices */
		.PortType = NORTH,
		.PortNum = 'X',
	},
	{
		/* PhyPort 20 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 21 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 22, as per spec its place holder for future devices */
		.PortType = NORTH,
		.PortNum = 'Y',
	},
	{
		/* PhyPort 23 */
		.PortType = EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 24 */
		.PortType = EAST,
		.PortNum = 1,
	},
	{
		/* PhyPort 25 */
		.PortType = EAST,
		.PortNum = 2,
	},
	{
		/* PhyPort 26 */
		.PortType = EAST,
		.PortNum = 3,
	},
	{
		/* PhyPort 27 */
		.PortType = SOUTH_CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 28 */
		.PortType = SOUTH_CTRL,
		.PortNum = 1,
	},
	{
		/* PhyPort 29 */
		.PortType = SWITCH_32b,
		.PortNum = 0,
	},
};

static const XAie_StrmSwPortMap Aie4TileStrmSw32bMasterPortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = _32B_SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = _32B_SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 2 */
		.PortType = _32B_WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 3 */
		.PortType = _32B_NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 4 */
		.PortType = _32B_NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 5 */
		.PortType = _32B_EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 6 */
		.PortType = _32B_SWITCH_512b,
		.PortNum = 0,
	},
};

static const XAie_StrmSwPortMap Aie4TileStrmSw32bSlavePortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = _32B_CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = _32B_TRACE,
		.PortNum = 0,
	},
	{
		/* PhyPort 2 */
		.PortType = _32B_TRACE,
		.PortNum = 1,
	},
	{
		/* PhyPort 3 */
		.PortType = _32B_SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 4 */
		.PortType = _32B_SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 5 */
		.PortType = _32B_WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 6 */
		.PortType = _32B_NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 7 */
		.PortType = _32B_NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 8 */
		.PortType = _32B_EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 9 */
		.PortType = _32B_SWITCH_512b,
		.PortNum = 0,
	},
};

static const XAie_StrmSwPortMap Aie4ShimStrmSwMasterPortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = DMA,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = DMA,
		.PortNum = 1,
	},
	{
		/* PhyPort 2 */
		.PortType = DMA,
		.PortNum = 2,
	},
	{
		/* PhyPort 3 */
		.PortType = DMA,
		.PortNum = 3,
	},
	{
		/* PhyPort 4 */
		.PortType = FIFO,
		.PortNum = 0,
	},
	{
		/* PhyPort 5 */
		.PortType = FIFO,
		.PortNum = 1,
	},
	{
		/* PhyPort 6 */
		.PortType = PL,
		.PortNum = 0,
	},
	{
		/* PhyPort 7 */
		.PortType = PL,
		.PortNum = 1,
	},
	{
		/* PhyPort 8 */
		.PortType = PL,
		.PortNum = 2,
	},
	{
		/* PhyPort 9 */
		.PortType = PL,
		.PortNum = 3,
	},
	{
		/* PhyPort 10 */
		.PortType = PL,
		.PortNum = 4,
	},
	{
		/* PhyPort 11 */
		.PortType = PL,
		.PortNum = 5,
	},
	{
		/* PhyPort 12 */
		.PortType = WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 13 */
		.PortType = WEST,
		.PortNum = 1,
	},
	{
		/* PhyPort 14 */
		.PortType = WEST,
		.PortNum = 2,
	},
	{
		/* PhyPort 15 */
		.PortType = WEST,
		.PortNum = 3,
	},
	{
		/* PhyPort 16 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 17 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 18 */
		.PortType = NORTH,
		.PortNum = 'X',
	},
	{
		/* PhyPort 19 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 20 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 21 */
		.PortType = NORTH,
		.PortNum = 'Y',
	},
	{
		/* PhyPort 22 */
		.PortType = EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 23 */
		.PortType = EAST,
		.PortNum = 1,
	},
	{
		/* PhyPort 24 */
		.PortType = EAST,
		.PortNum = 2,
	},
	{
		/* PhyPort 25 */
		.PortType = EAST,
		.PortNum = 3,
	},
	{
		/* PhyPort 26 */
		.PortType = NORTH_CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 27 */
		.PortType = NORTH_CTRL,
		.PortNum = 1,
	},
	{
		/* PhyPort 28 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 29 */
		.PortType = CTRL,
		.PortNum = 1,
	},
	{
		/* PhyPort 30 */
		.PortType = SWITCH_32b,
		.PortNum = 0,
	},
	{
		/* PhyPort 31 */
		.PortType = SWITCH_32b,
		.PortNum = 1,
	}
};

static const XAie_StrmSwPortMap Aie4ShimStrmSwMasterPortMap32b[] =
{
	{
		/* PhyPort 0 */
		.PortType = _32B_UCTRLR,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = _32B_UCTRLR,
		.PortNum = 1,
	},
	{
		/* PhyPort 2 */
		.PortType = _32B_WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 3 */
		.PortType = _32B_NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 4 */
		.PortType = _32B_NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 5 */
		.PortType = _32B_EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 6 */
		.PortType = _32B_SWITCH_512b,
		.PortNum = 0,
	},
	{
		/* PhyPort 7 */
		.PortType = _32B_SWITCH_512b,
		.PortNum = 1,
	},
	{
		/* PhyPort 8 */
		.PortType = _32B_PL,
		.PortNum = 0,
	},
	{
		/* PhyPort 9 */
		.PortType = _32B_PL,
		.PortNum = 1,
	},
};

static const XAie_StrmSwPortMap Aie4ShimStrmSwSlavePortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = DMA,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = DMA,
		.PortNum = 1,
	},
	{
		/* PhyPort 2 */
		.PortType = DMA,
		.PortNum = 2,
	},
	{
		/* PhyPort 3 */
		.PortType = DMA,
		.PortNum = 3,
	},
	{
		/* PhyPort 4 */
		.PortType = FIFO,
		.PortNum = 0,
	},
	{
		/* PhyPort 5 */
		.PortType = FIFO,
		.PortNum = 5,
	},
	/* TODO: If needed, PortType PL should be added.... Need to check this*/
	{
		/* PhyPort 14 */
		.PortType = WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 15 */
		.PortType = WEST,
		.PortNum = 1,
	},
	{
		/* PhyPort 16 */
		.PortType = WEST,
		.PortNum = 2,
	},
	{
		/* PhyPort 17 */
		.PortType = WEST,
		.PortNum = 3,
	},
	{
		/* PhyPort 18 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 19 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 20 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 21 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 22 */
		.PortType = EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 23 */
		.PortType = EAST,
		.PortNum = 1,
	},
	{
		/* PhyPort 24 */
		.PortType = EAST,
		.PortNum = 2,
	},
	{
		/* PhyPort 25 */
		.PortType = EAST,
		.PortNum = 3,
	},
	{
		/* PhyPort 26 */
		.PortType = DMA_CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 27 */
		.PortType = DMA_CTRL,
		.PortNum = 1,
	},
	{
		/* PhyPort 28 */
		.PortType = SWITCH_32b,
		.PortNum = 0,
	},
	{
		/* PhyPort 29 */
		.PortType = SWITCH_32b,
		.PortNum = 1,
	}
};

static const XAie_StrmSwPortMap Aie4ShimStrmSwSlavePortMap32b[] =
{
	{
		/* PhyPort 0 */
		.PortType = _32B_CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = _32B_CTRL,
		.PortNum = 1,
	},
	{
		/* PhyPort 2 */
		.PortType = _32B_TRACE,
		.PortNum = 0,
	},
	{
		/* PhyPort 3 */
		.PortType = _32B_TRACE,
		.PortNum = 1,
	},
	{
		/* PhyPort 4 */
		.PortType = _32B_UCTRLR,
		.PortNum = 0,
	},
	{
		/* PhyPort 5 */
		.PortType = _32B_UCTRLR,
		.PortNum = 1,
	},
	{
		/* PhyPort 6 */
		.PortType = _32B_WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 7 */
		.PortType = _32B_NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 8 */
		.PortType = _32B_NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 9 */
		.PortType = _32B_EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 10 */
		.PortType = _32B_SWITCH_512b,
		.PortNum = 0,
	},
	{
		/* PhyPort 11 */
		.PortType = _32B_SWITCH_512b,
		.PortNum = 1,
	},
	{
		/* PhyPort 12 */
		.PortType = _32B_PL,
		.PortNum = 0,
	},
	{
		/* PhyPort 13 */
		.PortType = _32B_PL,
		.PortNum = 1,
	}
};

static const XAie_StrmSwPortMap Aie4MemTileStrmSwMasterPortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = DMA,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = DMA,
		.PortNum = 1,
	},
	{
		/* PhyPort 2 */
		.PortType = DMA,
		.PortNum = 2,
	},
	{
		/* PhyPort 3 */
		.PortType = DMA,
		.PortNum = 3,
	},
	{
		/* PhyPort 4 */
		.PortType = DMA,
		.PortNum = 4,
	},
	{
		/* PhyPort 5 */
		.PortType = DMA,
		.PortNum = 5,
	},
	{
		/* PhyPort 6 */
		.PortType = DMA,
		.PortNum = 6,
	},
	{
		/* PhyPort 7 */
		.PortType = DMA,
		.PortNum = 7,
	},
	{
		/* PhyPort 8 */
		.PortType = FIFO,
		.PortNum = 0,
	},
	{
		/* PhyPort 9 */
		.PortType = FIFO,
		.PortNum = 1,
	},
	{
		/* PhyPort 10 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 11 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 12 */
		.PortType = SOUTH,
		.PortNum = 'X',
	},
	{
		/* PhyPort 13 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 14 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 15 */
		.PortType = SOUTH,
		.PortNum = 'Y',
	},
	{
		/* PhyPort 16 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 17 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 18 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 19 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 20 */
		.PortType = NORTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 21 */
		.PortType = NORTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 22 */
		.PortType = NORTH,
		.PortNum = 6,
	},
	{
		/* PhyPort 23 */
		.PortType = NORTH,
		.PortNum = 7,
	},
	{
		/* PhyPort 24 */
		.PortType = NORTH_CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 25 */
		.PortType = NORTH_CTRL,
		.PortNum = 1,
	},
	{
		/* PhyPort 26 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 27 */
		.PortType = CTRL,
		.PortNum = 1,
	},
	{
		/* PhyPort 28 */
		.PortType = SWITCH_32b,
		.PortNum = 0,
	},
	{
		/* PhyPort 29 */
		.PortType = SWITCH_32b,
		.PortNum = 1,
	}
};

static const XAie_StrmSwPortMap Aie4MemTileStrmSwSlavePortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = DMA,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = DMA,
		.PortNum = 1,
	},
	{
		/* PhyPort 2 */
		.PortType = DMA,
		.PortNum = 2,
	},
	{
		/* PhyPort 3 */
		.PortType = DMA,
		.PortNum = 3,
	},
	{
		/* PhyPort 4 */
		.PortType = DMA,
		.PortNum = 4,
	},
	{
		/* PhyPort 5, is hole in v1.1 */
		.PortType = SS_PORT_TYPE_MAX,
		.PortNum = 5,
	},
	{
		/* PhyPort 6 */
		.PortType = DMA,
		.PortNum = 6,
	},
	{
		/* PhyPort 7 */
		.PortType = DMA,
		.PortNum = 7,
	},
	{
		/* PhyPort 8 */
		.PortType = DMA,
		.PortNum = 8,
	},
	{
		/* PhyPort 9 */
		.PortType = DMA,
		.PortNum = 9,
	},
	{
		/* PhyPort 10 */
		.PortType = DMA,
		.PortNum = 10,
	},
	{
		/* PhyPort 11, is hole in v1.1 */
		.PortType = SS_PORT_TYPE_MAX,
		.PortNum = 11,
	},
	{
		/* PhyPort 12 */
		.PortType = FIFO,
		.PortNum = 0,
	},
	{
		/* PhyPort 13 */
		.PortType = FIFO,
		.PortNum = 1,
	},
	{
		/* PhyPort 14 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 15 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 16 */
		.PortType = SOUTH,
		.PortNum = 'X',
	},
	{
		/* PhyPort 17 */
		.PortType = SOUTH,
		.PortNum = 'Y',
	},
	{
		/* PhyPort 18 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 19 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 20 */
		.PortType = SOUTH,
		.PortNum = 'Z',
	},
	{
		/* PhyPort 21 */
		.PortType = SOUTH,
		.PortNum = 'A',
	},
	{
		/* PhyPort 22 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 23 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 24 */
		.PortType = NORTH,
		.PortNum = 'X',
	},
	{
		/* PhyPort 25 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 26 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 27 */
		.PortType = NORTH,
		.PortNum = 'Y',
	},
	{
		/* PhyPort 28 */
		.PortType = SOUTH_CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 29 */
		.PortType = SOUTH_CTRL,
		.PortNum = 1,
	},
	{
		/* PhyPort 30 */
		.PortType = SWITCH_32b,
		.PortNum = 0,
	},
	{
		/* PhyPort 31 */
		.PortType = SWITCH_32b,
		.PortNum = 1,
	}
};

static const XAie_StrmSwPortMap Aie4MemTileStrmSw32bMasterPortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = _32B_SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = _32B_SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 2 */
		.PortType = _32B_WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 3 */
		.PortType = _32B_NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 4 */
		.PortType = _32B_NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 5 */
		.PortType = _32B_EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 6 */
		.PortType = _32B_SWITCH_512b,
		.PortNum = 0,
	},
	{
		/* PhyPort 7 */
		.PortType = _32B_SWITCH_512b,
		.PortNum = 1,
	},
};

static const XAie_StrmSwPortMap Aie4MemTileStrmSw32bSlavePortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = CTRL,
		.PortNum = 1,
	},
	{
		/* PhyPort 2 */
		.PortType = TRACE,
		.PortNum = 0,
	},
	{
		/* PhyPort 3 */
		.PortType = TRACE,
		.PortNum = 1,
	},
	{
		/* PhyPort 4 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 5 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 6 */
		.PortType = WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 7 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 8 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 9 */
		.PortType = EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 10 */
		.PortType = _32B_SWITCH_512b,
		.PortNum = 0,
	},
	{
		/* PhyPort 11 */
		.PortType = _32B_SWITCH_512b,
		.PortNum = 1,
	},
};

/*
 * Data structure to capture stream switch deterministic merge properties for
 * AIE4 Tiles.
 */
static const XAie_StrmSwDetMerge Aie4AieTileStrmSwDetMerge = {
	.NumArbitors = 2U,
	.NumPositions = 4U,
	.ArbConfigOffset = 0x0C,
	.ConfigBase = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1,
	.EnableBase = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL,
	.SlvId0.Lsb = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_SUBORDINATE_ID_0_LSB,
	.SlvId0.Mask = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_SUBORDINATE_ID_0_MASK,
	.SlvId1.Lsb = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_SUBORDINATE_ID_1_LSB,
	.SlvId1.Mask = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_SUBORDINATE_ID_1_MASK,
	.PktCount0.Lsb = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_PACKET_COUNT_0_LSB,
	.PktCount0.Mask = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_PACKET_COUNT_0_MASK,
	.PktCount1.Lsb = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_PACKET_COUNT_1_LSB,
	.PktCount1.Mask = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_PACKET_COUNT_1_MASK,
	.Enable.Lsb = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_LSB,
	.Enable.Mask = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_MASK,
};

/*
 * Data structure to capture stream switch deterministic merge properties for
 * AIE4 Mem Tiles.
 * This is having half of the arbiter, user of this structure needs
 * to check and double the arbiter count if its in single app mode.
 */
static const XAie_StrmSwDetMerge Aie4MemTileStrmSwDetMerge = {
	.NumArbitors = 2U,
	.NumPositions = 4U,
	.ArbConfigOffset = 0x0C,
	.ConfigBase = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1,
	.EnableBase = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL,
	.SlvId0.Lsb = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_SUBORDINATE_ID_0_LSB,
	.SlvId0.Mask = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_SUBORDINATE_ID_0_MASK,
	.SlvId1.Lsb = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_SUBORDINATE_ID_1_LSB,
	.SlvId1.Mask = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_SUBORDINATE_ID_1_MASK,
	.PktCount0.Lsb = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_PACKET_COUNT_0_LSB,
	.PktCount0.Mask = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_PACKET_COUNT_0_MASK,
	.PktCount1.Lsb = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_PACKET_COUNT_1_LSB,
	.PktCount1.Mask = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_PACKET_COUNT_1_MASK,
	.Enable.Lsb = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_LSB,
	.Enable.Mask = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_MASK,
};

/*
 * Data structure to capture stream switch deterministic merge properties for
 * AIE4 SHIM PL Tiles.
 * This is having half of the arbiter, user of this structure needs
 * to check and double the arbiter count if its in single app mode.
 */
static const XAie_StrmSwDetMerge Aie4ShimTileStrmSwDetMerge = {
	.NumArbitors = 2U,
	.NumPositions = 4U,
	.ArbConfigOffset = 0x0C,
	.ConfigBase = XAIE4GBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1,
	.EnableBase = XAIE4GBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL,
	.SlvId0.Lsb = XAIE4GBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_SUBORDINATE_ID_0_LSB,
	.SlvId0.Mask = XAIE4GBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_SUBORDINATE_ID_0_MASK,
	.SlvId1.Lsb = XAIE4GBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_SUBORDINATE_ID_1_LSB,
	.SlvId1.Mask = XAIE4GBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_SUBORDINATE_ID_1_MASK,
	.PktCount0.Lsb = XAIE4GBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_PACKET_COUNT_0_LSB,
	.PktCount0.Mask = XAIE4GBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_PACKET_COUNT_0_MASK,
	.PktCount1.Lsb = XAIE4GBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_PACKET_COUNT_1_LSB,
	.PktCount1.Mask = XAIE4GBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SUBORDINATE0_1_PACKET_COUNT_1_MASK,
	.Enable.Lsb = XAIE4GBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_LSB,
	.Enable.Mask = XAIE4GBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_MASK,
};


/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_AIETILE
 */
static const  XAie_StrmMod Aie4TileStrmSw =
{
	.SlvConfigBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_AIE_CORE_0,
	.MstrConfigBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_AIE_CORE_0,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.DetMergeFeature = XAIE_FEATURE_AVAILABLE,
	.MstrEn = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_AIE_CORE_0_MANAGER_ENABLE_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_AIE_CORE_0_MANAGER_ENABLE_MASK},
	.MstrPktEn = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_AIE_CORE_0_PACKET_ENABLE_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_AIE_CORE_0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_AIE_CORE_0_DROP_HEADER_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_AIE_CORE_0_DROP_HEADER_MASK},
	.Config = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_AIE_CORE_0_CONFIGURATION_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_AIE_CORE_0_CONFIGURATION_MASK},
	.SlvEn = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_AIE_CORE_0_SUBORDINATE_ENABLE_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_AIE_CORE_0_SUBORDINATE_ENABLE_MASK},
	.SlvPktEn = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_AIE_CORE_0_PACKET_ENABLE_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_AIE_CORE_0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_AIE_CORE_0_SLOT0_ID_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_AIE_CORE_0_SLOT0_ID_MASK},
	.SlotMask = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_AIE_CORE_0_SLOT0_MASK_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_AIE_CORE_0_SLOT0_MASK_MASK},
	.SlotEn = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_AIE_CORE_0_SLOT0_ENABLE_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_AIE_CORE_0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_AIE_CORE_0_SLOT0_MSEL_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_AIE_CORE_0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_AIE_CORE_0_SLOT0_ARBIT_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_SUBORDINATE_AIE_CORE_0_SLOT0_ARBIT_MASK},
	.MstrConfig = Aie4TileStrmMstr,
	.SlvConfig = Aie4TileStrmSlv,
	.SlvSlotConfig = Aie4AieTileStrmSlaveSlot,
	.MaxMasterPhyPortId = 31U,
	.MaxSlavePhyPortId = 29U,
	.MasterPortMap = Aie4TileStrmSwMasterPortMap,
	.SlavePortMap = Aie4TileStrmSwSlavePortMap,
	.DetMerge = &Aie4AieTileStrmSwDetMerge,
	.PortVerify = _XAie4_AieTile_StrmSwCheckPortValidity,
};

static const XAie_StrmMod Aie4TileStrmSw32b =
{
	.SlvConfigBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0,
	.MstrConfigBaseAddr = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.DetMergeFeature = XAIE_FEATURE_UNAVAILABLE,
	.MstrEn = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0_MANAGER_ENABLE_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0_MANAGER_ENABLE_MASK},
	.MstrPktEn = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0_PACKET_ENABLE_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0_DROP_HEADER_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0_DROP_HEADER_MASK},
	.Config = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0_CONFIGURATION_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0_CONFIGURATION_MASK},
	.SlvEn = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0_SUBORDINATE_ENABLE_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0_SUBORDINATE_ENABLE_MASK},
	.SlvPktEn = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0_PACKET_ENABLE_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ID_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ID_MASK},
	.SlotMask = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_MASK_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_MASK_MASK},
	.SlotEn = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ENABLE_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_MSEL_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ARBIT_LSB, XAIE4GBL_CORE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ARBIT_MASK},
	.MstrConfig = Aie4TileStrmMstr32b,
	.SlvConfig = Aie4TileStrmSlv32b,
	.SlvSlotConfig = Aie4AieTileStrmSlaveSlot32b,
	.MaxMasterPhyPortId = 6U,
	.MaxSlavePhyPortId = 9U,
	.MasterPortMap = Aie4TileStrmSw32bMasterPortMap,
	.SlavePortMap = Aie4TileStrmSw32bSlavePortMap,
	.DetMerge = NULL,
	.PortVerify = _XAie4_StrmSw32bCheckPortValidity,
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_SHIM
 */
static const  XAie_StrmMod Aie4ShimStrmSw =
{
	.SlvConfigBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_DMA_0,
	.MstrConfigBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.DetMergeFeature = XAIE_FEATURE_AVAILABLE,
	.MstrEn = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0_MANAGER_ENABLE_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0_MANAGER_ENABLE_MASK},
	.MstrPktEn = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0_PACKET_ENABLE_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0_DROP_HEADER_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0_DROP_HEADER_MASK},
	.Config = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0_CONFIGURATION_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0_CONFIGURATION_MASK},
	.SlvEn = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_DMA_0_SUBORDINATE_ENABLE_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_DMA_0_SUBORDINATE_ENABLE_MASK},
	.SlvPktEn = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_DMA_0_PACKET_ENABLE_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_DMA_0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_ID_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_ID_MASK},
	.SlotMask = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_MASK_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_MASK_MASK},
	.SlotEn = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_ENABLE_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_MSEL_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_ARBIT_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_ARBIT_MASK},
	.MstrConfig = Aie4ShimStrmMstr,
	.SlvConfig = Aie4ShimStrmSlv,
	.SlvSlotConfig = Aie4ShimStrmSlaveSlot,
	.MaxMasterPhyPortId = 31U,
	.MaxSlavePhyPortId = 29U,
	.MasterPortMap = Aie4ShimStrmSwMasterPortMap,
	.SlavePortMap = Aie4ShimStrmSwSlavePortMap,
	.DetMerge = &Aie4ShimTileStrmSwDetMerge,
	.PortVerify = _XAie4_ShimTile_StrmSwCheckPortValidity,
};

/*
 * Data structure to capture all 32bit stream switch configs for XAIEGBL_TILE_TYPE_SHIM
 */
static const  XAie_StrmMod Aie4ShimStrmSw32b =
{
	.SlvConfigBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0,
	.MstrConfigBaseAddr = XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_UCONTROLLER_0,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.DetMergeFeature = XAIE_FEATURE_UNAVAILABLE,
	.MstrEn = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_UCONTROLLER_0_MANAGER_ENABLE_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_UCONTROLLER_0_MANAGER_ENABLE_MASK},
	.MstrPktEn = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_UCONTROLLER_0_PACKET_ENABLE_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_UCONTROLLER_0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_UCONTROLLER_0_DROP_HEADER_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_UCONTROLLER_0_DROP_HEADER_MASK},
	.Config = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_UCONTROLLER_0_CONFIGURATION_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_UCONTROLLER_0_CONFIGURATION_MASK},
	.SlvEn = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0_SUBORDINATE_ENABLE_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0_SUBORDINATE_ENABLE_MASK},
	.SlvPktEn = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0_PACKET_ENABLE_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ID_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ID_MASK},
	.SlotMask = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_MASK_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_MASK_MASK},
	.SlotEn = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ENABLE_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_MSEL_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ARBIT_LSB, XAIE4GBL_PL_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ARBIT_MASK},
	.MstrConfig = Aie4ShimStrmMstr32b,
	.SlvConfig = Aie4ShimStrmSlv32b,
	.SlvSlotConfig = Aie4ShimStrmSlaveSlot32b,
	.MaxMasterPhyPortId = 9U,
	.MaxSlavePhyPortId = 13U,
	.MasterPortMap = Aie4ShimStrmSwMasterPortMap32b,
	.SlavePortMap = Aie4ShimStrmSwSlavePortMap32b,
	.DetMerge = NULL,
	.PortVerify = _XAie4_StrmSw32bCheckPortValidity,
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_MEMTILE
 */
static const  XAie_StrmMod Aie4MemTileStrmSw =
{
        .SlvConfigBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_DMA_0,
        .MstrConfigBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0,
        .PortOffset = 0x4,
        .NumSlaveSlots = 4U,
        .SlotOffsetPerPort = 0x10,
        .SlotOffset = 0x4,
        .DetMergeFeature = XAIE_FEATURE_AVAILABLE,
        .MstrEn = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0_MANAGER_ENABLE_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0_MANAGER_ENABLE_MASK},
        .MstrPktEn = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0_PACKET_ENABLE_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0_PACKET_ENABLE_MASK},
        .DrpHdr = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0_DROP_HEADER_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0_DROP_HEADER_MASK},
        .Config = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0_CONFIGURATION_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_MANAGER_CONFIG_DMA_0_CONFIGURATION_MASK},
        .SlvEn = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_DMA_0_SUBORDINATE_ENABLE_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_DMA_0_SUBORDINATE_ENABLE_MASK},
        .SlvPktEn = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_DMA_0_PACKET_ENABLE_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_CONFIG_DMA_0_PACKET_ENABLE_MASK},
        .SlotPktId = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_ID_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_ID_MASK},
        .SlotMask = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_MASK_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_MASK_MASK},
        .SlotEn = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_ENABLE_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_ENABLE_MASK},
        .SlotMsel = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_MSEL_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_MSEL_MASK},
        .SlotArbitor = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_ARBIT_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_SUBORDINATE_DMA_0_SLOT0_ARBIT_MASK},
        .MstrConfig = Aie4MemTileStrmMstr,
        .SlvConfig = Aie4MemTileStrmSlv,
        .SlvSlotConfig = Aie4MemTileStrmSlaveSlot,
        .MaxMasterPhyPortId = 29U,
        .MaxSlavePhyPortId = 31U,
        .MasterPortMap = Aie4MemTileStrmSwMasterPortMap,
        .SlavePortMap = Aie4MemTileStrmSwSlavePortMap,
        .DetMerge = &Aie4MemTileStrmSwDetMerge,
        .PortVerify = _XAie4_MemTile_StrmSwCheckPortValidity,
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_MEMTILE
 * 32bit stream switch.
 */
static const  XAie_StrmMod Aie4MemTileStrmSw32b =
{
	.SlvConfigBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0,
	.MstrConfigBaseAddr = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.DetMergeFeature = XAIE_FEATURE_UNAVAILABLE,
	.MstrEn = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0_MANAGER_ENABLE_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0_MANAGER_ENABLE_MASK},
	.MstrPktEn = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0_PACKET_ENABLE_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0_DROP_HEADER_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0_DROP_HEADER_MASK},
	.Config = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0_CONFIGURATION_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_MANAGER_CONFIG_SOUTH_0_CONFIGURATION_MASK},
	.SlvEn = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0_SUBORDINATE_ENABLE_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0_SUBORDINATE_ENABLE_MASK},
	.SlvPktEn = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0_PACKET_ENABLE_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_CONFIG_TILE_CTRL_0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ID_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ID_MASK},
	.SlotMask = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_MASK_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_MASK_MASK},
	.SlotEn = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ENABLE_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_MSEL_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ARBIT_LSB, XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_32B_SUBORDINATE_TILE_CTRL_0_SLOT0_ARBIT_MASK},
	.MstrConfig = Aie4MemTileStrmMstr32b,
	.SlvConfig = Aie4MemTileStrmSlv32b,
	.SlvSlotConfig = Aie4MemTileStrmSlaveSlot32b,
	.MaxMasterPhyPortId = 7U,
	.MaxSlavePhyPortId = 11U,
	.MasterPortMap = Aie4MemTileStrmSw32bMasterPortMap,
	.SlavePortMap = Aie4MemTileStrmSw32bSlavePortMap,
	.DetMerge = NULL,
	.PortVerify = _XAie4_StrmSw32bCheckPortValidity,
};
#endif /* XAIE_FEATURE_SS_ENABLE */

#ifdef XAIE_FEATURE_PL_ENABLE
/* Register field attributes for PL interface down sizer for 32 and 64 bits */
static const  XAie_RegFldAttr Aie4DownSzr32_64Bit[] =
{
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH1_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH1_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH3_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH3_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH5_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH5_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH7_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH7_MASK}
};

/* Register field attributes for PL interface down sizer for 128 bits */
static const  XAie_RegFldAttr Aie4DownSzr128Bit[] =
{
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_SOUTH7_128_COMBINE_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_SOUTH7_128_COMBINE_MASK}
};

/* Register field attributes for PL interface up sizer */
static const  XAie_RegFldAttr Aie4UpSzr32_64Bit[] =
{
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH1_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH1_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH3_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH3_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH5_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH5_MASK}
};

/* Register field attributes for PL interface up sizer for 128 bits */
static const  XAie_RegFldAttr Aie4UpSzr128Bit[] =
{
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_MASK}
};

/* Register field attributes for PL interface down sizer bypass */
static const  XAie_RegFldAttr Aie4DownSzrByPass[] =
{
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH0_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH0_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH1_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH1_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH2_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH2_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH4_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH4_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH5_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH5_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH6_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH6_MASK}
};

/* Register field attributes for PL interface down sizer enable */
static const  XAie_RegFldAttr Aie4DownSzrEnable[] =
{
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH0_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH0_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH1_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH1_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH2_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH2_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH3_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH3_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH4_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH4_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH5_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH5_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH6_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH6_MASK},
	{XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH7_LSB, XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH7_MASK}
};


/* Register field attributes for SHIMNOC Mux configuration */
static const  XAie_RegFldAttr AieMlShimMuxConfig[] =
{
        {XAIEMLGBL_NOC_MODULE_MUX_CONFIG_SOUTH2_LSB, XAIEMLGBL_NOC_MODULE_MUX_CONFIG_SOUTH2_MASK},
        {XAIEMLGBL_NOC_MODULE_MUX_CONFIG_SOUTH3_LSB, XAIEMLGBL_NOC_MODULE_MUX_CONFIG_SOUTH3_MASK},
        {XAIEMLGBL_NOC_MODULE_MUX_CONFIG_SOUTH6_LSB, XAIEMLGBL_NOC_MODULE_MUX_CONFIG_SOUTH6_MASK},
        {XAIEMLGBL_NOC_MODULE_MUX_CONFIG_SOUTH7_LSB, XAIEMLGBL_NOC_MODULE_MUX_CONFIG_SOUTH7_MASK},
};

/* Register field attributes for SHIMNOC DeMux configuration */
static const  XAie_RegFldAttr AieMlShimDeMuxConfig[] =
{
        {XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH2_LSB, XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH2_MASK},
        {XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH3_LSB, XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH3_MASK},
        {XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH4_LSB, XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH4_MASK},
        {XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH5_LSB, XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH5_MASK}
};


#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE
/* Register to set SHIM clock buffer control */
static const XAie_ShimClkBufCntr Aie4ShimClkBufCntr =
{
	.RegOff = XAIE4GBL_PL_MODULE_COLUMN_CLOCK_CONTROL_PRIVILEGED,
	.RstEnable = XAIE_DISABLE,
	.ClkBufEnable = {XAIE4GBL_PL_MODULE_COLUMN_CLOCK_CONTROL_COLUMN_CLOCK_BUFFER_ENABLE_LSB_PRIVILEGED ,
			XAIE4GBL_PL_MODULE_COLUMN_CLOCK_CONTROL_COLUMN_CLOCK_BUFFER_ENABLE_MASK_PRIVILEGED}
};

/* Register to set Module_Clock_Control_0 */
static const XAie_ShimModClkCntr0 Aie4ShimModClkCntr0 =
{
	.RegOff = XAIE4GBL_PL_MODULE_MODULE_CLOCK_CONTROL_0_PRIVILEGED,
	.RstEnable = XAIE_DISABLE,
	.StrmSwClkEnable = {XAIE4GBL_PL_MODULE_MODULE_CLOCK_CONTROL_0_CTE_CLOCK_ENABLE_LSB_PRIVILEGED,
			XAIE4GBL_PL_MODULE_MODULE_CLOCK_CONTROL_0_CTE_CLOCK_ENABLE_MASK_PRIVILEGED},
	.PlIntClkEnable = {XAIE4GBL_PL_MODULE_MODULE_CLOCK_CONTROL_0_PL_INTERFACE_CLOCK_ENABLE_LSB_PRIVILEGED,
			XAIE4GBL_PL_MODULE_MODULE_CLOCK_CONTROL_0_PL_INTERFACE_CLOCK_ENABLE_MASK_PRIVILEGED},
	/* De-Featured in AIE4 */
	/*.CteClkEnable = {XAIE4GBL_PL_MODULE_MODULE_CLOCK_CONTROL_0_STREAM_SWITCH_CLOCK_ENABLE_LSB_PRIVILEGED,
			XAIE4GBL_PL_MODULE_MODULE_CLOCK_CONTROL_0_STREAM_SWITCH_CLOCK_ENABLE_MASK_PRIVILEGED}*/
};

/* Register to set Module_Clock_Control_1 */
static const XAie_ShimModClkCntr1 Aie4ShimModClkCntr1 =
{
	.RegOff = XAIE4GBL_PL_MODULE_MODULE_CLOCK_CONTROL_1_PRIVILEGED,
	.RstEnable = XAIE_DISABLE,
	.NocModClkEnable = {XAIE4GBL_PL_MODULE_MODULE_CLOCK_CONTROL_1_NOC_MODULE_CLOCK_ENABLE_LSB_PRIVILEGED,
		XAIE4GBL_PL_MODULE_MODULE_CLOCK_CONTROL_1_NOC_MODULE_CLOCK_ENABLE_MASK_PRIVILEGED}
};

static const XAie_ShimRstMod Aie4ShimTileRst =
{
	.RegOff = XAIE4GBL_PL_MODULE_APPLICATION_RESET_CONTROL_HW_CLK_STOP_PRIVILEGED,
	.RstCntr = {XAIE4GBL_PL_MODULE_APPLICATION_RESET_CONTROL_HW_CLK_STOP_APP_A_RESET_LSB_PRIVILEGED,XAIE4GBL_PL_MODULE_APPLICATION_RESET_CONTROL_HW_CLK_STOP_APP_A_RESET_MASK_PRIVILEGED},
	.RstCntr_B = {XAIE4GBL_PL_MODULE_APPLICATION_RESET_CONTROL_HW_CLK_STOP_APP_B_RESET_LSB_PRIVILEGED,XAIE4GBL_PL_MODULE_APPLICATION_RESET_CONTROL_HW_CLK_STOP_APP_B_RESET_MASK_PRIVILEGED},
	.RstShims = _XAieMl_RstShims,
};

/* Register feild attributes for Shim AXI MM config for NSU Errors */
static const XAie_ShimNocAxiMMConfig Aie4ShimNocAxiMMConfig =
{
	.RegOff = XAIE4GBL_NOC_MODULE_ME_AXIMM_CONFIG_PRIVILEGED,
	.NsuSlvErr = {XAIE4GBL_NOC_MODULE_ME_AXIMM_CONFIG_SLVERR_BLOCK_LSB_PRIVILEGED, XAIE4GBL_NOC_MODULE_ME_AXIMM_CONFIG_SLVERR_BLOCK_MASK_PRIVILEGED},
	.NsuDecErr = {XAIE4GBL_NOC_MODULE_ME_AXIMM_CONFIG_DECERR_BLOCK_LSB_PRIVILEGED, XAIE4GBL_NOC_MODULE_ME_AXIMM_CONFIG_DECERR_BLOCK_MASK_PRIVILEGED}
};
#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
#endif /* XAIE_FEATURE_PL_ENABLE */

#ifdef XAIE_FEATURE_CORE_ENABLE
/* Register field attribute for core process bus control */
static const XAie_RegCoreProcBusCtrl Aie4CoreProcBusCtrlReg =
{
	.RegOff = XAIE4GBL_CORE_MODULE_CORE_PROCESSOR_BUS,
	.CtrlEn = {XAIE4GBL_CORE_MODULE_CORE_PROCESSOR_BUS_ENABLE_LSB, XAIE4GBL_CORE_MODULE_CORE_PROCESSOR_BUS_ENABLE_MASK}
};

/* Core Module */
static const  XAie_CoreMod Aie4CoreMod =
{
	.IsCheckerBoard = 0U,
	.ProgMemAddr = 0x00000,
	.ProgMemSize = 32 * 1024,
	.DataMemAddr = 0x80000,
	.ProgMemHostOffset = XAIE4GBL_CORE_MODULE_PROGRAM_MEMORY,
	.DataMemSize = 128 * 1024,		/* AIE4 Tile Memory is 128kB */
	.DataMemShift = 16,
	.EccEvntRegOff = XAIE_FEATURE_UNAVAILABLE,
	.EccScubPeriodRegOff = XAIE4GBL_CORE_MODULE_ECC_SCRUBBING_PERIOD_PRIVILEGED,
	.CorePCOff = XAIE_FEATURE_UNAVAILABLE,
	.CoreSPOff = XAIE_FEATURE_UNAVAILABLE,
	.CoreLROff = XAIE_FEATURE_UNAVAILABLE,
	.CoreCtrl = &Aie4CoreCtrlReg,
	.CoreDebugStatus = &Aie4CoreDebugStatus,
	.CoreSts = &Aie4CoreStsReg,
	.CoreDebug = &Aie4CoreDebugReg,
	.CoreEvent = &Aie4CoreEventReg,
	.CoreAccumCtrl = &Aie4CoreAccumCtrlReg,
	.ProcBusCtrl = &Aie4CoreProcBusCtrlReg,
	.ConfigureDone = &_XAieMl_CoreConfigureDone,
	.Enable = &_XAieMl_CoreEnable,
	.WaitForDone = &_XAieMl_CoreWaitForDone,
	.ReadDoneBit = &_XAieMl_CoreReadDoneBit,
	.GetCoreStatus = &_XAieMl_CoreGetStatus
};

/* Core Module */
static const  XAie_CoreIntMod Aie4CoreIntMod =
{
    .ProgMemAddr = 0x00000,
	.CorePCOff = XAIE4GBL_CORE_INTERNAL_MODULE_CORE_PC,
	.CoreSPOff = XAIE4GBL_CORE_INTERNAL_MODULE_CORE_SP,
	.CoreLROff = XAIE4GBL_CORE_INTERNAL_MODULE_CORE_LR,
};

#endif /* XAIE_FEATURE_CORE_ENABLE */

#ifdef XAIE_FEATURE_DATAMEM_ENABLE

/* Data Memory Module for Tile data memory*/
static const  XAie_MemMod Aie4TileMemMod =
{
        .Size = 128 * 1024,			/* 0x20000,*/
        .MemAddr = XAIE4GBL_MEMORY_MODULE_DATAMEMORY,
        .EccEvntRegOff = XAIE_FEATURE_UNAVAILABLE,
	.EccScubPeriodRegOff = XAIE4GBL_MEMORY_MODULE_ECC_SCRUBBING_PERIOD_PRIVILEGED,
};

/* Data Memory Module for Mem Tile data memory*/
static const  XAie_MemMod Aie4MemTileMemMod =
{
	.Size =  4096 * 1024,				/* 0x400000, */
	.MemAddr = XAIE4GBL_MEM_TILE_MODULE_DATAMEMORY,
	.EccEvntRegOff = XAIE_FEATURE_UNAVAILABLE,
	.EccScubPeriodRegOff = XAIE4GBL_MEM_TILE_MODULE_ECC_SCRUBBING_PERIOD_PRIVILEGED,
};
#endif /* XAIE_FEATURE_DATAMEM_ENABLE */

#ifdef XAIE_FEATURE_PL_ENABLE
/* PL Only module for SHIMNOC Tiles */
static const  XAie_PlIfMod Aie4ShimTilePlIfMod =
{
	.UpSzrOff = XAIE4GBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG,
	.DownSzrOff = XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG,
	.DownSzrEnOff = XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE,
	.DownSzrByPassOff = XAIE4GBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS,
	.ColRstOff = 0xE00A0,
	.NumUpSzrPorts = 0x6,
	.MaxByPassPortNum = 0x6,
	.NumDownSzrPorts = 0x8,
	.UpSzr32_64Bit = Aie4UpSzr32_64Bit,
	.UpSzr128Bit = Aie4UpSzr128Bit,
	.DownSzr32_64Bit = Aie4DownSzr32_64Bit,
	.DownSzr128Bit = Aie4DownSzr128Bit,
	.DownSzrEn = Aie4DownSzrEnable,
	.DownSzrByPass = Aie4DownSzrByPass,
  .ColRst = {0, 0x3},
#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE
	.ClkBufCntr = &Aie4ShimClkBufCntr,
	.ShimTileRst = &Aie4ShimTileRst,
	.ShimNocAxiMM = &Aie4ShimNocAxiMMConfig,
	.ModClkCntr0 = &Aie4ShimModClkCntr0,
	.ModClkCntr1 = &Aie4ShimModClkCntr1,
#else
	.ClkBufCntr = NULL,
	.ShimTileRst = NULL,
	.ShimNocAxiMM = NULL,
	.ModClkCntr0 = NULL,
	.ModClkCntr1 = NULL,
#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
};

/* Noc module for SHIMNOC Tiles */
static const  XAie_NocMod Aie4ShimTileNocMod =
{
  .ShimNocMuxOff = XAIEMLGBL_NOC_MODULE_MUX_CONFIG,
  .ShimNocDeMuxOff = XAIEMLGBL_NOC_MODULE_DEMUX_CONFIG,
  .ShimNocMux = AieMlShimMuxConfig,
  .ShimNocDeMux = AieMlShimDeMuxConfig,
#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE
  .ShimNocAxiMM = &Aie4ShimNocAxiMMConfig, /* To be verified */
#else
  .ShimNocAxiMM = NULL,
#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
};
#endif /* XAIE_FEATURE_PL_ENABLE */

#ifdef XAIE_FEATURE_LOCK_ENABLE
static const XAie_RegFldAttr Aie4TileLockInit =
{
	.Lsb = XAIE4GBL_MEMORY_MODULE_LOCK0_VALUE_LOCK_VALUE_LSB,
	.Mask = XAIE4GBL_MEMORY_MODULE_LOCK0_VALUE_LOCK_VALUE_MASK,
};

/* Lock Module for AIE Tiles  */
static const  XAie_LockMod Aie4TileLockMod =
{
	.BaseAddr = XAIE4GBL_MEMORY_MODULE_LOCK_REQUEST,
	.NumLocks = 16U,
	.LockIdOff = 0x400,
	.RelAcqOff = 0x200,
	.LockValOff = 0x4,
	.LockValUpperBound = 15,
	.LockValLowerBound = -16,
	.LockSetValBase = XAIE4GBL_MEMORY_MODULE_LOCK0_VALUE,
	.LockSetValOff = 0x10,
	.LockInit = &Aie4TileLockInit,
	.Acquire = &_XAie4_LockAcquire,
	.Release = &_XAie4_LockRelease,
	.SetValue = &_XAie4_LockSetValue,
	.GetValue = &_XAie4_LockGetValue,
};

static const XAie_RegFldAttr Aie4ShimNocLockInit =
{
	.Lsb = XAIE4GBL_NOC_MODULE_LOCK0_VALUE_LOCK_VALUE_LSB,
	.Mask = XAIE4GBL_NOC_MODULE_LOCK0_VALUE_LOCK_VALUE_MASK,
};

/* Lock Module for SHIM NOC Tiles  */
static const  XAie_LockMod Aie4ShimNocLockMod =
{
	.BaseAddr = XAIE4GBL_NOC_MODULE_LOCK_REQUEST_A,
	.NumLocks = 16U,
	.LockIdOff = 0x400,
	.RelAcqOff = 0x200,
	.LockValOff = 0x4,
	.LockValUpperBound = 15,
	.LockValLowerBound = -16,
	.LockSetValBase = XAIE4GBL_NOC_MODULE_LOCK0_VALUE,
	.LockSetValOff = 0x10,
	.LockInit = &Aie4ShimNocLockInit,
	.Acquire = &_XAie4_LockAcquire,
	.Release = &_XAie4_LockRelease,
	.SetValue = &_XAie4_LockSetValue,
	.GetValue = &_XAie4_LockGetValue,
};

static const XAie_RegFldAttr Aie4MemTileLockInit =
{
	.Lsb = XAIE4GBL_MEM_TILE_MODULE_LOCK0_VALUE_LOCK_VALUE_LSB,
	.Mask = XAIE4GBL_MEM_TILE_MODULE_LOCK0_VALUE_LOCK_VALUE_MASK,
};

/* Lock Module for Mem Tiles  */
static const  XAie_LockMod Aie4MemTileLockMod =
{
	.BaseAddr = XAIE4GBL_MEM_TILE_MODULE_LOCK_REQUEST_A,
	.NumLocks = 32U,
	.LockIdOff = 0x400,
	.RelAcqOff = 0x200,
	.LockValOff = 0x4,
	.LockValUpperBound = 31,
	.LockValLowerBound = -32,
	.LockSetValBase = XAIE4GBL_MEM_TILE_MODULE_LOCK0_VALUE,
	.LockSetValOff = 0x10,
	.LockInit = &Aie4MemTileLockInit,
	.Acquire = &_XAie4_LockAcquire,
	.Release = &_XAie4_LockRelease,
	.SetValue = &_XAie4_LockSetValue,
	.GetValue = &_XAie4_LockGetValue,
};
#endif /* XAIE_FEATURE_LOCK_ENABLE */

#ifdef XAIE_FEATURE_EVENTS_ENABLE

/* Enum to event number mapping of all events of AIE4 Core Mod of aie tile */
static const u16 Aie4CoreModEventMapping[] =
{
		XAIE4_EVENTS_CORE_NONE,
        XAIE4_EVENTS_CORE_TRUE,
        XAIE4_EVENTS_CORE_GROUP_0,
        XAIE4_EVENTS_CORE_TIMER_SYNC,
        XAIE4_EVENTS_CORE_TIMER_VALUE_REACHED,
		XAIE4_EVENTS_CORE_PERF_CNT_0,
        XAIE4_EVENTS_CORE_PERF_CNT_1,
        XAIE4_EVENTS_CORE_PERF_CNT_2,
        XAIE4_EVENTS_CORE_PERF_CNT_3,
		XAIE4_EVENTS_CORE_PERF_CNT_4,
		XAIE4_EVENTS_CORE_PERF_CNT_5,
		XAIE4_EVENTS_CORE_PERF_CNT_6,
		XAIE4_EVENTS_CORE_PERF_CNT_7,
		XAIE4_EVENTS_CORE_PERF_CNT_8,
		XAIE4_EVENTS_CORE_PERF_CNT_9,
		XAIE4_EVENTS_CORE_PERF_CNT_10,
		XAIE4_EVENTS_CORE_PERF_CNT_11,
		XAIE4_EVENTS_CORE_COMBO_EVENT_0,
        XAIE4_EVENTS_CORE_COMBO_EVENT_1,
        XAIE4_EVENTS_CORE_COMBO_EVENT_2,
        XAIE4_EVENTS_CORE_COMBO_EVENT_3,
		XAIE4_EVENTS_CORE_COMBO_EVENT_4,
		XAIE4_EVENTS_CORE_COMBO_EVENT_5,
		XAIE4_EVENTS_CORE_COMBO_EVENT_6,
		XAIE4_EVENTS_CORE_COMBO_EVENT_7,
        XAIE4_EVENTS_CORE_GROUP_PC_EVENT,
        XAIE4_EVENTS_CORE_PC_0,
        XAIE4_EVENTS_CORE_PC_1,
        XAIE4_EVENTS_CORE_PC_2,
        XAIE4_EVENTS_CORE_PC_3,
		XAIE4_EVENTS_CORE_PC_4,
		XAIE4_EVENTS_CORE_PC_5,
		XAIE4_EVENTS_CORE_PC_6,
		XAIE4_EVENTS_CORE_PC_7,
		XAIE4_EVENTS_CORE_PC_8,
		XAIE4_EVENTS_CORE_PC_9,
        XAIE4_EVENTS_CORE_PC_RANGE_0_1,
        XAIE4_EVENTS_CORE_PC_RANGE_2_3,
		XAIE4_EVENTS_CORE_PC_RANGE_4_5,
		XAIE4_EVENTS_CORE_PC_RANGE_6_7,
		XAIE4_EVENTS_CORE_PC_RANGE_8_9,
        XAIE4_EVENTS_CORE_GROUP_STALL,
        XAIE4_EVENTS_CORE_MEMORY_STALL,
        XAIE4_EVENTS_CORE_STREAM_STALL,
        XAIE4_EVENTS_CORE_CASCADE_STALL,
        XAIE4_EVENTS_CORE_LOCK_STALL,
        XAIE4_EVENTS_CORE_DEBUG_HALTED,
        XAIE4_EVENTS_CORE_ACTIVE,
        XAIE4_EVENTS_CORE_DISABLED,
        XAIE4_EVENTS_CORE_ECC_ERROR_STALL,
        XAIE4_EVENTS_CORE_ECC_SCRUBBING_STALL,
        XAIE4_EVENTS_CORE_GROUP_PROGRAM_FLOW,
        XAIE4_EVENTS_CORE_INSTR_EVENT_0,
        XAIE4_EVENTS_CORE_INSTR_EVENT_1,
		XAIE4_EVENTS_CORE_INSTR_EVENT_2,
		XAIE4_EVENTS_CORE_INSTR_EVENT_3,
		XAIE4_EVENTS_CORE_INSTR_EVENT_4,
		XAIE4_EVENTS_CORE_INSTR_EVENT_5,
        XAIE4_EVENTS_CORE_INSTR_CALL,
        XAIE4_EVENTS_CORE_INSTR_RETURN,
        XAIE4_EVENTS_CORE_INSTR_VECTOR,
		XAIE4_EVENTS_CORE_INSTR_LOAD_A,
		XAIE4_EVENTS_CORE_INSTR_LOAD_B,
        XAIE4_EVENTS_CORE_INSTR_STORE,
        XAIE4_EVENTS_CORE_INSTR_STREAM_0_GET,
		XAIE4_EVENTS_CORE_INSTR_STREAM_1_GET,
        XAIE4_EVENTS_CORE_INSTR_STREAM_PUT,
        XAIE4_EVENTS_CORE_INSTR_CASCADE_GET,
        XAIE4_EVENTS_CORE_INSTR_CASCADE_PUT,
        XAIE4_EVENTS_CORE_INSTR_LOCK_ACQUIRE_REQ,
        XAIE4_EVENTS_CORE_INSTR_LOCK_RELEASE_REQ,
        XAIE4_EVENTS_CORE_GROUP_ERRORS_0,
        XAIE4_EVENTS_CORE_GROUP_ERRORS_1,
		XAIE4_EVENTS_CORE_GROUP_ERRORS_2,
        XAIE4_EVENTS_CORE_SRS_OVERFLOW,
        XAIE4_EVENTS_CORE_UPS_OVERFLOW,
        XAIE_EVENT_INVALID,
        XAIE_EVENT_INVALID,
        XAIE4_EVENTS_CORE_FP_INVALID,
        XAIE_EVENT_INVALID,
        XAIE_EVENT_INVALID,
		XAIE_EVENT_INVALID,
		XAIE_EVENT_INVALID,
        XAIE4_EVENTS_CORE_CONTROL_PKT_ERROR,
        XAIE4_EVENTS_CORE_AXI_MM_SLAVE_ERROR,
		XAIE_EVENT_INVALID,
        XAIE4_EVENTS_CORE_DM_ADDRESS_OUT_OF_RANGE,
        XAIE4_EVENTS_CORE_PM_ECC_ERROR_SCRUB_CORRECTED,
        XAIE4_EVENTS_CORE_PM_ECC_ERROR_SCRUB_2BIT,
        XAIE4_EVENTS_CORE_PM_ECC_ERROR_1BIT,
        XAIE4_EVENTS_CORE_PM_ECC_ERROR_2BIT,
        XAIE4_EVENTS_CORE_PM_ADDRESS_OUT_OF_RANGE,
        XAIE4_EVENTS_CORE_DM_ACCESS_TO_UNAVAILABLE,
        XAIE4_EVENTS_CORE_LOCK_ACCESS_TO_UNAVAILABLE,
        XAIE_EVENT_INVALID,
        XAIE_EVENT_INVALID,
        XAIE4_EVENTS_CORE_GROUP_STREAM_SWITCH,
        XAIE4_EVENTS_CORE_PORT_IDLE_0,
        XAIE4_EVENTS_CORE_PORT_RUNNING_0,
        XAIE4_EVENTS_CORE_PORT_STALLED_0,
        XAIE4_EVENTS_CORE_PORT_TLAST_0,
        XAIE4_EVENTS_CORE_PORT_IDLE_1,
        XAIE4_EVENTS_CORE_PORT_RUNNING_1,
        XAIE4_EVENTS_CORE_PORT_STALLED_1,
        XAIE4_EVENTS_CORE_PORT_TLAST_1,
        XAIE4_EVENTS_CORE_PORT_IDLE_2,
        XAIE4_EVENTS_CORE_PORT_RUNNING_2,
        XAIE4_EVENTS_CORE_PORT_STALLED_2,
        XAIE4_EVENTS_CORE_PORT_TLAST_2,
        XAIE4_EVENTS_CORE_PORT_IDLE_3,
        XAIE4_EVENTS_CORE_PORT_RUNNING_3,
        XAIE4_EVENTS_CORE_PORT_STALLED_3,
        XAIE4_EVENTS_CORE_PORT_TLAST_3,
        XAIE4_EVENTS_CORE_PORT_IDLE_4,
        XAIE4_EVENTS_CORE_PORT_RUNNING_4,
        XAIE4_EVENTS_CORE_PORT_STALLED_4,
        XAIE4_EVENTS_CORE_PORT_TLAST_4,
        XAIE4_EVENTS_CORE_PORT_IDLE_5,
        XAIE4_EVENTS_CORE_PORT_RUNNING_5,
        XAIE4_EVENTS_CORE_PORT_STALLED_5,
        XAIE4_EVENTS_CORE_PORT_TLAST_5,
        XAIE_EVENT_INVALID,
        XAIE_EVENT_INVALID,
        XAIE_EVENT_INVALID,
        XAIE_EVENT_INVALID,
        XAIE_EVENT_INVALID,
        XAIE_EVENT_INVALID,
        XAIE_EVENT_INVALID,
        XAIE_EVENT_INVALID,
        XAIE4_EVENTS_CORE_GROUP_BROADCAST,
        XAIE4_EVENTS_CORE_BROADCAST_0,
        XAIE4_EVENTS_CORE_BROADCAST_1,
        XAIE4_EVENTS_CORE_BROADCAST_2,
        XAIE4_EVENTS_CORE_BROADCAST_3,
        XAIE4_EVENTS_CORE_BROADCAST_4,
        XAIE4_EVENTS_CORE_BROADCAST_5,
        XAIE4_EVENTS_CORE_BROADCAST_6,
        XAIE4_EVENTS_CORE_BROADCAST_7,
        XAIE4_EVENTS_CORE_BROADCAST_8,
        XAIE4_EVENTS_CORE_BROADCAST_9,
        XAIE4_EVENTS_CORE_BROADCAST_10,
        XAIE4_EVENTS_CORE_BROADCAST_11,
        XAIE4_EVENTS_CORE_BROADCAST_12,
        XAIE4_EVENTS_CORE_BROADCAST_13,
        XAIE4_EVENTS_CORE_BROADCAST_14,
        XAIE4_EVENTS_CORE_BROADCAST_15,
        XAIE4_EVENTS_CORE_GROUP_USER_EVENT,
        XAIE4_EVENTS_CORE_USER_EVENT_0,
        XAIE4_EVENTS_CORE_USER_EVENT_1,
        XAIE4_EVENTS_CORE_USER_EVENT_2,
        XAIE4_EVENTS_CORE_USER_EVENT_3,
        XAIE4_EVENTS_CORE_USER_EVENT_4,
        XAIE4_EVENTS_CORE_USER_EVENT_5,
        XAIE4_EVENTS_CORE_USER_EVENT_6,
        XAIE4_EVENTS_CORE_USER_EVENT_7,
        XAIE4_EVENTS_CORE_EDGE_DETECTION_EVENT_0,
        XAIE4_EVENTS_CORE_EDGE_DETECTION_EVENT_1,
        XAIE4_EVENTS_CORE_EDGE_DETECTION_EVENT_2,
        XAIE4_EVENTS_CORE_EDGE_DETECTION_EVENT_3,
        XAIE4_EVENTS_CORE_EDGE_DETECTION_EVENT_4,
        XAIE4_EVENTS_CORE_EDGE_DETECTION_EVENT_5,
        XAIE4_EVENTS_CORE_EDGE_DETECTION_EVENT_6,
        XAIE4_EVENTS_CORE_EDGE_DETECTION_EVENT_7,
        XAIE4_EVENTS_CORE_FP_HUGE,
        XAIE4_EVENTS_CORE_INT_FP_TINY,
        XAIE4_EVENTS_CORE_FP_INF,
        XAIE4_EVENTS_CORE_INSTR_WARNING,
        XAIE4_EVENTS_CORE_INSTR_ERROR,
        XAIE_EVENT_INVALID,
        XAIE4_EVENTS_CORE_STREAM_SWITCH_PORT_PARITY_ERROR,
        XAIE4_EVENTS_CORE_PROCESSOR_BUS_ERROR,
		XAIE4_EVENTS_CORE_INSTR_MATRIX,
		XAIE4_EVENTS_CORE_INSTR_MOVE,
		XAIE4_EVENTS_CORE_INSTR_ALU,
		XAIE4_EVENTS_CORE_FATAL_ERROR,
		XAIE4_EVENTS_CORE_SPARSITY_OVERFLOW,
		XAIE4_EVENTS_CORE_DM_ECC_ERROR_SCRUB_CORRECTED,
		XAIE4_EVENTS_CORE_DM_ECC_ERROR_SCRUB_2BIT,
		XAIE4_EVENTS_CORE_DM_ECC_ERROR_1BIT, 
		XAIE4_EVENTS_CORE_DM_ECC_ERROR_2BIT,
		XAIE4_EVENTS_CORE_DM_PARITY_ERROR,
		XAIE4_EVENTS_CORE_DMA_ERROR,
		XAIE4_EVENTS_CORE_LOCK_ERROR,
		XAIE4_EVENTS_CORE_DMA_TASK_TOKEN_STALL,
		XAIE4_EVENTS_CORE_GROUP_WATCHPOINT,
		XAIE4_EVENTS_CORE_WATCHPOINT_0,
		XAIE4_EVENTS_CORE_WATCHPOINT_1,
		XAIE4_EVENTS_CORE_WATCHPOINT_2,
		XAIE4_EVENTS_CORE_GROUP_DMA_ACTIVITY,
		XAIE4_EVENTS_CORE_DMA_S2MM_0_START_BD,
		XAIE4_EVENTS_CORE_DMA_S2MM_1_START_BD,
		XAIE4_EVENTS_CORE_DMA_MM2S_0_START_BD,
		XAIE4_EVENTS_CORE_DMA_S2MM_0_START_TASK,
		XAIE4_EVENTS_CORE_DMA_S2MM_1_START_TASK,
		XAIE4_EVENTS_CORE_DMA_MM2S_0_START_TASK,
		XAIE4_EVENTS_CORE_DMA_S2MM_0_FINISHED_BD,
		XAIE4_EVENTS_CORE_DMA_S2MM_1_FINISHED_BD,
		XAIE4_EVENTS_CORE_DMA_MM2S_0_FINISHED_BD,
		XAIE4_EVENTS_CORE_DMA_S2MM_0_FINISHED_TASK,
		XAIE4_EVENTS_CORE_DMA_S2MM_1_FINISHED_TASK,
		XAIE4_EVENTS_CORE_DMA_MM2S_0_FINISHED_TASK,
		XAIE4_EVENTS_CORE_DMA_S2MM_0_STALLED_LOCK,
		XAIE4_EVENTS_CORE_DMA_S2MM_1_STALLED_LOCK,
		XAIE4_EVENTS_CORE_DMA_MM2S_0_STALLED_LOCK,
		XAIE4_EVENTS_CORE_DMA_S2MM_0_STREAM_STARVATION,
		XAIE4_EVENTS_CORE_DMA_S2MM_1_STREAM_STARVATION,
		XAIE4_EVENTS_CORE_DMA_MM2S_0_STREAM_BACKPRESSURE,
		XAIE4_EVENTS_CORE_DMA_S2MM_0_MEMORY_BACKPRESSURE,
		XAIE4_EVENTS_CORE_DMA_S2MM_1_MEMORY_BACKPRESSURE,
		XAIE4_EVENTS_CORE_DMA_MM2S_0_MEMORY_STARVATION,
		XAIE4_EVENTS_CORE_DMA_S2MM_0_RUNNING,
		XAIE4_EVENTS_CORE_DMA_S2MM_1_RUNNING,
		XAIE4_EVENTS_CORE_DMA_MM2S_0_RUNNING,
		XAIE4_EVENTS_CORE_GROUP_LOCK,
		XAIE4_EVENTS_CORE_LOCK_SEL0_ACQ_EQ,
		XAIE4_EVENTS_CORE_LOCK_SEL0_ACQ_GE,
		XAIE4_EVENTS_CORE_LOCK_SEL0_REL,
		XAIE4_EVENTS_CORE_LOCK_SEL0_EQUAL_TO_VALUE,
		XAIE4_EVENTS_CORE_LOCK_SEL1_ACQ_EQ,
		XAIE4_EVENTS_CORE_LOCK_SEL1_ACQ_GE,
		XAIE4_EVENTS_CORE_LOCK_SEL1_REL,
		XAIE4_EVENTS_CORE_LOCK_SEL1_EQUAL_TO_VALUE,
		XAIE4_EVENTS_CORE_LOCK_SEL2_ACQ_EQ,
		XAIE4_EVENTS_CORE_LOCK_SEL2_ACQ_GE,
		XAIE4_EVENTS_CORE_LOCK_SEL2_REL,
		XAIE4_EVENTS_CORE_LOCK_SEL2_EQUAL_TO_VALUE,
		XAIE4_EVENTS_CORE_LOCK_SEL3_ACQ_EQ,
		XAIE4_EVENTS_CORE_LOCK_SEL3_ACQ_GE,
		XAIE4_EVENTS_CORE_LOCK_SEL3_REL,
		XAIE4_EVENTS_CORE_LOCK_SEL3_EQUAL_TO_VALUE,
		XAIE4_EVENTS_CORE_LOCK_SEL4_ACQ_EQ,
		XAIE4_EVENTS_CORE_LOCK_SEL4_ACQ_GE,
		XAIE4_EVENTS_CORE_LOCK_SEL4_REL,
		XAIE4_EVENTS_CORE_LOCK_SEL4_EQUAL_TO_VALUE,
		XAIE4_EVENTS_CORE_LOCK_SEL5_ACQ_EQ,
		XAIE4_EVENTS_CORE_LOCK_SEL5_ACQ_GE,
		XAIE4_EVENTS_CORE_LOCK_SEL5_REL,
		XAIE4_EVENTS_CORE_LOCK_SEL5_EQUAL_TO_VALUE,
		XAIE4_EVENTS_CORE_GROUP_MEMORY_CONFLICT	,
		XAIE4_EVENTS_CORE_CONFLICT_DM_BANK_0,
		XAIE4_EVENTS_CORE_CONFLICT_DM_BANK_1,
		XAIE4_EVENTS_CORE_CONFLICT_DM_BANK_2,
		XAIE4_EVENTS_CORE_CONFLICT_DM_BANK_3,
		XAIE4_EVENTS_CORE_CONFLICT_DM_BANK_4,
		XAIE4_EVENTS_CORE_CONFLICT_DM_BANK_5,
		XAIE4_EVENTS_CORE_CONFLICT_DM_BANK_6,
		XAIE4_EVENTS_CORE_CONFLICT_DM_BANK_7,

};

/* Enum to event number mapping of all events of AIE4 PL Module */

static const u16 Aie4PlModEventMapping[] =
{
      XAIE4_EVENTS_PL_NONE,
      XAIE4_EVENTS_PL_TRUE,
      XAIE4_EVENTS_PL_GROUP_0,
      XAIE4_EVENTS_PL_TIMER_SYNC,
      XAIE4_EVENTS_PL_TIMER_VALUE_REACHED,
      XAIE4_EVENTS_PL_PERF_CNT_0,
      XAIE4_EVENTS_PL_PERF_CNT_1,
	  XAIE4_EVENTS_PL_PERF_CNT_2,
	  XAIE4_EVENTS_PL_PERF_CNT_3,
	  XAIE4_EVENTS_PL_PERF_CNT_4,
	  XAIE4_EVENTS_PL_PERF_CNT_5,
	  XAIE4_EVENTS_PL_PERF_CNT_6,
	  XAIE4_EVENTS_PL_PERF_CNT_7,
	  XAIE4_EVENTS_PL_PERF_CNT_8,
	  XAIE4_EVENTS_PL_PERF_CNT_9,
	  XAIE4_EVENTS_PL_PERF_CNT_10,
	  XAIE4_EVENTS_PL_PERF_CNT_11,
      XAIE4_EVENTS_PL_COMBO_EVENT_0,
      XAIE4_EVENTS_PL_COMBO_EVENT_1,
      XAIE4_EVENTS_PL_COMBO_EVENT_2,
      XAIE4_EVENTS_PL_COMBO_EVENT_3,
	  XAIE4_EVENTS_PL_COMBO_EVENT_4,
	  XAIE4_EVENTS_PL_COMBO_EVENT_5,
	  XAIE4_EVENTS_PL_COMBO_EVENT_6,
	  XAIE4_EVENTS_PL_COMBO_EVENT_7,
	  XAIE4_EVENTS_PL_GROUP_DMA_ACTIVITY,
      XAIE4_EVENTS_PL_DMA_S2MM_0_START_BD,
	  XAIE_EVENT_INVALID,
      XAIE4_EVENTS_PL_DMA_S2MM_2_START_BD,
      XAIE4_EVENTS_PL_DMA_MM2S_0_START_BD,
      XAIE4_EVENTS_PL_DMA_MM2S_1_START_BD,
      XAIE4_EVENTS_PL_DMA_MM2S_2_START_BD,
      XAIE4_EVENTS_PL_DMA_MM2S_3_START_BD,
      XAIE4_EVENTS_PL_DMA_S2MM_0_FINISHED_BD,
	  XAIE_EVENT_INVALID,
      XAIE4_EVENTS_PL_DMA_S2MM_2_FINISHED_BD,
      XAIE4_EVENTS_PL_DMA_MM2S_0_FINISHED_BD,
      XAIE4_EVENTS_PL_DMA_MM2S_1_FINISHED_BD,
      XAIE4_EVENTS_PL_DMA_MM2S_2_FINISHED_BD,
      XAIE4_EVENTS_PL_DMA_MM2S_3_FINISHED_BD,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
	  XAIE4_EVENTS_PL_GROUP_LOCK,
      XAIE_EVENT_INVALID,
	  XAIE4_EVENTS_PL_LOCK_0_REL,
      XAIE_EVENT_INVALID,
	  XAIE4_EVENTS_PL_LOCK_1_REL,
      XAIE_EVENT_INVALID,
	  XAIE4_EVENTS_PL_LOCK_2_REL,
      XAIE_EVENT_INVALID,
	  XAIE4_EVENTS_PL_LOCK_3_REL,
      XAIE_EVENT_INVALID,
	  XAIE4_EVENTS_PL_LOCK_4_REL,
      XAIE_EVENT_INVALID,
	  XAIE4_EVENTS_PL_LOCK_5_REL,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
      XAIE4_EVENTS_PL_GROUP_ERRORS,
	  XAIE_EVENT_INVALID,
      XAIE4_EVENTS_PL_CONTROL_PKT_ERROR,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_GROUP_ERRORS,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_IDLE_0,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_RUNNING_0,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_STALLED_0,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_TLAST_0,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_IDLE_1,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_RUNNING_1,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_STALLED_1,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_TLAST_1,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_IDLE_2,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_RUNNING_2,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_STALLED_2,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_TLAST_2,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_IDLE_3,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_RUNNING_3,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_STALLED_3,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_TLAST_3,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_IDLE_4,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_RUNNING_4,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_STALLED_4,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_TLAST_4,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_IDLE_5,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_RUNNING_5,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_STALLED_5,
	  XAIE4_EVENTS_PL_STREAM_SWITCH_PORT_TLAST_5,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE4_EVENTS_PL_GROUP_BROADCAST,
	  XAIE4_EVENTS_PL_BROADCAST_0,
      XAIE4_EVENTS_PL_BROADCAST_1,
      XAIE4_EVENTS_PL_BROADCAST_2,
      XAIE4_EVENTS_PL_BROADCAST_3,
      XAIE4_EVENTS_PL_BROADCAST_4,
      XAIE4_EVENTS_PL_BROADCAST_5,
      XAIE4_EVENTS_PL_BROADCAST_6,
      XAIE4_EVENTS_PL_BROADCAST_7,
      XAIE4_EVENTS_PL_BROADCAST_8,
      XAIE4_EVENTS_PL_BROADCAST_9,
      XAIE4_EVENTS_PL_BROADCAST_10,
      XAIE4_EVENTS_PL_BROADCAST_11,
      XAIE4_EVENTS_PL_BROADCAST_12,
      XAIE4_EVENTS_PL_BROADCAST_13,
      XAIE4_EVENTS_PL_BROADCAST_14,
      XAIE4_EVENTS_PL_BROADCAST_15,
      XAIE_EVENT_INVALID,
      XAIE4_EVENTS_PL_USER_EVENT_0,
      XAIE4_EVENTS_PL_USER_EVENT_1,
	  XAIE4_EVENTS_PL_USER_EVENT_2,
	  XAIE4_EVENTS_PL_USER_EVENT_3,
	  XAIE4_EVENTS_PL_USER_EVENT_4,
	  XAIE4_EVENTS_PL_USER_EVENT_5,
	  XAIE4_EVENTS_PL_USER_EVENT_6,
	  XAIE4_EVENTS_PL_USER_EVENT_7,
      XAIE4_EVENTS_PL_EDGE_DETECTION_EVENT_0,
      XAIE4_EVENTS_PL_EDGE_DETECTION_EVENT_1,
	  XAIE4_EVENTS_PL_EDGE_DETECTION_EVENT_2,
	  XAIE4_EVENTS_PL_EDGE_DETECTION_EVENT_3,
	  XAIE4_EVENTS_PL_EDGE_DETECTION_EVENT_4,
	  XAIE4_EVENTS_PL_EDGE_DETECTION_EVENT_5,
	  XAIE4_EVENTS_PL_EDGE_DETECTION_EVENT_6,
	  XAIE4_EVENTS_PL_EDGE_DETECTION_EVENT_7,
      XAIE4_EVENTS_PL_DMA_S2MM_0_START_TASK,
	  XAIE_EVENT_INVALID,
      XAIE4_EVENTS_PL_DMA_S2MM_2_START_TASK,
      XAIE4_EVENTS_PL_DMA_MM2S_0_START_TASK,
      XAIE4_EVENTS_PL_DMA_MM2S_1_START_TASK,
      XAIE4_EVENTS_PL_DMA_MM2S_2_START_TASK,
      XAIE4_EVENTS_PL_DMA_MM2S_3_START_TASK,
      XAIE4_EVENTS_PL_DMA_S2MM_0_FINISHED_TASK,
	  XAIE_EVENT_INVALID,
      XAIE4_EVENTS_PL_DMA_S2MM_2_FINISHED_TASK,
      XAIE4_EVENTS_PL_DMA_MM2S_0_FINISHED_TASK,
      XAIE4_EVENTS_PL_DMA_MM2S_1_FINISHED_TASK,
      XAIE4_EVENTS_PL_DMA_MM2S_2_FINISHED_TASK,
      XAIE4_EVENTS_PL_DMA_MM2S_3_FINISHED_TASK,
      XAIE4_EVENTS_PL_DMA_S2MM_0_STALLED_LOCK,
	  XAIE_EVENT_INVALID,
      XAIE4_EVENTS_PL_DMA_S2MM_2_STALLED_LOCK,
      XAIE4_EVENTS_PL_DMA_MM2S_0_STALLED_LOCK,
      XAIE4_EVENTS_PL_DMA_MM2S_1_STALLED_LOCK,
      XAIE4_EVENTS_PL_DMA_MM2S_2_STALLED_LOCK,
      XAIE4_EVENTS_PL_DMA_MM2S_3_STALLED_LOCK,
      XAIE4_EVENTS_PL_DMA_S2MM_0_STREAM_STARVATION,
	  XAIE_EVENT_INVALID,
      XAIE4_EVENTS_PL_DMA_S2MM_2_STREAM_STARVATION,
      XAIE4_EVENTS_PL_DMA_MM2S_0_STREAM_BACKPRESSURE,
      XAIE4_EVENTS_PL_DMA_MM2S_1_STREAM_BACKPRESSURE,
      XAIE4_EVENTS_PL_DMA_MM2S_2_STREAM_BACKPRESSURE,
      XAIE4_EVENTS_PL_DMA_MM2S_3_STREAM_BACKPRESSURE,
      XAIE4_EVENTS_PL_DMA_S2MM_0_MEMORY_BACKPRESSURE,
	  XAIE_EVENT_INVALID,
      XAIE4_EVENTS_PL_DMA_S2MM_2_MEMORY_BACKPRESSURE,
	  XAIE4_EVENTS_PL_DMA_MM2S_0_MEMORY_STARVATION,
	  XAIE4_EVENTS_PL_DMA_MM2S_1_MEMORY_STARVATION,
	  XAIE4_EVENTS_PL_DMA_MM2S_2_MEMORY_STARVATION,
	  XAIE4_EVENTS_PL_DMA_MM2S_3_MEMORY_STARVATION,
	  XAIE4_EVENTS_PL_DMA_MM2S_0_RUNNING,
	  XAIE_EVENT_INVALID,
	  XAIE4_EVENTS_PL_DMA_MM2S_2_RUNNING,
	  XAIE4_EVENTS_PL_DMA_S2MM_0_RUNNING,
	  XAIE4_EVENTS_PL_DMA_S2MM_1_RUNNING,
	  XAIE4_EVENTS_PL_DMA_S2MM_2_RUNNING,
	  XAIE4_EVENTS_PL_DMA_S2MM_3_RUNNING,
	  XAIE4_EVENTS_PL_LOCK_0_ACQ_EQ,
	  XAIE4_EVENTS_PL_LOCK_0_ACQ_GE,
	  XAIE4_EVENTS_PL_LOCK_0_EQUAL_TO_VALUE,
	  XAIE4_EVENTS_PL_LOCK_1_ACQ_EQ,
	  XAIE4_EVENTS_PL_LOCK_1_ACQ_GE,
	  XAIE4_EVENTS_PL_LOCK_1_EQUAL_TO_VALUE,
	  XAIE4_EVENTS_PL_LOCK_2_ACQ_EQ,
	  XAIE4_EVENTS_PL_LOCK_2_ACQ_GE,
	  XAIE4_EVENTS_PL_LOCK_2_EQUAL_TO_VALUE,
	  XAIE4_EVENTS_PL_LOCK_3_ACQ_EQ,
	  XAIE4_EVENTS_PL_LOCK_3_ACQ_GE,
	  XAIE4_EVENTS_PL_LOCK_3_EQUAL_TO_VALUE,
	  XAIE4_EVENTS_PL_LOCK_4_ACQ_EQ,
	  XAIE4_EVENTS_PL_LOCK_4_ACQ_GE,
	  XAIE4_EVENTS_PL_LOCK_4_EQUAL_TO_VALUE,
	  XAIE4_EVENTS_PL_LOCK_5_ACQ_EQ,
	  XAIE4_EVENTS_PL_LOCK_5_ACQ_GE,
	  XAIE4_EVENTS_PL_LOCK_5_EQUAL_TO_VALUE,
      XAIE4_EVENTS_PL_STREAM_SWITCH_PARITY_ERROR,
      XAIE_EVENT_INVALID,
      XAIE_EVENT_INVALID,
	  XAIE4_EVENTS_PL_LOCK_ERROR,
	  XAIE4_EVENTS_PL_DMA_TASK_TOKEN_STALL,
	  XAIE4_EVENTS_PL_AXI_MM_SUBORDINATE_TILE_ERROR,
	  XAIE4_EVENTS_PL_NSU_ERROR,
	  XAIE4_EVENTS_PL_DMA_ERROR,
	  XAIE4_EVENTS_PL_DMA_HW_ERROR,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE4_EVENTS_UC_DMA_GROUP_ACTIVITY,
	  XAIE4_EVENTS_UC_DMA_DM2MM_A_START_BD,
	  XAIE4_EVENTS_UC_DMA_DM2MM_B_START_BD,
	  XAIE4_EVENTS_UC_DMA_MM2DM_A_START_BD,
	  XAIE4_EVENTS_UC_DMA_MM2DM_B_START_BD,
	  XAIE4_EVENTS_UC_DMA_DM2MM_A_START_TASK,
	  XAIE4_EVENTS_UC_DMA_DM2MM_B_START_TASK,
	  XAIE4_EVENTS_UC_DMA_MM2DM_A_START_TASK,
	  XAIE4_EVENTS_UC_DMA_MM2DM_B_START_TASK,
	  XAIE4_EVENTS_UC_DMA_DM2MM_A_FINISHED_BD,
	  XAIE4_EVENTS_UC_DMA_DM2MM_B_FINISHED_BD,
	  XAIE4_EVENTS_UC_DMA_MM2DM_A_FINISHED_BD,
	  XAIE4_EVENTS_UC_DMA_MM2DM_B_FINISHED_BD,
	  XAIE4_EVENTS_UC_DMA_DM2MM_A_FINISHED_TASK,
	  XAIE4_EVENTS_UC_DMA_DM2MM_B_FINISHED_TASK,
	  XAIE4_EVENTS_UC_DMA_MM2DM_A_FINISHED_TASK,
	  XAIE4_EVENTS_UC_DMA_MM2DM_B_FINISHED_TASK,
	  XAIE4_EVENTS_UC_DMA_DM2MM_A_LOCAL_MEMORY_STARVATION,
	  XAIE4_EVENTS_UC_DMA_DM2MM_B_LOCAL_MEMORY_STARVATION,
	  XAIE4_EVENTS_UC_DMA_DM2MM_A_REMOTE_MEMORY_BACKPRESSURE,
	  XAIE4_EVENTS_UC_DMA_DM2MM_B_REMOTE_MEMORY_BACKPRESSURE,
	  XAIE4_EVENTS_UC_DMA_MM2DM_A_LOCAL_MEMORY_BACKPRESSURE,
	  XAIE4_EVENTS_UC_DMA_MM2DM_B_LOCAL_MEMORY_BACKPRESSURE,
	  XAIE4_EVENTS_UC_DMA_MM2DM_A_REMOTE_MEMORY_STARVATION,
	  XAIE4_EVENTS_UC_DMA_MM2DM_B_REMOTE_MEMORY_STARVATION,
	  XAIE4_EVENTS_UC_DMA_DM2MM_A_RUNNING,
	  XAIE4_EVENTS_UC_DMA_DM2MM_B_RUNNING,
	  XAIE4_EVENTS_UC_DMA_MM2DM_A_RUNNING,
	  XAIE4_EVENTS_UC_DMA_MM2DM_B_RUNNING,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE_EVENT_INVALID,
	  XAIE4_EVENTS_UC_CORE_PROGRAM_FLOW_GROUP_ERRORS,
	  XAIE4_EVENTS_UC_CORE_SLEEP,
	  XAIE4_EVENTS_UC_CORE_INTERRUPT,
	  XAIE4_EVENTS_UC_CORE_DEBUG_SYS_RESET,
	  XAIE4_EVENTS_UC_CORE_DEBUG_WAKEUP,
	  XAIE4_EVENTS_UC_CORE_TIMER1_INTERRUPT,
	  XAIE4_EVENTS_UC_CORE_TIMER2_INTERRUPT,
	  XAIE4_EVENTS_UC_CORE_TIMER3_INTERRUPT,
	  XAIE4_EVENTS_UC_CORE_TIMER4_INTERRUPT,
	  XAIE4_EVENTS_UC_CORE_REG_WRITE,
	  XAIE4_EVENTS_UC_CORE_EXCEPTION_TAKEN,
	  XAIE4_EVENTS_UC_CORE_JUMP_TAKEN,
	  XAIE4_EVENTS_UC_CORE_JUMP_HIT,
	  XAIE4_EVENTS_UC_CORE_DATA_READ,
	  XAIE4_EVENTS_UC_CORE_DATA_WRITE,
	  XAIE4_EVENTS_UC_CORE_PIPLINE_HALTED_DEBUG,
	  XAIE4_EVENTS_UC_CORE_STREAM_GET,
	  XAIE4_EVENTS_UC_CORE_STREAM_PUT,
	  XAIE4_EVENTS_UC_MODULE_A_ERROR,
	  XAIE4_EVENTS_UC_MODULE_B_ERROR,
	  XAIE4_EVENTS_UC_MODULE_A_AXI_MM_ERROR,
	  XAIE4_EVENTS_UC_MODULE_B_AXI_MM_ERROR,
	  XAIE4_EVENTS_UC_MODULE_A_ECC_ERROR_1BIT,
	  XAIE4_EVENTS_UC_MODULE_B_ECC_ERROR_1BIT,
	  XAIE4_EVENTS_UC_MODULE_A_ECC_ERROR_2BIT,
	  XAIE4_EVENTS_UC_MODULE_B_ECC_ERROR_2BIT,
	  XAIE4_EVENTS_UC_CORE_GROUP_PC_STATUS_EVENT,
	  XAIE4_EVENTS_UC_CORE_PC_0,
	  XAIE4_EVENTS_UC_CORE_PC_1,
	  XAIE4_EVENTS_UC_CORE_PC_2,
	  XAIE4_EVENTS_UC_CORE_PC_3,
	  XAIE4_EVENTS_UC_CORE_PC_4,
	  XAIE4_EVENTS_UC_CORE_PC_5,
	  XAIE4_EVENTS_UC_CORE_PC_RANGE_0_1,
	  XAIE4_EVENTS_UC_CORE_PC_RANGE_2_3,
	  XAIE4_EVENTS_UC_CORE_PC_RANGE_4_5,
	  XAIE4_EVENTS_UC_CORE_STATUS_0,
	  XAIE4_EVENTS_UC_CORE_STATUS_1,
	  XAIE4_EVENTS_UC_CORE_STATUS_2,
	  XAIE4_EVENTS_UC_CORE_STATUS_3,
	  XAIE4_EVENTS_UC_CORE_STATUS_4,
	  XAIE4_EVENTS_UC_CORE_STATUS_5,
	  XAIE4_EVENTS_UC_CORE_STATUS_6,
	  XAIE4_EVENTS_UC_CORE_STATUS_7,
};

/* Enum to event number mapping of all events of AIE4 Mem Tile Module */
static const u16 Aie4MemTileModEventMapping[] =
{
	XAIE4_EVENTS_MEM_TILE_NONE,
	XAIE4_EVENTS_MEM_TILE_TRUE,
	XAIE4_EVENTS_MEM_TILE_GROUP_0,
	XAIE4_EVENTS_MEM_TILE_TIMER_SYNC,
	XAIE4_EVENTS_MEM_TILE_TIMER_VALUE_REACHED,
	XAIE4_EVENTS_MEM_TILE_PERF_CNT0_EVENT,
	XAIE4_EVENTS_MEM_TILE_PERF_CNT1_EVENT,
	XAIE4_EVENTS_MEM_TILE_PERF_CNT2_EVENT,
	XAIE4_EVENTS_MEM_TILE_PERF_CNT3_EVENT,
	XAIE4_EVENTS_MEM_TILE_PERF_CNT4_EVENT,
	XAIE4_EVENTS_MEM_TILE_PERF_CNT5_EVENT,
	XAIE4_EVENTS_MEM_TILE_PERF_CNT6_EVENT,
	XAIE4_EVENTS_MEM_TILE_PERF_CNT7_EVENT,
	XAIE4_EVENTS_MEM_TILE_PERF_CNT8_EVENT,
	XAIE4_EVENTS_MEM_TILE_PERF_CNT9_EVENT,
	XAIE4_EVENTS_MEM_TILE_PERF_CNT10_EVENT,
	XAIE4_EVENTS_MEM_TILE_PERF_CNT11_EVENT,
	XAIE4_EVENTS_MEM_TILE_COMBO_EVENT_0,
	XAIE4_EVENTS_MEM_TILE_COMBO_EVENT_1,
	XAIE4_EVENTS_MEM_TILE_COMBO_EVENT_2,
	XAIE4_EVENTS_MEM_TILE_COMBO_EVENT_3,
	XAIE4_EVENTS_MEM_TILE_COMBO_EVENT_4,
	XAIE4_EVENTS_MEM_TILE_COMBO_EVENT_5,
	XAIE4_EVENTS_MEM_TILE_COMBO_EVENT_6,
	XAIE4_EVENTS_MEM_TILE_COMBO_EVENT_7,
	XAIE4_EVENTS_MEM_TILE_EDGE_DETECTION_EVENT_0,
	XAIE4_EVENTS_MEM_TILE_EDGE_DETECTION_EVENT_1,
	XAIE4_EVENTS_MEM_TILE_EDGE_DETECTION_EVENT_2,
	XAIE4_EVENTS_MEM_TILE_EDGE_DETECTION_EVENT_3,
	XAIE4_EVENTS_MEM_TILE_EDGE_DETECTION_EVENT_4,
	XAIE4_EVENTS_MEM_TILE_EDGE_DETECTION_EVENT_5,
	XAIE4_EVENTS_MEM_TILE_EDGE_DETECTION_EVENT_6,
	XAIE4_EVENTS_MEM_TILE_EDGE_DETECTION_EVENT_7,
	XAIE4_EVENTS_MEM_TILE_GROUP_WATCHPOINT,
	XAIE4_EVENTS_MEM_TILE_WATCHPOINT_0,
	XAIE4_EVENTS_MEM_TILE_WATCHPOINT_1,
	XAIE4_EVENTS_MEM_TILE_WATCHPOINT_2,
	XAIE4_EVENTS_MEM_TILE_WATCHPOINT_3,
	XAIE4_EVENTS_MEM_TILE_GROUP_DMA_ACTIVITY,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_SEL0_START_BD,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_SEL1_START_BD,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_SEL0_START_BD,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_SEL1_START_BD,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_SEL0_START_TASK,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_SEL1_START_TASK,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_SEL0_START_TASK,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_SEL1_START_TASK,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_SEL0_FINISHED_BD,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_SEL1_FINISHED_BD,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_SEL0_FINISHED_BD,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_SEL1_FINISHED_BD,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_SEL0_FINISHED_TASK,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_SEL1_FINISHED_TASK,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_SEL0_FINISHED_TASK,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_SEL1_FINISHED_TASK,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_SEL0_STALLED_LOCK,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_SEL1_STALLED_LOCK,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_SEL0_STALLED_LOCK,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_SEL1_STALLED_LOCK,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_SEL0_STREAM_STARVATION,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_SEL1_STREAM_STARVATION,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_SEL0_STREAM_BACKPRESSURE,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_SEL1_STREAM_BACKPRESSURE,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_SEL0_MEMORY_BACKPRESSURE,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_SEL1_MEMORY_BACKPRESSURE,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_SEL0_MEMORY_STARVATION,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_SEL1_MEMORY_STARVATION,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_SEL0_RUNNING,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_SEL1_RUNNING,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_SEL0_RUNNING,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_SEL1_RUNNING,
	XAIE4_EVENTS_MEM_TILE_GROUP_LOCK,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL0_ACQ_EQ,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL0_ACQ_GE,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL0_REL,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL0_EQUAL_TO_VALUE,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL1_ACQ_EQ,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL1_ACQ_GE,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL1_REL,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL1_EQUAL_TO_VALUE,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL2_ACQ_EQ,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL2_ACQ_GE,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL2_REL,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL2_EQUAL_TO_VALUE,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL3_ACQ_EQ,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL3_ACQ_GE,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL3_REL,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL3_EQUAL_TO_VALUE,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL4_ACQ_EQ,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL4_ACQ_GE,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL4_REL,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL4_EQUAL_TO_VALUE,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL5_ACQ_EQ,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL5_ACQ_GE,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL5_REL,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL5_EQUAL_TO_VALUE,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL6_ACQ_EQ,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL6_ACQ_GE,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL6_REL,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL6_EQUAL_TO_VALUE,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL7_ACQ_EQ,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL7_ACQ_GE,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL7_REL,
	XAIE4_EVENTS_MEM_TILE_LOCK_SEL7_EQUAL_TO_VALUE,
	XAIE4_EVENTS_MEM_TILE_GROUP_STREAM_SWITCH,
	XAIE4_EVENTS_MEM_TILE_PORT_IDLE_0,
	XAIE4_EVENTS_MEM_TILE_PORT_RUNNING_0,
	XAIE4_EVENTS_MEM_TILE_PORT_STALLED_0,
	XAIE4_EVENTS_MEM_TILE_PORT_TLAST_0,
	XAIE4_EVENTS_MEM_TILE_PORT_IDLE_1,
	XAIE4_EVENTS_MEM_TILE_PORT_RUNNING_1,
	XAIE4_EVENTS_MEM_TILE_PORT_STALLED_1,
	XAIE4_EVENTS_MEM_TILE_PORT_TLAST_1,
	XAIE4_EVENTS_MEM_TILE_PORT_IDLE_2,
	XAIE4_EVENTS_MEM_TILE_PORT_RUNNING_2,
	XAIE4_EVENTS_MEM_TILE_PORT_STALLED_2,
	XAIE4_EVENTS_MEM_TILE_PORT_TLAST_2,
	XAIE4_EVENTS_MEM_TILE_PORT_IDLE_3,
	XAIE4_EVENTS_MEM_TILE_PORT_RUNNING_3,
	XAIE4_EVENTS_MEM_TILE_PORT_STALLED_3,
	XAIE4_EVENTS_MEM_TILE_PORT_TLAST_3,
	XAIE4_EVENTS_MEM_TILE_PORT_IDLE_4,
	XAIE4_EVENTS_MEM_TILE_PORT_RUNNING_4,
	XAIE4_EVENTS_MEM_TILE_PORT_STALLED_4,
	XAIE4_EVENTS_MEM_TILE_PORT_TLAST_4,
	XAIE4_EVENTS_MEM_TILE_PORT_IDLE_5,
	XAIE4_EVENTS_MEM_TILE_PORT_RUNNING_5,
	XAIE4_EVENTS_MEM_TILE_PORT_STALLED_5,
	XAIE4_EVENTS_MEM_TILE_PORT_TLAST_5,
	XAIE4_EVENTS_MEM_TILE_PORT_IDLE_6,
	XAIE4_EVENTS_MEM_TILE_PORT_RUNNING_6,
	XAIE4_EVENTS_MEM_TILE_PORT_STALLED_6,
	XAIE4_EVENTS_MEM_TILE_PORT_TLAST_6,
	XAIE4_EVENTS_MEM_TILE_PORT_IDLE_7,
	XAIE4_EVENTS_MEM_TILE_PORT_RUNNING_7,
	XAIE4_EVENTS_MEM_TILE_PORT_STALLED_7,
	XAIE4_EVENTS_MEM_TILE_PORT_TLAST_7,
	XAIE4_EVENTS_MEM_TILE_GROUP_MEMORY_CONFLICT,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_0,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_1,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_2,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_3,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_4,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_5,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_6,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_7,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_8,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_9,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_10,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_11,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_12,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_13,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_14,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_15,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_16,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_17,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_18,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_19,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_20,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_21,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_22,
	XAIE4_EVENTS_MEM_TILE_CONFLICT_DM_BANK_23,
	XAIE4_EVENTS_MEM_TILE_GROUP_ERRORS,
	XAIE4_EVENTS_MEM_TILE_DM_ECC_ERROR_SCRUB_CORRECTED,
	XAIE4_EVENTS_MEM_TILE_DM_ECC_ERROR_SCRUB_2BIT,
	XAIE4_EVENTS_MEM_TILE_DM_ECC_ERROR_1BIT,
	XAIE4_EVENTS_MEM_TILE_DM_ECC_ERROR_2BIT,
	XAIE4_EVENTS_MEM_TILE_DMA_S2MM_ERROR,
	XAIE4_EVENTS_MEM_TILE_DMA_MM2S_ERROR,
	XAIE4_EVENTS_MEM_TILE_STREAM_SWITCH_PARITY_ERROR,
	XAIE_EVENT_INVALID,
	XAIE4_EVENTS_MEM_TILE_CONTROL_PKT_ERROR,
	XAIE4_EVENTS_MEM_TILE_AXI_MM_SLAVE_ERROR,
	XAIE4_EVENTS_MEM_TILE_LOCK_ERROR,
	XAIE4_EVENTS_MEM_TILE_DMA_TASK_TOKEN_STALL,
	XAIE4_EVENTS_MEM_TILE_GROUP_BROADCAST,
	XAIE4_EVENTS_MEM_TILE_BROADCAST_0,
	XAIE4_EVENTS_MEM_TILE_BROADCAST_1,
	XAIE4_EVENTS_MEM_TILE_BROADCAST_2,
	XAIE4_EVENTS_MEM_TILE_BROADCAST_3,
	XAIE4_EVENTS_MEM_TILE_BROADCAST_4,
	XAIE4_EVENTS_MEM_TILE_BROADCAST_5,
	XAIE4_EVENTS_MEM_TILE_BROADCAST_6,
	XAIE4_EVENTS_MEM_TILE_BROADCAST_7,
	XAIE4_EVENTS_MEM_TILE_BROADCAST_8,
	XAIE4_EVENTS_MEM_TILE_BROADCAST_9,
	XAIE4_EVENTS_MEM_TILE_BROADCAST_10,
	XAIE4_EVENTS_MEM_TILE_BROADCAST_11,
	XAIE4_EVENTS_MEM_TILE_BROADCAST_12,
	XAIE4_EVENTS_MEM_TILE_BROADCAST_13,
	XAIE4_EVENTS_MEM_TILE_BROADCAST_14,
	XAIE4_EVENTS_MEM_TILE_BROADCAST_15,
	XAIE4_EVENTS_MEM_TILE_GROUP_USER_EVENT,
	XAIE4_EVENTS_MEM_TILE_USER_EVENT_0,
	XAIE4_EVENTS_MEM_TILE_USER_EVENT_1,
	XAIE4_EVENTS_MEM_TILE_USER_EVENT_2,
	XAIE4_EVENTS_MEM_TILE_USER_EVENT_3,
	XAIE4_EVENTS_MEM_TILE_USER_EVENT_4,
	XAIE4_EVENTS_MEM_TILE_USER_EVENT_5,
	XAIE4_EVENTS_MEM_TILE_USER_EVENT_6,
	XAIE4_EVENTS_MEM_TILE_USER_EVENT_7,
};

#endif /* XAIE_FEATURE_EVENTS_ENABLE */

#ifdef XAIE_FEATURE_PERFCOUNT_ENABLE
/*
 * Data structure to capture registers & offsets for Core and memory Module of
 * performance counter.
 */
static const XAie_PerfMod Aie4TilePerfCnt[] =
{
        {       .MaxCounterVal = 0U,
                .StartStopShift = XAIE_FEATURE_UNAVAILABLE,
                .ResetShift = XAIE_FEATURE_UNAVAILABLE,
                .PerfCounterOffsetAdd = XAIE_FEATURE_UNAVAILABLE,
                .PerfCtrlBaseAddr = XAIE_FEATURE_UNAVAILABLE,
                .PerfCtrlOffsetAdd = XAIE_FEATURE_UNAVAILABLE,
				.PerfResetOffsetAdd = XAIE_FEATURE_UNAVAILABLE,
                .PerfCtrlResetBaseAddr = XAIE_FEATURE_UNAVAILABLE,
                .PerfCounterBaseAddr = XAIE_FEATURE_UNAVAILABLE,
                .PerfCounterEvtValBaseAddr = XAIE_FEATURE_UNAVAILABLE,
				.PerfCounterSsBaseAddr = XAIE_FEATURE_UNAVAILABLE,
				.PerfCounterSsLoadEvttBaseAddr = XAIE_FEATURE_UNAVAILABLE,
                {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
                {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
                {XAIE_FEATURE_UNAVAILABLE,XAIE_FEATURE_UNAVAILABLE},
        },
        {       .MaxCounterVal = 12U,
                .StartStopShift = 16U,
                .ResetShift = 8U,
                .PerfCounterOffsetAdd = 0X4,
                .PerfCtrlBaseAddr = XAIE4GBL_CORE_MODULE_PERFORMANCE_START_STOP_0_1,
                .PerfCtrlOffsetAdd = 0x4,
				.PerfResetOffsetAdd = 0x04,
                .PerfCtrlResetBaseAddr = XAIE4GBL_CORE_MODULE_PERFORMANCE_RESET_0_3,
                .PerfCounterBaseAddr = XAIE4GBL_CORE_MODULE_PERFORMANCE_COUNTER0,
                .PerfCounterEvtValBaseAddr = XAIE4GBL_CORE_MODULE_PERFORMANCE_COUNTER0_EVENT_VALUE,
				.PerfCounterSsBaseAddr = XAIE4GBL_CORE_MODULE_PERFORMANCE_COUNTER0_SNAPSHOT,
				.PerfCounterSsLoadEvttBaseAddr = XAIE4GBL_CORE_MODULE_PERFORMANCE_COUNTER_SNAPSHOTS_LOAD_EVENT,
                {XAIE4GBL_CORE_MODULE_PERFORMANCE_START_STOP_0_1_CNT0_START_EVENT_LSB, XAIE4GBL_CORE_MODULE_PERFORMANCE_START_STOP_0_1_CNT0_START_EVENT_MASK},
                {XAIE4GBL_CORE_MODULE_PERFORMANCE_START_STOP_0_1_CNT0_STOP_EVENT_LSB, XAIE4GBL_CORE_MODULE_PERFORMANCE_START_STOP_0_1_CNT0_STOP_EVENT_MASK},
                {XAIE4GBL_CORE_MODULE_PERFORMANCE_RESET_0_3_CNT0_RESET_EVENT_LSB, XAIE4GBL_CORE_MODULE_PERFORMANCE_RESET_0_3_CNT0_RESET_EVENT_MASK},
        }
};

/*
 * Data structure to capture registers & offsets for PL Module of performance
 * counter.
 */
static const XAie_PerfMod Aie4PlPerfCnt =
{
	.MaxCounterVal = 6U,
	.StartStopShift = 16U,
	.ResetShift = 8U,
	.PerfCounterOffsetAdd = 0x4,
	.PerfCtrlBaseAddr = XAIE4GBL_PL_MODULE_PERFORMANCE_START_STOP_0_1,
	.PerfCtrlOffsetAdd = 0x4,
	.PerfResetOffsetAdd = 0x4,
	.PerfCtrlResetBaseAddr = XAIE4GBL_PL_MODULE_PERFORMANCE_RESET_0_3,
	.PerfCounterBaseAddr = XAIE4GBL_PL_MODULE_PERFORMANCE_COUNTER0,
	.PerfCounterEvtValBaseAddr = XAIE4GBL_PL_MODULE_PERFORMANCE_COUNTER0_EVENT_VALUE,
	.PerfCounterSsBaseAddr = XAIE4GBL_PL_MODULE_PERFORMANCE_COUNTER0_SNAPSHOT,
	.PerfCounterSsLoadEvttBaseAddr = XAIE4GBL_PL_MODULE_PERFORMANCE_COUNTER_SNAPSHOTS_LOAD_EVENT_A,
	{XAIE4GBL_PL_MODULE_PERFORMANCE_START_STOP_0_1_CNT0_START_EVENT_LSB, XAIE4GBL_PL_MODULE_PERFORMANCE_START_STOP_0_1_CNT0_START_EVENT_MASK},
	{XAIE4GBL_PL_MODULE_PERFORMANCE_START_STOP_0_1_CNT0_STOP_EVENT_LSB, XAIE4GBL_PL_MODULE_PERFORMANCE_START_STOP_0_1_CNT0_STOP_EVENT_MASK},
	{XAIE4GBL_PL_MODULE_PERFORMANCE_RESET_0_3_CNT3_RESET_EVENT_LSB,XAIE4GBL_PL_MODULE_PERFORMANCE_RESET_0_3_CNT3_RESET_EVENT_MASK},
};

/*
 * Data structure to capture registers & offsets for Mem tile Module of
 * performance counter.
 */
static const XAie_PerfMod Aie4MemTilePerfCnt =
{
	.MaxCounterVal = 6U,
	.StartStopShift = 16U,
	.ResetShift = 8U,
	.PerfCounterOffsetAdd = 0X4,
	.PerfCtrlBaseAddr = XAIE4GBL_MEM_TILE_MODULE_PERFORMANCE_START_STOP_0_1,
	.PerfCtrlOffsetAdd = 0x4,
	.PerfResetOffsetAdd = 0x4,
	.PerfCtrlResetBaseAddr = XAIE4GBL_MEM_TILE_MODULE_PERFORMANCE_RESET_0_3,
	.PerfCounterBaseAddr = XAIE4GBL_MEM_TILE_MODULE_PERFORMANCE_COUNTER0,
	.PerfCounterEvtValBaseAddr = XAIE4GBL_MEM_TILE_MODULE_PERFORMANCE_COUNTER0_EVENT_VALUE,
	.PerfCounterSsBaseAddr = XAIE4GBL_MEM_TILE_MODULE_PERFORMANCE_COUNTER0_SNAPSHOT,
	.PerfCounterSsLoadEvttBaseAddr = XAIE4GBL_MEM_TILE_MODULE_PERFORMANCE_COUNTER_SNAPSHOTS_LOAD_EVENT_A,
	{XAIE4GBL_MEM_TILE_MODULE_PERFORMANCE_START_STOP_0_1_CNT0_START_EVENT_LSB, XAIE4GBL_MEM_TILE_MODULE_PERFORMANCE_START_STOP_0_1_CNT0_START_EVENT_MASK},
	{XAIE4GBL_MEM_TILE_MODULE_PERFORMANCE_START_STOP_0_1_CNT0_STOP_EVENT_LSB, XAIE4GBL_MEM_TILE_MODULE_PERFORMANCE_START_STOP_0_1_CNT0_STOP_EVENT_MASK},
	{XAIE4GBL_MEM_TILE_MODULE_PERFORMANCE_RESET_0_3_CNT3_RESET_EVENT_LSB, XAIE4GBL_MEM_TILE_MODULE_PERFORMANCE_RESET_0_3_CNT3_RESET_EVENT_MASK},
};
#endif /* XAIE_FEATURE_PERFCOUNT_ENABLE */

#ifdef XAIE_FEATURE_EVENTS_ENABLE

/* This structure need to be removed as there is no Trace module in Mem Module of Core Tile */
/*
static const XAie_EventGroup AieMlMemGroupEvent[] =
{
        {
                .GroupEvent = XAIE_EVENT_GROUP_0_MEM,
                .GroupOff = 0U,
                .GroupMask = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_0_ENABLE_MASK,
                .ResetValue = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_0_ENABLE_MASK,
        },
        {
                .GroupEvent = XAIE_EVENT_GROUP_WATCHPOINT_MEM,
                .GroupOff = 1U,
                .GroupMask = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_WATCHPOINT_ENABLE_MASK,
                .ResetValue = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_WATCHPOINT_ENABLE_MASK,
        },
        {
                .GroupEvent = XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM,
                .GroupOff = 2U,
                .GroupMask = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_DMA_ENABLE_MASK,
                .ResetValue = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_DMA_ENABLE_MASK,
        },
        {
                .GroupEvent = XAIE_EVENT_GROUP_LOCK_MEM,
                .GroupOff = 3U,
                .GroupMask = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_LOCK_ENABLE_MASK,
                .ResetValue = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_LOCK_ENABLE_MASK,
        },
        {
                .GroupEvent = XAIE_EVENT_GROUP_MEMORY_CONFLICT_MEM,
                .GroupOff = 4U,
                .GroupMask = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_MEMORY_CONFLICT_ENABLE_MASK,
                .ResetValue = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_MEMORY_CONFLICT_ENABLE_MASK,
        },
        {
                .GroupEvent = XAIE_EVENT_GROUP_ERRORS_MEM,
                .GroupOff = 5U,
                .GroupMask = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_ERROR_ENABLE_MASK,
                .ResetValue = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_ERROR_ENABLE_MASK,
        },
        {
                .GroupEvent = XAIE_EVENT_GROUP_BROADCAST_MEM,
                .GroupOff = 6U,
                .GroupMask = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_BROADCAST_ENABLE_MASK,
                .ResetValue = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_BROADCAST_ENABLE_MASK,
        },
        {
                .GroupEvent = XAIE_EVENT_GROUP_USER_EVENT_MEM,
                .GroupOff = 7U,
                .GroupMask = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_MASK,
                .ResetValue = XAIEMLGBL_MEMORY_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_MASK,
        },
};
*/


static const XAie_EventGroup Aie4CoreGroupEvent[] =
{
	{
		.GroupEvent = XAIE_EVENT_GROUP_0_CORE,
		.GroupOff = 0U,
		.GroupMask = XAIE4GBL_CORE_MODULE_EVENT_GROUP_0_ENABLE_MASK,
		.ResetValue = XAIE4GBL_CORE_MODULE_EVENT_GROUP_0_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_PC_EVENT_CORE,
		.GroupOff = 1U,
		.GroupMask = XAIE4GBL_CORE_MODULE_EVENT_GROUP_PC_ENABLE_MASK,
		.ResetValue = XAIE4GBL_CORE_MODULE_EVENT_GROUP_PC_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_CORE_STALL_CORE,
		.GroupOff = 2U,
		.GroupMask = XAIE4GBL_CORE_MODULE_EVENT_GROUP_CORE_STALL_ENABLE_MASK,
		.ResetValue = XAIE4GBL_CORE_MODULE_EVENT_GROUP_CORE_STALL_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE,
		.GroupOff = 3U,
		.GroupMask = XAIE4GBL_CORE_MODULE_EVENT_GROUP_CORE_PROGRAM_FLOW_ENABLE_MASK,
		.ResetValue = XAIE4GBL_CORE_MODULE_EVENT_GROUP_CORE_PROGRAM_FLOW_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_0_CORE,
		.GroupOff = 4U,
		.GroupMask = XAIE4GBL_CORE_MODULE_EVENT_GROUP_ERRORS0_ENABLE_MASK,
		.ResetValue = XAIE4GBL_CORE_MODULE_EVENT_GROUP_ERRORS0_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_1_CORE,
		.GroupOff = 5U,
		.GroupMask = XAIE4GBL_CORE_MODULE_EVENT_GROUP_ERRORS1_ENABLE_MASK,
		.ResetValue = XAIE4GBL_CORE_MODULE_EVENT_GROUP_ERRORS1_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_2_CORE,
		.GroupOff = 6U,
		.GroupMask = XAIE4GBL_CORE_MODULE_EVENT_GROUP_ERRORS2_ENABLE_MASK,
		.ResetValue = XAIE4GBL_CORE_MODULE_EVENT_GROUP_ERRORS2_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_STREAM_SWITCH_CORE,
		.GroupOff = 7U,
		.GroupMask = XAIE4GBL_CORE_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_MASK,
		.ResetValue = XAIE4GBL_CORE_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_WATCHPOINT_CORE,
		.GroupOff = 8U,
		.GroupMask = XAIE4GBL_CORE_MODULE_EVENT_GROUP_WATCHPOINT_MASK,
		.ResetValue = XAIE4GBL_CORE_MODULE_EVENT_GROUP_WATCHPOINT_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_DMA_ACTIVITY_CORE,
		.GroupOff = 9U,
		.GroupMask = XAIE4GBL_CORE_MODULE_EVENT_GROUP_DMA_ENABLE_MASK,
		.ResetValue = XAIE4GBL_CORE_MODULE_EVENT_GROUP_DMA_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_LOCK_CORE,
		.GroupOff = 10U,
		.GroupMask = XAIE4GBL_CORE_MODULE_EVENT_GROUP_LOCK_ENABLE_MASK,
		.ResetValue = XAIE4GBL_CORE_MODULE_EVENT_GROUP_LOCK_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_MEMORY_CONFLICT_CORE,
		.GroupOff = 11U,
		.GroupMask = XAIE4GBL_CORE_MODULE_EVENT_GROUP_MEMORY_CONFLICT_ENABLE_MASK,
		.ResetValue = XAIE4GBL_CORE_MODULE_EVENT_GROUP_MEMORY_CONFLICT_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_BROADCAST_CORE,
		.GroupOff = 12U,
		.GroupMask = XAIE4GBL_CORE_MODULE_EVENT_GROUP_BROADCAST_ENABLE_MASK,
		.ResetValue = XAIE4GBL_CORE_MODULE_EVENT_GROUP_BROADCAST_ENABLE_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_USER_EVENT_CORE,
		.GroupOff = 13U,
		.GroupMask = XAIE4GBL_CORE_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_MASK,
		.ResetValue = XAIE4GBL_CORE_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_MASK,
	},
};

static const XAie_EventGroup Aie4PlGroupEvent[] =
{
	{
		.GroupEvent = XAIE_EVENT_GROUP_0_PL,
		.GroupOff = 0U,
		.GroupMask = XAIE4GBL_PL_MODULE_EVENT_GROUP_0_ENABLE_A_MASK,
		.ResetValue = XAIE4GBL_PL_MODULE_EVENT_GROUP_0_ENABLE_A_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_DMA_ACTIVITY_PL,
		.GroupOff = 1U,
		.GroupMask = XAIE4GBL_PL_MODULE_EVENT_GROUP_DMA_ACTIVITY_ENABLE_0_A_MASK,
		.ResetValue = XAIE4GBL_PL_MODULE_EVENT_GROUP_DMA_ACTIVITY_ENABLE_0_A_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_UC_DMA_GROUP_ACTIVITY,
		.GroupOff = 3U,
		.GroupMask = XAIE4GBL_PL_MODULE_EVENT_GROUP_UC_DMA_ACTIVITY_ENABLE_A_MASK,
		.ResetValue = XAIE4GBL_PL_MODULE_EVENT_GROUP_UC_DMA_ACTIVITY_ENABLE_A_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_LOCK_PL,
		.GroupOff = 4U,
		.GroupMask = XAIE4GBL_PL_MODULE_EVENT_GROUP_LOCK_ENABLE_A_MASK,
		.ResetValue = XAIE4GBL_PL_MODULE_EVENT_GROUP_LOCK_ENABLE_A_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_ERRORS_PL,
		.GroupOff = 5U,
		.GroupMask = XAIE4GBL_PL_MODULE_EVENT_GROUP_ERRORS_ENABLE_A_MASK,
		.ResetValue = XAIE4GBL_PL_MODULE_EVENT_GROUP_ERRORS_ENABLE_A_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_STREAM_SWITCH_PL,
		.GroupOff = 6U,
		.GroupMask = XAIE4GBL_PL_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_A_MASK,
		.ResetValue = XAIE4GBL_PL_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_A_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_UC_CORE_PROGRAM_FLOW_GROUP_ERRORS,
		.GroupOff = 7U,
		.GroupMask = XAIE4GBL_PL_MODULE_EVENT_GROUP_UC_CORE_PROGRAM_FLOW_ENABLE_A_MASK,
		.ResetValue = XAIE4GBL_PL_MODULE_EVENT_GROUP_UC_CORE_PROGRAM_FLOW_ENABLE_A_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_UC_CORE_GROUP_PC_STATUS_EVENT,
		.GroupOff = 8U,
		.GroupMask = XAIE4GBL_PL_MODULE_EVENT_GROUP_UC_CORE_PC_STATUS_EVENT_ENABLE_A_MASK,
		.ResetValue = XAIE4GBL_PL_MODULE_EVENT_GROUP_UC_CORE_PC_STATUS_EVENT_ENABLE_A_MASK,
	},
	{
		.GroupEvent = XAIE_EVENT_GROUP_BROADCAST_A_PL,
		.GroupOff = 9U,
		.GroupMask = XAIE4GBL_PL_MODULE_EVENT_GROUP_BROADCAST_ENABLE_A_MASK,
		.ResetValue = XAIE4GBL_PL_MODULE_EVENT_GROUP_BROADCAST_ENABLE_A_MASK,
	},
};
static const XAie_EventGroup Aie4MemTileGroupEvent[] = {
    {
        .GroupEvent = XAIE_EVENT_GROUP_0_MEM_TILE,
        .GroupOff = 0U,
        .GroupMask = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_0_ENABLE_A_MASK,
        .ResetValue = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_0_ENABLE_A_MASK,
    },
    {
        .GroupEvent = XAIE_EVENT_GROUP_WATCHPOINT_MEM_TILE,
        .GroupOff = 1U,
        .GroupMask = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_WATCHPOINT_ENABLE_A_MASK,
        .ResetValue = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_WATCHPOINT_ENABLE_A_MASK,
    },
    {
        .GroupEvent = XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM_TILE,
        .GroupOff = 2U,
        .GroupMask = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_DMA_ENABLE_A_MASK,
        .ResetValue = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_DMA_ENABLE_A_MASK,
    },
    {
        .GroupEvent = XAIE_EVENT_GROUP_LOCK_MEM_TILE,
        .GroupOff = 3U,
        .GroupMask = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_LOCK_ENABLE_A_MASK,
        .ResetValue = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_LOCK_ENABLE_A_MASK,
    },
    {
        .GroupEvent = XAIE_EVENT_GROUP_STREAM_SWITCH_MEM_TILE,
        .GroupOff = 4U,
        .GroupMask = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_A_MASK,
        .ResetValue = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_STREAM_SWITCH_ENABLE_A_MASK,
    },
    {
        .GroupEvent = XAIE_EVENT_GROUP_MEMORY_CONFLICT_MEM_TILE,
        .GroupOff = 5U,
        .GroupMask = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_MEMORY_CONFLICT_ENABLE_A_MASK,
        .ResetValue = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_MEMORY_CONFLICT_ENABLE_A_MASK,
    },
    {
        .GroupEvent = XAIE_EVENT_GROUP_ERRORS_MEM_TILE,
        .GroupOff = 6U,
        .GroupMask = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_ERROR_ENABLE_A_MASK,
        .ResetValue = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_ERROR_ENABLE_A_MASK,
    },
    {
        .GroupEvent = XAIE_EVENT_GROUP_BROADCAST_MEM_TILE,
        .GroupOff = 7U,
        .GroupMask = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_BROADCAST_ENABLE_A_MASK,
        .ResetValue = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_BROADCAST_ENABLE_A_MASK,
    },
    {
        .GroupEvent = XAIE_EVENT_GROUP_USER_EVENT_MEM_TILE,
        .GroupOff = 8U,
        .GroupMask = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_A_MASK,
        .ResetValue = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_USER_EVENT_ENABLE_A_MASK,
    },
};

/* mapping of user events for core module */
static const XAie_EventMap Aie4TileCoreModUserEventMap =
{
	.RscId = 0U,
	.Event = XAIE_EVENT_USER_EVENT_0_CORE,
};

/* mapping of user events for memory module */
/* This structure need to be removed as there is no Trace module in Mem Module of Core Tile */
/*
static const XAie_EventMap AieMlTileMemModUserEventStart =
{
	.RscId = 0U,
	.Event = XAIE_EVENT_USER_EVENT_0_MEM,
};
*/

/* mapping of user events for mem tile memory module */
static const XAie_EventMap Aie4MemTileMemModUserEventStart =
{
	.RscId = 0U,
	.Event = XAIE_EVENT_USER_EVENT_0_MEM_TILE,
};

/* mapping of user events for memory module */
static const XAie_EventMap Aie4ShimTilePlModUserEventStart =
{
	.RscId = 0U,
	.Event = XAIE_EVENT_USER_EVENT_0_PL,
};

static const XAie_EventMap Aie4TileCoreModPCEventMap =
{
	.RscId = 0U,
	.Event = XAIE_EVENT_PC_0_CORE,
};

/* mapping of broadcast events for core module */
static const XAie_EventMap Aie4TileCoreModBroadcastEventMap =
{
	.RscId = 0U,
	.Event = XAIE_EVENT_BROADCAST_0_CORE,
};

/* mapping of broadcast events for memory module */
/* This structure need to be removed as there is no Trace module in Mem Module of Core Tile */
/*
static const XAie_EventMap AieMlTileMemModBroadcastEventStart =
{
	.RscId = 0U,
	.Event = XAIE_EVENT_BROADCAST_0_MEM,
};
*/

/* mapping of broadcast events for Pl module */
static const XAie_EventMap Aie4ShimTilePlModBroadcastEventStart =
{
	.RscId = 0U,
	.Event = XAIE_EVENT_BROADCAST_A_0_PL,
};

/* mapping of broadcast events for Mem tile mem module */
static const XAie_EventMap Aie4MemTileMemModBroadcastEventStart =
{
	.RscId = 0U,
	.Event = XAIE_EVENT_BROADCAST_0_MEM_TILE,
};

/*
 * Data structure to capture core and memory module events properties.
 * For memory module default error group mask enables,
 *	DM_ECC_Error_Scrub_2bit,
 *	DM_ECC_Error_2bit,
 *	DM_Parity_Error_Bank_2,
 *	DM_Parity_Error_Bank_3,
 *	DM_Parity_Error_Bank_4,
 *	DM_Parity_Error_Bank_5,
 *	DM_Parity_Error_Bank_6,
 *	DM_Parity_Error_Bank_7,
 *	DMA_0_S2MM_0_Error,
 *	DMA_0_S2MM_1_Error,
 *	DMA_0_MM2S_0_Error,
 *	DMA_0_MM2S_1_Error,
 *	Lock_Error.
 * For core module default error group mask enables,
 *	PM_Reg_Access_Failure,
 *	Stream_Pkt_Parity_Error,
 *	Control_Pkt_Error,
 *	AXI_MM_Slave_Error,
 *	Instruction_Decompression_Error,
 *	DM_address_out_of_range,
 *	PM_ECC_Error_Scrub_2bit,
 *	PM_ECC_Error_2bit,
 *	PM_address_out_of_range,
 *	DM_access_to_Unavailable,
 *	Lock_Access_to_Unavailable,
 *	Decompression_underflow,
 *	Stream_Switch_Port_Parity_Error,
 *	Processor_Bus_Error.
 */
static const XAie_EvntMod Aie4TileEvntMod[] =
{
	{
		.XAie_EventNumber = XAIE_FEATURE_UNAVAILABLE,
		.NumEventReg = 0U,
		.EventMin = 0U,
		.EventMax = 0U,
		.ComboEventBase = XAIE_FEATURE_UNAVAILABLE,
		.PerfCntEventBase = XAIE_FEATURE_UNAVAILABLE,
		.UserEventBase = XAIE_FEATURE_UNAVAILABLE,
		.PortIdleEventBase = XAIE_FEATURE_UNAVAILABLE,
		.GenEventRegOff = XAIE_FEATURE_UNAVAILABLE,
		.GenEvent = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
		.ComboInputRegOff = XAIE_FEATURE_UNAVAILABLE,
		.ComboEventMask = XAIE_FEATURE_UNAVAILABLE,
		.ComboEventOff = 0U,
		.ComboCtrlRegOff = XAIE_FEATURE_UNAVAILABLE,
		.ComboConfigMask = XAIE_FEATURE_UNAVAILABLE,
		.ComboConfigOff = 0U,
		.BaseStrmPortSelectRegOff = XAIE_FEATURE_UNAVAILABLE,
		.NumStrmPortSelectIds = XAIE_FEATURE_UNAVAILABLE,
		.StrmPortSelectIdsPerReg = XAIE_FEATURE_UNAVAILABLE,
		.PortIdMask = XAIE_FEATURE_UNAVAILABLE,
		.PortIdOff = XAIE_FEATURE_UNAVAILABLE,
		.PortMstrSlvMask = XAIE_FEATURE_UNAVAILABLE,
		.PortMstrSlvOff = XAIE_FEATURE_UNAVAILABLE,
		.Port32b512bMask = XAIE_FEATURE_UNAVAILABLE,
		.Port32b512bOff = XAIE_FEATURE_UNAVAILABLE,
		.BaseDmaChannelSelectRegOff = XAIE_FEATURE_UNAVAILABLE,
		.NumDmaChannelSelectIds = XAIE_FEATURE_UNAVAILABLE,
		.DmaChannelIdOff = XAIE_FEATURE_UNAVAILABLE,
		.DmaChannelIdMask = XAIE_FEATURE_UNAVAILABLE,
		.DmaChannelMM2SOff = XAIE_FEATURE_UNAVAILABLE,
		.EdgeEventRegOff = XAIE_FEATURE_UNAVAILABLE,
		.EdgeDetectEvent = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
		.EdgeDetectTrigger = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE | XAIE_FEATURE_UNAVAILABLE},
		.EdgeEventSelectIdOff = XAIE_FEATURE_UNAVAILABLE,
		.NumEdgeSelectIds = 0U,
		.BaseBroadcastRegOff = XAIE_FEATURE_UNAVAILABLE,
		.NumBroadcastIds = 0U,
		.BaseBroadcastSwBlockRegOff = XAIE_FEATURE_UNAVAILABLE,
		.BaseBroadcastSwUnblockRegOff = XAIE_FEATURE_UNAVAILABLE,
		.BroadcastSwOff = 0U,
		.BroadcastSwBlockOff = 0U,
		.BroadcastSwUnblockOff = 0U,
		.NumSwitches = 0U,
		.BaseGroupEventRegOff = XAIE_FEATURE_UNAVAILABLE,
		.NumGroupEvents = 0U,
		.DefaultGroupErrorMask = 0x0,
		.Group = NULL,
		.BasePCEventRegOff = XAIE_FEATURE_UNAVAILABLE,
		.NumPCEvents = XAIE_FEATURE_UNAVAILABLE,
		.PCAddr = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
		.PCValid = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
		.BaseStatusRegOff = XAIE_FEATURE_UNAVAILABLE,
		.NumUserEvents = 0U,
		.UserEventMap = NULL,
		.PCEventMap = NULL,
		.BroadcastEventMap = NULL,
		.ErrorHaltRegOff = XAIE_FEATURE_UNAVAILABLE,
	},
	{
		.XAie_EventNumber = Aie4CoreModEventMapping,
		.NumEventReg = 8U,
		.EventMin = (u32)XAIE_EVENT_NONE_CORE,
		.EventMax = (u32)XAIE_EVENT_CONFLICT_DM_BANK_7_CORE,
		.ComboEventBase = (u32)XAIE_EVENT_COMBO_EVENT_0_CORE,
		.PerfCntEventBase = (u32)XAIE_EVENT_PERF_CNT_0_CORE,
		.UserEventBase = (u32)XAIE_EVENT_USER_EVENT_0_CORE,
		.PortIdleEventBase = (u32)XAIE_EVENT_PORT_IDLE_0_CORE,
		.GenEventRegOff = XAIE4GBL_CORE_MODULE_EVENT_GENERATE,
		.GenEvent = {XAIE4GBL_CORE_MODULE_EVENT_GENERATE_EVENT_LSB, XAIE4GBL_CORE_MODULE_EVENT_GENERATE_EVENT_MASK},
		.ComboInputRegOff = XAIE4GBL_CORE_MODULE_COMBO_EVENT_INPUTS_A_D,
		.ComboEventMask = XAIE4GBL_CORE_MODULE_COMBO_EVENT_INPUTS_A_D_EVENTA_MASK,
		.ComboEventOff = 8U,
		.ComboCtrlRegOff = XAIE4GBL_CORE_MODULE_COMBO_EVENT_CONTROL_0_2,
		.ComboConfigMask = XAIE4GBL_CORE_MODULE_COMBO_EVENT_CONTROL_0_2_COMBO0_MASK,
		.ComboConfigOff = 8U,
		.BaseStrmPortSelectRegOff = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0,
		.NumStrmPortSelectIds = 6U,
		.StrmPortSelectIdsPerReg = 4U,
		.PortIdMask = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_ID_MASK,
		.PortIdOff = 8U,
		.PortMstrSlvMask = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_MANAGER_SUBORDINATE_MASK,
		.PortMstrSlvOff = 5U,
		.Port32b512bMask = XAIE4GBL_CORE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_0_PORT_0_32B_512B_MASK,
		.Port32b512bOff = 6U,
		.BaseDmaChannelSelectRegOff = XAIE_FEATURE_UNAVAILABLE,
		.NumDmaChannelSelectIds = XAIE_FEATURE_UNAVAILABLE,
		.DmaChannelIdOff = XAIE_FEATURE_UNAVAILABLE,
		.DmaChannelIdMask = XAIE_FEATURE_UNAVAILABLE,
		.DmaChannelMM2SOff = XAIE_FEATURE_UNAVAILABLE,
		.EdgeEventRegOff = XAIE4GBL_CORE_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1,
		.EdgeDetectEvent = {XAIE4GBL_CORE_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_EDGE_DETECTION_EVENT_0_LSB, XAIE4GBL_CORE_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_EDGE_DETECTION_EVENT_0_MASK},
		.EdgeDetectTrigger = {XAIE4GBL_CORE_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_EDGE_DETECTION_0_TRIGGER_RISING_LSB, XAIE4GBL_CORE_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_EDGE_DETECTION_0_TRIGGER_FALLING_MASK | XAIE4GBL_CORE_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_EDGE_DETECTION_0_TRIGGER_RISING_MASK},
		.EdgeEventSelectIdOff = XAIE4GBL_CORE_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_EDGE_DETECTION_EVENT_1_LSB,
		.NumEdgeSelectIds = 8U,
		.BaseBroadcastRegOff = XAIE4GBL_CORE_MODULE_EVENT_BROADCAST0,
		.NumBroadcastIds = 16U,
		.BaseBroadcastSwBlockRegOff = XAIE4GBL_CORE_MODULE_EVENT_BROADCAST_BLOCK_SOUTH_SET,
		.BaseBroadcastSwUnblockRegOff = XAIE4GBL_CORE_MODULE_EVENT_BROADCAST_BLOCK_SOUTH_CLR,
		.BroadcastSwOff = 0U,
		.BroadcastSwBlockOff = 16U,
		.BroadcastSwUnblockOff = 16U,
		.NumSwitches = 1U,
		.BaseGroupEventRegOff = XAIE4GBL_CORE_MODULE_EVENT_GROUP_0_ENABLE,
		.NumGroupEvents = 14U,
		.DefaultGroupErrorMask = 0xEB9EBC0U,
		.Group = Aie4CoreGroupEvent,
		.BasePCEventRegOff = XAIE4GBL_CORE_MODULE_PC_EVENT0,
		.NumPCEvents = 10U,
		.PCAddr = {XAIE4GBL_CORE_MODULE_PC_EVENT0_PC_ADDRESS_LSB, XAIE4GBL_CORE_MODULE_PC_EVENT0_PC_ADDRESS_MASK},
		.PCValid = {XAIE4GBL_CORE_MODULE_PC_EVENT0_VALID_LSB, XAIE4GBL_CORE_MODULE_PC_EVENT0_VALID_MASK},
		.BaseStatusRegOff = XAIE4GBL_CORE_MODULE_EVENT_STATUS0,
		.NumUserEvents = 8U,
		.UserEventMap = &Aie4TileCoreModUserEventMap,
		.PCEventMap = &Aie4TileCoreModPCEventMap,
		.BroadcastEventMap = &Aie4TileCoreModBroadcastEventMap,
		.ErrorHaltRegOff = XAIE4GBL_CORE_MODULE_ERROR_HALT_EVENT,
	}
};

/*
 * Data structure to capture NOC tile events properties.
 * For PL module default error group mask enables,
 *	AXI_MM_Slave_Tile_Error,
 *	Control_Pkt_Error,
 *	Stream_Switch_Parity_Error,
 *	AXI_MM_Decode_NSU_Error,
 *	AXI_MM_Slave_NSU_Error,
 *	AXI_MM_Unsupported_Traffic,
 *	AXI_MM_Unsecure_Access_in_Secure_Mode,
 *	AXI_MM_Byte_Strobe_Error,
 *	DMA_0_S2MM_Error,
 *	DMA_0_MM2S_Error,
 *	Lock_Error.
 */
static const XAie_EvntMod Aie4NocEvntMod =
{
	.XAie_EventNumber = Aie4PlModEventMapping,
	.NumEventReg = 8U,
	.EventMin = (u32)XAIE_EVENT_NONE_PL,
	.EventMax = (u32)XAIE_EVENT_UC_CORE_STATUS_7,
	.ComboEventBase = (u32)XAIE_EVENT_COMBO_EVENT_0_PL,
	.PerfCntEventBase = (u32)XAIE_EVENT_PERF_CNT_0_PL,
	.UserEventBase = (u32)XAIE_EVENT_USER_EVENT_0_PL,
	.PortIdleEventBase = (u32)XAIE_EVENT_PORT_IDLE_0_PL,
	.GenEventRegOff = XAIE4GBL_PL_MODULE_EVENT_GENERATE_A,
	.GenEvent = {XAIE4GBL_PL_MODULE_EVENT_GENERATE_A_EVENT_LSB, XAIE4GBL_PL_MODULE_EVENT_GENERATE_A_EVENT_MASK},
	.ComboInputRegOff = XAIE4GBL_PL_MODULE_COMBO_EVENT_INPUTS_A_D_A,
	.ComboEventMask = XAIE4GBL_PL_MODULE_COMBO_EVENT_INPUTS_A_D_A_EVENTA_MASK,
	.ComboEventOff = 8U,
	.ComboCtrlRegOff = XAIE4GBL_PL_MODULE_COMBO_EVENT_CONTROL_0_2_A,
	.ComboConfigMask = XAIE4GBL_PL_MODULE_COMBO_EVENT_CONTROL_0_2_A_COMBO0_MASK,
	.ComboConfigOff = 8U,
	.BaseStrmPortSelectRegOff = XAIE4GBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_A_0,
	.NumStrmPortSelectIds = 6U,
	.StrmPortSelectIdsPerReg = 4U,
	.PortIdMask = XAIE4GBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_A_0_PORT_0_ID_MASK,
	.PortIdOff = 8U,
	.PortMstrSlvMask = XAIE4GBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_A_0_PORT_0_MANAGER_SUBORDINATE_MASK,
	.PortMstrSlvOff = 5U,
	.Port32b512bMask = XAIE4GBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_A_0_PORT_0_32B_512B_MASK,
	.Port32b512bOff = 6U,
	.BaseDmaChannelSelectRegOff = XAIE4GBL_NOC_MODULE_DMA_EVENT_CHANNEL_SELECTION_A,
	.NumDmaChannelSelectIds = 4,
	.DmaChannelIdOff = 8U,
	.DmaChannelIdMask = XAIE4GBL_NOC_MODULE_DMA_EVENT_CHANNEL_SELECTION_A_MM2S_SEL0_CHANNEL_MASK,
	.DmaChannelMM2SOff = XAIE4GBL_NOC_MODULE_DMA_EVENT_CHANNEL_SELECTION_A_MM2S_SEL0_CHANNEL_LSB,
	.EdgeEventRegOff = XAIE4GBL_PL_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A,
	.EdgeDetectEvent = {XAIE4GBL_PL_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_EVENT_0_LSB, XAIE4GBL_PL_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_EVENT_0_MASK},
	.EdgeDetectTrigger = {XAIE4GBL_PL_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_0_TRIGGER_RISING_LSB, XAIE4GBL_PL_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_0_TRIGGER_FALLING_MASK | XAIE4GBL_PL_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_0_TRIGGER_RISING_MASK},
	.EdgeEventSelectIdOff = XAIE4GBL_PL_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_EVENT_1_LSB,
	.NumEdgeSelectIds = 8U,
	.BaseBroadcastRegOff = XAIE4GBL_PL_MODULE_EVENT_BROADCAST_A_0,
	.NumBroadcastIds = 16U,
	.BaseBroadcastSwBlockRegOff = XAIE4GBL_PL_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_SET,
	.BaseBroadcastSwUnblockRegOff = XAIE4GBL_PL_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_CLR,
	.BroadcastSwOff = 0U,
	.BroadcastSwBlockOff = 12U,
	.BroadcastSwUnblockOff = 12U,
	.NumSwitches = 1U,
	.BaseGroupEventRegOff = XAIE4GBL_PL_MODULE_EVENT_GROUP_0_ENABLE_A,
	.NumGroupEvents = 9U,
	.DefaultGroupErrorMask = 0xCFBFU,
	.Group = Aie4PlGroupEvent,
	.BasePCEventRegOff = XAIE_FEATURE_UNAVAILABLE,
	.NumPCEvents = XAIE_FEATURE_UNAVAILABLE,
	.PCAddr = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PCValid = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.BaseStatusRegOff = XAIE4GBL_PL_MODULE_EVENT_STATUS_A_0,
	.NumUserEvents = 8U,
	.UserEventMap = &Aie4ShimTilePlModUserEventStart,
	.PCEventMap = NULL,
	.BroadcastEventMap = &Aie4ShimTilePlModBroadcastEventStart,
	.ErrorHaltRegOff = XAIE_FEATURE_UNAVAILABLE,
};

/*
 * Data structure to capture PL module events properties.
 * For PL module default error group mask enables,
 *	AXI_MM_Slave_Tile_Error,
 *	Control_Pkt_Error,
 *	Stream_Switch_Parity_Error,
 *	AXI_MM_Decode_NSU_Error,
 *	AXI_MM_Slave_NSU_Error,
 *	AXI_MM_Unsupported_Traffic,
 *	AXI_MM_Unsecure_Access_in_Secure_Mode,
 *	AXI_MM_Byte_Strobe_Error,
 *	DMA_0_S2MM_Error,
 *	DMA_0_MM2S_Error,
 *	Lock_Error.
 */
static const XAie_EvntMod Aie4PlEvntMod =
{
	.XAie_EventNumber = Aie4PlModEventMapping,
	.NumEventReg = 8U,
	.EventMin = (u32)XAIE_EVENT_NONE_PL,
	.EventMax = (u32)XAIE_EVENT_UC_CORE_STATUS_7,
	.ComboEventBase = (u32)XAIE_EVENT_COMBO_EVENT_0_PL,
	.PerfCntEventBase = (u32)XAIE_EVENT_PERF_CNT_0_PL,
	.UserEventBase = (u32)XAIE_EVENT_USER_EVENT_0_PL,
	.PortIdleEventBase = (u32)XAIE_EVENT_PORT_IDLE_0_PL,
	.GenEventRegOff = XAIE4GBL_PL_MODULE_EVENT_GENERATE_A,
	.GenEvent = {XAIE4GBL_PL_MODULE_EVENT_GENERATE_A_EVENT_LSB, XAIE4GBL_PL_MODULE_EVENT_GENERATE_A_EVENT_MASK},
	.ComboInputRegOff = XAIE4GBL_PL_MODULE_COMBO_EVENT_INPUTS_A_D_A,
	.ComboEventMask = XAIE4GBL_PL_MODULE_COMBO_EVENT_INPUTS_A_D_A_EVENTA_MASK,
	.ComboEventOff = 8U,
	.ComboCtrlRegOff = XAIE4GBL_PL_MODULE_COMBO_EVENT_CONTROL_0_2_A,
	.ComboConfigMask = XAIE4GBL_PL_MODULE_COMBO_EVENT_CONTROL_0_2_A_COMBO0_MASK,
	.ComboConfigOff = 8U,
	.BaseStrmPortSelectRegOff = XAIE4GBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_A_0,
	.NumStrmPortSelectIds = 8U,
	.StrmPortSelectIdsPerReg = 4U,
	.PortIdMask = XAIE4GBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_A_0_PORT_0_ID_MASK,
	.PortIdOff = 8U,
	.PortMstrSlvMask = XAIE4GBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_A_0_PORT_0_MANAGER_SUBORDINATE_MASK,
	.PortMstrSlvOff = 5U,
	.Port32b512bMask = XAIE4GBL_PL_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_A_0_PORT_0_32B_512B_MASK,
	.Port32b512bOff = 6U,
	.BaseDmaChannelSelectRegOff = XAIE4GBL_NOC_MODULE_DMA_EVENT_CHANNEL_SELECTION_A,
	.NumDmaChannelSelectIds = 4,
	.DmaChannelIdOff = 8U,
	.DmaChannelIdMask = XAIE4GBL_NOC_MODULE_DMA_EVENT_CHANNEL_SELECTION_A_MM2S_SEL0_CHANNEL_MASK,
	.DmaChannelMM2SOff = XAIE4GBL_NOC_MODULE_DMA_EVENT_CHANNEL_SELECTION_A_MM2S_SEL0_CHANNEL_LSB,
	.EdgeEventRegOff = XAIE4GBL_PL_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A,
	.EdgeDetectEvent = {XAIE4GBL_PL_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_EVENT_0_LSB, XAIE4GBL_PL_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_EVENT_0_MASK},
	.EdgeDetectTrigger = {XAIE4GBL_PL_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_0_TRIGGER_RISING_LSB, XAIE4GBL_PL_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_0_TRIGGER_FALLING_MASK | XAIE4GBL_PL_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_0_TRIGGER_RISING_MASK},
	.EdgeEventSelectIdOff = XAIE4GBL_PL_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_EVENT_1_LSB,
	.NumEdgeSelectIds = 8U,
	.BaseBroadcastRegOff = XAIE4GBL_PL_MODULE_EVENT_BROADCAST_A_0,
	.NumBroadcastIds = 16U,
	.BaseBroadcastSwBlockRegOff = XAIE4GBL_PL_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_SET,
	.BaseBroadcastSwUnblockRegOff = XAIE4GBL_PL_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_CLR,
	.BroadcastSwOff = 0U,
	.BroadcastSwBlockOff = 12U,
	.BroadcastSwUnblockOff = 12U,
	.NumSwitches = 1U,
	.BaseGroupEventRegOff = XAIE4GBL_PL_MODULE_EVENT_GROUP_0_ENABLE_A,
	.NumGroupEvents = 9U,
	.DefaultGroupErrorMask = 0xCFBFU,
	.Group = Aie4PlGroupEvent,
	.BasePCEventRegOff = XAIE_FEATURE_UNAVAILABLE,
	.NumPCEvents = XAIE_FEATURE_UNAVAILABLE,
	.PCAddr = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PCValid = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.BaseStatusRegOff = XAIE4GBL_PL_MODULE_EVENT_STATUS_A_0,
	.NumUserEvents = 8U,
	.UserEventMap = &Aie4ShimTilePlModUserEventStart,
	.BroadcastEventMap = &Aie4ShimTilePlModBroadcastEventStart,
	.PCEventMap = NULL,
	.ErrorHaltRegOff = XAIE_FEATURE_UNAVAILABLE,
};

/*
 * Data structure to capture mem tile module events properties.
 * For mem tile default error group mask enables,
 *	DM_ECC_Error_Scrub_2bit,
 *	DM_ECC_Error_2bit,
 *	DMA_0_S2MM_Error,
 *	DMA_0_MM2S_Error,
 *	Stream_Switch_Parity_Error,
 *	Stream_Pkt_Parity_Error,
 *	Control_Pkt_Error,
 *	AXI-MM_Slave_Error,
 *	Lock_Error.
 */
static const XAie_EvntMod Aie4MemTileEvntMod =
{
	.XAie_EventNumber = Aie4MemTileModEventMapping,
	.NumEventReg = 7U,
	.EventMin = (u32)XAIE_EVENT_NONE_MEM_TILE,
	.EventMax = (u32)XAIE_EVENT_USER_EVENT_7_MEM_TILE,
	.ComboEventBase = (u32)XAIE_EVENT_COMBO_EVENT_0_MEM_TILE,
	.PerfCntEventBase = (u32)XAIE_EVENT_PERF_CNT0_EVENT_MEM_TILE,
	.UserEventBase = (u32)XAIE_EVENT_USER_EVENT_0_MEM_TILE,
	.PortIdleEventBase = (u32)XAIE_EVENT_PORT_IDLE_0_MEM_TILE,
	.GenEventRegOff = XAIE4GBL_MEM_TILE_MODULE_EVENT_GENERATE_A,
	.GenEvent = {XAIE4GBL_MEM_TILE_MODULE_EVENT_GENERATE_A_EVENT_LSB, XAIE4GBL_MEM_TILE_MODULE_EVENT_GENERATE_A_EVENT_MASK},
	.ComboInputRegOff = XAIE4GBL_MEM_TILE_MODULE_COMBO_EVENT_INPUTS_A_D_A,
	.ComboEventMask = XAIE4GBL_MEM_TILE_MODULE_COMBO_EVENT_INPUTS_A_D_A_EVENTA_MASK,
	.ComboEventOff = 8U,
	.ComboCtrlRegOff = XAIE4GBL_MEM_TILE_MODULE_COMBO_EVENT_CONTROL_0_2_A,
	.ComboConfigMask = XAIE4GBL_MEM_TILE_MODULE_COMBO_EVENT_CONTROL_0_2_A_COMBO0_MASK,
	.ComboConfigOff = 8U,
	.BaseStrmPortSelectRegOff = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_A_0,
	.NumStrmPortSelectIds = 8U,
	.StrmPortSelectIdsPerReg = 4U,
	.PortIdMask = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_A_0_PORT_0_ID_MASK,
	.PortIdOff = 8U,
	.PortMstrSlvMask = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_A_0_PORT_0_MANAGER_SUBORDINATE_MASK,
	.PortMstrSlvOff = 5U,
	.Port32b512bMask = XAIE4GBL_MEM_TILE_MODULE_STREAM_SWITCH_EVENT_PORT_SELECTION_A_0_PORT_0_32B_512B_MASK,
	.Port32b512bOff = 6U,
	.BaseDmaChannelSelectRegOff = XAIE4GBL_MEM_TILE_MODULE_DMA_EVENT_CHANNEL_SELECTION_A,
	.NumDmaChannelSelectIds = 2U,
	.DmaChannelIdOff = 8U,
	.DmaChannelIdMask = XAIE4GBL_MEM_TILE_MODULE_DMA_EVENT_CHANNEL_SELECTION_A_S2MM_SEL0_CHANNEL_MASK,
	.DmaChannelMM2SOff = XAIE4GBL_MEM_TILE_MODULE_DMA_EVENT_CHANNEL_SELECTION_A_MM2S_SEL0_CHANNEL_LSB,
	.EdgeEventRegOff = XAIE4GBL_MEM_TILE_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A,
	.EdgeDetectEvent = {XAIE4GBL_MEM_TILE_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_EVENT_0_LSB, XAIE4GBL_MEM_TILE_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_EVENT_0_MASK},
	.EdgeDetectTrigger = {XAIE4GBL_MEM_TILE_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_0_TRIGGER_RISING_LSB, XAIE4GBL_MEM_TILE_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_0_TRIGGER_FALLING_MASK | XAIE4GBL_MEM_TILE_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_0_TRIGGER_RISING_MASK},
	.EdgeEventSelectIdOff = XAIE4GBL_MEM_TILE_MODULE_EDGE_DETECTION_EVENT_CONTROL_0_1_A_EDGE_DETECTION_EVENT_1_LSB,
	.NumEdgeSelectIds = 8U,
	.BaseBroadcastRegOff = XAIE4GBL_MEM_TILE_MODULE_EVENT_BROADCAST_A_0,
	.NumBroadcastIds = 16U,
	.BaseBroadcastSwBlockRegOff = XAIE4GBL_MEM_TILE_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_SET,
	.BaseBroadcastSwUnblockRegOff = XAIE4GBL_MEM_TILE_MODULE_EVENT_BROADCAST_A_BLOCK_SOUTH_CLR,
	.BroadcastSwOff = 0U,
	.BroadcastSwBlockOff = 12U,
	.BroadcastSwUnblockOff = 12U,
	.NumSwitches = 1U,
	.BaseGroupEventRegOff = XAIE4GBL_MEM_TILE_MODULE_EVENT_GROUP_0_ENABLE_A,
	.NumGroupEvents = 9U,
	.DefaultGroupErrorMask = 0x3FAU,
	.Group = Aie4MemTileGroupEvent,
	.BasePCEventRegOff = XAIE_FEATURE_UNAVAILABLE,
	.NumPCEvents = XAIE_FEATURE_UNAVAILABLE,
	.PCAddr = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PCValid = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.BaseStatusRegOff = XAIE4GBL_MEM_TILE_MODULE_EVENT_STATUS_A_0,
	.NumUserEvents = 8U,
	.UserEventMap = &Aie4MemTileMemModUserEventStart,
	.PCEventMap = NULL,
	.BroadcastEventMap = &Aie4MemTileMemModBroadcastEventStart,
	.ErrorHaltRegOff = XAIE_FEATURE_UNAVAILABLE,
};
#endif /* XAIE_FEATURE_EVENTS_ENABLE */

#ifdef XAIE_FEATURE_TIMER_ENABLE
static const XAie_TimerMod Aie4TileTimerMod[] =
{
	{
		.TrigEventLowValOff = XAIE_FEATURE_UNAVAILABLE,
		.TrigEventHighValOff = XAIE_FEATURE_UNAVAILABLE,
		.LowOff = XAIE_FEATURE_UNAVAILABLE,
		.HighOff = XAIE_FEATURE_UNAVAILABLE,
		.CtrlOff = XAIE_FEATURE_UNAVAILABLE,
		{XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
		{XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	},
	{
		.TrigEventLowValOff = XAIE4GBL_CORE_MODULE_TIMER_TRIG_EVENT_LOW_VALUE,
		.TrigEventHighValOff = XAIE4GBL_CORE_MODULE_TIMER_TRIG_EVENT_HIGH_VALUE,
		.LowOff = XAIE4GBL_CORE_MODULE_TIMER_LOW,
		.HighOff = XAIE4GBL_CORE_MODULE_TIMER_HIGH,
		.CtrlOff = XAIE4GBL_CORE_MODULE_TIMER_CONTROL,
		{XAIE4GBL_CORE_MODULE_TIMER_CONTROL_RESET_LSB, XAIE4GBL_CORE_MODULE_TIMER_CONTROL_RESET_MASK},
		{XAIE4GBL_CORE_MODULE_TIMER_CONTROL_RESET_EVENT_LSB, XAIE4GBL_CORE_MODULE_TIMER_CONTROL_RESET_EVENT_MASK},
	}
};

static const XAie_TimerMod Aie4PlTimerMod =
{
	.TrigEventLowValOff = XAIE4GBL_PL_MODULE_TIMER_A_TRIG_EVENT_LOW_VALUE,
	.TrigEventHighValOff = XAIE4GBL_PL_MODULE_TIMER_A_TRIG_EVENT_HIGH_VALUE,
	.LowOff = XAIE4GBL_PL_MODULE_TIMER_A_LOW,
	.HighOff = XAIE4GBL_PL_MODULE_TIMER_A_HIGH,
	.CtrlOff = XAIE4GBL_PL_MODULE_TIMER_A_CONTROL,
	{XAIE4GBL_PL_MODULE_TIMER_A_CONTROL_RESET_LSB, XAIE4GBL_PL_MODULE_TIMER_A_CONTROL_RESET_MASK},
	{XAIE4GBL_PL_MODULE_TIMER_A_CONTROL_RESET_EVENT_LSB, XAIE4GBL_PL_MODULE_TIMER_A_CONTROL_RESET_EVENT_MASK}
};

static const XAie_TimerMod Aie4MemTileTimerMod =
{
	.TrigEventLowValOff = XAIE4GBL_MEM_TILE_MODULE_TIMER_A_TRIG_EVENT_LOW_VALUE ,
	.TrigEventHighValOff = XAIE4GBL_MEM_TILE_MODULE_TIMER_A_TRIG_EVENT_HIGH_VALUE,
	.LowOff = XAIE4GBL_MEM_TILE_MODULE_TIMER_A_LOW,
	.HighOff = XAIE4GBL_MEM_TILE_MODULE_TIMER_A_HIGH,
	.CtrlOff = XAIE4GBL_MEM_TILE_MODULE_TIMER_A_CONTROL,
	{XAIE4GBL_MEM_TILE_MODULE_TIMER_A_CONTROL_RESET_LSB, XAIE4GBL_MEM_TILE_MODULE_TIMER_A_CONTROL_RESET_MASK},
	{XAIE4GBL_MEM_TILE_MODULE_TIMER_A_CONTROL_RESET_EVENT_LSB, XAIE4GBL_MEM_TILE_MODULE_TIMER_A_CONTROL_RESET_EVENT_MASK}
};
#endif /* XAIE_FEATURE_TIMER_ENABLE */

#ifdef XAIE_FEATURE_TRACE_ENABLE
/*
 * Data structure to configure trace event register for XAIE_CORE_MOD module
 * type
 */

/* This structure is assigned Trace1 Events to Mem Modules, As In Core Tile there are
   Two Trace modules available one is for Core Module and one is Mem Module. But Both
   Trace Module registers are in Core module, so assigning one to Core Module and one
   to mem module */

static const XAie_RegFldAttr Aie4CoreTraceEvent[] =
{
	{XAIE4GBL_CORE_MODULE_TRACE0_EVENT0_TRACE_EVENT0_LSB, XAIE4GBL_CORE_MODULE_TRACE0_EVENT0_TRACE_EVENT0_MASK},
	{XAIE4GBL_CORE_MODULE_TRACE0_EVENT0_TRACE_EVENT1_LSB, XAIE4GBL_CORE_MODULE_TRACE0_EVENT0_TRACE_EVENT1_MASK},
	{XAIE4GBL_CORE_MODULE_TRACE0_EVENT0_TRACE_EVENT2_LSB, XAIE4GBL_CORE_MODULE_TRACE0_EVENT0_TRACE_EVENT2_MASK},
	{XAIE4GBL_CORE_MODULE_TRACE0_EVENT0_TRACE_EVENT3_LSB, XAIE4GBL_CORE_MODULE_TRACE0_EVENT0_TRACE_EVENT3_MASK},
	{XAIE4GBL_CORE_MODULE_TRACE0_EVENT1_TRACE_EVENT4_LSB, XAIE4GBL_CORE_MODULE_TRACE0_EVENT1_TRACE_EVENT4_MASK},
	{XAIE4GBL_CORE_MODULE_TRACE0_EVENT1_TRACE_EVENT5_LSB, XAIE4GBL_CORE_MODULE_TRACE0_EVENT1_TRACE_EVENT5_MASK},
	{XAIE4GBL_CORE_MODULE_TRACE0_EVENT1_TRACE_EVENT6_LSB, XAIE4GBL_CORE_MODULE_TRACE0_EVENT1_TRACE_EVENT6_MASK},
	{XAIE4GBL_CORE_MODULE_TRACE0_EVENT1_TRACE_EVENT7_LSB, XAIE4GBL_CORE_MODULE_TRACE0_EVENT1_TRACE_EVENT7_MASK}
};

/*
 * Data structure to configure trace event register for XAIE_MEM_MOD module
 * type
 */
static const XAie_RegFldAttr Aie4MemTraceEvent[] =
{
	{XAIE4GBL_CORE_MODULE_TRACE1_EVENT0_TRACE_EVENT0_LSB, XAIE4GBL_CORE_MODULE_TRACE1_EVENT0_TRACE_EVENT0_MASK},
	{XAIE4GBL_CORE_MODULE_TRACE1_EVENT0_TRACE_EVENT1_LSB, XAIE4GBL_CORE_MODULE_TRACE1_EVENT0_TRACE_EVENT1_MASK},
	{XAIE4GBL_CORE_MODULE_TRACE1_EVENT0_TRACE_EVENT2_LSB, XAIE4GBL_CORE_MODULE_TRACE1_EVENT0_TRACE_EVENT2_MASK},
	{XAIE4GBL_CORE_MODULE_TRACE1_EVENT0_TRACE_EVENT3_LSB, XAIE4GBL_CORE_MODULE_TRACE1_EVENT0_TRACE_EVENT3_MASK},
	{XAIE4GBL_CORE_MODULE_TRACE1_EVENT1_TRACE_EVENT4_LSB, XAIE4GBL_CORE_MODULE_TRACE1_EVENT1_TRACE_EVENT4_MASK},
	{XAIE4GBL_CORE_MODULE_TRACE1_EVENT1_TRACE_EVENT5_LSB, XAIE4GBL_CORE_MODULE_TRACE1_EVENT1_TRACE_EVENT5_MASK},
	{XAIE4GBL_CORE_MODULE_TRACE1_EVENT1_TRACE_EVENT6_LSB, XAIE4GBL_CORE_MODULE_TRACE1_EVENT1_TRACE_EVENT6_MASK},
	{XAIE4GBL_CORE_MODULE_TRACE1_EVENT1_TRACE_EVENT7_LSB, XAIE4GBL_CORE_MODULE_TRACE1_EVENT1_TRACE_EVENT7_MASK}
};

/*
 * Data structure to configure trace event register for XAIE_PL_MOD module
 * type
 */
static const XAie_RegFldAttr Aie4PlTraceEvent[] =
{
	{XAIE4GBL_PL_MODULE_TRACE_A_EVENT0_TRACE_EVENT0_LSB, XAIE4GBL_PL_MODULE_TRACE_A_EVENT0_TRACE_EVENT0_MASK},
	{XAIE4GBL_PL_MODULE_TRACE_A_EVENT0_TRACE_EVENT1_LSB, XAIE4GBL_PL_MODULE_TRACE_A_EVENT0_TRACE_EVENT1_MASK},
	{XAIE4GBL_PL_MODULE_TRACE_A_EVENT0_TRACE_EVENT2_LSB, XAIE4GBL_PL_MODULE_TRACE_A_EVENT0_TRACE_EVENT2_MASK},
	{XAIE4GBL_PL_MODULE_TRACE_A_EVENT0_TRACE_EVENT3_LSB, XAIE4GBL_PL_MODULE_TRACE_A_EVENT0_TRACE_EVENT3_MASK},
	{XAIE4GBL_PL_MODULE_TRACE_A_EVENT1_TRACE_EVENT4_LSB, XAIE4GBL_PL_MODULE_TRACE_A_EVENT1_TRACE_EVENT4_MASK},
	{XAIE4GBL_PL_MODULE_TRACE_A_EVENT1_TRACE_EVENT5_LSB, XAIE4GBL_PL_MODULE_TRACE_A_EVENT1_TRACE_EVENT5_MASK},
	{XAIE4GBL_PL_MODULE_TRACE_A_EVENT1_TRACE_EVENT6_LSB, XAIE4GBL_PL_MODULE_TRACE_A_EVENT1_TRACE_EVENT6_MASK},
	{XAIE4GBL_PL_MODULE_TRACE_A_EVENT1_TRACE_EVENT7_LSB, XAIE4GBL_PL_MODULE_TRACE_A_EVENT1_TRACE_EVENT7_MASK}
};

/*
 * Data structure to configure trace event register for
 * XAIEGBL_TILE_TYPE_MEMTILE type
 */
static const XAie_RegFldAttr Aie4MemTileTraceEvent[] =
{
	{XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT0_TRACE_EVENT0_LSB, XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT0_TRACE_EVENT0_MASK},
	{XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT0_TRACE_EVENT1_LSB, XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT0_TRACE_EVENT1_MASK},
	{XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT0_TRACE_EVENT2_LSB, XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT0_TRACE_EVENT2_MASK},
	{XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT0_TRACE_EVENT3_LSB, XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT0_TRACE_EVENT3_MASK},
	{XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT1_TRACE_EVENT4_LSB, XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT1_TRACE_EVENT4_MASK},
	{XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT1_TRACE_EVENT5_LSB, XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT1_TRACE_EVENT5_MASK},
	{XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT1_TRACE_EVENT6_LSB, XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT1_TRACE_EVENT6_MASK},
	{XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT1_TRACE_EVENT7_LSB, XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT1_TRACE_EVENT7_MASK}
};

/*
 * Data structure to configure trace module for XAIEGBL_TILE_TYPE_AIETILE tile
 * type
 */
static const XAie_TraceMod Aie4TileTraceMod[] =
{
	{
		.CtrlRegOff = XAIE4GBL_CORE_MODULE_TRACE1_CONTROL0,
		.PktConfigRegOff = XAIE4GBL_CORE_MODULE_TRACE1_CONTROL1,
		.StatusRegOff = XAIE4GBL_CORE_MODULE_TRACE1_STATUS,
		.EventRegOffs = (u32 []){XAIE4GBL_CORE_MODULE_TRACE1_EVENT0, XAIE4GBL_CORE_MODULE_TRACE1_EVENT1},
		.NumTraceSlotIds = 8U,
		.NumEventsPerSlot = 4U,
		.StopEvent = {XAIE4GBL_CORE_MODULE_TRACE1_CONTROL0_TRACE_STOP_EVENT_LSB, XAIE4GBL_CORE_MODULE_TRACE1_CONTROL0_TRACE_STOP_EVENT_MASK},
		.StartEvent = {XAIE4GBL_CORE_MODULE_TRACE1_CONTROL0_TRACE_START_EVENT_LSB, XAIE4GBL_CORE_MODULE_TRACE1_CONTROL0_TRACE_START_EVENT_MASK},
		.ModeConfig = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
		.PktType = {XAIE4GBL_CORE_MODULE_TRACE1_CONTROL1_PACKET_TYPE_LSB, XAIE4GBL_CORE_MODULE_TRACE1_CONTROL1_PACKET_TYPE_MASK},
		.PktId = {XAIE4GBL_CORE_MODULE_TRACE1_CONTROL1_ID_LSB, XAIE4GBL_CORE_MODULE_TRACE1_CONTROL1_ID_MASK},
		.State = {XAIE4GBL_CORE_MODULE_TRACE1_STATUS_STATE_LSB, XAIE4GBL_CORE_MODULE_TRACE1_STATUS_STATE_MASK},
		.ModeSts = {XAIE4GBL_CORE_MODULE_TRACE1_STATUS_MODE_LSB, XAIE4GBL_CORE_MODULE_TRACE1_STATUS_MODE_MASK},
		.Event = Aie4MemTraceEvent
	},
	{
		.CtrlRegOff = XAIE4GBL_CORE_MODULE_TRACE0_CONTROL0,
		.PktConfigRegOff = XAIE4GBL_CORE_MODULE_TRACE0_CONTROL1,
		.StatusRegOff = XAIE4GBL_CORE_MODULE_TRACE0_STATUS,
		.EventRegOffs = (u32 []){XAIE4GBL_CORE_MODULE_TRACE0_EVENT0, XAIE4GBL_CORE_MODULE_TRACE0_EVENT1},
		.NumTraceSlotIds = 8U,
		.NumEventsPerSlot = 4U,
		.StopEvent = {XAIE4GBL_CORE_MODULE_TRACE0_CONTROL0_TRACE_STOP_EVENT_LSB, XAIE4GBL_CORE_MODULE_TRACE0_CONTROL0_TRACE_STOP_EVENT_MASK},
		.StartEvent = {XAIE4GBL_CORE_MODULE_TRACE0_CONTROL0_TRACE_START_EVENT_LSB, XAIE4GBL_CORE_MODULE_TRACE0_CONTROL0_TRACE_START_EVENT_MASK},
		.ModeConfig = {XAIE4GBL_CORE_MODULE_TRACE0_CONTROL0_MODE_LSB, XAIE4GBL_CORE_MODULE_TRACE0_CONTROL0_MODE_MASK},
		.PktType = {XAIE4GBL_CORE_MODULE_TRACE0_CONTROL1_PACKET_TYPE_LSB, XAIE4GBL_CORE_MODULE_TRACE0_CONTROL1_PACKET_TYPE_MASK},
		.PktId = {XAIE4GBL_CORE_MODULE_TRACE0_CONTROL1_ID_LSB, XAIE4GBL_CORE_MODULE_TRACE0_CONTROL1_ID_MASK},
		.State = {XAIE4GBL_CORE_MODULE_TRACE0_STATUS_STATE_LSB, XAIE4GBL_CORE_MODULE_TRACE0_STATUS_STATE_MASK},
		.ModeSts = {XAIE4GBL_CORE_MODULE_TRACE0_STATUS_MODE_LSB, XAIE4GBL_CORE_MODULE_TRACE0_STATUS_MODE_MASK},
		.Event = Aie4CoreTraceEvent
	},
};

/*
 * Data structure to configure trace module for XAIEGBL_TILE_TYPE_SHIMNOC/PL
 * tile type
 */
static const XAie_TraceMod Aie4PlTraceMod =
{
	.CtrlRegOff = XAIE4GBL_PL_MODULE_TRACE_A_CONTROL0,
	.PktConfigRegOff = XAIE4GBL_PL_MODULE_TRACE_A_CONTROL1,
	.StatusRegOff = XAIE4GBL_PL_MODULE_TRACE_A_STATUS,
	.EventRegOffs = (u32 []){XAIE4GBL_PL_MODULE_TRACE_A_EVENT0, XAIE4GBL_PL_MODULE_TRACE_A_EVENT1},
	.NumTraceSlotIds = 8U,
	.NumEventsPerSlot = 4U,
	.StopEvent = {XAIE4GBL_PL_MODULE_TRACE_A_CONTROL0_TRACE_STOP_EVENT_LSB, XAIE4GBL_PL_MODULE_TRACE_A_CONTROL0_TRACE_STOP_EVENT_MASK},
	.StartEvent = {XAIE4GBL_PL_MODULE_TRACE_A_CONTROL0_TRACE_START_EVENT_LSB, XAIE4GBL_PL_MODULE_TRACE_A_CONTROL0_TRACE_START_EVENT_MASK},
	.ModeConfig = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PktType = {XAIE4GBL_PL_MODULE_TRACE_A_CONTROL1_PACKET_TYPE_LSB, XAIE4GBL_PL_MODULE_TRACE_A_CONTROL1_PACKET_TYPE_MASK},
	.PktId = {XAIE4GBL_PL_MODULE_TRACE_A_CONTROL1_ID_LSB, XAIE4GBL_PL_MODULE_TRACE_A_CONTROL1_ID_MASK},
	.State = {XAIE4GBL_PL_MODULE_TRACE_A_STATUS_STATE_LSB, XAIE4GBL_PL_MODULE_TRACE_A_STATUS_STATE_MASK},
	.ModeSts = {XAIE4GBL_PL_MODULE_TRACE_A_STATUS_MODE_LSB, XAIE4GBL_PL_MODULE_TRACE_A_STATUS_MODE_MASK},
	.Event = Aie4PlTraceEvent
};

/*
 * Data structure to configure trace module for XAIEGBL_TILE_TYPE_MEMTILE
 * tile type
 */
static const XAie_TraceMod Aie4MemTileTraceMod =
{
	.CtrlRegOff = XAIE4GBL_MEM_TILE_MODULE_TRACE_A_CONTROL0,
	.PktConfigRegOff = XAIE4GBL_MEM_TILE_MODULE_TRACE_A_CONTROL1,
	.StatusRegOff = XAIE4GBL_MEM_TILE_MODULE_TRACE_A_STATUS,
	.EventRegOffs = (u32 []){XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT0, XAIE4GBL_MEM_TILE_MODULE_TRACE_A_EVENT1},
	.NumTraceSlotIds = 8U,
	.NumEventsPerSlot = 4U,
	.StopEvent = {XAIE4GBL_MEM_TILE_MODULE_TRACE_A_CONTROL0_TRACE_STOP_EVENT_LSB, XAIE4GBL_MEM_TILE_MODULE_TRACE_A_CONTROL0_TRACE_STOP_EVENT_MASK},
	.StartEvent = {XAIE4GBL_MEM_TILE_MODULE_TRACE_A_CONTROL0_TRACE_START_EVENT_LSB, XAIE4GBL_MEM_TILE_MODULE_TRACE_A_CONTROL0_TRACE_START_EVENT_MASK},
	.ModeConfig = {XAIE_FEATURE_UNAVAILABLE, XAIE_FEATURE_UNAVAILABLE},
	.PktType = {XAIE4GBL_MEM_TILE_MODULE_TRACE_A_CONTROL1_PACKET_TYPE_LSB, XAIE4GBL_MEM_TILE_MODULE_TRACE_A_CONTROL1_PACKET_TYPE_MASK},
	.PktId = {XAIE4GBL_MEM_TILE_MODULE_TRACE_A_CONTROL1_ID_LSB, XAIE4GBL_MEM_TILE_MODULE_TRACE_A_CONTROL1_ID_MASK},
	.State = {XAIE4GBL_MEM_TILE_MODULE_TRACE_A_STATUS_STATE_LSB, XAIE4GBL_MEM_TILE_MODULE_TRACE_A_STATUS_STATE_MASK},
	.ModeSts = {XAIE4GBL_MEM_TILE_MODULE_TRACE_A_STATUS_MODE_LSB, XAIE4GBL_MEM_TILE_MODULE_TRACE_A_STATUS_MODE_MASK},
	.Event = Aie4MemTileTraceEvent
};
#endif /* XAIE_FEATURE_TRACE_ENABLE */

#ifdef XAIE_FEATURE_INTR_L2_ENABLE
/*
 * Data structure to configures second level interrupt controller for
 * XAIEGBL_TILE_TYPE_SHIMNOC tile type
 */
static const XAie_L2IntrMod Aie4NoCL2IntrMod =
{
	.EnableRegOff = XAIEMLGBL_NOC_MODULE_INTERRUPT_CONTROLLER_2ND_LEVEL_ENABLE,
	.DisableRegOff = XAIEMLGBL_NOC_MODULE_INTERRUPT_CONTROLLER_2ND_LEVEL_DISABLE,
	.IrqRegOff = XAIEMLGBL_NOC_MODULE_INTERRUPT_CONTROLLER_2ND_LEVEL_INTERRUPT,
	.NumBroadcastIds = 16U,
	.NumNoCIntr = 4U,
	.MaxErrorBcIdsRvd = 4U, //Kotesh(TODO): Fix as per AIE4 requirement after discussion with architect team.
};
#endif /* XAIE_FEATURE_INTR_L2_ENABLE */

#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE
/*
 * Data structure to configures tile control for
 * XAIEGBL_TILE_TYPE_AIETILE tile type
 */
static const XAie_TileCtrlMod Aie4CoreTileCtrlMod =
{
	.TileCtrlRegOff = XAIE4GBL_CORE_MODULE_TILE_CONTROL_PRIVILEGED,
	.IsolateEast = {XAIE4GBL_CORE_MODULE_TILE_CONTROL_ISOLATE_FROM_EAST_LSB_PRIVILEGED, XAIE4GBL_CORE_MODULE_TILE_CONTROL_ISOLATE_FROM_EAST_MASK_PRIVILEGED},
	.IsolateNorth = {XAIE4GBL_CORE_MODULE_TILE_CONTROL_ISOLATE_FROM_NORTH_LSB_PRIVILEGED, XAIE4GBL_CORE_MODULE_TILE_CONTROL_ISOLATE_FROM_NORTH_MASK_PRIVILEGED},
	.IsolateWest = {XAIE4GBL_CORE_MODULE_TILE_CONTROL_ISOLATE_FROM_WEST_LSB_PRIVILEGED, XAIE4GBL_CORE_MODULE_TILE_CONTROL_ISOLATE_FROM_WEST_MASK_PRIVILEGED},
	.IsolateSouth = {XAIE4GBL_CORE_MODULE_TILE_CONTROL_ISOLATE_FROM_SOUTH_LSB_PRIVILEGED, XAIE4GBL_CORE_MODULE_TILE_CONTROL_ISOLATE_FROM_SOUTH_MASK_PRIVILEGED},
	.IsolateDefaultOn = XAIE_ENABLE,
	.DualAppModeRegOff = XAIE4GBL_CORE_MODULE_DUAL_APP_MODE_PRIVILEGED,
	.DualAppControl = {XAIE4GBL_CORE_MODULE_DUAL_APP_MODE_DUAL_APP_MODE_LSB_PRIVILEGED, XAIE4GBL_CORE_MODULE_DUAL_APP_MODE_DUAL_APP_MODE_MASK_PRIVILEGED},


};

/*
 * Data structure to configures tile control for
 * XAIEGBL_TILE_TYPE_MEMTILE tile type
 */
static const XAie_TileCtrlMod Aie4MemTileCtrlMod =
{
	.TileCtrlRegOff = XAIE4GBL_MEM_TILE_MODULE_TILE_CONTROL_PRIVILEGED,
	.IsolateEast = {XAIE4GBL_MEM_TILE_MODULE_TILE_CONTROL_ISOLATE_FROM_EAST_LSB_PRIVILEGED, XAIE4GBL_MEM_TILE_MODULE_TILE_CONTROL_ISOLATE_FROM_EAST_MASK_PRIVILEGED},
	.IsolateNorth = {XAIE4GBL_MEM_TILE_MODULE_TILE_CONTROL_ISOLATE_FROM_NORTH_LSB_PRIVILEGED, XAIE4GBL_MEM_TILE_MODULE_TILE_CONTROL_ISOLATE_FROM_NORTH_MASK_PRIVILEGED},
	.IsolateWest = {XAIE4GBL_MEM_TILE_MODULE_TILE_CONTROL_ISOLATE_FROM_WEST_LSB_PRIVILEGED, XAIE4GBL_MEM_TILE_MODULE_TILE_CONTROL_ISOLATE_FROM_WEST_MASK_PRIVILEGED},
	.IsolateSouth = {XAIE4GBL_MEM_TILE_MODULE_TILE_CONTROL_ISOLATE_FROM_SOUTH_LSB_PRIVILEGED, XAIE4GBL_MEM_TILE_MODULE_TILE_CONTROL_ISOLATE_FROM_SOUTH_MASK_PRIVILEGED},
	.IsolateDefaultOn = XAIE_ENABLE,
	.DualAppModeRegOff = XAIE4GBL_MEM_TILE_MODULE_DUAL_APP_MODE_PRIVILEGED,
	.DualAppControl = {XAIE4GBL_MEM_TILE_MODULE_DUAL_APP_MODE_DUAL_APP_MODE_LSB_PRIVILEGED, XAIE4GBL_MEM_TILE_MODULE_DUAL_APP_MODE_DUAL_APP_MODE_MASK_PRIVILEGED},
	.L2SplitRegOff = XAIE4GBL_MEM_TILE_MODULE_L2_SPLIT_PRIVILEGED,
	.L2SplitControl = {XAIE4GBL_MEM_TILE_MODULE_L2_SPLIT_MASK_PRIVILEGED, XAIE4GBL_MEM_TILE_MODULE_L2_SPLIT_L2_SPLIT_LSB_PRIVILEGED},
};

/*
 * Data structure to configures tile control for
 * XAIEGBL_TILE_TYPE_SHIMPL/NOC tile type
 */
static const XAie_TileCtrlMod Aie4ShimTileCtrlMod =
{
	.TileCtrlRegOff = XAIE4GBL_PL_MODULE_TILE_CONTROL_PRIVILEGED,
	.TileCtrlAxiRegOff = XAIE4GBL_PL_MODULE_TILE_CONTROL_AXI_MM_PRIVILEGED,
	.IsolateEast = {XAIE4GBL_PL_MODULE_TILE_CONTROL_ISOLATE_FROM_EAST_LSB_PRIVILEGED, XAIE4GBL_PL_MODULE_TILE_CONTROL_ISOLATE_FROM_EAST_MASK_PRIVILEGED},
	.IsolateNorth = {XAIE4GBL_PL_MODULE_TILE_CONTROL_ISOLATE_FROM_NORTH_LSB_PRIVILEGED, XAIE4GBL_PL_MODULE_TILE_CONTROL_ISOLATE_FROM_NORTH_MASK_PRIVILEGED},
	.IsolateWest = {XAIE4GBL_PL_MODULE_TILE_CONTROL_ISOLATE_FROM_WEST_LSB_PRIVILEGED, XAIE4GBL_PL_MODULE_TILE_CONTROL_ISOLATE_FROM_WEST_MASK_PRIVILEGED},
	.IsolateSouth = {XAIE4GBL_PL_MODULE_TILE_CONTROL_ISOLATE_FROM_SOUTH_LSB_PRIVILEGED, XAIE4GBL_PL_MODULE_TILE_CONTROL_ISOLATE_FROM_SOUTH_MASK_PRIVILEGED},
	.IsolateAxiEast = {XAIE4GBL_PL_MODULE_TILE_CONTROL_AXI_MM_ISOLATE_FROM_EAST_LSB_PRIVILEGED, XAIE4GBL_PL_MODULE_TILE_CONTROL_AXI_MM_ISOLATE_FROM_EAST_MASK_PRIVILEGED},
	.IsolateAxiWest = {XAIE4GBL_PL_MODULE_TILE_CONTROL_AXI_MM_ISOLATE_FROM_WEST_LSB_PRIVILEGED, XAIE4GBL_PL_MODULE_TILE_CONTROL_AXI_MM_ISOLATE_FROM_WEST_MASK_PRIVILEGED},
	.IsolateDefaultOn = XAIE_ENABLE,
	.DualAppModeRegOff = XAIE4GBL_PL_MODULE_DUAL_APP_MODE_PRIVILEGED,
	.DualAppControl = {XAIE4GBL_MEM_TILE_MODULE_DUAL_APP_MODE_DUAL_APP_MODE_LSB_PRIVILEGED, XAIE4GBL_MEM_TILE_MODULE_DUAL_APP_MODE_DUAL_APP_MODE_MASK_PRIVILEGED},
};

/*
 * Data structure to configures memory control for
 * XAIEGBL_TILE_TYPE_AIETILE tile type
 */
static const XAie_MemCtrlMod Aie4TileMemCtrlMod[] =
{
	{
		.MemZeroisationCtrlRegOff = XAIE4GBL_MEMORY_MODULE_MEMORY_CONTROL_PRIVILEGED,
		.MemZeroisation = {XAIE4GBL_MEMORY_MODULE_MEMORY_CONTROL_MEMORY_ZEROISATION_LSB_PRIVILEGED, XAIE4GBL_MEMORY_MODULE_MEMORY_CONTROL_MEMORY_ZEROISATION_MASK_PRIVILEGED},
	},
	{
		.MemZeroisationCtrlRegOff = XAIE4GBL_CORE_MODULE_MEMORY_CONTROL_PRIVILEGED,
		.MemZeroisation = {XAIE4GBL_CORE_MODULE_MEMORY_CONTROL_MEMORY_ZEROISATION_LSB_PRIVILEGED, XAIE4GBL_CORE_MODULE_MEMORY_CONTROL_MEMORY_ZEROISATION_MASK_PRIVILEGED},
	},
};

/*
 * Data structure to configure memory interleaving for
 * XAIEGBL_TILE_TYPE_AIETILE tile type
 */
static const XAie_MemCtrlMod Aie4TileMemIntrlvCtrlMod =
{
	.MemInterleavingCtrlRegOff = XAIE4GBL_CORE_MODULE_MEMORY_INTERLEAVING_PRIVILEGED,
	.MemInterleaving = {XAIE4GBL_CORE_MODULE_MEMORY_INTERLEAVING_MEMORY_INTERLEAVING_LSB_PRIVILEGED, XAIE4GBL_CORE_MODULE_MEMORY_INTERLEAVING_MEMORY_INTERLEAVING_MASK_PRIVILEGED},
};

/*
 * Data structure to configures memory control for
 * XAIEGBL_TILE_TYPE_MEMTILE tile type
 */
/* *** TODO: This XAie_MemCtrlMod needs to modify , Need to add MEMORY_ZEROIZATION_B register information after further discussions***  * */
static const XAie_MemCtrlMod Aie4MemTileMemCtrlMod_A =
{
  .MemZeroisationCtrlRegOff = XAIE4GBL_MEM_TILE_MODULE_MEMORY_ZEROIZATION_A_PRIVILEGED,
  .MemZeroisation = {XAIE4GBL_MEM_TILE_MODULE_MEMORY_ZEROIZATION_A_MEMORY_ZEROISATION_LSB_PRIVILEGED, XAIE4GBL_MEM_TILE_MODULE_MEMORY_ZEROIZATION_A_MEMORY_ZEROISATION_MASK_PRIVILEGED},
};

static const XAie_MemCtrlMod Aie4MemTileMemCtrlMod_B =
{
  .MemZeroisationCtrlRegOff = XAIE4GBL_MEM_TILE_MODULE_MEMORY_ZEROIZATION_B_PRIVILEGED,
  .MemZeroisation = {XAIE4GBL_MEM_TILE_MODULE_MEMORY_ZEROIZATION_B_MEMORY_ZEROISATION_LSB_PRIVILEGED, XAIE4GBL_MEM_TILE_MODULE_MEMORY_ZEROIZATION_B_MEMORY_ZEROISATION_MASK_PRIVILEGED},
};

static const XAie_MemCtrlMod Aie4MemTileMemIntrLvCtrlMod =
{
  .MemInterleavingCtrlRegOff = XAIE4GBL_MEM_TILE_MODULE_MEMORY_INTERLEAVING_PRIVILEGED,
  .MemInterleaving = {XAIE4GBL_MEM_TILE_MODULE_MEMORY_INTERLEAVING_MEMORY_INTERLEAVING_LSB_PRIVILEGED, XAIE4GBL_MEM_TILE_MODULE_MEMORY_INTERLEAVING_MEMORY_INTERLEAVING_MASK_PRIVILEGED},
};

static const XAie_MemCtrlMod Aie4UcCntrlMod =
{
  .MemZeroisationCtrlRegOff = XAIE4GBL_PL_MODULE_MEMORY_ZEROIZATION_A_PRIVILEGED,
  .MemPrivilegeCtrlRegOff = XAIE4GBL_PL_MODULE_MEMORY_PRIVILEGED,
  .MemZeroisation  = {XAIE4GBL_PL_MODULE_MEMORY_ZEROIZATION_A_MASK_PRIVILEGED, XAIE4GBL_PL_MODULE_MEMORY_ZEROIZATION_A_MASK_PRIVILEGED},
  .MemPrivilegeCtrl = {XAIE4GBL_PL_MODULE_MEMORY_PRIVILEGED_MEMORY_LSB_PRIVILEGED, XAIE4GBL_PL_MODULE_MEMORY_PRIVILEGED_MEMORY_MASK_PRIVILEGED},
};

static const XAie_MemCtrlMod Aie4UcCntrlMod_B =
{
  .MemZeroisationCtrlRegOff = XAIE4GBL_PL_MODULE_MEMORY_ZEROIZATION_B_PRIVILEGED,
  .MemZeroisation  = {XAIE4GBL_PL_MODULE_MEMORY_ZEROIZATION_B_MASK_PRIVILEGED, XAIE4GBL_PL_MODULE_MEMORY_ZEROIZATION_B_MASK_PRIVILEGED},
};
#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */

#ifdef XAIE_FEATURE_CORE_ENABLE
	#define AIE4COREMOD &Aie4CoreMod
	#define AIE4COREINTMOD &Aie4CoreIntMod
#else
	#define AIE4COREMOD NULL
	#define AIE4COREINTMOD NULL
#endif
#ifdef XAIE_FEATURE_UC_ENABLE
	#define AIE4UCMOD &Aie4UcMod
#else
	#define AIE4UCMOD NULL
#endif
#ifdef XAIE_FEATURE_SS_ENABLE
	#define AIE4TILESTRMSW &Aie4TileStrmSw
	#define AIE4SHIMSTRMSW &Aie4ShimStrmSw
	#define AIE4MEMTILESTRMSW &Aie4MemTileStrmSw
	#define AIE4TILESTRMSW32B &Aie4TileStrmSw32b
	#define AIE4SHIMSTRMSW32B &Aie4ShimStrmSw32b
	#define AIE4MEMTILESTRMSW32B &Aie4MemTileStrmSw32b
#else
	#define AIE4TILESTRMSW NULL
	#define AIE4SHIMSTRMSW NULL
	#define AIE4MEMTILESTRMSW NULL
	#define AIE4TILESTRMSW32B NULL
	#define AIE4SHIMSTRMSW32B NULL
	#define AIE4MEMTILESTRMSW32B NULL
#endif
#ifdef XAIE_FEATURE_DMA_ENABLE
	#define AIE4TILEDMAMOD &Aie4TileDmaMod
	#define AIE4SHIMDMAMOD &Aie4ShimDmaMod
	#define AIE4MEMTILEDMAMOD &Aie4MemTileDmaMod
#else
	#define AIE4TILEDMAMOD NULL
	#define AIE4SHIMDMAMOD NULL
	#define AIE4MEMTILEDMAMOD NULL
#endif
#ifdef XAIE_FEATURE_DATAMEM_ENABLE
	#define AIE4TILEMEMMOD &Aie4TileMemMod
	#define AIE4MEMTILEMEMMOD &Aie4MemTileMemMod
#else
	#define AIE4TILEMEMMOD NULL
	#define AIE4MEMTILEMEMMOD NULL
#endif
#ifdef XAIE_FEATURE_LOCK_ENABLE
	#define AIE4TILELOCKMOD &Aie4TileLockMod
	#define AIE4SHIMNOCLOCKMOD &Aie4ShimNocLockMod
	#define AIE4MEMTILELOCKMOD &Aie4MemTileLockMod
#else
	#define AIE4TILELOCKMOD NULL
	#define AIE4SHIMNOCLOCKMOD NULL
	#define AIE4MEMTILELOCKMOD NULL
#endif
#ifdef XAIE_FEATURE_PERFCOUNT_ENABLE
	#define AIE4TILEPERFCNT Aie4TilePerfCnt
	#define AIE4PLPERFCNT &Aie4PlPerfCnt
	#define AIE4MEMTILEPERFCNT &Aie4MemTilePerfCnt
#else
	#define AIE4TILEPERFCNT NULL
	#define AIE4PLPERFCNT NULL
	#define AIE4MEMTILEPERFCNT NULL
#endif
#ifdef XAIE_FEATURE_EVENTS_ENABLE
	#define AIE4TILEEVNTMOD Aie4TileEvntMod
	#define AIE4NOCEVNTMOD &Aie4NocEvntMod
	#define AIE4PLEVNTMOD &Aie4PlEvntMod
	#define AIE4MEMTILEEVNTMOD &Aie4MemTileEvntMod
#else
	#define AIE4TILEEVNTMOD NULL
	#define AIE4NOCEVNTMOD NULL
	#define AIE4PLEVNTMOD NULL
	#define AIE4MEMTILEEVNTMOD NULL
#endif
#ifdef XAIE_FEATURE_TIMER_ENABLE
	#define AIE4TILETIMERMOD Aie4TileTimerMod
	#define AIE4PLTIMERMOD &Aie4PlTimerMod
	#define AIE4MEMTILETIMERMOD &Aie4MemTileTimerMod
#else
	#define AIE4TILETIMERMOD NULL
	#define AIE4PLTIMERMOD NULL
	#define AIE4MEMTILETIMERMOD NULL
#endif
#ifdef XAIE_FEATURE_TRACE_ENABLE
	#define AIE4TILETRACEMOD Aie4TileTraceMod
	#define AIE4PLTRACEMOD &Aie4PlTraceMod
	#define AIE4MEMTILETRACEMOD &Aie4MemTileTraceMod
#else
	#define AIE4TILETRACEMOD NULL
	#define AIE4PLTRACEMOD NULL
	#define AIE4MEMTILETRACEMOD NULL
#endif
#ifdef XAIE_FEATURE_PL_ENABLE
	#define AIE4SHIMTILEPLIFMOD &Aie4ShimTilePlIfMod
	#define AIE4SHIMTILENOCMOD &Aie4ShimTileNocMod
#else
	#define AIE4SHIMTILEPLIFMOD NULL
	#define AIE4SHIMTILENOCMOD NULL
#endif
#ifdef XAIE_FEATURE_INTR_L2_ENABLE
	#define AIE4NOCL2INTRMOD &Aie4NoCL2IntrMod
#else
	#define AIE4NOCL2INTRMOD NULL
#endif
#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE
	#define AIE4CORETILECTRLMOD &Aie4CoreTileCtrlMod
	#define AIE4TILEMEMCTRLMOD Aie4TileMemCtrlMod
	#define AIE4TILEMEMINTRLVCTRLMOD &Aie4TileMemIntrlvCtrlMod
	#define AIE4SHIMTILECTRLMOD &Aie4ShimTileCtrlMod
	#define AIE4MEMTILECTRLMOD &Aie4MemTileCtrlMod
	#define AIE4MEMTILEMEMCTRLMOD_A &Aie4MemTileMemCtrlMod_A
	#define AIE4MEMTILEMEMCTRLMOD_B &Aie4MemTileMemCtrlMod_B
    #define AIE4MEMTILEMEMINTRLVCTRLMOD &Aie4MemTileMemIntrLvCtrlMod
	#define AIE4UCMEMCNTRLMOD   &Aie4UcCntrlMod
	#define AIE4UCMEMCNTRLMOD_B   &Aie4UcCntrlMod_B
#else
	#define AIE4CORETILECTRLMOD NULL
	#define AIE4TILEMEMCTRLMOD NULL
	#define AIE4TILEMEMINTRLVCTRLMOD NULL
	#define AIE4SHIMTILECTRLMOD NULL
	#define AIE4MEMTILECTRLMOD NULL
	#define AIE4MEMTILEMEMCTRLMOD_A NULL
	#define AIE4MEMTILEMEMCTRLMOD_B NULL
	#define AIE4MEMTILEMEMINTRLVCTRLMOD NULL
	#define AIE4UCMEMCNTRLMOD NULL
	#define AIE4UCMEMCNTRLMOD_B NULL
#endif

/*
 * AIE4 Module
 * This data structure captures all the modules for each tile type.
 * Depending on the tile type, this data strcuture can be used to access all
 * hardware properties of individual modules.
 */
XAie_TileMod Aie4MedusaMod[] =
{
	{
		/*
		 * AIE4 Tile Module indexed using XAIEGBL_TILE_TYPE_AIETILE
		 */
		.NumModules = 2U,
		.CoreMod = AIE4COREMOD,
		.CoreIntMod = AIE4COREINTMOD,
		.StrmSw  = AIE4TILESTRMSW,
		.StrmSw32b	= AIE4TILESTRMSW32B,
		.DmaMod  = AIE4TILEDMAMOD,
		.MemMod  = AIE4TILEMEMMOD,
		.PlIfMod = NULL,
		.NocMod = NULL,
		.LockMod = AIE4TILELOCKMOD,
		.PerfMod = AIE4TILEPERFCNT,
		.EvntMod = AIE4TILEEVNTMOD,
		.TimerMod = AIE4TILETIMERMOD,
		.TraceMod = AIE4TILETRACEMOD,
		.L2IntrMod = NULL,
		.TileCtrlMod = AIE4CORETILECTRLMOD,
		.MemCtrlMod = AIE4TILEMEMCTRLMOD,
		.MemCtrlInterLvMod = AIE4TILEMEMINTRLVCTRLMOD,
	},
	{
		/*
		 * AIE4 Shim Noc Module indexed using XAIEGBL_TILE_TYPE_SHIMNOC
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
		.CoreIntMod = NULL,
		.StrmSw  = AIE4SHIMSTRMSW,
		.StrmSw32b  = AIE4SHIMSTRMSW32B,
		.DmaMod  = AIE4SHIMDMAMOD,
		.MemMod  = NULL,
		.PlIfMod = AIE4SHIMTILEPLIFMOD,
		.NocMod = AIE4SHIMTILENOCMOD,
		.LockMod = AIE4SHIMNOCLOCKMOD,
		.PerfMod = AIE4PLPERFCNT,
		.EvntMod = AIE4NOCEVNTMOD,
		.TimerMod = AIE4PLTIMERMOD,
		.TraceMod = AIE4PLTRACEMOD,
		.L2IntrMod = AIE4NOCL2INTRMOD,
		.TileCtrlMod = AIE4SHIMTILECTRLMOD,
		.MemCtrlMod = NULL,
		.UcMod = AIE4UCMOD,
		.MemCtrlUcMod = AIE4UCMEMCNTRLMOD,
		.MemCtrlUcMod_B = AIE4UCMEMCNTRLMOD_B,
	},
	{
		/*
		 * AIE4 Shim PL Module indexed using XAIEGBL_TILE_TYPE_SHIMPL
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
		.CoreIntMod = NULL,
		.StrmSw  = NULL,
		.DmaMod  = NULL,
		.MemMod  = NULL,
		.PlIfMod = NULL,
		.NocMod = NULL,
		.LockMod = NULL,
		.PerfMod = AIE4PLPERFCNT,
		.EvntMod = AIE4PLEVNTMOD,
		.TimerMod = NULL,
		.TraceMod = NULL,
		.L2IntrMod = NULL,
		.TileCtrlMod = NULL,
		.MemCtrlMod = NULL,
		.MemCtrlUcMod = NULL,
		.MemCtrlUcMod_B = NULL,
	},
	{
		/*
		 * AIE4 MemTile Module indexed using XAIEGBL_TILE_TYPE_MEMTILE
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
		.CoreIntMod = NULL,
		.StrmSw  = AIE4MEMTILESTRMSW,
		.StrmSw32b = AIE4MEMTILESTRMSW32B,
		.DmaMod  = AIE4MEMTILEDMAMOD,
		.MemMod  = AIE4MEMTILEMEMMOD,
		.PlIfMod = NULL,
		.NocMod = NULL,
		.LockMod = AIE4MEMTILELOCKMOD,
		.PerfMod = AIE4MEMTILEPERFCNT,
		.EvntMod = AIE4MEMTILEEVNTMOD,
		.TimerMod = AIE4MEMTILETIMERMOD,
		.TraceMod = AIE4MEMTILETRACEMOD,
		.L2IntrMod = NULL,
		.TileCtrlMod = AIE4MEMTILECTRLMOD,
		.MemCtrlMod_A = AIE4MEMTILEMEMCTRLMOD_A,
		.MemCtrlMod_B = AIE4MEMTILEMEMCTRLMOD_B,
		.MemCtrlInterLvMod = AIE4MEMTILEMEMINTRLVCTRLMOD,
	}
};

/* Device level operations for aieml */
XAie_DeviceOps Aie4MedusaDevOps =
{
	.IsCheckerBoard = 0U,
	.TilesInUse = Aie4TilesInUse,
	.MemInUse = Aie4MemInUse,
	.CoreInUse = Aie4CoreInUse,
	.GetTTypefromLoc = &_XAie4_GetTTypefromLoc,
#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE
	.SetPartColShimReset = &_XAie4_SetPartApplicationReset,
	.SetPartColClockAfterRst = &_XAieMl_SetPartColClockAfterRst,
	.SetPartIsolationAfterRst = &_XAie4_SetPartIsolationAfterRst,
	.SetUCMemoryPrivileged = &_XAie4_SetUCMemoryPrivileged,
	.ZeroInitUcMem = &_XAie4_ZeroInitUcMemory,
	.PartMemZeroInit = &_XAie4_PartMemZeroInit,
	.PartMemL2Split = &_XAie4_PartInitL2Split,
	.RequestTiles = &_XAie4_RequestTiles,
	.SetColumnClk = &_XAieMl_SetColumnClk,
	.SetAppMode = &_XAie4_SetDualAppModePrivileged,
#else
	.SetPartColShimReset = NULL,
	.SetPartColClockAfterRst = NULL,
	.SetPartIsolationAfterRst = NULL,
	.SetUCMemoryPrivileged = NULL,
	.ZeroInitUcMem = NULL,
	.PartMemZeroInit = NULL,
	.PartMemL2Split = NULL,
	.RequestTiles = NULL,
	.SetColumnClk = NULL,
	.SetAppMode = NULL,
#endif
};

/** @} */
