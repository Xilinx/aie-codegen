/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xaie_lite.c
* @{
*
* This file contains lite routines.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Nishad  06/23/2022  Initial creation
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xaie_feature_config.h"

#if defined(XAIE_FEATURE_PRIVILEGED_ENABLE) && defined(XAIE_FEATURE_LITE)

#include "xaie_lite.h"
#include "xaie_lite_internal.h"
#include "xaiegbl_defs.h"
#include "xaiegbl.h"

/***************************** Macro Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API maps given IRQ ID to a range of columns it is programmed to receive
* interrupts from.
*
* @param	IrqId:
* @param	Range: Pointer to return column range mapping.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None
*
******************************************************************************/
AieRC XAie_MapIrqIdToCols(u8 IrqId, XAie_Range *Range)
{
	XAIE_ERROR_RETURN(IrqId >= XAIE_MAX_NUM_NOC_INTR, XAIE_INVALID_ARGS,
			XAIE_ERROR_MSG("Invalid AIE IRQ ID\n"));

	XAie_Range Temp = _XAie_MapIrqIdToCols(IrqId);
	Range->Start = Temp.Start;
	Range->Num = Temp.Num;

	return XAIE_OK;
}

AieRC XAie_ClearCoreReg(XAie_DevInst *DevInst)
{
	AieRC RC = XAIE_OK;
	/* Based on the Architecture corresponding API will be
	   called*/
        if (_XAie_LIsDeviceGenAIE4()) {
                return XAIE_NOT_SUPPORTED;
        }
	RC = _XAie_ClearCoreReg(DevInst);
	return RC;
}

AieRC XAie_PauseMem(XAie_DevInst *DevInst)
{
        if (_XAie_LIsDeviceGenAIE4()) {
                return XAIE_NOT_SUPPORTED;
        }
        _XAie_PauseMem(DevInst);
        return XAIE_OK;
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
AieRC XAie_ClearBCPort(XAie_DevInst *DevInst, u8 BcChan) {
        AieRC RC;

        if (!_XAie_LIsDeviceGenAIE4()) {
                return XAIE_NOT_SUPPORTED;
        }

        RC = _XAie_LClearBCPort(DevInst, BcChan);
        if (RC != XAIE_OK) {
                XAIE_ERROR("AIE array Clear Broadcast Interrupt Failed\n");
                return RC;
        } else {
                return XAIE_OK;
        }
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
AieRC XAie_TrigColIntr(XAie_DevInst *DevInst, u8 BcChan) {
        AieRC RC;

        if (!_XAie_LIsDeviceGenAIE4()) {
                return XAIE_NOT_SUPPORTED;
        }

        RC = _XAie_LTrigColIntr(DevInst, BcChan);
        if (RC != XAIE_OK) {
                XAIE_ERROR("AIE array Clear Broadcast Interrupt Failed\n");
                return RC;
        } else {
                return XAIE_OK;
        }
}


/*****************************************************************************/
/**
*
* This API is used to wakeup the micro controller(s) in shim tile by trigger
* XAIE_EVENT_USER_EVENT_0_PL (USER_EVENT_O) to givem column.
*
* @param	DevInst: Device Instance
* @param	ColNum: Column Number
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_WakeupShimUc(XAie_DevInst *DevInst, u8 ColNum)
{
	AieRC RC = XAIE_OK;

	/* Based on the Architecture corresponding API will be called*/
	if (!_XAie_LIsDeviceGenAIE4()) {
		XAIE_ERROR("Unsupported device generation\n");
		return XAIE_NOT_SUPPORTED;
	}

	RC = _XAie_WakeupShimUc(DevInst, ColNum);
	return RC;
}

/*****************************************************************************/
/**
 *
 * This is a helper API to write a block of data to the specified data memory
 * location of the selected tile. Byte-level writes are supported by this API.
 * For unaligned data memory offsets, this API implements read-modify-write
 * operation.
 *
 * @param        DevInst: Device Instance
 * @param        Loc: Loc of AIE Tiles
 * @param        Offset: Address in data memory to write.
 * @param        DMBaseAddr: Starting address of the data memory.
 * @param        Src: Source to write data.
 * @param        Size: Size in bytes to write.
 *
 * @return       XAIE_OK on success and error code on failure
 *
 * @note         Internal only.
 *
 *******************************************************************************/
static AieRC _XAie_LDataMemoryBlockWrite(XAie_DevInst* DevInst, XAie_LocType Loc,
		u32 Offset, u32 DMBaseAddr, const unsigned char *Src, u32 Size) {
	AieRC RC = XAIE_OK;
	u32 BytePtr = 0, RemBytes = Size, TempWord = 0, Mask = 0;
	u64 DmAddrRoundDown, DmAddrRoundUp;
	u8 FirstWriteOffset = (u8)(Offset & XAIE_MEM_WORD_ALIGN_MASK);

	/* Absolute 4-byte aligned AXI-MM address to write */
	DmAddrRoundDown =  (u64)(DMBaseAddr + XAIE_MEM_WORD_ROUND_DOWN(Offset)) +
		_XAie_LGetTileAddr(Loc.Row, Loc.Col);

	/* Round-up unaligned Addr */
	DmAddrRoundUp = (u64)(DMBaseAddr + XAIE_MEM_WORD_ROUND_UP(Offset)) +
		_XAie_LGetTileAddr(Loc.Row, Loc.Col);

	/* Round-up unaligned Addr */
	DmAddrRoundUp = (u64)(DMBaseAddr + XAIE_MEM_WORD_ROUND_UP(Offset)) +
		_XAie_LGetTileAddr(Loc.Row, Loc.Col);

	/* Unaligned start bytes */
	if(FirstWriteOffset) {
		/*
		 * Construct 4-byte word along with bit mask to read-modify
		 * write at unaligned offset
		 */
		for(u32 UnalignedByte = FirstWriteOffset;
				(u8)((UnalignedByte < XAIE_MEM_WORD_ALIGN_SIZE) && (RemBytes != 0U));
				UnalignedByte++, RemBytes--) {
			TempWord |= (u32)((u32)Src[BytePtr++] << (UnalignedByte * 8U));
			Mask |= (u32)(0xFFU << (UnalignedByte * 8U));
		}
		_XAie_LPartMaskWrite32(DevInst, DmAddrRoundDown, Mask, TempWord);
	}

	/* Aligned bytes */
	if (RemBytes >= XAIE_MEM_WORD_ALIGN_SIZE) {
		_XAie_LPartBlockWrite32(DevInst, DmAddrRoundUp,
				(const u32 *)(uintptr_t)(Src + BytePtr),
				(RemBytes / XAIE_MEM_WORD_ALIGN_SIZE));
	}
	/* Remaining unaligned bytes */
	if(RemBytes % XAIE_MEM_WORD_ALIGN_SIZE) {
		DmAddrRoundDown = DmAddrRoundUp + (u64)(XAIE_MEM_WORD_ALIGN_SIZE *
				(RemBytes / XAIE_MEM_WORD_ALIGN_SIZE));
		BytePtr += XAIE_MEM_WORD_ALIGN_SIZE *
			(RemBytes / XAIE_MEM_WORD_ALIGN_SIZE);
		TempWord = 0;
		Mask = 0;
		for (u32 UnalignedByte = 0;
				UnalignedByte < RemBytes % XAIE_MEM_WORD_ALIGN_SIZE;
				UnalignedByte++) {
			TempWord |= (u32)((u32)Src[BytePtr++] << (UnalignedByte * 8U));
			Mask |= (u32)(0xFFU << (UnalignedByte * 8U));
		}
		_XAie_LPartMaskWrite32(DevInst, DmAddrRoundDown, Mask, TempWord);
	}

	return RC;
}

/*****************************************************************************/
/**
 *
 * This API writes a block of data to the specified data memory location of
 * the selected tile. Byte-level writes are supported by this API. For unaligned
 * data memory offsets, this API implements read-modify-write operation.
 *
 * @param        DevInst: Device Instance
 * @param        Loc: Loc of AIE Tiles
 * @param        Addr: Address in data memory to write with Memory Offset
 * @param        Src - Source to write data.
 * @param        Size - Size in bytes to write.
 *
 * @return       XAIE_OK on success and error code on failure
 *
 * @note         None.
 *
 *******************************************************************************/
AieRC XAie_LDataMemBlockWrite(XAie_DevInst *DevInst, XAie_LocType Loc, u32 Addr,
		const void *Src, u32 Size)
{
	AieRC RC;
	u32 MemSize, MemAddr;
	u8 TileType;
	const unsigned char *CharSrc = (const unsigned char *)Src;

	if((DevInst == XAIE_NULL) || (Src == NULL))
	{
		XAIE_ERROR("Invalid device instance or source pointer\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_LGetTTypefromLoc(DevInst, Loc);
	if(TileType >= XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	switch(TileType)
	{
		case XAIEGBL_TILE_TYPE_AIETILE:
                        if((Addr & XAIE_CORE_PROGRAM_MEMORY) == XAIE_CORE_PROGRAM_MEMORY) {
                                MemSize = XAIE_CORE_PROGRAM_MEMORY_SIZE;
                                MemAddr = XAIE_CORE_PROGRAM_MEMORY;
                        }
                        else if ((Addr & XAIE_CORE_MEMORY_MODULE_DATAMEMORY) == XAIE_CORE_MEMORY_MODULE_DATAMEMORY) {
                                MemSize = XAIE_CORE_DATA_MEMORY_SIZE;
                                MemAddr = XAIE_CORE_MEMORY_MODULE_DATAMEMORY;
                        }
			break;
		case XAIEGBL_TILE_TYPE_MEMTILE:
			MemSize = XAIE_MEM_TILE_MEMORY_SIZE;
			MemAddr = XAIE_MEM_TILE_MEMORY_MODULE_DATAMEMORY;
			break;
		case XAIEGBL_TILE_TYPE_SHIMNOC:
			#if DEV_GEN_AIE4
			if((Addr & XAIE_UC_PRIVATE_DATA_MEMORY) == XAIE_UC_PRIVATE_DATA_MEMORY) {
				MemSize = XAIE_UC_PRIVATE_DATA_MEMORY_SIZE;
				MemAddr = XAIE_UC_PRIVATE_DATA_MEMORY;
			}
			else if((Addr & XAIE_UC_SHARED_DATA_MEMORY) == XAIE_UC_SHARED_DATA_MEMORY) {
				MemSize = XAIE_UC_SHARED_DATA_MEMORY_SIZE;
				MemAddr = XAIE_UC_SHARED_DATA_MEMORY;
			}
			else if((Addr & XAIE_UC_PROGRAM_MEMORY) == XAIE_UC_PROGRAM_MEMORY) {
				MemSize = XAIE_UC_PROGRAM_MEMORY_SIZE;
				MemAddr = XAIE_UC_PROGRAM_MEMORY;
			}
			break;
			#else
				return XAIE_INVALID_TILE;
			#endif
	}
	Addr = (~MemAddr) & Addr;

	/* Check for any size overflow */
	if((u64)Addr + Size > MemSize) {
		XAIE_ERROR("Size of source block overflows tile data memory\n");
		return XAIE_ERR_OUTOFBOUND;
	}

	RC = _XAie_LDataMemoryBlockWrite(DevInst, Loc, Addr, MemAddr, CharSrc,
			Size);

	return RC;
}


/*****************************************************************************/
/**
 *
 * This API reads a block of data from the specified data memory location of
 * the selected tile. Byte-level reads are supported by this API. For unaligned
 * data memory offsets, this API implements read-modify-write operation.
 *
 * @param        DevInst: Device Instance
 * @param        Loc: Loc of AIE Tiles
 * @param        Addr: Address in data memory to read.
 * @param        Dst - Destination to store read data.
 * @param        Size - Size in bytes to read.
 *
 * @return       XAIE_OK on success and error code on failure
 *
 * @note         None.
 *
 *******************************************************************************/
AieRC XAie_LDataMemBlockRead(XAie_DevInst *DevInst, XAie_LocType Loc, u32 Addr,
		void *Dst, u32 Size)
{
	AieRC RC;
	u32 MemSize, MemAddr;
	u64 DmAddrRoundDown, DmAddrRoundUp;
	u32 BytePtr = 0;
	u32 RemBytes = Size;
	u32 TempWord;
	u8 TileType;

	u8 FirstReadOffset = (u8)(Addr & XAIE_MEM_WORD_LAST_BYTE_MASK) & XAIE_MEM_WORD_ALIGN_MASK;
	unsigned char *CharDst = (unsigned char *)Dst;

	if((DevInst == XAIE_NULL) || (Dst == NULL))
	{
		XAIE_ERROR("Invalid device instance or destination pointer\n");
		return XAIE_INVALID_ARGS;
	}
	TileType = _XAie_LGetTTypefromLoc(DevInst, Loc);
	if(TileType >= XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	switch(TileType)
	{
		case XAIEGBL_TILE_TYPE_AIETILE:
                        if((Addr & XAIE_CORE_PROGRAM_MEMORY) == XAIE_CORE_PROGRAM_MEMORY) {
                                MemSize = XAIE_CORE_PROGRAM_MEMORY_SIZE;
                                MemAddr = XAIE_CORE_PROGRAM_MEMORY;
                        }
                        else if ((Addr & XAIE_CORE_MEMORY_MODULE_DATAMEMORY) == XAIE_CORE_MEMORY_MODULE_DATAMEMORY) {
                                MemSize = XAIE_CORE_DATA_MEMORY_SIZE;
                                MemAddr = XAIE_CORE_MEMORY_MODULE_DATAMEMORY;
                        }
			break;
		case XAIEGBL_TILE_TYPE_MEMTILE:
			MemSize = XAIE_MEM_TILE_MEMORY_SIZE;
			MemAddr = XAIE_MEM_TILE_MEMORY_MODULE_DATAMEMORY;
			break;
		case XAIEGBL_TILE_TYPE_SHIMNOC:
			#if DEV_GEN_AIE4
			if((Addr & XAIE_UC_PRIVATE_DATA_MEMORY) == XAIE_UC_PRIVATE_DATA_MEMORY) {
				MemSize = XAIE_UC_PRIVATE_DATA_MEMORY_SIZE;
				MemAddr = XAIE_UC_PRIVATE_DATA_MEMORY;
			}
			else if((Addr & XAIE_UC_SHARED_DATA_MEMORY) == XAIE_UC_SHARED_DATA_MEMORY) {
				MemSize = XAIE_UC_SHARED_DATA_MEMORY_SIZE;
				MemAddr = XAIE_UC_SHARED_DATA_MEMORY;
			}
			else if((Addr & XAIE_UC_PROGRAM_MEMORY) == XAIE_UC_PROGRAM_MEMORY) {
				MemSize = XAIE_UC_PROGRAM_MEMORY_SIZE;
				MemAddr = XAIE_UC_PROGRAM_MEMORY;
			}
			break;
			#else
				return XAIE_INVALID_TILE;
			#endif
	}
	Addr = (~MemAddr) & Addr;
	/* Check for any size overflow */

	if((u64)Addr + Size > MemSize) {
		XAIE_ERROR("Size of read block overflows tile data memory\n");
		return XAIE_ERR_OUTOFBOUND;
	}

	/* Absolute 4-byte aligned AXI-MM address to write */
	DmAddrRoundDown = (u64)(MemAddr + XAIE_MEM_WORD_ROUND_DOWN(Addr)) +
		_XAie_LGetTileAddr(Loc.Row, Loc.Col);

	/* Round-up unaligned Addr */
	DmAddrRoundUp = (u64)(MemAddr + XAIE_MEM_WORD_ROUND_UP(Addr)) +
		_XAie_LGetTileAddr(Loc.Row, Loc.Col);

	/* First unaligned byte read into destination block */
	if(FirstReadOffset) {

		TempWord = _XAie_LPartRead32(DevInst, DmAddrRoundDown);

		for(u32 UnalignedByte = FirstReadOffset;
				(u8)((UnalignedByte < XAIE_MEM_WORD_ALIGN_SIZE) != 0U) && (RemBytes != 0U);
				UnalignedByte++, RemBytes--) {
			CharDst[BytePtr++] = (u8)(TempWord >> (UnalignedByte * 8U) &
					0xFFU);
		}

	}
	/* Aligned bytes */
	for(u32 AlignedWord = 0; AlignedWord < RemBytes / 4U;
			AlignedWord++, BytePtr += 4U, DmAddrRoundUp += 4U) {
		*((u32 *)(CharDst + BytePtr)) =  _XAie_LPartRead32(DevInst, DmAddrRoundUp);
	}

	/* Remaining bytes */
	if(RemBytes % XAIE_MEM_WORD_ALIGN_SIZE) {
		TempWord = _XAie_LPartRead32(DevInst, DmAddrRoundDown);

		for(u32 UnalignedByte = 0;
				UnalignedByte < RemBytes % XAIE_MEM_WORD_ALIGN_SIZE;
				UnalignedByte++) {
			CharDst[BytePtr++] = (u8)(TempWord >> (UnalignedByte * 8U) &
					0xFFU);
		}

	}
	return XAIE_OK;
}

#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE && XAIE_FEATURE_LITE */
/** @} */
