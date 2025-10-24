/******************************************************************************
 * Copyright (C) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
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
#define XAIE4_MEMTILEDMA_NUM_BD_WORDS			11U
#define XAIE4_DMA_STEPSIZE_DEFAULT			1U
#define XAIE4_DMA_ITERWRAP_DEFAULT			1U
#define XAIE4_DMA_PAD_NUM_DIMS				3U

#define XAIE4_DMA_STATUS_IDLE				0x0U
#define XAIE4_DMA_STATUS_CHANNEL_NOT_RUNNING		0x0U
#define XAIE4_DMA_STATUS_CHNUM_OFFSET			0x4U
#define XAIE4_DMA_STATUS_TASK_Q_SIZE_MSB		24
#define XAIE4_LOCK_ACQ_MASK				0x7FU

#define XAIE4_DMA_PAD_WORDS_MAX				0xFFU /* 8 bits */
/*These values are referenced from AIE4 architecture spec V1.5
 * Table 6-7 */
#define DMA_MEMTILE_LOCAL_LOCK_LOW			448U
#define DMA_MEMTILE_LOCAL_LOCK_HIGH			479U

/*********************** Function Definitions *************************/
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
AieRC _XAie4_DmaMemTileCheckPaddingConfig(XAie_DmaDesc *DmaDesc)
{
	XAie_AieMlDimDesc *DDesc = DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc;
	XAie_PadDesc *PDesc = DmaDesc->PadDesc;

	for(u8 Dim = 0U; Dim < XAIE4_DMA_PAD_NUM_DIMS; Dim++) {
		/* The max number of words that can be padded for dimension 0, 1 and 2 are 8 bits only in AIE4 */
		if((PDesc[Dim].After > XAIE4_DMA_PAD_WORDS_MAX) || (PDesc[Dim].Before > XAIE4_DMA_PAD_WORDS_MAX)) {
			XAIE_ERROR("Before %d or After %d Padding for dimension %d must be less than or equal %d\n",
				PDesc[Dim].Before, PDesc[Dim].After, Dim, XAIE4_DMA_PAD_WORDS_MAX);
			return XAIE_INVALID_DMA_DESC;
		}

		if(DDesc[Dim].Wrap == 0U) {
			if(PDesc[Dim].After != 0U) {
				XAIE_ERROR("Padding after for dimension %u must"
						" be 0 when wrap is 0\n", Dim);
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

/*****************************************************************************/
/**
 * _XAie4_DmaGetChannelCtrlAddr - Retrieves the control address for a specific DMA channel.
 * @DevInst: Pointer to the device instance.
 * @DmaMod: Pointer to the DMA module.
 * @Loc: Location type of the DMA module.
 * @Dir: Direction of the DMA transfer.
 * @ChNum: Channel number for which the control address is to be retrieved.
 *
 * This function calculates and returns the control address for a given DMA channel
 * based on the provided device instance, DMA module, location, direction, and channel number.
 *
 * Return: The control address for the specified DMA channel.
*****************************************************************************/
u64 _XAie4_DmaGetChannelCtrlAddr(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
				      XAie_LocType Loc, XAie_DmaDirection Dir, u8 ChNum)
{
	u8 NumChannels = 0;
	u64 ChCtrlBase = 0;
	u64 Addr = 0;
	
	switch (Dir) {
		case DMA_S2MM:
			NumChannels = DmaMod->NumChannels;
			ChCtrlBase = DmaMod->ChCtrlBase;
			break;
		case DMA_MM2S:
			NumChannels = DmaMod->NumMm2sChannels;
			ChCtrlBase = DmaMod->ChCtrlMm2sBase;
			break;
		case DMA_MM2S_CTRL:
			NumChannels = DmaMod->CtrlMm2sProp->NumChannels;
			ChCtrlBase = DmaMod->CtrlMm2sProp->ChCtrlBase;
			break;
		case DMA_S2MM_TRACE:
			NumChannels =  DmaMod->TraceS2mmProp->NumChannels;
			ChCtrlBase = DmaMod->TraceS2mmProp->ChCtrlBase;
			break;
		default:
			XAIE_ERROR("Invalid Channel direction \n");
			break; 
	}

	/*Add App_B base address if ChNum is from second half of the resources*/
	if (DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) {
		if (ChNum >= NumChannels) {
			ChCtrlBase |= _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, ChCtrlBase);
			ChNum -= NumChannels;
		}
	}

	Addr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		ChCtrlBase + ChNum * (u64)DmaMod->ChIdxOffset;

	return Addr;
}

/*****************************************************************************/
/**
 * _XAie4_DmaGetChannelStatusAddr - Get the address of the channel status register
 * @DevInst: Pointer to the device instance
 * @DmaMod: Pointer to the DMA module
 * @Loc: Location type
 * @ChNum: Channel number
 * @Dir: Direction of the channel
 *
 * This function calculates and returns the address of the channel status
 * register for a given DMA module, location, channel number, and direction.
 *
 * Return: Address of the channel status register
*****************************************************************************/
u64 _XAie4_DmaGetChannelStatusAddr(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir)
{
	u8 NumChannels = 0;
	u64 ChStatusBase = 0;
	u64 addr = 0;

	switch (Dir) {
		case DMA_S2MM:
			NumChannels = DmaMod->NumChannels;
			ChStatusBase = DmaMod->ChStatusBase;
			break;
		case DMA_MM2S:
			NumChannels = DmaMod->NumMm2sChannels;
			ChStatusBase = DmaMod->ChStatusBase + ((u8)Dir * DmaMod->ChStatusOffset);
			break;
		case DMA_MM2S_CTRL:
			NumChannels = DmaMod->CtrlMm2sProp->NumChannels;
			ChStatusBase = DmaMod->CtrlMm2sProp->ChStatusBase;
			break;
		case DMA_S2MM_TRACE:
			NumChannels =  DmaMod->TraceS2mmProp->NumChannels;
			ChStatusBase = DmaMod->TraceS2mmProp->ChStatusBase;
			break;
		default:
			XAIE_ERROR("Invalid Channel direction \n");
			break;
	}
	/*Add App_B base address if ChNum is from second half of the resources*/
	if (DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) {
		if (ChNum >= NumChannels) {
			ChStatusBase |= _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, ChStatusBase);
			ChNum -= NumChannels;
		}
	}

	addr |= ChStatusBase + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			ChNum * XAIE4_DMA_STATUS_CHNUM_OFFSET;

	return addr;
}

/*****************************************************************************/
/**
 * _XAie4_GetMemTileBdBaseAddr - Retrieves the base address of the memory tile
 * buffer descriptor (BD) for a given DMA module.
 *
 * @DmaMod: Pointer to the DMA module structure.
 * @BdNum: Buffer descriptor number.
 * @ChNum: Channel number.
 * @Dir: Direction of the DMA transfer.
 *
 * This function calculates and returns the base address of the memory tile
 * buffer descriptor based on the provided DMA module, buffer descriptor number,
 * channel number, and direction.
 *
 * Return: The base address of the memory tile buffer descriptor.
*****************************************************************************/
u64 _XAie4_GetMemTileBdBaseAddr(const XAie_DmaMod *DmaMod, u16 BdNum, u8 ChNum, XAie_DmaDirection Dir)
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
	 * means app dont need to set the D0_StepSize at all for AIE4, For now printing a
	 * warning message
	 */
	if ( (Tensor->NumDim > 0) && (Tensor->Dim[0].AieMlDimDesc.StepSize != 1U) ) {
		XAIE_WARN("AIE4 : D0_Stepsize is removed (assumed to be always 1, i.e. linear accesses)\n");
	}

	for(u8 i = 0U; i < Tensor->NumDim; i++) {
		const XAie_DmaBdProp *BdProp = DmaDesc->DmaMod->BdProp;
		if(((u8)i > 0U) && (Tensor->Dim[i].AieMlDimDesc.StepSize > (u32)BdProp->StepSizeMax)) {
			XAIE_ERROR("Invalid stepsize for dimension %d with stepsize %d\n",
					i, Tensor->Dim[i].AieMlDimDesc.StepSize);
			return XAIE_ERR;
		}
		/* In AIE4 Wrap Size Max is 511 for AIE Tile, 4095 for MemTile 
		   and shimTile.*/
		if(((u8)i > 0U) && (Tensor->Dim[i].AieMlDimDesc.Wrap > (u16)BdProp->WrapMax)) {
			XAIE_ERROR("Invalid wrap for dimension %d with wrap %d\n",
					i, Tensor->Dim[i].AieMlDimDesc.Wrap);
			return XAIE_ERR;
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

	Addr = _XAie4_DmaGetChannelStatusAddr(DevInst, DmaMod, Loc, ChNum, Dir);
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

	Addr = _XAie4_DmaGetChannelStatusAddr(DevInst, DmaMod, Loc, ChNum, Dir);

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
		if (Status != XAIE_OK) {
			XAIE_ERROR("Dma Wait Done Status MaskPoll time out : %d\n", TimeOutUs);
			return XAIE_ERR;
		}
	} else {
		Status = XAie_MaskPollBusy(DevInst, Addr, Mask, Value, TimeOutUs);
		if (Status != XAIE_OK) {
			XAIE_ERROR("Dma Wait Done Status MaskPollBusy time out : %d\n", TimeOutUs);
			return XAIE_ERR;
		}
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

	Addr = _XAie4_DmaGetChannelStatusAddr(DevInst, DmaMod, Loc, ChNum, Dir);

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
		if (Status != XAIE_OK) {
			XAIE_ERROR("Wait for bd task queue MaskPoll timed out : %d\n", TimeOutUs);
			return XAIE_ERR;
		}
	} else {
		Status = XAie_MaskPollBusy(DevInst, Addr, (1U << XAIE4_DMA_STATUS_TASK_Q_SIZE_MSB),
				 0, TimeOutUs);
		if (Status != XAIE_OK) {
			XAIE_ERROR("Wait for bd task queue MaskPollBusy timed out : %d\n", TimeOutUs);
			return XAIE_ERR;
		}
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
AieRC _XAie4_ShimTileDmaCheckBdChValidity(const XAie_DmaMod *DmaMod, XAie_DmaDirection Dir, u16 BdNum, u8 ChNum)
{
	u8 NumChannels = 0;
	switch (Dir) {
		case DMA_S2MM:
			NumChannels = DmaMod->NumChannels;
			break;
		case DMA_MM2S:
			NumChannels = DmaMod->NumMm2sChannels;
			break;
		case DMA_MM2S_CTRL:			
		case DMA_S2MM_TRACE:			
			break;
		default:
			XAIE_ERROR("Invalid Channel direction \n");
			break; 
	}
	
	/* for Ctrl MM2S and Trace S2MM channel */
	if(NumChannels == 0)
		return XAIE_OK;

	/* Each DMA transfer is defined by a DMA Buffer Descriptor (BD). There is a shared pool of 32 BDs:
		• BDs 0-15 are accessible from MM2S channels 0 and 1 and S2MM channel 0
			o In dual-application mode, these are assigned to application A
		• BDs 16-31 are accessible from MM2S channels 2 and 3 and S2MM channel 1
			o In dual-application mode, these are assigned to application B*/

	if((BdNum < 16U) && (ChNum < NumChannels))
		return XAIE_OK;
	else if(((BdNum >= 16U) && (BdNum < 32U)) && (ChNum >= NumChannels))
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

	Addr = _XAie4_DmaGetChannelStatusAddr(DevInst, DmaMod, Loc, ChNum, Dir);
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
 * _XAie4_MemTileDmaUpdateBdLenPvtBuffPool - Updates the buffer descriptor length
 *                                            in the private buffer pool for a memory tile DMA.
 * @DevInst: Pointer to the device instance.
 * @DmaMod: Pointer to the DMA module.
 * @Loc: Location type.
 * @ChNum: Channel number.
 * @Dir: DMA direction.
 * @Len: Length to be updated.
 * @BdNum: Buffer descriptor number.
 *
 * This function updates the length of a buffer descriptor in the private buffer
 * pool for a specified memory tile DMA channel. It takes into account the device
 * instance, DMA module, channel number, direction, location, length, and buffer
 * descriptor number.
 *
 * Return: AieRC - Status of the operation.
*****************************************************************************/
AieRC _XAie4_MemTileDmaUpdateBdLenPvtBuffPool(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
			XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u32 Len, u16 BdNum)
{
	u64 RegAddr = 0;
	u32 RegVal, Mask;
	
	RegAddr = _XAie4_GetMemTileBdBaseAddr(DmaMod, BdNum, ChNum, Dir) +
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

AieRC _XAie4_ShimTileDmaUpdateBdLen_common(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u32 Len, u64 BdBaseAddr)
{
	u32 RegVal, Mask;

	BdBaseAddr |= (DmaMod->BdProp->BufferLen.Idx * 4U) +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* get mask and Value for the buffer_length */
	Mask = DmaMod->BdProp->BufferLen.Mask;

	
	if ((_XAie_CheckPrecisionExceeds(DmaMod->BdProp->BufferLen.Lsb,
			_XAie_MaxBitsNeeded(Len),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RegVal = XAie_SetField(Len, DmaMod->BdProp->BufferLen.Lsb, Mask);

	return XAie_MaskWrite32(DevInst, BdBaseAddr, Mask, RegVal);
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
	u64 BdBaseAddr = 0;

	/*Add App_B base address if BD is from second half of the resources*/
	if (BdNum >= DmaMod->NumBds) {
		BdBaseAddr = XAIE4_MASK_VALUE_APP_B;
		BdNum -= DmaMod->NumBds;
	}
	BdBaseAddr |= (u64)(DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset);

	return _XAie4_ShimTileDmaUpdateBdLen_common(DevInst, DmaMod, Loc, Len, BdBaseAddr);	
}

/*****************************************************************************/
/**
 * _XAie4_ShimTileDmaUpdateBdLenPvtBuffPool - Updates the buffer descriptor length
 *                                             in the private buffer pool for a 
 *                                             specified DMA channel.
 * @DevInst: Pointer to the device instance.
 * @DmaMod: Pointer to the DMA module.
 * @Loc: Location type.
 * @ChNum: DMA channel number.
 * @Dir: Direction of the DMA transfer.
 * @Len: Length to be updated in the buffer descriptor.
 * @BdNum: Buffer descriptor number.
 *
 * This function updates the length of the buffer descriptor in the private
 * buffer pool for a specified DMA channel in the AI Engine. It takes into
 * account the device instance, DMA module, channel number, direction of the
 * transfer, location type, length to be updated, and the buffer descriptor
 * number.
 *
 * Return: A status code indicating success or failure of the operation.
*****************************************************************************/
AieRC _XAie4_ShimTileDmaUpdateBdLenPvtBuffPool(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
				XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u32 Len, u16 BdNum)
{
	u64 BdBaseAddr = 0;
	u16 MaxNumBds = 0;

	if(Dir == DMA_MM2S_CTRL) {
		MaxNumBds = DmaMod->CtrlMm2sProp->NumBds;
		BdBaseAddr = DmaMod->CtrlMm2sProp->BdBaseAddr;
		/*Add App_B base address if ChNum is from second half of the resources*/
		if (ChNum >= DmaMod->CtrlMm2sProp->NumChannels) {
			BdBaseAddr |= XAIE4_MASK_VALUE_APP_B;
			ChNum -= DmaMod->CtrlMm2sProp->NumChannels;
		}
	} else if(Dir == DMA_S2MM_TRACE) {
		MaxNumBds = DmaMod->TraceS2mmProp->NumBds;
		BdBaseAddr = DmaMod->TraceS2mmProp->BdBaseAddr;
	}

	BdBaseAddr = BdBaseAddr +
				(ChNum * (u64)MaxNumBds * (u64)DmaMod->IdxOffset) + 
				(BdNum * (u64)DmaMod->IdxOffset);

	return _XAie4_ShimTileDmaUpdateBdLen_common(DevInst, DmaMod, Loc, Len, BdBaseAddr);	
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
	u64 BdBaseAddr = 0;
	u32 RegVal = 0;
	AieRC RC;

	BdBaseAddr = (u64)(DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset) +
			XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			DmaMod->BdProp->BufferLen.Idx * 4U;
	RC = XAie_Read32(DevInst, BdBaseAddr, &RegVal);
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
 * _XAie4_MemTileDmaGetBdLenPvtBuffPool - Retrieves the length of a buffer descriptor
 *                                         from the private buffer pool in a memory tile DMA.
 * @DevInst: Pointer to the device instance.
 * @DmaMod: Pointer to the DMA module.
 * @Loc: Location type.
 * @ChNum: Channel number.
 * @Dir: DMA direction.
 * @Len: Pointer to store the length of the buffer descriptor.
 * @BdNum: Buffer descriptor number.
 *
 * This function retrieves the length of a specified buffer descriptor from the
 * private buffer pool in a memory tile DMA. The length is stored in the location
 * pointed to by @Len.
 *
 * Return: AieRC - Status of the operation.
*****************************************************************************/
AieRC _XAie4_MemTileDmaGetBdLenPvtBuffPool(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u32 *Len, u16 BdNum)
{
	u64 RegAddr = 0;
	u32 RegVal;
	AieRC RC;

	RegAddr = _XAie4_GetMemTileBdBaseAddr(DmaMod, BdNum, ChNum, Dir) +
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

AieRC _XAie4_ShimTileDmaGetBdLen_common(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u32 *Len, u64 BdBaseAddr)
{
	u32 RegVal;
	AieRC RC;
	
	BdBaseAddr |= XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			DmaMod->BdProp->BufferLen.Idx * 4U;
	RC = XAie_Read32(DevInst, BdBaseAddr, &RegVal);
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
	u64 BdBaseAddr = 0;

	/*Add App_B base address if BD is from second half of the resources*/
	if (BdNum >= DmaMod->NumBds) {
		BdBaseAddr = XAIE4_MASK_VALUE_APP_B;
		BdNum -= DmaMod->NumBds;
	}

	BdBaseAddr |= DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset;

	return _XAie4_ShimTileDmaGetBdLen_common(DevInst, DmaMod, Loc, Len, BdBaseAddr);
}

/*****************************************************************************/
/**
 * _XAie4_ShimTileDmaGetBdLenPvtBuffPool - Retrieves the buffer descriptor length
 * for a given DMA channel in the private buffer pool.
 *
 * @DevInst: Pointer to the device instance.
 * @DmaMod: Pointer to the DMA module.
 * @Loc: Location type.
 * @ChNum: DMA channel number.
 * @Dir: Direction of the DMA transfer.
 * @Len: Pointer to store the length of the buffer descriptor.
 * @BdNum: Buffer descriptor number.
 *
 * This function retrieves the length of the buffer descriptor for a specified
 * DMA channel in the private buffer pool. The length is stored in the location
 * pointed to by @Len.
 *
 * Return: AieRC - Status of the operation.
*****************************************************************************/
AieRC _XAie4_ShimTileDmaGetBdLenPvtBuffPool(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u32 *Len, u16 BdNum)
{
	u16 MaxNumBds = 0;
	u64 BdBaseAddr = 0;

	if(Dir == DMA_MM2S_CTRL) {
		MaxNumBds = DmaMod->CtrlMm2sProp->NumBds;
		BdBaseAddr = DmaMod->CtrlMm2sProp->BdBaseAddr;
		/*Add App_B base address if ChNum is from second half of the resources*/
		if (ChNum >= DmaMod->CtrlMm2sProp->NumChannels) {
			BdBaseAddr |= XAIE4_MASK_VALUE_APP_B;
			ChNum -= DmaMod->CtrlMm2sProp->NumChannels;
		}
	} else if(Dir == DMA_S2MM_TRACE) {
		MaxNumBds = DmaMod->TraceS2mmProp->NumBds;
		BdBaseAddr = DmaMod->TraceS2mmProp->BdBaseAddr;
	}

	BdBaseAddr = BdBaseAddr +
				(ChNum * (u64)MaxNumBds * (u64)DmaMod->IdxOffset) + 
				(BdNum * (u64)DmaMod->IdxOffset);

	return _XAie4_ShimTileDmaGetBdLen_common(DevInst, DmaMod, Loc, Len, BdBaseAddr);
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
 * _XAie4_MemTileDmaUpdateBdAddrPvtBuffPool - Updates the buffer descriptor address
 *                                             in the private buffer pool for a memory tile DMA.
 * @DevInst: Pointer to the device instance.
 * @DmaMod: Pointer to the DMA module.
 * @Loc: Location type.
 * @ChNum: Channel number.
 * @Dir: Direction of the DMA transfer.
 * @Addr: Address to be updated.
 * @BdNum: Buffer descriptor number.
 *
 * This function updates the address of a buffer descriptor in the private buffer
 * pool for a specified memory tile DMA channel and direction.
 *
 * Return: AieRC - Return code indicating success or failure.
*****************************************************************************/
AieRC _XAie4_MemTileDmaUpdateBdAddrPvtBuffPool(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
			XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u64 Addr, u16 BdNum)
{
	u64 RegAddr = 0;
	u32 RegVal, Mask;

	RegAddr = _XAie4_GetMemTileBdBaseAddr(DmaMod, BdNum, ChNum, Dir) +
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

AieRC _XAie4_ShimTileDmaUpdateBdAddr_common(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u64 Addr, u64 BaseAddr)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegVal, Mask;

	BaseAddr |= XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	RegAddr = BaseAddr + (DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Idx * 4U);
	Mask = DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Mask;

	if(((DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb > _XAie_MaxBitsNeeded(Addr)) &&
		(Addr >= (1ULL << DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb))) || 
		(_XAie_CheckPrecisionExceeds(DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb,
			_XAie_MaxBitsNeeded((Addr >> DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb) & 0xFFFFFFFFU),
			MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RegVal = XAie_SetField((Addr >> DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb) & 0xFFFFFFFFU,
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
	u64 BdBaseAddr = 0;

	/*Add App_B base address if BD is from second half of the resources*/
	if (BdNum >= DmaMod->NumBds) {
		BdBaseAddr = XAIE4_MASK_VALUE_APP_B;
		BdNum -= DmaMod->NumBds;
	}
	
	BdBaseAddr |= (DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset);

	return _XAie4_ShimTileDmaUpdateBdAddr_common(DevInst, DmaMod, Loc, Addr, BdBaseAddr);
}

/*****************************************************************************/
/**
 * _XAie4_ShimTileDmaUpdateBdAddrPvtBuffPool - Updates the buffer descriptor address
 * for a private buffer pool in the Shim Tile DMA.
 *
 * @DevInst: Device instance pointer.
 * @DmaMod: Pointer to the DMA module.
 * @Loc: Location type.
 * @ChNum: Channel number.
 * @Dir: DMA direction.
 * @Addr: Address to be updated.
 * @BdNum: Buffer descriptor number.
 *
 * This function updates the address of a buffer descriptor in the private buffer
 * pool for the specified Shim Tile DMA channel and direction.
 *
 * Return: AieRC - Status of the operation.
*****************************************************************************/
AieRC _XAie4_ShimTileDmaUpdateBdAddrPvtBuffPool(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
				XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u64 Addr, u16 BdNum)
{
	u64 BdBaseAddr = 0;
	u16 MaxNumBds = 0;

	if(Dir == DMA_MM2S_CTRL) {
		MaxNumBds = DmaMod->CtrlMm2sProp->NumBds;
		BdBaseAddr = DmaMod->CtrlMm2sProp->BdBaseAddr;
		/*Add App_B base address if ChNum is from second half of the resources*/
		if (ChNum >= DmaMod->CtrlMm2sProp->NumChannels) {
			BdBaseAddr |= XAIE4_MASK_VALUE_APP_B;
			ChNum -= DmaMod->CtrlMm2sProp->NumChannels;
		}
	} else if(Dir == DMA_S2MM_TRACE) {
		MaxNumBds = DmaMod->TraceS2mmProp->NumBds;
		BdBaseAddr = DmaMod->TraceS2mmProp->BdBaseAddr;
	}
	
	BdBaseAddr = BdBaseAddr +
				(ChNum * (u64)MaxNumBds * (u64)DmaMod->IdxOffset) + 
				(BdNum * (u64)DmaMod->IdxOffset);

	return _XAie4_ShimTileDmaUpdateBdAddr_common(DevInst, DmaMod, Loc, Addr, BdBaseAddr);
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
	u8 LockAcqVal, LockRelVal;

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

	if(DmaDesc->LockDesc.LockAcqVal < 0) {
			LockAcqVal = (u8)DmaDesc->LockDesc.LockAcqVal;
			LockAcqVal = (LockAcqVal & XAIE4_LOCK_ACQ_MASK);
	}
	else {
			LockAcqVal = DmaDesc->LockDesc.LockAcqVal;
	}

	if(DmaDesc->LockDesc.LockRelVal < 0) {
			LockRelVal = (u8)DmaDesc->LockDesc.LockRelVal;
			LockRelVal = (LockRelVal & XAIE4_LOCK_ACQ_MASK);
	}
	else {
			LockRelVal = DmaDesc->LockDesc.LockRelVal;
	}
				

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
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockAcqId),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
			_XAie_MaxBitsNeeded(LockRelVal),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
			_XAie_MaxBitsNeeded(LockAcqVal),MAX_VALID_AIE_REG_BIT_INDEX))) {
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
		XAie_SetField(LockRelVal,
				BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
				BdProp->Lock->AieMlDmaLock.LckRelVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelId,
				BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
				BdProp->Lock->AieMlDmaLock.LckRelId.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqEn,
				BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
				BdProp->Lock->AieMlDmaLock.LckAcqEn.Mask) |
		XAie_SetField(LockAcqVal,
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
 * _XAie4_MemTileDmaWriteBdPvtBuffPool - Writes a buffer descriptor to the private buffer pool
 * in the memory tile DMA.
 *
 * @DevInst: Pointer to the device instance.
 * @DmaDesc: Pointer to the DMA descriptor.
 * @Loc: Location type.
 * @ChNum: Channel number.
 * @Dir: Direction of the DMA transfer.
 * @BdNum: Buffer descriptor number.
 *
 * This function writes a buffer descriptor to the private buffer pool in the memory tile DMA
 * for the specified channel and direction.
 *
 * Return: AieRC - Status of the operation.
*****************************************************************************/
AieRC _XAie4_MemTileDmaWriteBdPvtBuffPool(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u16 BdNum)
{
	AieRC RC;
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIE4_MEMTILEDMA_NUM_BD_WORDS] = {0};
	const XAie_DmaMod *DmaMod;	
	const XAie_DmaBdProp *BdProp;
	u8 LockAcqVal, LockRelVal;

	RC = _XAie4_DmaMemTileCheckPaddingConfig(DmaDesc);
	if (RC != XAIE_OK) {
		return RC;
	}

	DmaMod = DevInst->DevProp.DevMod[DmaDesc->TileType].DmaMod;
	BdProp = DmaMod->BdProp;

	BdBaseAddr = _XAie4_GetMemTileBdBaseAddr(DmaMod, BdNum, ChNum, Dir);

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

	if(DmaDesc->LockDesc.LockAcqVal < 0) {
		LockAcqVal = (u8)DmaDesc->LockDesc.LockAcqVal;
		LockAcqVal = (LockAcqVal & XAIE4_LOCK_ACQ_MASK);
	}
	else {
		LockAcqVal = DmaDesc->LockDesc.LockAcqVal;
	}

	if ((_XAie_CheckPrecisionExceeds(BdProp->BufferLen.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AddrDesc.Length),MAX_VALID_AIE_REG_BIT_INDEX)) || 
			(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
			_XAie_MaxBitsNeeded(LockAcqVal),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[1U] = XAie_SetField(LockAcqVal,
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask) |
		XAie_SetField(DmaDesc->AddrDesc.Length,
			BdProp->BufferLen.Lsb,
			BdProp->BufferLen.Mask);

	if(DmaDesc->LockDesc.LockRelVal < 0) {
		LockRelVal = (u8)DmaDesc->LockDesc.LockRelVal;
		LockRelVal = (LockRelVal & XAIE4_LOCK_ACQ_MASK);
	}
	else
		LockRelVal = DmaDesc->LockDesc.LockRelVal;

	if ((_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockAcqId),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockRelId),
			MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckAcqEn.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockAcqEn),
			MAX_VALID_AIE_REG_BIT_INDEX))  ||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
			_XAie_MaxBitsNeeded(LockRelVal),
			MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[2U] = XAie_SetField(DmaDesc->LockDesc.LockAcqId,
			BdProp->Lock->AieMlDmaLock.LckAcqId.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqId.Mask) |
		XAie_SetField(LockRelVal,
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

AieRC _XAie4_ShimDmaWriteBd_common(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, XAie_DmaDirection Dir, u16 BdNum, u64 BdBaseAddr)
{
	u64 Addr;
	AieRC RC;
	u32 BdWord[XAIE4_SHIMDMA_NUM_BD_WORDS];
	XAie_ShimDmaBdArgs Args;
	u8 LockAcqVal, LockRelVal;

	const XAie_DmaBdProp *BdProp = DmaDesc->DmaMod->BdProp;

	/* Setup BdWord with the right values from DmaDesc */

	if ((_XAie_CheckPrecisionExceeds(BdProp->SysProp->AxUser.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AxiDesc.AxUser), MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->SysProp->KeyIdx.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AxiDesc.KeyIdx), MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb,
			_XAie_MaxBitsNeeded((DmaDesc->AddrDesc.Address >> 32U) & 0xFFFFFFFFU),
			MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	BdWord[0U] = XAie_SetField(DmaDesc->AxiDesc.AxUser,
					BdProp->SysProp->AxUser.Lsb,
					BdProp->SysProp->AxUser.Mask) |
			XAie_SetField(DmaDesc->AxiDesc.KeyIdx,
				BdProp->SysProp->KeyIdx.Lsb,
				BdProp->SysProp->KeyIdx.Mask) |
			XAie_SetField((DmaDesc->AddrDesc.Address >> 32U),
				BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb,
				BdProp->Buffer->ShimDmaBuff.AddrHigh.Mask);

	if ((BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb > _XAie_MaxBitsNeeded(DmaDesc->AddrDesc.Address)) &&
		(DmaDesc->AddrDesc.Address >= (1ULL << BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb))) {
		XAIE_ERROR("Loss of precision for Rightshift\n");
		return XAIE_INVALID_ADDRESS;
	}

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
	

	/* BD[4] index is not used for Ctrl MM2S and Trace S2MM channel*/
	if(!((Dir == DMA_MM2S_CTRL) || (Dir == DMA_S2MM_TRACE))){

		if(DmaDesc->LockDesc.LockAcqVal < 0) {
			LockAcqVal = (u8)DmaDesc->LockDesc.LockAcqVal;
			LockAcqVal = (LockAcqVal & XAIE4_LOCK_ACQ_MASK);
		} else {
			LockAcqVal = DmaDesc->LockDesc.LockAcqVal;
		}

		if(DmaDesc->LockDesc.LockRelVal < 0) {
			LockRelVal = (u8)DmaDesc->LockDesc.LockRelVal;
			LockRelVal = (LockRelVal & XAIE4_LOCK_ACQ_MASK);
		} else {
			LockRelVal = DmaDesc->LockDesc.LockRelVal;
		}

		if ((_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
				_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].Wrap),
				MAX_VALID_AIE_REG_BIT_INDEX)) ||
			(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
				_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockRelId), MAX_VALID_AIE_REG_BIT_INDEX)) ||
			(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
				_XAie_MaxBitsNeeded(LockAcqVal), MAX_VALID_AIE_REG_BIT_INDEX)) ||
			(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
				_XAie_MaxBitsNeeded(LockRelVal), MAX_VALID_AIE_REG_BIT_INDEX))) {
			XAIE_ERROR("Check Precision Exceeds Failed\n");
			return XAIE_ERR;
		}

		BdWord[4U] = XAie_SetField(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[0U].Wrap,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb,
				BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask) |
		XAie_SetField(LockRelVal,
				BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
				BdProp->Lock->AieMlDmaLock.LckRelVal.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelId,
				BdProp->Lock->AieMlDmaLock.LckRelId.Lsb,
				BdProp->Lock->AieMlDmaLock.LckRelId.Mask) |
		XAie_SetField(LockAcqVal,
				BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
				BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask);
	}
	
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
		(_XAie_CheckPrecisionExceeds(BdProp->SysProp->DataReuse.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AxiDesc.DataReuse),
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
		XAie_SetField(DmaDesc->AxiDesc.DataReuse,
				BdProp->SysProp->DataReuse.Lsb,
				BdProp->SysProp->DataReuse.Mask) |
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
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[3U].StepSize), MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->SysProp->IOCoherence.Lsb,
				_XAie_MaxBitsNeeded(DmaDesc->AxiDesc.IOCoherence), MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->BdEn->OutofOrderBdId.Lsb,
				_XAie_MaxBitsNeeded(DmaDesc->BdEnDesc.OutofOrderBdId), MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Pkt->PktId.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PktDesc.PktId), MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[8U] = XAie_SetField((DmaDesc->MultiDimDesc.AieMlMultiDimDesc.DimDesc[3U].StepSize),
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Lsb,
			BdProp->AddrMode->AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.IOCoherence,
				BdProp->SysProp->IOCoherence.Lsb,
				BdProp->SysProp->IOCoherence.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.OutofOrderBdId,
				BdProp->BdEn->OutofOrderBdId.Lsb,
				BdProp->BdEn->OutofOrderBdId.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktId,
				BdProp->Pkt->PktId.Lsb,
				BdProp->Pkt->PktId.Mask);

	Addr = BdBaseAddr + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	Args.Loc = Loc;
	Args.VAddr = DmaDesc->AddrDesc.Address;
	Args.BdNum = BdNum;	
	Args.MemInst = DmaDesc->MemInst;
	
	/* for Ctrl MM2S asn Trace S2MM channel the total number of BD words is 8*/
	if(!((Dir == DMA_MM2S_CTRL) || (Dir == DMA_S2MM_TRACE))) {
		Args.Addr = Addr;
		Args.BdWords = &BdWord[0U];
		Args.NumBdWords = XAIE4_SHIMDMA_NUM_BD_WORDS;
		RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_CONFIG_SHIMDMABD, (void *)&Args);
	} else {
		Args.Addr = Addr;
		Args.BdWords = &BdWord[0U];
		Args.NumBdWords = 4;
		RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_CONFIG_SHIMDMABD, (void *)&Args);
		if(RC != XAIE_OK) {
			return RC;
		}

		Args.Addr = Addr + 0x14;
		Args.BdWords = &BdWord[5U];
		Args.NumBdWords = 4;
		RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_CONFIG_SHIMDMABD, (void *)&Args);
	}	
	return RC;
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
	u64 BdBaseAddr = 0;
	const XAie_DmaMod *DmaMod = DmaDesc->DmaMod;
	XAie_DmaDirection Dir = DMA_MAX;

	/*Add App_B base address if BD is from second half of the resources*/
	if (BdNum >= DmaMod->NumBds) {
		BdBaseAddr = XAIE4_MASK_VALUE_APP_B;
		BdNum -= DmaMod->NumBds;
	}

	BdBaseAddr |= DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset;

	return _XAie4_ShimDmaWriteBd_common(DevInst , DmaDesc,
										Loc, Dir, BdNum, BdBaseAddr);
}

/*****************************************************************************/
/**
 * _XAie4_ShimDmaWriteBdPvtBuffPool - Writes a buffer descriptor to the private buffer pool
 * 
 * @DevInst: Device instance pointer
 * @DmaDesc: DMA descriptor pointer
 * @Loc: Location type
 * @ChNum: Channel number
 * @Dir: DMA direction
 * @BdNum: Buffer descriptor number
 * 
 * This function writes a buffer descriptor to the private buffer pool for the specified
 * device instance, DMA descriptor, channel number, direction, location, and buffer descriptor number.
 * 
 * Return: AieRC - Return code indicating success or failure of the operation
*****************************************************************************/
AieRC _XAie4_ShimDmaWriteBdPvtBuffPool(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u16 BdNum)
{	
	u64 BdBaseAddr = 0;	
	u16 MaxNumBds = 0;	
	const XAie_DmaMod *DmaMod = DmaDesc->DmaMod;

	if(Dir == DMA_MM2S_CTRL) {
		MaxNumBds = DmaMod->CtrlMm2sProp->NumBds;
		BdBaseAddr = DmaMod->CtrlMm2sProp->BdBaseAddr;
		/*Add App_B base address if ChNum is from second half of the resources*/
		if (ChNum >= DmaMod->CtrlMm2sProp->NumChannels) {
			BdBaseAddr |= XAIE4_MASK_VALUE_APP_B;
			ChNum -= DmaMod->CtrlMm2sProp->NumChannels;
		}
	} else if(Dir == DMA_S2MM_TRACE) {
		MaxNumBds = DmaMod->TraceS2mmProp->NumBds;
		BdBaseAddr = DmaMod->TraceS2mmProp->BdBaseAddr;
	}

	BdBaseAddr = BdBaseAddr +
				(ChNum * (u64)MaxNumBds * (u64)DmaMod->IdxOffset) + 
				(BdNum * (u64)DmaMod->IdxOffset);

	return _XAie4_ShimDmaWriteBd_common(DevInst , DmaDesc,
										Loc, Dir, BdNum, BdBaseAddr);
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
	u8 LockAcqVal, LockRelVal;

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
	LockRelVal  = (u8)(XAie_GetField(BdWord[5U],
			BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelVal.Mask) & 0xFFU);

	if(LockRelVal > 127)
		DmaDesc->LockDesc.LockRelVal = (s8)LockRelVal;
	else
		DmaDesc->LockDesc.LockRelVal = LockRelVal;

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
	LockAcqVal  = (u8)(XAie_GetField(BdWord[5U],
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask) & 0xFFU);
	if(LockAcqVal > 127)
		DmaDesc->LockDesc.LockAcqVal = (s8)LockAcqVal;
	else
		DmaDesc->LockDesc.LockAcqVal = LockAcqVal;

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
 * _XAie4_MemTileDmaReadBdPvtBuffPool - Reads a buffer descriptor from the private buffer pool
 * in a memory tile DMA.
 *
 * @DevInst: Pointer to the device instance.
 * @DmaDesc: Pointer to the DMA descriptor.
 * @Loc: Location type.
 * @ChNum: Channel number.
 * @Dir: Direction of the DMA transfer.
 * @BdNum: Buffer descriptor number.
 *
 * This function reads a buffer descriptor from the private buffer pool in a memory tile DMA
 * based on the provided parameters.
 *
 * Return: AieRC - Return code indicating success or failure.
*****************************************************************************/
AieRC _XAie4_MemTileDmaReadBdPvtBuffPool(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u16 BdNum)
{
	AieRC RC;
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIE4_MEMTILEDMA_NUM_BD_WORDS];
	const XAie_DmaBdProp *BdProp;
	u8 LockAcqVal, LockRelVal;

	BdProp = DmaDesc->DmaMod->BdProp;
	
	BdBaseAddr = _XAie4_GetMemTileBdBaseAddr(DmaDesc->DmaMod, BdNum, ChNum, Dir);
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
	LockAcqVal = (u8)(XAie_GetField(BdWord[1U],
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask) & 0xFFU);

	if(LockAcqVal > 127)
		DmaDesc->LockDesc.LockAcqVal = (s8)LockAcqVal;
	else
		DmaDesc->LockDesc.LockAcqVal = LockAcqVal;

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
	LockRelVal  = (u8)(XAie_GetField(BdWord[2U],
			BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelVal.Mask) & 0xFFU);

	if(LockRelVal > 127)
		DmaDesc->LockDesc.LockRelVal = (s8)LockRelVal;
	else
		DmaDesc->LockDesc.LockRelVal = LockRelVal;

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

AieRC _XAie4_ShimDmaReadBd_common(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u64 BdBaseAddr)
{
	AieRC RC;
	u64 Addr;
	u32 BdWord[XAIE4_SHIMDMA_NUM_BD_WORDS];
	const XAie_DmaBdProp *BdProp = DmaDesc->DmaMod->BdProp;
	u8 LockAcqVal, LockRelVal;
	
	Addr = BdBaseAddr + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* Setup DmaDesc with values read from bd registers */
	for(u8 i = 0; i < XAIE4_SHIMDMA_NUM_BD_WORDS; i++) {
		RC = XAie_Read32(DevInst, Addr, &BdWord[i]);
		if (RC != XAIE_OK) {
			return RC;
		}
		Addr += 4U;
	}

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->SysProp->AxUser.Lsb,
			BdProp->SysProp->AxUser.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AxiDesc.AxUser = (u64)XAie_GetField(BdWord[0U],
			BdProp->SysProp->AxUser.Lsb,
			BdProp->SysProp->AxUser.Mask) & 0xFFU;

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->SysProp->KeyIdx.Lsb,
			BdProp->SysProp->KeyIdx.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AxiDesc.KeyIdx = (u64)XAie_GetField(BdWord[0U],
			BdProp->SysProp->KeyIdx.Lsb,
			BdProp->SysProp->KeyIdx.Mask) & 0xFFU;

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
	LockRelVal = (u8)(XAie_GetField(BdWord[4U],
			BdProp->Lock->AieMlDmaLock.LckRelVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckRelVal.Mask) & 0xFFU);
		
	if(LockRelVal > 127)
		DmaDesc->LockDesc.LockRelVal = (s8)LockRelVal;
	else
		DmaDesc->LockDesc.LockRelVal = LockRelVal;

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
	LockAcqVal = XAie_GetField(BdWord[4U],
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Lsb,
			BdProp->Lock->AieMlDmaLock.LckAcqVal.Mask) & 0xFFU;
	if(LockAcqVal > 127)
		DmaDesc->LockDesc.LockAcqVal = (s8)LockAcqVal;
	else
		DmaDesc->LockDesc.LockAcqVal = LockAcqVal;


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

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->SysProp->DataReuse.Lsb,
			BdProp->SysProp->DataReuse.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AxiDesc.DataReuse = XAie_GetField(BdWord[6U],
			BdProp->SysProp->DataReuse.Lsb,
			BdProp->SysProp->DataReuse.Mask) & 0xFFU;

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
	
	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->SysProp->IOCoherence.Lsb,
			BdProp->SysProp->IOCoherence.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AxiDesc.IOCoherence = XAie_GetField(BdWord[8U],
			BdProp->SysProp->IOCoherence.Lsb,
			BdProp->SysProp->IOCoherence.Mask) & 0xFFU;

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
	u64 BdBaseAddr = 0;
	const XAie_DmaMod *DmaMod = DmaDesc->DmaMod;

	/*Add App_B base address if BD is from second half of the resources*/
	if (BdNum >= DmaMod->NumBds) {
		BdBaseAddr = XAIE4_MASK_VALUE_APP_B;
		BdNum -= DmaMod->NumBds;
	}

	BdBaseAddr |= DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset;

	return _XAie4_ShimDmaReadBd_common(DevInst , DmaDesc, Loc, BdBaseAddr);
}

/*****************************************************************************/
/**
 * _XAie4_ShimDmaReadBdPvtBuffPool - Reads a buffer descriptor from the private buffer pool
 * in the Shim DMA.
 *
 * @DevInst: Device instance pointer.
 * @DmaDesc: DMA descriptor pointer.
 * @Loc: Location type.
 * @ChNum: Channel number.
 * @Dir: DMA direction.
 * @BdNum: Buffer descriptor number.
 *
 * This function reads a buffer descriptor from the private buffer pool in the Shim DMA
 * for the specified channel number, direction, and location type.
 *
 * Return: AieRC - Status of the operation.
*****************************************************************************/
AieRC _XAie4_ShimDmaReadBdPvtBuffPool(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
			XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir, u16 BdNum)
{
	u64 BdBaseAddr = 0;
	u16 MaxNumBds = 0;

	const XAie_DmaMod *DmaMod = DmaDesc->DmaMod;

	if(Dir == DMA_MM2S_CTRL) {
		MaxNumBds = DmaMod->CtrlMm2sProp->NumBds;
		BdBaseAddr = DmaMod->CtrlMm2sProp->BdBaseAddr;
		/*Add App_B base address if ChNum is from second half of the resources*/
		if (ChNum >= DmaMod->CtrlMm2sProp->NumChannels) {
			BdBaseAddr |= XAIE4_MASK_VALUE_APP_B;
			ChNum -= DmaMod->CtrlMm2sProp->NumChannels;
		}
	} else if(Dir == DMA_S2MM_TRACE) {
		MaxNumBds = DmaMod->TraceS2mmProp->NumBds;
		BdBaseAddr = DmaMod->TraceS2mmProp->BdBaseAddr;
	}

	BdBaseAddr = BdBaseAddr +
				(ChNum * (u64)MaxNumBds * (u64)DmaMod->IdxOffset) + 
				(BdNum * (u64)DmaMod->IdxOffset);

	return  _XAie4_ShimDmaReadBd_common(DevInst , DmaDesc, Loc, BdBaseAddr);
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

/*****************************************************************************/
/**
*
* This API checks for correct Burst length.
*
* @param	BurstLen: Burst length to check if it has correct value or not.
* @param	AxiBurstLen: Based on BurstLen initialize AxiBurstLen Parameter
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAie4_AxiBurstLenCheck(u8 BurstLen, u8 *AxiBurstLen)
{
        switch (BurstLen) {
        case 1:
                *AxiBurstLen = 0;
                return XAIE_OK;
        case 2:
                *AxiBurstLen = 1;
                return XAIE_OK;
        case 4:
                *AxiBurstLen = 2;
                return XAIE_OK;
        case 8:
                *AxiBurstLen = 3;
                return XAIE_OK;
        default:
                return XAIE_INVALID_BURST_LENGTH;
        }
}

/*AieRC _XAie4_AxiBurstLenCheck(u8 BurstLen)
{
	switch (BurstLen) {
	case 1:
	case 2:
	case 4:
	case 8:
		return XAIE_OK;
	default:
		return XAIE_INVALID_BURST_LENGTH;
	}
}*/

/*****************************************************************************/
/**
*
* This API initializes the Acquire and Release Locks for a a Dma for AIE4
* descriptor.
*
* @param        DmaDesc: Initialized Dma Descriptor.
* @param        Acq: Lock object with acquire lock ID and lock value.
* @param        Rel: Lock object with release lock ID and lock value.
* @param        AcqEn: XAIE_ENABLE/XAIE_DISABLE for enable or disable acquire
*               lock.
* @param        RelEn: XAIE_ENABLE/XAIE_DISABLE for enable or disable release
*               lock.
*
* @return       XAIE_OK on success, Error code on failure.
*
* @note         Internal Only. Should not be called directly. This function is
*               called from the internal Dma Module data structure.
*
******************************************************************************/
AieRC _XAie4_DmaSetLock(XAie_DmaDesc *DmaDesc, XAie_Lock Acq, XAie_Lock Rel,
                u8 AcqEn, u8 RelEn)
{
        const XAie_DmaMod *DmaMod;
        const XAie_LockMod *LockMod;


        DmaMod = DmaDesc->DmaMod;
        LockMod = DmaDesc->LockMod;


        /* This is special case for Dual App mode. In dual App, resource values will be half but
           for DMA to access Lock Indexes which are local to Tile, the numbers has to be physical
           So that adding hardcoded values to check error condition. */
        if ((DmaDesc->TileType == XAIEGBL_TILE_TYPE_MEMTILE) &&
                ((DmaDesc->AppMode == XAIE_DEVICE_DUAL_APP_MODE_A) ||
                (DmaDesc->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B))) {
                if (Acq.LockId < DMA_MEMTILE_LOCAL_LOCK_LOW ||
			Acq.LockId > DMA_MEMTILE_LOCAL_LOCK_HIGH) {
                        XAIE_ERROR("Invalid Lock\n");
                        return XAIE_INVALID_LOCK_ID;
                }
        }
        else {
                if (Acq.LockId >= DmaMod->NumLocks) {
                        XAIE_ERROR("Invalid Lock\n");
                        return XAIE_INVALID_LOCK_ID;
                }
        }

        if((Acq.LockVal > LockMod->LockValUpperBound) ||
           (Rel.LockVal > LockMod->LockValUpperBound)) {
                XAIE_ERROR("Invalid Lock Value \n");
                return XAIE_INVALID_LOCK_ID;
        }


        DmaDesc->LockDesc.LockAcqId = Acq.LockId;
        DmaDesc->LockDesc.LockRelId = Rel.LockId;
        DmaDesc->LockDesc.LockAcqEn = AcqEn;
        DmaDesc->LockDesc.LockRelEn = RelEn;
        DmaDesc->LockDesc.LockRelVal = Rel.LockVal;
        DmaDesc->LockDesc.LockAcqVal = Acq.LockVal;

        return XAIE_OK;
}


#endif /* XAIE_FEATURE_DMA_ENABLE */

/** @} */
