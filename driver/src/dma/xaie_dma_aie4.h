/******************************************************************************
* Copyright (C) 2024 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_dma_aie4.h
* @{
*
* This file contains routines for AIE4 DMA configuration and controls. This
* header file is not exposed to the user.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who        Date        Changes
* ----- ------     --------    -----------------------------------------------------
* 1.0   jbaniset   22/01/2024  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_DMA_AIE4_H
#define XAIE_DMA_AIE4_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"

/**************************** Type Definitions *******************************/
typedef struct XAie4_MemTileDmaBdChMap {
	u16 BdNum;
	u8 Dir;
	u8 ChNum;
} XAie4_MemTileDmaBdChMap;

/************************** Function Prototypes  *****************************/
void _XAie4_ShimDmaInit(XAie_DmaDesc* Desc);
void _XAie4_TileDmaInit(XAie_DmaDesc* Desc);
void _XAie4_MemTileDmaInit(XAie_DmaDesc* Desc);
u64 _XAie4_DmaGetChannelCtrlAddr(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
				      XAie_LocType Loc, XAie_DmaDirection Dir, u8 ChNum);

AieRC _XAie4_DmaSetMultiDim(XAie_DmaDesc *DmaDesc, XAie_DmaTensor *Tensor);
AieRC _XAie4_DmaGetPendingBdCount(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u8 *PendingBd);
AieRC _XAie4_DmaWaitForDone(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u32 TimeOutUs, u8 BusyPoll);
AieRC _XAie4_DmaWaitForBdTaskQueue(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u32 TimeOutUs, u8 BusyPoll);
AieRC _XAie4_ShimTileDmaCheckBdChValidity(u8 BdNum, u8 ChNum);
AieRC _XAie4_DmaGetChannelStatus(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u32 *Status);

AieRC _XAie4_TileDmaUpdateBdLen(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u32 Len, u16 BdNum);
AieRC _XAie4_MemTileDmaUpdateBdLenPvtBuffPool(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u32 Len, u16 BdNum);
AieRC _XAie4_ShimTileDmaUpdateBdLen(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u32 Len, u16 BdNum);
AieRC _XAie4_ShimTileDmaUpdateBdLenPvtBuffPool(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u32 Len, u16 BdNum);

AieRC _XAie4_TileDmaGetBdLen(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u32 *Len, u16 BdNum);
AieRC _XAie4_MemTileDmaGetBdLenPvtBuffPool(XAie_DevInst *DevInst,const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u32 *Len, u16 BdNum);
AieRC _XAie4_ShimTileDmaGetBdLen(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u32 *Len, u16 BdNum);
AieRC _XAie4_ShimTileDmaGetBdLenPvtBuffPool(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u32 *Len, u16 BdNum);

AieRC _XAie4_TileDmaUpdateBdAddr(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u64 Addr, u16 BdNum);
AieRC _XAie4_MemTileDmaUpdateBdAddrPvtBuffPool(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u64 Addr, u16 BdNum);
AieRC _XAie4_ShimTileDmaUpdateBdAddr(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u64 Addr, u16 BdNum);
AieRC _XAie4_ShimTileDmaUpdateBdAddrPvtBuffPool(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u64 Addr, u16 BdNum);

AieRC _XAie4_TileDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u16 BdNum);
AieRC _XAie4_MemTileDmaWriteBdPvtBuffPool(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u16 BdNum);
AieRC _XAie4_ShimDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u16 BdNum);
AieRC _XAie4_ShimDmaWriteBdPvtBuffPool(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u16 BdNum);

AieRC _XAie4_TileDmaReadBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u16 BdNum);
AieRC _XAie4_MemTileDmaReadBdPvtBuffPool(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u16 BdNum);
AieRC _XAie4_ShimDmaReadBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u16 BdNum);
AieRC _XAie4_ShimDmaReadBdPvtBuffPool(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u16 BdNum);
		
AieRC _XAie4_DmaSetBdIteration(XAie_DmaDesc *DmaDesc, u32 StepSize, u16 Wrap,
		u8 IterCurr);
AieRC _XAie4_AxiBurstLenCheck(u8 BurstLen);

#endif /* XAIE_DMA_AIE4_H */
/** @} */
