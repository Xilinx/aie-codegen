/******************************************************************************
 * * Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
 * * SPDX-License-Identifier: MIT
 * ******************************************************************************/


/*****************************************************************************/
/**
 * * @file xaie_device_aie4.h
 * * @{
 * *
 * * This file contains the apis for device specific operations of aie4.
 * *
 * * <pre>
 * * MODIFICATION HISTORY:
 * *
 * * Ver   Who     Date     Changes
 * * ----- ------  -------- -----------------------------------------------------
 * * 1.0   Ramakant   11/25/2023  Initial creation
 * * </pre>
 * *
 * ******************************************************************************/
#ifndef XAIE_DEVICE_AIE4
#define XAIE_DEVICE_AIE4

/***************************** Include Files *********************************/
/************************** Function Prototypes  *****************************/
u8 _XAie4_GetTTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC _XAie4_PartMemZeroInit(XAie_DevInst *DevInst);
AieRC _XAie4_RequestTiles(XAie_DevInst *DevInst, XAie_BackendTilesArray *Args);
AieRC _XAie4_SetPartIsolationAfterRst(XAie_DevInst *DevInst);
AieRC _XAie4_SetPartApplicationReset(XAie_DevInst *DevInst, u8 Enable);
AieRC _XAie4_SetUCMemoryPrivileged(XAie_DevInst *DevInst, u8 Enable);
AieRC _XAie4_ZeroInitUcMemory(XAie_DevInst *DevInst);
AieRC _XAie4_PartInitL2Split(XAie_DevInst *DevInst);
AieRC _XAie4_SetDualAppModePrivileged(XAie_DevInst *DevInst, XAie_BackendTilesArray *Args);
#endif /* XAIE_DEVICE_AIE4 */
/** @} */
