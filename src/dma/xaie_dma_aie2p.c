/******************************************************************************
* Copyright (C) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_dma_aie2p.c
* @{
*
* This file contains routines for AIE2P DMA configuration and controls. This
* header file is not exposed to the user.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who        Date        Changes
* ----- ------     --------    -----------------------------------------------------
* 1.0   jbaniset   16/02/2024  Initial creation
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaiegbl.h"

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API checks for correct Burst length.
*
* @param	BurstLen: Burst length to check if it has correct value or not.
* @param        AxiBurstLen: Based on BurstLen initialize AxiBurstLen Parameter
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAie2P_AxiBurstLenCheck(u8 BurstLen, u8 *AxiBurstLen)
{
        switch (BurstLen) {
        case 4:
                *AxiBurstLen = 0;
                return XAIE_OK;
        case 8:
                *AxiBurstLen = 1;
                return XAIE_OK;
        case 16:
                *AxiBurstLen = 2;
                return XAIE_OK;
        case 32:
                *AxiBurstLen = 3;
                return XAIE_OK;
        default:
                return XAIE_INVALID_BURST_LENGTH;
        }
}
