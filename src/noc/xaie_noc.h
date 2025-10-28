/******************************************************************************
* Copyright (C) 2023-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_noc.h
* @{
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
#ifndef XAIENOC_H
#define XAIENOC_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
/**************************** Type Definitions *******************************/


/************************** Function Prototypes  *****************************/
XAIE_AIG_EXPORT  AieRC XAie_EnableShimDmaToAieStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
XAIE_AIG_EXPORT  AieRC XAie_EnableAieToShimDmaStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
XAIE_AIG_EXPORT  AieRC XAie_EnableNoCToAieStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
XAIE_AIG_EXPORT  AieRC XAie_EnableAieToNoCStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
XAIE_AIG_EXPORT  AieRC XAie_EnablePlToAieStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
XAIE_AIG_EXPORT  AieRC XAie_EnableAieToPlStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
#endif		/* end of protection macro */
