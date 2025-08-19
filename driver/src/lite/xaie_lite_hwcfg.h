/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite_hwcfg.h
* @{
*
* This header file capture hardware configurations for all supported devices.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who       Date        Changes
* ----- --------  --------    ------------------------------------------------
* 1.0  Nishad     06/23/2022  Initial creation
* 1.1  avasi      11/16/2022  Additions for Status Dump of Core Tile, Mem Tile
*                             and Shim Tile registers.
* 1.2  Ramakant   01/01/2024  Additions for AIE4 Core, Mem and Shim time registers
* </pre>
*
******************************************************************************/
#ifndef XAIE_LITE_HWCFG_H
#define XAIE_LITE_HWCFG_H

#ifdef XAIE_FEATURE_LITE

#define XAIE_LITE_AIE4_AIE_TILE_SHIFT_OFFSET 4
#define XAIE_LITE_AIE4_MEM_TILE_SHIFT_OFFSET 5


/***************************** Include Files *********************************/
/************************** Constant Definitions *****************************/
#if XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE

#ifndef XAIE_BASE_ADDR
#define XAIE_BASE_ADDR			0x20000000000
#endif

#ifndef XAIE_NPI_BASEADDR
#define XAIE_NPI_BASEADDR		0xF70A0000
#endif

#ifndef XAIE_NUM_ROWS
#define XAIE_NUM_ROWS			9
#endif

#ifndef XAIE_NUM_COLS
#define XAIE_NUM_COLS			50
#endif

#define XAIE_TILE_NUM_LOCKS     	16
#define XAIE_SHIM_NUM_LOCKS     	16

#define XAIE_TILE_DMA_NUM_CH		2U
#define XAIE_SHIM_DMA_NUM_CH		2U

#define XAIE_COL_SHIFT			23
#define XAIE_ROW_SHIFT			18
#define XAIE_SHIM_ROW			0
#define XAIE_SHIM_NUM_ROWS          1
#define XAIE_MEM_TILE_ROW_START		0
#define XAIE_MEM_TILE_NUM_ROWS		0
#define XAIE_AIE_TILE_ROW_START     1
#define XAIE_AIE_TILE_NUM_ROWS      8
#define XAIE_NUM_NOC_INTR_OFFSET	1

#define XAIE_CORE_TILE_NUM_EVENT_STATUS_REGS 4
#define XAIE_SHIM_TILE_NUM_EVENT_STATUS_REGS 4

#elif XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIEML

#ifndef XAIE_BASE_ADDR
#define XAIE_BASE_ADDR			0x20000000000
#endif

#ifndef XAIE_NPI_BASEADDR
#define XAIE_NPI_BASEADDR		0xF6D10000
#endif

#ifndef XAIE_NUM_ROWS
#define XAIE_NUM_ROWS			11
#endif

#ifndef XAIE_NUM_COLS
#define XAIE_NUM_COLS			38
#endif

#define XAIE_TILE_NUM_LOCKS     	16
#define XAIE_MEM_TILE_NUM_LOCKS 	64
#define XAIE_SHIM_NUM_LOCKS     	16

#define XAIE_TILE_DMA_NUM_CH		2U
#define XAIE_MEM_TILE_DMA_NUM_CH	6U
#define XAIE_SHIM_DMA_NUM_CH		2U

#define XAIE_COL_SHIFT			25
#define XAIE_ROW_SHIFT			20
#define XAIE_SHIM_ROW			0
#define XAIE_SHIM_NUM_ROWS		1
#define XAIE_MEM_TILE_ROW_START		1
#define XAIE_MEM_TILE_NUM_ROWS		2
#define XAIE_AIE_TILE_ROW_START     3
#define XAIE_AIE_TILE_NUM_ROWS      8
#define XAIE_NUM_NOC_INTR_OFFSET	1

#define XAIE_CORE_TILE_NUM_EVENT_STATUS_REGS    4
#define XAIE_MEM_TILE_NUM_EVENT_STATUS_REGS     6
#define XAIE_SHIM_TILE_NUM_EVENT_STATUS_REGS    4


#elif XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE2IPU

#ifdef __AIEIPU__
#include <platform-hw-config.h>
#endif

#ifndef XAIE_BASE_ADDR
#define XAIE_BASE_ADDR			IPU_AIE_BASEADDR
#endif

#ifndef XAIE_NPI_BASEADDR
#define XAIE_NPI_BASEADDR		IPU_AIE_NPI_ADDR
#endif

#ifndef XAIE_NUM_ROWS
#define XAIE_NUM_ROWS			6
#endif

#ifndef XAIE_NUM_COLS
#define XAIE_NUM_COLS			5
#endif

#define XAIE_TILE_NUM_LOCKS     	16
#define XAIE_MEM_TILE_NUM_LOCKS 	64
#define XAIE_SHIM_NUM_LOCKS     	16

#define XAIE_TILE_DMA_NUM_CH		2U
#define XAIE_MEM_TILE_DMA_NUM_CH	6U
#define XAIE_SHIM_DMA_NUM_CH		2U

#define XAIE_COL_SHIFT			25
#define XAIE_ROW_SHIFT			20
#define XAIE_SHIM_ROW			0
#define XAIE_SHIM_NUM_ROWS		1
#define XAIE_MEM_TILE_ROW_START		1
#define XAIE_MEM_TILE_NUM_ROWS		1

#define XAIE_AIE_TILE_ROW_START     2
#define XAIE_AIE_TILE_NUM_ROWS      4

#define XAIE_CORE_TILE_NUM_EVENT_STATUS_REGS    4
#define XAIE_MEM_TILE_NUM_EVENT_STATUS_REGS     6
#define XAIE_SHIM_TILE_NUM_EVENT_STATUS_REGS    4

#elif ((XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE2P) || \
		(XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE2P_STRIX_A0))

#ifdef __AIEIPU__
#include <platform-hw-config.h>
#endif

#ifndef XAIE_BASE_ADDR
#define XAIE_BASE_ADDR			IPU_AIE_BASEADDR
#endif

#ifndef XAIE_NPI_BASEADDR
#define XAIE_NPI_BASEADDR		IPU_AIE_NPI_ADDR
#endif

#ifndef XAIE_NUM_ROWS
#define XAIE_NUM_ROWS			6
#endif

#ifndef XAIE_NUM_COLS
#define XAIE_NUM_COLS			4
#endif


#define XAIE_TILE_NUM_LOCKS     	16
#define XAIE_MEM_TILE_NUM_LOCKS 	64
#define XAIE_SHIM_NUM_LOCKS     	16

#define XAIE_TILE_DMA_NUM_CH		2U
#define XAIE_MEM_TILE_DMA_NUM_CH	6U
#define XAIE_SHIM_DMA_NUM_CH		2U

#define XAIE_COL_SHIFT			25
#define XAIE_ROW_SHIFT			20
#define XAIE_SHIM_ROW			0
#define XAIE_SHIM_NUM_ROWS          1
#define XAIE_MEM_TILE_ROW_START		1
#define XAIE_MEM_TILE_NUM_ROWS		1
#define XAIE_AIE_TILE_ROW_START     2
#define XAIE_AIE_TILE_NUM_ROWS      4

#define XAIE_CORE_TILE_NUM_EVENT_STATUS_REGS    4
#define XAIE_MEM_TILE_NUM_EVENT_STATUS_REGS     6
#define XAIE_SHIM_TILE_NUM_EVENT_STATUS_REGS    4

#elif XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE2P_STRIX_B0

#ifdef __AIEIPU__
#include <platform-hw-config.h>
#endif

#ifndef XAIE_BASE_ADDR
#define XAIE_BASE_ADDR			IPU_AIE_BASEADDR
#endif

#ifndef XAIE_NPI_BASEADDR
#define XAIE_NPI_BASEADDR		IPU_AIE_NPI_ADDR
#endif

#ifndef XAIE_NUM_ROWS
#define XAIE_NUM_ROWS			6
#endif

#ifndef XAIE_NUM_COLS
#define XAIE_NUM_COLS			8
#endif


#define XAIE_TILE_NUM_LOCKS     	16
#define XAIE_MEM_TILE_NUM_LOCKS 	64
#define XAIE_SHIM_NUM_LOCKS     	16

#define XAIE_TILE_DMA_NUM_CH		2U
#define XAIE_MEM_TILE_DMA_NUM_CH	6U
#define XAIE_SHIM_DMA_NUM_CH		2U

#define XAIE_COL_SHIFT			25
#define XAIE_ROW_SHIFT			20
#define XAIE_SHIM_ROW			0
#define XAIE_SHIM_NUM_ROWS          1
#define XAIE_MEM_TILE_ROW_START		1
#define XAIE_MEM_TILE_NUM_ROWS		1
#define XAIE_AIE_TILE_ROW_START     2
#define XAIE_AIE_TILE_NUM_ROWS      4

#define XAIE_CORE_TILE_NUM_EVENT_STATUS_REGS    4
#define XAIE_MEM_TILE_NUM_EVENT_STATUS_REGS     6
#define XAIE_SHIM_TILE_NUM_EVENT_STATUS_REGS    4


#elif ((XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE4_GENERIC) || \
	   (XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE4) || \
	   (XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE4_A))

//To-Do - Update register count ,address and other details with new specification if required
#ifdef __AIEIPU__
#include <platform-hw-config.h>
#endif

#ifndef XAIE_BASE_ADDR
#define XAIE_BASE_ADDR			0x20000000000
#endif

#ifndef XAIE_NPI_BASEADDR
#define XAIE_NPI_BASEADDR		0xF70A0000
#endif

#ifndef XAIE_NUM_ROWS
#define XAIE_NUM_ROWS			6
#endif

#ifndef XAIE_NUM_COLS
#define XAIE_NUM_COLS			3
#endif


#define XAIE_TILE_NUM_LOCKS     	16
#define XAIE_MEM_TILE_NUM_LOCKS 	32
#define XAIE_SHIM_NUM_LOCKS     	16

#define XAIE_TILE_DMA_NUM_CH		3U
#define XAIE_TILE_DMA_MM2S_NUM_CH	1U
#define XAIE_TILE_DMA_S2MM_NUM_CH	2U

#define XAIE_MEM_TILE_DMA_MM2S_NUM_CH	10U
#define XAIE_MEM_TILE_DMA_S2MM_NUM_CH	8U

#define XAIE_SHIM_DMA_S2MM_NUM_CH	2U
#define XAIE_SHIM_DMA_MM2S_NUM_CH	4U

#define XAIE_COL_SHIFT			25
#define XAIE_ROW_SHIFT			20
#define XAIE_SHIM_ROW			0
#define XAIE_SHIM_NUM_ROWS          	1
#define XAIE_SHIM_TILE_NUM_ROWS_SOUTH   1
#define XAIE_SHIM_TILE_NUM_ROWS_NORTH   0
#define XAIE_MEM_TILE_ROW_START		1
#define XAIE_MEM_TILE_NUM_ROWS		1
#define XAIE_AIE_TILE_ROW_START     	2
#define XAIE_AIE_TILE_NUM_ROWS      	4

#define XAIE_CORE_TILE_NUM_EVENT_STATUS_REGS    8
#define XAIE_MEM_TILE_NUM_EVENT_STATUS_REGS     7
#define XAIE_SHIM_TILE_NUM_EVENT_STATUS_REGS    8

#define XAIE_SHIM_ROW_START_OFFSET		0X00010000
#define XAIE_SHIM_ROW_END_OFFSET		0X000E8074
#define XAIE_MEM_ROW_START_OFFSET		0x00000000
#define XAIE_MEM_ROW_END_OFFSET			0x005D2000
#define XAIE_AIE_ROW_START_OFFSET		0x00000000
#define XAIE_AIE_ROW_END_OFFSET			0x000F0030

#else
#include <xaie_custom_device.h>
#endif

#endif		/* XAIE_FEATURE_LITE */
#endif		/* end of protection macro */

/** @} */
