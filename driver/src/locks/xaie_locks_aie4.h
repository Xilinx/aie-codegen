/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_locks_aie4.h
* @{
*
* This file contains function prototypes for AIE4 locks. This header file is
* not exposed to the user.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Keyur   01/12/2024  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIELOCKS_AIE4_H
#define XAIELOCKS_AIE4_H
/***************************** Include Files *********************************/
#include "xaie_locks.h"
#include "xaiegbl_defs.h"
#include "xaie_helper.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes  *****************************/
AieRC _XAie4_LockAcquire(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
		XAie_LocType Loc, XAie_Lock Lock, u32 TimeOut, u8 BusyPoll);
AieRC _XAie4_LockRelease(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
		XAie_LocType Loc, XAie_Lock Lock, u32 TimeOut, u8 BusyPoll);
AieRC _XAie4_LockSetValue(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
		XAie_LocType Loc, XAie_Lock Lock);
AieRC _XAie4_LockGetValue(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
		XAie_LocType Loc, XAie_Lock Lock, u32 *LockVal);

#endif
/** @} */
