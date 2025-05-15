/******************************************************************************
* Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file swig_controlcode_interface.h
* @{
*
* This file contains swig mapping controlcode backend functions for AIE drivers.
*
******************************************************************************/
#ifndef XAIECONTROLCODE_H
#define XAIECONTROLCODE_H

#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaie_io_common.h"
#include "xaie_io_privilege.h"
#include "xaie_npi.h"
#include "xaiegbl.h"

/***************************** Macro Definitions *****************************/

/****************************** Type Definitions *****************************/
#ifdef __SWIGINTERFACE__
// redirect XAIE_ERROR to printf
#define XAIE_ERROR       printf
// no need for debug/warn printf so empty macro
#define XAIE_DEBUG
#define XAIE_WARN
#endif

/* Forward declaration of the ControlCodeBackend variable */
extern const XAie_Backend ControlCodeBackend;

typedef struct {
	XAie_DevInst *DevInst;
	u64 BaseAddr;
	u64 NpiBaseAddr;
	FILE *ControlCodefp;
	FILE *ControlCodedatafp;
	FILE *ControlCodedata2fp;
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
} Swig_ControlCodeIO;

typedef struct {
	u64 BaseAddr; /* Base address of the partition*/
	u8 StartCol;  /* Absolute start column of the partition */
	u8 NumRows;   /* Number of rows allocated to the partition */
	u8 NumCols;   /* Number of cols allocated to the partition */
	u8 ShimRow;   /* ShimRow location */
	u8 MemTileRowStart; /* Mem tile starting row in the partition */
	u8 MemTileNumRows;  /* Number of memtile rows in the partition */
	u8 AieTileRowStart; /* Aie tile starting row in the partition */
	u8 AieTileNumRows;  /* Number of aie tile rows in the partition */
	u8 IsReady;
	u8 EccStatus;		/* Ecc On/Off status of the partition */
	u8 AppMode;		/* 0 - Single Application,1 - Application A, 2 - Application B */
	u8 L2Split;     /* Set L2 Split in Dual App Mode */
	u8 L2PreserveMem;    /*Set or Clear to preserve L2 Memory Data */
	u8 PmLoadingActive; /*To keep track of of PM Loading is active or not*/
	const XAie_Backend *Backend; /* Backend IO properties */
	void *IOInst;	       /* IO Instance for the backend */
	XAie_DevProp DevProp; /* Pointer to the device property. To be
				     setup to AIE prop during intialization*/
	XAie_DeviceOps *DevOps; /* Device level operations */
	XAie_PartitionProp PartProp; /* Partition property */
	XAie_List TxnList; /* Head of the list of txn buffers */
} Swig_DevInst;

typedef struct {
	u8 AieGen;
	u64 BaseAddr;
	u8 ColShift;
	u8 RowShift;
	u8 NumRows;
	u8 NumCols;
	u8 ShimRowNum;
	u8 MemTileRowStart;
	u8 MemTileNumRows;
	u8 AieTileRowStart;
	u8 AieTileNumRows;
	XAie_PartitionProp PartProp;
} Swig_Config;

/***
 * SWIG Interface API Prototype definitions
 */
int XAie_ControlCodeIO_swig_Finish(Swig_ControlCodeIO IOInst);
int XAie_ControlCodeIO_swig_Init(Swig_DevInst *DevInst);
int XAie_ControlCodeIO_swig_ConfigMode(Swig_DevInst *DevInst, XAie_ModeSelect Mode);
XAie_ModeSelect XAie_ControlCodeIO_swig_GetConfigMode(Swig_DevInst *DevInst);
int XAie_ControlCodeIO_swig_Write32(Swig_DevInst *DevInst, u64 RegOff, u32 Value);
int XAie_ControlCodeIO_swig_Read32(Swig_DevInst *DevInst, u64 RegOff, u32 *Data);
int XAie_ControlCodeIO_swig_MaskWrite32(Swig_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value);
int XAie_ControlCodeIO_swig_MaskPoll(Swig_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value, u32 TimeOutUs);
int XAie_ControlCodeIO_swig_BlockWrite32(Swig_DevInst *DevInst, u64 RegOff, u32 *Data, u32 DataSize);
int XAie_ControlCodeIO_swig_BlockSet32(Swig_DevInst *DevInst, u64 RegOff, u32 Data, u32 DataSize);
int XAie_ControlCodeIO_swig_AddressPatching(Swig_DevInst *DevInst, u32 Arg_Index, u8 Num_BDs);
int XAie_ControlCodeIO_swig_WaitUcDMA(Swig_DevInst *DevInst);
int XAie_ControlCodeIO_swig_WaitTaskCompleteToken(Swig_DevInst *DevInst, u16 Column, u16 Row, u32 Channel, u8 NumTokens);
int XAie_ControlCodeIO_swig_SaveTimestamp(Swig_DevInst *DevInst, u32 Timestamp);
int XAie_ControlCodeIO_swig_RunOp(Swig_DevInst *DevInst, XAie_BackendOpCode Op, void *Arg);
int XAie_ControlCodeIO_swig_OpenControlCodeFile(Swig_DevInst *DevInst, const char *FileName, u32 PageSize);
int XAie_ControlCodeIO_swig_StartNewJob(Swig_DevInst *DevInst);
int XAie_ControlCodeIO_swig_EndJob(Swig_DevInst *DevInst);
int XAie_ControlCodeIO_swig_EndPage(Swig_DevInst *DevInst);
void XAie_ControlCodeIO_swig_CloseControlCodeFile(Swig_DevInst *DevInst);
int XAie_ControlCodeIO_swig_ControlCodeAddAnnotation(Swig_DevInst *DevInst, u32 Id, const char *Name, const char *Description);
int XAie_ControlCodeIO_swig_ControlCodeSetScrachPad(Swig_DevInst *DevInst, const char *Scrachpad);

/***
 * Control Code Backend API Prototype definitions
 */
AieRC XAie_ControlCodeIO_Finish(void *IOInst);
AieRC XAie_ControlCodeIO_Init(XAie_DevInst *DevInst);
AieRC XAie_ConfigMode(void *IOInst, XAie_ModeSelect Mode);
XAie_ModeSelect XAie_GetConfigMode(void *IOInst);
AieRC XAie_ControlCodeIO_Write32(void *IOInst, u64 RegOff, u32 Value);
AieRC XAie_ControlCodeIO_Read32(void *IOInst, u64 RegOff, u32 *Data);
AieRC XAie_ControlCodeIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Value);
AieRC XAie_ControlCodeIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value, u32 TimeOutUs);
AieRC XAie_ControlCodeIO_BlockWrite32(void *IOInst, u64 RegOff, const u32 *Data, u32 Size);
AieRC XAie_ControlCodeIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size);
AieRC XAie_ControlCodeIO_AddressPatching(void *IOInst, u32 Arg_Index, u8 Num_BDs);
AieRC XAie_ControlCodeIO_WaitUcDMA(void *IOInst);
AieRC XAie_WaitTaskCompleteToken(XAie_DevInst *DevInst, uint16_t Column, uint16_t Row, uint32_t Channel, uint8_t NumTokens);
AieRC XAie_ControlCodeSaveTimestamp(XAie_DevInst *DevInst, u32 Timestamp);
AieRC XAie_ControlCodeIO_RunOp(void *IOInst, XAie_DevInst *DevInst, XAie_BackendOpCode Op, void *Arg);
AieRC XAie_OpenControlCodeFile(XAie_DevInst *DevInst, const char *FileName, u32 PageSize);
AieRC XAie_StartNewJob(XAie_DevInst *DevInst);
AieRC XAie_EndJob(XAie_DevInst *DevInst);
AieRC XAie_EndPage(XAie_DevInst *DevInst);
void XAie_CloseControlCodeFile(XAie_DevInst *DevInst);
AieRC XAie_ControlCodeAddAnnotation(XAie_DevInst *DevInst, u32 Id, const char *Name, const char *Description);
AieRC XAie_ControlCodeSetScrachPad(XAie_DevInst *DevInst, const char *Scrachpad);
#endif