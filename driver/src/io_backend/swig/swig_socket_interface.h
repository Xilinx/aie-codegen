/******************************************************************************
* Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_socket.h
* @{
*
* This file contains Public socket backend functions for AIE drivers.
*
******************************************************************************/
#ifndef XAIESOCKET_H
#define XAIESOCKET_H

#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaie_io_common.h"
#include "xaie_io_privilege.h"
#include "xaie_npi.h"
#include "xaiegbl.h"

/***************************** Macro Definitions *****************************/
#define XAIE_IO_SOCKET_CMDBUFSIZE	48U
#define XAIE_IO_SOCKET_RDBUFSIZE	11U /* "0xDEADBEEF\n" */

/****************************** Type Definitions *****************************/
#ifdef __SWIGINTERFACE__
// redirect XAIE_ERROR to printf
#define XAIE_ERROR	printf
// no need for debug/warn printf so empty macro
#define XAIE_DBG
#define XAIE_WARN
#endif

typedef struct {
	u64 BaseAddr;
	u64 NpiBaseAddr;
	int SocketFd;
} Swig_SocketIO;

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
int XAie_SocketIO_swig_Write32(Swig_SocketIO IOInst, u64 RegOff, u32 Value);
int XAie_SocketIO_swig_Read32(Swig_SocketIO IOInst, u64 RegOff, u32 Data);
int XAie_SocketIO_swig_Init(Swig_DevInst *DevInst);
int XAie_SocketIO_swig_Finish(Swig_SocketIO IOInst);

/***
 * Socket Backend API Prototype definitions
 */
AieRC XAie_SocketIO_Finish(void *IOInst);
AieRC XAie_SocketIO_Init(XAie_DevInst *DevInst);
AieRC XAie_SocketIO_Write32(void *IOInst, u64 RegOff, u32 Value);
AieRC XAie_SocketIO_Read32(void *IOInst, u64 RegOff, u32 *Data);
#endif /* XAIESOCKET_H */