/******************************************************************************
* Copyright (C) 2023 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_ss_aie2ps.h
* @{
*
* This file contains internal api implementations for AIE4 stream switch.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who         Date        Changes
* ----- ---------   ----------  -----------------------------------------------
* 1.0   jbaniset    11/21/2023  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_SS_AIE2PS_H
#define XAIE_SS_AIE2PS_H

/***************************** Include Files *********************************/
#include "xaie_helper.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes  *****************************/

AieRC _XAie4_AieTile_StrmSwCheckPortValidity(XAie_DevInst *DevInst,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum);
AieRC _XAie4_MemTile_StrmSwCheckPortValidity(XAie_DevInst *DevInst,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum);
AieRC _XAie4_ShimTile_StrmSwCheckPortValidity(XAie_DevInst *DevInst,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum);
AieRC _XAie4_StrmSw32bCheckPortValidity(XAie_DevInst *DevInst,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum);
#endif /* XAIE_SS_AIE2PS_H */
/** @} */
