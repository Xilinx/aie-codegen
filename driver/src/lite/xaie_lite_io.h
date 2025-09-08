/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite_io.h
* @{
*
* This header file defines a lightweight version of AIE driver IO APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Nishad   08/30/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_LITE_IO_H
#define XAIE_LITE_IO_H

/***************************** Include Files *********************************/
#include "xaie_lite_hwcfg.h"
#include "xaiegbl.h"

#ifdef __AIE_REGDUMP__
void printk(const char *fmt, ...);
#endif


/************************** Constant Definitions *****************************/
/************************** Variable Definitions *****************************/
/************************** Function Prototypes  *****************************/
 #ifdef XAIE_FEATURE_LITE
__FORCE_INLINE__
static inline u64 _XAie_LGetTileAddr(u32 Row, u32 Col)
{
	if(XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE4_GENERIC || XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE4
			|| XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE4_A) {
		if(Row >= XAIE_AIE_TILE_ROW_START) {
			Row += (XAIE_MEM_TILE_NUM_ROWS * XAIE_LITE_AIE4_AIE_TILE_SHIFT_OFFSET);
		} else if(Row > 0 && Row < XAIE_AIE_TILE_ROW_START) {
			Row = (Row * XAIE_LITE_AIE4_MEM_TILE_SHIFT_OFFSET) - XAIE_LITE_AIE4_AIE_TILE_SHIFT_OFFSET;
		}
	}
	return (Row << XAIE_ROW_SHIFT) | (Col << XAIE_COL_SHIFT);
}

__FORCE_INLINE__
static inline u64 _XAie_LPartGetTileAddr(XAie_DevInst *DevInst,
		XAie_LocType Loc)
{
	return DevInst->BaseAddr + _XAie_LGetTileAddr(Loc.Row, Loc.Col);
}

#if defined(__AIECUSTOMIO__)

extern inline void _XAie_LRawWrite32(u64 RegAddr, u32 Value);
extern inline void _XAie_LRawMaskWrite32(u64 RegAddr, u32 Mask, u32 Value);
extern inline u32 _XAie_LRawRead32(u64 RegAddr);
extern inline int _XAie_LRawPoll32(u64 RegAddr, u32 Mask, u32 Value, u32 TimeoutUs);

#elif defined(__AIEDEBUG__)
__FORCE_INLINE__
static inline void _XAie_LRawWrite32(u64 RegAddr, u32 Value)
{
	printf("W: %p, 0x%x\n", (void *) RegAddr, Value);
}

__FORCE_INLINE__
static inline void _XAie_LRawMaskWrite32(u64 RegAddr, u32 Mask, u32 Value)
{
	printf("MW: %p, 0x%x 0x%x\n", (void *) RegAddr, Mask, Value);
}

__FORCE_INLINE__
static inline u32 _XAie_LRawRead32(u64 RegAddr)
{
	printf("R: %p\n", (void *) RegAddr);
	return 0;
}

__FORCE_INLINE__
static inline int _XAie_LRawPoll32(u64 RegAddr, u32 Mask, u32 Value, u32 TimeOutUs)
{
	(void)TimeOutUs;
	printf("P: %p, 0x%x, 0x%x\n", (void *) RegAddr, Mask, Value);
	return 0;
}

#else

extern int usleep(unsigned int usec);

__FORCE_INLINE__
static inline void _XAie_LRawWrite32(u64 RegAddr, u32 Value)
{
#if UINTPTR_MAX == 0xFFFFFFFF  // 32-bit system
    if (RegAddr > UINTPTR_MAX) {
    	return;
    }
#endif
	#ifdef __AIE_REGDUMP__
	printk("_XAie_LRawWrite32 RegAddr - 0x%llx Value - 0x%x \n",RegAddr,Value);
	#endif
	*(volatile u32*)(uintptr_t)RegAddr = Value;
}

__FORCE_INLINE__
static inline void _XAie_LRawMaskWrite32(u64 RegAddr, u32 Mask, u32 Value)
{
#if UINTPTR_MAX == 0xFFFFFFFF  // 32-bit system
    if (RegAddr > UINTPTR_MAX) {
    	return;
    }
#endif
	u32 RegVal = *(volatile u32*)(uintptr_t) RegAddr;

	RegVal = (RegVal & (~Mask)) | Value;
	#ifdef __AIE_REGDUMP__
	printk("_XAie_LRawMaskWrite32 RegAddr - 0x%llx Value - 0x%x \n",RegAddr,RegVal);
	#endif
	*(volatile u32*) (uintptr_t)RegAddr = RegVal;
}

__FORCE_INLINE__
static inline u32 _XAie_LRawRead32(u64 RegAddr)
{
#if UINTPTR_MAX == 0xFFFFFFFF  // 32-bit system
    if (RegAddr > UINTPTR_MAX) {
    	return XAIE_ERR;
    }
#endif
	#ifdef __AIE_REGDUMP__
	printk("_XAie_LRawRead32 RegAddr - 0x%llx \n",RegAddr);
	#endif
	return *(volatile u32*)(uintptr_t)RegAddr;
}

__FORCE_INLINE__
static inline int _XAie_LRawPoll32(u64 RegAddr, u32 Mask, u32 Value, u32 TimeOutUs)
{
	u32 MinTimeOutUs = 1, Count, RegVal;
	Count = TimeOutUs / MinTimeOutUs;

	/* User needs to pass valid timeout value to make sure, HW gets enough
	   Time to change register status.*/
	if(Count == 0) {
		Count++;
	}
	do {
		RegVal = _XAie_LRawRead32(RegAddr);
		#ifdef __AIE_REGDUMP__
		printk("_XAie_LRawPoll32 RegVal - 0x%x \n",RegVal);
		#endif
		if((RegVal & Mask) == Value) {
			return 0;
		}
		usleep(MinTimeOutUs);
	} while(Count--);

	return -1;
}
#endif

__FORCE_INLINE__
static inline void _XAie_LWrite32(u64 RegAddr, u32 Value)
{
	_XAie_LRawWrite32((XAIE_BASE_ADDR + RegAddr), Value);
}

__FORCE_INLINE__
static inline u32 _XAie_LRead32(u64 RegAddr)
{
	return _XAie_LRawRead32(XAIE_BASE_ADDR + RegAddr);
}

__FORCE_INLINE__
static inline void _XAie_LPartWrite32(XAie_DevInst *DevInst, u64 RegAddr,
		u32 Value)
{
	_XAie_LRawWrite32((DevInst->BaseAddr + RegAddr), Value);
}

__FORCE_INLINE__
static inline void _XAie_LPartMaskWrite32(XAie_DevInst *DevInst, u64 RegAddr,
		u32 Mask, u32 Value)
{
	_XAie_LRawMaskWrite32((DevInst->BaseAddr + RegAddr), Mask, Value);
}

__FORCE_INLINE__
static inline void _XAie_LPartBlockSet32(XAie_DevInst *DevInst, u64 RegAddr,
		u32 Value, u32 SizeByte)
{
	for (u32 Count = 0; Count < SizeByte / sizeof(u32); Count++) {
		_XAie_LPartWrite32(DevInst, RegAddr, Value);
		RegAddr += sizeof(u32);
	}
}

__FORCE_INLINE__
static inline u32 _XAie_LPartRead32(XAie_DevInst *DevInst, u64 RegAddr)
{
	return _XAie_LRawRead32(DevInst->BaseAddr + RegAddr);
}

__FORCE_INLINE__
static inline int _XAie_LPartPoll32(XAie_DevInst *DevInst, u64 RegAddr,
		u32 Mask, u32 Value, u32 TimeOutUs)
{
	return _XAie_LRawPoll32((DevInst->BaseAddr + RegAddr), Mask, Value,
			TimeOutUs);
}


/*****************************************************************************/
/**
*
* This API used to write Block of data into give address. This will provide
* functionality to user to call sigle API to write block of data.
*
* @param        DevInst: Device Instance
* @param        RegAddr: Starting address on which Data needs to written.
* @param        Data: Pointer to Data block
* @param        Size: Total number of bytes needs to be written
*
* @note         None
*
******************************************************************************/
__FORCE_INLINE__
static inline void _XAie_LPartBlockWrite32(XAie_DevInst *DevInst, u64 RegAddr,
                const u32 *Data, u32 Size)
{
        for(u32 i = 0U; i < Size; i++) {
                _XAie_LPartWrite32(DevInst, RegAddr + i * 4U, *Data);
                Data++;
        }
}


#endif /* XAIE_FEATURE_LITE */
#endif /* XAIE_LITE_IO_H */

/** @} */
