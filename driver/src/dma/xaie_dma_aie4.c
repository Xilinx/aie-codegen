/******************************************************************************
 * Copyright (C) 2024 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/


/*****************************************************************************/
/**
 * @file xaie_dma_aie4.c
 * @{
 *
 * This file contains routines for AIE4 DMA configuration and controls.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who        Date     Changes
 * ----- ------     -------- -----------------------------------------------------
 * 1.0   jbaniset   22/01/2024  Initial creation
 ** </pre>
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaiegbl_regdef.h"
#include "xaie_dma_aie4.h"

#ifdef XAIE_FEATURE_DMA_ENABLE

#include "xaie_helper_internal.h"
/************************** Constant Definitions *****************************/
#define XAIE_DMA_32BIT_TXFER_LEN			2U

#define XAIE4_TILEDMA_NUM_BD_WORDS			6U
#define XAIE4_SHIMDMA_NUM_BD_WORDS			9U
#define XAIE4_MEMTILEDMA_NUM_BD_WORDS		11U
#define XAIE4_DMA_STEPSIZE_DEFAULT			1U
#define XAIE4_DMA_ITERWRAP_DEFAULT			1U
#define XAIE4_DMA_PAD_NUM_DIMS				3U

#define XAIE4_DMA_STATUS_IDLE				0x0U
#define XAIE4_DMA_STATUS_CHANNEL_NOT_RUNNING		0x0U
#define XAIE4_DMA_STATUS_CHNUM_OFFSET			0x4U
#define XAIE4_DMA_STATUS_TASK_Q_SIZE_MSB	24

XAie4_MemTileDmaBdChMap XAie4_MemTileDmaBdChLut[320] = {
	/* 0  -  15 */
	{0,DMA_S2MM, 0},{1,DMA_S2MM, 0},{2,DMA_S2MM, 0},{3,DMA_S2MM, 0},{4,DMA_S2MM, 0},{5,DMA_S2MM, 0},{6,DMA_S2MM, 0},{7,DMA_S2MM, 0},{8,DMA_S2MM, 0},{9,DMA_S2MM, 0},{10,DMA_S2MM, 0},{11,DMA_S2MM, 0},{12,DMA_S2MM, 0},{13,DMA_S2MM, 0},{14,DMA_S2MM, 0},{15,DMA_S2MM, 0},
	/* 16  -  31 */
	{0,DMA_S2MM, 1},{1,DMA_S2MM, 1},{2,DMA_S2MM, 1},{3,DMA_S2MM, 1},{4,DMA_S2MM, 1},{5,DMA_S2MM, 1},{6,DMA_S2MM, 1},{7,DMA_S2MM, 1},{8,DMA_S2MM, 1},{9,DMA_S2MM, 1},{10,DMA_S2MM, 1},{11,DMA_S2MM, 1},{12,DMA_S2MM, 1},{13,DMA_S2MM, 1},{14,DMA_S2MM, 1},{15,DMA_S2MM, 1},
	/* 32  -  47 */
	{0,DMA_S2MM, 2},{1,DMA_S2MM, 2},{2,DMA_S2MM, 2},{3,DMA_S2MM, 2},{4,DMA_S2MM, 2},{5,DMA_S2MM, 2},{6,DMA_S2MM, 2},{7,DMA_S2MM, 2},{8,DMA_S2MM, 2},{9,DMA_S2MM, 2},{10,DMA_S2MM, 2},{11,DMA_S2MM, 2},{12,DMA_S2MM, 2},{13,DMA_S2MM, 2},{14,DMA_S2MM, 2},{15,DMA_S2MM, 2},
	/* 48  -  63 */
	{0,DMA_S2MM, 3},{1,DMA_S2MM, 3},{2,DMA_S2MM, 3},{3,DMA_S2MM, 3},{4,DMA_S2MM, 3},{5,DMA_S2MM, 3},{6,DMA_S2MM, 3},{7,DMA_S2MM, 3},{8,DMA_S2MM, 3},{9,DMA_S2MM, 3},{10,DMA_S2MM, 3},{11,DMA_S2MM, 3},{12,DMA_S2MM, 3},{13,DMA_S2MM, 3},{14,DMA_S2MM, 3},{15,DMA_S2MM, 3},
	/* 64  -  79 */
	{0,DMA_S2MM, 4},{1,DMA_S2MM, 4},{2,DMA_S2MM, 4},{3,DMA_S2MM, 4},{4,DMA_S2MM, 4},{5,DMA_S2MM, 4},{6,DMA_S2MM, 4},{7,DMA_S2MM, 4},{8,DMA_S2MM, 4},{9,DMA_S2MM, 4},{10,DMA_S2MM, 4},{11,DMA_S2MM, 4},{12,DMA_S2MM, 4},{13,DMA_S2MM, 4},{14,DMA_S2MM, 4},{15,DMA_S2MM, 4},
	/* 80  -  95 */
	{0,DMA_S2MM, 5},{1,DMA_S2MM, 5},{2,DMA_S2MM, 5},{3,DMA_S2MM, 5},{4,DMA_S2MM, 5},{5,DMA_S2MM, 5},{6,DMA_S2MM, 5},{7,DMA_S2MM, 5},{8,DMA_S2MM, 5},{9,DMA_S2MM, 5},{10,DMA_S2MM, 5},{11,DMA_S2MM, 5},{12,DMA_S2MM, 5},{13,DMA_S2MM, 5},{14,DMA_S2MM, 5},{15,DMA_S2MM, 5},
	/* 96  -  111 */
	{0,DMA_S2MM, 6},{1,DMA_S2MM, 6},{2,DMA_S2MM, 6},{3,DMA_S2MM, 6},{4,DMA_S2MM, 6},{5,DMA_S2MM, 6},{6,DMA_S2MM, 6},{7,DMA_S2MM, 6},{8,DMA_S2MM, 6},{9,DMA_S2MM, 6},{10,DMA_S2MM, 6},{11,DMA_S2MM, 6},{12,DMA_S2MM, 6},{13,DMA_S2MM, 6},{14,DMA_S2MM, 6},{15,DMA_S2MM, 6},
	/* 112  -  127 */
	{0,DMA_S2MM, 7},{1,DMA_S2MM, 7},{2,DMA_S2MM, 7},{3,DMA_S2MM, 7},{4,DMA_S2MM, 7},{5,DMA_S2MM, 7},{6,DMA_S2MM, 7},{7,DMA_S2MM, 7},{8,DMA_S2MM, 7},{9,DMA_S2MM, 7},{10,DMA_S2MM, 7},{11,DMA_S2MM, 7},{12,DMA_S2MM, 7},{13,DMA_S2MM, 7},{14,DMA_S2MM, 7},{15,DMA_S2MM, 7},
	/* 128  -  143 */
	{0,DMA_MM2S, 0},{1,DMA_MM2S, 0},{2,DMA_MM2S, 0},{3,DMA_MM2S, 0},{4,DMA_MM2S, 0},{5,DMA_MM2S, 0},{6,DMA_MM2S, 0},{7,DMA_MM2S, 0},{8,DMA_MM2S, 0},{9,DMA_MM2S, 0},{10,DMA_MM2S, 0},{11,DMA_MM2S, 0},{12,DMA_MM2S, 0},{13,DMA_MM2S, 0},{14,DMA_MM2S, 0},{15,DMA_MM2S, 0},
	/* 144  -  159 */
	{0,DMA_MM2S, 1},{1,DMA_MM2S, 1},{2,DMA_MM2S, 1},{3,DMA_MM2S, 1},{4,DMA_MM2S, 1},{5,DMA_MM2S, 1},{6,DMA_MM2S, 1},{7,DMA_MM2S, 1},{8,DMA_MM2S, 1},{9,DMA_MM2S, 1},{10,DMA_MM2S, 1},{11,DMA_MM2S, 1},{12,DMA_MM2S, 1},{13,DMA_MM2S, 1},{14,DMA_MM2S, 1},{15,DMA_MM2S, 1},
	/* 160  -  175 */
	{0,DMA_MM2S, 2},{1,DMA_MM2S, 2},{2,DMA_MM2S, 2},{3,DMA_MM2S, 2},{4,DMA_MM2S, 2},{5,DMA_MM2S, 2},{6,DMA_MM2S, 2},{7,DMA_MM2S, 2},{8,DMA_MM2S, 2},{9,DMA_MM2S, 2},{10,DMA_MM2S, 2},{11,DMA_MM2S, 2},{12,DMA_MM2S, 2},{13,DMA_MM2S, 2},{14,DMA_MM2S, 2},{15,DMA_MM2S, 2},
	/* 176  -  191 */
	{0,DMA_MM2S, 3},{1,DMA_MM2S, 3},{2,DMA_MM2S, 3},{3,DMA_MM2S, 3},{4,DMA_MM2S, 3},{5,DMA_MM2S, 3},{6,DMA_MM2S, 3},{7,DMA_MM2S, 3},{8,DMA_MM2S, 3},{9,DMA_MM2S, 3},{10,DMA_MM2S, 3},{11,DMA_MM2S, 3},{12,DMA_MM2S, 3},{13,DMA_MM2S, 3},{14,DMA_MM2S, 3},{15,DMA_MM2S, 3},
	/* 192  -  207 */
	{0,DMA_MM2S, 4},{1,DMA_MM2S, 4},{2,DMA_MM2S, 4},{3,DMA_MM2S, 4},{4,DMA_MM2S, 4},{5,DMA_MM2S, 4},{6,DMA_MM2S, 4},{7,DMA_MM2S, 4},{8,DMA_MM2S, 4},{9,DMA_MM2S, 4},{10,DMA_MM2S, 4},{11,DMA_MM2S, 4},{12,DMA_MM2S, 4},{13,DMA_MM2S, 4},{14,DMA_MM2S, 4},{15,DMA_MM2S, 4},
	/* 208  -  223 */
	{0,DMA_MM2S, 5},{1,DMA_MM2S, 5},{2,DMA_MM2S, 5},{3,DMA_MM2S, 5},{4,DMA_MM2S, 5},{5,DMA_MM2S, 5},{6,DMA_MM2S, 5},{7,DMA_MM2S, 5},{8,DMA_MM2S, 5},{9,DMA_MM2S, 5},{10,DMA_MM2S, 5},{11,DMA_MM2S, 5},{12,DMA_MM2S, 5},{13,DMA_MM2S, 5},{14,DMA_MM2S, 5},{15,DMA_MM2S, 5},
	/* 224  -  239 */
	{0,DMA_MM2S, 6},{1,DMA_MM2S, 6},{2,DMA_MM2S, 6},{3,DMA_MM2S, 6},{4,DMA_MM2S, 6},{5,DMA_MM2S, 6},{6,DMA_MM2S, 6},{7,DMA_MM2S, 6},{8,DMA_MM2S, 6},{9,DMA_MM2S, 6},{10,DMA_MM2S, 6},{11,DMA_MM2S, 6},{12,DMA_MM2S, 6},{13,DMA_MM2S, 6},{14,DMA_MM2S, 6},{15,DMA_MM2S, 6},
	/* 240  -  255 */
	{0,DMA_MM2S, 7},{1,DMA_MM2S, 7},{2,DMA_MM2S, 7},{3,DMA_MM2S, 7},{4,DMA_MM2S, 7},{5,DMA_MM2S, 7},{6,DMA_MM2S, 7},{7,DMA_MM2S, 7},{8,DMA_MM2S, 7},{9,DMA_MM2S, 7},{10,DMA_MM2S, 7},{11,DMA_MM2S, 7},{12,DMA_MM2S, 7},{13,DMA_MM2S, 7},{14,DMA_MM2S, 7},{15,DMA_MM2S, 7},
	/* 256  -  271 */
	{0,DMA_MM2S, 8},{1,DMA_MM2S, 8},{2,DMA_MM2S, 8},{3,DMA_MM2S, 8},{4,DMA_MM2S, 8},{5,DMA_MM2S, 8},{6,DMA_MM2S, 8},{7,DMA_MM2S, 8},{8,DMA_MM2S, 8},{9,DMA_MM2S, 8},{10,DMA_MM2S, 8},{11,DMA_MM2S, 8},{12,DMA_MM2S, 8},{13,DMA_MM2S, 8},{14,DMA_MM2S, 8},{15,DMA_MM2S, 8},
	/* 272  -  287 */
	{0,DMA_MM2S, 9},{1,DMA_MM2S, 9},{2,DMA_MM2S, 9},{3,DMA_MM2S, 9},{4,DMA_MM2S, 9},{5,DMA_MM2S, 9},{6,DMA_MM2S, 9},{7,DMA_MM2S, 9},{8,DMA_MM2S, 9},{9,DMA_MM2S, 9},{10,DMA_MM2S, 9},{11,DMA_MM2S, 9},{12,DMA_MM2S, 9},{13,DMA_MM2S, 9},{14,DMA_MM2S, 9},{15,DMA_MM2S, 9},
	/* 288  -  303 */
	{0,DMA_MM2S,10},{1,DMA_MM2S,10},{2,DMA_MM2S,10},{3,DMA_MM2S,10},{4,DMA_MM2S,10},{5,DMA_MM2S,10},{6,DMA_MM2S,10},{7,DMA_MM2S,10},{8,DMA_MM2S,10},{9,DMA_MM2S,10},{10,DMA_MM2S,10},{11,DMA_MM2S,10},{12,DMA_MM2S,10},{13,DMA_MM2S,10},{14,DMA_MM2S,10},{15,DMA_MM2S,10},
	/* 304  -  319 */
	{0,DMA_MM2S,11},{1,DMA_MM2S,11},{2,DMA_MM2S,11},{3,DMA_MM2S,11},{4,DMA_MM2S,11},{5,DMA_MM2S,11},{6,DMA_MM2S,11},{7,DMA_MM2S,11},{8,DMA_MM2S,11},{9,DMA_MM2S,11},{10,DMA_MM2S,11},{11,DMA_MM2S,11},{12,DMA_MM2S,11},{13,DMA_MM2S,11},{14,DMA_MM2S,11},{15,DMA_MM2S,11},
};

/*********************** Static Function Definitions *************************/
/*****************************************************************************/
/**
 *
 * This API checks the validity of wrap and padding before and after fields of
 * the Dma descriptor.
 *
 * @param	DmaDesc: Dma Descriptor
 *
 * @return	XAIE_OK on success, XAIE_INVALID_DMA_DESC on failure
 *
 * @note		Internal Only.
 *		If D0_wrap == 0:
 *			D1/D2 padding after/before must be 0
 *			D0 padding after must be 0
 *		If D1_wrap == 0:
 *			D2 padding after/before must be 0
 *			D1 padding after must be 0
 *		If D2_wrap == 0:
 *			D2 padding after must be 0
 *
 ******************************************************************************/
static AieRC _XAie4_DmaMemTileCheckPaddingConfig(XAie_DmaDesc *DmaDesc)
{
	XAie_AieMlDimDesc *DDesc = DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc;
	XAie_PadDesc *PDesc = DmaDesc->PadDesc;

	for(u8 Dim = 0U; Dim < XAIE4_DMA_PAD_NUM_DIMS; Dim++) {
		if(DDesc[Dim].Wrap == 0U) {

			if(PDesc[Dim].After != 0U) {
				XAIE_ERROR("Padding after for dimension %u must"
						" be 0 when wrap is 1\n", Dim);
				return XAIE_INVALID_DMA_DESC;
			}

			for(u8 PadDim = Dim + 1U;
					PadDim < XAIE4_DMA_PAD_NUM_DIMS;
					PadDim++) {
				if((PDesc[PadDim].After != 0U) ||
						(PDesc[PadDim].Before != 0U)) {
					XAIE_ERROR("After and Before pading "
							"for dimension %u must "
							"be 0 when wrap for "
							"dimension %u is 0\n",
							PadDim, Dim);
					return XAIE_ERR;
				}
			}
		}
	}

	XAIE_DBG("Zero padding and wrap configuration is correct\n");
	return XAIE_OK;
}

static u64 _GetChannelStatusAddr(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir)
{
	u8 TileType;
	u8 NumChannels;
	u64 Addr = 0;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);

	if ((TileType == XAIEGBL_TILE_TYPE_MEMTILE) && (Dir == DMA_MM2S))
		NumChannels = DmaMod->NumMm2sChannels;
	else
		NumChannels = DmaMod->NumChannels;

	/*Add App_B base address if ChNum is from second half of the resources*/
	if (ChNum >= NumChannels) {
		Addr = XAIE4_MASK_VALUE_APP_B;
		ChNum -= NumChannels;
	}

	Addr |= DmaMod->ChStatusBase + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		ChNum * XAIE4_DMA_STATUS_CHNUM_OFFSET +
		(u8)Dir * DmaMod->ChStatusOffset;

	return Addr;
}

static u64 _GetMemTileBdBaseAddr(const XAie_DmaMod *DmaMod, u8 BdNum, u8 ChNum, u8 Dir)
{
	u64 RegAddr = 0;
	u8 NumChannels;

	if (Dir == DMA_MM2S)
		NumChannels = DmaMod->NumMm2sChannels;
	else
		NumChannels = DmaMod->NumChannels;

	/*Add App_B base address if ChNum is from second half of the resources*/
	if (ChNum >= NumChannels) {
		RegAddr = XAIE4_MASK_VALUE_APP_B;
		ChNum -= NumChannels;
	}

	/* Calculate BD base address */
	RegAddr |= (u64)DmaMod->BaseAddr +
			((u32)Dir * DmaMod->NumChannels * DmaMod->NumBds * DmaMod->IdxOffset) + /* Direction traverse */
			(ChNum * (u32)DmaMod->NumBds * DmaMod->IdxOffset) + /* Channel traverse */
			(BdNum * (u64)DmaMod->IdxOffset); /* BD traverse */

	return RegAddr;
}

static u64 _GetShimTileCtrlMm2sChanBdBaseAddr(const XAie_DmaMod *DmaMod,
		u16 BdNum, u16 MaxNumBds)
{
	u8 ChNum;
	u64 BdBaseAddr = 0;

	if (((BdNum - MaxNumBds) > UINT16_MAX )||((BdNum - MaxNumBds) < 0)) {
		XAIE_ERROR("Wrong BdNum passed\n");
		return XAIE_ERR;
	}

	/* Get control_mm2s channel number and private pool BdNum from given BdNum */
	BdNum = BdNum - MaxNumBds;
	ChNum = BdNum / DmaMod->NumMm2sCtrlBds;
	BdNum = BdNum % DmaMod->NumMm2sCtrlBds;

	/*Add App_B base address if BD is from second half of the resources*/
	if (ChNum >= DmaMod->NumMm2sCtrlChannels) {
		BdBaseAddr = XAIE4_MASK_VALUE_APP_B;
		ChNum -= DmaMod->NumMm2sCtrlChannels;
	}

	BdBaseAddr |= DmaMod->BaseAddr +
			(DmaMod->NumBds * (u64)DmaMod->IdxOffset) + /* size of Shared BD pool */
			(ChNum * (u32)DmaMod->NumMm2sCtrlBds * (u64)DmaMod->IdxOffset) + /* Size of channel's Private BD pool */
			(BdNum * (u64)DmaMod->IdxOffset);

	return BdBaseAddr;
}

/*********************** Global Function Definitions *************************/
/*****************************************************************************/
/**
*
* This API initializes the Dma Descriptor for AIE4 Shim Dma.
*
* @param	Desc: Dma Descriptor
*
* @return	None.
*
* @note		Internal Only.
*
******************************************************************************/
void _XAie4_ShimDmaInit(XAie_DmaDesc* Desc)
{
	for (u8 i = 0U; i < 4U; i++) {
		Desc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[i].StepSize =
			XAIE4_DMA_STEPSIZE_DEFAULT;
	}

	Desc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap =
		XAIE4_DMA_ITERWRAP_DEFAULT;
	Desc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize =
		XAIE4_DMA_STEPSIZE_DEFAULT;
}

/*****************************************************************************/
/**
*
* This API initializes the Dma Descriptor for AIE4 Tile Dma.
*
* @param	Desc: Dma Descriptor
*
* @return	None.
*
* @note		Internal Only.
*
******************************************************************************/
void _XAie4_TileDmaInit(XAie_DmaDesc* Desc)
{
	for (u8 i = 0U; i < 3U; i++) {
		Desc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[i].StepSize =
			XAIE4_DMA_STEPSIZE_DEFAULT;
	}

	Desc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap =
		XAIE4_DMA_ITERWRAP_DEFAULT;
	Desc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize =
		XAIE4_DMA_STEPSIZE_DEFAULT;
}

/*****************************************************************************/
/**
*
* This API initializes the Dma Descriptor for AIE4 Mem Tile Dma.
*
* @param	Desc: Dma Descriptor
*
* @return	None.
*
* @note		Internal Only.
*
******************************************************************************/
void _XAie4_MemTileDmaInit(XAie_DmaDesc* Desc)
{
	for (u8 i = 0U; i < 4U; i++) {
		Desc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[i].StepSize =
			XAIE4_DMA_STEPSIZE_DEFAULT;
	}

	Desc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap =
		XAIE4_DMA_ITERWRAP_DEFAULT;
	Desc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize =
		XAIE4_DMA_STEPSIZE_DEFAULT;
}

/*****************************************************************************/
/**
 *
 * This API setups the DmaDesc with the register fields required for the dma
 * addressing mode of AIE4.
 *
 * @param	DmaDesc: Initialized Dma Descriptor.
 * @param	Tensor: Dma Tensor describing the address mode of dma.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		This API is taken from _XAieMl_DmaSetMultiDim and added AIE4
 * 		related changes.
 * 		Internal API only. In AIE4, D0_stepSize considered always 1,
 * 		thats why all checks have been added with (i > 0)
 *
 ******************************************************************************/
AieRC _XAie4_DmaSetMultiDim(XAie_DmaDesc *DmaDesc, XAie_DmaTensor *Tensor)
{
	/* As there is no config bits for D0_StepSize this check is not needed,
	 * means, app dont need to set the D0_StepSize at all for AIE4, For now printing a
	 * warning message
	 */
	if ( (Tensor->NumDim > 0) && (Tensor->Dim[0].AieMlDimDesc.StepSize != 1U) ) {
		XAIE_WARN("AIE4 : D0_Stepsize is removed (assumed to be always 1, i.e. linear accesses)\n");
		return XAIE_ERR;
	}

	for(u8 i = 0U; i < Tensor->NumDim; i++) {
		const XAie_DmaBdProp *BdProp = DmaDesc->DmaMod->BdProp;
		if(((u8)i > 0U) && (Tensor->Dim[i].AieMlDimDesc.StepSize > (u32)BdProp->StepSizeMax)) {
			XAIE_ERROR("Invalid stepsize for dimension %d\n", i);
			return XAIE_ERR;
		}
		/* In AIE4 Wrap Size Max is 511 for AIE Tile, 4095 for MemTile 
		   and shimTile.*/
		if(((u8)i > 0U) && (Tensor->Dim[i].AieMlDimDesc.Wrap > (u16)BdProp->WrapMax)) {
			XAIE_ERROR("Invalid wrap for dimension %d\n", i);
			return XAIE_ERR;
		}
		//There is no wrap parameter on the highest dimension. So it should passed as zero
		if(((u8)i == (Tensor->NumDim - 1)) && (Tensor->Dim[i].AieMlDimDesc.Wrap != 0)) {
			XAIE_WARN("There is no wrap parameter on the highest dimension. Dn-1_wrapSize must be 0 %d\n", i);
			Tensor->Dim[i].AieMlDimDesc.Wrap = 0;
		}
	}

	for(u8 i = 0U; i < Tensor->NumDim; i++) {
		DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[i].StepSize =
			Tensor->Dim[i].AieMlDimDesc.StepSize;
		DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[i].Wrap =
			Tensor->Dim[i].AieMlDimDesc.Wrap;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
 *
 * This API is used to get the count of scheduled BDs in pending.
 *
 * @param	DevInst: Device Instance
 * @param	Loc: Location of AIE Tile
 * @param	DmaMod: Dma module pointer
 * @param	ChNum: Channel number of the DMA.
 * @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
 * @param	PendingBd: Pointer to store the number of pending BDs.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. For AIE4 Tiles only.
 *
 ******************************************************************************/
AieRC _XAie4_DmaGetPendingBdCount(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u8 *PendingBd)
{
	AieRC RC;
	u64 Addr;
	u32 Mask, StatusReg, TaskQSize;

	Addr = _GetChannelStatusAddr(DevInst, DmaMod, Loc, ChNum, Dir);
	RC = XAie_Read32(DevInst, Addr, &StatusReg);
	if(RC != XAIE_OK) {
		return RC;
	}

	if (_XAie_CheckPrecisionExceedsForRightShift(DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.TaskQSize.Lsb,
			DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.TaskQSize.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	TaskQSize = XAie_GetField(StatusReg,
			DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.TaskQSize.Lsb,
			DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.TaskQSize.Mask);
	if(TaskQSize > DmaMod->ChProp->StartQSizeMax) {
		XAIE_ERROR("Invalid start queue size from register\n");
		return XAIE_ERR;
	}

	Mask = DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.ChannelRunning.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledLockAcq.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledLockRel.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledStreamStarve.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledTCT.Mask;

	/* Check if BD is being used by a channel */
	if(StatusReg & Mask) {
		TaskQSize++;
	}

	*PendingBd = (u8)TaskQSize;

	return XAIE_OK;
}

/*****************************************************************************/
/**
 *
 * This API is used to wait on DMA channel to be completed.
 *
 * @param	DevInst: Device Instance
 * @param	Loc: Location of Tile
 * @param	DmaMod: Dma module pointer
 * @param	ChNum: Channel number of the DMA.
 * @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
 * @param	TimeOutUs - Minimum timeout value in micro seconds.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. For AIE4 Tiles only.
 *
 ******************************************************************************/
AieRC _XAie4_DmaWaitForDone(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u32 TimeOutUs, u8 BusyPoll)
{
	u64 Addr;
	u32 Mask, Value;
	AieRC Status = XAIE_OK;

	Addr = _GetChannelStatusAddr(DevInst, DmaMod, Loc, ChNum, Dir);

	Mask = DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.TaskQSize.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.ChannelRunning.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledLockAcq.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledLockRel.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledStreamStarve.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledTCT.Mask;

	if ((_XAie_CheckPrecisionExceeds(DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.ChannelRunning.Lsb,
			_XAie_MaxBitsNeeded(XAIE4_DMA_STATUS_CHANNEL_NOT_RUNNING),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	/* This will check the stalled and start queue size bits to be zero */
	Value = (u32)(XAIE4_DMA_STATUS_CHANNEL_NOT_RUNNING <<
			DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.ChannelRunning.Lsb);

	if (BusyPoll != XAIE_ENABLE){
		Status = XAie_MaskPoll(DevInst, Addr, Mask, Value, TimeOutUs);
	} else {
		Status = XAie_MaskPollBusy(DevInst, Addr, Mask, Value, TimeOutUs);
	}

	if (Status != XAIE_OK) {
		XAIE_DBG("Dma Wait Done Status poll time out\n");
		return XAIE_ERR;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to wait on DMA channel taskqueue till its free with at least
* one task or till the timeout.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	DmaMod: Dma module pointer
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param    TimeOutUs: Minimum timeout value in micro seconds.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIEML Tiles only.
*
******************************************************************************/
AieRC _XAie4_DmaWaitForBdTaskQueue(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u32 TimeOutUs, u8 BusyPoll)
{
	u64 Addr;
	AieRC Status = XAIE_OK;

	Addr = _GetChannelStatusAddr(DevInst, DmaMod, Loc, ChNum, Dir);

	if ((_XAie_CheckPrecisionExceeds(XAIE4_DMA_STATUS_TASK_Q_SIZE_MSB,
			_XAie_MaxBitsNeeded(1U), MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	/* Poll for the MSB bit of Task_queue_size bits to ensure
	 * queue is not full*/
	if (BusyPoll != XAIE_ENABLE){
		Status = XAie_MaskPoll(DevInst, Addr, (1U << XAIE4_DMA_STATUS_TASK_Q_SIZE_MSB),
		 0, TimeOutUs);
	} else {
		Status = XAie_MaskPollBusy(DevInst, Addr, (1U << XAIE4_DMA_STATUS_TASK_Q_SIZE_MSB),
		 0, TimeOutUs);
	}

	if (Status != XAIE_OK) {
		XAIE_DBG("Wait for task queue timed out\n");
		return XAIE_ERR;
	}

	return Status;
}

/*****************************************************************************/
/**
 *
 * This API is used to check the validity of Bd number and Channel number
 * combination.
 *
 * @param	BdNum: Buffer descriptor number.
 * @param	ChNum: Channel number of the DMA.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. For AIE4 SHIM Tiles only.
 *
 ******************************************************************************/
AieRC _XAie4_ShimTileDmaCheckBdChValidity(u8 BdNum, u8 ChNum)
{
	if((BdNum < 16U) && (ChNum < 2U))
		return XAIE_OK;
	else if((BdNum < 32U) && (ChNum < 4U))
		return XAIE_OK;

	XAIE_ERROR("Invalid BdNum, ChNum combination\n");
	return XAIE_INVALID_ARGS;
}

/*****************************************************************************/
/**
 *
 * This API is used to get Channel Status register value
 *
 * @param	DevInst: Device Instance
 * @param	Loc: Location of AIE Tile
 * @param	DmaMod: Dma module pointer
 * @param	ChNum: Channel number of the DMA.
 * @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
 * @param	Status - Channel Status Register value
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note	Internal only. For AIE4 Tiles only.
 *
 ******************************************************************************/
AieRC _XAie4_DmaGetChannelStatus(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u32 *Status)
{
	u64 Addr;
	u32 Mask;
	AieRC RC;

	Addr = _GetChannelStatusAddr(DevInst, DmaMod, Loc, ChNum, Dir);
	Mask = DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.TaskQSize.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.TaskQOverFlow.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.ChannelRunning.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledLockAcq.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledLockRel.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledStreamStarve.Mask |
		DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledTCT.Mask;

	RC = XAie_Read32(DevInst, Addr, Status);
	if (RC != XAIE_OK) {
		return RC;
	}

	*Status = (*Status & Mask);
	return XAIE_OK;
}

/*****************************************************************************/
/**
 *
 * This API updates the length of the buffer descriptor in the AIE tile
 * dma module.
 *
 * @param	DevInst: Device Instance.
 * @param	DmaMod: Dma module pointer
 * @param	Loc: Location of AIE Tile
 * @param	Len: Length of BD in bytes.
 * @param	BdNum: Hardware BD number to be written to.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. This API accesses the hardware directly and does
 *		not operate on software descriptor.
 ******************************************************************************/
AieRC _XAie4_TileDmaUpdateBdLen(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u32 Len, u16 BdNum)
{
	u64 RegAddr = 0;
	u32 RegVal, Mask;

	RegAddr = (u64)(DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset) +
			(DmaMod->BdProp->BufferLen.Idx * 4U) +
			XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* get mask and Value for the buffer_length */
	Mask = DmaMod->BdProp->BufferLen.Mask;

	if ((_XAie_CheckPrecisionExceeds(DmaMod->BdProp->BufferLen.Lsb,
			_XAie_MaxBitsNeeded(Len),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RegVal = XAie_SetField(Len, DmaMod->BdProp->BufferLen.Lsb, Mask);

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, RegVal);
}

/*****************************************************************************/
/**
 *
 * This API updates the length of the buffer descriptor in the MEM tile
 * dma module.
 *
 * @param	DevInst: Device Instance.
 * @param	DmaMod: Dma module pointer
 * @param	Loc: Location of AIE Tile
 * @param	Len: Length of BD in bytes.
 * @param	BdNum: Hardware BD number to be written to.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. This API accesses the hardware directly and does
 *		not operate on software descriptor.
 ******************************************************************************/
AieRC _XAie4_MemTileDmaUpdateBdLen(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u32 Len, u16 BdNum)
{
	u64 RegAddr = 0;
	u32 RegVal, Mask;
	u8 BdNumTemp, Dir, ChNum;

	/* Get Private pool BD num, and channel info */
	BdNumTemp = XAie4_MemTileDmaBdChLut[BdNum].BdNum;
	Dir = XAie4_MemTileDmaBdChLut[BdNum].Dir;
	ChNum = XAie4_MemTileDmaBdChLut[BdNum].ChNum;

	RegAddr = _GetMemTileBdBaseAddr(DmaMod, BdNumTemp, ChNum, Dir) +
			(DmaMod->BdProp->BufferLen.Idx * 4U) +
			XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* get mask and Value for the buffer_length */
	Mask = DmaMod->BdProp->BufferLen.Mask;

	if ((_XAie_CheckPrecisionExceeds(DmaMod->BdProp->BufferLen.Lsb,
			_XAie_MaxBitsNeeded(Len),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RegVal = XAie_SetField(Len, DmaMod->BdProp->BufferLen.Lsb, Mask);

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, RegVal);
}

/*****************************************************************************/
/**
 *
 * This API updates the length of the buffer descriptor in the SHIM tile
 * dma module.
 *
 * @param	DevInst: Device Instance.
 * @param	DmaMod: Dma module pointer
 * @param	Loc: Location of AIE Tile
 * @param	Len: Length of BD in bytes.
 * @param	BdNum: Hardware BD number to be written to.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. This API accesses the hardware directly and does
 *		not operate on software descriptor.
 ******************************************************************************/
AieRC _XAie4_ShimTileDmaUpdateBdLen(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u32 Len, u16 BdNum)
{
	u64 RegAddr = 0;
	u32 RegVal, Mask;
	u16 MaxNumBds;

	/* This is needed to check if BdNum is for control_mm2s channels */
	MaxNumBds = DmaMod->NumBds * 2;

	if (BdNum >= MaxNumBds) {
		/* BD is for Control MM2S channel */
		RegAddr = _GetShimTileCtrlMm2sChanBdBaseAddr(DmaMod, BdNum, MaxNumBds);
	} else {
		/*Add App_B base address if BD is from second half of the resources*/
		if (BdNum >= DmaMod->NumBds) {
			RegAddr = XAIE4_MASK_VALUE_APP_B;
			BdNum -= DmaMod->NumBds;
		}
		RegAddr |= (u64)(DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset);
	}
	RegAddr += (DmaMod->BdProp->BufferLen.Idx * 4U) +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* get mask and Value for the buffer_length */
	Mask = DmaMod->BdProp->BufferLen.Mask;

	if ((_XAie_CheckPrecisionExceeds(DmaMod->BdProp->BufferLen.Lsb,
			_XAie_MaxBitsNeeded(Len),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RegVal = XAie_SetField(Len, DmaMod->BdProp->BufferLen.Lsb, Mask);

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, RegVal);
}

/*****************************************************************************/
/**
 *
 * This API returns the length of the buffer descriptor in the AIE Tile
 * dma module.
 *
 * @param	DevInst: Device Instance.
 * @param	DmaMod: Dma module pointer
 * @param	Loc: Location of AIE Tile
 * @param	Len: Length of BD in bytes.
 * @param	BdNum: Hardware BD number to be written to.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. This API accesses the hardware directly and does
 *		not operate on software descriptor.
 ******************************************************************************/
AieRC _XAie4_TileDmaGetBdLen(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u32 *Len, u16 BdNum)
{
	u64 RegAddr = 0;
	u32 RegVal;
	AieRC RC;

	RegAddr = (u64)(DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset) +
			XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			DmaMod->BdProp->BufferLen.Idx * 4U;
	RC = XAie_Read32(DevInst, RegAddr, &RegVal);
	if(RC != XAIE_OK) {
		return RC;
	}


	if (_XAie_CheckPrecisionExceedsForRightShift(DmaMod->BdProp->BufferLen.Lsb,
			DmaMod->BdProp->BufferLen.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	*Len = (XAie_GetField(RegVal, DmaMod->BdProp->BufferLen.Lsb,
				DmaMod->BdProp->BufferLen.Mask) +
			DmaMod->BdProp->LenActualOffset) << XAIE_DMA_32BIT_TXFER_LEN;

	return XAIE_OK;
}

/*****************************************************************************/
/**
 *
 * This API returns the length of the buffer descriptor in the MEM Tile
 * dma module.
 *
 * @param	DevInst: Device Instance.
 * @param	DmaMod: Dma module pointer
 * @param	Loc: Location of AIE Tile
 * @param	Len: Length of BD in bytes.
 * @param	BdNum: Hardware BD number to be written to.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. This API accesses the hardware directly and does
 *		not operate on software descriptor.
 ******************************************************************************/
AieRC _XAie4_MemTileDmaGetBdLen(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u32 *Len, u16 BdNum)
{
	u8 BdNumTemp, ChNum, Dir;
	u64 RegAddr = 0;
	u32 RegVal;
	AieRC RC;

	/* Get Private pool BD num, and channel info */
	BdNumTemp = XAie4_MemTileDmaBdChLut[BdNum].BdNum;
	Dir = XAie4_MemTileDmaBdChLut[BdNum].Dir;
	ChNum = XAie4_MemTileDmaBdChLut[BdNum].ChNum;

	RegAddr = _GetMemTileBdBaseAddr(DmaMod, BdNumTemp, ChNum, Dir) +
			XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			(DmaMod->BdProp->BufferLen.Idx * 4U);
	RC = XAie_Read32(DevInst, RegAddr, &RegVal);
	if(RC != XAIE_OK) {
		return RC;
	}


	if (_XAie_CheckPrecisionExceedsForRightShift(DmaMod->BdProp->BufferLen.Lsb,
			DmaMod->BdProp->BufferLen.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	*Len = (XAie_GetField(RegVal, DmaMod->BdProp->BufferLen.Lsb,
				DmaMod->BdProp->BufferLen.Mask) +
			DmaMod->BdProp->LenActualOffset) << XAIE_DMA_32BIT_TXFER_LEN;

	return XAIE_OK;
}

/*****************************************************************************/
/**
 *
 * This API returns the length of the buffer descriptor in the SHIM Tile
 * dma module.
 *
 * @param	DevInst: Device Instance.
 * @param	DmaMod: Dma module pointer
 * @param	Loc: Location of AIE Tile
 * @param	Len: Length of BD in bytes.
 * @param	BdNum: Hardware BD number to be written to.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. This API accesses the hardware directly and does
 *		not operate on software descriptor.
 ******************************************************************************/
AieRC _XAie4_ShimTileDmaGetBdLen(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u32 *Len, u16 BdNum)
{
	u16 MaxNumBds;
	u64 RegAddr = 0;
	u32 RegVal;
	AieRC RC;

	/* get the maximum BDs that the shim tile can support in
	 * single app mode*/
	MaxNumBds = DmaMod->NumBds * 2;

	/* If given BdNum is > MaxNumBds, It means it is for
	 * Control_MM2S channels */
	if (BdNum >= MaxNumBds) {
		RegAddr = _GetShimTileCtrlMm2sChanBdBaseAddr(DmaMod, BdNum, MaxNumBds);
	} else {
		/*Add App_B base address if BD is from second half of the resources*/
		if (BdNum >= DmaMod->NumBds) {
			RegAddr = XAIE4_MASK_VALUE_APP_B;
			BdNum -= DmaMod->NumBds;
		}

		RegAddr |= DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset;
	}

	RegAddr |= XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			DmaMod->BdProp->BufferLen.Idx * 4U;
	RC = XAie_Read32(DevInst, RegAddr, &RegVal);
	if(RC != XAIE_OK) {
		return RC;
	}


	if (_XAie_CheckPrecisionExceedsForRightShift(DmaMod->BdProp->BufferLen.Lsb,
			DmaMod->BdProp->BufferLen.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	*Len = (XAie_GetField(RegVal, DmaMod->BdProp->BufferLen.Lsb,
				DmaMod->BdProp->BufferLen.Mask) +
			DmaMod->BdProp->LenActualOffset) << XAIE_DMA_32BIT_TXFER_LEN;

	return XAIE_OK;
}

/*****************************************************************************/
/**
 *
 * This API updates the address of the buffer descriptor in the AIE tile
 * dma module.
 *
 * @param	DevInst: Device Instance.
 * @param	DmaMod: Dma module pointer
 * @param	Loc: Location of AIE Tile
 * @param	Addr: Buffer address
 * @param	BdNum: Hardware BD number to be written to.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. This API accesses the hardware directly and does
 *		not operate on software descriptor.
 ******************************************************************************/
AieRC _XAie4_TileDmaUpdateBdAddr(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u64 Addr, u16 BdNum)
{
	u64 RegAddr = 0;
	u32 RegVal, Mask;

	/* Calculate register address for Base_address */
	RegAddr = (u64)(DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset) +
			XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			DmaMod->BdProp->Buffer->TileDmaBuff.BaseAddr.Idx * 4U;

	/* get mask and Value for the Base_address */
	Mask = DmaMod->BdProp->Buffer->TileDmaBuff.BaseAddr.Mask;

	if ((_XAie_CheckPrecisionExceeds(DmaMod->BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
			_XAie_MaxBitsNeeded(Addr & 0xFFFFFFFFU),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RegVal = XAie_SetField(Addr,
			DmaMod->BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb, Mask);

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, RegVal);
}

/*****************************************************************************/
/**
 *
 * This API updates the address of the buffer descriptor in the MEM tile
 * dma module.
 *
 * @param	DevInst: Device Instance.
 * @param	DmaMod: Dma module pointer
 * @param	Loc: Location of AIE Tile
 * @param	Addr: Buffer address
 * @param	BdNum: Hardware BD number to be written to.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. This API accesses the hardware directly and does
 *		not operate on software descriptor.
 ******************************************************************************/
AieRC _XAie4_MemTileDmaUpdateBdAddr(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u64 Addr, u16 BdNum)
{
	u64 RegAddr = 0;
	u32 RegVal, Mask;
	u8 BdNumTemp, Dir, ChNum;

	/* Get Private pool BD num, and channel info */
	BdNumTemp = XAie4_MemTileDmaBdChLut[BdNum].BdNum;
	Dir = XAie4_MemTileDmaBdChLut[BdNum].Dir;
	ChNum = XAie4_MemTileDmaBdChLut[BdNum].ChNum;

	RegAddr = _GetMemTileBdBaseAddr(DmaMod, BdNumTemp, ChNum, Dir) +
			XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			DmaMod->BdProp->Buffer->TileDmaBuff.BaseAddr.Idx * 4U;

	/* get mask and Value for the Base_address */
	Mask = DmaMod->BdProp->Buffer->TileDmaBuff.BaseAddr.Mask;

	if ((_XAie_CheckPrecisionExceeds(DmaMod->BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
			_XAie_MaxBitsNeeded(Addr & 0xFFFFFFFFU),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RegVal = XAie_SetField(Addr,
			DmaMod->BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb, Mask);

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, RegVal);
}

/*****************************************************************************/
/**
 *
 * This API updates the address of the buffer descriptor in the SHIM Tile
 * dma module.
 *
 * @param	DevInst: Device Instance.
 * @param	DmaMod: Dma module pointer
 * @param	Loc: Location of AIE Tile
 * @param	Addr: Buffer address
 * @param	BdNum: Hardware BD number to be written to.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. This API accesses the hardware directly and does
 *		not operate on software descriptor.
 ******************************************************************************/
AieRC _XAie4_ShimTileDmaUpdateBdAddr(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u64 Addr, u16 BdNum)
{
	AieRC RC;
	u64 RegAddr, BaseAddr = 0;
	u32 RegVal, Mask;
	u16 MaxNumBds;

	/* get the maximum BDs that the shim tile can support in
	 * single app mode*/
	MaxNumBds = DmaMod->NumBds * 2;

	/* If given BdNum is > MaxNumBds, It means it is for
	 * Control_MM2S channels */
	if (BdNum >= MaxNumBds) {
		BaseAddr = _GetShimTileCtrlMm2sChanBdBaseAddr(DmaMod, BdNum, MaxNumBds);
	} else {
		/*Add App_B base address if BD is from second half of the resources*/
		if (BdNum >= DmaMod->NumBds) {
			BaseAddr = XAIE4_MASK_VALUE_APP_B;
			BdNum -= DmaMod->NumBds;
		}
		BaseAddr |= (DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset);
	}
	BaseAddr |= XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	RegAddr = BaseAddr + (DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Idx * 4U);
	Mask = DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Mask;
	RegVal = XAie_SetField(Addr >> DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb,
			DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb, Mask);

	/* Addrlow maps to a single register without other fields */
	RC =  XAie_Write32(DevInst, RegAddr, RegVal);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to update lower 32 bits of address\n");
		goto ret;
	}

	RegAddr = BaseAddr + (DmaMod->BdProp->Buffer->ShimDmaBuff.AddrHigh.Idx * 4U);
	Mask = DmaMod->BdProp->Buffer->ShimDmaBuff.AddrHigh.Mask;

	if ((_XAie_CheckPrecisionExceeds(DmaMod->BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb,
			_XAie_MaxBitsNeeded((Addr >> 32) & 0xFFFFFFFFU),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RegVal = XAie_SetField(Addr >> 32,
			DmaMod->BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb, Mask);

	/* Addrhigh maps to a single register without other fields */
	RC = XAie_Write32(DevInst, RegAddr, RegVal);
	if(RC != XAIE_OK)
		XAIE_ERROR("Failed to update 32_56 bits of address\n");

ret:
	return RC;
}



/*****************************************************************************/
/**
 *
 * This API writes a Dma Descriptor which is initialized and setup by other APIs
 * into the corresponding registers and register fields in the hardware. This API
 * is specific to AIE4 Tiles only.
 *
 * @param	DevInst: Device Instance
 * @param	DmaDesc: Initialized Dma Descriptor.
 * @param	Loc: Location of AIE Tile
 * @param	BdNum: Hardware BD number to be written to.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. For AIE4 Tiles only.
 *
 ******************************************************************************/
AieRC _XAie4_TileDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u16 BdNum)
{
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIE4_TILEDMA_NUM_BD_WORDS] = {0};
	const XAie_DmaMod *DmaMod;
	const XAie_DmaBdProp *BdProp;

	DmaMod = DevInst->DevProp.DevMod[DmaDesc->TileType].DmaMod;
	BdProp = DmaMod->BdProp;

	BdBaseAddr = (u64)(DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset);

	if ((_XAie_CheckPrecisionExceeds(BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AddrDesc.Address & 0xFFFFFFFFU),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->BufferLen.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AddrDesc.Length),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	/* Setup BdWord with the right values from DmaDesc */
	BdWord[0U] = XAie_SetField(DmaDesc->AddrDesc.Address,
			BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
			BdProp->Buffer->TileDmaBuff.BaseAddr.Mask) |
		XAie_SetField(DmaDesc->AddrDesc.Length,
				BdProp->BufferLen.Lsb,
				BdProp->BufferLen.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->Pkt->EnPkt.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PktDesc.PktEn),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->BdEn->OutofOrderBdId.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->BdEnDesc.OutofOrderBdId),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Pkt->PktId.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PktDesc.PktId),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Pkt->PktType.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PktDesc.PktType),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[1U] = XAie_SetField(DmaDesc->PktDesc.PktEn,
			BdProp->Pkt->EnPkt.Lsb,
			BdProp->Pkt->EnPkt.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.OutofOrderBdId,
				BdProp->BdEn->OutofOrderBdId.Lsb,
				BdProp->BdEn->OutofOrderBdId.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktId,
				BdProp->Pkt->PktId.Lsb,
				BdProp->Pkt->PktId.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktType,
				BdProp->Pkt->PktType.Lsb,
				BdProp->Pkt->PktType.Mask);

	if (_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].StepSize),
			MAX_VALID_AIE_REG_BIT_INDEX)){
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[2U] = XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].StepSize),
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterCurr),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].Wrap),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].StepSize),
			MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[3U] = XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterCurr,
			BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].Wrap,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].StepSize),
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].Wrap),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
			(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Lsb,
				_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap - 1U),
				MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize),
			MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[4U] = XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].Wrap,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize),
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->BdEn->TlastSuppress.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->TlastSuppress),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->BdEn->NxtBd.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->BdEnDesc.NxtBd),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->BdEn->UseNxtBd.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->BdEnDesc.UseNxtBd),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockRelId),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockAcqEn),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockAcqId),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[5U] = XAie_SetField(DmaDesc->TlastSuppress,
			BdProp->BdEn->TlastSuppress.Lsb,
			BdProp->BdEn->TlastSuppress.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.NxtBd,
				BdProp->BdEn->NxtBd.Lsb,
				BdProp->BdEn->NxtBd.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.UseNxtBd,
				BdProp->BdEn->UseNxtBd.Lsb,
				BdProp->BdEn->UseNxtBd.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelVal,
				BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
				BdProp->Lock->AieMlDmaLock.LckRelVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelId,
				BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
				BdProp->Lock->AieMlDmaLock.LckRelId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqEn,
				BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
				BdProp->Lock->AieMlDmaLock.LckAcqEn.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqVal,
				BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
				BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqId,
				BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
				BdProp->Lock->AieMlDmaLock.LckAcqId.Mask);

	Addr = BdBaseAddr + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	return XAie_BlockWrite32(DevInst, Addr, BdWord, XAIE4_TILEDMA_NUM_BD_WORDS);
}

/*****************************************************************************/
/**
 *
 * This API writes a Dma Descriptor which is initialized and setup by other APIs
 * into the corresponding registers and register fields in the hardware. This API
 * is specific to AIE4 Memory Tiles only.
 *
 * @param	DevInst: Device Instance
 * @param	DmaDesc: Initialized Dma Descriptor.
 * @param	Loc: Location of AIE Tile
 * @param	BdNum: Hardware BD number to be written to.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. For AIE4 Mem Tiles only.
 *
 ******************************************************************************/
AieRC _XAie4_MemTileDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u16 BdNum)
{
	AieRC RC;
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIE4_MEMTILEDMA_NUM_BD_WORDS] = {0};
	const XAie_DmaMod *DmaMod;
	const XAie_DmaBdProp *BdProp;
	u8 ChNum, Dir, BdNumTemp;

	RC = _XAie4_DmaMemTileCheckPaddingConfig(DmaDesc);
	if (RC != XAIE_OK) {
		return RC;
	}

	DmaMod = DevInst->DevProp.DevMod[DmaDesc->TileType].DmaMod;
	BdProp = DmaMod->BdProp;

	/* Get Private pool BD num, and channel info */
	BdNumTemp = XAie4_MemTileDmaBdChLut[BdNum].BdNum;
	Dir = XAie4_MemTileDmaBdChLut[BdNum].Dir;
	ChNum = XAie4_MemTileDmaBdChLut[BdNum].ChNum;

	BdBaseAddr = _GetMemTileBdBaseAddr(DmaMod, BdNumTemp, ChNum, Dir);

	/* Setup BdWord with the right values from DmaDesc */

	if (_XAie_CheckPrecisionExceeds(BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AddrDesc.Address & 0xFFFFFFFFU),
			MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	BdWord[0U] = XAie_SetField(DmaDesc->AddrDesc.Address,
			BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
			BdProp->Buffer->TileDmaBuff.BaseAddr.Mask);

	if (_XAie_CheckPrecisionExceeds(BdProp->BufferLen.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AddrDesc.Length),MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[1U] = XAie_SetField(DmaDesc->LockDesc.LockAcqVal,
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask) |
		XAie_SetField(DmaDesc->AddrDesc.Length,
			BdProp->BufferLen.Lsb,
			BdProp->BufferLen.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockAcqId),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockRelId),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockAcqEn),
			MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[2U] = XAie_SetField(DmaDesc->LockDesc.LockAcqId,
			BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelVal,
			BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelId,
			BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqEn,
			BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqEn.Mask);

	if (_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].StepSize),
			MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[3U] = XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].StepSize),
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask);

	if (_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].StepSize),
			MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[4U] = XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].StepSize),
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask);

	if (_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[3U].StepSize),
			MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[5U] = XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[3U].StepSize),
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->BdEn->TlastSuppress.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->TlastSuppress),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize),
			MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[6U] = XAie_SetField(DmaDesc->TlastSuppress,
			BdProp->BdEn->TlastSuppress.Lsb,
			BdProp->BdEn->TlastSuppress.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize),
			BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterCurr),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Pkt->EnPkt.Lsb,
				_XAie_MaxBitsNeeded(DmaDesc->PktDesc.PktEn),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Pkt->PktId.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PktDesc.PktId),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Pad->D0_PadAfter.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PadDesc[0U].After),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Pad->D0_PadBefore.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PadDesc[0U].Before),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[7U] = XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterCurr,
			BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktEn,
			BdProp->Pkt->EnPkt.Lsb,
			BdProp->Pkt->EnPkt.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktId,
			BdProp->Pkt->PktId.Lsb,
			BdProp->Pkt->PktId.Mask) |
		XAie_SetField(DmaDesc->PadDesc[0U].After,
			BdProp->Pad->D0_PadAfter.Lsb,
			BdProp->Pad->D0_PadAfter.Mask) |
		XAie_SetField(DmaDesc->PadDesc[0U].Before,
			BdProp->Pad->D0_PadBefore.Lsb,
			BdProp->Pad->D0_PadBefore.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->Pad->D2_PadAfter.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PadDesc[2U].After),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Pad->D2_PadBefore.Lsb,
				_XAie_MaxBitsNeeded(DmaDesc->PadDesc[2U].Before),
				MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Pad->D1_PadAfter.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PadDesc[1U].After),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Pad->D1_PadBefore.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PadDesc[1U].Before),
			MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[8U] = XAie_SetField(DmaDesc->PadDesc[2U].After,
		BdProp->Pad->D2_PadAfter.Lsb,
		BdProp->Pad->D2_PadAfter.Mask) |
		XAie_SetField(DmaDesc->PadDesc[2U].Before,
			BdProp->Pad->D2_PadBefore.Lsb,
			BdProp->Pad->D2_PadBefore.Mask) |
		XAie_SetField(DmaDesc->PadDesc[1U].After,
			BdProp->Pad->D1_PadAfter.Lsb,
			BdProp->Pad->D1_PadAfter.Mask) |
		XAie_SetField(DmaDesc->PadDesc[1U].Before,
			BdProp->Pad->D1_PadBefore.Lsb,
			BdProp->Pad->D1_PadBefore.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].Wrap),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].Wrap),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->BdEn->OutofOrderBdId.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->BdEnDesc.OutofOrderBdId),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[9U] = XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].Wrap,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].Wrap,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.OutofOrderBdId,
			BdProp->BdEn->OutofOrderBdId.Lsb,
			BdProp->BdEn->OutofOrderBdId.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->BdEn->NxtBd.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->BdEnDesc.NxtBd),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->BdEn->UseNxtBd.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->BdEnDesc.UseNxtBd),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].Wrap),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap - 1U),
			MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[10U] = XAie_SetField(DmaDesc->BdEnDesc.NxtBd,
			BdProp->BdEn->NxtBd.Lsb,
			BdProp->BdEn->NxtBd.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.UseNxtBd,
			BdProp->BdEn->UseNxtBd.Lsb,
			BdProp->BdEn->UseNxtBd.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].Wrap,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap - 1U),
			BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Mask);

	Addr = BdBaseAddr + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	return XAie_BlockWrite32(DevInst, Addr, BdWord, XAIE4_MEMTILEDMA_NUM_BD_WORDS);
}

/*****************************************************************************/
/**
 *
 * This API writes a Dma Descriptor which is initialized and setup by other APIs
 * into the corresponding registers and register fields in the hardware. This API
 * is specific to AIE4 Shim Tiles only.
 *
 * @param	DevInst: Device Instance
 * @param	DmaDesc: Initialized Dma Descriptor.
 * @param	Loc: Location of AIE Tile
 * @param	BdNum: Hardware BD number to be written to.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. For AIE4 Shim Tiles only.
 *
 ******************************************************************************/
AieRC _XAie4_ShimDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u16 BdNum)
{
	u64 Addr;
	u64 BdBaseAddr = 0;
	u32 BdWord[XAIE4_SHIMDMA_NUM_BD_WORDS];
	u16 MaxNumBds;
	XAie_ShimDmaBdArgs Args;
	const XAie_DmaMod *DmaMod;
	const XAie_DmaBdProp *BdProp;

	DmaMod = DevInst->DevProp.DevMod[DmaDesc->TileType].DmaMod;
	BdProp = DmaMod->BdProp;

	/* get the maximum BDs that the shim tile can support in
	 * single app mode*/
	MaxNumBds = DmaMod->NumBds * 2;

	/* If given BdNum is > MaxNumBds, It means it is for
	 * Control_MM2S channels */
	if (BdNum >= MaxNumBds) {
		BdBaseAddr = _GetShimTileCtrlMm2sChanBdBaseAddr(DmaMod, BdNum, MaxNumBds);
	} else {
		/*Add App_B base address if BD is from second half of the resources*/
		if (BdNum >= DmaMod->NumBds) {
			BdBaseAddr = XAIE4_MASK_VALUE_APP_B;
			BdNum -= DmaMod->NumBds;
		}

		BdBaseAddr |= DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset;
	}

	/* Setup BdWord with the right values from DmaDesc */

	if (_XAie_CheckPrecisionExceeds(BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb,
			_XAie_MaxBitsNeeded((DmaDesc->AddrDesc.Address >> 32U) & 0xFFFFFFFFU),
			MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	BdWord[0U] = XAie_SetField((DmaDesc->AddrDesc.Address >> 32U),
			BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrHigh.Mask);

	BdWord[1U] = XAie_SetField(DmaDesc->AddrDesc.Address >>	BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrLow.Mask);

	if (_XAie_CheckPrecisionExceeds(BdProp->BufferLen.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AddrDesc.Length),MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[2U] = XAie_SetField(DmaDesc->AddrDesc.Length,
			BdProp->BufferLen.Lsb,
			BdProp->BufferLen.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockAcqId),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockAcqEn),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->SysProp->BurstLen.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AxiDesc.BurstLen),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].Wrap),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].Wrap),
			MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[3U] = XAie_SetField(DmaDesc->LockDesc.LockAcqId,
			BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqEn,
				BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
				BdProp->Lock->AieMlDmaLock.LckAcqEn.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.BurstLen,
				BdProp->SysProp->BurstLen.Lsb,
				BdProp->SysProp->BurstLen.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].Wrap,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].Wrap,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].Wrap),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockRelId), MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[4U] = XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].Wrap,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelVal,
				BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
				BdProp->Lock->AieMlDmaLock.LckRelVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelId,
				BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
				BdProp->Lock->AieMlDmaLock.LckRelId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqVal,
				BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
				BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->Pkt->EnPkt.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PktDesc.PktEn),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->BdEn->NxtBd.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->BdEnDesc.NxtBd), MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->BdEn->UseNxtBd.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->BdEnDesc.UseNxtBd),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->SysProp->AxQos.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AxiDesc.AxQos), MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].StepSize),
			MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[5U] = XAie_SetField(DmaDesc->PktDesc.PktEn,
			BdProp->Pkt->EnPkt.Lsb,
			BdProp->Pkt->EnPkt.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.NxtBd,
				BdProp->BdEn->NxtBd.Lsb,
				BdProp->BdEn->NxtBd.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.UseNxtBd,
				BdProp->BdEn->UseNxtBd.Lsb,
				BdProp->BdEn->UseNxtBd.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.AxQos,
				BdProp->SysProp->AxQos.Lsb,
				BdProp->SysProp->AxQos.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].StepSize),
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->Compression->EnCompression.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->EnCompression), MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->BdEn->TlastSuppress.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->TlastSuppress), MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterCurr),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].StepSize),
			MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[6U] = XAie_SetField(DmaDesc->EnCompression,
			BdProp->Compression->EnCompression.Lsb,
			BdProp->Compression->EnCompression.Mask) |
		XAie_SetField(DmaDesc->TlastSuppress,
				BdProp->BdEn->TlastSuppress.Lsb,
				BdProp->BdEn->TlastSuppress.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterCurr,
				BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].StepSize),
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->SysProp->AxCache.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AxiDesc.AxCache),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap - 1U),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize),
			MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[7U] = XAie_SetField(DmaDesc->AxiDesc.AxCache,
			BdProp->SysProp->AxCache.Lsb,
			BdProp->SysProp->AxCache.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap - 1U),
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize),
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[3U].StepSize),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->BdEn->OutofOrderBdId.Lsb,
				_XAie_MaxBitsNeeded(DmaDesc->BdEnDesc.OutofOrderBdId),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Pkt->PktId.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PktDesc.PktId),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[8U] = XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[3U].StepSize),
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.OutofOrderBdId,
				BdProp->BdEn->OutofOrderBdId.Lsb,
				BdProp->BdEn->OutofOrderBdId.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktId,
				BdProp->Pkt->PktId.Lsb,
				BdProp->Pkt->PktId.Mask);

	Addr = BdBaseAddr + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	Args.NumBdWords = XAIE4_SHIMDMA_NUM_BD_WORDS;
	Args.BdWords = &BdWord[0U];
	Args.Loc = Loc;
	Args.VAddr = DmaDesc->AddrDesc.Address;
	Args.BdNum = BdNum;
	Args.Addr = Addr;
	Args.MemInst = DmaDesc->MemInst;

	return XAie_RunOp(DevInst, XAIE_BACKEND_OP_CONFIG_SHIMDMABD, (void *)&Args);
}

/*****************************************************************************/
/**
 *
 * This API reads the data from the buffer descriptor registers to fill the
 * DmaDesc structure. This API is meant for AIE4 AIE Tiles only.
 *
 * @param	DevInst: Device Instance
 * @param	DmaDesc: Dma Descriptor to be filled.
 * @param	Loc: Location of AIE Tile
 * @param	BdNum: Hardware BD number to be read from.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. For AIE4 AIE Tiles only.
 *
 ******************************************************************************/
AieRC _XAie4_TileDmaReadBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u16 BdNum)
{
	AieRC RC;
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIE4_TILEDMA_NUM_BD_WORDS];
	const XAie_DmaBdProp *BdProp;

	BdProp = DmaDesc->DmaMod->BdProp;
	BdBaseAddr = (u64)(DmaDesc->DmaMod->BaseAddr +
			BdNum * (u64)DmaDesc->DmaMod->IdxOffset);
	Addr = BdBaseAddr + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* Setup DmaDesc with values read from bd registers */
	for(u8 i = 0; i < XAIE4_TILEDMA_NUM_BD_WORDS; i++) {
		RC = XAie_Read32(DevInst, Addr, &BdWord[i]);
		if (RC != XAIE_OK) {
			return RC;
		}
		Addr += 4U;
	}


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
			BdProp->Buffer->TileDmaBuff.BaseAddr.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AddrDesc.Address = (u64)XAie_GetField(BdWord[0U],
			BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
			BdProp->Buffer->TileDmaBuff.BaseAddr.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BufferLen.Lsb,
			BdProp->BufferLen.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AddrDesc.Length = XAie_GetField(BdWord[0U],
			BdProp->BufferLen.Lsb,
			BdProp->BufferLen.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pkt->EnPkt.Lsb,
			BdProp->Pkt->EnPkt.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PktDesc.PktEn = (u8)(XAie_GetField(BdWord[1U],
			BdProp->Pkt->EnPkt.Lsb,
			BdProp->Pkt->EnPkt.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->OutofOrderBdId.Lsb,
			BdProp->BdEn->OutofOrderBdId.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->BdEnDesc.OutofOrderBdId = (u8)(XAie_GetField(BdWord[1U],
			BdProp->BdEn->OutofOrderBdId.Lsb,
			BdProp->BdEn->OutofOrderBdId.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pkt->PktId.Lsb,
			BdProp->Pkt->PktId.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PktDesc.PktId = (u8)(XAie_GetField(BdWord[1U],
			BdProp->Pkt->PktId.Lsb,
			BdProp->Pkt->PktId.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pkt->PktType.Lsb,
			BdProp->Pkt->PktType.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PktDesc.PktType = (u8)(XAie_GetField(BdWord[1U],
			BdProp->Pkt->PktType.Lsb,
			BdProp->Pkt->PktType.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].StepSize = 1U +
		XAie_GetField(BdWord[2U],
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterCurr =
		(u8)(XAie_GetField(BdWord[3U],
				BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].Wrap =
		(u16)(XAie_GetField(BdWord[3U],
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask) & 0xFFFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].StepSize = 1U +
		XAie_GetField(BdWord[3U],
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].Wrap =
		(u16)(XAie_GetField(BdWord[4U],
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask) & 0xFFFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap = (u16)((1U +
			XAie_GetField(BdWord[4U],
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Mask)) & 0xFFFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize = 1U +
		XAie_GetField(BdWord[4U],
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Mask);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->TlastSuppress.Lsb,
			BdProp->BdEn->TlastSuppress.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->TlastSuppress = (u8)(XAie_GetField(BdWord[5U],
			BdProp->BdEn->TlastSuppress.Lsb,
			BdProp->BdEn->TlastSuppress.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->NxtBd.Lsb,
			BdProp->BdEn->NxtBd.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->BdEnDesc.NxtBd = (u8)(XAie_GetField(BdWord[5U],
			BdProp->BdEn->NxtBd.Lsb,
			BdProp->BdEn->NxtBd.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->UseNxtBd.Lsb,
			BdProp->BdEn->UseNxtBd.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->BdEnDesc.UseNxtBd = (u8)(XAie_GetField(BdWord[5U],
			BdProp->BdEn->UseNxtBd.Lsb,
			BdProp->BdEn->UseNxtBd.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelVal.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockRelVal = (s8)XAie_GetField(BdWord[5U],
			BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelVal.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelId.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockRelId = (u8)(XAie_GetField(BdWord[5U],
			BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelId.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqEn.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqEn = (u8)(XAie_GetField(BdWord[5U],
			BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqEn.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqVal = (s8)XAie_GetField(BdWord[5U],
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqId.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqId = (u8)(XAie_GetField(BdWord[5U],
			BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqId.Mask) & 0xFFU);

	return XAIE_OK;
}

/*****************************************************************************/
/**
 *
 * This API reads a the data from the buffer descriptor registers to fill the
 * DmaDesc structure. This API is meant for AIE4 memory tiles only.
 *
 * @param	DevInst: Device Instance
 * @param	DmaDesc: Dma Descriptor to be filled.
 * @param	Loc: Location of MEM Tile
 * @param	BdNum: Hardware BD number to be read from.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. For AIE4 Mem Tiles only.
 *
 ******************************************************************************/
AieRC _XAie4_MemTileDmaReadBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u16 BdNum)
{
	AieRC RC;
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIE4_MEMTILEDMA_NUM_BD_WORDS];
	const XAie_DmaBdProp *BdProp;
	u8 BdNumTemp, Dir, ChNum;

	BdProp = DmaDesc->DmaMod->BdProp;

	/* Get Private pool BD num, and channel info */
	BdNumTemp = XAie4_MemTileDmaBdChLut[BdNum].BdNum;
	Dir = XAie4_MemTileDmaBdChLut[BdNum].Dir;
	ChNum = XAie4_MemTileDmaBdChLut[BdNum].ChNum;

	BdBaseAddr = _GetMemTileBdBaseAddr(DmaDesc->DmaMod, BdNumTemp, ChNum, Dir);
	Addr = BdBaseAddr + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* Setup DmaDesc with values read from bd registers */
	for(u8 i = 0; i < XAIE4_MEMTILEDMA_NUM_BD_WORDS; i++) {
		RC = XAie_Read32(DevInst, Addr, &BdWord[i]);
		if (RC != XAIE_OK) {
			return RC;
		}
		Addr += 4U;
	}

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
			BdProp->Buffer->TileDmaBuff.BaseAddr.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AddrDesc.Address = (u64)XAie_GetField(BdWord[0U],
			BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
			BdProp->Buffer->TileDmaBuff.BaseAddr.Mask);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqVal = (s8)XAie_GetField(BdWord[1U],
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BufferLen.Lsb,
			BdProp->BufferLen.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AddrDesc.Length = XAie_GetField(BdWord[1U],
			BdProp->BufferLen.Lsb,
			BdProp->BufferLen.Mask);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqId.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqId = (u8)(XAie_GetField(BdWord[2U],
			BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqId.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelVal.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockRelVal = (s8)XAie_GetField(BdWord[2U],
			BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelVal.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelId.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockRelId = (u8)(XAie_GetField(BdWord[2U],
			BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelId.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqEn.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqEn = (u8)(XAie_GetField(BdWord[2U],
			BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqEn.Mask) & 0xFFU);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].StepSize =
		XAie_GetField(BdWord[3U],
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].StepSize =
		XAie_GetField(BdWord[4U],
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[3U].StepSize =
		XAie_GetField(BdWord[5U],
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Mask);
	

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->TlastSuppress.Lsb,
			BdProp->BdEn->TlastSuppress.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->TlastSuppress = (u8)(XAie_GetField(BdWord[6U],
			BdProp->BdEn->TlastSuppress.Lsb,
			BdProp->BdEn->TlastSuppress.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize =
		XAie_GetField(BdWord[6U],
			BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Mask);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}	
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterCurr =
		(u8)(XAie_GetField(BdWord[7U],
			BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pkt->EnPkt.Lsb,
			BdProp->Pkt->EnPkt.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PktDesc.PktEn = (u8)(XAie_GetField(BdWord[7U],
			BdProp->Pkt->EnPkt.Lsb,
			BdProp->Pkt->EnPkt.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pkt->PktId.Lsb,
			BdProp->Pkt->PktId.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PktDesc.PktId = (u8)(XAie_GetField(BdWord[7U],
			BdProp->Pkt->PktId.Lsb,
			BdProp->Pkt->PktId.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pad->D0_PadAfter.Lsb,
			BdProp->Pad->D0_PadAfter.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PadDesc[0U].After = (u8)(XAie_GetField(BdWord[7U],
			BdProp->Pad->D0_PadAfter.Lsb,
			BdProp->Pad->D0_PadAfter.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pad->D0_PadBefore.Lsb,
			BdProp->Pad->D0_PadBefore.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PadDesc[0U].Before = (u8)(XAie_GetField(BdWord[7U],
			BdProp->Pad->D0_PadBefore.Lsb,
			BdProp->Pad->D0_PadBefore.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pad->D2_PadAfter.Lsb,
			BdProp->Pad->D2_PadAfter.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PadDesc[2U].After = (u8)(XAie_GetField(BdWord[5U],
			BdProp->Pad->D2_PadAfter.Lsb,
			BdProp->Pad->D2_PadAfter.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pad->D2_PadBefore.Lsb,
			BdProp->Pad->D2_PadBefore.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PadDesc[2U].Before = (u8)(XAie_GetField(BdWord[4U],
			BdProp->Pad->D2_PadBefore.Lsb,
			BdProp->Pad->D2_PadBefore.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pad->D1_PadAfter.Lsb,
			BdProp->Pad->D1_PadAfter.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PadDesc[1U].After = (u8)(XAie_GetField(BdWord[5U],
			BdProp->Pad->D1_PadAfter.Lsb,
			BdProp->Pad->D1_PadAfter.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pad->D1_PadBefore.Lsb,
			BdProp->Pad->D1_PadBefore.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PadDesc[1U].Before = (u8)(XAie_GetField(BdWord[5U],
			BdProp->Pad->D1_PadBefore.Lsb,
			BdProp->Pad->D1_PadBefore.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].Wrap =
		(u16)(XAie_GetField(BdWord[9U],
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask) & 0xFFFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].Wrap =
		(u16)(XAie_GetField(BdWord[9U],
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask) & 0xFFFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->OutofOrderBdId.Lsb,
			BdProp->BdEn->OutofOrderBdId.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->BdEnDesc.OutofOrderBdId = (u8)(XAie_GetField(BdWord[9U],
		BdProp->BdEn->OutofOrderBdId.Lsb,
		BdProp->BdEn->OutofOrderBdId.Mask) & 0xFFU);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->NxtBd.Lsb,
			BdProp->BdEn->NxtBd.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->BdEnDesc.NxtBd = (u8)(XAie_GetField(BdWord[10U],
		BdProp->BdEn->NxtBd.Lsb,
		BdProp->BdEn->NxtBd.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->UseNxtBd.Lsb,
			BdProp->BdEn->UseNxtBd.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->BdEnDesc.UseNxtBd = (u8)(XAie_GetField(BdWord[10U],
		BdProp->BdEn->UseNxtBd.Lsb,
		BdProp->BdEn->UseNxtBd.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].Wrap =
		(u16)(XAie_GetField(BdWord[10U],
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Mask) & 0xFFFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap = (u16)((1U +
		XAie_GetField(BdWord[10U],
			BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Mask)) & 0xFFFFU);

	return XAIE_OK;
}

/*****************************************************************************/
/**
 *
 * This API reads a the data from the buffer descriptor registers to fill the
 * DmaDesc structure. This API is meant for AIE4 Shim Tiles only.
 *
 * @param	DevInst: Device Instance
 * @param	DmaDesc: Dma Descriptor to be filled.
 * @param	Loc: Location of AIE Tile
 * @param	BdNum: Hardware BD number to be read from.
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		Internal only. For AIE4 Shim Tiles only.
 *
 ******************************************************************************/
AieRC _XAie4_ShimDmaReadBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u16 BdNum)
{
	AieRC RC;
	u64 Addr;
	u64 BdBaseAddr = 0;
	u32 BdWord[XAIE4_SHIMDMA_NUM_BD_WORDS];
	u16 MaxNumBds;
	const XAie_DmaBdProp *BdProp;

	BdProp = DmaDesc->DmaMod->BdProp;

	/* get the maximum BDs that the shim tile can support in
	 * single app mode*/
	MaxNumBds = DmaDesc->DmaMod->NumBds * 2;

	/* If given BdNum is > MaxNumBds, It means it is for
	 * Control_MM2S channels */
	if (BdNum >= MaxNumBds) {
		BdBaseAddr = _GetShimTileCtrlMm2sChanBdBaseAddr(DmaDesc->DmaMod, BdNum, MaxNumBds);
	} else {
		/*Add App_B base address if BD is from second half of the resources*/
		if (BdNum >= DmaDesc->DmaMod->NumBds) {
			BdBaseAddr = XAIE4_MASK_VALUE_APP_B;
			BdNum -= DmaDesc->DmaMod->NumBds;
		}

		BdBaseAddr |= DmaDesc->DmaMod->BaseAddr + BdNum * (u64)DmaDesc->DmaMod->IdxOffset;
	}
	Addr = BdBaseAddr + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* Setup DmaDesc with values read from bd registers */
	for(u8 i = 0; i < XAIE4_SHIMDMA_NUM_BD_WORDS; i++) {
		RC = XAie_Read32(DevInst, Addr, &BdWord[i]);
		if (RC != XAIE_OK) {
			return RC;
		}
		Addr += 4U;
	}


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrHigh.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AddrDesc.Address |= (u64)XAie_GetField(BdWord[0U],
			BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrHigh.Mask) << 32U;


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrLow.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AddrDesc.Address |= (u64)XAie_GetField(BdWord[1U],
			BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrLow.Mask) <<
		BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb;


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BufferLen.Lsb,
			BdProp->BufferLen.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AddrDesc.Length = XAie_GetField(BdWord[2U],
			BdProp->BufferLen.Lsb,
			BdProp->BufferLen.Mask);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqId.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqId = XAie_GetField(BdWord[3U],
			BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqId.Mask) & 0xFFFFU;

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqEn.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqEn = XAie_GetField(BdWord[3U],
			BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqEn.Mask) & 0xFFU;

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->SysProp->BurstLen.Lsb,
			BdProp->SysProp->BurstLen.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AxiDesc.BurstLen = XAie_GetField(BdWord[3U],
			BdProp->SysProp->BurstLen.Lsb,
			BdProp->SysProp->BurstLen.Mask) & 0xFFU;

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].Wrap =
		XAie_GetField(BdWord[3U],
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask) & 0xFFFFU;

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].Wrap =
		XAie_GetField(BdWord[3U],
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Mask) & 0xFFFFU;


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].Wrap =
		XAie_GetField(BdWord[4U],
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask) & 0xFFFFU;

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelVal.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockRelVal = XAie_GetField(BdWord[4U],
			BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelVal.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelId.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockRelId = XAie_GetField(BdWord[4U],
			BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelId.Mask) & 0xFFU;

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqVal = XAie_GetField(BdWord[4U],
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask) & 0xFFU;


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pkt->EnPkt.Lsb,
			BdProp->Pkt->EnPkt.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PktDesc.PktEn = XAie_GetField(BdWord[5U],
			BdProp->Pkt->EnPkt.Lsb,
			BdProp->Pkt->EnPkt.Mask) & 0xFFU;

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->NxtBd.Lsb,
			BdProp->BdEn->NxtBd.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->BdEnDesc.NxtBd = XAie_GetField(BdWord[5U],
			BdProp->BdEn->NxtBd.Lsb,
			BdProp->BdEn->NxtBd.Mask) & 0xFFU;

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->UseNxtBd.Lsb,
			BdProp->BdEn->UseNxtBd.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->BdEnDesc.UseNxtBd = XAie_GetField(BdWord[5U],
			BdProp->BdEn->UseNxtBd.Lsb,
			BdProp->BdEn->UseNxtBd.Mask) & 0xFFU;

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->SysProp->AxQos.Lsb,
			BdProp->SysProp->AxQos.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AxiDesc.AxQos = XAie_GetField(BdWord[5U],
			BdProp->SysProp->AxQos.Lsb,
			BdProp->SysProp->AxQos.Mask) & 0xFFU;

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[1U].StepSize = 1U +
		XAie_GetField(BdWord[5U],
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Compression->EnCompression.Lsb,
			BdProp->Compression->EnCompression.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->EnCompression = (u8)(XAie_GetField(BdWord[6U],
			BdProp->Compression->EnCompression.Lsb,
			BdProp->Compression->EnCompression.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->TlastSuppress.Lsb,
			BdProp->BdEn->TlastSuppress.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->TlastSuppress = XAie_GetField(BdWord[6U],
			BdProp->BdEn->TlastSuppress.Lsb,
			BdProp->BdEn->TlastSuppress.Mask) & 0xFFU;

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterCurr =
		XAie_GetField(BdWord[6U],
				BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.IterCurr.Mask) & 0xFFU;

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[2U].StepSize = 1U +
		XAie_GetField(BdWord[6U],
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->SysProp->AxCache.Lsb,
			BdProp->SysProp->AxCache.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AxiDesc.AxCache = XAie_GetField(BdWord[7U],
			BdProp->SysProp->AxCache.Lsb,
			BdProp->SysProp->AxCache.Mask)& 0xFFU;

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap = ((1U +
		XAie_GetField(BdWord[7U],
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.Wrap.Mask)) & 0xFFFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize = 1U +
		XAie_GetField(BdWord[7U],
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.Iter.StepSize.Mask);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[3U].StepSize = 1U +
		XAie_GetField(BdWord[8U],
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->OutofOrderBdId.Lsb,
			BdProp->BdEn->OutofOrderBdId.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->BdEnDesc.OutofOrderBdId = XAie_GetField(BdWord[8U],
			BdProp->BdEn->OutofOrderBdId.Lsb,
			BdProp->BdEn->OutofOrderBdId.Mask) & 0xFFU;

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pkt->PktId.Lsb,
			BdProp->Pkt->PktId.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PktDesc.PktId = XAie_GetField(BdWord[8U],
			BdProp->Pkt->PktId.Lsb,
			BdProp->Pkt->PktId.Mask) & 0xFFU;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the iteration parameters for a Buffer descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	StepSize: Offset applied at each execution of the BD.
* @param	Wrap: Iteration Wrap.
* @param	IterCurr: Current iteration step. This field is incremented by
*		the hardware after BD is loaded.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal. The stepsize and wrap parameters operate at 32 bit
*		granularity. The address is the absolute address of the buffer
*		which is 32 bit aligned. The driver will configure the BD
*		register with necessary bits(<32 bits) as required by the
*		hardware.
*
******************************************************************************/
AieRC _XAie4_DmaSetBdIteration(XAie_DmaDesc *DmaDesc, u32 StepSize, u16 Wrap,
		u8 IterCurr)
{
	const XAie_DmaBdProp *BdProp = DmaDesc->DmaMod->BdProp;

	if((StepSize > (BdProp->IterStepSizeMax )) || (Wrap < 1) || (Wrap > (BdProp->IterWrapMax + 1U)) ||
			(IterCurr > BdProp->IterCurrMax)) {
		XAIE_ERROR("Iteration parameters out of Range.\n");
		return XAIE_ERR;
	}

	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.StepSize = StepSize;
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterDesc.Wrap = Wrap;
	DmaDesc->MultiDimDesc.AieMlMultiDimDesc.IterCurr = IterCurr;

	return XAIE_OK;
}

#endif /* XAIE_FEATURE_DMA_ENABLE */

/** @} */
