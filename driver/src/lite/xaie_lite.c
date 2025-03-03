/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
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
#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE && XAIE_FEATURE_LITE */
/** @} */
