/******************************************************************************
* Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef AIE_CODEGEN_HPP
#define AIE_CODEGEN_HPP

namespace aie_codegen {
//=============================xaiengine/xaiegbl_defs.h

/*****************************************************************************/
/**
* @file xaiegbl_defs.h
*
* This file contains the generic definitions for the AIE drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/23/2018  Initial creation
* 1.1  Naresh  07/11/2018  Updated copyright info
* 1.2  Hyun    10/10/2018  Added the mask write API
* 1.3  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.4  Hyun    01/08/2019  Add the mask poll function
* 1.5  Tejus   08/01/2019  Restructure code for AIE
* 1.6  Dishita 04/17/2020  Fix compiler warning
* 1.7  Dishita 05/07/2020  Removed Reset related macros
* 1.8  Tejus   06/09/2020  Remove NPI apis.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stddef.h>

/************************** Constant Definitions *****************************/
typedef int8_t			s8;
typedef uint8_t			u8;
typedef uint16_t		u16;
typedef int32_t			s32;
typedef uint32_t		u32;
typedef uint64_t		u64;

constexpr u32 _XAIE_DEV_GENERIC_DEVICE		= 0U;
constexpr u32 _XAIE_DEV_GEN_AIE		= 1U;
constexpr u32 _XAIE_DEV_GEN_AIEML		= 2U;
constexpr u32 _XAIE_DEV_GEN_AIE2IPU   	        = 3U;
constexpr u32 _XAIE_DEV_GEN_AIE2P		= 4U;
constexpr u32 _XAIE_DEV_GEN_AIE2PS		= 5U;
constexpr u32 _XAIE_DEV_GEN_S100		= 6U;
constexpr u32 _XAIE_DEV_GEN_S200		= 7U;
constexpr u32 _XAIE_DEV_GEN_AIE2P_STRIX_A0	= 8U;
constexpr u32 _XAIE_DEV_GEN_AIE2P_STRIX_B0	= 9U;
constexpr u32 _XAIE_DEV_GEN_AIE4_A     = 40U;
constexpr u32 _XAIE_DEV_GEN_AIE4_GENERIC               = 41U;
constexpr u32 _XAIE_DEV_GEN_AIE4        = 42U;

constexpr u32 _XAIE4_MASK_VALUE_APP_B	= 0x40000;

constexpr u32 _XAIE_COMPONENT_IS_READY		= 1U;

constexpr void* _XAIE_NULL			= nullptr;
constexpr u32 _XAIE_ENABLE			= 1U;
constexpr u32 _XAIE_DISABLE			= 0U;
constexpr u32 _XAIE_MASK           		= 1U;
constexpr u32 _XAIE_UNMASK         		= 0U;

constexpr u32 _XAIEGBL_TILE_TYPE_AIETILE	= 0U;
constexpr u32 _XAIEGBL_TILE_TYPE_SHIMNOC	= 1U;
constexpr u32 _XAIEGBL_TILE_TYPE_SHIMPL	= 2U;
constexpr u32 _XAIEGBL_TILE_TYPE_MEMTILE	= 3U;
constexpr u32 _XAIEGBL_TILE_TYPE_MAX		= 4U;

constexpr u32 _XAie_SetField(u32 Val, u32 Lsb, u32 Mask) {
	return ((Val << Lsb) & Mask);
}

constexpr u32 _XAie_GetField(u32 Val, u32 Lsb, u32 Mask) {
	return ((Val & Mask) >> Lsb);
}

constexpr u32 _REGISTER_NOT_AVAILABLE = 0xFFFFFFFF;
/************************** Variable Definitions *****************************/
/************************** Function Prototypes  *****************************/
//=============================xaiengine/xaiegbl.h

/*****************************************************************************/
/**
* @file xaiegbl.h
*
* This file contains the instances of the register bit field definitions for the
* Core, Memory, NoC and PL module registers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   10/28/2019  Add error type for pl interface apis
* 1.2   Tejus   12/09/2019  Forward declaration of structures
* 1.3   Tejus   03/17/2020  Add error types and data structure for lock apis
* 1.4   Tejus   03/21/2020  Add data strcuture and initialization function for
*			    packets.
* 1.5   Tejus   03/22/2020  Remove data strcutures from initial dma api
*			    implementation
* 1.6   Tejus   03/23/2020  Add data structures for common dma apis.
* 1.7   Tejus   04/13/2020  Remove range structure and helper function.
* 1.8   Dishita 04/27/2020  Add enum for reset and modules.
* 1.9   Tejus   06/05/2020  Change name of FifoMode field.
* 2.0   Nishad  06/18/2020  Add macros for max value of packet Id and type.
* 2.1   Tejus   06/10/2020  Add IO backend data structures.
* 2.2   Tejus   06/10/2020  Add ess simulation backend.
* 2.3   Tejus   06/10/2020  Add api to change backend at runtime.
* </pre>
*
******************************************************************************/

/************************** Constant Definitions *****************************/
constexpr s8 _XAIE_LOCK_WITH_NO_VALUE = -1;
constexpr u32 _XAIE_PACKET_ID_MAX = 0x1FU;
constexpr u32 _XAIE_PACKET_TYPE_MAX = 0x7U;
constexpr u32 _XAIE_TILES_BITMAP_SIZE = 32U;
constexpr u32 _XAIE_MEMINTERLEAVE_MODE_SHIFT = 5U;

constexpr u32 _XAIE_TRANSACTION_ENABLE_AUTO_FLUSH = 0b1U;
constexpr u32 _XAIE_TRANSACTION_DISABLE_AUTO_FLUSH = 0b0U;

constexpr u32 _XAIE_PART_INIT_OPT_COLUMN_RST = (1U << 0);
constexpr u32 _XAIE_PART_INIT_OPT_SHIM_RST = (1U << 1);
constexpr u32 _XAIE_PART_INIT_OPT_BLOCK_NOCAXIMMERR = (1U << 2);
constexpr u32 _XAIE_PART_INIT_OPT_ISOLATE = (1U << 3);
constexpr u32 _XAIE_PART_INIT_OPT_ZEROIZEMEM = (1U << 4);
constexpr u32 _XAIE_PART_INIT_OPT_CONFIG_MEMINTERLEAVING = (3U << 5);
constexpr u32 _XAIE_PART_INIT_OPT_UC_MEM_PRIV = (1U << 7);
constexpr u32 _XAIE_PART_INIT_OPT_APP_MODE = (3U << 8);
constexpr u32 _XAIE_PART_INIT_OPT_L2_SPLIT = (31U << 10);
constexpr u32 _XAIE_PART_INIT_OPT_DEFAULT = (_XAIE_PART_INIT_OPT_COLUMN_RST | 
		_XAIE_PART_INIT_OPT_SHIM_RST | 
		_XAIE_PART_INIT_OPT_BLOCK_NOCAXIMMERR | 
		_XAIE_PART_INIT_OPT_UC_MEM_PRIV | 
		_XAIE_PART_INIT_OPT_ISOLATE);

/**************************** Constant Macros *******************************/
constexpr u32 _XAIE_PART_INIT_APP_MODE(u32 X) { return (X & 3U) << 8; }
constexpr u32 _XAIE_PART_INIT_L2_SPLIT(u32 X) { return (X & 31U) << 10; }

/* Migrated from AIE-CONTROLLER */
#define _OP_LIST(OP) \
		OP(TRANSACTION_OP) \
		OP(WAIT_OP) \
		OP(PENDINGBDCOUNT_OP) \
		OP(DBGPRINT_OP) \
		OP(PATCHBD_OP) \
		OP(TRANSACTION_V2_OP)
#define _GENERATE_ENUM(ENUM) e_##ENUM,

constexpr u32 _XAIE_INIT_ISOLATION = 0;
constexpr u32 _XAIE_CLEAR_ISOLATION = 1;
constexpr u32 _XAIE_INIT_WEST_ISOLATION = 2;
constexpr u32 _XAIE_INIT_EAST_ISOLATION = 4;
constexpr u32 _XAIE_PERF_CORE_NUM_CYCLES = 2U;
/**************************** Type Definitions *******************************/
typedef struct XAie_TileMod XAie_TileMod;
typedef struct XAie_DeviceOps XAie_DeviceOps;
typedef struct XAie_DmaMod XAie_DmaMod;
typedef struct XAie_LockMod XAie_LockMod;
typedef struct XAie_Backend XAie_Backend;
typedef struct XAie_TxnCmd XAie_TxnCmd;
typedef struct XAie_ResourceManager XAie_ResourceManager;

/*
 * This typedef captures all the properties of a AIE Device
 */
typedef struct XAie_DevProp {
	u8 DevGen;
	u8 RowShift;
	u8 ColShift;
	const XAie_TileMod *DevMod;
} XAie_DevProp;

/*
 * This typedef captures all the IO Backends supported by the driver
 */
 typedef enum class XAie_BackendType {
	XAIE_IO_BACKEND_SIM,   /* Ess simulation backend */
	XAIE_IO_BACKEND_CDO,   /* Cdo generation backend */
	XAIE_IO_BACKEND_DEBUG, /* IO debug backend */
	XAIE_IO_BACKEND_IPU, /* IPU Backend */
	XAIE_IO_BACKEND_SOCKET, /* Socket backend */
	XAIE_IO_BACKEND_CONTROLCODE,
	XAIE_IO_BACKEND_MAX
} XAie_BackendType;

/*
 * This typedef captures all the Application Modes supported by the driver
 */
 typedef enum class XAie_DeviceMode {
	XAIE_DEVICE_SINGLE_APP_MODE, /* Single Application Mode */
	XAIE_DEVICE_DUAL_APP_MODE_A,   /* Dual Application Mode , Application A */
	XAIE_DEVICE_DUAL_APP_MODE_B,   /* Dual Application Mode, Application B */
	XAIE_DEVICE_INVALID_MODE, /* Invalid Mode */
} XAie_DeviceMode;

/*
 * This typedef contains the attributes for an AIE partition properties.
 * It will contain the fields required to intialize the AIE partition software
 * instance.
 */
 typedef struct XAie_PartitionProp {
	u64 Handle;	/* AI engine partition handle. If AI engine handle is
			 * specified, the NID, and the UID will be ignored. It
			 * is used in case of Linux Xilinx runtime stack. */
	u32 Nid;	/* Partition Node ID */
	u32 Uid;	/* UID of the image runs on the AI engine partition */
	u32 CntrFlag;	/* AI enigne parition control flag. E.g.to indicate
			 * if to reset and gate all the tiles when the parition
			 * is closed. */
} XAie_PartitionProp;

/* Generic linked list structure */
 typedef struct XAie_List {
	struct XAie_List *Next;
} XAie_List;

/*
 * This typedef contains the attributes for an AIE partition. The structure is
 * setup during intialization.
 */
typedef struct {
	u64 BaseAddr; /* Base address of the partition*/
	u8 StartCol;  /* Absolute start column of the partition */
	u8 NumRows;   /* Number of rows allocated to the partition */
	u8 NumCols;   /* Number of cols allocated to the partition */
	u8 ShimRow;   /* ShimRow location */
	u8 ShimTileNumRowsNorth; /* Number of North Shim rows in the partition */
	u8 ShimTileNumRowsSouth; /* Number of South Shim rows in the partition */
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
	u8 DevType;         /* Device type for thread-safe device-specific logic (replaces global XAieDevType) */
	const XAie_Backend *Backend; /* Backend IO properties */
	void *IOInst;	       /* IO Instance for the backend */
	XAie_DevProp DevProp; /* Pointer to the device property. To be
				     setup to AIE prop during intialization*/
	const XAie_DeviceOps *DevOps; /* Device level operations */
	XAie_PartitionProp PartProp; /* Partition property */
	XAie_List TxnList; /* Head of the list of txn buffers */
	u32 InitialTxnCmdArraySize; /* TXN command array max size to begin with */
        XAie_List PartitionList;
	
	/*use to clear A2S buffer (work-around for data leak)*/
	u64 HostddrBuffAddr;
	size_t HostddrBuffSize;
	u32 HostddrBuff_SMID;
	u32 HostddrBuff_AxUSER;
} XAie_DevInst;

/* enum to capture cache property of allocate memory */
typedef enum class XAie_MemCacheProp{
	XAIE_MEM_CACHEABLE,
	XAIE_MEM_NONCACHEABLE
} XAie_MemCacheProp;

/* typedef to capture properties of an allcoated memory buffer */
typedef struct {
	u64 Size;
	void *VAddr;
	u64 DevAddr;
	XAie_MemCacheProp Cache;
	XAie_DevInst *DevInst;
	void *BackendHandle; /* Backend specific properties */
} XAie_MemInst;

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
	u8 ShimTileNumRowsNorth;
	u8 ShimTileNumRowsSouth;
} XAie_Config;

/*
 * This typedef contains attributes for a tile coordinate.
 */
typedef struct {
	u8 Row;
	u8 Col;
} XAie_LocType;

/*
 * This typedef captures the start and total count of an attribute.
 */
typedef struct {
	u8 Start;
	u8 Num;
} XAie_Range;

/*
 * This typedef contains the attributes for an AIE partition initialization
 * options. The structure is used by the AI engine partition initialization
 * API.
 */
typedef struct XAie_PartInitOpts {
	XAie_LocType *Locs; /* Array of tiles locations which will be used */
	u32 NumUseTiles; /* Number of tiles to use */
	u32 InitOpts; /* AI engine partition initialization options */
} XAie_PartInitOpts;

/*
 *
 * This typedef contains the attributes for AIE partiton/ device partition
 *
 */
typedef struct XAie_DevicePartInfo {
	u8 StartCol;  /* Absolute start column of the partition */
	u8 NumCols;   /* Number of cols allocated to the partition */
	u64 BaseAddr;
} XAie_DevicePartInfo;

/*
 * This enum contains all the Stream Switch Port types. These enums are used to
 * access the base address of stream switch configuration registers.
 */
typedef enum class StrmSwPortType{
	_512B_PORT_START,
	CORE = _512B_PORT_START,
	DMA,
	CTRL,
	FIFO,
	SOUTH,
	WEST,
	NORTH,
	EAST,
	TRACE,
	UCTRLR,
	NORTH_CTRL, /* Introduced from AIE4 */
	SOUTH_CTRL, /* Introduced from AIE4 */
	SWITCH_32b, /* Introduced from AIE4 */
	DMA_CTRL, /* Introduced from AIE4 */
	DMA_Trace, /* Introduced from AIE4 */
	PL, /* Introduced from AIE4 */
	_512B_PORT_END = PL,

	/* AIE4 specific 32b switch PortTypes */
	_32B_PORT_START,
	_32B_CTRL = _32B_PORT_START,
	_32B_TRACE,
	_32B_SOUTH,
	_32B_WEST,
	_32B_NORTH,
	_32B_EAST,
	_32B_SWITCH_512b,
	_32B_UCTRLR,
	_32B_PL,
	_32B_PORT_END = _32B_PL,

	SS_PORT_TYPE_MAX,
} StrmSwPortType;

/* Data structures to capture data shape for dmas */
typedef struct XAie_AieMlDmaDimDesc{
	u32 StepSize;
	u16 Wrap;
} XAie_AieMlDmaDimDesc;

typedef struct XAie_AieDmaDimDesc {
	u32 Offset;
	u32 Incr;
	u16 Wrap;
} XAie_AieDmaDimDesc;

typedef union XAie_DmaDimDesc{
	XAie_AieDmaDimDesc AieDimDesc;
	XAie_AieMlDmaDimDesc AieMlDimDesc;
} XAie_DmaDimDesc;

typedef struct XAie_DmaTensor {
	u8 NumDim;
	XAie_DmaDimDesc *Dim;
} XAie_DmaTensor;

typedef struct XAie_LockDesc {
	u16 LockAcqId;
	u16 LockRelId;
	u8 LockAcqEn;
	s8 LockAcqVal;
	u8 LockAcqValEn;
	u8 LockRelEn;
	s8 LockRelVal;
	u8 LockRelValEn;
} XAie_LockDesc;

typedef struct XAie_PktDesc {
	u8 PktId;
	u8 PktType;
	u8 PktEn;
} XAie_PktDesc;

typedef struct XAie_AddrDesc {
	u64 Address;
	u32 Length;
} XAie_AddrDesc;

typedef struct XAie_BdEnDesc {
	u8 ValidBd;
	u8 NxtBd;
	u8 UseNxtBd;
	u8 OutofOrderBdId;
} XAie_BdEnDesc;

typedef struct XAie_DmaAxiDesc {
	u8 SMID;
	u8 BurstLen;
	u8 AxQos;
	u8 SecureAccess;
	u8 AxCache;
	u8 AxUser;
	u8 IOCoherence;
	u8 KeyIdx;
	u8 DataReuse;
} XAie_DmaAxiDesc;

typedef struct XAie_AieMultiDimDesc {
	u8 X_Incr;
	u8 X_Wrap;
	u16 X_Offset;
	u8 Y_Incr;
	u8 Y_Wrap;
	u16 Y_Offset;
	u8 IntrleaveBufSelect;
	u16 CurrPtr;
	u8 IntrleaveCount;
	u8 EnInterleaved;
} XAie_AieMultiDimDesc;

typedef struct XAie_AieMlDimDesc {
	u16 Wrap;
	u32 StepSize;
} XAie_AieMlDimDesc;

typedef struct XAie_AieMlMultiDimDesc {
	u8 IterCurr;
	XAie_AieMlDimDesc IterDesc;
	XAie_AieMlDimDesc DimDesc[4U];	/* Max 4D addressing supported */
} XAie_AieMlMultiDimDesc;

typedef union XAie_MultiDimDesc {
	XAie_AieMultiDimDesc AieMultiDimDesc;
	XAie_AieMlMultiDimDesc AieMlMultiDimDesc;
} XAie_MultiDimDesc;

typedef struct XAie_PadDesc {
	u16 Before;
	u16 After;
} XAie_PadDesc;

typedef struct XAie_DmaPadTensor {
	u8 NumDim;
	XAie_PadDesc *PadDesc;
} XAie_DmaPadTensor;

typedef struct XAie_DmaChannelDesc {
	u8 EnOutofOrderId;
	u8 EnTokenIssue;
	u8 EnCompression;
	u8 FoTMode;
	u8 TileType;
	u8 IsReady;
	u32 ControllerId;
	const XAie_DmaMod *DmaMod;
} XAie_DmaChannelDesc;

typedef struct XAie_DmaDesc {
	XAie_PktDesc PktDesc;
	XAie_LockDesc LockDesc;
	XAie_AddrDesc AddrDesc;
	XAie_DmaAxiDesc AxiDesc;
	XAie_BdEnDesc BdEnDesc;
	XAie_LockDesc LockDesc_2;
	XAie_AddrDesc AddrDesc_2;
	XAie_MultiDimDesc MultiDimDesc;
	XAie_PadDesc PadDesc[3U];
	const XAie_DmaMod *DmaMod;
	const XAie_LockMod *LockMod;
	XAie_MemInst *MemInst;
	u8 EnDoubleBuff;
	u8 FifoMode;
	u8 EnCompression;
	u8 EnOutofOrderBdId;
	u8 TlastSuppress;
	u8 TileType;
	u8 IsReady;
	u8 DevGen;	/* Added from and for AIE4 */
	u8 AppMode;	/* Added from and for AIE4 */
} XAie_DmaDesc;

typedef struct XAie_DmaQueueDesc {
	u32 RepeatCount;
	u16 StartBd;
	u8 EnTokenIssue;
	u8 OutOfOrder;
} XAie_DmaQueueDesc;

/*
 * This enum contains the dma channel reset for aie dmas.
 */
typedef enum class XAie_DmaChReset {
	DMA_CHANNEL_UNRESET,
	DMA_CHANNEL_RESET
} XAie_DmaChReset;
/*
 * This enum contains the dma direction for aie dmas.
 */
typedef enum class XAie_DmaDirection{
	DMA_S2MM,
	DMA_MM2S,
	DMA_MM2S_CTRL, /* Added from AIE4 Shim DMA */
	DMA_S2MM_TRACE,
	DMA_MAX
} XAie_DmaDirection;

/*
 * This enum contains the FoT mode for aie Dma Channel.
 */
typedef enum class XAie_DmaChannelFoTMode{
	DMA_FoT_DISABLED,
	DMA_FoT_NO_COUNTS,
	DMA_FoT_COUNTS_WITH_TASK_TOKENS,
	DMA_FoT_COUNTS_FROM_MM_REG,
} XAie_DmaChannelFoTMode;

/*
 * This enum contains the positions for dma zero padding.
 */
typedef enum class XAie_DmaZeroPaddingPos{
	DMA_ZERO_PADDING_BEFORE,
	DMA_ZERO_PADDING_AFTER,
} XAie_DmaZeroPaddingPos;

/*
 * This enum contains the application index to A & B mapping.
 */
typedef enum class XAie_AppIndex {
        APPLICATION_A,
        APPLICATION_B,
} XAie_AppIndex;

/*
 * This enum captures all the error codes from the driver
 */
typedef enum class AieRC{
	XAIE_OK,
	XAIE_ERR,
	XAIE_INVALID_DEVICE,
	XAIE_INVALID_RANGE,
	XAIE_INVALID_ARGS,
	XAIE_INVALID_TILE,
	XAIE_ERR_STREAM_PORT,
	XAIE_INVALID_DMA_TILE,
	XAIE_INVALID_BD_NUM,
	XAIE_ERR_OUTOFBOUND,
	XAIE_INVALID_DATA_MEM_ADDR,
	XAIE_INVALID_ELF,
	XAIE_CORE_STATUS_TIMEOUT,
	XAIE_INVALID_CHANNEL_NUM,
	XAIE_INVALID_LOCK,
	XAIE_INVALID_DMA_DIRECTION,
	XAIE_INVALID_PLIF_WIDTH,
	XAIE_INVALID_LOCK_ID,
	XAIE_INVALID_LOCK_VALUE,
	XAIE_LOCK_RESULT_FAILED,
	XAIE_INVALID_DMA_DESC,
	XAIE_INVALID_ADDRESS,
	XAIE_FEATURE_NOT_SUPPORTED,
	XAIE_INVALID_BURST_LENGTH,
	XAIE_INVALID_BACKEND,
	XAIE_INSUFFICIENT_BUFFER_SIZE,
	XAIE_INVALID_API_POINTER,
	XAIE_NOT_SUPPORTED,
	XAIE_INVALID_APP_MODE,
	XAIE_AXIMM_PENDING_TRANSACTION_TIMEOUT,
	XAIE_ERR_MAX
} AieRC;

/*
 * This enum is to identify different hardware modules within a tile type.
 * An AIE tile can have memory or core module. A PL or Shim tile will have
 * Pl module. A mem tile will have memory module. Any hardware module
 * addition in future generations of AIE needs to be appended to this enum.
 */
typedef enum class XAie_ModuleType{
	XAIE_MEM_MOD,
	XAIE_CORE_MOD,
	XAIE_PL_MOD,
} XAie_ModuleType;

/* This enum contains reset input values. */
typedef enum class XAie_Reset {
	XAIE_RESETDISABLE,
	XAIE_RESETENABLE,
} XAie_Reset;

/* This enum is used to identify the different types of memories in uc module */
typedef enum class XAie_UcMemType {
        XAIE_PROGRAM_MEMORY,
        XAIE_PRIVATE_DATA_MEMORY,
        XAIE_MODULE_DATA_MEMORY,
} XAie_UcMemType;

/* Data structure to capture lock id and value */
typedef struct XAie_Lock {
	u16 LockId; /* From AIE4 ID value is of 10bit size, so increased from u8 to u16 */
	s8 LockVal;
} XAie_Lock;

/* Data structure to capture packet id and packet type */
typedef struct XAie_Packet {
	u8 PktId;
	u8 PktType;
} XAie_Packet;

/* Enum to capture event switches */
typedef enum class XAie_BroadcastSw {
	XAIE_EVENT_SWITCH_A,
	XAIE_EVENT_SWITCH_B,
} XAie_BroadcastSw;

/*
 * Data structure to capture error information.
 * Loc: Location of tile reporting error event.
 * Module: Module type of tile reporting error event.
 * EventId: Event ID of tile reporting error event.
 */
typedef struct XAie_ErrorPayload {
	XAie_LocType Loc;
	XAie_ModuleType Module;
	u8 EventId;
} XAie_ErrorPayload;

/*
 * Data structure to provide Error information to Host
 * ErrorCount: Total Number of valid payloads returned.
 * ReturnCode: Return success or Insufficient Buffer error to host.
 * Payload: Array of Error Payload Structure.
 */
typedef struct XAie_ErrorInfo {
	u32 ErrorCount;
	u32 ReturnCode;
	XAie_ErrorPayload *Payload;
} XAie_ErrorInfo;

/*
 * Data structure to capture metadata required for backtracking errors.
 * ErrInfo: Pointer to buffer capturing array of error payloads and .
 * ArraySize: Array size of payload buffer. Value corresponds to total number of
 *	      XAie_ErrorPayload structs.
 * Cols: Range of columns to be backtracked.
 */
typedef struct XAie_ErrorMetaData {
	XAie_ErrorInfo *ErrInfo;
	u32 ArraySize;
	XAie_Range Cols;
} XAie_ErrorMetaData;

/*
 * Typedef for enum of AIE backend attribute type
 */
typedef enum class XAie_BackendAttrType {
	XAIE_BACKEND_ATTR_CORE_PROG_MEM_SIZE,
} XAie_BackendAttrType;

/*
 * This typedef contains members necessary to store the tile location and
 * %kernel utilization captured
 */
typedef struct XAie_Occupancy {
	XAie_LocType Loc;
	uint8_t PerfCnt[2];
	float KernelUtil;
} XAie_Occupancy;

/*
 * This typedef contains attributes necessary to capture kernel utilization in
 * core tiles.
 */
typedef struct XAie_PerfInst {
	XAie_Range *Range;
	uint32_t TimeInterval_ms;
	XAie_Occupancy *Util;
	uint32_t UtilSize;
} XAie_PerfInst;

/*
 * This typedef captures all the memory interleaving modes supported by the driver
 */
typedef enum class XAie_MemInterleavingMode {
	XAIE_MEM_INTERLEAVING_MODE_DISABLE, /* Disable memory interleaving */
	XAIE_MEM_INTERLEAVING_MODE_ENABLE = (1 << _XAIE_MEMINTERLEAVE_MODE_SHIFT),   /* Enable memory interleaving */
	XAIE_MEM_INTERLEAVING_MODE_SKEW = (2 << _XAIE_MEMINTERLEAVE_MODE_SHIFT),   /* Enable memory interleaving skew mode */
	XAIE_MEM_INTERLEAVING_MODE_INVALID = (3 << _XAIE_MEMINTERLEAVE_MODE_SHIFT), /* Invalid Mode */
} XAie_MemInterleavingMode;

/**************************** Function prototypes ***************************/
/*****************************************************************************/
/*
*
* This API returns a structure of type XAie_Loc which captures the lock id and
* lock value.
*
* @param	Id: Lock id
* @param	Value: Lock value.
*
* @return	Lock: Lock strcuture initialized with Id and Value.
*
* @note		None.
*
******************************************************************************/
static inline XAie_Lock XAie_LockInit(u16 Id, s8 Value)
{
	XAie_Lock Lock = {Id, Value};
	return Lock;
}

/*****************************************************************************/
/*
*
* This API returns a structure of type XAie_Packet which captures the packet id
* and packet type. XAie_Packet can be used to configure the packet properties
* of aie dmas and stream switches. Packet ID determins the route between ports,
* and packet type is used to differentiate packets from the same source.
*
* @param	PktId: Packet id(5 bits)
* @param	PktType: Packet type(3 bits)
*
* @return	Pkt: Packet strcuture initialized with Id and Type.
*
* @note		None.
*
******************************************************************************/
static inline XAie_Packet XAie_PacketInit(u8 PktId, u8 PktType)
{
	XAie_Packet Pkt = {PktId, PktType};
	return Pkt;
}

/*****************************************************************************/
/*
*
* This API returns a structure of type XAie_LocType given a col and row index of
* AIE. All APIs use this structure to identify the coordinates of AIE tiles.
*
* @param	col: column index
* @param	row: row index
*
* @return	Loc: strcuture containing row and col index.
*
* @note		None.
*
******************************************************************************/
static inline XAie_LocType XAie_TileLoc(u8 col, u8 row)
{
	XAie_LocType Loc = { row, col };
	return Loc;
}

/*****************************************************************************/
/**
*
* This API setups the AI engine partition property in AI engine config
*
* @param	Config: XAie_Config structure.
* @param	Nid: AI enigne partition node ID
* @param	Uid: AI enigne partition image UID
* @param	Handle: AI engine partition handle, in some OS such as Linux
*			the AI engine partition is presented as file descriptor.
*			In case of Xilinx runtime stack, the Xilinx runtime
*			module has requested the AI engine partition which will
*			have the handle can be passed to the userspace
*			application.
* @param	CntrFlag: AI engine partition control flag. E.g., it can be used
*			to indicate if the partition needs to cleanup when
*			application terminates.
*
* @return	None.
*
* @note		This function is to set the partition system design property to
*		the AI engine config. It needs to be called before intialize
*		AI engine partition.
*
*******************************************************************************/
static inline void XAie_SetupConfigPartProp(XAie_Config *ConfigPtr, u32 Nid,
		u32 Uid, u64 Handle, u32 CntrFlag)
{
	ConfigPtr->PartProp.Nid = Nid;
	ConfigPtr->PartProp.Uid = Uid;
	ConfigPtr->PartProp.Handle = Handle;
	ConfigPtr->PartProp.CntrFlag = CntrFlag;
}

/*****************************************************************************/
/**
*
* This API setups the AI engine Shim North and South tile rows in AI engine
*
* @param	Config: XAie_Config structure.
* @param	NumShimTileRowsNorth: Number of shim tile rows in the north direction.
* @param	NumShimTileRowsSouth: Number of shim tile rows in the south direction.
* @return	None.
*
* @note		This function is to setup the shim tile rows in the AI engine config.
*			It needs to be called before intialize AI engine partition.
*
*******************************************************************************/
static inline void XAie_SetupConfigNorthSouthShimTileRows(XAie_Config *ConfigPtr, 
	u8 NumShimTileRowsNorth, u8 NumShimTileRowsSouth)
{
	ConfigPtr->ShimTileNumRowsNorth = NumShimTileRowsNorth;
	ConfigPtr->ShimTileNumRowsSouth = NumShimTileRowsSouth;
}

/*****************************************************************************/
/**
*
* Macro to setup the configurate pointer data structure with hardware specific
* details.
*
* @param	Config: XAie_Config structure.
* @param	_AieGen: Aie device generation.
* @param	_BaseAddr: Base Address of the device.
* @param	_ColShift: Bit shift value for column.
* @param	_RowShift: Bit shift value for row.
* @param	_NumCols: Number of cols in the hardware.
* @param	_NumRows: Number of rows in the hardware.
* @param	_ShimRowNum: Row number of the shimrow.
* @param	_MemTileRowStart: Starting row number of the mem tile.
* @param	_MemTileNumRows: Number of mem tile rows.
* @param	_AieTileRowStart: Starting row number of the mem tile.
* @param	_AieTileNumRows: Number of mem tile rows.
*
* @return	None.
*
* @note		The macro declares it XAie_Config as a stack variable.
*
*******************************************************************************/
#define _XAie_SetupConfig(Config, _AieGen, _BaseAddr, _ColShift, _RowShift,\
		_NumCols, _NumRows, _ShimRowNum, _MemTileRowStart, _MemTileNumRows,\
		_AieTileRowStart, _AieTileNumRows) \
		XAie_Config Codegen_Config = {\
			.AieGen = _AieGen,\
			.BaseAddr = _BaseAddr,\
			.ColShift = _ColShift,\
			.RowShift = _RowShift,\
			.NumRows = _NumRows,\
			.NumCols = _NumCols,\
			.ShimRowNum = _ShimRowNum,\
			.MemTileRowStart = _MemTileRowStart,\
			.MemTileNumRows = _MemTileNumRows,\
			.AieTileRowStart = _AieTileRowStart,\
			.AieTileNumRows = _AieTileNumRows,\
			.PartProp = {0}, \
			.ShimTileNumRowsNorth = 0,\
			.ShimTileNumRowsSouth = 1,\
		}\

/*****************************************************************************/
/**
*
* Macro to declare device instance.
*
* @param	Inst: Name of the Device Instance variable.
* @param	ConfigPtr: Pointer to the XAie_Config structure containing the
*			   hardware details..
*
* @return	None.
*
* @note		The instance of a device must be always declared using this
*		macro. In future, the same macro will be expanded to allocate
*		more memory from the user application for resource management.
*
*******************************************************************************/
#define _XAie_InstDeclare(Inst, ConfigPtr) XAie_DevInst Inst = { 0 }

/*****************************************************************************/
/**
*
* Macro enabling the caller to get the base address of a given column in the
* array.
*
* @param	BaseAddr: The base address of the AIE array.
* @param	StartCol: The start column in the array.
* @param	ColShift: Number of bits to shift the column.
*
* @return	None.
*
* @note	The column base address will be the absolute base address plus
*		the offset of the column.
*
*******************************************************************************/

#define _XAie_GetCol_BaseAddr(BaseAddr, StartCol, ColShift)              \
	(BaseAddr + (StartCol << ColShift))


//=============================xaiengine/xaie_clock.h

/*****************************************************************************/
/**
* @file xaie_clock.h
*
* Header file for timer implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date        Changes
* ----- ------   --------    --------------------------------------------------
* 1.0   Dishita  06/26/2020  Initial creation
* </pre>
*
******************************************************************************/


/************************** Enum *********************************************/

/************************** Function Prototypes  *****************************/
//=============================xaiengine/xaie_events.h

/*****************************************************************************/
/**
* @file xaie_events.h
*
* Header file for AIE events module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- ------   -------- -----------------------------------------------------
* 1.0   Dishita  11/21/2019  Initial creation
* 1.1   Nishad   06/02/2020  Move event files under events directory
* 1.2   Nishad   06/02/2020  Remove unused included header files
* 1.3   Nishad   07/06/2020  Add APIs to configure stream switch port event
*			     selection, event generation, and combo event
*			     registers.
* 1.4   Nishad   07/12/2020  Add APIs to configure event broadcast, PC event,
*			     and group event registers.
* 1.5   Nishad  07/14/2020  Add APIs to reset individual stream switch port
*			    event selection ID and combo event.
* 1.6   Nishad   07/23/2020  Add API to block brodcast signals using bitmap.
* </pre>
*
******************************************************************************/


/***************************** Macro Definitions *****************************/
constexpr u32 _XAIE_COMBO_PER_MOD = 4U;	//for Legacy Devices (Pre-AIE4)
constexpr u32 _XAIE4_COMBO_PER_MOD = 8U;	//for AIE4 Device

constexpr u16 _XAIE_EVENT_INVALID = 0xFFFFU;

#define _XAIE_EVENT_GENERATE_ENUM(ENUM)		XAIE_EVENT_##ENUM,
#define _XAIE_EVENT_GENERATE_ENUM_STRING(STRING)	[XAIE_EVENT_##STRING]	=	#STRING,

	/* All core module events of aie tile */
#define _XAIE_EVENT_GENERATE_CORE_MOD(TARGET)				\
	_XAIE_EVENT_GENERATE_##TARGET(NONE_CORE)				\
	_XAIE_EVENT_GENERATE_##TARGET(TRUE_CORE)				\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_0_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(TIMER_SYNC_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(TIMER_VALUE_REACHED_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_0_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_1_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_2_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_3_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_4_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_5_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_6_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_7_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_8_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_9_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_10_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_11_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_0_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_1_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_2_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_3_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_4_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_5_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_6_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_7_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_PC_EVENT_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PC_0_CORE)				\
	_XAIE_EVENT_GENERATE_##TARGET(PC_1_CORE)				\
	_XAIE_EVENT_GENERATE_##TARGET(PC_2_CORE)				\
	_XAIE_EVENT_GENERATE_##TARGET(PC_3_CORE)				\
	_XAIE_EVENT_GENERATE_##TARGET(PC_4_CORE)				\
	_XAIE_EVENT_GENERATE_##TARGET(PC_5_CORE)				\
	_XAIE_EVENT_GENERATE_##TARGET(PC_6_CORE)				\
	_XAIE_EVENT_GENERATE_##TARGET(PC_7_CORE)				\
	_XAIE_EVENT_GENERATE_##TARGET(PC_8_CORE)				\
	_XAIE_EVENT_GENERATE_##TARGET(PC_9_CORE)				\
	_XAIE_EVENT_GENERATE_##TARGET(PC_RANGE_0_1_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PC_RANGE_2_3_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PC_RANGE_4_5_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PC_RANGE_6_7_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PC_RANGE_8_9_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_CORE_STALL_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(MEMORY_STALL_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(STREAM_STALL_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(CASCADE_STALL_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_STALL_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(DEBUG_HALTED_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(ACTIVE_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(DISABLED_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(ECC_ERROR_STALL_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(ECC_SCRUBBING_STALL_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_CORE_PROGRAM_FLOW_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_EVENT_0_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_EVENT_1_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_EVENT_2_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_EVENT_3_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_EVENT_4_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_EVENT_5_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_CALL_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_RETURN_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_VECTOR_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_LOAD_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_LOAD_B_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_STORE_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_STREAM_GET_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_STREAM_GET_1_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_STREAM_PUT_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_CASCADE_GET_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_CASCADE_PUT_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_LOCK_ACQUIRE_REQ_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_LOCK_RELEASE_REQ_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_ERRORS_0_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_ERRORS_1_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_ERRORS_2_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(SRS_SATURATE_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(UPS_SATURATE_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(FP_OVERFLOW_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(FP_UNDERFLOW_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(FP_INVALID_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(FP_DIV_BY_ZERO_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(TLAST_IN_WSS_WORDS_0_2_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(PM_REG_ACCESS_FAILURE_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(STREAM_PKT_PARITY_ERROR_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(CONTROL_PKT_ERROR_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(AXI_MM_SLAVE_ERROR_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_DECOMPRSN_ERROR_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(DM_ADDRESS_OUT_OF_RANGE_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(PM_ECC_ERROR_SCRUB_CORRECTED_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(PM_ECC_ERROR_SCRUB_2BIT_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(PM_ECC_ERROR_1BIT_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PM_ECC_ERROR_2BIT_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PM_ADDRESS_OUT_OF_RANGE_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(DM_ACCESS_TO_UNAVAILABLE_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_ACCESS_TO_UNAVAILABLE_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_EVENT_2)			\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_EVENT_3)			\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_STREAM_SWITCH_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_0_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_0_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_0_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_0_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_1_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_1_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_1_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_1_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_2_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_2_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_2_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_2_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_3_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_3_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_3_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_3_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_4_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_4_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_4_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_4_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_5_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_5_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_5_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_5_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_6_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_6_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_6_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_6_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_7_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_7_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_7_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_7_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_BROADCAST_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_0_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_1_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_2_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_3_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_4_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_5_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_6_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_7_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_8_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_9_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_10_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_11_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_12_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_13_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_14_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_15_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_USER_EVENT_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_0_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_1_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_2_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_3_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_4_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_5_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_6_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_7_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_0_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_1_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_2_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_3_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_4_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_5_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_6_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_7_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(FP_HUGE_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(INT_FP_0_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(FP_INF_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_WARNING_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_ERROR_CORE)			\
	_XAIE_EVENT_GENERATE_##TARGET(DECOMPRESSION_UNDERFLOW_CORE)	\
	_XAIE_EVENT_GENERATE_##TARGET(STREAM_SWITCH_PORT_PARITY_ERROR_CORE) \
	_XAIE_EVENT_GENERATE_##TARGET(PROCESSOR_BUS_ERROR_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_MATRIX_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_MOVE_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(INSTR_ALU_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(FATAL_ERROR_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(SPARSITY_OVERFLOW_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DATA_ERROR_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DM_ECC_ERROR_SCRUB_CORRECTED_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DM_ECC_ERROR_SCRUB_2BIT_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DM_ECC_ERROR_1BIT_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DM_ECC_ERROR_2BIT_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DM_PARITY_ERROR_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_ERROR_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_ERROR_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_TASK_TOKEN_STALL_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_WATCHPOINT_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(WATCHPOINT_0_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(WATCHPOINT_1_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(WATCHPOINT_2_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_DMA_ACTIVITY_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_START_BD_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_START_BD_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_START_BD_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_START_TASK_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_START_TASK_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_START_TASK_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_FINISHED_BD_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_FINISHED_BD_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_FINISHED_BD_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_FINISHED_TASK_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_FINISHED_TASK_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_FINISHED_TASK_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_STALLED_LOCK_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_STALLED_LOCK_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_STALLED_LOCK_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_STREAM_STARVATION_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_STREAM_STARVATION_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_STREAM_BACKPRESSURE_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_MEMORY_BACKPRESSURE_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_MEMORY_BACKPRESSURE_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_MEMORY_STARVATION_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_RUNNING_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_RUNNING_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_RUNNING_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_LOCK_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL0_ACQ_EQ_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL0_ACQ_GE_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL0_REL_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL0_EQUAL_TO_VALUE_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL1_ACQ_EQ_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL1_ACQ_GE_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL1_REL_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL1_EQUAL_TO_VALUE_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL2_ACQ_EQ_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL2_ACQ_GE_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL2_REL_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL2_EQUAL_TO_VALUE_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL3_ACQ_EQ_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL3_ACQ_GE_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL3_REL_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL3_EQUAL_TO_VALUE_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL4_ACQ_EQ_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL4_ACQ_GE_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL4_REL_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL4_EQUAL_TO_VALUE_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL5_ACQ_EQ_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL5_ACQ_GE_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL5_REL_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL5_EQUAL_TO_VALUE_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_MEMORY_CONFLICT_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_0_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_1_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_2_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_3_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_4_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_5_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_6_CORE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_7_CORE)

	/* All memory module events of aie tile */
#define _XAIE_EVENT_GENERATE_MEM_MOD(TARGET)				\
	_XAIE_EVENT_GENERATE_##TARGET(NONE_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(TRUE_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_0_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(TIMER_SYNC_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(TIMER_VALUE_REACHED_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_0_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_1_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_0_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_1_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_2_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_3_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_WATCHPOINT_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(WATCHPOINT_0_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(WATCHPOINT_1_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_DMA_ACTIVITY_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_START_BD_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_START_BD_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_START_BD_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_START_BD_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_FINISHED_BD_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_FINISHED_BD_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_FINISHED_BD_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_FINISHED_BD_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_GO_TO_IDLE_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_GO_TO_IDLE_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_GO_TO_IDLE_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_GO_TO_IDLE_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_STALLED_LOCK_ACQUIRE_MEM)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_STALLED_LOCK_ACQUIRE_MEM)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_STALLED_LOCK_ACQUIRE_MEM)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_STALLED_LOCK_ACQUIRE_MEM)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_MEMORY_CONFLICT_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_MEMORY_CONFLICT_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_MEMORY_CONFLICT_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_MEMORY_CONFLICT_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_LOCK_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_0_ACQ_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_0_REL_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_1_ACQ_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_1_REL_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_2_ACQ_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_2_REL_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_3_ACQ_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_3_REL_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_4_ACQ_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_4_REL_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_5_ACQ_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_5_REL_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_6_ACQ_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_6_REL_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_7_ACQ_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_7_REL_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_8_ACQ_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_8_REL_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_9_ACQ_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_9_REL_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_10_ACQ_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_10_REL_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_11_ACQ_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_11_REL_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_12_ACQ_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_12_REL_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_13_ACQ_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_13_REL_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_14_ACQ_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_14_REL_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_15_ACQ_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_15_REL_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_MEMORY_CONFLICT_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_0_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_1_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_2_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_3_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_4_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_5_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_6_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_7_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_ERRORS_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(DM_ECC_ERROR_SCRUB_CORRECTED_MEM)			\
	_XAIE_EVENT_GENERATE_##TARGET(DM_ECC_ERROR_SCRUB_2BIT_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DM_ECC_ERROR_1BIT_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(DM_ECC_ERROR_2BIT_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(DM_PARITY_ERROR_BANK_2_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DM_PARITY_ERROR_BANK_3_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DM_PARITY_ERROR_BANK_4_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DM_PARITY_ERROR_BANK_5_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DM_PARITY_ERROR_BANK_6_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DM_PARITY_ERROR_BANK_7_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_ERROR_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_ERROR_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_ERROR_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_ERROR_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_BROADCAST_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_0_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_1_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_2_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_3_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_4_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_5_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_6_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_7_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_8_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_9_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_10_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_11_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_12_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_13_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_14_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_15_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_USER_EVENT_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_0_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_1_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_2_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_3_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_0_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_1_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_START_TASK_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_START_TASK_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_START_TASK_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_START_TASK_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_FINISHED_TASK_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_FINISHED_TASK_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_FINISHED_TASK_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_FINISHED_TASK_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_STALLED_LOCK_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_STALLED_LOCK_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_STALLED_LOCK_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_STALLED_LOCK_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_STREAM_STARVATION_MEM)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_STREAM_STARVATION_MEM)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_STREAM_BACKPRESSURE_MEM)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_STREAM_BACKPRESSURE_MEM)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_MEMORY_BACKPRESSURE_MEM)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_MEMORY_BACKPRESSURE_MEM)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_MEMORY_STARVATION_MEM)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_MEMORY_STARVATION_MEM)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL0_ACQ_EQ_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL0_ACQ_GE_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL0_EQUAL_TO_VALUE_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL1_ACQ_EQ_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL1_ACQ_GE_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL1_EQUAL_TO_VALUE_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL2_ACQ_EQ_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL2_ACQ_GE_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL2_EQUAL_TO_VALUE_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL3_ACQ_EQ_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL3_ACQ_GE_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL3_EQUAL_TO_VALUE_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL4_ACQ_EQ_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL4_ACQ_GE_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL4_EQUAL_TO_VALUE_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL5_ACQ_EQ_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL5_ACQ_GE_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL5_EQUAL_TO_VALUE_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL6_ACQ_EQ_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL6_ACQ_GE_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL6_EQUAL_TO_VALUE_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL7_ACQ_EQ_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL7_ACQ_GE_MEM)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL7_EQUAL_TO_VALUE_MEM)				\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_ERROR_MEM)						\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_TASK_TOKEN_STALL_MEM)				\

	/* All PL/Noc module events */
#define _XAIE_EVENT_GENERATE_PL_MOD(TARGET)	\
	_XAIE_EVENT_GENERATE_##TARGET(NONE_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(TRUE_PL)							\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_0_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(TIMER_SYNC_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(TIMER_VALUE_REACHED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_0_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_1_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_2_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_3_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_4_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_5_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_6_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_7_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_8_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_9_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_10_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT_11_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_0_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_1_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_2_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_3_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_4_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_5_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_6_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_7_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_DMA_ACTIVITY_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_START_BD_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_START_BD_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_START_BD_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_START_BD_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_2_START_BD_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_3_START_BD_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_FINISHED_BD_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_FINISHED_BD_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_FINISHED_BD_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_FINISHED_BD_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_2_FINISHED_BD_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_3_FINISHED_BD_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_GO_TO_IDLE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_GO_TO_IDLE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_GO_TO_IDLE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_GO_TO_IDLE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_STALLED_LOCK_ACQUIRE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_STALLED_LOCK_ACQUIRE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_STALLED_LOCK_ACQUIRE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_STALLED_LOCK_ACQUIRE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_LOCK_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_0_ACQUIRED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_0_RELEASED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_1_ACQUIRED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_1_RELEASED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_2_ACQUIRED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_2_RELEASED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_3_ACQUIRED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_3_RELEASED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_4_ACQUIRED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_4_RELEASED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_5_ACQUIRED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_5_RELEASED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_6_ACQUIRED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_6_RELEASED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_7_ACQUIRED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_7_RELEASED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_8_ACQUIRED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_8_RELEASED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_9_ACQUIRED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_9_RELEASED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_10_ACQUIRED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_10_RELEASED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_11_ACQUIRED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_11_RELEASED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_12_ACQUIRED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_12_RELEASED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_13_ACQUIRED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_13_RELEASED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_14_ACQUIRED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_14_RELEASED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_15_ACQUIRED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_15_RELEASED_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_ERRORS_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(AXI_MM_SLAVE_TILE_ERROR_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(CONTROL_PKT_ERROR_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(AXI_MM_DECODE_NSU_ERROR_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(AXI_MM_SLAVE_NSU_ERROR_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(AXI_MM_UNSUPPORTED_TRAFFIC_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(AXI_MM_UNSECURE_ACCESS_IN_SECURE_MODE_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(AXI_MM_BYTE_STROBE_ERROR_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_ERROR_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_ERROR_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_ERROR_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_ERROR_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_STREAM_SWITCH_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_0_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_0_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_0_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_0_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_1_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_1_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_1_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_1_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_2_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_2_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_2_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_2_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_3_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_3_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_3_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_3_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_4_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_4_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_4_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_4_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_5_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_5_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_5_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_5_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_6_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_6_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_6_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_6_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_7_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_7_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_7_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_7_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_BROADCAST_A_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_A_0_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_A_1_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_A_2_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_A_3_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_A_4_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_A_5_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_A_6_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_A_7_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_A_8_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_A_9_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_A_10_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_A_11_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_A_12_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_A_13_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_A_14_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_A_15_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_USER_EVENT_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_0_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_1_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_2_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_3_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_4_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_5_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_6_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_7_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_0_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_1_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_2_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_3_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_4_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_5_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_6_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_7_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_START_TASK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_START_TASK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_START_TASK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_START_TASK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_2_START_TASK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_3_START_TASK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_FINISHED_TASK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_FINISHED_TASK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_FINISHED_TASK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_FINISHED_TASK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_2_FINISHED_TASK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_3_FINISHED_TASK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_STALLED_LOCK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_STALLED_LOCK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_STALLED_LOCK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_STALLED_LOCK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_2_STALLED_LOCK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_3_STALLED_LOCK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_STREAM_STARVATION_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_STREAM_STARVATION_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_STREAM_BACKPRESSURE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_STREAM_BACKPRESSURE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_2_STREAM_BACKPRESSURE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_3_STREAM_BACKPRESSURE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_MEMORY_BACKPRESSURE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_MEMORY_BACKPRESSURE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_MEMORY_STARVATION_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_MEMORY_STARVATION_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_2_MEMORY_STARVATION_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_3_MEMORY_STARVATION_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_0_RUNNING_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_1_RUNNING_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_0_RUNNING_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_1_RUNNING_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_2_RUNNING_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_3_RUNNING_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_0_ACQ_EQ_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_0_ACQ_GE_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_0_EQUAL_TO_VALUE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_1_ACQ_EQ_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_1_ACQ_GE_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_1_EQUAL_TO_VALUE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_2_ACQ_EQ_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_2_ACQ_GE_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_2_EQUAL_TO_VALUE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_3_ACQ_EQ_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_3_ACQ_GE_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_3_EQUAL_TO_VALUE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_4_ACQ_EQ_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_4_ACQ_GE_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_4_EQUAL_TO_VALUE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_5_ACQ_EQ_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_5_ACQ_GE_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_5_EQUAL_TO_VALUE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(STREAM_SWITCH_PARITY_ERROR_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_ERROR_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_ERROR_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_ERROR_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_TASK_TOKEN_STALL_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(AXI_MM_SUBORDINATE_TILE_ERROR_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(NSU_ERROR_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_ERROR_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_HW_ERROR_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_TRACE_S2MM_START_BD_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_TRACE_S2MM_START_TASK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_TRACE_S2MM_FINISHED_BD_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_TRACE_S2MM_FINISHED_TASK_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_TRACE_S2MM_STREAM_STARVATION_PL)					\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_TRACE_S2MM_MEMORY_BACKPRESSURE_PL)						\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_TRACE_S2MM_RUNNING_PL)					\
									\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_GROUP_DMA_ACTIVITY_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_S2MM_0_START_TASK_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_S2MM_1_START_TASK_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_MM2S_0_START_TASK_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_MM2S_1_START_TASK_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_S2MM_0_FINISHED_BD_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_S2MM_1_FINISHED_BD_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_MM2S_0_FINISHED_BD_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_MM2S_1_FINISHED_BD_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_S2MM_0_FINISHED_TASK_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_S2MM_1_FINISHED_TASK_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_MM2S_0_FINISHED_TASK_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_MM2S_1_FINISHED_TASK_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_S2MM_0_STALLED_LOCK_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_S2MM_1_STALLED_LOCK_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_MM2S_0_STALLED_LOCK_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_MM2S_1_STALLED_LOCK_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_S2MM_0_STREAM_STARVATION_PL)	\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_S2MM_1_STREAM_STARVATION_PL)	\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_MM2S_0_STREAM_BACKPRESSURE_PL)	\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_MM2S_1_STREAM_BACKPRESSURE_PL)	\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_S2MM_0_MEMORY_BACKPRESSURE_PL)	\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_S2MM_1_MEMORY_BACKPRESSURE_PL)	\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_MM2S_0_MEMORY_STARVATION_PL)	\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_MM2S_1_MEMORY_STARVATION_PL)	\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_GROUP_DMA_ACTIVITY_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_S2MM_0_START_TASK_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_S2MM_1_START_TASK_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_MM2S_0_START_TASK_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_MM2S_1_START_TASK_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_S2MM_0_FINISHED_BD_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_S2MM_1_FINISHED_BD_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_MM2S_0_FINISHED_BD_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_MM2S_1_FINISHED_BD_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_S2MM_0_FINISHED_TASK_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_S2MM_1_FINISHED_TASK_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_MM2S_0_FINISHED_TASK_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_MM2S_1_FINISHED_TASK_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_S2MM_0_STALLED_LOCK_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_S2MM_1_STALLED_LOCK_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_MM2S_0_STALLED_LOCK_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_MM2S_1_STALLED_LOCK_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_S2MM_0_STREAM_STARVATION_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_S2MM_1_STREAM_STARVATION_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_MM2S_0_STREAM_BACKPRESSURE_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_MM2S_1_STREAM_BACKPRESSURE_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_S2MM_0_MEMORY_BACKPRESSURE_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_S2MM_1_MEMORY_BACKPRESSURE_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_MM2S_0_MEMORY_STARVATION_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_MM2S_1_MEMORY_STARVATION_PL)		\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_GROUP_LOCK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_0_ACQ_EQ_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_0_ACQ_GE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_0_REL_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_0_EQUAL_TO_VALUE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_1_ACQ_EQ_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_1_ACQ_GE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_1_REL_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_1_EQUAL_TO_VALUE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_2_ACQ_EQ_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_2_ACQ_GE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_2_REL_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_2_EQUAL_TO_VALUE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_3_ACQ_EQ_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_3_ACQ_GE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_3_REL_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_3_EQUAL_TO_VALUE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_4_ACQ_EQ_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_4_ACQ_GE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_4_REL_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_4_EQUAL_TO_VALUE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_5_ACQ_EQ_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_5_ACQ_GE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_5_REL_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_5_EQUAL_TO_VALUE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_GROUP_LOCK_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_0_ACQ_EQ_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_0_ACQ_GE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_0_REL_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_0_EQUAL_TO_VALUE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_1_ACQ_EQ_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_1_ACQ_GE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_1_REL_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_1_EQUAL_TO_VALUE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_2_ACQ_EQ_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_2_ACQ_GE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_2_REL_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_2_EQUAL_TO_VALUE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_3_ACQ_EQ_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_3_ACQ_GE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_3_REL_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_3_EQUAL_TO_VALUE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_4_ACQ_EQ_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_4_ACQ_GE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_4_REL_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_4_EQUAL_TO_VALUE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_5_ACQ_EQ_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_5_ACQ_GE_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_5_REL_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_5_EQUAL_TO_VALUE_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_S2MM_ERROR_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_S2MM_ERROR_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_MM2S_ERROR_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_MM2S_ERROR_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_LOCK_ERROR_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_LOCK_ERROR_PL)				\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_TASK_TOKEN_STALL_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_TASK_TOKEN_STALL_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC0_DMA_HW_ERROR_PL)			\
	_XAIE_EVENT_GENERATE_##TARGET(NOC1_DMA_HW_ERROR_PL)			\
										\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_GROUP_ACTIVITY_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_START_TASK_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_START_TASK_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_FINISHED_BD_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_FINISHED_BD_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_FINISHED_TASK_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_FINISHED_TASK_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_LOCAL_MEMORY_STARVATION_UC)	\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_REMOTE_MEMORY_BACKPRESSURE_UC)	\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_LOCAL_MEMORY_BACKPRESSURE_UC)	\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_REMOTE_MEMORY_STARVATION_UC)	\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_ERRORS_UC)					\
	_XAIE_EVENT_GENERATE_##TARGET(AXI_MM_CORE_MASTER_DECODE_ERROR_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(AXI_MM_DMA_MASTER_DECODE_ERROR_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(AXI_MM_CORE_MASTER_SLAVE_ERROR_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(AXI_MM_DMA_MASTER_SLAVE_ERROR_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_ERROR_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_ERROR_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(PM_ECC_ERROR_1BIT_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(PM_ECC_ERROR_2BIT_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(PRIVATE_DM_ECC_ERROR_1BIT_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(PRIVATE_DM_ECC_ERROR_2BIT_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(SHARED_DM_ECC_ERROR_1BIT_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(SHARED_DM_ECC_ERROR_2BIT_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_STRAMS_GROUP_ERRORS_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_AXIS_MASTER_IDLE_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_AXIS_MASTER_RUNNING_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_AXIS_MASTER_STALLED_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_AXIS_MASTER_TLAST_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_AXIS_SLAVE_IDLE_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_AXIS_SLAVE_RUNNING_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_AXIS_SLAVE_STALLED_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_AXIS_SLAVE_TLAST_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_PROGRAM_FLOW_GROUP_ERRORS_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_SLEEP_UC)					\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_INTERRUPT_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_DEBUG_SYS_RESET_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_DEBUG_WAKEUP_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_TIMER1_INTERRUPT_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_TIMER2_INTERRUPT_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_TIMER3_INTERRUPT_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_TIMER4_INTERRUPT_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_REG_WRITE_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_EXCEPTION_TAKEN_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_JUMP_TAKEN_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_JUMP_HIT_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_DATA_READ_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_DATA_WRITE_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_PIPLINE_HALTED_DEBUG_UC)			\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_STREAM_GET_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_STREAM_PUT_UC)				\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_A_START_BD_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_B_START_BD_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_A_START_BD_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_B_START_BD_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_A_START_TASK_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_B_START_TASK_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_A_START_TASK_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_B_START_TASK_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_A_FINISHED_BD_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_B_FINISHED_BD_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_A_FINISHED_BD_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_B_FINISHED_BD_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_A_FINISHED_TASK_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_B_FINISHED_TASK_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_A_FINISHED_TASK_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_B_FINISHED_TASK_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_A_LOCAL_MEMORY_STARVATION_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_B_LOCAL_MEMORY_STARVATION_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_A_REMOTE_MEMORY_BACKPRESSURE_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_B_REMOTE_MEMORY_BACKPRESSURE_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_A_LOCAL_MEMORY_BACKPRESSURE_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_B_LOCAL_MEMORY_BACKPRESSURE_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_A_REMOTE_MEMORY_STARVATION_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_B_REMOTE_MEMORY_STARVATION_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_A_RUNNING_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_DM2MM_B_RUNNING_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_A_RUNNING_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2DM_B_RUNNING_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(MODULE_A_ERROR_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(MODULE_B_ERROR_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(MODULE_A_AXI_MM_ERROR_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(MODULE_B_AXI_MM_ERROR_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(MODULE_A_ECC_ERROR_1BIT_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(MODULE_B_ECC_ERROR_1BIT_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(MODULE_A_ECC_ERROR_2BIT_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(MODULE_B_ECC_ERROR_2BIT_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_GROUP_PC_STATUS_EVENT_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_PC_0_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_PC_1_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_PC_2_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_PC_3_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_PC_4_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_PC_5_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_PC_RANGE_0_1_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_PC_RANGE_2_3_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_PC_RANGE_4_5_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_STATUS_0_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_STATUS_1_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_STATUS_2_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_STATUS_3_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_STATUS_4_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_STATUS_5_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_STATUS_6_UC)		\
	_XAIE_EVENT_GENERATE_##TARGET(CORE_STATUS_7_UC)
	/* All Mem Tile events */
#define _XAIE_EVENT_GENERATE_MEM_TILE(TARGET)				\
	_XAIE_EVENT_GENERATE_##TARGET(NONE_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(TRUE_MEM_TILE)				\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_0_MEM_TILE)				\
	_XAIE_EVENT_GENERATE_##TARGET(TIMER_SYNC_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(TIMER_VALUE_REACHED_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT0_EVENT_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT1_EVENT_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT2_EVENT_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT3_EVENT_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT4_EVENT_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT5_EVENT_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT6_EVENT_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT7_EVENT_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT8_EVENT_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT9_EVENT_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT10_EVENT_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PERF_CNT11_EVENT_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_0_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_1_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_2_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_3_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_4_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_5_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_6_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(COMBO_EVENT_7_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_0_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_1_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_2_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_3_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_4_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_5_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_6_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(EDGE_DETECTION_EVENT_7_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_WATCHPOINT_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(WATCHPOINT_0_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(WATCHPOINT_1_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(WATCHPOINT_2_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(WATCHPOINT_3_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_DMA_ACTIVITY_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_SEL0_START_BD_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_SEL1_START_BD_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_SEL0_START_BD_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_SEL1_START_BD_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_SEL0_START_TASK_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_SEL1_START_TASK_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_SEL0_START_TASK_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_SEL1_START_TASK_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_SEL0_FINISHED_BD_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_SEL1_FINISHED_BD_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_SEL0_FINISHED_BD_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_SEL1_FINISHED_BD_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_SEL0_FINISHED_TASK_MEM_TILE)	\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_SEL1_FINISHED_TASK_MEM_TILE)	\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_SEL0_FINISHED_TASK_MEM_TILE)	\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_SEL1_FINISHED_TASK_MEM_TILE)	\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_SEL0_STALLED_LOCK_ACQUIRE_MEM_TILE)\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_SEL1_STALLED_LOCK_ACQUIRE_MEM_TILE)\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_SEL0_STALLED_LOCK_ACQUIRE_MEM_TILE)\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_SEL1_STALLED_LOCK_ACQUIRE_MEM_TILE)\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_SEL0_STREAM_STARVATION_MEM_TILE)	\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_SEL1_STREAM_STARVATION_MEM_TILE)	\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_SEL0_STREAM_BACKPRESSURE_MEM_TILE)	\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_SEL1_STREAM_BACKPRESSURE_MEM_TILE)	\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_SEL0_MEMORY_BACKPRESSURE_MEM_TILE)	\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_SEL1_MEMORY_BACKPRESSURE_MEM_TILE)	\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_SEL0_MEMORY_STARVATION_MEM_TILE)	\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_SEL1_MEMORY_STARVATION_MEM_TILE)	\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_SEL0_RUNNING_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_SEL1_RUNNING_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_SEL0_RUNNING_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_SEL1_RUNNING_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_LOCK_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL0_ACQ_EQ_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL0_ACQ_GE_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL0_REL_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL0_EQUAL_TO_VALUE_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL1_ACQ_EQ_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL1_ACQ_GE_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL1_REL_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL1_EQUAL_TO_VALUE_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL2_ACQ_EQ_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL2_ACQ_GE_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL2_REL_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL2_EQUAL_TO_VALUE_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL3_ACQ_EQ_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL3_ACQ_GE_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL3_REL_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL3_EQUAL_TO_VALUE_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL4_ACQ_EQ_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL4_ACQ_GE_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL4_REL_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL4_EQUAL_TO_VALUE_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL5_ACQ_EQ_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL5_ACQ_GE_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL5_REL_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL5_EQUAL_TO_VALUE_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL6_ACQ_EQ_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL6_ACQ_GE_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL6_REL_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL6_EQUAL_TO_VALUE_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL7_ACQ_EQ_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL7_ACQ_GE_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL7_REL_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_SEL7_EQUAL_TO_VALUE_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_STREAM_SWITCH_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_0_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_0_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_0_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_0_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_1_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_1_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_1_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_1_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_2_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_2_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_2_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_2_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_3_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_3_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_3_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_3_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_4_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_4_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_4_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_4_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_5_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_5_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_5_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_5_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_6_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_6_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_6_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_6_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_IDLE_7_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_RUNNING_7_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_STALLED_7_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(PORT_TLAST_7_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_MEMORY_CONFLICT_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_0_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_1_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_2_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_3_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_4_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_5_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_6_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_7_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_8_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_9_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_10_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_11_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_12_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_13_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_14_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_15_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_16_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_17_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_18_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_19_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_20_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_21_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_22_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(CONFLICT_DM_BANK_23_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_ERRORS_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(DM_ECC_ERROR_SCRUB_CORRECTED_MEM_TILE)	\
	_XAIE_EVENT_GENERATE_##TARGET(DM_ECC_ERROR_SCRUB_2BIT_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(DM_ECC_ERROR_1BIT_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(DM_ECC_ERROR_2BIT_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_S2MM_ERROR_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_MM2S_ERROR_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(STREAM_SWITCH_PARITY_ERROR_MEM_TILE)	\
	_XAIE_EVENT_GENERATE_##TARGET(STREAM_PKT_ERROR_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(CONTROL_PKT_ERROR_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(AXI_MM_SLAVE_ERROR_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(LOCK_ERROR_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(DMA_TASK_TOKEN_STALL_MEM_TILE)		\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_BROADCAST_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_0_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_1_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_2_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_3_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_4_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_5_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_6_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_7_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_8_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_9_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_10_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_11_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_12_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_13_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_14_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(BROADCAST_15_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(GROUP_USER_EVENT_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_0_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_1_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_2_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_3_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_4_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_5_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_6_MEM_TILE)			\
	_XAIE_EVENT_GENERATE_##TARGET(USER_EVENT_7_MEM_TILE)

/**************************** Type Definitions *******************************/
/*
 * This enum contains all the Events for all modules: Core, Memory of AIE tile
 * MEM tile and PL tile
 */
typedef enum class XAie_Events {
	_XAIE_EVENT_GENERATE_CORE_MOD(ENUM)
	_XAIE_EVENT_CORE_MOD_END = (1000 - 1),
	_XAIE_EVENT_GENERATE_MEM_MOD(ENUM)
	_XAIE_EVENT_MEM_MOD_END = (2000U - 1),
	_XAIE_EVENT_GENERATE_PL_MOD(ENUM)
	_XAIE_EVENT_PL_MOD_END = (3000U - 1),
	_XAIE_EVENT_GENERATE_MEM_TILE(ENUM)
	_XAIE_EVENT_MAX
} XAie_Events;

/* Enum to capture stream switch port interface */
typedef enum class XAie_StrmPortIntf {
	XAIE_STRMSW_SLAVE,
	XAIE_STRMSW_MASTER,
} XAie_StrmPortIntf;

/* Enum to capture different event combo operations */
typedef enum class XAie_EventComboOps {
	XAIE_EVENT_COMBO_E1_AND_E2,
	XAIE_EVENT_COMBO_E1_AND_NOTE2,
	XAIE_EVENT_COMBO_E1_OR_E2,
	XAIE_EVENT_COMBO_E1_OR_NOTE2
} XAie_EventComboOps;

/* Enum to capture different event combo configs */
typedef enum class XAie_EventComboId {
	XAIE_EVENT_COMBO0,
	XAIE_EVENT_COMBO1,
	XAIE_EVENT_COMBO2,
	XAIE_EVENT_COMBO4 = 4,
	XAIE_EVENT_COMBO5,
	XAIE_EVENT_COMBO6,
} XAie_EventComboId;

/* Enum to capture event broadcast directions */
typedef enum class XAie_BroadcastDir {
	XAIE_EVENT_BROADCAST_SOUTH = 0b0001U,
	XAIE_EVENT_BROADCAST_WEST  = 0b0010U,
	XAIE_EVENT_BROADCAST_NORTH = 0b0100U,
	XAIE_EVENT_BROADCAST_EAST  = 0b1000U,
	XAIE_EVENT_BROADCAST_ALL   = 0b1111U,
} XAie_BroadcastDir;

/* Enum to capture edge event config */
typedef enum class XAie_EdgeEventTrig {
	XAIE_EDGE_EVENT_RISING  = 0b0001U,
	XAIE_EDGE_EVENT_FALLING = 0b0010U,
} XAie_EdgeEventTrig;

/************************** Function Prototypes  *****************************/
/**********************Function and enum for utils*************************************/


//=============================xaiengine/xaie_txn.h
/*****************************************************************************/
/**
* @file xaie_txn.h
*
* This file contains data structure for TxN flow
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Keyur   08/25/2023  Initial creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
/* All New custom Ops should be added above XAIE_IO_CUSTOM_OP_NEXT
 * To support backward compatibility existing enums should not be
 * modified. */
typedef enum class XAie_TxnOpcode {
	XAIE_IO_WRITE,
	XAIE_IO_BLOCKWRITE,
	XAIE_IO_BLOCKSET,
	XAIE_IO_MASKWRITE,
	XAIE_IO_MASKPOLL,
	XAIE_IO_NOOP,
	XAIE_IO_PREEMPT,
	XAIE_IO_MASKPOLL_BUSY,
	XAIE_IO_LOADPDI,
	XAIE_IO_LOAD_PM_START,
	XAIE_IO_CREATE_SCRATCHPAD,
	XAIE_IO_UPDATE_STATE_TABLE,
	XAIE_IO_UPDATE_REG,
	XAIE_IO_UPDATE_SCRATCH,
	XAIE_CONFIG_SHIMDMA_BD,
	XAIE_CONFIG_SHIMDMA_DMABUF_BD,
	XAIE_IO_CUSTOM_OP_BEGIN = 1U<<7U,
	XAIE_IO_CUSTOM_OP_TCT = XAIE_IO_CUSTOM_OP_BEGIN,
	XAIE_IO_CUSTOM_OP_DDR_PATCH, // Previously this was XAIE_IO_CUSTOM_OP_BEGIN + 1
	XAIE_IO_CUSTOM_OP_READ_REGS, // Previously this was XAIE_IO_CUSTOM_OP_BEGIN + 2
	XAIE_IO_CUSTOM_OP_RECORD_TIMER, // Previously this was XAIE_IO_CUSTOM_OP_BEGIN + 3
	XAIE_IO_CUSTOM_OP_MERGE_SYNC, // Previously this was XAIE_IO_CUSTOM_OP_BEGIN + 4
	XAIE_IO_CUSTOM_OP_NEXT,
	XAIE_IO_LOAD_PM_END_INTERNAL = 200,
	XAIE_IO_CUSTOM_OP_MAX = UCHAR_MAX,
} XAie_TxnOpcode;

struct XAie_TxnCmd {
	XAie_TxnOpcode Opcode;
	u32 Mask;
	u64 RegOff;
	u32 Value;
	u32 Size;
	u64 DataPtr;
	u32 PmId;
	u16 PdiId;
	u8 Preempt_level;
	u64 ScratchOffset;
	u8 UsageType;
	u8 StateTableIdx;
	u32 Func;
	u32 FuncArg;
};

/* typedef to capture transaction buffer data */
typedef struct XAie_TxnInst {
	u64 Tid;
	u32 Flags;
	u32 NumCmds;	// Actual no of command available in TXN Cmd Array
	u32 MaxCmds;	// Current allocated size of TXN Cmd Array
	u32 InitCmds;	// Initial allocated size of TXN Cmd Array
	u8  NextCustomOp;
	XAie_TxnCmd *CmdBuf;
	XAie_List Node;
} XAie_TxnInst;

typedef struct XAie_TxnHeader {
	uint8_t Major;
	uint8_t Minor;
	uint8_t DevGen;
	uint8_t NumRows;
	uint8_t NumCols;
	uint8_t NumMemTileRows;
	uint16_t padding;
	uint32_t NumOps;
	uint32_t TxnSize;
} XAie_TxnHeader;

typedef struct XAie_OpHdr {
	uint8_t Op;
	uint8_t Col;
	uint8_t Row;
} XAie_OpHdr;

typedef struct XAie_Write32Hdr {
	XAie_OpHdr OpHdr;
	uint64_t RegOff;
	uint32_t Value;
	uint32_t Size;
} XAie_Write32Hdr;

typedef struct XAie_MaskWrite32Hdr {
	XAie_OpHdr OpHdr;
	uint64_t RegOff;
	uint32_t Value;
	uint32_t Mask;
	uint32_t Size;
} XAie_MaskWrite32Hdr;

typedef struct XAie_MaskPoll32Hdr {
	XAie_OpHdr OpHdr;
	uint64_t RegOff;
	uint32_t Value;
	uint32_t Mask;
	uint32_t Size;
} XAie_MaskPoll32Hdr;

typedef struct XAie_BlockWrite32Hdr {
	XAie_OpHdr OpHdr;
	uint8_t Col;
	uint8_t Row;
	uint32_t RegOff;
	uint32_t Size;
} XAie_BlockWrite32Hdr;

typedef struct XAie_CustomOpHdr {
	XAie_OpHdr OpHdr;
	uint32_t Size;
} XAie_CustomOpHdr;

typedef struct XAie_NoOpHdr {
	uint8_t Op;
	uint8_t padding[3];
} XAie_NoOpHdr;

typedef enum class XAie_Preempt_level {
	NOOP,
	MEM_TILE,
	AIE_TILE,
	AIE_REGISTERS,
	INVALID
} XAie_Preempt_level;

typedef struct XAie_PreemptHdr {
	uint8_t Op;
	uint8_t Preempt_level;
	uint16_t Reserved;
} XAie_PreemptHdr;

typedef struct XAie_LoadPdiHdr {
	uint8_t Op;
	uint8_t Padding;
	uint16_t PdiId;
	uint32_t PdiSize;
	uint64_t PdiAddress;
} XAie_LoadPdiHdr;

typedef struct XAie_PmLoadHdr {
	uint8_t Op;
	uint8_t LoadSequenceCount[3];
	uint32_t PmLoadId;
} XAie_PmLoadHdr;

typedef enum class XAie_UsageType {
	XAIE_STATE_TABLE
}XAie_UsageType;

typedef struct XAie_CreateScratchpadHdr {
	uint8_t Op;
	uint8_t UsageType;
	uint16_t padding;
	uint32_t Size;
	uint64_t ScratchOffset;
}XAie_CreateScratchpadHdr;

typedef enum class XAie_StateTableFuncType {
	XAIE_STATE_TABLE_MUL,
	XAIE_STATE_TABLE_INCR,
	XAIE_STATE_TABLE_DECR,
}XAie_StateTableFuncType;

struct XAie_UpdateStateHdr {
	uint8_t Op;
	uint8_t StateTableIdx;
	uint8_t Func;
	uint8_t Reserved;
	uint32_t FuncArg;
} ;

struct XAie_UpdateRegHdr {
	uint8_t Op;
	uint8_t StateTableIdx;
	uint8_t Func;
	uint8_t Reserved;
	uint32_t FuncArg;
	uint32_t RegOff;
} ;

struct XAie_UpdateScratchHdr {
	uint8_t Op;
	uint8_t padding[3];
} ;

/* Migrated from Aie-controller */

enum op_types {
    e_GENERATE_ENUM,
};

typedef struct op_base{
    enum op_types type;
    unsigned int size_in_bytes;
} op_base;

typedef struct tct_op_t {
    uint32_t word;
    uint32_t config;
} tct_op_t;

typedef struct patch_op_t {
    op_base b;
    u32 action;
    u64 regaddr; // regaddr to patch
    u64 argidx;  // kernel arg idx to get value to write at regaddr
    u64 argplus; // value to add to what's passed @ argidx (e.g., offset to shim addr)
} patch_op_t;

/*
 * Structs for reading registers
 */
typedef struct register_data_t {
    uint64_t address;
} register_data_t;

typedef struct read_register_op_t {
    uint32_t count;
    register_data_t data[1]; // variable size
} read_register_op_t;

/*
 * Struct for record timer identifier
 */
typedef struct record_timer_op_t {
    uint32_t id;
} record_timer_op_t;



/* Optimized Txn structs start from here */
typedef struct{
	uint8_t Op;
	uint8_t padding[3];
} XAie_OpHdr_opt;

typedef struct XAie_Write32Hdr_opt {
	XAie_OpHdr_opt OpHdr;
	uint32_t RegOff;
	uint32_t Value;
} XAie_Write32Hdr_opt;

typedef struct XAie_MaskWrite32Hdr_opt {
	XAie_OpHdr_opt OpHdr;
	uint32_t RegOff;
	uint32_t Value;
	uint32_t Mask;
} XAie_MaskWrite32Hdr_opt;

typedef struct XAie_MaskPoll32Hdr_opt {
	XAie_OpHdr_opt OpHdr;
	uint32_t RegOff;
	uint32_t Value;
	uint32_t Mask;
} XAie_MaskPoll32Hdr_opt;

typedef struct XAie_BlockWrite32Hdr_opt {
	XAie_OpHdr_opt OpHdr;
	uint32_t RegOff;
	uint32_t Size;
} XAie_BlockWrite32Hdr_opt;

typedef struct XAie_CustomOpHdr_opt {
	XAie_OpHdr_opt OpHdr;
	uint32_t Size;
} XAie_CustomOpHdr_opt;

typedef struct patch_op_opt_t {
    uint32_t regaddr; // regaddr to patch
    uint8_t argidx;  // kernel arg idx to get value to write at regaddr
    uint8_t padding[3];
    uint64_t argplus; // value to add to what's passed @ argidx (e.g., offset to shim addr)
} patch_op_opt_t;

typedef struct tct_op_opt_t {
    uint32_t word;
    uint32_t config;
} tct_op_opt_t;

/*
 * Structs for reading registers
 */

typedef struct register_data_opt_t {
    uint64_t address;
} register_data_opt_t;

typedef struct read_register_op_opt_t {
    uint32_t count;
    uint32_t padding;
    register_data_opt_t data[1]; // variable size
} read_register_op_opt_t;

/*
 * Struct for record timer identifier
 */
typedef struct record_timer_op_opt_t {
    uint32_t id;
} record_timer_op_opt_t;

//=============================xaie_io.h

/*****************************************************************************/
/**
* @file xaie_io.c
*
* This file contains the data structures and routines for low level IO
* operations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   06/09/2020 Initial creation.
* 1.1   Tejus   06/10/2020 Add helper function to get backend pointer.
* </pre>
*
******************************************************************************/

/***************************** Macro Definitions *****************************/
constexpr u32 _XAIE_RSC_MGR_CONTIG_FLAG = 0x1U;
/****************************** Type Definitions *****************************/

/*
 * Typedef for enum to capture backend function code
 */
typedef enum class XAie_BackendOpCode {
        XAIE_BACKEND_OP_NPIWR32,
        XAIE_BACKEND_OP_NPIMASKPOLL32,
        XAIE_BACKEND_OP_RST_PART,
        XAIE_BACKEND_OP_ASSERT_SHIMRST,
        XAIE_BACKEND_OP_SET_PROTREG,
        XAIE_BACKEND_OP_CONFIG_SHIMDMABD,
        XAIE_BACKEND_OP_REQUEST_TILES,
        XAIE_BACKEND_OP_RELEASE_TILES,
        XAIE_BACKEND_OP_PARTITION_INITIALIZE,
        XAIE_BACKEND_OP_PARTITION_TEARDOWN,
        XAIE_BACKEND_OP_PARTITION_CLEAR_CONTEXT,
        XAIE_BACKEND_OP_UPDATE_NPI_ADDR,
        XAIE_BACKEND_OP_SET_COLUMN_CLOCK,
        XAIE_BACKEND_OP_PERFORMANCE_UTILIZATION,
        XAIE_BACKEND_OP_UPDATE_SHIM_DMA_BD_ADDR,
        XAIE_BACKEND_OP_CONFIG_MEM_INTRLVNG,
} XAie_BackendOpCode;

/*
 * Typedef for structure for NPI write 32bit structure
 */
typedef struct XAie_BackendNpiWrReq {
        u32 NpiRegOff;
        u32 Val;
} XAie_BackendNpiWrReq;

/*
 * Typedef for structure for NPI Mask Poll structure
 */
typedef struct XAie_BackendNpiMaskPollReq {
        u32 NpiRegOff;
        u32 Mask;
        u32 Val;
        u32 TimeOutUs;
} XAie_BackendNpiMaskPollReq;

/*
 * Typedef for structure for tiles array
 */
typedef struct XAie_BackendTilesArray {
        XAie_LocType *Locs;
        u32 NumTiles;
} XAie_BackendTilesArray;

/*
 * Typedef for structure for columns
 */
typedef struct XAie_BackendColumnReq {
        u32 StartCol;
        u32 NumCols;
        u8 Enable;
} XAie_BackendColumnReq;

/*
 * Typedef for structure for tiles array with enable/disable flag for all of them
 */
typedef struct XAie_BackendTilesEnableArray {
        XAie_LocType *Locs;
        u32 NumTiles;
        u8 Enable;
} XAie_BackendTilesEnableArray;

/* Typedef to capture shimdma Bd arguments */
typedef struct XAie_ShimDmaBdArgs {
        XAie_MemInst *MemInst;
        u8 NumBdWords;
        u32 *BdWords;
        XAie_LocType Loc;
        u64 VAddr;
        u32 BdNum;
        u64 Addr;
} XAie_ShimDmaBdArgs;

typedef enum class XAie_ModeSelect {
        XAIE_SHIM_BD_CHAINING_DISABLE,
        XAIE_SHIM_BD_CHAINING_ENABLE,
        XAIE_WRITE_DES_ASYNC_DISABLE,
        XAIE_WRITE_DES_ASYNC_ENABLE,
        XAIE_INVALID_MODE
} XAie_ModeSelect;

/*
 * Typdef to capture all the backend IO operations
 * Init        : Backend specific initialization function. Init should attach
 *               private data to DevInst which is later used by other ops.
 * Finish      : Backend specific IO finish function. Backend specific cleanup
 *               should be part of this function.
 * Write32     : IO operation to write 32-bit data.
 * Read32      : IO operation to read 32-bit data.
 * MaskWrite32 : IO operation to write masked 32-bit data.
 * MaskPoll    : IO operation to mask poll an address for a value.
 * BlockWrite32: IO operation to write a block of data at 32-bit granularity.
 * BlockSet32  : IO operation to initialize a chunk of aie address space with a
 *               a specified value at 32-bit granularity.
 * CmdWrite32  : This IO operation is required only in simulation mode. Other
 *               backends should have a no-op.
 * RunOp       : Run operation specified by the operation code
 * MemAllocate : Backend operation to allocate memory for the user. In addition
 *               to that, the operation is expected to allocate memory for
 *               MemInst and populate Size, virtual address and device address..
 * MemFree     : Backend operation to free allocated memory, MemInst allocated
 *               by the MemAllocate api.
 * MemSyncForCPU: Backend operation to prepare memory for CPU access.
 * MemSyncForDev: Backend operation to prepare memory for Device access.
 * MemAttach    : Backend operation to attach memory to AI engine device.
 * MemDetach    : Backend operation to detach memory from AI engine device
 * GetTid       : Backend operation to get unique thread id.
 * SubmitTxn    : Backend operation to submit transaction.
 */
typedef struct XAie_BackendOps {
        AieRC (*Init)(XAie_DevInst *DevInst);
        AieRC (*Finish)(void *IOInst);
        AieRC (*Write32)(void *IOInst, u64 RegOff, u32 Value);
        AieRC (*Read32)(void *IOInst,  u64 RegOff, u32 *Data);
        AieRC (*MaskWrite32)(void *IOInst, u64 RegOff, u32 Mask, u32 Value);
        AieRC (*MaskPoll)(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
                        u32 TimeOutUs);
        AieRC (*BlockWrite32)(void *IOInst, u64 RegOff, const u32 *Data, u32 Size);
        AieRC (*BlockSet32)(void *IOInst, u64 RegOff, u32 Data, u32 Size);
        AieRC (*CmdWrite)(void *IOInst, u8 Col, u8 Row, u8 Command, u32 CmdWd0,
                        u32 CmdWd1, const char *CmdStr);
        AieRC (*RunOp)(void *IOInst, XAie_DevInst *DevInst,
                     XAie_BackendOpCode Op, void *Arg);
        AieRC (*AddressPatching)(void *IOInst, u16 Arg_Index, u8 Num_BDs);
        AieRC (*WaitTaskCompleteToken) (XAie_DevInst *DevInst,
        uint16_t Column, uint16_t Row, uint32_t Channel, uint8_t NumTokens);
        XAie_MemInst* (*MemAllocate)(XAie_DevInst *DevInst, u64 Size,
                        XAie_MemCacheProp Cache);
        AieRC (*MemFree)(XAie_MemInst *MemInst);
        AieRC (*MemSyncForCPU)(XAie_MemInst *MemInst);
        AieRC (*MemSyncForDev)(XAie_MemInst *MemInst);
        AieRC (*MemAttach)(XAie_MemInst *MemInst, u64 MemHandle);
        AieRC (*MemDetach)(XAie_MemInst *MemInst);
        u64 (*GetTid)(void);
        AieRC (*SubmitTxn)(void *IOInst, XAie_TxnInst *TxnInst);
        void* (*GetShimDmaBdConfig)(XAie_ShimDmaBdArgs *Args);
        u64 (*GetAttr)(void *IOInst, XAie_BackendAttrType Type);
        AieRC (*SetAttr)(void *IOInst, XAie_BackendAttrType Type, u64 AttrVal);
        AieRC (*WaitUcDMA) (void *IOInst);
        AieRC (*ConfigMode)(void *IOInst, XAie_ModeSelect Mode);
        XAie_ModeSelect (*GetConfigMode) (void *IOInst);
        AieRC (*Preempt)(void *IOInst, u16 PreemptId, char* SaveLabel, char* RestoreLabel);
        AieRC (*SetPadInteger)(void *IOInst, char* BuffName, u32 BuffSize);
        AieRC (*SetPadString)(void *IOInst, char* BuffName, char* BuffBlobPath);
        AieRC (*AttachToGroup)(void *IOInst, uint8_t GroupId);
        AieRC (*RemoteBarrier)(void *IOInst, uint8_t RbId, uint32_t UcMask);
        AieRC (*SaveRegister) (void *IOInst, u32 RegOff, u32 Id);
} XAie_BackendOps;

/* Typedef to capture all backend information */
typedef struct XAie_Backend {
        XAie_BackendType Type;
        XAie_BackendOps Ops;
} XAie_Backend;



//=============================xaiegbl_regdef.h

/*****************************************************************************/
/**
* @file xaiegbl_regdef.h
*
* Header to include type definitions for the register bit field definitions
* of Core, Memory, NoC and PL module registers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus  09/26/2019  Initial creation
* 1.1   Tejus  10/21/2019  Optimize stream switch data structures
* 1.2   Tejus  10/28/2019  Add data structures for pl interface module
* 1.3   Tejus  12/09/2019  Forward declaration of structures
* 1.4   Tejus  03/16/2020  Add register properties for Mux/Demux registers
* 1.5   Tejus  03/17/2020  Add data structures for lock module
* 1.6   Tejus  03/21/2020  Add data structures for stream switch slot registers
* 1.7   Tejus  03/23/2020  Re-organize data structure to capture aie dmas
* 1.8  Dishita 03/24/2020  Add data structure for performance counter
* 1.9  Dishita 04/16/2020  Fix compiler warnings related to performance counter
* 2.0  Dishita 04/20/2020  Add data structure for timer
* 2.1   Tejus   05/26/2020  Restructure and optimize core module.
* 2.2   Tejus  06/01/2020  Add data structure for debug halt register.
* 2.3   Tejus  06/05/2020  Add field in data structure for dma fifo mode.
* 2.4   Nishad 06/16/2020  Add data structures for trace module
* 2.5   Nishad 06/28/2020  Add data structures for event selection and combo
*                          event registers
* 2.6   Nishad 07/01/2020  Add MstConfigBaseAddr property to stream switch data
*                          structure
* 2.7   Nishad 07/12/2020  Add data structure and register properties to support
*                          event broadcast, PC event, and group events.
* 2.8   Nishad 07/21/2020  Add data structure for interrupt controller.
* 2.9   Nishad 07/24/2020  Add event property to capture default group error
*                          mask.
* </pre>
*
******************************************************************************/

/**************************** Macro Definitions ******************************/
constexpr u32 _XAIE_FEATURE_AVAILABLE   = 1U;
constexpr u32 _XAIE_FEATURE_UNAVAILABLE = 0U;

/************************** Constant Definitions *****************************/
/**
 * This typedef contains the attributes for the register bit fields.
 */
typedef struct XAie_RegFldAttr {
        u32 Lsb;        /**< Bit position of the register bit-field in the 32-bit value */
        u32 Mask;       /**< Bit mask of the register bit-field in the 32-bit value */
} XAie_RegFldAttr;

/**
 * This typedef contains the attributes for the register bit fields of buffer descriptors.
 */
typedef struct XAie_RegBdFldAttr {
        u8  Idx;
        u32 Lsb;        /**< Bit position of the register bit-field in the 32-bit value */
        u32 Mask;       /**< Bit mask of the register bit-field in the 32-bit value */
} XAie_RegBdFldAttr;
/**
 *  * This typedef contains the attributes for the uC module Dma pause register.
 *  */
typedef struct XAie_RegUcDmaPause {
       u32 RegOff;                     /**< Register offset */
       XAie_RegFldAttr Mm2dm;          /**< Mm2dm field attributes */
       XAie_RegFldAttr Dm2mm;          /**< Dm2mm field attributes */
} XAie_RegUcDmaPause ;

/**
 *  * This typedef contains the attributes for the Uc module axi-mm dma outstanding transaction register.
 *  */
typedef struct XAie_RegUcDmaOutsTxn {
       u32 RegOff;
       XAie_RegFldAttr UcModuleToArray;
       XAie_RegFldAttr UcDMAToNMU;
} XAie_RegUcDmaOutsTxn;

/**
 *  * This typedef contains the attributes for the Noc module axi-mm dma outstanding transaction register.
 *  */
typedef struct XAie_RegNocDmaOutsTxn {
       u32 RegOff;
       XAie_RegFldAttr NoCModuleToNMU;
} XAie_RegNocDmaOutsTxn;
/**
 *  * This typedef contains the attributes for the Noc module Dma pause register.
 *  */
typedef struct XAie_RegNocDmaPause {
        u32 RegOff;                     /**< Register offset */
        XAie_RegFldAttr Mm2s_1;          /**< Mm2s_1 field attributes */
        XAie_RegFldAttr Mm2s_0;          /**< Mm2s_0 field attributes */
        XAie_RegFldAttr S2mm_1;          /**< S2mm_1 field attributes */
        XAie_RegFldAttr S2mm_0;          /**< S2mm_0 field attributes */
} XAie_RegNocDmaPause;

/**
 * This typedef contains the attributes for the uC module Core control register.
*/
typedef struct XAie_RegUcCoreCtrl {
        u32 RegOff;                     /**< Register offset */
        XAie_RegFldAttr CtrlWakeup;     /**< Wakeup field attributes */
        XAie_RegFldAttr CtrlSleep;      /**< Sleep field attributes */
} XAie_RegUcCoreCtrl ;
/**
 * This typedef contains the attributes for the uC module Core status register.
 */
typedef struct XAie_RegUcCoreSts {
        u32 RegOff;                     /**< Register offset */
        u32 Mask;                       /**< Core status register Mask */
        XAie_RegFldAttr Intr;           /**< Interrupt value field attributes */
        XAie_RegFldAttr Sleep;          /**< Sleep value field attributes */
} XAie_RegUcCoreSts ;

/**
 * This typedef contains the attributes for the Core control register.
 */
typedef struct XAie_RegCoreCtrl {
        u32 RegOff;                     /**< Register offset */
        XAie_RegFldAttr CtrlEn; /**< Enable field attributes */
        XAie_RegFldAttr CtrlRst;        /**< Reset field attributes */
} XAie_RegCoreCtrl ;

/**
 * This typedef contains the attributes for the Core status register.
 */
typedef struct XAie_RegCoreSts {
        u32 RegOff;                     /**< Register offset */
        u32 Mask;                       /**< Core status register Mask */
        XAie_RegFldAttr Done;
        XAie_RegFldAttr Rst;            /**< Reset value field attributes */
        XAie_RegFldAttr En;             /**< Enable value field attributes */
} XAie_RegCoreSts ;

/*
 * This typedef contains the attributes for core debug halt register
 */
typedef struct XAie_RegCoreDebug {
        u32 RegOff;
        u32 DebugCtrl1Offset;
        XAie_RegFldAttr DebugHalt;
        XAie_RegFldAttr DebugHaltCoreEvent1;
        XAie_RegFldAttr DebugHaltCoreEvent0;
        XAie_RegFldAttr DebugSStepCoreEvent;
        XAie_RegFldAttr DebugResumeCoreEvent;
} XAie_RegCoreDebug ;

/*
 * This typedef contains the attributes for core debug halt status register
 */
typedef struct XAie_RegCoreDebugStatus {
        u32 RegOff;
        XAie_RegFldAttr DbgEvent1Halt;
        XAie_RegFldAttr DbgEvent0Halt;
        XAie_RegFldAttr DbgStrmStallHalt;
        XAie_RegFldAttr DbgLockStallHalt;
        XAie_RegFldAttr DbgMemStallHalt;
        XAie_RegFldAttr DbgPCEventHalt;
        XAie_RegFldAttr DbgHalt;
} XAie_RegCoreDebugStatus ;

/*
 * This typedef contains the attributes for enable events register
 */
typedef struct XAie_RegCoreEvents {
        u32 EnableEventOff;
        XAie_RegFldAttr DisableEventOccurred;
        XAie_RegFldAttr EnableEventOccurred;
        XAie_RegFldAttr DisableEvent;
        XAie_RegFldAttr EnableEvent;
} XAie_RegCoreEvents ;

/*
 * This typedef contains the attributes for core accumulator control register
 */
typedef struct XAie_RegCoreAccumCtrl {
        XAie_RegFldAttr CascadeInput;
        XAie_RegFldAttr CascadeOutput;
        u32 RegOff;
} XAie_RegCoreAccumCtrl ;

/*
 * This typedef captures port base address and number of slave ports available
 * for stream switch master and salve ports
 */
typedef struct XAie_StrmPort {
        u8 NumPorts;
        u32 PortBaseAddr;
        u8 PortLogicalId;
        u8 PortPhysicalId;
} XAie_StrmPort ;

/*
 * This typedef captures physical port to logical port mapping for stream
 * switch modules
 */
typedef struct XAie_StrmSwPortMap {
        StrmSwPortType PortType;
        u8 PortNum;
} XAie_StrmSwPortMap ;

/*
 * This typedef captures the register fields required to configure stream switch
 * deterministic merge registers
 */
typedef struct XAie_StrmSwDetMerge {
        u8 NumArbitors;
        u8 NumPositions;
        u32 ArbConfigOffset;
        u32 ConfigBase;
        u32 EnableBase;
        XAie_RegFldAttr SlvId0;
        XAie_RegFldAttr SlvId1;
        XAie_RegFldAttr PktCount0;
        XAie_RegFldAttr PktCount1;
        XAie_RegFldAttr Enable;
} XAie_StrmSwDetMerge ;

/*
 * This typedef contains the attributes for Stream Switch Module
 */
typedef struct XAie_StrmMod {
        u8 NumSlaveSlots;
        u8 MaxMasterPhyPortId;
        u8 MaxSlavePhyPortId;
        u8 DetMergeFeature;
        u32 SlvConfigBaseAddr;
        u32 MstrConfigBaseAddr;
        u32 PortOffset;           /**< Offset between ports */
        u32 SlotOffsetPerPort;
        u32 SlotOffset;
        XAie_RegFldAttr MstrEn;   /**< Enable bit field attributes */
        XAie_RegFldAttr MstrPktEn;/**< Packet enable bit field attributes */
        XAie_RegFldAttr DrpHdr;   /**< Drop header bit field attributes */
        XAie_RegFldAttr Config;   /**< Configuration bit field attributes */
        XAie_RegFldAttr SlvEn;    /**< Enable bit field attributes */
        XAie_RegFldAttr SlvPktEn; /**< Packet enable bit field attributes */
        XAie_RegFldAttr SlotPktId;
        XAie_RegFldAttr SlotMask;
        XAie_RegFldAttr SlotEn;
        XAie_RegFldAttr SlotMsel;
        XAie_RegFldAttr SlotArbitor;
        const XAie_StrmPort *MstrConfig;
        const XAie_StrmPort *SlvConfig;
        const XAie_StrmPort *SlvSlotConfig;
        const XAie_StrmSwPortMap *MasterPortMap;
        const XAie_StrmSwPortMap *SlavePortMap;
        const XAie_StrmSwDetMerge *DetMerge;

        AieRC (*PortVerify)(XAie_DevInst *DevInst, StrmSwPortType Slave,
                        u8 SlvPortNum, StrmSwPortType Master, u8 MstrPortNum);
} XAie_StrmMod ;

/*
 * The typedef contains the attributes of core processor bus.
 */
typedef struct XAie_RegCoreProcBusCtrl {
        u32 RegOff;
        XAie_RegFldAttr CtrlEn;
} XAie_RegCoreProcBusCtrl ;
	
/*
 * The typedef contains the attributes of Core Modules
 */
typedef struct XAie_CoreMod {
        u8 IsCheckerBoard;
        u32 ProgMemAddr;
        u32 ProgMemSize;
        u32 ProgMemHostOffset;
        u32 DataMemAddr;
        u32 DataMemSize;
        u32 DataMemShift;
        u32 EccEvntRegOff;
        u32 EccScubPeriodRegOff;
        u32 CorePCOff;
        u32 CoreSPOff;
        u32 CoreLROff;
        const XAie_RegCoreDebugStatus *CoreDebugStatus;
        const XAie_RegCoreSts *CoreSts;
        const XAie_RegCoreCtrl *CoreCtrl;
        const XAie_RegCoreDebug *CoreDebug;
        const XAie_RegCoreEvents *CoreEvent;
        const XAie_RegCoreAccumCtrl *CoreAccumCtrl;
        const XAie_RegCoreProcBusCtrl *ProcBusCtrl;
        AieRC (*ConfigureDone)(XAie_DevInst *DevInst, XAie_LocType Loc,
                        const struct XAie_CoreMod *CoreMod);
        AieRC (*WaitForDone)(XAie_DevInst *DevInst, XAie_LocType Loc,
                        u32 TimeOut, const struct XAie_CoreMod *CoreMod, u8 BusyPoll);
        AieRC (*ReadDoneBit)(XAie_DevInst *DevInst, XAie_LocType Loc,
                        u8 *DoneBit, const struct XAie_CoreMod *CoreMod);
        AieRC (*Enable)(XAie_DevInst *DevInst, XAie_LocType Loc,
                        const struct XAie_CoreMod *CoreMod);
        AieRC (*GetCoreStatus)(XAie_DevInst *DevInst, XAie_LocType Loc,
                        u32 *CoreStatus, const struct XAie_CoreMod *CoreMod);
} XAie_CoreMod ;

/*
 * The typedef contains the attributes of Core Internal Modules
 */
typedef struct XAie_CoreIntMod {
        u32 ProgMemAddr;
        u32 CorePCOff;
        u32 CoreSPOff;
        u32 CoreLROff;
} XAie_CoreIntMod;

/*
 * This typedef contains the attributes for shim uC MDM performance counter
 * event setting register
 */
typedef struct XAie_RegUcMdmPerfEvents {
        u8 MaxEventId;
        u32 RegOff;
        u32 Mask;
} XAie_RegUcMdmPerfEvents ;

/*
 * This typedef contains the attributes for shim uC MDM performance counter
 * control register
 */
typedef struct XAie_RegUcMdmPerfCtrl {
        u32 RegOff;
        XAie_RegFldAttr Clear;
        XAie_RegFldAttr Start;
        XAie_RegFldAttr Stop;
        XAie_RegFldAttr Sample;
        XAie_RegFldAttr Reset;
} XAie_RegUcMdmPerfCtrl ;

/*
 * This typedef contains the attributes for shim uC MDM performance counter
 * status register
 */
typedef struct XAie_RegUcMdmPerfSts {
        u32 RegOff;
        XAie_RegFldAttr Full;
        XAie_RegFldAttr Overflow;
} XAie_RegUcMdmPerfSts ;

/*
 * This typedef contains the attributes for shim uC Microblaze Debug Module (MDM)
 */
typedef struct XAie_UcMdm {
        u32 PerfCntReadRegOff;
        u32 PerfCntWriteRegOff;
        const XAie_RegUcMdmPerfEvents *PerfEvents;
        const XAie_RegUcMdmPerfCtrl *PerfCtrl;
        const XAie_RegUcMdmPerfSts *PerfSts;
        u8 NumEventCounters;
        u8 NumLatencyCounters;
        u8 CounterWidth;
} XAie_UcMdm ;

/*
 * The typedef contains the attributes of uC Modules
 */
typedef struct XAie_UcMod {
        u8 IsCheckerBoard;
        u32 BaseAddress;
        u32 ProgMemAddr;
        u32 ProgMemSize;
        u32 ProgMemHostOffset;
        u32 PrivDataMemAddr;
        u32 PrivDataMemSize;
        u32 DataMemAddr;
        u32 DataMemSize;
        u32 DataMemUcOffset;
        u32 MemPrivilegedOffset;
        u32 UcModuleEventSelect;
        u32 UcModuleBasePCEventRegOff;
        u8 NumPCEvents;
        XAie_RegFldAttr PCAddr;
        XAie_RegFldAttr PCValid;
        const XAie_RegUcCoreCtrl *CoreCtrl;
        const XAie_RegUcCoreSts *CoreSts;
        const XAie_RegUcDmaOutsTxn *UcDmaOutstandingReg;
        const XAie_RegUcDmaPause  *UcDmaPauseReg;
        const XAie_UcMdm *UcMdm;
        AieRC (*Wakeup)(XAie_DevInst *DevInst, XAie_LocType Loc,
                const struct XAie_UcMod *UcMod);
        AieRC (*Sleep)(XAie_DevInst *DevInst, XAie_LocType Loc,
                const struct XAie_UcMod *UcMod);
        AieRC (*GetCoreStatus)(XAie_DevInst *DevInst, XAie_LocType Loc,
                u32 *CoreStatus, const struct XAie_UcMod *UcMod);
        AieRC (*UcDmaPause)(XAie_DevInst *DevInst, XAie_LocType *Loc,
                u32 ChNum, u8 Pause, const struct XAie_UcMod *UcMod);
        AieRC (*GetUcDmaAxiMmOutstandingTxn)(XAie_DevInst *DevInst, XAie_LocType Loc,
                const struct XAie_UcMod *UcMod, u32 *Status);
} XAie_UcMod;

/*
 * The typedef captures the Buffer descriptor validity properties
 */
typedef struct XAie_DmaBdEnProp {
        XAie_RegBdFldAttr TlastSuppress;
        XAie_RegBdFldAttr ValidBd;
        XAie_RegBdFldAttr NxtBd;
        XAie_RegBdFldAttr UseNxtBd;
        XAie_RegBdFldAttr OutofOrderBdId;
} XAie_DmaBdEnProp ;

/*
 * The typedef captures the buffer descriptor packet properties
 */
typedef struct XAie_DmaBdPkt {
        XAie_RegBdFldAttr EnPkt;
        XAie_RegBdFldAttr PktId;
        XAie_RegBdFldAttr PktType;
} XAie_DmaBdPkt ;

/*
 * The typedef captures the buffer descriptor lock properties of aie
 */
typedef struct XAie_AieDmaLock {
        XAie_RegBdFldAttr LckId_A;
        XAie_RegBdFldAttr LckRelEn_A;
        XAie_RegBdFldAttr LckRelVal_A;
        XAie_RegBdFldAttr LckRelUseVal_A;
        XAie_RegBdFldAttr LckAcqEn_A;
        XAie_RegBdFldAttr LckAcqVal_A;
        XAie_RegBdFldAttr LckAcqUseVal_A;
        XAie_RegBdFldAttr LckId_B;
        XAie_RegBdFldAttr LckRelEn_B;
        XAie_RegBdFldAttr LckRelVal_B;
        XAie_RegBdFldAttr LckRelUseVal_B;
        XAie_RegBdFldAttr LckAcqEn_B;
        XAie_RegBdFldAttr LckAcqVal_B;
        XAie_RegBdFldAttr LckAcqUseVal_B;
} XAie_AieDmaLock ;

/*
 * The typedef captures the buffer descriptor lock properties of aieml
 */
typedef struct XAie_AieMlDmaLock {
        XAie_RegBdFldAttr LckRelVal;
        XAie_RegBdFldAttr LckRelId;
        XAie_RegBdFldAttr LckAcqEn;
        XAie_RegBdFldAttr LckAcqVal;
        XAie_RegBdFldAttr LckAcqId;
} XAie_AieMlDmaLock ;

/*
 * union to capture lock properties of dma
 */
typedef union XAie_DmaBdLock {
        XAie_AieDmaLock AieDmaLock;
        XAie_AieMlDmaLock AieMlDmaLock;
} XAie_DmaBdLock;

/*
 * typedef to capture Buffer properties of tile dma
 */
typedef struct XAie_TileDmaBuffer {
        XAie_RegBdFldAttr BaseAddr;
} XAie_TileDmaBuffer;

/*
 * The typedef captures the buffer properties of shim dma
 */
typedef struct XAie_ShimDmaBuffer {
        XAie_RegBdFldAttr AddrLow;
        XAie_RegBdFldAttr AddrHigh;
        XAie_RegBdFldAttr AddrExtHigh;
} XAie_ShimDmaBuffer ;

/*
 * union to capture buffer address and length properties
 */
typedef union XAie_DmaBdBuffer {
        XAie_TileDmaBuffer TileDmaBuff;
        XAie_ShimDmaBuffer ShimDmaBuff;
} XAie_DmaBdBuffer;

/*
 * The typedef captures DoubleBuffer properties
 */
typedef struct XAie_DmaBdDoubleBuffer {
        XAie_RegBdFldAttr EnDoubleBuff;
        XAie_RegBdFldAttr BaseAddr_B;
        XAie_RegBdFldAttr FifoMode;
        XAie_RegBdFldAttr EnIntrleaved;
        XAie_RegBdFldAttr IntrleaveCnt;
        XAie_RegBdFldAttr BuffSelect;
} XAie_DmaBdDoubleBuffer ;

/*
 * The typedef captures buffer descriptor fields of aie 2D Mode
 */
typedef struct XAie_AieAddressMode {
        XAie_RegBdFldAttr X_Incr;
        XAie_RegBdFldAttr X_Wrap;
        XAie_RegBdFldAttr X_Offset;
        XAie_RegBdFldAttr Y_Incr;
        XAie_RegBdFldAttr Y_Wrap;
        XAie_RegBdFldAttr Y_Offset;
        XAie_RegBdFldAttr CurrPtr;
} XAie_AieAddressMode ;

/*
 * The typedef captures the dimension descriptors for aieml
 */
typedef struct XAie_AieMlDmaDimProp {
        XAie_RegBdFldAttr StepSize;
        XAie_RegBdFldAttr Wrap;
} XAie_AieMlDmaDimProp ;

/*
 * The typedef captures buffer descriptor fields of aieml multi dimension
 * address generation
 */
typedef struct XAie_AieMlAddressMode {
        XAie_AieMlDmaDimProp DmaDimProp[4U];
        XAie_AieMlDmaDimProp Iter;
        XAie_RegBdFldAttr IterCurr;
        XAie_RegBdFldAttr StepSize_Zero;
} XAie_AieMlAddressMode ;

/*
 * union captures multi dimension address generation properties between hardware
 * generations
 */
typedef union XAie_DmaBdMultiDimAddr {
        XAie_AieAddressMode AieMultiDimAddr;
        XAie_AieMlAddressMode AieMlMultiDimAddr;
} XAie_DmaBdMultiDimAddr;

/*
 * The typedef captures padding properties of buffer descriptor
 */
typedef struct XAie_DmaBdPad {
        XAie_RegBdFldAttr D0_PadBefore;
        XAie_RegBdFldAttr D0_PadAfter;
        XAie_RegBdFldAttr D1_PadBefore;
        XAie_RegBdFldAttr D1_PadBeforeHigh;
        XAie_RegBdFldAttr D1_PadAfter;
        XAie_RegBdFldAttr D1_PadAfterHigh;
        XAie_RegBdFldAttr D2_PadBefore;
        XAie_RegBdFldAttr D2_PadBeforeHigh;
        XAie_RegBdFldAttr D2_PadAfter;
        XAie_RegBdFldAttr D2_PadAfterHigh;
} XAie_DmaBdPad ;

/*
 * The typedef captures zero compression properties of aie
 */
typedef struct XAie_DmaBdCompression {
        XAie_RegBdFldAttr EnCompression;
} XAie_DmaBdCompression ;

/*
 * The typedef captures system level properties of DMA. This is applicable only
 * for SHIM DMA
 */
typedef struct XAie_DmaSysProp {
        XAie_RegBdFldAttr SMID;
        XAie_RegBdFldAttr BurstLen;
        XAie_RegBdFldAttr AxQos;
        XAie_RegBdFldAttr SecureAccess;
        XAie_RegBdFldAttr AxCache;
        XAie_RegBdFldAttr AxUser;
        XAie_RegBdFldAttr IOCoherence;
        XAie_RegBdFldAttr KeyIdx;
        XAie_RegBdFldAttr DataReuse;
} XAie_DmaSysProp ;

/*
 * The typedef captures all the buffer descriptor properties for AIE DMAs
 */
typedef struct XAie_DmaBdProp {
        u64 AddrMax;
        u8 AddrAlignMask;
        u8 AddrAlignShift;
        u8 LenActualOffset;
        u32 StepSizeMax;
        u16 WrapMax;
        u32 IterStepSizeMax;
        u8 IterWrapMax;
        u8 IterCurrMax;
        XAie_RegBdFldAttr BufferLen;
        const XAie_DmaBdBuffer *Buffer;
        const XAie_DmaBdDoubleBuffer *DoubleBuffer;
        const XAie_DmaBdLock *Lock;
        const XAie_DmaBdPkt *Pkt;
        const XAie_DmaBdEnProp *BdEn;
        const XAie_DmaBdMultiDimAddr *AddrMode;
        const XAie_DmaBdPad *Pad;
        const XAie_DmaBdCompression *Compression;
        const XAie_DmaSysProp *SysProp;
} XAie_DmaBdProp ;

typedef struct XAie_AieDmaChStatus {
        XAie_RegFldAttr Status;
        XAie_RegFldAttr StartQSize;
        XAie_RegFldAttr Stalled;
} XAie_AieDmaChStatus ;

typedef struct XAie_AieMlDmaChStatus {
        XAie_RegFldAttr Status;
        XAie_RegFldAttr ChannelRunning;
        XAie_RegFldAttr StalledLockRel;
        XAie_RegFldAttr StalledLockAcq;
        XAie_RegFldAttr StalledStreamStarve;
        XAie_RegFldAttr TaskQSize;
        XAie_RegFldAttr TaskQOverFlow;
        XAie_RegFldAttr StalledTCT;
} XAie_AieMlDmaChStatus ;

 typedef union XAie_DmaChStatus {
        XAie_AieDmaChStatus AieDmaChStatus;
        XAie_AieMlDmaChStatus AieMlDmaChStatus;
} XAie_DmaChStatus;

/*
 * The typedef contains the attributes of the Dma Channels
 */
typedef struct XAie_DmaChProp {
        u8 StartQSizeMax;
        u8 HasFoTMode;
        u8 HasControllerId;
        u8 HasEnCompression;
        u8 HasEnOutOfOrder;
        u8 MaxFoTMode;
        u32 MaxRepeatCount;
        XAie_RegBdFldAttr EnToken;
        XAie_RegBdFldAttr RptCount;
        XAie_RegBdFldAttr StartBd;
        XAie_RegBdFldAttr ControllerId;
        XAie_RegBdFldAttr EnCompression;
        XAie_RegBdFldAttr EnOutofOrder;
        XAie_RegBdFldAttr FoTMode;
        XAie_RegBdFldAttr Reset;
        XAie_RegBdFldAttr Enable;
        XAie_RegBdFldAttr PauseMem;
        XAie_RegBdFldAttr PauseStream;
        const XAie_DmaChStatus *DmaChStatus;
} XAie_DmaChProp;

/*
 * The typedef contains the attributes of the Dma Channels
 */
typedef struct XAie_DmaCustomChProp {
        u8 NumBds;
        u8 NumChannels;
        u32 BdBaseAddr;
        u32 ChCtrlBase;
        u32 ChStatusBase;
} XAie_DmaCustomChProp ;

/*
 * The typedef contains attributes of Dma Modules for AIE Tiles and Mem Tiles
 */
typedef struct XAie_DmaMod {
        u8  NumBds;
        u16  NumLocks;
        u8  ChIdxOffset;
        u8  NumAddrDim;
        u8  DoubleBuffering;
        u8  Compression;
        u8  Padding;
        u8  OutofOrderBdId;
        u8  InterleaveMode;
        u8  FifoMode;
        u8  EnTokenIssue;
        u8  RepeatCount;
        u8  TlastSuppress;
        u32 StartQueueBase;
        u32 BaseAddr;
        u32 IdxOffset;
        u32 ChCtrlBase;
        u32 ChCtrlMm2sBase;
        u8 NumChannels;
        u8 NumMm2sChannels;
        u32 ChStatusBase;
        u32 ChStatusOffset;
        u32 PadValueBase;
        const XAie_RegNocDmaPause *NocDmaPauseReg;
        const XAie_RegNocDmaOutsTxn *NocDmaOutstandingReg;
        const XAie_DmaCustomChProp *CtrlMm2sProp;
        const XAie_DmaCustomChProp *TraceS2mmProp;
        const XAie_DmaBdProp *BdProp;
        const XAie_DmaChProp *ChProp;
        void (*DmaBdInit)(XAie_DmaDesc *Desc);
        AieRC (*SetLock) (XAie_DmaDesc *Desc, XAie_Lock Acq,
                        XAie_Lock Rel, u8 AcqEn, u8 RelEn);
        AieRC (*SetIntrleave) (XAie_DmaDesc *Desc, u8 DoubleBuff,
                        u8 IntrleaveCount, u16 IntrleaveCurr);
        AieRC (*SetMultiDim) (XAie_DmaDesc *Desc, XAie_DmaTensor *Tensor);
        AieRC (*SetBdIter) (XAie_DmaDesc *Desc, u32 StepSize, u16 Wrap,
                        u8 IterCurr);
        AieRC (*WriteBd)(XAie_DevInst *DevInst, XAie_DmaDesc *Desc,
                        XAie_LocType Loc, u16 BdNum);
        AieRC (*ReadBd)(XAie_DevInst *DevInst, XAie_DmaDesc *Desc,
                        XAie_LocType Loc, u16 BdNum);
        AieRC (*WriteBdPvtBuffPool)(XAie_DevInst *DevInst, XAie_DmaDesc *Desc, XAie_LocType Loc,
                        u8 ChNum, XAie_DmaDirection Dir, u16 BdNum);
        AieRC (*ReadBdPvtBuffPool)(XAie_DevInst *DevInst, XAie_DmaDesc *Desc, XAie_LocType Loc,
                        u8 ChNum, XAie_DmaDirection Dir, u16 BdNum);
        AieRC (*PendingBd)(XAie_DevInst *DevInst, XAie_LocType Loc,
                        const XAie_DmaMod *DmaMod, u8 ChNum,
                        XAie_DmaDirection Dir, u8 *PendingBd);
        AieRC (*WaitforDone)(XAie_DevInst *DevINst, XAie_LocType Loc,
                        const XAie_DmaMod *DmaMod, u8 ChNum,
                        XAie_DmaDirection Dir, u32 TimeOutUs, u8 BusyPoll);
        AieRC (*WaitforBdTaskQueue)(XAie_DevInst *DevINst, XAie_LocType Loc,
                        const XAie_DmaMod *DmaMod, u8 ChNum,
                        XAie_DmaDirection Dir, u32 TimeOutUs, u8 BusyPoll);
        AieRC (*BdChValidity)(const XAie_DmaMod *DmaMod, XAie_DmaDirection Dir, u16 BdNum, u8 ChNum);
        AieRC (*UpdateBdLen)(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
                        XAie_LocType Loc, u32 Len, u16 BdNum);
        AieRC (*GetBdLen)(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
                        XAie_LocType Loc, u32 *Len, u16 BdNum);
        AieRC (*UpdateBdAddr)(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
                        XAie_LocType Loc, u64 Addr, u16 BdNum);
        AieRC (*UpdateBdLenPvtBuffPool)(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod, XAie_LocType Loc,
                        u8 ChNum, XAie_DmaDirection Dir, u32 Len, u16 BdNum);
        AieRC (*GetBdLenPvtBuffPool)(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod, XAie_LocType Loc,
                        u8 ChNum, XAie_DmaDirection Dir, u32 *Len, u16 BdNum);
        AieRC (*UpdateBdAddrPvtBuffPool)(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod, XAie_LocType Loc,
                        u8 ChNum, XAie_DmaDirection Dir, u64 Addr, u16 BdNum);
        AieRC (*GetChannelStatus)(XAie_DevInst *DevInst, XAie_LocType Loc,
                        const XAie_DmaMod *DmaMod, u8 ChNum,
                        XAie_DmaDirection Dir, u32 *Status);
        AieRC (*AxiBurstLenCheck)(u8 BurstLen, u8 *AxiBurstLen);
        AieRC (*NocDmaPause)(XAie_DevInst *DevInst, XAie_LocType *Loc, u8 ChNum,
                       XAie_DmaDirection Dir, u8 Pause, const struct XAie_DmaMod *DmaMod);
        AieRC (*GetNocDmaAxiMmOutstandingTxn)(XAie_DevInst *DevInst, XAie_LocType Loc,
                       const XAie_DmaMod *DmaMod, u32 *Status);
} XAie_DmaMod ;

/*
 * The typedef contains the attributes of Memory Module
 */
typedef struct XAie_MemMod {
        u32 Size;
        u32 MemAddr;
        u32 EccEvntRegOff;
        u32 EccScubPeriodRegOff;
} XAie_MemMod;
	
/*
 * The typedef contains the attributes of reset configuration
 */
typedef struct XAie_ShimRstMod {
        u32 RegOff;
        XAie_RegFldAttr RstCntr;
        XAie_RegFldAttr RstCntr_B; /* Control Application reset for Application B in Dual App mode */
        AieRC (*RstShims)(XAie_DevInst *DevInst, u32 StartCol, u32 NumCols);
} XAie_ShimRstMod ;

/*
 * The typedef contains the attributes of SHIM NOC AXI MM configuration
 */
typedef struct XAie_ShimNocAxiMMConfig {
        u32 RegOff;
        XAie_RegFldAttr NsuSlvErr;
        XAie_RegFldAttr NsuDecErr;
} XAie_ShimNocAxiMMConfig ;

/*
 * The typedef contains the attributes of SHIM clock buffer configuration
 */
typedef struct XAie_ShimClkBufCntr {
        u32 RegOff;
        u32 RstEnable;                  /* Reset value of enable bit */
        XAie_RegFldAttr ClkBufEnable;
} XAie_ShimClkBufCntr ;

/*
 * The typedef contains the attributes of Module_Clock_Control_0 configuration
 */
typedef struct XAie_ShimModClkCntr0 {
        u32 RegOff;
        u32 RstEnable;                  /* Reset value of enable bit */
        XAie_RegFldAttr StrmSwClkEnable;
        XAie_RegFldAttr PlIntClkEnable;
        XAie_RegFldAttr CteClkEnable;
        XAie_RegFldAttr UcClkEnable;
} XAie_ShimModClkCntr0 ;

/*
 * The typedef contains the attributes of Module_Clock_Control_1 configuration
 */
typedef struct XAie_ShimModClkCntr1 {
        u32 RegOff;
        u32 RstEnable;                  /* Reset value of enable bit */
        XAie_RegFldAttr NocModClkEnable;
} XAie_ShimModClkCntr1 ;

/*
 * The typedef contains attributes of PL interface module
 */
typedef struct XAie_PlIfMod {
        u32 UpSzrOff;
        u32 DownSzrOff;
        u32 DownSzrEnOff;
        u32 DownSzrByPassOff;
        u32 ShimNocNmuSwitchOff;
        u32 ColRstOff;
        u8  NumUpSzrPorts;
        u8  MaxByPassPortNum;
        u8  NumDownSzrPorts;
        const XAie_RegFldAttr   *UpSzr32_64Bit;
        const XAie_RegFldAttr *UpSzr128Bit;
        const XAie_RegFldAttr   *DownSzr32_64Bit;
        const XAie_RegFldAttr *DownSzr128Bit;
        const XAie_RegFldAttr *DownSzrEn;
        const XAie_RegFldAttr *DownSzrByPass;
        const XAie_RegFldAttr ShimNocNmuSwitch0;
        const XAie_RegFldAttr ShimNocNmuSwitch1;
        const XAie_ShimClkBufCntr *ClkBufCntr; /* Shim clock buffer control configuration */
        XAie_RegFldAttr ColRst; /* Tile column reset configuration */
        const XAie_ShimRstMod *ShimTileRst; /* SHIM tile reset enable configuration */
        const XAie_ShimNocAxiMMConfig *ShimNocAxiMM; /* SHIM NOC AXI MM configuration */
        const XAie_ShimModClkCntr0 *ModClkCntr0; /* Module_Clock_Control_0  configuration */
        const XAie_ShimModClkCntr1 *ModClkCntr1; /* Module_Clock_Control_1  configuration */
} XAie_PlIfMod;


/*
 * The typedef contains attributes of Noc module
 */
typedef struct XAie_NocMod {
        u32 ShimNocMuxOff;
        u32 ShimNocDeMuxOff;
        const XAie_RegFldAttr *ShimNocMux;
        const XAie_RegFldAttr *ShimNocDeMux;
        const XAie_ShimNocAxiMMConfig *ShimNocAxiMM; /* SHIM NOC AXI MM configuration */
} XAie_NocMod ;

/*
 * The typdef contains attributes of Lock modules.
 * The lock acquire and release mechanism for lock module are different across
 * hardware generations. In the first generation, the lock is acquired by
 * reading a register via AXI-MM path. The register address is specific to
 * the lock number and whether the lock is being acquired or released with
 * value. However, in subsequent generations, the lock access mechanism is
 * different. The register address to read from is computed differently.
 * To hide this change in the architecture, the information captured in the
 * below data strcuture does not use the register database directly. For more
 * details, please refer to the AI Engine hardware architecture specification
 * document.
 */
typedef struct XAie_LockMod {
        u8  NumLocks;           /* Number of lock in the module */
        s8  LockValUpperBound;  /* Upper bound of the lock value */
        s8  LockValLowerBound;  /* Lower bound of the lock value */
        u32 BaseAddr;           /* Base address of the lock module */
        u32 LockIdOff;          /* Offset between conseccutive locks */
        u32 RelAcqOff;          /* Offset between Release and Acquire locks */
        u32 LockValOff;         /* Offset thats added to the lock address for a value. */
        u32 LockSetValBase;     /* Base address of the register to set lock value */
        u32 LockSetValOff;      /* Offset between lock set value registers */
        const XAie_RegFldAttr *LockInit; /* Lock intialization reg attributes */
        AieRC (*Acquire)(XAie_DevInst *DevInst,
                        const struct XAie_LockMod *LockMod, XAie_LocType Loc,
                        XAie_Lock Lock, u32 TimeOut, u8 BusyPoll);
        AieRC (*Release)(XAie_DevInst *DevInst,
                        const struct XAie_LockMod *LockMod, XAie_LocType Loc,
                        XAie_Lock Lock, u32 TimeOut, u8 BusyPoll);
        AieRC (*SetValue)(XAie_DevInst *DevInst,
                        const struct XAie_LockMod *LockMod, XAie_LocType Loc,
                        XAie_Lock Lock);
        AieRC (*GetValue)(XAie_DevInst *DevInst,
                        const struct XAie_LockMod *LockMod, XAie_LocType Loc,
                        XAie_Lock Lock, u32 *LockValue);
} XAie_LockMod;

/* This typedef contains attributes of Performace Counter module */
typedef struct XAie_PerfMod {
        u8 MaxCounterVal;       /* Maximum counter value per module */
        u8 StartStopShift;      /* Shift for start stop perf ctrl reg */
        u8 ResetShift;          /* Shift for reset perf ctrl reg */
        u8 PerfCounterOffsetAdd;/* Add to calc perf cntrl offset for counter */
        u32 PerfCtrlBaseAddr;   /* Perf counter ctrl register offset address */
        u32 PerfCtrlOffsetAdd;  /* Add this val for next Perf counter ctrl reg*/
        u32 PerfResetOffsetAdd;  /* Add this val for next Perf reset reg*/
        u32 PerfCtrlResetBaseAddr;/* Perf counter ctrl offset addr for reset */
        u32 PerfCounterBaseAddr; /* Offset addr for perf counter 0 */
        u32 PerfCounterEvtValBaseAddr; /* Offset addr for perf counter evnt val*/
        u32 PerfCounterSsBaseAddr; /* Offset addr for perf counter snapshot*/
        u32 PerfCounterSsLoadEvttBaseAddr; /* Offset addr for perf counter snapshot load event*/
        const XAie_RegFldAttr Start; /* lsb and mask for start event for ctr0 */
        const XAie_RegFldAttr Stop; /* lsb and mask for stop event for ctr0 */
        const XAie_RegFldAttr Reset; /* lsb and mask for reset event for ctr0 */
} XAie_PerfMod;

typedef struct XAie_EventGroup {
        XAie_Events GroupEvent;
        u8 GroupOff;
        u32 GroupMask;
        u32 ResetValue;
} XAie_EventGroup;

/* structure to capture RscId to Events mapping */
typedef struct XAie_EventMap {
        u8 RscId;
        XAie_Events Event;
} XAie_EventMap;

/* This typedef contains attributes of Events module */
typedef struct XAie_EvntMod {
        const u16 *XAie_EventNumber;    /* Array of event numbers with true event val */
        u8  NumEventReg;
        XAie_Events EventMin;           /* number corresponding to evt 0 in the enum */
        XAie_Events EventMax;           /* number corresponding to last evt in enum */
        u32 ComboEventBase;
        u32 PerfCntEventBase;
        u32 UserEventBase;
        u32 PortIdleEventBase;
        u32 GenEventRegOff;
        XAie_RegFldAttr GenEvent;
        u32 ComboInputRegOff;
        u32 ComboEventMask;
        u8 ComboEventOff;
        u32 ComboCtrlRegOff;
        u32 ComboConfigMask;
        u8 ComboConfigOff;
        u32 BaseStrmPortSelectRegOff;
        u32 NumStrmPortSelectIds;
        u8 StrmPortSelectIdsPerReg;
        u32 PortIdMask;
        u8 PortIdOff;
        u32 PortMstrSlvMask;
        u8 PortMstrSlvOff;
        u32 Port32b512bMask;
        u8 Port32b512bOff;
        u32 BaseDmaChannelSelectRegOff;
        u8 NumDmaChannelSelectIds;
        u8 DmaChannelIdOff;
        u8 DmaChannelIdMask;
        u8 DmaChannelMM2SOff;
        u32 EdgeEventRegOff;
        XAie_RegFldAttr EdgeDetectEvent;
        XAie_RegFldAttr EdgeDetectTrigger;
        u32 EdgeEventSelectIdOff;
        u8 NumEdgeSelectIds;
        u32 BaseBroadcastRegOff;
        u8 NumBroadcastIds;
        u32 BaseBroadcastSwBlockRegOff;
        u32 BaseBroadcastSwUnblockRegOff;
        u8 BroadcastSwOff;
        u8 BroadcastSwBlockOff;
        u8 BroadcastSwUnblockOff;
        u8 NumSwitches;
        u32 BaseGroupEventRegOff;
        u8 NumGroupEvents;
        u8 GroupDmaRegSelect;
        u32 DefaultGroupErrorMask;
        const XAie_EventGroup *Group;
        u32 BasePCEventRegOff;
        u8 NumPCEvents;
        XAie_RegFldAttr PCAddr;
        XAie_RegFldAttr PCValid;
        u32 BaseStatusRegOff;
        u8 NumUserEvents;
        const XAie_EventMap *UserEventMap;
        const XAie_EventMap *PCEventMap;
        const XAie_EventMap *BroadcastEventMap;
        u32 ErrorHaltRegOff;
} XAie_EvntMod;

/* This typedef contains attributes of timer module */
typedef struct XAie_TimerMod {
        u32 TrigEventLowValOff;  /* Timer trigger evel low val register offset */
        u32 TrigEventHighValOff; /* Timer trigger evel high val register offset */
        u32 LowOff;              /* Timer low value Register offset */
        u32 HighOff;             /* Timer high value Register offset */
        u32 CtrlOff;             /* Timer control Register offset */
        const XAie_RegFldAttr CtrlReset; /* Timer control reset field */
        const XAie_RegFldAttr CtrlResetEvent; /* Timer control reset event field */
} XAie_TimerMod;

/* This structure captures all the attributes relevant to trace module */
typedef struct XAie_TraceMod {
        u32 CtrlRegOff;
        u32 PktConfigRegOff;
        u32 StatusRegOff;
        u32 *EventRegOffs;
        u8 NumTraceSlotIds;
        u8 NumEventsPerSlot;
        XAie_RegFldAttr StopEvent;
        XAie_RegFldAttr StartEvent;
        XAie_RegFldAttr ModeConfig;
        XAie_RegFldAttr PktType;
        XAie_RegFldAttr PktId;
        XAie_RegFldAttr State;
        XAie_RegFldAttr ModeSts;
        const XAie_RegFldAttr *Event;
} XAie_TraceMod;

/* This structure captures all attributes related to Clock Module */
typedef struct XAie_ClockMod {
        u32 ClockRegOff;
        const XAie_RegFldAttr NextTileClockCntrl;
} XAie_ClockMod;

/* This structure captures all attributes related to Control Packet Handler Module */
typedef struct XAie_CtrlPktHndlrMod {
        u64 CtrlPktHndlrRegOff;
        const XAie_RegFldAttr SlvErrOnAccess;
} XAie_CtrlPktHndlrMod;

/*
 * This structure captures all attributes related to first level interrupt
 * controller.
 */
typedef struct XAie_L1IntrMod {
        u32 BaseMaskRegOff;
        u32 BaseEnableRegOff;
        u32 BaseDisableRegOff;
        u32 BaseStatusRegOff;
        u32 BaseIrqRegOff;
        u32 BaseIrqEventRegOff;
        u32 BaseIrqEventMask;
        u32 BaseBroadcastBlockRegOff;
        u32 BaseBroadcastUnblockRegOff;
        u8 SwOff;
        u8 NumIntrIds;
        u8 NumIrqEvents;
        u8 IrqEventOff;
        u8 NumBroadcastIds;
        u8 MaxErrorBcIdsRvd;
        u8 (*IntrCtrlL1IrqId)(XAie_DevInst *DevInst, XAie_LocType Loc,
                XAie_BroadcastSw Switch);
} XAie_L1IntrMod;

/*
 * This structure captures all attributes related to second level interrupt
 * controller.
 */
typedef struct XAie_L2IntrMod {
        u32 MaskRegOff;
        u32 EnableRegOff;
        u32 DisableRegOff;
        u32 StatusRegOff;
        u32 IrqRegOff;
        u8 NumBroadcastIds;
        u8 NumNoCIntr;
        u8 MaxErrorBcIdsRvd;
} XAie_L2IntrMod;

/**
 * Kotesh - TODO: Considering HW Err Config is a privileged operation.
 * Need to check is this structure def is needed at all ?
 */

/*
 * This structure captures all attributes related to HW Error interrupt
 * controller.
 */
typedef struct XAie_HwErrIntrMod {
        u32 EnableRegOff;
        u32 DisableRegOff;
        u32 IrqRegOff;
        u8 NumNoCIntr;
} XAie_HwErrIntrMod;

/*
 * This typedef contains the attributes for Tile control Module
 */
typedef struct XAie_TileCtrlMod{
        u32 TileCtrlRegOff;
        u32 TileCtrlAxiRegOff;
        XAie_RegFldAttr IsolateEast;      /**< Isolate from east */
        XAie_RegFldAttr IsolateNorth;     /**< Isolate from north */
        XAie_RegFldAttr IsolateWest;      /**< Isolate from west */
        XAie_RegFldAttr IsolateSouth;     /**< Isolate from south */
        XAie_RegFldAttr IsolateAxiEast;   /**< Isolate Axi-MM from East */
        XAie_RegFldAttr IsolateAxiWest;   /**< Isolate Axi-MM from West */
        XAie_RegFldAttr DualAppControl;
        XAie_RegFldAttr L2SplitControl;
        u8  IsolateDefaultOn;
        u32 DualAppModeRegOff;
        u32 L2SplitRegOff;
} XAie_TileCtrlMod;

typedef struct XAie_AxiMMTileCtrlMod {
        u32 TileCtrlAxiMMRegOff;
        XAie_RegFldAttr AxiMMIsolateEast; /**< Isolate AXI-MM from east**/
        XAie_RegFldAttr AxiMMIsolateWest; /**< Isolate AXI-MM from west**/
} XAie_AxiMMTileCtrlMod;

/*
 * This typedef contains the attributes for memory control module
 */
typedef struct XAie_MemCtrlMod{
        u32 MemZeroisationCtrlRegOff;           /**< memory control reg offset for Zeroisation*/
        u32 MemPrivilegeCtrlRegOff;         /** <memory control reg for UC privilege memory> **/
        u32 MemInterleavingCtrlRegOff;      /**< memory control reg offset for Interleaving */
        XAie_RegFldAttr MemZeroisation; /**< memory zeroisation field */
        XAie_RegFldAttr MemInterleaving;        /**< memory interleaving field */
        XAie_RegFldAttr MemPrivilegeCtrl;  /** <UC privilege memory field>**/
} XAie_MemCtrlMod;

/*
 * This structure captures all attributes related to resource manager.
 */
typedef struct XAie_ResourceManager {
        u32 **Bitmaps;
} XAie_ResourceManager;

/*
 * This typedef contains all the modules for a Tile type
 */
typedef struct XAie_TileMod {
        const u8 NumModules;
        const XAie_CoreMod *CoreMod;
        const XAie_CoreIntMod *CoreIntMod;
        const XAie_StrmMod *StrmSw;
        const XAie_DmaMod  *DmaMod;
        const XAie_MemMod  *MemMod;
        const XAie_PlIfMod *PlIfMod;
        const XAie_LockMod *LockMod;
        const XAie_PerfMod *PerfMod;
        const XAie_EvntMod *EvntMod;
        const XAie_TimerMod *TimerMod;
        const XAie_TraceMod *TraceMod;
        const XAie_ClockMod *ClockMod;
        const XAie_L1IntrMod *L1IntrMod;
        const XAie_L2IntrMod *L2IntrMod;
        const XAie_TileCtrlMod *TileCtrlMod;
        const XAie_AxiMMTileCtrlMod *AxiMMTileCtrlMod;
        const XAie_MemCtrlMod *MemCtrlMod;
        const XAie_MemCtrlMod *MemCtrlMod_A;
        const XAie_MemCtrlMod *MemCtrlMod_B;
        const XAie_MemCtrlMod *MemCtrlInterLvMod;
        const XAie_MemCtrlMod *MemCtrlUcMod;
        const XAie_MemCtrlMod *MemCtrlUcMod_B;
        const XAie_UcMod *UcMod;
        const XAie_NocMod *NocMod;
        const XAie_StrmMod *StrmSw32b;
        const XAie_CtrlPktHndlrMod *CtrlPktHndlrMod;
} XAie_TileMod;


typedef struct XAie_DeviceOps {
        const u8 IsCheckerBoard;
        u32 *TilesInUse;
        u32 *MemInUse;
        u32 *CoreInUse;
        u8 (*GetTTypefromLoc)(XAie_DevInst *DevInst, XAie_LocType Loc);
        AieRC (*SetPartColShimReset)(XAie_DevInst *DevInst, u8 Enable);
        AieRC (*SetPartColClockAfterRst)(XAie_DevInst *DevInst, u8 Enable);
        AieRC (*SetPartIsolationAfterRst)(XAie_DevInst *DevInst, u8 IsolationFlags);
        AieRC (*SetAxiMMIsolation)(XAie_DevInst* DevInst, u8 IsolationFlags);
        AieRC (*PartMemZeroInit)(XAie_DevInst *DevInst);
        AieRC (*PartMemL2Split)(XAie_DevInst *DevInst);
        AieRC (*ZeroInitUcMem)(XAie_DevInst *DevInst);
        AieRC (*SetUCMemoryPrivileged)(XAie_DevInst *DevInst, u8 Enable);
        AieRC (*RequestTiles)(XAie_DevInst *DevInst,
                        XAie_BackendTilesArray *Args);
        AieRC (*const SetColumnClk)(XAie_DevInst *DevInst,
                        XAie_BackendColumnReq *Args);
        AieRC (*SetAppMode)(XAie_DevInst *DevInst, XAie_BackendTilesArray *Args);
} XAie_DeviceOps;

static inline u16 XAie_GetEventNumber(const struct XAie_EvntMod *EventMod, XAie_Events EventId)
{
        if ((EventMod ==NULL) ||
            (EventMod->XAie_EventNumber == NULL)) {
                return _XAIE_EVENT_INVALID;
        }
        if ((EventId < EventMod->EventMin) ||
            (EventId > EventMod->EventMax) ||
            ((EventId != EventMod->EventMin) && (EventMod->XAie_EventNumber[(u32)EventId] == 0))) {
                return _XAIE_EVENT_INVALID;
        }

        return EventMod->XAie_EventNumber[(u32)EventId];
}


//=============================xaiengine/xaie_dma.h

/*****************************************************************************/
/**
* @file xaie_dma.h
*
* Header file for dma functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   03/22/2020  Remove initial dma implemenatation
* 1.2   Tejus   03/22/2020  Dma apis for aie
* 1.3   Tejus   04/09/2020  Remove unused argument from interleave enable api
* 1.4   Tejus   06/05/2020  Add api to enable fifo mode.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

/**************************** Type Definitions *******************************/
/*
 * This enum captures the DMA Fifo Counters
 */
typedef enum class XAie_DmaFifoCounter {
	XAIE_DMA_FIFO_COUNTER_NONE = 0U,
	XAIE_DMA_FIFO_COUNTER_0 = 2U,
	XAIE_DMA_FIFO_COUNTER_1 = 3U,
} XAie_DmaFifoCounter;

/************************** Function Prototypes  *****************************/

/*****************************************************************************/
/**
*
* Macro to declare DMA queue configuration variable.
*
* @param	_Config: XAie DMA Queue configuration variable name
* @param	_StartBD: Start BD of the Queue
* @param	_RepeatCount: Repeat count
* @param	_EnTokenIssue: XAIE_ENABLE to issue token when completes,
*			otherwise disable.
* @param	_OutOfOrder: XAIE_ENABLE to indicate it is out of order mode,
*			otherwise it is not out of order. If out of order mode
*			is enabled, it will ignore the _StartBd setting.
* @return	None.
*
* @note		The macro declares @_Config as an XAie_DmaQueueDesc stack
*		variable.
*
*******************************************************************************/
#define _XAie_DmaDeclareQueueConfig(_Config, _StartBD, _RepeatCount, \
		_EnTokenIssue, _OutOfOrder) \
	XAie_DmaQueueDesc _Config; \
	{ \
		_Config.StartBd = (_StartBD); \
		_Config.RepeatCount = (_RepeatCount); \
		_Config.EnTokenIssue = (_EnTokenIssue); \
		_Config.OutOfOrder = (_OutOfOrder); \
	}




//=============================xaiengine/xaie_locks.h

/*****************************************************************************/
/**
* @file xaie_locks.h
*
* This file contains routines for AIE tile control.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   03/17/2020  Initial creation
* 1.1   Tejus   03/23/2020  Include xaiegbl header file
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
/**************************** Type Definitions *******************************/
/************************** Function Prototypes  *****************************/

//=============================xaie_helper.h

/*****************************************************************************/
/**
* @file xaie_helper.h
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

/***************************** Macro Definitions *****************************/
#define _CheckBit(bitmap, pos)   ((bitmap)[(u64)(pos) / (sizeof((bitmap)[0]) * 8U)] & \
				(u32)(1U << (u64)(pos) % (sizeof((bitmap)[0]) * 8U)))

#ifndef __SWIGINTERFACE__

#define _XAIE_ERROR(...)							      \
	do {								      \
		XAie_Log((FILE*)(uintptr_t)stderr, "[AIE ERROR]", __func__, __LINE__,	      \
				__VA_ARGS__);				      \
	} while(0)

#define _XAIE_WARN(...)							      \
	do {								      \
		XAie_Log((FILE*)(uintptr_t)stderr, "[AIE WARNING]", __func__, __LINE__,	      \
				__VA_ARGS__);				      \
	} while(0)

/**
* Note: Enable the definition of XAIE_FDEBUG macro to enable prints from aie-rt
*/
//#define XAIE_DEBUG 1

#ifdef XAIE_DEBUG

#define _XAIE_DBG(...)							      \
	do {								      \
		XAie_Log(stdout, "[AIE DEBUG]", __func__, __LINE__,	      \
				__VA_ARGS__);				      \
	} while(0)

#else

#define _XAIE_DBG(DevInst, ...) {}

#endif /* XAIE_DEBUG */

#else

// redirect XAIE_ERROR to printf
#define _XAIE_ERROR     printf

// no need for debug/warn printf so empty macro
#define _XAIE_DBG(...)   {}
#define _XAIE_WARN(...)  {}

#endif /* __SWIGINTERFACE__ */

/* Compute offset of field within a structure */
#define _XAIE_OFFSET_OF(structure, member) offsetof(structure, member)

/* Compute a pointer to a structure given a pointer to one of its fields */
#define _XAIE_CONTAINER_OF(ptr, structure, member) \
	 ((uintptr_t)(ptr) - XAIE_OFFSET_OF(structure, member))

/* Loop through the set bits in Value */
#define _for_each_set_bit(Index, Value, Len)				      \
	for((Index) = first_set_bit((Value)) - 1;			      \
	    (Index) < (Len);						      \
	    (Value) &= (Value) - 1, (Index) = first_set_bit((Value)) - 1)

/* Generate value with a set bit at given Index */
constexpr long long int _BIT(int Index) { return 1 << Index; }

/*as AIE address space is 32bit , the max valid bit index will be 31*/
constexpr int _MAX_VALID_AIE_REG_BIT_INDEX = 32;
constexpr int _MAX_VALID_U8_BIT_INDEX = 8;
constexpr int _MAX_VALID_U16_BIT_INDEX = 16;

/*the below Macros are to capture AIE4 features*/
constexpr int _NO_L1_INTERRUPT_SUPPORT = 1;		/* L1 interrupt is removed */
constexpr int _PERFORMANCE_SNAPSHOT_SUPPORT = 2;	/*performance snapshot registers are added */
constexpr int _BITMAP64_GROUPEVENT_SUPPORT = 3;	/*PL Groupevent contains 64bitmap in AIE4*/
constexpr int _NO_MEM_MOD_IN_AIE_TILE = 4;	/*core and mem module in AIE tile are combined*/


/*
 * __attribute is not supported for windows. remove it conditionally.
 */
#ifdef _MSC_VER
#define _XAIE_PACK_ATTRIBUTE
#else
#define _XAIE_PACK_ATTRIBUTE  __attribute__((packed, aligned(4)))
#endif

/* Data structure to capture the dma status */
typedef struct XAie_DmaStatus {
        u32 S2MMStatus;
        u32 MM2SStatus;
} XAie_DmaStatus;

/* Data structure to capture the core tile status */
typedef struct XAie_CoreTileStatus {
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
typedef struct XAie_MemTileStatus {
        XAie_DmaStatus *Dma;
        u32 *EventStatus;
        u8 *LockValue;
} XAie_MemTileStatus;

/* Data structure to capture the shim tile status */
typedef struct XAie_ShimTileStatus {
        XAie_DmaStatus *Dma;
        u32 *EventStatus;
        u8 *LockValue;
} XAie_ShimTileStatus;
/* Data structure to capture column status */
typedef struct XAie_ColStatus {
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
//=============================xaiengine/xaie_core.h

/*****************************************************************************/
/**
* @file xaie_core.h
*
* Header file for core control and wait functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   03/20/2020  Remove range apis
* 1.2   Tejus   06/01/2020  Add core debug halt apis
* 1.3   Tejus   06/01/2020  Add api to read core done bit.
* 1.4   Tejus   06/05/2020  Add api to reset/unreset aie cores.
* </pre>
*
******************************************************************************/

/************************** Constant Definitions *****************************/
constexpr u32 _XAIE_CORE_DEBUG_STATUS_ANY_HALT			= (1U << 0U);
constexpr u32 _XAIE_CORE_DEBUG_STATUS_PC_EVENT_HALT		= (1U << 1U);
constexpr u32 _XAIE_CORE_DEBUG_STATUS_MEM_STALL_HALT		= (1U << 2U);
constexpr u32 _XAIE_CORE_DEBUG_STATUS_LOCK_STALL_HALT		= (1U << 3U);
constexpr u32 _XAIE_CORE_DEBUG_STATUS_STREAM_STALL_HALT 	= (1U << 4U);
constexpr u32 _XAIE_CORE_DEBUG_STATUS_EVENT0_STALL_HALT 	= (1U << 5U);
constexpr u32 _XAIE_CORE_DEBUG_STATUS_EVENT1_STALL_HALT 	= (1U << 6U);

constexpr u32 _XAIE_CORE_STATUS_ENABLE				= (1U <<  0U);
constexpr u32 _XAIE_CORE_STATUS_RESET     			= (1U <<  1U);
constexpr u32 _XAIE_CORE_STATUS_MEM_STALL_S			= (1U <<  2U);
constexpr u32 _XAIE_CORE_STATUS_MEM_STALL_W			= (1U <<  3U);
constexpr u32 _XAIE_CORE_STATUS_MEM_STALL_N			= (1U <<  4U);
constexpr u32 _XAIE_CORE_STATUS_MEM_STALL_E			= (1U <<  5U);
constexpr u32 _XAIE_CORE_STATUS_LOCK_STALL_S			= (1U <<  6U);
constexpr u32 _XAIE_CORE_STATUS_LOCK_STALL_W			= (1U <<  7U);
constexpr u32 _XAIE_CORE_STATUS_LOCK_STALL_N			= (1U <<  8U);
constexpr u32 _XAIE_CORE_STATUS_LOCK_STALL_E			= (1U <<  9U);
constexpr u32 _XAIE_CORE_STATUS_STREAM_STALL_SS0  		= (1U << 10U);
constexpr u32 _XAIE_CORE_STATUS_STREAM_STALL_SS1  		= (1U << 11U);
constexpr u32 _XAIE_CORE_STATUS_STREAM_STALL_MS0  		= (1U << 12U);
constexpr u32 _XAIE_CORE_STATUS_STREAM_STALL_MS1  		= (1U << 13U);
constexpr u32 _XAIE_CORE_STATUS_CASCADE_STALL_SCD 		= (1U << 14U);
constexpr u32 _XAIE_CORE_STATUS_CASCADE_STALL_MCD 		= (1U << 15U);
constexpr u32 _XAIE_CORE_STATUS_DEBUG_HALT        		= (1U << 16U);
constexpr u32 _XAIE_CORE_STATUS_ECC_ERROR_STALL        	= (1U << 17U);
constexpr u32 _XAIE_CORE_STATUS_ECC_SCRUBBING_STALL      	= (1U << 18U);
constexpr u32 _XAIE_CORE_STATUS_ERROR_HALT        		= (1U << 19U);
constexpr u32 _XAIE_CORE_STATUS_DONE              		= (1U << 20U);
constexpr u32 _XAIE_CORE_STATUS_PROCESSOR_BUS_STALL      	= (1U << 21U);

/*****************************************************************************/
/*
*
* This API checks for the event due to which AIE was debug halted.
*
* @param	DebugStatus: Value of debug status register.
* @param	DebugEventMask: Debug Event Mask.
*		Any of XAIE_CORE_DEBUG_STATUS_*_HALT macros.
* @return	1 on success. 0 of failure.
*
* @note		None.
*
******************************************************************************/
static inline u32 XAie_CheckDebugHaltStatus(u32 DebugStatus, u32 DebugEventMask)
{
	return ((DebugStatus & DebugEventMask) != 0U) ? 1U : 0U;
}

/*****************************************************************************/
/*
*
* This API checks for the state of the indiviual bits in core status register.
*
* @param	CoreStatus: Value of core status register.
* @param	CoreStatMask: Core Event Mask.
*		Any of XAIE_CORE_STATUS_* macros.
* @return	1 if the bit is set. 0 otherwise.
*
* @note		None.
*
******************************************************************************/
static inline u32 XAie_CheckCoreStatus(u32 CoreStatus, u32 CoreStatMask)
{
	return ((CoreStatus & CoreStatMask) != 0U) ? 1U : 0U;
}
/************************** Function Prototypes  *****************************/
//=============================xaiengine/xaie_elfloader.h

/*****************************************************************************/
/**
* @file xaie_elfloader.h
*
* Header file for core elf loader functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   03/20/2020  Remove range apis
* 1.2   Tejus   05/26/2020  Add API to load elf from memory.
* </pre>
*
******************************************************************************/

/************************** Constant Definitions *****************************/
constexpr u32 _XAIE_LOAD_ELF_TXT = (1U << 0U);
constexpr u32 _XAIE_LOAD_ELF_BSS = (1U << 1U);
constexpr u32 _XAIE_LOAD_ELF_DATA = (1U << 2U);
constexpr u32 _XAIE_LOAD_ELF_ALL = (_XAIE_LOAD_ELF_TXT | _XAIE_LOAD_ELF_BSS | 
					_XAIE_LOAD_ELF_DATA);

/************************** Variable Definitions *****************************/
typedef struct XAieSim_StackSz {
	u32 start;	/**< Stack start address */
	u32 end;	/**< Stack end address */
} XAieSim_StackSz;

/************************** Function Prototypes  *****************************/

//=============================xaiengine/xaie_interrupt.h

/*****************************************************************************/
/**
* @file xaie_interrupt.h
*
* Header file for AIE interrupt module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Nishad  07/21/2020  Initial creation
* 1.1   Nishad  07/23/2020  Add APIs to configure second level interrupt
*			    controller.
* 1.2   Nishad  07/23/2020  Add API to initialize error broadcast network.
* 1.3   Nishad  08/13/2020  Add macro for error broadcast mask.
* </pre>
*
******************************************************************************/

/**************************** Type Definitions *******************************/
constexpr u32 _XAIE_ERROR_BROADCAST_ID = 0x0U;
constexpr u32 _XAIE_ERROR_BROADCAST_MASK = 0x1U;

constexpr u32 _XAIE_ERROR_BROADCAST_ID_UC_EVENT = 0x1U;
constexpr u32 _XAIE_ERROR_BROADCAST_ID_USER_EVENT1 = 0x2U;

constexpr u32 _XAIE_ERROR_SHIM_INTR_ID = 0x10U;
constexpr u32 _XAIE_ERROR_SHIM_INTR_MASK = 0x10000U;
constexpr u32 _XAIE_ERROR_NPI_INTR_ID = 0x1U;

constexpr u32 _XAIE_ERROR_L1_ENABLE = 0x3FU;
#if ((XAIE_DEV_SINGLE_GEN != XAIE_DEV_GEN_AIE4_GENERIC) && \
	(XAIE_DEV_SINGLE_GEN != XAIE_DEV_GEN_AIE4) && \
	  (XAIE_DEV_SINGLE_GEN != XAIE_DEV_GEN_AIE4_A))
constexpr u32 _XAIE_ERROR_L2_ENABLE = 0x3FU;
#else
/**
 * (Kotesh)TODO : Fix it as per AIE4 requirement.
 * For now setting it to enable all the 17 irqs to L2 controller
 * Bit 0-15  -> 16 Broadcast Channels
 * Bit 16    -> 1 uC interrupt input.
*/
constexpr u32 _XAIE_ERROR_L2_ENABLE = 0x1FFFFU;
#endif
 

//=============================xaiengine/xaie_mem.h

/*****************************************************************************/
/**
* @file xaie_mem.h
*
* Header file for data memory implementations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   03/20/2020  Remove range apis
* 1.2   Nishad  07/30/2020  Add API to read and write block of data from tile
*			    data memory.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

/***************************** Macro Definitions *****************************/
constexpr u32 _XAIE_MEM_WORD_ALIGN_SHIFT = 2U;
constexpr u32 _XAIE_MEM_WORD_ALIGN_MASK = ((1U << _XAIE_MEM_WORD_ALIGN_SHIFT) - 1U);
constexpr u32 _XAIE_MEM_WORD_ALIGN_SIZE = (1U << _XAIE_MEM_WORD_ALIGN_SHIFT);
constexpr u32 _XAIE_MEM_WORD_LAST_BYTE = 8U;
constexpr u32 _XAIE_MEM_WORD_LAST_BYTE_MASK = ((1U << _XAIE_MEM_WORD_LAST_BYTE) - 1U);

#define _XAIE_MEM_WORD_ROUND_UP(Addr)	(((Addr) + XAIE_MEM_WORD_ALIGN_MASK) & \
						~XAIE_MEM_WORD_ALIGN_MASK)
#define _XAIE_MEM_WORD_ROUND_DOWN(Addr)	((Addr) & (~XAIE_MEM_WORD_ALIGN_MASK))

/************************** Function Prototypes  *****************************/


//=============================xaiengine/xaie_perfcnt.h

/*****************************************************************************/
/**
* @file xaie_perfcnt.h
*
* Header file for performance counter implementations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- ------   -------- -----------------------------------------------------
* 1.0   Dishita  11/21/2019  Initial creation
* </pre>
*
******************************************************************************/

/************************** Function Prototypes  *****************************/

//=============================xaiengine/xaie_plif.h

/*****************************************************************************/
/**
* @file xaie_plif.h
*
* This file contains routines for AIE tile control.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   10/28/2019  Initial creation
* 1.1   Tejus   03/16/2020  Implementation of apis for Mux/Demux configuration
* 1.2   Tejus   03/20/2020  Remove range apis
* </pre>
*
******************************************************************************/

/**************************** Type Definitions *******************************/
/*
 * This enum captures the AIE-PL interface bit widths available in the hardware.
 */
typedef enum class XAie_PlIfWidth {
	PLIF_WIDTH_32 = 32,
	PLIF_WIDTH_64 = 64,
	PLIF_WIDTH_128 = 128
} XAie_PlIfWidth;

/************************** Function Prototypes  *****************************/
//=============================xaiengine/xaie_reset.h

/*****************************************************************************/
/**
* @file xaie_reset.h
*
* This file contains routines for AI engine resets.
*
******************************************************************************/

/**************************** Type Definitions *******************************/
/************************** Function Prototypes  *****************************/

//=============================xaiengine/xaie_ss.h

/*****************************************************************************/
/**
* @file xaie_ss.h
*
* Header file for stream switch implementations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   03/20/2020  Remove range apis
* 1.2   Tejus   03/21/2020  Add stream switch packet switch mode apis
* </pre>
*
******************************************************************************/


/**************************** Type Definitions *******************************/
/* Typedef to capture Packet drop header */
typedef enum class XAie_StrmSwPktHeader {
	XAIE_SS_PKT_DONOT_DROP_HEADER,
	XAIE_SS_PKT_DROP_HEADER
} XAie_StrmSwPktHeader;

/************************** Function Prototypes  *****************************/

//=============================xaiengine/xaie_timer.h

/*****************************************************************************/
/**
* @file xaie_timer.h
*
* Header file for timer implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- ------   -------- -----------------------------------------------------
* 1.0   Dishita  04/05/2020  Initial creation
* </pre>
*
******************************************************************************/

/************************** Enum *********************************************/

/************************** Function Prototypes  *****************************/

//=============================xaiengine/xaie_trace.h

/*****************************************************************************/
/**
* @file xaie_trace.h
*
* Header file for AIE trace module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Nishad  06/16/2020  Initial creation
* </pre>
*
******************************************************************************/

/**************************** Type Definitions *******************************/
/* This enum captures various states of a trace module */
typedef enum class XAie_TraceState {
	XAIE_TRACE_IDLE,
	XAIE_TRACE_RUNNING,
	XAIE_TRACE_OVERRUN,
} XAie_TraceState;

/* This enum captures various trace modes */
typedef enum class XAie_TraceMode {
	XAIE_TRACE_EVENT_TIME,
	XAIE_TRACE_EVENT_PC,
	XAIE_TRACE_INST_EXEC,
} XAie_TraceMode;
/************************** Function Prototypes  *****************************/

//=============================xaiengine/xaie_noc.h

/*****************************************************************************/
/**
* @file xaie_noc.h
*
* This file contains routines for Noc module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Sandip   12/11/2023  Initial creation
* </pre>
*
******************************************************************************/

/**************************** Type Definitions *******************************/


/************************** Function Prototypes  *****************************/
//=============================

} // namespace aie_codegen
#endif		/* end of protection macro AIE_CODEGEN_HPP */
