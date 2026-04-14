/******************************************************************************
* Copyright (C) 2023-2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_cdo.c
* @{
*
* This file contains the data structures and routines for low level IO
* operations for controlcode backend.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who        Date     Changes
* ----- ------     -------- -----------------------------------------------------
* 1.0   Sankarji   03/08/2023 Initial creation.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaie_io_common.h"
#include "xaie_io_privilege.h"
#include "xaie_npi.h"
#include "isa_stubs.h"
#include "xaie_locks.h"
#include "xaie_helper_internal.h"

#ifdef __AIECONTROLCODE__

/* Define ssize_t for cross-platform compatibility */
#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

#define TEMP_ASM_FILE1    ".temp_data1.txt"
#define TEMP_ASM_FILE2    ".temp_data2.txt"
#define TEMP_ASM_FILE3    ".temp_data3.txt"
#define TEMP_ASM_FILE4    ".temp_data4.txt"
#define PAGE_SIZE_MAX	  8192
#define SHIM_BD_NUM_REGS  9
#define MAX_LABELS_PER_ASM_FILE 1000
#define FNAME_SIZE 100
#define HASH_INVALID -1
#define MAX_REMOTE_BARRIER_ID 7
#define MAX_COMMENT_LENGTH 128
#define BLOCKWRITE32 0
#define WRITE32 1

#define EXTRACT_LOWER_FOUR_BYTES(RegOff) (u32)(RegOff & UINT32_MAX)

/* Helper macro to safely call fprintf only when file pointer is non-NULL */
#define SAFE_FPRINTF(fp, ...) \
	do { \
		if (fp) \
			fprintf(fp, __VA_ARGS__); \
	} while(0)

#define SAFE_FSEEK(fp, offset, whence) \
	do { \
		if (fp) \
			fseek(fp, offset, whence); \
	} while(0)

/* Helper macro to check for critical errors in ControlCodeInst */
#define CHECK_ERROR_STATE(inst) \
	do { \
		if ((inst)->ErrorState) { \
			XAIE_ERROR("Operation failed due to previous critical error\n"); \
			return XAIE_ERR; \
		} \
	} while(0)

/* 
 * Helper macro that combines _XAie_ControlCodePrintf with automatic error checking.
 * This ensures critical errors (like memory allocation failures) are caught immediately.
 * Use this macro instead of calling _XAie_ControlCodePrintf directly in functions that return AieRC.
 */
#define CONTROLCODE_PRINTF_CHECK(inst, target, ...) \
	do { \
		_XAie_ControlCodePrintf(inst, target, __VA_ARGS__); \
		CHECK_ERROR_STATE(inst); \
	} while(0)

/*
 * Void-safe variant of CONTROLCODE_PRINTF_CHECK for functions that return void.
 * Calls _XAie_ControlCodePrintf and checks ErrorState, logging errors and returning early.
 */
#define CONTROLCODE_PRINTF_VOID(inst, target, ...) \
	do { \
		_XAie_ControlCodePrintf(inst, target, __VA_ARGS__); \
		if ((inst)->ErrorState) { \
			XAIE_ERROR("Critical error in void function - ErrorState set after printf\n"); \
			return; \
		} \
	} while(0)

/*
 * ERROR HANDLING STRATEGY:
 * 
 * 1. _XAie_ControlCodePrintf() can fail on critical errors (e.g., memory allocation).
 *    When this happens, it sets ControlCodeInst->ErrorState = 1 and returns -1.
 * 
 * 2. For functions returning AieRC:
 *    - Use CONTROLCODE_PRINTF_CHECK() macro for automatic error propagation
 *    - This macro calls _XAie_ControlCodePrintf() then CHECK_ERROR_STATE()
 *    - If ErrorState is set, the function returns XAIE_ERR immediately
 * 
 * 3. For void functions:
 *    - Use CONTROLCODE_PRINTF_VOID() macro for automatic error handling
 *    - This macro calls _XAie_ControlCodePrintf() and returns early if ErrorState is set
 * 
 * 4. assert() is NOT used because:
 *    - Only active in debug builds
 *    - Doesn't provide runtime error handling in production
 *    - ErrorState + CHECK_ERROR_STATE provides better error propagation
 */

//#define UC_DMA_DATASZ					4
//#define DATA_SECTION_ALIGNMENT        16

/*#define START_JOB_OPSZ				8
#define END_JOB_OPSZ					4
#define EOF_OPSZ						4
#define UC_DMA_WRITE_DES_SYNC_OPSZ		4
#define MASK_WRITE32_OPSZ				16
#define WRITE_32_DATA_OPSZ				12
#define UC_DMA_BD_OPSZ					16
#define UC_DMA_DATASZ					4
#define DATA_SECTION_ALIGNMENT          16
#define HEADER_SIZE						16*/

/* In-memory buffer constants */
#define INITIAL_BUFFER_SIZE 8192
#define BUFFER_GROWTH_FACTOR 2

/************************** Constant Definitions *****************************/
char FName[FNAME_SIZE];

/****************************** Type Definitions *****************************/

/**
 * XAie_FileTarget - Enumeration for file/buffer targets
 */
typedef enum {
	XAIE_FILE_TARGET_CONTROLCODE = 0,
	XAIE_FILE_TARGET_CONTROLCODEDATA = 1,
	XAIE_FILE_TARGET_CONTROLCODEDATA2 = 2,
	XAIE_FILE_TARGET_DEBUGASM = 3,
	XAIE_FILE_TARGET_DEBUGASMDATA0 = 4,
	XAIE_FILE_TARGET_DEBUGASMDATA1 = 5
} XAie_FileTarget;

/**
 * XAie_MemBuffer - Dynamic memory buffer for in-memory ASM generation
 */
typedef struct {
	char *Data;          /* Buffer data */
	size_t Size;         /* Current size of data in buffer */
	size_t Capacity;     /* Allocated capacity */
} XAie_MemBuffer;

typedef struct {
	u32 *BWDataSizes;
	int *HashBW;
	int *HashW;
} XAie_LabelMap;

typedef struct {
	XAie_DevInst *DevInst;
	u64 BaseAddr;
	u64 NpiBaseAddr;
	FILE *ControlCodefp;
	FILE *ControlCodedatafp;
	FILE *ControlCodedata2fp;
	FILE* DebugAsmFile;
	FILE* DebugAsmFileData0;
	FILE* DebugAsmFileData1;
	/* In-memory buffers */
	XAie_MemBuffer *ControlCodeBuf;
	XAie_MemBuffer *ControlCodeDataBuf;
	XAie_MemBuffer *ControlCodeData2Buf;
	XAie_MemBuffer *DebugAsmBuf;
	XAie_MemBuffer *DebugAsmDataBuf0;
	XAie_MemBuffer *DebugAsmDataBuf1;
	u8   UseInMemoryBuffers;  /* Flag to indicate in-memory mode */
	u8   ErrorState;          /* Flag to track critical errors (1 = error occurred) */
	u8   DisableDebugAsm;     /* Flag to disable .DEBUG file generation (0 = enabled, 1 = disabled) */
	u32  UcbdLabelNum;
	u32  UcbdDataNum;
	u32  UcDmaDataNum;
	u32  UcJobNum;
	u32  UcPageSize;
	u32  UcPageTextSize;
	u32  PageSizeMax;
	u32  NumShimBDsChained;
	u32  DataAligner;
	u8   CombineCommands;
	u8   IsJobOpen;
	u8   IsPageOpen;
	u8   IsShimBd;
	u8   Mode;
	u8   IsAdjacentMemWrite;
	u32  CombinedMemWriteSize;
	u64  CalculatedNextRegOff;
	char *ScrachpadName;
	u8 PageBreak;
	XAie_LabelMap* LabelMap;
	int CompareLabelUpto;
	int CompareLabelUptoWrite;
	u64 TotalLabelsAllocated;
	u64 TotalLabelsAllocatedWrite;
	u32 PageId;
	u32 CurrentDataLabel;
	u32 CurrentDataBWLabel;
	u8 LabelMatchFound;
	int PrevMemWriteType;
	u64 BarrierId;
	u32 HintMapId;
} XAie_ControlCodeIO;

/************************** Function Definitions *****************************/

/* Forward declarations */
static void _XAie_MergeFiles(FILE *SrcFp, FILE *DesFp);
static int _XAie_ControlCodeSeekAndOverwrite(XAie_ControlCodeIO *ControlCodeInst,
		XAie_FileTarget FileTarget, long Offset, const char *Replacement);

/*****************************************************************************/
/**
*
* This is a printf-like function for writing to control code buffers/files.
* On critical errors (e.g., memory allocation failures), this function sets
* the ErrorState flag in ControlCodeInst and returns -1. Callers should check
* ControlCodeInst->ErrorState at appropriate points to detect and handle errors.
*
* @param        ControlCodeInst: ControlCode instance pointer
* @param        FileTarget: Target file/buffer (use XAie_FileTarget enum)
* @param        fmt: Format string
* @param        ...: Variable arguments
*
* @return       Number of bytes written on success, or -1 on error.
*               On critical errors, also sets ControlCodeInst->ErrorState = 1.
*
* @note         Internal only.
*               For functions returning AieRC, prefer using CONTROLCODE_PRINTF_CHECK macro
*               which automatically checks ErrorState after the call.
*
*******************************************************************************/
static int _XAie_ControlCodePrintf(XAie_ControlCodeIO *ControlCodeInst, XAie_FileTarget FileTarget,
						   const char *fmt, ...);

/*****************************************************************************/
/**
*
* This function computes hash for given array of Data
*
* @param        Data: Pointer to Data Payload struct
*
* @param        Count: Number of elements in the Data array
*
* @return       u32 Hash Value.
*
* @note         Internal only.
*
*******************************************************************************/

u32 _XAie_ComputeHash(const u32* Data, u32 Count) {
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    const uint32_t r1 = 15;
    const uint32_t r2 = 13;
    const uint32_t m = 5;
    const uint32_t n = 0xe6546b64;

    uint32_t hash = 5;
    u32 i;

    // Process blocks of four bytes (uint32_t)
    for (i = 0; i < Count; i++) {
        uint32_t k = Data[i];
        k *= c1;
        k = (k << r1) | (k >> (32 - r1)); // ROTL32(k, r1)
        k *= c2;

        hash ^= k;
        hash = ( (hash << r2) | (hash >> (32 - r2)) ) * m + n; // ROTL32(hash, r2)
    }

    // Finalize the hash with the length of the data
    hash ^= Count * sizeof(uint32_t);
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;

    return hash;
}

/*****************************************************************************/
/**
*
* This function initializes a memory buffer
*
* @param        Buf: Pointer to XAie_MemBuffer structure
*
* @return       Pointer to initialized buffer or NULL on failure
*
* @note         Internal only.
*
*******************************************************************************/
static XAie_MemBuffer* _XAie_MemBufferInit(void)
{
	XAie_MemBuffer *Buf = (XAie_MemBuffer*)calloc(1, sizeof(XAie_MemBuffer));
	if (!Buf) {
		return NULL;
	}
	
	Buf->Data = (char*)malloc(INITIAL_BUFFER_SIZE);
	if (!Buf->Data) {
		free(Buf);
		return NULL;
	}
	
	Buf->Capacity = INITIAL_BUFFER_SIZE;
	Buf->Size = 0;
	Buf->Data[0] = '\0';
	
	return Buf;
}

/*****************************************************************************/
/**
*
* This function frees a memory buffer
*
* @param        Buf: Pointer to XAie_MemBuffer structure
*
* @return       None
*
* @note         Internal only.
*
*******************************************************************************/
static void _XAie_MemBufferFree(XAie_MemBuffer *Buf)
{
	if (Buf) {
		if (Buf->Data) {
			free(Buf->Data);
		}
		free(Buf);
	}
}

/*****************************************************************************/
/**
*
* This function allocates memory for LabelMap Struct
*
* @param        Map: XAie_LabelMap structure pointer
*
* @return       XAIE_OK on success.
*
* @note         Internal only.
*
*******************************************************************************/
static AieRC _XAie_LabelMapSetup(XAie_LabelMap *Map, XAie_ControlCodeIO *ControlCodeInst)
{
	if(Map) {
		Map->HashW = (int*)calloc(ControlCodeInst->TotalLabelsAllocatedWrite, sizeof(int));
		Map->BWDataSizes = (u32*)calloc(ControlCodeInst->TotalLabelsAllocated, sizeof(u32));
		Map->HashBW = (int*)calloc(ControlCodeInst->TotalLabelsAllocated, sizeof(int));
	}
	if(Map->BWDataSizes && Map->HashBW && Map->HashW) {
		return XAIE_OK;
	}
	return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This function frees memory for LabelMap Struct
*
* @param        Map: XAie_LabelMap structure pointer
*
* @return       XAIE_OK on success.
*
* @note         Internal only.
*
*******************************************************************************/
static AieRC _XAie_LabelMapTeardown(XAie_LabelMap *Map)
{
	if(Map->BWDataSizes && Map->HashBW && Map->HashW) {
		free(Map->BWDataSizes);
		Map->BWDataSizes = NULL;
		free(Map->HashW);
		Map->HashW = NULL;
		free(Map->HashBW);
		Map->HashBW = NULL;
		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* Helper function to write formatted output to either FILE or memory buffer
*
* @param        ControlCodeInst: Control code instance
* @param        FileTarget: Which file/buffer to write to (0-5)
*                           0: ControlCodefp/ControlCodeBuf
*                           1: ControlCodedatafp/ControlCodeDataBuf
*                           2: ControlCodedata2fp/ControlCodeData2Buf
*                           3: DebugAsmFile/DebugAsmBuf
*                           4: DebugAsmFileData0/DebugAsmDataBuf0
*                           5: DebugAsmFileData1/DebugAsmDataBuf1
* @param        fmt: Format string
* @param        ...: Variable arguments
*
* @return       Number of characters written or negative on error
*
* @note         Internal only.
*
*******************************************************************************/
static int _XAie_ControlCodePrintf(XAie_ControlCodeIO *ControlCodeInst, XAie_FileTarget FileTarget, const char *fmt, ...) {
	va_list args;
	va_list args_file;
	int result = 0;

	if (!ControlCodeInst || !fmt) {
		return -1;
	}

	if (ControlCodeInst->DisableDebugAsm &&
	    (FileTarget == XAIE_FILE_TARGET_DEBUGASM ||
	     FileTarget == XAIE_FILE_TARGET_DEBUGASMDATA0 ||
	     FileTarget == XAIE_FILE_TARGET_DEBUGASMDATA1)) {
		return 0;
	}

	va_start(args, fmt);

	/* Write to buffer if in memory mode */
	if (ControlCodeInst->UseInMemoryBuffers) {
		XAie_MemBuffer *buf = NULL;

		XAIE_DBG("_XAie_ControlCodePrintf: FileTarget=%d, UseInMemoryBuffers=1\n", FileTarget);

		switch(FileTarget) {
			case XAIE_FILE_TARGET_CONTROLCODE: buf = ControlCodeInst->ControlCodeBuf; break;
			case XAIE_FILE_TARGET_CONTROLCODEDATA: buf = ControlCodeInst->ControlCodeDataBuf; break;
			case XAIE_FILE_TARGET_CONTROLCODEDATA2: buf = ControlCodeInst->ControlCodeData2Buf; break;
			case XAIE_FILE_TARGET_DEBUGASM: buf = ControlCodeInst->DebugAsmBuf; break;
			case XAIE_FILE_TARGET_DEBUGASMDATA0: buf = ControlCodeInst->DebugAsmDataBuf0; break;
			case XAIE_FILE_TARGET_DEBUGASMDATA1: buf = ControlCodeInst->DebugAsmDataBuf1; break;
			default:
				va_end(args);
				return -1;
		}

		if (!buf) {
			XAIE_ERROR("Buffer is NULL for FileTarget=%d\n", FileTarget);
			va_end(args);
			return -1;
		}

		XAIE_DBG("_XAie_ControlCodePrintf: buf=%p, Size=%zu, Capacity=%zu\n",
				 (void*)buf, buf->Size, buf->Capacity);

		/* Try to write with current capacity */
		size_t available = buf->Capacity - buf->Size;
		XAIE_DBG("_XAie_ControlCodePrintf: Attempting write, available=%zu\n", available);

		/* Copy args for this vsnprintf() call (va_list can't be reused after consumption). */
		va_list args_copy;
		va_copy(args_copy, args);
		result = vsnprintf(buf->Data + buf->Size, available, fmt, args_copy);
		va_end(args_copy);
		XAIE_DBG("_XAie_ControlCodePrintf: vsnprintf returned %d\n", result);

		/* If buffer was too small, grow it and retry */
		if (result >= 0 && (size_t)result >= available) {
			XAIE_DBG("_XAie_ControlCodePrintf: Output truncated, need %d bytes, growing buffer\n", result);
			size_t needed = result + 1;  /* +1 for null terminator */

			/* Grow buffer to accommodate */
			size_t new_capacity = buf->Capacity;
			while (buf->Size + needed > new_capacity) {
				new_capacity *= BUFFER_GROWTH_FACTOR;
				if (new_capacity <= buf->Capacity) {
					XAIE_ERROR("Buffer capacity overflow\n");
					ControlCodeInst->ErrorState = 1;
					va_end(args_copy);
					va_end(args);
					return -1;
				}
			}

			XAIE_DBG("_XAie_ControlCodePrintf: Growing buffer from %zu to %zu\n",
					 buf->Capacity, new_capacity);
			char *new_data = (char*)realloc(buf->Data, new_capacity);
			if (!new_data) {
				XAIE_ERROR("Buffer realloc failed (size: %zu)\n", new_capacity);
				ControlCodeInst->ErrorState = 1;
				va_end(args_copy);
				va_end(args);
				return -1;
			}
			buf->Data = new_data;
			buf->Capacity = new_capacity;

			/* Retry the write with the full buffer capacity */
			available = buf->Capacity - buf->Size;
			va_list args_copy_retry;
			va_copy(args_copy_retry, args);
			result = vsnprintf(buf->Data + buf->Size, available, fmt, args_copy_retry);
			va_end(args_copy_retry);
			XAIE_DBG("_XAie_ControlCodePrintf: After grow, vsnprintf returned %d\n", result);
		}

		if (result >= 0) {
			buf->Size += result;
			XAIE_DBG("_XAie_ControlCodePrintf: Success, new buf->Size=%zu\n", buf->Size);
		}
	}

	/* Write to file if file pointer is available (supports parallel file + memory mode) */
	FILE *fp = NULL;

	switch(FileTarget) {
		case XAIE_FILE_TARGET_CONTROLCODE: fp = ControlCodeInst->ControlCodefp; break;
		case XAIE_FILE_TARGET_CONTROLCODEDATA: fp = ControlCodeInst->ControlCodedatafp; break;
		case XAIE_FILE_TARGET_CONTROLCODEDATA2: fp = ControlCodeInst->ControlCodedata2fp; break;
		case XAIE_FILE_TARGET_DEBUGASM: 
			fp = ControlCodeInst->DebugAsmFile; 
			break;
		case XAIE_FILE_TARGET_DEBUGASMDATA0: fp = ControlCodeInst->DebugAsmFileData0; break;
		case XAIE_FILE_TARGET_DEBUGASMDATA1: fp = ControlCodeInst->DebugAsmFileData1; break;
		default:
			va_end(args);
			return result > 0 ? result : -1;
	}

	if (fp) {
		va_copy(args_file, args);
		int file_result = vfprintf(fp, fmt, args_file);
		va_end(args_file);
		/* Use file result if buffer wasn't written */
		if (!ControlCodeInst->UseInMemoryBuffers) {
			result = file_result;
		}
	}

	va_end(args);
	return result;
}

/*****************************************************************************/
/**
*
* Helper function to seek back and overwrite content in memory buffer or file.
* This replaces SAFE_FSEEK + printf pattern for in-memory mode.
*
* @param	ControlCodeInst: Control code instance pointer.
* @param	FileTarget: Target buffer/file identifier (0-5).
* @param	Offset: Negative offset from current position (e.g., -3 to go back 3 chars).
* @param	Replacement: String to write at the seeked position.
*
* @return	Number of characters written, or -1 on error.
*
* @note		Internal API only.
*
******************************************************************************/
static int _XAie_ControlCodeSeekAndOverwrite(XAie_ControlCodeIO *ControlCodeInst,
		XAie_FileTarget FileTarget, long Offset, const char *Replacement)
{
	if (!ControlCodeInst || !Replacement) {
		return -1;
	}

	if (ControlCodeInst->DisableDebugAsm &&
	    (FileTarget == XAIE_FILE_TARGET_DEBUGASM ||
	     FileTarget == XAIE_FILE_TARGET_DEBUGASMDATA0 ||
	     FileTarget == XAIE_FILE_TARGET_DEBUGASMDATA1)) {
		return 0;
	}
	
	size_t rep_len = strlen(Replacement);
	int result = 0;

	/* Write to buffer if in memory mode */
	if (ControlCodeInst->UseInMemoryBuffers) {
		XAie_MemBuffer *buf = NULL;

		switch(FileTarget) {
			case XAIE_FILE_TARGET_CONTROLCODE: buf = ControlCodeInst->ControlCodeBuf; break;
			case XAIE_FILE_TARGET_CONTROLCODEDATA: buf = ControlCodeInst->ControlCodeDataBuf; break;
			case XAIE_FILE_TARGET_CONTROLCODEDATA2: buf = ControlCodeInst->ControlCodeData2Buf; break;
			case XAIE_FILE_TARGET_DEBUGASM: buf = ControlCodeInst->DebugAsmBuf; break;
			case XAIE_FILE_TARGET_DEBUGASMDATA0: buf = ControlCodeInst->DebugAsmDataBuf0; break;
			case XAIE_FILE_TARGET_DEBUGASMDATA1: buf = ControlCodeInst->DebugAsmDataBuf1; break;
			default: return -1;
		}

		if (!buf || !buf->Data) {
			return -1;
		}

		/* Calculate the position to overwrite */
		ssize_t overwrite_pos = (ssize_t)buf->Size + Offset;

		if (overwrite_pos < 0 || overwrite_pos >= (ssize_t)buf->Size) {
			XAIE_ERROR("Invalid seek position: %zd (buf size: %zu)\n", overwrite_pos, buf->Size);
			return -1;
		}

		/* Overwrite at the calculated position */
		size_t bytes_to_write = rep_len;
		size_t available = buf->Size - overwrite_pos;

		if (bytes_to_write > available) {
			XAIE_WARN("Replacement string longer than available space, truncating\n");
			bytes_to_write = available;
		}

		memcpy(buf->Data + overwrite_pos, Replacement, bytes_to_write);
		result = (int)bytes_to_write;
	}

	/* Write to file if file pointer is available (supports parallel file + memory mode) */
	FILE *fp = NULL;

	switch(FileTarget) {
		case XAIE_FILE_TARGET_CONTROLCODE: fp = ControlCodeInst->ControlCodefp; break;
		case XAIE_FILE_TARGET_CONTROLCODEDATA: fp = ControlCodeInst->ControlCodedatafp; break;
		case XAIE_FILE_TARGET_CONTROLCODEDATA2: fp = ControlCodeInst->ControlCodedata2fp; break;
		case XAIE_FILE_TARGET_DEBUGASM: fp = ControlCodeInst->DebugAsmFile; break;
		case XAIE_FILE_TARGET_DEBUGASMDATA0: fp = ControlCodeInst->DebugAsmFileData0; break;
		case XAIE_FILE_TARGET_DEBUGASMDATA1: fp = ControlCodeInst->DebugAsmFileData1; break;
		default: return result > 0 ? result : -1;
	}

	if (fp) {
		/* For file mode, use fseek and fprintf */
		if (fseek(fp, Offset, SEEK_CUR) == 0) {
			int file_result = fprintf(fp, "%s", Replacement);
			/* Use file result if buffer wasn't written */
			if (!ControlCodeInst->UseInMemoryBuffers) {
				result = file_result;
			}
		}
	}

	return result;
}

/*****************************************************************************/
/**
*
* This is the function to add annotation in asm code.
*
* @param        Fp: File Pointer
* @param        PageId: Serves as Page Number
* @param        PageSize: Serves as PageSize

* @return       XAIE_OK or XAIE_ERR.
*
*
*******************************************************************************/
AieRC _XAie_ControlCodePageInfo(FILE *Fp, u32 PageId, u32 PageSize,
								u32 TextSecSize, u32 DataAligner)
{
        if (Fp != NULL) {
                fprintf(Fp,";Page#: %d, PageSize: %d, TextSecSize: %d, DataAligner: %d\n",
					 PageId,PageSize, TextSecSize, DataAligner);
                return XAIE_OK;
        }
        else
                return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This function is used to add a custom comment in the control code ASM file.
*
* @param        DevInst: Device instance pointer
* @param        Comment: Custom comment string to be inserted
*
* @return       XAIE_OK or XAIE_ERR.
*
* @note         Maximum comment length is MAX_COMMENT_LENGTH characters.
*
*******************************************************************************/
AieRC XAie_ControlCodeAddComment(XAie_DevInst *DevInst, const char *Comment)
{
	if(DevInst->Backend->Type != XAIE_IO_BACKEND_CONTROLCODE) {
			XAIE_ERROR("This is supported only in Controlcode Backend %d \n", DevInst->Backend->Type);
			return XAIE_INVALID_BACKEND;
	}

	if(Comment == NULL) {
		XAIE_ERROR("Comment string cannot be NULL\n");
		return XAIE_ERR;
	}

	size_t CommentLen = strlen(Comment);
	if(CommentLen == 0 || CommentLen > MAX_COMMENT_LENGTH) {
		XAIE_ERROR("Comment string must be non-empty and not exceed %d characters (provided: %zu)\n", 
			MAX_COMMENT_LENGTH, CommentLen);
		return XAIE_ERR;
	}

	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

	if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "; %s\n", Comment);
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "; %s\n", Comment);
		return XAIE_OK;
	}
	else {
		XAIE_ERROR("Control code file pointer is NULL\n");
		return XAIE_ERR;
	}
}

/*****************************************************************************/
/**
*
* This is the function to add annotation in asm code.
*
* @param        DevInst: Device instance pointer
* @param        Id: Serves as a key to a dynamic tracing probe.
* @param        Name: Serves as a key to look up interested annotations.
* @param        Description: Serves as comments.

* @return       XAIE_OK or XAIE_ERR.
*
*
*******************************************************************************/
AieRC XAie_ControlCodeAddAnnotation(XAie_DevInst *DevInst,
               u32 Id, const char *Name, const char *Description)
{
        if(DevInst->Backend->Type != XAIE_IO_BACKEND_CONTROLCODE) {
                XAIE_ERROR("This is supported only in Controlcode Backend %d \n", DevInst->Backend->Type);
                return XAIE_INVALID_BACKEND;
        }

        XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

        if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
                CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ".section annotation\n");
                CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "id: %d\n", Id);
                CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "name: %s\n", Name);
                CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "description: %s\n", Description);
                CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ".section annotation\n");
                CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "id: %d\n", Id);
                CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "name: %s\n", Name);
                CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "description: %s\n", Description);
                CHECK_ERROR_STATE(ControlCodeInst);
                return XAIE_OK;
        }
        else
                return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This function allows to modify the length of data in the DMAWRITE command.
*
* @param        ControlCodeInst: ControlCode instance pointer
*
* @param        Datalength     : Value to update the length of data
*
* @return       XAIE_OK on success.
*
* @note         Internal only.
*
*******************************************************************************/
static AieRC _XAie_UpdateDataLengthDmaBd(XAie_ControlCodeIO *ControlCodeInst, u32 Datalength)
{
	/* Handle in-memory mode (if buffers are available) */
	if (ControlCodeInst->UseInMemoryBuffers) {
		/* Update ControlCodeDataBuf */
		if (ControlCodeInst->ControlCodeDataBuf && ControlCodeInst->ControlCodeDataBuf->Data) {
			char *data = ControlCodeInst->ControlCodeDataBuf->Data;
			size_t size = ControlCodeInst->ControlCodeDataBuf->Size;
			long Position = size - 1;
			int Count = 0;

			/* Search backwards for the 3rd comma from the end */
			while (Position >= 0) {
				if (data[Position] == ',')
					Count++;

				if (Count == 3) {
					/* Found the position - now update the word count */
					char new_ending[64];
					snprintf(new_ending, sizeof(new_ending), " 0x%x, 0, 0\n", Datalength);

					/* Find the newline after this position */
					size_t newline_pos = Position + 1;
					while (newline_pos < size && data[newline_pos] != '\n') {
						newline_pos++;
					}

					if (newline_pos < size) {
						/* Update the buffer - keep everything up to Position+1, replace with new ending */
						size_t new_size = Position + 1 + strlen(new_ending);
						data[Position + 1] = '\0'; /* Temporarily terminate */

						/* Ensure we have enough capacity */
						if (new_size <= ControlCodeInst->ControlCodeDataBuf->Capacity) {
							strcpy(data + Position + 1, new_ending);
							ControlCodeInst->ControlCodeDataBuf->Size = new_size;
						}
					}
					break;
				}

				if (data[Position] == '\n' && Position != (long)(size - 1))
					break;

				Position--;
			}
		}

		/* Update DebugAsmDataBuf0 */
		if (ControlCodeInst->DebugAsmDataBuf0 && ControlCodeInst->DebugAsmDataBuf0->Data) {
			char *data = ControlCodeInst->DebugAsmDataBuf0->Data;
			size_t size = ControlCodeInst->DebugAsmDataBuf0->Size;
			long Position = size - 1;
			int Count = 0;

			/* Search backwards for the 3rd comma from the end */
			while (Position >= 0) {
				if (data[Position] == ',')
					Count++;

				if (Count == 3) {
					/* Found the position - now update the word count */
					char new_ending[64];
					snprintf(new_ending, sizeof(new_ending), " 0x%x, 0, 0\n", Datalength);

					/* Find the newline after this position */
					size_t newline_pos = Position + 1;
					while (newline_pos < size && data[newline_pos] != '\n') {
						newline_pos++;
					}

					if (newline_pos < size) {
						/* Update the buffer - keep everything up to Position+1, replace with new ending */
						size_t new_size = Position + 1 + strlen(new_ending);
						data[Position + 1] = '\0'; /* Temporarily terminate */

						/* Ensure we have enough capacity */
						if (new_size <= ControlCodeInst->DebugAsmDataBuf0->Capacity) {
							strcpy(data + Position + 1, new_ending);
							ControlCodeInst->DebugAsmDataBuf0->Size = new_size;
						}
					}
					break;
				}

				if (data[Position] == '\n' && Position != (long)(size - 1))
					break;

				Position--;
			}
		}
	}
	/* Handle file mode (works in both file-only and dual mode) */
	if (ControlCodeInst->ControlCodedatafp) {
		long FileSize = ftell(ControlCodeInst->ControlCodedatafp);
		long Position = FileSize - 1;
		int Count = 0;
		int Data;

		SAFE_FSEEK(ControlCodeInst->ControlCodedatafp, 0, SEEK_END);

		while (Position >= 0U) {
			SAFE_FSEEK(ControlCodeInst->ControlCodedatafp, Position, SEEK_SET);
			Data = fgetc(ControlCodeInst->ControlCodedatafp);

			if (Data == ',')
				Count++;

			if (Count == 3U) {
				SAFE_FSEEK(ControlCodeInst->ControlCodedatafp, Position + 1, SEEK_SET);
				fprintf(ControlCodeInst->ControlCodedatafp, " 0x%x, 0, 0\n", Datalength);
				break;
			}

			if (Data == '\n' && Position != FileSize - 1)
				break;

			Position--;
		}

		SAFE_FSEEK(ControlCodeInst->ControlCodedatafp, 0, SEEK_END);
	}

	if (!ControlCodeInst->DisableDebugAsm && ControlCodeInst->DebugAsmFileData0) {
		long FileSize = ftell(ControlCodeInst->DebugAsmFileData0);
		long Position = FileSize - 1;
		int Count = 0;
		char Data;

		SAFE_FSEEK(ControlCodeInst->DebugAsmFileData0, 0, SEEK_END);

		while (Position >= 0U) {
			SAFE_FSEEK(ControlCodeInst->DebugAsmFileData0, Position, SEEK_SET);
			Data = fgetc(ControlCodeInst->DebugAsmFileData0);

			if (Data == ',')
				Count++;

			if (Count == 3U) {
				SAFE_FSEEK(ControlCodeInst->DebugAsmFileData0, Position + 1, SEEK_SET);
				fprintf(ControlCodeInst->DebugAsmFileData0, " 0x%x, 0, 0\n", Datalength);
				break;
			}

			if (Data == '\n' && Position != FileSize - 1)
				break;

			Position--;
		}

		SAFE_FSEEK(ControlCodeInst->DebugAsmFileData0, 0, SEEK_END);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to free the global IO instance
*
* @param	IOInst: IO Instance pointer.
*
* @return	None.
*
* @note		The global IO instance is a singleton and freed when
* the reference count reaches a zero. Internal only.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_Finish(void *IOInst)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	XAie_DevInst *DevInst = ControlCodeInst->DevInst;
        if(ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
                XAie_CloseControlCodeFile(DevInst);
        }

	if(IOInst) {
		free(IOInst);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to initialize the global IO instance
*
* @param	DevInst: Device instance pointer.
*
* @return	XAIE_OK on success. Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_Init(XAie_DevInst *DevInst)
{
	XAie_ControlCodeIO *IOInst;

	IOInst = (XAie_ControlCodeIO *)calloc(1,sizeof(*IOInst));
	if(IOInst == NULL) {
		XAIE_ERROR("Memory allocation failed\n");
		return XAIE_ERR;
	}

	memset(IOInst, 0, sizeof(XAie_ControlCodeIO));
	IOInst->BaseAddr = DevInst->BaseAddr;
	IOInst->NpiBaseAddr = XAIE_NPI_BASEADDR;
	IOInst->ScrachpadName = NULL;
	IOInst->DevInst = DevInst;
	IOInst->ControlCodefp = NULL;
	IOInst->ControlCodedatafp = NULL;
	IOInst->ControlCodedata2fp = NULL;
	IOInst->ScrachpadName = NULL;

	DevInst->IOInst = IOInst;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This function ends a Control code Job
*
* @param	ControlCodeInst: ControlCodeInst instance pointer.
*
* @return	void.
*
* @note		Internal only.
*
*******************************************************************************/
static void _XAie_EndJob(XAie_ControlCodeIO  *ControlCodeInst) {

	if(ControlCodeInst->Mode == XAIE_WRITE_DES_ASYNC_ENABLE)
	{
		CONTROLCODE_PRINTF_VOID(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "WAIT_UC_DMA\t $r0\n");
		CONTROLCODE_PRINTF_VOID(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "WAIT_UC_DMA\t $r0\n");
		ControlCodeInst->UcPageTextSize += ISA_OPSIZE_WAIT_UC_DMA;
		ControlCodeInst->UcPageSize += ISA_OPSIZE_WAIT_UC_DMA;
		_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
			 ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);

	}
	else if(ControlCodeInst->NumShimBDsChained > 0) {
		CONTROLCODE_PRINTF_VOID(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE,
				"UC_DMA_WRITE_DES_SYNC\t @UCBD_label_%d\n",
				ControlCodeInst->UcbdLabelNum);
		CONTROLCODE_PRINTF_VOID(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM,
				"UC_DMA_WRITE_DES_SYNC\t @UCBD_label_%d\n",
				ControlCodeInst->UcbdLabelNum);
		ControlCodeInst->UcPageSize += ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC;
		_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
			 ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);

		ControlCodeInst->UcPageTextSize += ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC;
		ControlCodeInst->NumShimBDsChained = 0;
		ControlCodeInst->CombineCommands = 0;
		ControlCodeInst->UcbdLabelNum++;
	}
	
	if(ControlCodeInst->IsPageOpen && ControlCodeInst->IsJobOpen) {
		CONTROLCODE_PRINTF_VOID(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "END_JOB\n\n");
		XAIE_DBG("Writing END_JOB to DebugAsmFile (fp=%p)\n", (void*)ControlCodeInst->DebugAsmFile);
		CONTROLCODE_PRINTF_VOID(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "END_JOB\n\n");
		if (ControlCodeInst->DebugAsmFile) {
			fflush(ControlCodeInst->DebugAsmFile);
		}
		ControlCodeInst->IsJobOpen 		= 0;
		ControlCodeInst->CombineCommands 	= 0;
	}
}

/*****************************************************************************/
/**
*
* This function ends a Control code Page
*
* @param	ControlCodeInst: ControlCodeInst instance pointer.
*
* @return	void.
*
* @note		Internal only.
*
*******************************************************************************/
static void _XAie_EndPage(XAie_ControlCodeIO  *ControlCodeInst) {

	if(ControlCodeInst->IsPageOpen) {
		_XAie_EndJob(ControlCodeInst);
		CONTROLCODE_PRINTF_VOID(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ".eop\n\n");
		CONTROLCODE_PRINTF_VOID(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ".eop\n\n");

		ControlCodeInst->UcPageSize 	= 0;
		ControlCodeInst->UcPageTextSize = 0;
		ControlCodeInst->IsPageOpen 	= 0;
		ControlCodeInst->UcDmaDataNum = ControlCodeInst->CurrentDataBWLabel;
		ControlCodeInst->UcbdDataNum = ControlCodeInst->CurrentDataLabel;
		ControlCodeInst->CompareLabelUpto = ControlCodeInst->UcDmaDataNum;
		ControlCodeInst->CompareLabelUptoWrite = ControlCodeInst->UcbdDataNum;
		ControlCodeInst->PageId++;
	}
}

/*****************************************************************************/
/**
*
* This function starts a new Control code Page
*
* @param	ControlCodeInst: ControlCodeInst instance pointer.
*
* @return	void.
*
* @note		Internal only.
*
*******************************************************************************/
static void _XAie_StartNewPage(XAie_ControlCodeIO  *ControlCodeInst) {

	if(ControlCodeInst->IsPageOpen) {
		_XAie_EndPage(ControlCodeInst);
	}

	ControlCodeInst->UcPageTextSize	 = ISA_OPSIZE_EOF;
	ControlCodeInst->UcPageSize 	 = PAGE_HEADER_SIZE + ISA_OPSIZE_EOF;
	ControlCodeInst->CombineCommands = 0;
	ControlCodeInst->IsPageOpen 	 = 1;
	ControlCodeInst->LabelMatchFound = 0;
}

/*****************************************************************************/
/**
*
* This function starts a new Control code Job
*
* @param	ControlCodeInst: ControlCodeInst instance pointer.
*
* @return	void.
*
* @note		Internal only.
*
*******************************************************************************/
static void _XAie_StartNewJob(XAie_ControlCodeIO  *ControlCodeInst,
	XAie_CertStartJobType JobType) {

	if (JobType > XAIE_START_COND_JOB_PREEMPT) {
		XAIE_ERROR("Invalid Job Type\n");
		return;
	}

	// no open page, create one
	if (!ControlCodeInst->IsPageOpen) {
		_XAie_StartNewPage(ControlCodeInst);
	}

	ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT -
		((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_START_JOB + ISA_OPSIZE_END_JOB) % DATA_SECTION_ALIGNMENT));
        if (ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT) {
                ControlCodeInst->DataAligner = 0U;
        }

	// the existing page cannot fit in a new job
	// >= to prevent an empty job
	if (ControlCodeInst->UcPageSize + ISA_OPSIZE_START_JOB + ISA_OPSIZE_END_JOB + ControlCodeInst->DataAligner >= ControlCodeInst->PageSizeMax) {
		_XAie_StartNewPage(ControlCodeInst);
	}

	// close current job on current page
	if (ControlCodeInst->IsJobOpen) {
		_XAie_EndJob(ControlCodeInst);
	}

	if (JobType == XAIE_START_COND_JOB_PREEMPT) {
		CONTROLCODE_PRINTF_VOID(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "START_COND_JOB_PREEMPT %d\n",
			ControlCodeInst->UcJobNum);
		CONTROLCODE_PRINTF_VOID(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "START_COND_JOB_PREEMPT %d\n",
			ControlCodeInst->UcJobNum);
	} else if (JobType == XAIE_START_JOB_DEFERRED) {
		CONTROLCODE_PRINTF_VOID(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "START_JOB_DEFERRED %d\n",
			ControlCodeInst->UcJobNum);
		CONTROLCODE_PRINTF_VOID(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "START_JOB_DEFERRED %d\n",
			ControlCodeInst->UcJobNum);
	} else if (JobType == XAIE_START_JOB) {
		CONTROLCODE_PRINTF_VOID(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "START_JOB %d\n",
			ControlCodeInst->UcJobNum);
		XAIE_DBG("Writing START_JOB to DebugAsmFile (fp=%p)\n", (void*)ControlCodeInst->DebugAsmFile);
		CONTROLCODE_PRINTF_VOID(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "START_JOB %d\n",
			ControlCodeInst->UcJobNum);
		if (ControlCodeInst->DebugAsmFile) {
			fflush(ControlCodeInst->DebugAsmFile);
		}
	}

	ControlCodeInst->UcPageTextSize += ISA_OPSIZE_START_JOB + ISA_OPSIZE_END_JOB;
	ControlCodeInst->UcPageSize += ISA_OPSIZE_START_JOB + ISA_OPSIZE_END_JOB;
	_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
		 ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);

	ControlCodeInst->UcJobNum++;
	ControlCodeInst->IsJobOpen = 1;
	ControlCodeInst->CombineCommands = 0;
}

/*****************************************************************************/
/**
*
* This function Configures various modes of operations to be performed.
*
* @param	IOInst: IO instance pointer
* @param	Mode: Type of Mod user wants to enable
* @return	AieRC.
*
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_ConfigMode(void *IOInst, XAie_ModeSelect Mode)
{
	if(Mode >= XAIE_INVALID_MODE) {
		XAIE_ERROR("Invalid Mode Selection\n");
		return XAIE_ERR;
	}
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	ControlCodeInst->Mode = Mode;
	switch (Mode)
	{
		case XAIE_SHIM_BD_CHAINING_ENABLE:
			{
				ControlCodeInst->CombineCommands = 0;
				ControlCodeInst->NumShimBDsChained = 0;
			}
			break;
		case XAIE_SHIM_BD_CHAINING_DISABLE:
			{
				if(ControlCodeInst->NumShimBDsChained != 0) {
					CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE,
							"UC_DMA_WRITE_DES_SYNC\t @UCBD_label_%d\n",
							ControlCodeInst->UcbdLabelNum);
					CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM,
							"UC_DMA_WRITE_DES_SYNC\t @UCBD_label_%d\n",
							ControlCodeInst->UcbdLabelNum);
					ControlCodeInst->UcPageSize += ISA_OPCODE_UC_DMA_WRITE_DES_SYNC;
				_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
						 ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);
					ControlCodeInst->NumShimBDsChained = 0;
					ControlCodeInst->UcbdLabelNum++;
				}
			}
			break;
		case XAIE_WRITE_DES_ASYNC_ENABLE:
			break;
		default:
			break;
	}
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This function returns the current mode of operation that is set.
*
* @param	IOInst: IO instance pointer
* @return	XAie_ModeSelect.
*
* @note		Internal only.
*
*******************************************************************************/
XAie_ModeSelect XAie_GetConfigMode(void *IOInst)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	return ( (XAie_ModeSelect)ControlCodeInst->Mode );
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 32bit data to the specified address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	u32 OpSize;

	XAie_LabelMap* Map = ControlCodeInst->LabelMap;


	if(ControlCodeInst->Mode == XAIE_WRITE_DES_ASYNC_ENABLE) {
		OpSize = ISA_OPSIZE_UC_DMA_WRITE_DES;
	}
	else {
		OpSize = ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC;
	}

	if((RegOff == ControlCodeInst->CalculatedNextRegOff) && (ControlCodeInst->PrevMemWriteType != -1)) {
		ControlCodeInst->IsAdjacentMemWrite = 1;
	}
	else {
		ControlCodeInst->IsAdjacentMemWrite = 0;
	}

	ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT -
		((ControlCodeInst->UcPageTextSize + (ControlCodeInst->IsAdjacentMemWrite ? 0:OpSize)) % DATA_SECTION_ALIGNMENT));
	if (ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT) {
		ControlCodeInst->DataAligner = 0U;
	}

	if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {

		if (!ControlCodeInst->IsJobOpen) {
			_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
		}

		if(ControlCodeInst->IsAdjacentMemWrite == 1 && ControlCodeInst->LabelMatchFound == 0) {
			if((ControlCodeInst->UcPageSize + UC_DMA_WORD_LEN + ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
				_XAie_StartNewPage(ControlCodeInst);
				_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
				ControlCodeInst->IsAdjacentMemWrite = 0;
			}
			else {
				_XAie_UpdateDataLengthDmaBd(ControlCodeInst, (ControlCodeInst->CombinedMemWriteSize + 1));
				if(Map) {
					if(ControlCodeInst->PrevMemWriteType == BLOCKWRITE32) {
						Map->BWDataSizes[ControlCodeInst->CurrentDataBWLabel-1] += 1;
						Map->HashBW[ControlCodeInst->CurrentDataBWLabel-1] = HASH_INVALID;
					}
					else {
						Map->HashW[ControlCodeInst->CurrentDataLabel-1] = HASH_INVALID;
					}
				}
			}
		}

		if(ControlCodeInst->IsAdjacentMemWrite == 0) {

			if((ControlCodeInst->UcPageSize + OpSize +
				UC_DMA_BD_SIZE + UC_DMA_WORD_LEN + ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
				_XAie_StartNewPage(ControlCodeInst);
				_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
			}

			if(Map) {
				if(ControlCodeInst->CurrentDataLabel == ControlCodeInst->TotalLabelsAllocatedWrite) {
					Map->HashW = (int*)realloc(Map->HashW, (ControlCodeInst->TotalLabelsAllocatedWrite*2)*sizeof(int));
					if(Map->HashW) {
						ControlCodeInst->TotalLabelsAllocatedWrite *= 2;
					}
					else {
						XAIE_ERROR("Memory allocation failed\n");
						return XAIE_ERR;
					}
				}

				for(int Label = ControlCodeInst->CurrentDataLabel-1; Label >= ControlCodeInst->CompareLabelUptoWrite; Label--) {
					if(Map->HashW[Label] == ((int)Value)) {
						ControlCodeInst->UcbdDataNum = Label;
						ControlCodeInst->LabelMatchFound = 1;
						break;
					}
				}	

				if(ControlCodeInst->LabelMatchFound == 0) {
					Map->HashW[ControlCodeInst->CurrentDataLabel] = Value;
				}
			}

			if(ControlCodeInst->CombineCommands) {
				/* Modify buffers if in memory mode */
				if (ControlCodeInst->UseInMemoryBuffers) {
					/* In memory mode, modify the last " 0" to " 1" in the buffers */
					XAie_MemBuffer *buf1 = ControlCodeInst->ControlCodeDataBuf;
					XAie_MemBuffer *buf4 = ControlCodeInst->DebugAsmDataBuf0;

					/* Find and replace the last " 0\n" with " 1\n" */
					if (buf1 && buf1->Size >= 3) {
						if (buf1->Data[buf1->Size - 2] == '0' && buf1->Data[buf1->Size - 1] == '\n') {
							buf1->Data[buf1->Size - 2] = '1';
						}
					}
					if (buf4 && buf4->Size >= 3) {
						if (buf4->Data[buf4->Size - 2] == '0' && buf4->Data[buf4->Size - 1] == '\n') {
							buf4->Data[buf4->Size - 2] = '1';
						}
					}
				}
				/* ALWAYS handle files if available (parallel flow) */
				if (ControlCodeInst->ControlCodedatafp || ControlCodeInst->DebugAsmFileData0) {
					_XAie_ControlCodeSeekAndOverwrite(ControlCodeInst, 1, -3, " 1\n");
					_XAie_ControlCodeSeekAndOverwrite(ControlCodeInst, 4, -3, " 1\n");
				}
			}
			else {
				if(ControlCodeInst->Mode == XAIE_WRITE_DES_ASYNC_ENABLE) {
					if((ControlCodeInst->UcPageSize + OpSize + ISA_OPSIZE_WAIT_UC_DMA +
						UC_DMA_BD_SIZE + UC_DMA_WORD_LEN + ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
						_XAie_StartNewPage(ControlCodeInst);
						_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
					}

					CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE,
							"UC_DMA_WRITE_DES\t $r0, @UCBD_label_%d\n",
							ControlCodeInst->UcbdLabelNum);
					CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM,
							"UC_DMA_WRITE_DES\t $r0, @UCBD_label_%d\n",
							ControlCodeInst->UcbdLabelNum);
					
				}
				else {		
					if((ControlCodeInst->UcPageSize + OpSize +
						UC_DMA_BD_SIZE + UC_DMA_WORD_LEN + ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
						_XAie_StartNewPage(ControlCodeInst);
						_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
					}

					CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE,
							"UC_DMA_WRITE_DES_SYNC\t @UCBD_label_%d\n",
							ControlCodeInst->UcbdLabelNum);
					CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM,
							"UC_DMA_WRITE_DES_SYNC\t @UCBD_label_%d\n",
							ControlCodeInst->UcbdLabelNum);
				}
				ControlCodeInst->UcPageTextSize += OpSize;
				ControlCodeInst->UcPageSize += OpSize;

				CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA, "UCBD_label_%d:\n",
						ControlCodeInst->UcbdLabelNum);
				CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA0, "UCBD_label_%d:\n",
						ControlCodeInst->UcbdLabelNum);
				ControlCodeInst->CombineCommands = 1;
				ControlCodeInst->UcbdLabelNum++;
			}

CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA,
			"\t UC_DMA_BD\t 0, 0x%x, @WRITE_data_%d, 1, 0, 0\n",
			EXTRACT_LOWER_FOUR_BYTES(RegOff),  ControlCodeInst->UcbdDataNum);
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA0,
			"\t UC_DMA_BD\t 0, 0x%x, @WRITE_data_%d, 1, 0, 0\n",
			EXTRACT_LOWER_FOUR_BYTES(RegOff),  ControlCodeInst->UcbdDataNum);

		ControlCodeInst->UcPageSize += UC_DMA_BD_SIZE;

		ControlCodeInst->UcbdDataNum = ControlCodeInst->CurrentDataLabel;
		
		if(ControlCodeInst->LabelMatchFound == 0) {
				CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA2, "WRITE_data_%d:\n",
					ControlCodeInst->UcbdDataNum);
			CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA1, "WRITE_data_%d:\n",
					ControlCodeInst->UcbdDataNum);
				ControlCodeInst->UcbdDataNum++;
				ControlCodeInst->CurrentDataLabel = ControlCodeInst->UcbdDataNum;
			}
			ControlCodeInst->PrevMemWriteType = WRITE32;
		}
		if(ControlCodeInst->LabelMatchFound == 0) {
			CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA2, "\t.long 0x%08x\n", Value);
			CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA1, "\t.long 0x%08x\n", Value);
			ControlCodeInst->UcPageSize += UC_DMA_WORD_LEN;
		}
	}

	if(ControlCodeInst->LabelMatchFound)
	{
		ControlCodeInst->CombinedMemWriteSize = 0;
		ControlCodeInst->CalculatedNextRegOff = UINT64_MAX;
	}
	else if(ControlCodeInst->IsAdjacentMemWrite == 1) {
		ControlCodeInst->CombinedMemWriteSize += 1;
		ControlCodeInst->CalculatedNextRegOff = RegOff + sizeof(Value);
	}
	else {
		ControlCodeInst->CombinedMemWriteSize = 1;
		ControlCodeInst->CalculatedNextRegOff = RegOff + sizeof(Value);
	}
	ControlCodeInst->LabelMatchFound = 0;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 32bit data from the specified address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Pointer to store the 32 bit value
*
* @return	XAIE_OK on success.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	*Data = 0U;
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write masked 32bit data to the specified
* address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Mask: Mask to be applied to Data.
* @param	Value: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT -
		((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_MASK_WRITE_32) % DATA_SECTION_ALIGNMENT));
	if (ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT) {
		ControlCodeInst->DataAligner = 0U;
	}

	if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
		if (!ControlCodeInst->IsJobOpen) {
			_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
		}

		if((ControlCodeInst->UcPageSize + ISA_OPSIZE_MASK_WRITE_32 +
			ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
			_XAie_StartNewPage(ControlCodeInst);
			_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
		}

		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "MASK_WRITE_32\t 0x%x, 0x%x, 0x%x\n",
				EXTRACT_LOWER_FOUR_BYTES(RegOff), Mask, Value );
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "MASK_WRITE_32\t 0x%x, 0x%x, 0x%x\n",
				EXTRACT_LOWER_FOUR_BYTES(RegOff), Mask, Value );
		ControlCodeInst->CombineCommands = 0;
		ControlCodeInst->UcPageSize += ISA_OPSIZE_MASK_WRITE_32;
		ControlCodeInst->UcPageTextSize += ISA_OPSIZE_MASK_WRITE_32;
		_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
			 ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);

	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to mask poll an address for a value.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Mask: Mask to be applied to Data.
* @param	Value: 32-bit value to poll for
* @param	TimeOutUs: Timeout in micro seconds.
*
* @return	XAIE_OK or XAIE_ERR.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT -
		((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_MASK_POLL_32) % DATA_SECTION_ALIGNMENT));
	if (ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT) {
		ControlCodeInst->DataAligner = 0U;
	}

	(void) TimeOutUs;
	if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
		if (!ControlCodeInst->IsJobOpen) {
			_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
		}

		if((ControlCodeInst->UcPageSize + ISA_OPSIZE_MASK_POLL_32 +
			ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
			_XAie_StartNewPage(ControlCodeInst);
			_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
		}

		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "MASK_POLL_32\t 0x%x, 0x%x, 0x%x\n",
				(u32)(EXTRACT_LOWER_FOUR_BYTES(RegOff)), Mask, Value );
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "MASK_POLL_32\t 0x%x, 0x%x, 0x%x\n",
				(u32)(EXTRACT_LOWER_FOUR_BYTES(RegOff)), Mask, Value );
		ControlCodeInst->CombineCommands = 0;
		ControlCodeInst->UcPageSize += ISA_OPSIZE_MASK_POLL_32;
		ControlCodeInst->UcPageTextSize += ISA_OPSIZE_MASK_POLL_32;
		_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
			 ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);

	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write a block of data to aie.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Pointer to the data buffer.
* @param	Size: Number of 32-bit words.
*
* @return	None.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_BlockWrite32(void *IOInst, u64 RegOff, const u32 *Data,
		u32 Size)
{
	u8 PageBreakOccured = 0;
	u32 Hash = 0;
	u32 CompletedSize = 0;
	u32 IterationSize;
	u32 TempItrSize = 0;
	u32 OpSize;
	u32 NewPagePayloadSize;
	u64 AdjustedOff = 0;
	u64 NewPageRegOff = 0;
	u64 CumilativeRegOff = 0;

	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	XAie_LabelMap* Map = ControlCodeInst->LabelMap;

	if(ControlCodeInst->Mode == XAIE_WRITE_DES_ASYNC_ENABLE) {
		OpSize = ISA_OPSIZE_UC_DMA_WRITE_DES;
	}
	else {
		OpSize = ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC;
	}

	while (Size > CompletedSize) {
		if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
			if (!ControlCodeInst->IsJobOpen) {
				_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
			}

			if((RegOff == ControlCodeInst->CalculatedNextRegOff) && 
			   (ControlCodeInst->PageBreak == 0) &&
			   (ControlCodeInst->PrevMemWriteType != -1)) {
				ControlCodeInst->IsAdjacentMemWrite = 1;
			}
			else {
				ControlCodeInst->IsAdjacentMemWrite = 0;
			}

			if(ControlCodeInst->PageBreak == 0) {
				ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT -
					((ControlCodeInst->UcPageTextSize + (ControlCodeInst->IsAdjacentMemWrite ? 0:OpSize)) % DATA_SECTION_ALIGNMENT));
				if(ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT) {
					ControlCodeInst->DataAligner = 0U;
				}
			}

			if(ControlCodeInst->IsAdjacentMemWrite == 1) {
				if((ControlCodeInst->UcPageSize + UC_DMA_WORD_LEN
							+ ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
					_XAie_StartNewPage(ControlCodeInst);
					_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
					ControlCodeInst->IsAdjacentMemWrite = 0;
				}
				if(ControlCodeInst->IsShimBd) {
					ControlCodeInst->IsAdjacentMemWrite = 0;
				}
			}

			if((ControlCodeInst->IsAdjacentMemWrite == 0) || (ControlCodeInst->PageBreak == 1)) {

				if(ControlCodeInst->Mode == XAIE_WRITE_DES_ASYNC_ENABLE) {
					if((ControlCodeInst->UcPageSize + OpSize + ISA_OPSIZE_WAIT_UC_DMA +
						UC_DMA_BD_SIZE + UC_DMA_WORD_LEN + ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
						_XAie_StartNewPage(ControlCodeInst);
						_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
					}
				}
				else {
					if((ControlCodeInst->UcPageSize + OpSize +
						UC_DMA_BD_SIZE + UC_DMA_WORD_LEN +ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
						_XAie_StartNewPage(ControlCodeInst);
						_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
					}
				}

				if(Map) {
					if(ControlCodeInst->CurrentDataBWLabel == ControlCodeInst->TotalLabelsAllocated) {
						Map->BWDataSizes = (u32*)realloc(Map->BWDataSizes, (ControlCodeInst->TotalLabelsAllocated*2)*(sizeof(u32)));
						Map->HashBW = (int*)realloc(Map->HashBW, (ControlCodeInst->TotalLabelsAllocated*2)*sizeof(int));
						if(Map->BWDataSizes && Map->HashBW) {
						ControlCodeInst->TotalLabelsAllocated *= 2;
						}
						else {
							XAIE_ERROR("Memory allocation failed\n");
							return XAIE_ERR;
						}
					}

					Hash = _XAie_ComputeHash(Data, Size);
					for(int Label = ControlCodeInst->CurrentDataBWLabel-1; Label >= ControlCodeInst->CompareLabelUpto; Label--) {
						if(Map->BWDataSizes[Label] == Size) {
							if(Map->HashBW[Label] == ((int)Hash)) {
								ControlCodeInst->UcDmaDataNum = Label;
								ControlCodeInst->LabelMatchFound = 1;
								break;
							}
						}
					}	
				}

				if( (ControlCodeInst->IsShimBd) &&
					(ControlCodeInst->NumShimBDsChained == 0) ) {
					ControlCodeInst->CombineCommands = 0;
					ControlCodeInst->LabelMatchFound = 0;
					ControlCodeInst->UcDmaDataNum = ControlCodeInst->CurrentDataBWLabel;
				}

				if(ControlCodeInst->CombineCommands) {
					_XAie_ControlCodeSeekAndOverwrite(ControlCodeInst, 1, -3, " 1\n");
					_XAie_ControlCodeSeekAndOverwrite(ControlCodeInst, 4, -3, " 1\n");
				}
				else {
					if(ControlCodeInst->Mode != XAIE_SHIM_BD_CHAINING_ENABLE) {
						if(ControlCodeInst->Mode == XAIE_WRITE_DES_ASYNC_ENABLE) {
							CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE,
								"UC_DMA_WRITE_DES\t $r0, @UCBD_label_%d\n",
								ControlCodeInst->UcbdLabelNum);
							CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM,
								"UC_DMA_WRITE_DES\t $r0, @UCBD_label_%d\n",
								ControlCodeInst->UcbdLabelNum);
						}
						else {
							CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE,
									"UC_DMA_WRITE_DES_SYNC\t @UCBD_label_%d\n",
									ControlCodeInst->UcbdLabelNum);
							CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM,
									"UC_DMA_WRITE_DES_SYNC\t @UCBD_label_%d\n",
									ControlCodeInst->UcbdLabelNum);
						}
						ControlCodeInst->UcPageSize += OpSize;
						ControlCodeInst->UcPageTextSize += OpSize;
						ControlCodeInst->CombineCommands = 1;

					}
					if(ControlCodeInst->NumShimBDsChained == 0) {
						CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA, "UCBD_label_%d:\n",
								ControlCodeInst->UcbdLabelNum);
						CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA0, "UCBD_label_%d:\n",
								ControlCodeInst->UcbdLabelNum);
					}
					if(ControlCodeInst->Mode != XAIE_SHIM_BD_CHAINING_ENABLE) {
						ControlCodeInst->UcbdLabelNum++;
					}
				}

				if(ControlCodeInst->IsShimBd){
					if(ControlCodeInst->Mode == XAIE_SHIM_BD_CHAINING_ENABLE) {
						ControlCodeInst->CombineCommands = 1;
						ControlCodeInst->NumShimBDsChained++;
					}
					else
					{
						/**
						 * Note:
						 * If we want to ensure that SHIM BD doesn't get
						 * chained to non shim BD's. But non shim BDs need
						 * to get chanined with shim BDs.
						 *
						 * Then set the below flag to 1.
						 */
						ControlCodeInst->CombineCommands = 0;
					}
				}
				ControlCodeInst->PageBreak = 0;
				
				if(ControlCodeInst->LabelMatchFound == 0) {
					CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA2, "DMAWRITE_data_%d:\n",
							ControlCodeInst->UcDmaDataNum);
					CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA1, "DMAWRITE_data_%d:\n",
						ControlCodeInst->UcDmaDataNum);
				}
				else {
					CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA,
                    	"\t UC_DMA_BD\t 0, 0x%x, @DMAWRITE_data_%d, 0x%x, 0, 0\n",
                    	EXTRACT_LOWER_FOUR_BYTES(RegOff),  ControlCodeInst->UcDmaDataNum,
						Size);
					CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA0,
                    	"\t UC_DMA_BD\t 0, 0x%x, @DMAWRITE_data_%d, 0x%x, 0, 0\n",
                    	EXTRACT_LOWER_FOUR_BYTES(RegOff),  ControlCodeInst->UcDmaDataNum,
						Size);
					ControlCodeInst->UcPageSize += UC_DMA_BD_SIZE;
					ControlCodeInst->UcDmaDataNum = ControlCodeInst->CurrentDataBWLabel;
					_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
						 ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);
					break;
				}
			}

			ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT -
				(ControlCodeInst->UcPageTextSize % DATA_SECTION_ALIGNMENT));
			if (ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT) {
				ControlCodeInst->DataAligner = 0U;
			}

			NewPagePayloadSize = Size - TempItrSize;

			if (ControlCodeInst->IsAdjacentMemWrite == 0)
				ControlCodeInst->UcPageSize += UC_DMA_BD_SIZE;
			for (IterationSize = TempItrSize; IterationSize < Size; IterationSize++) {
				 if( (ControlCodeInst->UcPageSize + UC_DMA_WORD_LEN + ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax )
				 {
					ControlCodeInst->PageBreak = 1;
					PageBreakOccured = 1;
					break;
				 }
				CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA2, "\t.long 0x%08x\n", *(Data+IterationSize));
				CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA1, "\t.long 0x%08x\n", *(Data+IterationSize));
				ControlCodeInst->UcPageSize += UC_DMA_WORD_LEN;
			}

			if(ControlCodeInst->IsAdjacentMemWrite == 1) {
				_XAie_UpdateDataLengthDmaBd(ControlCodeInst,
						(ControlCodeInst->CombinedMemWriteSize + IterationSize));
				if (Map) {
					if(ControlCodeInst->PrevMemWriteType == WRITE32) {
						Map->HashW[ControlCodeInst->CurrentDataLabel-1] = HASH_INVALID;
					}
					else {
						Map->BWDataSizes[ControlCodeInst->CurrentDataBWLabel-1] = ControlCodeInst->CombinedMemWriteSize + IterationSize;
						Map->HashBW[ControlCodeInst->CurrentDataBWLabel-1] = HASH_INVALID;
					}
				}
			}
			else {
				if((ControlCodeInst->LabelMatchFound == 0) && Map) {
						Map->HashBW[ControlCodeInst->CurrentDataBWLabel] = _XAie_ComputeHash(Data, IterationSize - TempItrSize);
						Map->BWDataSizes[ControlCodeInst->CurrentDataBWLabel] = (IterationSize - TempItrSize);
				}
				CumilativeRegOff = RegOff + AdjustedOff;
            	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA,
                    	"\t UC_DMA_BD\t 0, 0x%x, @DMAWRITE_data_%d, 0x%x, 0, 0\n",
                    	EXTRACT_LOWER_FOUR_BYTES(CumilativeRegOff),  ControlCodeInst->UcDmaDataNum,
						(IterationSize - TempItrSize));
				CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA0,
                    	"\t UC_DMA_BD\t 0, 0x%x, @DMAWRITE_data_%d, 0x%x, 0, 0\n",
                    	EXTRACT_LOWER_FOUR_BYTES(CumilativeRegOff),  ControlCodeInst->UcDmaDataNum,
						(IterationSize - TempItrSize));
				ControlCodeInst->UcDmaDataNum++;
				ControlCodeInst->CurrentDataBWLabel = ControlCodeInst->UcDmaDataNum;
				ControlCodeInst->PrevMemWriteType = BLOCKWRITE32;
			}
			
			NewPageRegOff = RegOff + AdjustedOff;
			AdjustedOff += ((IterationSize - TempItrSize) * UC_DMA_WORD_LEN);
			CompletedSize += (IterationSize - TempItrSize);
			TempItrSize = IterationSize;
			
		}
		_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
			 ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);
	}

	ControlCodeInst->CalculatedNextRegOff = (u64)(RegOff + (Size * (sizeof(*Data))));

	if(ControlCodeInst->LabelMatchFound == 1) {
		ControlCodeInst->CombinedMemWriteSize = 0;
		ControlCodeInst->CalculatedNextRegOff = UINT64_MAX;
	}
	else if(PageBreakOccured == 1) {
		ControlCodeInst->CombinedMemWriteSize =  NewPagePayloadSize;
		ControlCodeInst->CalculatedNextRegOff =  (u64) ( NewPageRegOff + (NewPagePayloadSize * (sizeof(*Data))) );
	}
	else if(ControlCodeInst->IsAdjacentMemWrite == 1) {
		ControlCodeInst->CombinedMemWriteSize += Size;
	}
	else {
		ControlCodeInst->CombinedMemWriteSize = Size;
	}

	ControlCodeInst->LabelMatchFound = 0;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to initialize a chunk of aie address space with
* a specified value.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Data to initialize a chunk of aie address space..
* @param	Size: Number of 32-bit words.
*
* @return	None.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	u32 CompletedSize = 0;
	u32 IterationSize;
	u64 AdjustedOff = 0;
	u64 CumilativeRegOff = 0;

	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;

	CompletedSize = 0;
	while (Size > CompletedSize) {
		if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
			if (!ControlCodeInst->IsJobOpen) {
				_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
			}

			ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT -
				((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC) % DATA_SECTION_ALIGNMENT));
			if (ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT) {
				ControlCodeInst->DataAligner = 0U;
			}

			if((ControlCodeInst->UcPageSize + ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC +
				UC_DMA_BD_SIZE + UC_DMA_WORD_LEN + ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
				_XAie_StartNewPage(ControlCodeInst);
				_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
			}

			if(ControlCodeInst->CombineCommands) {
				_XAie_ControlCodeSeekAndOverwrite(ControlCodeInst, 1, -3, " 1\n");
				_XAie_ControlCodeSeekAndOverwrite(ControlCodeInst, 4, -3, " 1\n");
			}
			else {
				CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE,
						"UC_DMA_WRITE_DES_SYNC\t @UCBD_label_%d\n",
						ControlCodeInst->UcbdLabelNum);
				CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM,
						"UC_DMA_WRITE_DES_SYNC\t @UCBD_label_%d\n",
						ControlCodeInst->UcbdLabelNum);
				CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA, "UCBD_label_%d:\n",
						ControlCodeInst->UcbdLabelNum);
				CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA0, "UCBD_label_%d:\n",
						ControlCodeInst->UcbdLabelNum);
				ControlCodeInst->CombineCommands = 1;
				ControlCodeInst->UcbdLabelNum++;
				ControlCodeInst->UcPageSize += ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC;
				ControlCodeInst->UcPageTextSize += ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC;
			}

			ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT -
				(ControlCodeInst->UcPageTextSize % DATA_SECTION_ALIGNMENT));
			if (ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT) {
				ControlCodeInst->DataAligner = 0U;
			}

			CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA2, "DMAWRITE_data_%d:\n",
					ControlCodeInst->UcDmaDataNum);
			CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA1, "DMAWRITE_data_%d:\n",
					ControlCodeInst->UcDmaDataNum);
			ControlCodeInst->UcPageSize += UC_DMA_BD_SIZE;
			for(IterationSize = 0; (IterationSize + CompletedSize) < Size &&
				(ControlCodeInst->UcPageSize + UC_DMA_WORD_LEN + ControlCodeInst->DataAligner)
				<= ControlCodeInst->PageSizeMax; IterationSize++)
			{
				CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA2, "\t.long 0x%08x\n", Data);
				CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA1, "\t.long 0x%08x\n", Data);
				ControlCodeInst->UcPageSize += UC_DMA_WORD_LEN;
			}
			CumilativeRegOff = RegOff + AdjustedOff;
            CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA,
                    "\t UC_DMA_BD\t 0, 0x%x, @DMAWRITE_data_%d, %d, 0, 0\n\n",
                    EXTRACT_LOWER_FOUR_BYTES(CumilativeRegOff),
                    ControlCodeInst->UcDmaDataNum, IterationSize);

			AdjustedOff += (IterationSize * UC_DMA_WORD_LEN);
			CompletedSize += IterationSize;
			ControlCodeInst->UcDmaDataNum++;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to perform Address Patching by CERT
* @param	IOInst:    IO instance pointer
* @param	Arg_Index: Index of the argument to be patched coresponding to its index in Kernel Signature/0xFFFF.
* 					   Note: If the index is 0xFFFF, then shimDMA BD will be patched against control code instead of kernel arg.
* @param	Num_BDs:   Represents Number of BDs to be patched.
*
* @return	None.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_AddressPatching(void *IOInst, u16 Arg_Index, u8 Num_BDs)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	u32 OpSize;

	if(ControlCodeInst->Mode == XAIE_WRITE_DES_ASYNC_ENABLE) {
		OpSize = ISA_OPSIZE_UC_DMA_WRITE_DES + ISA_OPSIZE_WAIT_UC_DMA;
	}
	else {
		OpSize = ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC;
	}

	ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT -
		((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_APPLY_OFFSET_57 + OpSize) % DATA_SECTION_ALIGNMENT));

	if (ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT) {
		ControlCodeInst->DataAligner = 0U;
	}

	if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {

		if (!ControlCodeInst->IsJobOpen) {
			_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
		}
		
		if((ControlCodeInst->UcPageSize + ISA_OPSIZE_APPLY_OFFSET_57 + OpSize +
			(UC_DMA_BD_SIZE + (Num_BDs * UC_DMA_WORD_LEN * SHIM_BD_NUM_REGS)) + ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
			_XAie_StartNewPage(ControlCodeInst);
			_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
		}

		if(ControlCodeInst->ScrachpadName == NULL) {
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE,
				"APPLY_OFFSET_57\t @DMAWRITE_data_%d, %d, %d\n",
				ControlCodeInst->UcDmaDataNum,
				Num_BDs, Arg_Index);
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM,
				"APPLY_OFFSET_57\t @DMAWRITE_data_%d, %d, %d\n",
				ControlCodeInst->UcDmaDataNum,
				Num_BDs, Arg_Index);
	}
	else {
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE,
                                        "APPLY_OFFSET_57\t @DMAWRITE_data_%d, %d, %d, @%s\n",
                                        ControlCodeInst->UcDmaDataNum,
                                        Num_BDs, Arg_Index, ControlCodeInst->ScrachpadName);
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM,
                                        "APPLY_OFFSET_57\t @DMAWRITE_data_%d, %d, %d, @%s\n",
                                        ControlCodeInst->UcDmaDataNum,
                                        Num_BDs, Arg_Index, ControlCodeInst->ScrachpadName);
	}

	ControlCodeInst->UcPageTextSize += ISA_OPSIZE_APPLY_OFFSET_57;
	ControlCodeInst->UcPageSize += ISA_OPSIZE_APPLY_OFFSET_57;
	_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
		 ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);
	}

	if(ControlCodeInst->ScrachpadName != NULL) {
                free(ControlCodeInst->ScrachpadName);
                ControlCodeInst->ScrachpadName = NULL;
        }

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This fuction inserts WAIT_UC_DMA instruction in the control code.
* @param	IOInst:    IO instance pointer
* 
*
* @return	None.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_WaitUcDMA(void *IOInst)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;

	ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT -
		((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_WAIT_UC_DMA) % DATA_SECTION_ALIGNMENT));

	if (ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT) {
		ControlCodeInst->DataAligner = 0U;
	}

	if((ControlCodeInst->UcPageSize + ISA_OPSIZE_WAIT_UC_DMA + ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
			XAIE_ERROR("ISA_OPSIZE_WAIT_UC_DMA opcode should in the same page as ISA_UC_DMA_WRITE_DES Opcode\n");
			return XAIE_ERR;
	}

	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "WAIT_UC_DMA\t $r0\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "WAIT_UC_DMA\t $r0\n");
	ControlCodeInst->UcPageTextSize += ISA_OPSIZE_WAIT_UC_DMA;
	ControlCodeInst->UcPageSize += ISA_OPSIZE_WAIT_UC_DMA;
	_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
		 ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the function to generate code for wait for task complete token.
*
* @param	DevInst: Device instance pointer
* @param	Column: Column number.
* @param	Row: Row number.Mask to be applied to Data.
* @param	Channel: Channel number.
* @param	NumTokens: Number of tokens to wait for completion.

* @return	0 on success.
*
*
*******************************************************************************/
AieRC XAie_WaitTaskCompleteToken(XAie_DevInst *DevInst,
	uint16_t Column, uint16_t Row, uint32_t Channel, uint8_t NumTokens)
{
	if (DevInst->Backend->Type != XAIE_IO_BACKEND_CONTROLCODE) {
		XAIE_ERROR("This is supported only in Controlcode Backend %d \n", DevInst->Backend->Type);
		return XAIE_INVALID_BACKEND;
	}

	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

	uint32_t TileId;
	ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT -
		((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_WAIT_TCTS) % DATA_SECTION_ALIGNMENT));
	if (ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT) {
		ControlCodeInst->DataAligner = 0U;
	}

	if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {

		if (!ControlCodeInst->IsJobOpen) {
			_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
		}

		if((ControlCodeInst->UcPageSize + ISA_OPSIZE_WAIT_TCTS +
			ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
			_XAie_StartNewPage(ControlCodeInst);
			_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
		}

		TileId = ((Column << 5) | Row);
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "WAIT_TCTS\t 0x%x, 0x%x, 0x%x\n",
				TileId, Channel, NumTokens );
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "WAIT_TCTS\t 0x%x, 0x%x, 0x%x\n",
				TileId, Channel, NumTokens );
		ControlCodeInst->CombineCommands = 0;
		ControlCodeInst->UcPageSize += ISA_OPSIZE_WAIT_TCTS;
		ControlCodeInst->UcPageTextSize += ISA_OPSIZE_WAIT_TCTS;
		_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
			 ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This function is used to save the time stamps.
*
* @param        DevInst: Device instance pointer
* @param        Timestamp: Unique id.
*
* @return       XAIE_OK or XAIE_ERR.
*
*******************************************************************************/
AieRC XAie_ControlCodeSaveTimestamp(XAie_DevInst *DevInst, u32 Timestamp)
{
        if(DevInst->Backend->Type != XAIE_IO_BACKEND_CONTROLCODE) {
                XAIE_ERROR("This is supported only in Controlcode Backend %d \n", DevInst->Backend->Type);
                return XAIE_INVALID_BACKEND;
        }

        XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

        ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT -
                ((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_SAVE_TIMESTAMPS)% DATA_SECTION_ALIGNMENT));
        if (ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT) {
                ControlCodeInst->DataAligner = 0U;
        }

        if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {

                if (!ControlCodeInst->IsJobOpen) {
                        _XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
                }

                if((ControlCodeInst->UcPageSize + ISA_OPSIZE_SAVE_TIMESTAMPS +
                        ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
                        _XAie_StartNewPage(ControlCodeInst);
                        _XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
                }

                CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "SAVE_TIMESTAMPS\t %d\n",
                                Timestamp );
				CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "SAVE_TIMESTAMPS\t %d\n",
                                Timestamp );
                ControlCodeInst->CombineCommands = 0;
                ControlCodeInst->UcPageSize += ISA_OPSIZE_SAVE_TIMESTAMPS;
                ControlCodeInst->UcPageTextSize += ISA_OPSIZE_SAVE_TIMESTAMPS;
				_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
					 ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);

                return XAIE_OK;
        }
        else
                return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This function is used to insert a NOP (no-operation) opcode into the
* control code. NOP does not perform any action and is typically used for
* padding or timing alignment.
*
* @param        IOInst: IO instance pointer
*
* @return       XAIE_OK on success, error code on failure.
*
* @note         Internal only.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_Nop(void *IOInst)
{
        XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;

        ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT -
                ((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_NOP) % DATA_SECTION_ALIGNMENT));
        if (ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT) {
                ControlCodeInst->DataAligner = 0U;
        }

        if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {

                if (!ControlCodeInst->IsJobOpen) {
                        _XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
                }

                if((ControlCodeInst->UcPageSize + ISA_OPSIZE_NOP +
                        ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
                        _XAie_StartNewPage(ControlCodeInst);
                        _XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
                }

                CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "NOP\n");
                CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "NOP\n");
                ControlCodeInst->CombineCommands = 0;
                ControlCodeInst->UcPageSize += ISA_OPSIZE_NOP;
                ControlCodeInst->UcPageTextSize += ISA_OPSIZE_NOP;
                _XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
                         ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);

                return XAIE_OK;
        }
        else
                return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This function is used to add preempt opcode to asm file.
*
* @param        IOInst: IO instance pointer
* @param        PreemptId: Preemption level
* @param        SaveLabel: Name of Save Label
* @param        RestoreLabel: Name of Restore Label
* @param        HintMap: Pointer to the hint map array which needs to be saved along with preempt opcode.
* @param        HintMapSizeInWords: Size of the hint map in words. 

* @return       XAIE_OK or XAIE_ERR.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_Preempt(void *IOInst, u16 PreemptId, char* SaveLabel, char* RestoreLabel, u32* HintMap, u32 HintMapSizeInWords)
{
	if(SaveLabel == NULL || RestoreLabel == NULL) {
		XAIE_ERROR("SaveLabel and RestoreLabel cannot be NULL\n");
		return XAIE_ERR;
	}
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT - ((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_PREEMPT) % DATA_SECTION_ALIGNMENT));
	if (ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT)
	{
		ControlCodeInst->DataAligner = 0U;
	}

	if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers)
	{

        if((ControlCodeInst->UcPageSize + ISA_OPSIZE_PREEMPT + (HintMapSizeInWords * UC_DMA_WORD_LEN)
        	+ ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
        	_XAie_StartNewPage(ControlCodeInst);
        }

		_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
		if(HintMap && HintMapSizeInWords) {
			CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "PREEMPT\t0x%x, @%s, @%s , @hintmap_%d\n",PreemptId, SaveLabel, RestoreLabel, ControlCodeInst->HintMapId);
			CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "PREEMPT\t0x%x, @%s, @%s , @hintmap_%d\n",PreemptId, SaveLabel, RestoreLabel, ControlCodeInst->HintMapId);
			
			CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA2, "hintmap_%d:\n",
						ControlCodeInst->HintMapId);
			CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA1, "hintmap_%d:\n",
						ControlCodeInst->HintMapId);

			for(u32 i=0; i<HintMapSizeInWords; i++) {
				CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA2, "\t.long 0x%08x\n", HintMap[i]);
				CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA1, "\t.long 0x%08x\n", HintMap[i]);
			}
			ControlCodeInst->UcPageSize += ISA_OPSIZE_PREEMPT + (HintMapSizeInWords * UC_DMA_WORD_LEN);
			ControlCodeInst->HintMapId++;
		}
		else {
			CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "PREEMPT\t0x%x, @%s, @%s\n",PreemptId, SaveLabel, RestoreLabel);
			CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "PREEMPT\t0x%x, @%s, @%s\n",PreemptId, SaveLabel, RestoreLabel);
			ControlCodeInst->UcPageSize += ISA_OPSIZE_PREEMPT;
		}
		_XAie_EndJob(ControlCodeInst);

		ControlCodeInst->CombineCommands = 0;
		ControlCodeInst->UcPageTextSize += ISA_OPSIZE_PREEMPT;
		_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId, ControlCodeInst->UcPageSize, 
								  ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);
	}
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This function is used to add preempt opcode to asm file.
*
* @param        IOInst: IO instance pointer
* @param        BuffName: Name of Save/Restore Buffer
* @param        BuffSize: Size of Save/Restore Buffer
* eg - 			.setpad ctrl_pkt, 0x1000
* @return       XAIE_OK or XAIE_ERR.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_SetPadInteger(void *IOInst, char* BuffName, u32 BuffSize)
{
	if(BuffName == NULL) {
		XAIE_ERROR("Buffer name cannot be NULL\n");
		return XAIE_ERR;
	}
	if(BuffSize == 0) {
		XAIE_ERROR("Buffer size cannot be zero\n");
		return XAIE_ERR;
	}
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	
	if(ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ".setpad\t %s, 0x%x\n",BuffName, BuffSize);
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ".setpad\t %s, 0x%x\n",BuffName, BuffSize);
		ControlCodeInst->CombineCommands = 0;
		return XAIE_OK;
	}
	else {
		XAIE_ERROR("Control code file pointer is NULL\n");
		return XAIE_ERR;
	}
}

/*****************************************************************************/
/**
*
* This function is used to add preempt opcode to asm file.
*
* @param        IOInst: IO instance pointer
* @param        BuffName: Name of Save/Restore Buffer
* @param        BuffBlobPath: Path to the Buffer Blob .bin file
* eg - 			.setpad ctrl_pkt, ctrlpkt.bin
* @return       XAIE_OK or XAIE_ERR.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_SetPadString(void *IOInst, char* BuffName, char* BuffBlobPath)
{
	if(BuffName == NULL) {
		XAIE_ERROR("Buffer name cannot be NULL\n");
		return XAIE_ERR;
	}
	if(BuffBlobPath == NULL) {
		XAIE_ERROR("Buffer Blob Path cannot be NULL\n");
		return XAIE_ERR;
	}
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	
	if(ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ".setpad\t %s, %s\n",BuffName, BuffBlobPath);
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ".setpad\t %s, %s\n",BuffName, BuffBlobPath);
		ControlCodeInst->CombineCommands = 0;
		return XAIE_OK;
	}
	else {
		XAIE_ERROR("Control code file pointer is NULL\n");
		return XAIE_ERR;
	}
}

/*****************************************************************************/
/**
*
* This function is used to add ".attach_to_group" cert directive to asm file.
*
* @param        IOInst: IO instance pointer
* @param        UcIndex: uC Index
*               ---------------------------------------------------------
*               | aie4                    |   | aie2ps                  |
*               | column index | uC index |   | column index | uC index |
*               |-------------------------------------------------------|
*               | column 0_A   | 0        |   | column 0     | 0        |
*               | column 0_B   | 1        |   | column 1     | 1        |
*               | column 1_A   | 2        |   | column 2     | 2        |
*               | column 1_B   | 3        |   | column 3     | 3        |
*               | column 2_A   | 4        |   | column 4     | 4        |
*               | column 2_B   | 5        |   | ...          | ...      |
*               ---------------------------------------------------------  
*
* @return       XAIE_OK or XAIE_ERR.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_AttachToGroup(void *IOInst, uint8_t UcIndex)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	
	if(ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ".attach_to_group\t %d\n",UcIndex);
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ".attach_to_group\t %d\n",UcIndex);
		ControlCodeInst->CombineCommands = 0;
		return XAIE_OK;
	}
	else {
		XAIE_ERROR("Control code file pointer is NULL\n");
		return XAIE_ERR;
	}
}

/*****************************************************************************/
/**
*
* This function is used to add "REMOTE BARRIER" cert opcode to asm file.
*
* @param        IOInst: IO instance pointer
* @param        RbId: Remote Barrier ID
*				There are 8 remote barriers namely:
*               ---------------------------------------------------------
*    			|  $rb0-$rb7											|
*               ---------------------------------------------------------
* @param        Mask: Mask to be applied, bitmap of uCs which are going to
*					  use the barrier.
*               ---------------------------------------------------------
*               | aie4                    |   | aie2ps                  |
*               | column index | uC index |   | column index | uC index |
*               |-------------------------------------------------------|
*               | column 0_A   | 0        |   | column 0     | 0        |
*               | column 0_B   | 1        |   | column 1     | 1        |
*               | column 1_A   | 2        |   | column 2     | 2        |
*               | column 1_B   | 3        |   | column 3     | 3        |
*               | column 2_A   | 4        |   | column 4     | 4        |
*               | column 2_B   | 5        |   | ...          | ...      |
*               ---------------------------------------------------------
*
* @return       XAIE_OK or XAIE_ERR.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_RemoteBarrier(void *IOInst, uint8_t RbId, uint32_t UcMask)
{
	if(RbId > MAX_REMOTE_BARRIER_ID) {
		XAIE_ERROR("Remote Barrier ID should be between 0 to %d\n", MAX_REMOTE_BARRIER_ID);
		return XAIE_ERR;
	}

	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT -
		((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_REMOTE_BARRIER) % DATA_SECTION_ALIGNMENT));
	if (ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT)
	{
		ControlCodeInst->DataAligner = 0U;
	}

	if(ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
        if (!ControlCodeInst->IsJobOpen) {
            _XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
        }

        if((ControlCodeInst->UcPageSize + ISA_OPSIZE_REMOTE_BARRIER +
            ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
            _XAie_StartNewPage(ControlCodeInst);
            _XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
        }

		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "REMOTE_BARRIER\t $rb%d, 0x%x\n", RbId, UcMask);
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "REMOTE_BARRIER\t $rb%d, 0x%x\n", RbId, UcMask);
		ControlCodeInst->CombineCommands = 0;
		ControlCodeInst->UcPageSize += ISA_OPSIZE_REMOTE_BARRIER;
		ControlCodeInst->UcPageTextSize += ISA_OPSIZE_REMOTE_BARRIER;
		_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
			 ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);
		return XAIE_OK;
	}
	else {
		XAIE_ERROR("Control code file pointer is NULL\n");
		return XAIE_ERR;
	}
}

/*****************************************************************************/
/**
*
* This function is used to add Save Register opcode to asm file.
*
* @param        IOInst: IO instance pointer
* @param        RegOff: Register Address/Offset whose value needs to be saved
* @param        Id: Unique Id
* @return       XAIE_OK or XAIE_ERR.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_SaveRegister(void *IOInst, u32 RegOff, u32 Id)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT -
		((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_SAVE_REGISTER) % DATA_SECTION_ALIGNMENT));
	if (ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT)
	{
		ControlCodeInst->DataAligner = 0U;
	}

	if(ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {

		if (!ControlCodeInst->IsJobOpen) {
    		_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
   	 	}

		if((ControlCodeInst->UcPageSize + ISA_OPSIZE_SAVE_REGISTER +
        	ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
    		_XAie_StartNewPage(ControlCodeInst);
        	_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
    	}

		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "SAVE_REGISTER\t 0x%x, 0x%x\n", RegOff, Id);
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "SAVE_REGISTER\t 0x%x, 0x%x\n", RegOff, Id);
		ControlCodeInst->CombineCommands = 0;
		ControlCodeInst->UcPageSize += ISA_OPSIZE_SAVE_REGISTER;
		ControlCodeInst->UcPageTextSize += ISA_OPSIZE_SAVE_REGISTER;
		_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
			 ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);
		return XAIE_OK;
	}
	else {
		XAIE_ERROR("Control code file pointer is NULL\n");
		return XAIE_ERR;
	}
}

/*****************************************************************************/
/**
*
* This function generates REL_ACQ_SYNC instruction in controlcode backend
*
* @param        DevInst: Device Instance
* @param		LockMod: Internal lock module data structure.
* @param		Loc: Location of the tile.
* @param		RelLock: Release lock structure.
* @param		AcqLock: Acquire lock structure.

* @return       AieRC status.
*
*******************************************************************************/

AieRC XAie_ControlCodeRelAcqSync(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
								 XAie_LocType Loc, XAie_Lock RelLock, XAie_Lock AcqLock) {

	if(DevInst->Backend->Type != XAIE_IO_BACKEND_CONTROLCODE) {
    	XAIE_ERROR("This is supported only in Controlcode Backend %d \n", DevInst->Backend->Type);
        return XAIE_INVALID_BACKEND;
    }

	u8 TempRelVal = (u8)RelLock.LockVal;
	u8 TempAcqVal = (u8)AcqLock.LockVal;
	u16 LockId    = AcqLock.LockId;
	u64 RegAddrRel;
	u64 RegOffRel = 0;
	u64 RegAddrAcq;
	u64 RegOffAcq = 0;

	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

	if((DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) && (LockId >= LockMod->NumLocks)) {
		RegOffRel = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, RegOffRel);
		RegOffAcq = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, RegOffAcq);
		LockId -= LockMod->NumLocks;
	}

	RegOffRel |= LockMod->BaseAddr + (LockId * (u64)LockMod->LockIdOff) +
	((TempRelVal & LockMod->LockValueMask) << LockMod->LockValueShift);
	
	RegOffAcq |= LockMod->BaseAddr + (LockId * (u64)LockMod->LockIdOff) +
	(LockMod->RelAcqOff) + ((TempAcqVal & LockMod->LockValueMask) << LockMod->LockValueShift);

	RegAddrRel = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffRel;
	RegAddrAcq = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffAcq;

	ControlCodeInst->DataAligner = (DATA_SECTION_ALIGNMENT -
		((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_REL_ACQ_SYNC) % DATA_SECTION_ALIGNMENT));
	if (ControlCodeInst->DataAligner == DATA_SECTION_ALIGNMENT) {
		ControlCodeInst->DataAligner = 0U;
	}
	if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
		if (!ControlCodeInst->IsJobOpen) {
			_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
		}

		if((ControlCodeInst->UcPageSize + ISA_OPSIZE_REL_ACQ_SYNC +
			ControlCodeInst->DataAligner) > ControlCodeInst->PageSizeMax) {
			_XAie_StartNewPage(ControlCodeInst);
			_XAie_StartNewJob(ControlCodeInst, XAIE_START_JOB);
		}

	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "REL_ACQ_SYNC\t 0x%x, 0x%x\n",
			(u32)(EXTRACT_LOWER_FOUR_BYTES(RegAddrRel)),
			(u32)(EXTRACT_LOWER_FOUR_BYTES(RegAddrAcq)));

	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "REL_ACQ_SYNC\t 0x%x, 0x%x ; barrierid:%" PRIu64 ", lockid:%u, relval:%d, acqval:%d\n",
				ControlCodeInst->BarrierId, LockId, RelLock.LockVal, AcqLock.LockVal);

		ControlCodeInst->CombineCommands = 0;
		ControlCodeInst->UcPageSize += ISA_OPSIZE_REL_ACQ_SYNC;
		ControlCodeInst->UcPageTextSize += ISA_OPSIZE_REL_ACQ_SYNC;
		_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
			 ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);

		ControlCodeInst->BarrierId++;

		return XAIE_OK;
	}
	return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This is the function to write 32 bit value to NPI register address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: NPI register offset
* @param	RegVal: Value to write to register
*
* @return	None.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
static void _XAie_ControlCodeIO_NpiWrite32(void *IOInst, u32 RegOff, u32 RegVal)
{
	(void)IOInst;
	(void)RegOff;
	(void)RegVal;

	return;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to mask poll a NPI address for a value.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Mask: Mask to be applied to Data.
* @param	Value: 32-bit value to poll for
* @param	TimeOutUs: Timeout in micro seconds.
*
* @return	XAIE_OK or XAIE_ERR.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_ControlCodeIO_NpiMaskPoll(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value, u32 TimeOutUs)
{
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;
	(void)TimeOutUs;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the function to run backend operations
*
* @param	IOInst: IO instance pointer
* @param	DevInst: AI engine partition device instance
* @param	Op: Backend operation code
* @param	Arg: Backend operation argument
*
* @return	XAIE_OK for success and error code for failure.
*
* @note		Internal only.
*
*******************************************************************************/
AieRC XAie_ControlCodeIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		     XAie_BackendOpCode Op, void *Arg)
{
	AieRC RC = XAIE_OK;
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;

	switch(Op) {
		case XAIE_BACKEND_OP_NPIWR32:
		{
			XAie_BackendNpiWrReq *Req = Arg;

			_XAie_ControlCodeIO_NpiWrite32(IOInst, Req->NpiRegOff,
					Req->Val);
			break;
		}
		case XAIE_BACKEND_OP_NPIMASKPOLL32:
		{
			XAie_BackendNpiMaskPollReq *Req = Arg;

			return _XAie_ControlCodeIO_NpiMaskPoll(IOInst, Req->NpiRegOff,
					Req->Mask, Req->Val, Req->TimeOutUs);
		}
		case XAIE_BACKEND_OP_ASSERT_SHIMRST:
		{
			u8 RstEnable = (u8)((uintptr_t)Arg & 0xFF);

			_XAie_NpiSetShimReset(DevInst, RstEnable);
			break;
		}
		case XAIE_BACKEND_OP_SET_PROTREG:
		{
			RC = _XAie_NpiSetProtectedRegEnable(DevInst, Arg);
			break;
		}
		case XAIE_BACKEND_OP_CONFIG_SHIMDMABD:
		{
			ControlCodeInst->IsShimBd = 1;
			XAie_ShimDmaBdArgs *BdArgs =
				(XAie_ShimDmaBdArgs *)Arg;

			XAie_ControlCodeIO_BlockWrite32(IOInst, BdArgs->Addr,
				BdArgs->BdWords, BdArgs->NumBdWords);
			ControlCodeInst->IsShimBd = 0;
			break;
		}
		case XAIE_BACKEND_OP_REQUEST_TILES:
			return _XAie_PrivilegeRequestTiles(DevInst,
					(XAie_BackendTilesArray *)Arg);
		case XAIE_BACKEND_OP_PARTITION_INITIALIZE:
			return _XAie_PrivilegeInitPart(DevInst,
					(XAie_PartInitOpts *)Arg);
		case XAIE_BACKEND_OP_PARTITION_TEARDOWN:
			return _XAie_PrivilegeTeardownPart(DevInst);
		case XAIE_BACKEND_OP_UPDATE_NPI_ADDR:
		{
			XAie_ControlCodeIO *ControlCodeIOInst = (XAie_ControlCodeIO *)IOInst;
			ControlCodeIOInst->NpiBaseAddr = *((u64 *)Arg);
			break;
		}
		case XAIE_BACKEND_OP_CONFIG_MEM_INTRLVNG:
			return _XAie_PrivilegeConfigMemInterleavingLoc(DevInst,
					(XAie_BackendTilesEnableArray *)Arg);
		default:
			XAIE_ERROR("CDO backend doesn't support operation"
					" %u.\n", Op);
			RC = XAIE_FEATURE_NOT_SUPPORTED;
			break;
	}

	return RC;
}

/*****************************************************************************/
/**
*
* This is the function used to start capturing the control code
* This is usefull only in control code backend. In other backend calling this
* makes no sense.
*
*  @return    1 on failure.
*
*
******************************************************************************/
AieRC XAie_OpenControlCodeFile(XAie_DevInst *DevInst, const char *FileName, u32 PageSize)
{
	if(PageSize > PAGE_SIZE_MAX)
	{
		XAIE_ERROR("PageSize cannot be > PAGE_SIZE_MAX\n");
		return XAIE_ERR;
	}
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;
	char* TmpPtr = FName;

	if(DevInst->Backend->Type != XAIE_IO_BACKEND_CONTROLCODE) {
		XAIE_ERROR("This is supported only in Controlcode Backend %d \n", DevInst->Backend->Type);
		return XAIE_INVALID_BACKEND;
	}
	memset(ControlCodeInst, 0, sizeof(XAie_ControlCodeIO));
	ControlCodeInst->DisableDebugAsm = (DevInst->DisableDebugAsm != 0U) ? 1U : 0U;
	ControlCodeInst->ScrachpadName = NULL;
	ControlCodeInst->Mode = (u8)XAIE_INVALID_MODE;
	ControlCodeInst->ControlCodefp      = fopen(FileName, "w");
	ControlCodeInst->ControlCodedatafp  = fopen(TEMP_ASM_FILE1, "w+");
	ControlCodeInst->ControlCodedata2fp = fopen(TEMP_ASM_FILE2, "w+");

	/* Validate control code file pointers before opening debug ASM files
	 * to avoid unnecessary resource allocation if control code setup fails */
	if (ControlCodeInst->ControlCodefp == NULL ||
		ControlCodeInst->ControlCodedatafp == NULL ||
		ControlCodeInst->ControlCodedata2fp == NULL) {

		if(ControlCodeInst->ControlCodefp) {
			fclose(ControlCodeInst->ControlCodefp);
			ControlCodeInst->ControlCodefp = NULL;
		}
		if (ControlCodeInst->ControlCodedatafp) {
			fclose(ControlCodeInst->ControlCodedatafp);
			ControlCodeInst->ControlCodedatafp = NULL;
		}
		if (ControlCodeInst->ControlCodedata2fp) {
			fclose(ControlCodeInst->ControlCodedata2fp);
			ControlCodeInst->ControlCodedata2fp = NULL;
		}
		return XAIE_ERR;
	}

	/* Open debug ASM files only after control code files are confirmed valid */
	if (!ControlCodeInst->DisableDebugAsm) {
		strcat(FName,FileName);
		while(TmpPtr && (*TmpPtr  != '.' ))
			TmpPtr++;
		strcpy(TmpPtr,".DEBUG");

		ControlCodeInst->DebugAsmFileData0 = fopen(TEMP_ASM_FILE3, "w+");
		ControlCodeInst->DebugAsmFileData1 = fopen(TEMP_ASM_FILE4, "w+");
		ControlCodeInst->DebugAsmFile = fopen(FName, "w+");
	}

	ControlCodeInst->PageSizeMax = PageSize;
	ControlCodeInst->TotalLabelsAllocated = MAX_LABELS_PER_ASM_FILE;
	ControlCodeInst->TotalLabelsAllocatedWrite = MAX_LABELS_PER_ASM_FILE;
	ControlCodeInst->CurrentDataLabel = ControlCodeInst->UcbdDataNum;
	ControlCodeInst->CurrentDataBWLabel = ControlCodeInst->UcDmaDataNum;
	ControlCodeInst->LabelMap = (XAie_LabelMap*)calloc(1,sizeof(XAie_LabelMap));
	ControlCodeInst->PrevMemWriteType = -1;
	ControlCodeInst->BarrierId = 0;
	
	if(ControlCodeInst->LabelMap) {
		if(_XAie_LabelMapSetup(ControlCodeInst->LabelMap, ControlCodeInst) == XAIE_OK) {
			XAIE_DBG("Label optimization setup success\n");
		}
	}

	/* Debug ASM file open failed; close both debug ASM and control code
	 * files since we cannot proceed without debug ASM when it is enabled */
	if (!ControlCodeInst->DisableDebugAsm &&
		(ControlCodeInst->DebugAsmFileData0 == NULL ||
		 ControlCodeInst->DebugAsmFileData1 == NULL ||
		 ControlCodeInst->DebugAsmFile == NULL)) {

		if (ControlCodeInst->DebugAsmFileData0) {
			fclose(ControlCodeInst->DebugAsmFileData0);
			ControlCodeInst->DebugAsmFileData0 = NULL;
		}
		if (ControlCodeInst->DebugAsmFileData1) {
			fclose(ControlCodeInst->DebugAsmFileData1);
			ControlCodeInst->DebugAsmFileData1 = NULL;
		}
		if (ControlCodeInst->DebugAsmFile) {
			fclose(ControlCodeInst->DebugAsmFile);
			ControlCodeInst->DebugAsmFile = NULL;
		}
		fclose(ControlCodeInst->ControlCodefp);
		ControlCodeInst->ControlCodefp = NULL;
		fclose(ControlCodeInst->ControlCodedatafp);
		ControlCodeInst->ControlCodedatafp = NULL;
		fclose(ControlCodeInst->ControlCodedata2fp);
		ControlCodeInst->ControlCodedata2fp = NULL;
		return XAIE_ERR;
	}
	XAIE_DBG("Generating: %s\n", FileName);

	if(DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE4) {
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ".target\t aie4\n");
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ".target\t aie4\n");
	}
	else if(DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE4_A) {
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ".target\t aie4-a\n");
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ".target\t aie4-a\n");
	}
	else if(DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE2PS) {
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ".target\t aie2ps\n");
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ".target\t aie2ps\n");
	}
	else {
		//Any case other than above 3 is unknown and would be errored out currently
		XAIE_ERROR("Unknown Target Device Generation %d\n", DevInst->DevProp.DevGen);
		return XAIE_INVALID_DEVICE;
	}

	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ".aie_row_topology\t %d-%d-%d-%d\n", 
		DevInst->ShimTileNumRowsSouth, DevInst->MemTileNumRows, 
		DevInst->AieTileNumRows, DevInst->ShimTileNumRowsNorth);
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ".aie_row_topology\t %d-%d-%d-%d\n", 
		DevInst->ShimTileNumRowsSouth, DevInst->MemTileNumRows, 
		DevInst->AieTileNumRows, DevInst->ShimTileNumRowsNorth);

	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ".partition\t %dcolumn\n",DevInst->NumCols);
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ";\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ";text\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ";\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ".partition\t %dcolumn\n",DevInst->NumCols);
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ";\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ";text\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ";\n");
	ControlCodeInst->UcPageTextSize += ISA_OPSIZE_EOF;
	ControlCodeInst->UcPageSize += PAGE_HEADER_SIZE + ISA_OPSIZE_EOF;
	ControlCodeInst->IsPageOpen = 1;
	_XAie_ControlCodePageInfo(ControlCodeInst->DebugAsmFile, ControlCodeInst->PageId,
		 ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);

	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA, ";\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA, ";data\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA, ";\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA, ".align    16\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA2, ".align    4\n");

	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA0, ";\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA0, ";data\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA0, ";\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA0, ".align    16\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA1, ".align    4\n");

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the function used to start capturing the control code in memory
* This is useful only in control code backend. In other backend calling this
* makes no sense.
*
* @param	DevInst: Device instance pointer.
* @param	PageSize: Page Size for the control code.
*
* @return	XAIE_OK on success, error code on failure.
*
******************************************************************************/
AieRC XAie_AllocControlCodeBuffer(XAie_DevInst *DevInst, u32 PageSize)
{
	if(PageSize > PAGE_SIZE_MAX)
	{
		XAIE_ERROR("PageSize cannot be > PAGE_SIZE_MAX\n");
		return XAIE_ERR;
	}
	
	if(DevInst->Backend->Type != XAIE_IO_BACKEND_CONTROLCODE) {
		XAIE_ERROR("This is supported only in Controlcode Backend %d \n", DevInst->Backend->Type);
		return XAIE_INVALID_BACKEND;
	}
	
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;
	
	/* Save existing file pointers and flags if file mode was already opened */
	FILE *SavedControlCodefp = ControlCodeInst->ControlCodefp;
	FILE *SavedControlCodedatafp = ControlCodeInst->ControlCodedatafp;
	FILE *SavedControlCodedata2fp = ControlCodeInst->ControlCodedata2fp;
	FILE *SavedDebugAsmFile = ControlCodeInst->DebugAsmFile;
	FILE *SavedDebugAsmFileData0 = ControlCodeInst->DebugAsmFileData0;
	FILE *SavedDebugAsmFileData1 = ControlCodeInst->DebugAsmFileData1;
	
	memset(ControlCodeInst, 0, sizeof(XAie_ControlCodeIO));
	
	/* Restore file pointers if they existed, and copy flag from DevInst */
	ControlCodeInst->ControlCodefp = SavedControlCodefp;
	ControlCodeInst->ControlCodedatafp = SavedControlCodedatafp;
	ControlCodeInst->ControlCodedata2fp = SavedControlCodedata2fp;
	ControlCodeInst->DebugAsmFile = SavedDebugAsmFile;
	ControlCodeInst->DebugAsmFileData0 = SavedDebugAsmFileData0;
	ControlCodeInst->DebugAsmFileData1 = SavedDebugAsmFileData1;
	ControlCodeInst->DisableDebugAsm = (DevInst->DisableDebugAsm != 0U) ? 1U : 0U;
	
	ControlCodeInst->ScrachpadName = NULL;
	ControlCodeInst->Mode = (u8)XAIE_INVALID_MODE;
	ControlCodeInst->UseInMemoryBuffers = 1;
	
	/* Initialize control code memory buffers and validate before allocating
	 * debug ASM buffers to avoid unnecessary allocation if control code fails */
	ControlCodeInst->ControlCodeBuf = _XAie_MemBufferInit();
	ControlCodeInst->ControlCodeDataBuf = _XAie_MemBufferInit();
	ControlCodeInst->ControlCodeData2Buf = _XAie_MemBufferInit();

	if (!ControlCodeInst->ControlCodeBuf || !ControlCodeInst->ControlCodeDataBuf ||
		!ControlCodeInst->ControlCodeData2Buf) {
		_XAie_MemBufferFree(ControlCodeInst->ControlCodeBuf);
		_XAie_MemBufferFree(ControlCodeInst->ControlCodeDataBuf);
		_XAie_MemBufferFree(ControlCodeInst->ControlCodeData2Buf);
		XAIE_ERROR("Failed to allocate control code memory buffers\n");
		return XAIE_ERR;
	}

	/* Allocate debug ASM buffers only after control code buffers are confirmed valid */
	if (!ControlCodeInst->DisableDebugAsm) {
		ControlCodeInst->DebugAsmBuf = _XAie_MemBufferInit();
		ControlCodeInst->DebugAsmDataBuf0 = _XAie_MemBufferInit();
		ControlCodeInst->DebugAsmDataBuf1 = _XAie_MemBufferInit();
	}

	/* Debug ASM buffer allocation failed; free both debug ASM and control code
	 * buffers since we cannot proceed without debug ASM when it is enabled */
	if (!ControlCodeInst->DisableDebugAsm &&
		(!ControlCodeInst->DebugAsmBuf ||
		 !ControlCodeInst->DebugAsmDataBuf0 || !ControlCodeInst->DebugAsmDataBuf1)) {
		_XAie_MemBufferFree(ControlCodeInst->ControlCodeBuf);
		_XAie_MemBufferFree(ControlCodeInst->ControlCodeDataBuf);
		_XAie_MemBufferFree(ControlCodeInst->ControlCodeData2Buf);
		_XAie_MemBufferFree(ControlCodeInst->DebugAsmBuf);
		_XAie_MemBufferFree(ControlCodeInst->DebugAsmDataBuf0);
		_XAie_MemBufferFree(ControlCodeInst->DebugAsmDataBuf1);
		XAIE_ERROR("Failed to allocate debug ASM memory buffers\n");
		return XAIE_ERR;
	}
	
	ControlCodeInst->PageSizeMax = PageSize;
	ControlCodeInst->TotalLabelsAllocated = MAX_LABELS_PER_ASM_FILE;
	ControlCodeInst->TotalLabelsAllocatedWrite = MAX_LABELS_PER_ASM_FILE;
	ControlCodeInst->CurrentDataLabel = ControlCodeInst->UcbdDataNum;
	ControlCodeInst->CurrentDataBWLabel = ControlCodeInst->UcDmaDataNum;
	ControlCodeInst->LabelMap = (XAie_LabelMap*)calloc(1,sizeof(XAie_LabelMap));
	ControlCodeInst->PrevMemWriteType = -1;
	ControlCodeInst->BarrierId = 0;
	
	if(ControlCodeInst->LabelMap) {
		if(_XAie_LabelMapSetup(ControlCodeInst->LabelMap, ControlCodeInst) == XAIE_OK) {
			XAIE_DBG("Label optimization setup success\n");
		}
	}
	
	XAIE_DBG("Generating control code in memory\n");
	fflush(stdout);

	/* If file mode was already active, temporarily NULL file pointers to avoid duplicate header writes */
	/* Headers were already written to files by XAie_OpenControlCodeFile */
	u8 file_mode_was_active = (SavedControlCodefp != NULL);
	if (file_mode_was_active) {
		ControlCodeInst->ControlCodefp = NULL;
		ControlCodeInst->ControlCodedatafp = NULL;
		ControlCodeInst->ControlCodedata2fp = NULL;
		ControlCodeInst->DebugAsmFile = NULL;
		ControlCodeInst->DebugAsmFileData0 = NULL;
		ControlCodeInst->DebugAsmFileData1 = NULL;
	}

	/* Write header content to buffers */
	XAIE_DBG("Writing target directive (DevGen=%d)\n", DevInst->DevProp.DevGen);
	if(DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE4) {
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ".target\t aie4\n");
		if (!ControlCodeInst->ControlCodefp) {
			CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ".target\t aie4\n");
		}
	}
	else if(DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE4_A) {
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ".target\t aie4-a\n");
		if (!ControlCodeInst->ControlCodefp) {
			CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ".target\t aie4-a\n");
		}
	}
	else if(DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE2PS) {
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ".target\t aie2ps\n");
		if (!ControlCodeInst->ControlCodefp) {
			CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ".target\t aie2ps\n");
		}
	}
	else {
		XAIE_ERROR("Unknown Target Device Generation %d\n", DevInst->DevProp.DevGen);
		return XAIE_INVALID_DEVICE;
	}

	XAIE_DBG("Writing topology directive\n");
	XAIE_DBG("Topology values: South=%d, MemTile=%d, AieTile=%d, North=%d\n",
			 DevInst->ShimTileNumRowsSouth, DevInst->MemTileNumRows,
			 DevInst->AieTileNumRows, DevInst->ShimTileNumRowsNorth);
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ".aie_row_topology\t %d-%d-%d-%d\n", 
		DevInst->ShimTileNumRowsSouth, DevInst->MemTileNumRows, 
		DevInst->AieTileNumRows, DevInst->ShimTileNumRowsNorth);
	if (!ControlCodeInst->ControlCodefp) {
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ".aie_row_topology\t %d-%d-%d-%d\n", 
			DevInst->ShimTileNumRowsSouth, DevInst->MemTileNumRows, 
			DevInst->AieTileNumRows, DevInst->ShimTileNumRowsNorth);
	}

	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ".partition\t %dcolumn\n",DevInst->NumCols);
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ";\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ";text\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, ";\n");
	
	if (!ControlCodeInst->ControlCodefp) {
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ".partition\t %dcolumn\n",DevInst->NumCols);
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ";\n");
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ";text\n");
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ";\n");
	}
	
	ControlCodeInst->UcPageTextSize += ISA_OPSIZE_EOF;
	ControlCodeInst->UcPageSize += PAGE_HEADER_SIZE + ISA_OPSIZE_EOF;
	ControlCodeInst->IsPageOpen = 1;
	
	/* Write page info to debug buffer (only if file mode not active) */
	if (!ControlCodeInst->ControlCodefp) {
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, ";Page#: %d, PageSize: %d, TextSecSize: %d, DataAligner: %d\n",
			ControlCodeInst->PageId, ControlCodeInst->UcPageSize, ControlCodeInst->UcPageTextSize, ControlCodeInst->DataAligner);
	}

	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA, ";\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA, ";data\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA, ";\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA, ".align    16\n");
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODEDATA2, ".align    4\n");

	if (!ControlCodeInst->ControlCodefp) {
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA0, ";\n");
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA0, ";data\n");
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA0, ";\n");
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA0, ".align    16\n");
		CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASMDATA1, ".align    4\n");
	}

	/* Restore file pointers if they were temporarily NULLed to avoid duplicate header writes */
	if (file_mode_was_active) {
		ControlCodeInst->ControlCodefp = SavedControlCodefp;
		ControlCodeInst->ControlCodedatafp = SavedControlCodedatafp;
		ControlCodeInst->ControlCodedata2fp = SavedControlCodedata2fp;
		ControlCodeInst->DebugAsmFile = SavedDebugAsmFile;
		ControlCodeInst->DebugAsmFileData0 = SavedDebugAsmFileData0;
		ControlCodeInst->DebugAsmFileData1 = SavedDebugAsmFileData1;
	}

	XAIE_DBG("XAie_AllocControlCodeBuffer: Completed successfully\n");
	XAIE_DBG("Control code in-memory initialization complete\n");
	fflush(stdout);
	
	return XAIE_OK;
}

/*****************************************************************************/
/**
* Merges source memory buffer into destination buffer.
* @param	SrcBuf: Pointer to source buffer.
* @param	DesBuf: Pointer to destination buffer.
*
* @return	XAIE_OK on success, XAIE_ERR on failure.
*
* @note		Internal API only.
*
******************************************************************************/
static AieRC _XAie_MergeMemBuffers(XAie_MemBuffer *SrcBuf, XAie_MemBuffer *DesBuf) {
	if (!SrcBuf || !DesBuf) {
		XAIE_ERROR("Buffers not initialized\n");
		return XAIE_ERR;
	}
	
	/* Ensure destination buffer has enough capacity */
	size_t needed_capacity = DesBuf->Size + SrcBuf->Size;
	if (needed_capacity > DesBuf->Capacity) {
		size_t new_capacity = DesBuf->Capacity;
		while (new_capacity < needed_capacity) {
			new_capacity *= BUFFER_GROWTH_FACTOR;
		}
		
		char *new_data = (char*)realloc(DesBuf->Data, new_capacity);
		if (!new_data) {
			return XAIE_ERR;
		}
		DesBuf->Data = new_data;
		DesBuf->Capacity = new_capacity;
	}
	
	/* Append source buffer to destination */
	memcpy(DesBuf->Data + DesBuf->Size, SrcBuf->Data, SrcBuf->Size);
	DesBuf->Size += SrcBuf->Size;
	DesBuf->Data[DesBuf->Size] = '\0';  /* Ensure null termination */
	
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This function retrieves the ASM file content as an in-memory buffer
*
* @param	DevInst: Device instance pointer.
* @param	Buffer: Pointer to receive the buffer pointer (output parameter).
*                       The caller should NOT free this buffer.
* @param	Size: Pointer to receive the buffer size (output parameter).
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		This function should be called after all operations are complete
*           and before XAie_ReleaseControlCodeBuffer().
*           The buffer is owned by the library and will be freed when
*           XAie_ReleaseControlCodeBuffer() is called.
*
******************************************************************************/
AieRC XAie_GetControlCodeBuffer(XAie_DevInst *DevInst, const char **Buffer, size_t *Size)
{
	if(DevInst->Backend->Type != XAIE_IO_BACKEND_CONTROLCODE) {
		XAIE_ERROR("This is supported only in Controlcode Backend %d \n", DevInst->Backend->Type);
		return XAIE_INVALID_BACKEND;
	}
	
	if (!Buffer || !Size) {
		XAIE_ERROR("Invalid parameters: Buffer and Size cannot be NULL\n");
		return XAIE_ERR;
	}
	
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;
	
	if (!ControlCodeInst->UseInMemoryBuffers) {
		XAIE_ERROR("Control code was not opened in memory mode\n");
		return XAIE_ERR;
	}
	
	if (!ControlCodeInst->ControlCodeBuf) {
		XAIE_ERROR("Control code buffer not initialized\n");
		return XAIE_ERR;
	}
	
	/* Finalize the page by calling _XAie_EndPage (adds END_JOB and .eop) */
	_XAie_EndPage(ControlCodeInst);
	
	/* In dual mode (file + memory), temporarily NULL file pointer to avoid writing EOF to file */
	/* EOF will be written to file when XAie_CloseControlCodeFile is called */
	FILE *saved_fp = ControlCodeInst->ControlCodefp;
	if (saved_fp) {
		ControlCodeInst->ControlCodefp = NULL;
	}
	
	/* Finalize the buffer by adding EOF (only to buffer, not file) */
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "EOF\n\n");
	
	/* Restore file pointer */
	if (saved_fp) {
		ControlCodeInst->ControlCodefp = saved_fp;
	}
	
	/* Merge data sections into main buffer */
	if (_XAie_MergeMemBuffers(ControlCodeInst->ControlCodeDataBuf, ControlCodeInst->ControlCodeBuf) != XAIE_OK) {
		XAIE_ERROR("Failed to merge data buffer\n");
		/* Free intermediate buffers on error to prevent memory leak */
		_XAie_MemBufferFree(ControlCodeInst->ControlCodeDataBuf);
		_XAie_MemBufferFree(ControlCodeInst->ControlCodeData2Buf);
		ControlCodeInst->ControlCodeDataBuf = NULL;
		ControlCodeInst->ControlCodeData2Buf = NULL;
		return XAIE_ERR;
	}
	if (_XAie_MergeMemBuffers(ControlCodeInst->ControlCodeData2Buf, ControlCodeInst->ControlCodeBuf) != XAIE_OK) {
		XAIE_ERROR("Failed to merge data2 buffer\n");
		/* Free intermediate buffers on error to prevent memory leak */
		_XAie_MemBufferFree(ControlCodeInst->ControlCodeDataBuf);
		_XAie_MemBufferFree(ControlCodeInst->ControlCodeData2Buf);
		ControlCodeInst->ControlCodeDataBuf = NULL;
		ControlCodeInst->ControlCodeData2Buf = NULL;
		return XAIE_ERR;
	}
	
	/* Free intermediate data buffers after merge - no longer needed */
	_XAie_MemBufferFree(ControlCodeInst->ControlCodeDataBuf);
	_XAie_MemBufferFree(ControlCodeInst->ControlCodeData2Buf);
	ControlCodeInst->ControlCodeDataBuf = NULL;
	ControlCodeInst->ControlCodeData2Buf = NULL;
	
	*Buffer = ControlCodeInst->ControlCodeBuf->Data;
	*Size = ControlCodeInst->ControlCodeBuf->Size;
	
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This function retrieves the debug ASM file content as an in-memory buffer
*
* @param	DevInst: Device instance pointer.
* @param	Buffer: Pointer to receive the buffer pointer (output parameter).
*                       The caller should NOT free this buffer.
* @param	Size: Pointer to receive the buffer size (output parameter).
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		This function should be called after all operations are complete
*           and before XAie_ReleaseControlCodeBuffer().
*           The buffer is owned by the library and will be freed when
*           XAie_ReleaseControlCodeBuffer() is called.
*
******************************************************************************/
AieRC XAie_GetDebugAsmBuffer(XAie_DevInst *DevInst, const char **Buffer, size_t *Size)
{
	if(DevInst->Backend->Type != XAIE_IO_BACKEND_CONTROLCODE) {
		XAIE_ERROR("This is supported only in Controlcode Backend %d \n", DevInst->Backend->Type);
		return XAIE_INVALID_BACKEND;
	}
	
	if (!Buffer || !Size) {
		XAIE_ERROR("Invalid parameters: Buffer and Size cannot be NULL\n");
		return XAIE_ERR;
	}
	
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

	if (ControlCodeInst->DisableDebugAsm) {
		XAIE_ERROR("Debug ASM generation is disabled (DevInst->DisableDebugAsm is set).\n");
		return XAIE_ERR;
	}
	
	if (!ControlCodeInst->UseInMemoryBuffers) {
		XAIE_ERROR("Control code was not opened in memory mode\n");
		return XAIE_ERR;
	}
	
	if (!ControlCodeInst->DebugAsmBuf) {
		XAIE_ERROR("Debug ASM buffer not initialized\n");
		return XAIE_ERR;
	}
	
	/* Finalize the page by calling _XAie_EndPage (adds END_JOB and .eop) */
	/* Note: _XAie_EndPage writes to both ControlCodeBuf and DebugAsmBuf */
	if (ControlCodeInst->IsPageOpen) {
		_XAie_EndPage(ControlCodeInst);
	}
	
	/* In dual mode (file + memory), temporarily NULL debug file pointer to avoid writing EOF to file */
	/* EOF will be written to file when XAie_CloseControlCodeFile is called */
	FILE *saved_debug_fp = ControlCodeInst->DebugAsmFile;
	if (saved_debug_fp) {
		ControlCodeInst->DebugAsmFile = NULL;
	}
	
	/* Finalize the debug buffer by adding EOF (only to buffer, not file) */
	CONTROLCODE_PRINTF_CHECK(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "EOF\n\n");
	
	/* Restore debug file pointer */
	if (saved_debug_fp) {
		ControlCodeInst->DebugAsmFile = saved_debug_fp;
	}
	
	/* Merge debug data sections into debug buffer */
	if (_XAie_MergeMemBuffers(ControlCodeInst->DebugAsmDataBuf0, ControlCodeInst->DebugAsmBuf) != XAIE_OK) {
		XAIE_ERROR("Failed to merge debug data buffer 0\n");
		/* Free intermediate buffers on error to prevent memory leak */
		_XAie_MemBufferFree(ControlCodeInst->DebugAsmDataBuf0);
		_XAie_MemBufferFree(ControlCodeInst->DebugAsmDataBuf1);
		ControlCodeInst->DebugAsmDataBuf0 = NULL;
		ControlCodeInst->DebugAsmDataBuf1 = NULL;
		return XAIE_ERR;
	}
	if (_XAie_MergeMemBuffers(ControlCodeInst->DebugAsmDataBuf1, ControlCodeInst->DebugAsmBuf) != XAIE_OK) {
		XAIE_ERROR("Failed to merge debug data buffer 1\n");
		/* Free intermediate buffers on error to prevent memory leak */
		_XAie_MemBufferFree(ControlCodeInst->DebugAsmDataBuf0);
		_XAie_MemBufferFree(ControlCodeInst->DebugAsmDataBuf1);
		ControlCodeInst->DebugAsmDataBuf0 = NULL;
		ControlCodeInst->DebugAsmDataBuf1 = NULL;
		return XAIE_ERR;
	}
	
	/* Free intermediate data buffers after merge - no longer needed */
	_XAie_MemBufferFree(ControlCodeInst->DebugAsmDataBuf0);
	_XAie_MemBufferFree(ControlCodeInst->DebugAsmDataBuf1);
	ControlCodeInst->DebugAsmDataBuf0 = NULL;
	ControlCodeInst->DebugAsmDataBuf1 = NULL;
	
	*Buffer = ControlCodeInst->DebugAsmBuf->Data;
	*Size = ControlCodeInst->DebugAsmBuf->Size;
	
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This function closes the in-memory control code and frees all buffers
*
* @param	DevInst: Device instance pointer.
*
* @return	None.
*
******************************************************************************/
void XAie_ReleaseControlCodeBuffer(XAie_DevInst *DevInst)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

	if (!ControlCodeInst->UseInMemoryBuffers) {
		XAIE_ERROR("Control code was not opened in memory mode\n");
		return;
	}

	/* If file mode is still active, don't free buffers yet - they're still needed */
	if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
		/* Just mark in-memory mode as closed, buffers will be freed by XAie_CloseControlCodeFile */
		ControlCodeInst->UseInMemoryBuffers = 0;
		return;
	}

	/* Free control code memory buffers */
	_XAie_MemBufferFree(ControlCodeInst->ControlCodeBuf);
	_XAie_MemBufferFree(ControlCodeInst->ControlCodeDataBuf);
	_XAie_MemBufferFree(ControlCodeInst->ControlCodeData2Buf);
	ControlCodeInst->ControlCodeBuf = NULL;
	ControlCodeInst->ControlCodeDataBuf = NULL;
	ControlCodeInst->ControlCodeData2Buf = NULL;

	/* Free debug ASM memory buffers only if debug ASM was enabled */
	if (!ControlCodeInst->DisableDebugAsm) {
		_XAie_MemBufferFree(ControlCodeInst->DebugAsmBuf);
		_XAie_MemBufferFree(ControlCodeInst->DebugAsmDataBuf0);
		_XAie_MemBufferFree(ControlCodeInst->DebugAsmDataBuf1);
		ControlCodeInst->DebugAsmBuf = NULL;
		ControlCodeInst->DebugAsmDataBuf0 = NULL;
		ControlCodeInst->DebugAsmDataBuf1 = NULL;
	}

	if(ControlCodeInst->LabelMap) {
		if(_XAie_LabelMapTeardown(ControlCodeInst->LabelMap) == XAIE_OK) {
			XAIE_DBG("Label optimization teardown success\n");
		}
		free(ControlCodeInst->LabelMap);
		ControlCodeInst->LabelMap = NULL;
	}

	ControlCodeInst->UseInMemoryBuffers = 0;
}

/*****************************************************************************/
/**
* This function ends the current job and starts the new job
* @param DevInst AI engine device instance pointer
*
* @return  0 on success.
*
******************************************************************************/
AieRC XAie_StartNewJob(XAie_DevInst *DevInst, XAie_CertStartJobType JobType)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

	if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
		_XAie_StartNewJob(ControlCodeInst, JobType);
		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
* This function ends the current job
* @param DevInst AI engine device instance pointer
*
* @return  0 on success.
*
******************************************************************************/
AieRC XAie_EndJob(XAie_DevInst *DevInst) {
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

	if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
		_XAie_EndJob(ControlCodeInst);
		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/

/**
* This function ends the current page
* @param DevInst AI engine device instance pointer
*
* @return  0 on success.
*
******************************************************************************/
AieRC XAie_EndPage(XAie_DevInst *DevInst) {

	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

	if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
		_XAie_EndPage(ControlCodeInst);
		ControlCodeInst->PageBreak = 1;
		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
* Merges the given text file.
* @param	SrcFp: file pointer of Source File.
* @param	DesFile: file pointer of Destination File.
*
* @note		Internal API only.
*
******************************************************************************/
static void _XAie_MergeFiles(FILE *SrcFp, FILE *DesFp) {
	int TempBuf;

	if (!SrcFp || !DesFp) {
		XAIE_ERROR("Files not opened\n");
		return;
	}

	fseek(SrcFp, 0, SEEK_SET);
	fseek(DesFp, 0, SEEK_END);

	while ((TempBuf = fgetc(SrcFp)) != EOF) {
		fputc(TempBuf, DesFp);
	}

}

/*****************************************************************************/
/**
* This function used to stop the control code capture.
* This also merges the temp files and updates the control code file.
*
******************************************************************************/
void XAie_CloseControlCodeFile(XAie_DevInst *DevInst) {
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

	if (ControlCodeInst->ControlCodefp || ControlCodeInst->UseInMemoryBuffers) {
		_XAie_EndPage(ControlCodeInst);
		CONTROLCODE_PRINTF_VOID(ControlCodeInst, XAIE_FILE_TARGET_CONTROLCODE, "EOF\n\n");
		if (ControlCodeInst->DebugAsmFile) {
			CONTROLCODE_PRINTF_VOID(ControlCodeInst, XAIE_FILE_TARGET_DEBUGASM, "EOF\n\n");
		}

		_XAie_MergeFiles(ControlCodeInst->ControlCodedatafp, ControlCodeInst->ControlCodefp);
		_XAie_MergeFiles(ControlCodeInst->ControlCodedata2fp, ControlCodeInst->ControlCodefp);
		if (ControlCodeInst->DebugAsmFile && ControlCodeInst->DebugAsmFileData0) {
			_XAie_MergeFiles(ControlCodeInst->DebugAsmFileData0, ControlCodeInst->DebugAsmFile);
		}
		if (ControlCodeInst->DebugAsmFile && ControlCodeInst->DebugAsmFileData1) {
			_XAie_MergeFiles(ControlCodeInst->DebugAsmFileData1, ControlCodeInst->DebugAsmFile);
		}

		fclose(ControlCodeInst->ControlCodefp);
		if (ControlCodeInst->ControlCodedatafp) fclose(ControlCodeInst->ControlCodedatafp);
		if (ControlCodeInst->ControlCodedata2fp) fclose(ControlCodeInst->ControlCodedata2fp);
		if (ControlCodeInst->DebugAsmFile) fclose(ControlCodeInst->DebugAsmFile);
		if (ControlCodeInst->DebugAsmFileData0) fclose(ControlCodeInst->DebugAsmFileData0);
		if (ControlCodeInst->DebugAsmFileData1) fclose(ControlCodeInst->DebugAsmFileData1);
		
		memset(FName, '\0', FNAME_SIZE);

		remove(TEMP_ASM_FILE1);
		remove(TEMP_ASM_FILE2);
		if (!ControlCodeInst->DisableDebugAsm) {
			remove(TEMP_ASM_FILE3);
			remove(TEMP_ASM_FILE4);
		}

		ControlCodeInst->ControlCodefp = NULL;
		ControlCodeInst->ControlCodedatafp = NULL;
		ControlCodeInst->ControlCodedata2fp = NULL;
		ControlCodeInst->DebugAsmFile = NULL;
		ControlCodeInst->DebugAsmFileData0 = NULL;
		ControlCodeInst->DebugAsmFileData1 = NULL;

		/* Free memory buffers if they exist (from dual mode usage) */
		if (ControlCodeInst->ControlCodeBuf) {
			_XAie_MemBufferFree(ControlCodeInst->ControlCodeBuf);
			_XAie_MemBufferFree(ControlCodeInst->ControlCodeDataBuf);
			_XAie_MemBufferFree(ControlCodeInst->ControlCodeData2Buf);
			ControlCodeInst->ControlCodeBuf = NULL;
			ControlCodeInst->ControlCodeDataBuf = NULL;
			ControlCodeInst->ControlCodeData2Buf = NULL;
		}

		if (!ControlCodeInst->DisableDebugAsm) {
			_XAie_MemBufferFree(ControlCodeInst->DebugAsmBuf);
			_XAie_MemBufferFree(ControlCodeInst->DebugAsmDataBuf0);
			_XAie_MemBufferFree(ControlCodeInst->DebugAsmDataBuf1);
			ControlCodeInst->DebugAsmBuf = NULL;
			ControlCodeInst->DebugAsmDataBuf0 = NULL;
			ControlCodeInst->DebugAsmDataBuf1 = NULL;
		}

		if(ControlCodeInst->LabelMap) {
			if(_XAie_LabelMapTeardown(ControlCodeInst->LabelMap) == XAIE_OK) {
				XAIE_DBG("Label optimization teardown success\n");
			}
			free(ControlCodeInst->LabelMap);
			ControlCodeInst->LabelMap = NULL;
		}
	}
}

/*****************************************************************************/
/**
*
* This function is used to set scrachpad string.
*
* @param        DevInst: Device instance pointer.
* @param        Scrachpad: It is the name of a declared scratchpad buffer
* @return       XAIE_OK or XAIE_ERR.
*
*
*******************************************************************************/
AieRC XAie_ControlCodeSetScrachPad(XAie_DevInst *DevInst, const char *Scrachpad)
{
        XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

        ControlCodeInst->ScrachpadName = (char *)calloc(strlen(Scrachpad) + 1, sizeof(char));
        if(ControlCodeInst->ScrachpadName == NULL)
                return XAIE_ERR;
        else {
                snprintf(ControlCodeInst->ScrachpadName, strlen(Scrachpad)+1, "%s", Scrachpad);
                return XAIE_OK;
        }
}

#else

AieRC XAie_OpenControlCodeFile(XAie_DevInst *DevInst, const char *FileName, u32 PageSize)
{
	/* no-op */
	(void)DevInst;
	(void)FileName;
	(void)PageSize;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeAddAnnotation(XAie_DevInst *DevInst,
               u32 Id, const char *Name, const char *Description)
{
        (void)DevInst;
        (void)Id;
        (void)Name;
        (void)Description;

        XAIE_ERROR("Driver is not compiled with ControlCode generation "
                        "backend (__AIECONTROLCODE__)\n");
       return XAIE_INVALID_BACKEND;
}

AieRC XAie_StartNewJob(XAie_DevInst *DevInst, XAie_CertStartJobType JobType)
{
	/* no-op */
	(void)DevInst;
	(void)JobType;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_EndJob(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_EndPage(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

void XAie_CloseControlCodeFile(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return;
}

AieRC XAie_WaitTaskCompleteToken(XAie_DevInst *DevInst, uint16_t Column, uint16_t Row, uint32_t Channel, uint8_t NumTokens)
{
	/* no-op */
	(void)DevInst;
	(void)Column;
	(void)Row;
	(void)Channel;
	(void)NumTokens;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeIO_Finish(void *IOInst)
{
	/* no-op */
	(void)IOInst;
	return XAIE_OK;
}

AieRC XAie_ControlCodeIO_Init(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Value;

	return XAIE_ERR;
}

AieRC XAie_ControlCodeIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	return 0;
}

AieRC XAie_ControlCodeIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;

	return XAIE_ERR;
}

AieRC XAie_ControlCodeIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;
	(void)TimeOutUs;

	return XAIE_ERR;
}

AieRC XAie_ControlCodeIO_BlockWrite32(void *IOInst, u64 RegOff, const u32 *Data,
		u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;

	return XAIE_ERR;
}

AieRC XAie_ControlCodeIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;

	return XAIE_ERR;
}

AieRC XAie_ControlCodeIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		     XAie_BackendOpCode Op, void *Arg)
{
	(void)IOInst;
	(void)DevInst;
	(void)Op;
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

AieRC XAie_ControlCodeIO_AddressPatching(void *IOInst, u16 Arg_Index, u8 Num_BDs)
{
	/* no-op */
	(void)IOInst;
	(void)Arg_Index;
	(void)Num_BDs;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeAddComment(XAie_DevInst *DevInst, const char *Comment)
{
        (void)DevInst;
        (void)Comment;
        XAIE_ERROR("Driver is not compiled with ControlCode generation "
                        "backend (__AIECONTROLCODE__)\n");
        return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeSaveTimestamp(XAie_DevInst *DevInst, u32 Timestamp)
{
        (void)DevInst;
        (void)Timestamp;
        XAIE_ERROR("Driver is not compiled with ControlCode generation "
                        "backend (__AIECONTROLCODE__)\n");
        return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeSetScrachPad(XAie_DevInst *DevInst, const char *Scrachpad)
{
        (void)DevInst;
        (void)Scrachpad;
        XAIE_ERROR("Driver is not compiled with ControlCode generation "
                        "backend (__AIECONTROLCODE__)\n");
        return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeIO_Nop(void *IOInst)
{
	/* no-op */
	(void)IOInst;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeIO_WaitUcDMA(void *IOInst)
{
	/* no-op */
	(void)IOInst;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
		"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_ConfigMode(void *IOInst, XAie_ModeSelect Mode)
{
	/* no-op */
	(void)IOInst;
	(void)Mode;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
		"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

XAie_ModeSelect XAie_GetConfigMode(void *IOInst)
{
	/* no-op */
	(void)IOInst;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
		"backend (__AIECONTROLCODE__), hence default mode returned by this API is XAIE_INVALID_MODE\n");
	return XAIE_INVALID_MODE;
}

AieRC XAie_ControlCodeIO_Preempt(void *IOInst, u16 PreemptId, char* SaveLabel, char* RestoreLabel, u32* HintMap, u32 HintMapSizeInWords)
{
	/* no-op */
	(void)IOInst;
	(void)PreemptId;
	(void)SaveLabel;
	(void)RestoreLabel;
	(void)HintMap;
	(void)HintMapSizeInWords;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeIO_SetPadInteger(void *IOInst, char* BuffName, u32 BuffSize)
{
	/* no-op */
	(void)IOInst;
	(void)BuffName;
	(void)BuffSize;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeIO_SetPadString(void *IOInst, char* BuffName, char* BuffBlobPath)
{
	/* no-op */
	(void)IOInst;
	(void)BuffName;
	(void)BuffBlobPath;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeIO_AttachToGroup(void *IOInst, uint8_t UcIndex)
{
	/* no-op */
	(void)IOInst;
	(void)UcIndex;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeIO_RemoteBarrier(void *IOInst, uint8_t RbId, uint32_t UcMask)
{
	/* no-op */
	(void)IOInst;
	(void)RbId;
	(void)UcMask;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeIO_SaveRegister(void *IOInst, u32 RegOff, u32 Id)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Id;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeRelAcqSync(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
		XAie_LocType Loc, XAie_Lock RelLock,  XAie_Lock AcqLock)
{
	/* no-op */
	(void)DevInst;
	(void)LockMod;
	(void)Loc;
	(void)RelLock;
	(void)AcqLock;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;

}

#endif /* __AIECONTROLCODE__ */

static AieRC XAie_ControlCodeIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr)
{
	/* no-op */
	(void)IOInst;
	(void)Col;
	(void)Row;
	(void)Command;
	(void)CmdWd0;
	(void)CmdWd1;
	(void)CmdStr;

	return XAIE_OK;
}

static XAie_MemInst* XAie_ControlCodeMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	(void)DevInst;
	(void)Size;
	(void)Cache;
	return NULL;
}

static AieRC XAie_ControlCodeMemFree(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_ControlCodeMemSyncForCPU(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_ControlCodeMemSyncForDev(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_ControlCodeMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	(void)MemInst;
	(void)MemHandle;
	return XAIE_ERR;
}

static AieRC XAie_ControlCodeMemDetach(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

const XAie_Backend ControlCodeBackend =
{
	.Type = XAIE_IO_BACKEND_CONTROLCODE,
	.Ops.Init = XAie_ControlCodeIO_Init,
	.Ops.Finish = XAie_ControlCodeIO_Finish,
	.Ops.Write32 = XAie_ControlCodeIO_Write32,
	.Ops.Read32 = XAie_ControlCodeIO_Read32,
	.Ops.MaskWrite32 = XAie_ControlCodeIO_MaskWrite32,
	.Ops.MaskPoll = XAie_ControlCodeIO_MaskPoll,
	.Ops.BlockWrite32 = XAie_ControlCodeIO_BlockWrite32,
	.Ops.BlockSet32 = XAie_ControlCodeIO_BlockSet32,
	.Ops.CmdWrite = XAie_ControlCodeIO_CmdWrite,
	.Ops.RunOp = XAie_ControlCodeIO_RunOp,
	.Ops.AddressPatching = XAie_ControlCodeIO_AddressPatching,
	.Ops.WaitTaskCompleteToken = XAie_WaitTaskCompleteToken,
	.Ops.MemAllocate = XAie_ControlCodeMemAllocate,
	.Ops.MemFree = XAie_ControlCodeMemFree,
	.Ops.MemSyncForCPU = XAie_ControlCodeMemSyncForCPU,
	.Ops.MemSyncForDev = XAie_ControlCodeMemSyncForDev,
	.Ops.MemAttach = XAie_ControlCodeMemAttach,
	.Ops.MemDetach = XAie_ControlCodeMemDetach,
	.Ops.GetTid = XAie_IODummyGetTid,
	.Ops.ConfigMode = XAie_ConfigMode,
	.Ops.WaitUcDMA = XAie_ControlCodeIO_WaitUcDMA,
	.Ops.GetConfigMode = XAie_GetConfigMode,
	.Ops.Preempt = XAie_ControlCodeIO_Preempt,
	.Ops.SetPadInteger = XAie_ControlCodeIO_SetPadInteger,
	.Ops.SetPadString = XAie_ControlCodeIO_SetPadString,
	.Ops.AttachToGroup = XAie_ControlCodeIO_AttachToGroup,
	.Ops.RemoteBarrier = XAie_ControlCodeIO_RemoteBarrier,
	.Ops.SaveRegister = XAie_ControlCodeIO_SaveRegister,
	.Ops.Nop = XAie_ControlCodeIO_Nop,
	.Ops.SubmitTxn = NULL,
};

/** @} */


