/******************************************************************************
* Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_helper.h
* @{
*
* This file contains inline helper functions for AIE drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   12/09/2019  Include correct header file to avoid cyclic
*			    dependancy
* 1.2   Tejus   03/22/2020  Remove helper functions used by initial dma
*			    implementations
* 1.3   Tejus   04/13/2020  Add api to get tile type from Loc
* 1.4   Tejus   04/13/2020  Remove helper functions for range apis
* 1.5   Tejus   06/10/2020  Add helper functions for IO backend.
* 1.6   Nishad  07/06/2020  Add helper functions for stream switch module.
* 1.7   Nishad  07/24/2020  Add _XAie_GetFatalGroupErrors() helper function.
* </pre>
*
******************************************************************************/
#ifndef XAIEHELPER_H
#define XAIEHELPER_H

/***************************** Include Files *********************************/
#include <limits.h>
#include <stdio.h>
#include "xaie_io.h"
#include "xaiegbl_regdef.h"
#include "xaie_dma.h"
#include "xaie_locks.h"

/***************************** Macro Definitions *****************************/
#define CheckBit(bitmap, pos)   ((bitmap)[(u64)(pos) / (sizeof((bitmap)[0]) * 8U)] & \
				(u32)(1U << (u64)(pos) % (sizeof((bitmap)[0]) * 8U)))

#define XAIE_ERROR(...)							      \
	do {								      \
		XAie_Log((FILE*)(uintptr_t)stderr, "[AIE ERROR]", __func__, __LINE__,	      \
				__VA_ARGS__);				      \
	} while(0)

#define XAIE_WARN(...)							      \
	do {								      \
		XAie_Log((FILE*)(uintptr_t)stderr, "[AIE WARNING]", __func__, __LINE__,	      \
				__VA_ARGS__);				      \
	} while(0)

/**
* Note: Enable the definition of XAIE_FDEBUG macro to enable prints from aie-rt
*/
//#define XAIE_DEBUG 1

#ifdef XAIE_DEBUG

#define XAIE_DBG(...)							      \
	do {								      \
		XAie_Log(stdout, "[AIE DEBUG]", __func__, __LINE__,	      \
				__VA_ARGS__);				      \
	} while(0)

#else

#define XAIE_DBG(DevInst, ...) {}

#endif /* XAIE_DEBUG */

/* Compute offset of field within a structure */
#define XAIE_OFFSET_OF(structure, member) \
	((uintptr_t)&(((structure *)0)->member))

/* Compute a pointer to a structure given a pointer to one of its fields */
#define XAIE_CONTAINER_OF(ptr, structure, member) \
	 ((uintptr_t)(ptr) - XAIE_OFFSET_OF(structure, member))

/* Loop through the set bits in Value */
#define for_each_set_bit(Index, Value, Len)				      \
	for((Index) = first_set_bit((Value)) - 1;			      \
	    (Index) < (Len);						      \
	    (Value) &= (Value) - 1, (Index) = first_set_bit((Value)) - 1)

/* Generate value with a set bit at given Index */
#ifndef BIT
#define BIT(Index)		(1 << (Index))
#endif

/*as AIE address space is 32bit , the max valid bit index will be 31*/
#define MAX_VALID_AIE_REG_BIT_INDEX 32
#define MAX_VALID_U8_BIT_INDEX 8
#define MAX_VALID_U16_BIT_INDEX 16

/*the below Macros are to capture AIE4 features*/
#define NO_L1_INTERRUPT_SUPPORT 1		/* L1 interuupt is removed */
#define PERFORMANCE_SNAPSHOT_SUPPORT 2	/*performance snapshot registers are added */
#define BITMAP64_GROUPEVENT_SUPPORT 3	/*PL Groupevent contains 64bitmap in AIE4*/
#define NO_MEM_MOD_IN_AIE_TILE	4	/*core and mem module in AIE tile are combined*/


/*
 * __attribute is not supported for windows. remove it conditionally.
 */
#ifdef _MSC_VER
#define XAIE_PACK_ATTRIBUTE
#else
#define XAIE_PACK_ATTRIBUTE  __attribute__((packed, aligned(4)))
#endif

/* Data structure to capture the dma status */
typedef struct {
        u32 S2MMStatus;
        u32 MM2SStatus;
} XAie_DmaStatus;

/* Data structure to capture the core tile status */
typedef struct {
        XAie_DmaStatus *Dma;
        u32 *EventCoreModStatus;
        u32 *EventMemModStatus;
        u32 CoreStatus;
        u32 ProgramCounter;
        u32 StackPtr;
        u32 LinkReg;
        u8  *LockValue;
} XAie_CoreTileStatus;
/* Data structure to capture the mem tile status */
typedef struct {
        XAie_DmaStatus *Dma;
        u32 *EventStatus;
        u8 *LockValue;
} XAie_MemTileStatus;

/* Data structure to capture the shim tile status */
typedef struct {
        XAie_DmaStatus *Dma;
        u32 *EventStatus;
        u8 *LockValue;
} XAie_ShimTileStatus;

/* Data structure to capture column status */
typedef struct {
        XAie_CoreTileStatus *CoreTile;
        XAie_MemTileStatus *MemTile;
        XAie_ShimTileStatus *ShimTile;
} XAie_ColStatus;

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* Calculates the index value of first set bit. Indexing starts with a value of
* 1.
*
* @param	Value: Value
* @return	Index of first set bit.
*
* @note		Internal API only.
*
******************************************************************************/
static inline u32 first_set_bit(u64 Value)
{
	u32 Index = 1;

	if (Value == 0U) {
		return 0;
	}

	while ((Value & 1U) == 0U) {
		Value >>= 1;
		Index++;
	}

	return Index;
}

/* Private Functions (can be called by AIE Internal Driver Only */
void BuffHexDump(const char* buff,u32 size);

/* Private Functions used by Public Headers. Need to be discussed as not to export Private API's  */
XAIE_AIG_EXPORT u8 XAie_GetTileTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc);
XAIE_AIG_EXPORT AieRC XAie_CheckModule(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module);
XAIE_AIG_EXPORT AieRC XAie_GetUngatedLocsInPartition(XAie_DevInst *DevInst, u32 *NumTiles,
		XAie_LocType *Locs);
XAIE_AIG_EXPORT u32 XAie_GetNumRows(XAie_DevInst *DevInst, u8 TileType);
XAIE_AIG_EXPORT u32 XAie_GetStartRow(XAie_DevInst *DevInst, u8 TileType);
XAIE_AIG_EXPORT u64 XAie_GetTileAddr(XAie_DevInst *DevInst, u8 R, u8 C);
XAIE_AIG_EXPORT u8 XAie_IsUcModulePresent(XAie_DevInst* DevInst, u8 TileType);

/* AIE4 Specific global APIs*/
XAIE_AIG_EXPORT u8 XAie_IsTileTypeAndModuleSupportForEvents(XAie_DevInst* DevInst,
							XAie_LocType Loc, XAie_ModuleType Module);
XAIE_AIG_EXPORT	u16 XAie_GetMaxElementValue(u8 DevGen, u8 TileType, u8 AppMode, u16 elementValue);
XAIE_AIG_EXPORT u8 XAie_IsDeviceSupportsL1Interrupt(u8 DevGen);
XAIE_AIG_EXPORT u8 XAie_GetComboEventsNumber(XAie_DevInst* DevInst, u8 TileType, XAie_ModuleType Module);
XAIE_AIG_EXPORT u8 XAie_IsFeatureSupportCheck(u8 DevGen, u8 Feature);

/* this below  Functions will be removed , once other teams migrate to above listed functions  */
XAIE_AIG_EXPORT u8 _XAie_GetTileTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc);
XAIE_AIG_EXPORT AieRC _XAie_CheckModule(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module);
XAIE_AIG_EXPORT AieRC _XAie_GetUngatedLocsInPartition(XAie_DevInst *DevInst, u32 *NumTiles,
		XAie_LocType *Locs);
XAIE_AIG_EXPORT u32 _XAie_GetNumRows(XAie_DevInst *DevInst, u8 TileType);
XAIE_AIG_EXPORT u32 _XAie_GetStartRow(XAie_DevInst *DevInst, u8 TileType);
XAIE_AIG_EXPORT u64 _XAie_GetTileAddr(XAie_DevInst *DevInst, u8 R, u8 C);

/*Public functions. Need to by discussed why it should be public */
XAIE_AIG_EXPORT AieRC XAie_Read32(XAie_DevInst *DevInst, u64 RegOff, u32 *Data);
XAIE_AIG_EXPORT AieRC XAie_MaskWrite32(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value);
XAIE_AIG_EXPORT AieRC XAie_MaskPoll(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs);
XAIE_AIG_EXPORT AieRC XAie_MaskPollBusy(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs);
XAIE_AIG_EXPORT AieRC XAie_BlockWrite32(XAie_DevInst *DevInst, u64 RegOff, const u32 *Data,
			u32 Size);
XAIE_AIG_EXPORT AieRC XAie_BlockSet32(XAie_DevInst *DevInst, u64 RegOff, u32 Data, u32 Size);
XAIE_AIG_EXPORT void XAie_Log(FILE *Fd, const char *prefix, const char *func, u32 line,
		const char *Format, ...);
XAIE_AIG_EXPORT AieRC XAie_StatusDump(XAie_DevInst *DevInst, XAie_ColStatus *Status);
XAIE_AIG_EXPORT AieRC XAie_RunOp(XAie_DevInst *DevInst, XAie_BackendOpCode Op, void *Arg);
XAIE_AIG_EXPORT AieRC XAie_CmdWrite(XAie_DevInst *DevInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr);

/* Public Functions. Later this should be moved to xaiegbl.h. Also functions should be moved to xaiegbl.c */
XAIE_AIG_EXPORT AieRC XAie_Write32(XAie_DevInst *DevInst, u64 RegOff, u32 Value);
XAIE_AIG_EXPORT AieRC XAie_AddressPatching(XAie_DevInst *DevInst, u16 Arg_Offset, u8 Num_BDs);
XAIE_AIG_EXPORT AieRC XAie_WaitTct(XAie_DevInst *DevInst, uint16_t Column, uint16_t Row, uint32_t Channel, uint8_t NumTokens);
XAIE_AIG_EXPORT AieRC XAie_WaitUCDMA(XAie_DevInst *DevInst);
XAIE_AIG_EXPORT AieRC XAie_ModeConfig(XAie_DevInst *DevInst, XAie_ModeSelect Mode);
XAIE_AIG_EXPORT XAie_ModeSelect XAie_GetModeConfig(XAie_DevInst *DevInst);

/* While calling XAie_Preempt API, the caller does not have to call StartNewPage, StartNewJob, EndJob and EndPage APIs explicitly.
   Since, the requirement of this opcode is that it should be in a new, independent and self contained page, the Preempt API itself
   will take care of starting/ending the page and job.
*/
XAIE_AIG_EXPORT AieRC XAie_Preempt(XAie_DevInst *DevInst, u16 PreemptId, char* SaveLabel, char* RestoreLabel);
XAIE_AIG_EXPORT AieRC XAie_SetPadInteger(XAie_DevInst *DevInst, char* BuffName, u32 BuffSize);
XAIE_AIG_EXPORT AieRC XAie_SetPadString(XAie_DevInst *DevInst, char* BuffName, char* BuffBlobPath);
XAIE_AIG_EXPORT AieRC XAie_AttachToGroup(XAie_DevInst *DevInst, uint8_t UcIndex);
XAIE_AIG_EXPORT AieRC XAie_RemoteBarrier(XAie_DevInst *DevInst,  uint8_t RbId, uint32_t UcMask);
#endif		/* end of protection macro */
/** @} */
