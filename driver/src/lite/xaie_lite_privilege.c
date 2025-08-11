/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite_privilege.c
* @{
*
* This file contains lite version of AI engine partition management operations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Wendy 05/17/2021  Initial creation
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xaie_feature_config.h"

#if defined(XAIE_FEATURE_PRIVILEGED_ENABLE) && defined(XAIE_FEATURE_LITE)

#include "xaie_lite.h"
#include "xaie_lite_io.h"
#include "xaie_lite_internal.h"
#include "xaiegbl_defs.h"
#include "xaiegbl.h"
#include "xaie_helper.h"

/***************************** Macro Definitions *****************************/
#define XAIE_ISOLATE_EAST_MASK	(1U << 3)
#define XAIE_ISOLATE_NORTH_MASK	(1U << 2)
#define XAIE_ISOLATE_WEST_MASK	(1U << 1)
#define XAIE_ISOLATE_SOUTH_MASK	(1U << 0)
#define XAIE_ISOLATE_ALL_MASK	((1U << 4) - 1)

#define XAIE_APP_MODE_SHIFT     8U
#define XAIE_L2_SPLIT_SHIFT     10U

#define XAIE_CORE_TILE_GROUP_ERRORS0_EVENT_NUM          0x56     // Group_Error0 = 86 ; Page 210 in spec version v1.3
#define XAIE_CORE_TILE_GROUP_ERRORS1_EVENT_NUM          0x57     // Group_Error0 = 87 ; Page 210 in spec version v1.3
#define XAIE_CORE_TILE_GROUP_ERRORS2_EVENT_NUM          0x58     // Group_Error0 = 88 ; Page 210 in spec version v1.3
#define XAIE_MEM_TILE_GROUP_ERRORS_EVENT_NUM            0xA2     // Group_Errors = 162 ; Page 298 in spec version v1.3
#define XAIE_SHIM_TILE_GROUP_ERRORS_EVENT_NUM           0x98     // Group_Errors = 152 ; Page 338 in spec version v1.3

#define BC_DIR_UNBLOCK		0
#define BC_DIR_BLOCK		1

/***
 * Interrupt_controller_enable
 * BIT-16  Enable_uC_Core	Enable the uC-Core interrupt by setting its mask bit
 * BIT-15  Enable_BC15	        Enable the Broadcast channel 15 by setting its mask bit
 * ...
 * BIT-0   Enable_BC0           Enable the Broadcast channel 15 by setting its mask bit 
 ***/
#define XAIE_L2_INTR_ENABLE_ALL_SOURCES                 0x1FFFF
/***
 * Event_Group_Errors_Enable_A
 * SHIM Tile Group Error Event Number = 152 (153 to 169)
 *
 * [153,154,155,156,157,158,160,161,162,163,164,167,168]
 *
 * BIT-00 	153		AXI_MM_Subordinate_Tile_Error       Set
 * BIT-01 	154		Control_Pkt_Error                   Set
 * BIT-02 	155		Stream_Switch_Parity_Error          Set
 * BIT-03 	156		NSU_Error                           Set
 * BIT-04 	157	 	DMA_Error                           Set
 * BIT-05 	158	 	Lock_Error                          Set
 * BIT-06 	159		DMA_task_token_stall                Set
 * BIT-07 	160		DMA_HW_Error                        Set
 * BIT-08 	161		uC_Module_A_Error                   Set
 * BIT-09 	162		uC_Module_B_Error                   Set
 * BIT-10 	163		uC_Module_A_AXI_MM_Error            Set
 * BIT-11 	164		uC_Module_B_AXI_MM_Error            Set
 * BIT-12 	165		uC_Module_A_ECC_Error_1bit          Set
 * BIT-13 	166		uC_Module_B_ECC_Error_1bit          Set
 * BIT-14 	167		uC_Module_A_ECC_Error_2bit          Set
 * BIT-15 	168		uC_Module_B_ECC_Error_2bit          Set
 ****/
#define XAIE_SHIM_TILE_GROUP_ERROR_VALUE                0xFFFF

/***
 * Event_Group_Error_Enable_A
 * Mem Tile Group Error Event Number = 162 (163 to 175)
 *
 * [164,166,167,168,169,170,171,172]
 *
 * BIT-00 	163		DM_ECC_Error_Scrub_Corrected        Set
 * BIT-01 	164		DM_ECC_Error_Scrub_2bit             Set
 * BIT-02 	165		DM_ECC_Error_1bit                   Set
 * BIT-03 	166		DM_ECC_Error_2bit                   Set
 * BIT-04 	167	 	DMA_S2MM_Error                      Set
 * BIT-05 	168	 	DMA_MM2S_Error                      Set
 * BIT-06 	169		Stream_Switch_Port_Parity_Error     Set
 * BIT-07 	170		Control_Pkt_Error                   Set
 * BIT-08 	171		AXI_MM_Subordinate_Error            Set
 * BIT-09 	172		Lock_Error                          Set
 * BIT-10 	173		DMA_Task_Token_Stall                Set
 ****/
#define XAIE_MEM_TILE_GROUP_ERROR_VALUE                 0x7FF

/***
 * Event_Group_Errors0_Enable	86
 * Event_Group_Errors1_Enable	87
 * Event_Group_Errors2_Enable	88
 * Core Tile Group Error Event Number = (89 to 120)
 *
 * [95,96,97,98,100,102,103,104,105,108,109,110,112,114,115,116]
 *
 * BIT-00 	89		SRS_Overflow                        Set
 * BIT-01 	90		UPS_Overflow                        Set
 * BIT-02 	91		FP_Huge                             Set
 * BIT-03 	92		Int_FP_Zero                         Set
 * BIT-04 	93	 	FP_Invalid                          Set
 * BIT-05 	94	 	FP_INF                              Set
 * BIT-06 	95		Control_Pkt_Error                   Set
 * BIT-07 	96		AXI_MM_Subordinate_Error            Set
 * BIT-08 	97		Core_Fatal_Error                    Set
 * BIT-09 	98		DM_address_out_of_range             Set
 * BIT-10 	99		PM_ECC_Error_Scrub_Corrected        Set
 * BIT-11 	100		PM_ECC_Error_Scrub_2bit             Set
 * BIT-12 	101		PM_ECC_Error_1bit                   Set
 * BIT-13 	102		PM_ECC_Error_2bit                   Set
 * BIT-14 	103		PM_address_out_of_range             Set
 * BIT-15 	104		DM_access_to_Unavailable            Set
 * BIT-16 	105		Lock_Access_to_Unavailable          Set
 * BIT-17 	106		Instr_Warning                       Set
 * BIT-18 	107		Instr_Error                         Set
 * BIT-19 	108		Data_Error                          Set
 * BIT-20 	109		Stream_Switch_Port_Parity_Error     Set
 * BIT-21 	110		Processor_Bus_Error                 Set
 * BIT-22 	111		DM_ECC_Error_Scrub_Corrected        Set
 * BIT-23 	112		DM_ECC_Error_Scrub_2bit             Set
 * BIT-24 	113		DM_ECC_Error_1bit                   Set
 * BIT-25 	114		DM_ECC_Error_2bit                   Set
 * BIT-26 	115		DM_Parity_Error                     Set
 * BIT-27 	116		DMA_Error                           Set
 * BIT-28 	117		Lock_Error                          Set
 * BIT-29 	118		DMA_task_token_stall                Set
 ****/
#define XAIE_CORE_TILE_GROUP_ERROR_VALUE                0x3FFFFFFF

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* This API sets up the error network for the given device partition to be used
* by MPNPU self test.
*
* @param        DevInst: Device Instance
*
* @return       AieRC
*               - XAIE_OK on success
*
* @note         It is implemented to support MPNPU self test framework.
*
******************************************************************************/
AieRC XAie_SetupErrorNetwork(XAie_DevInst *DevInst)
{
	u64 RegAddr;

#if DEV_GEN_AIE4
	XAie_SetColumnClk(DevInst, XAIE_ENABLE);

	// Event Broadcast Network Configuration for each column
	for (int col = 0; col < DevInst->NumCols; col++) {
		// Row-0 : Shim Tile
		// Configure Shim-Tile to enable all interrupt line 16 BC and 1 uC
		RegAddr = _XAie_LGetTileAddr(0, 0) + ((DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) ?
			XAIE_NOC_MOD_INTR_L2_APP_B_ENABLE : XAIE_NOC_MOD_INTR_L2_ENABLE);
		_XAie_LPartWrite32(DevInst, RegAddr, XAIE_L2_INTR_ENABLE_ALL_SOURCES);

		// Configure Shim-Tile to Block Events in all directions (North East South and West)
		RegAddr = _XAie_LGetTileAddr(0, col) + ((DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) ?
			XAIE_PL_MOD_EVENT_BROADCAST_B_BLOCK_SOUTH_SET : XAIE_PL_MOD_EVENT_BROADCAST_A_BLOCK_SOUTH_SET);
		_XAie_LPartWrite32(DevInst, RegAddr, BC_DIR_UNBLOCK); // UnBlock South

		RegAddr = _XAie_LGetTileAddr(0, col) + ((DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) ?
			XAIE_PL_MOD_EVENT_BROADCAST_B_BLOCK_NORTH_SET : XAIE_PL_MOD_EVENT_BROADCAST_A_BLOCK_NORTH_SET);
		_XAie_LPartWrite32(DevInst, RegAddr, BC_DIR_BLOCK); // Block North

		if (DevInst->AppMode != XAIE_DEVICE_DUAL_APP_MODE_B) {
			RegAddr = _XAie_LGetTileAddr(0, col) +  XAIE_PL_MOD_EVENT_BROADCAST_A_BLOCK_WEST_SET;
			_XAie_LPartWrite32(DevInst, RegAddr, BC_DIR_BLOCK); // Block West

			RegAddr = _XAie_LGetTileAddr(0, col) +  XAIE_PL_MOD_EVENT_BROADCAST_A_BLOCK_EAST_SET;
			_XAie_LPartWrite32(DevInst, RegAddr, BC_DIR_BLOCK); // Block East
		}

		// Row-1 : Mem Tile
		// Configure Mem-Tile to Block Events in West, North and East
		RegAddr = _XAie_LGetTileAddr(1, col) + ((DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) ?
			XAIE_MEM_TILE_EVENT_BROADCAST_B_BLOCK_SOUTH_SET : XAIE_MEM_TILE_EVENT_BROADCAST_A_BLOCK_SOUTH_SET);
		_XAie_LPartWrite32(DevInst, RegAddr, BC_DIR_UNBLOCK); // UnBlock South

		RegAddr = _XAie_LGetTileAddr(1, col) + ((DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) ?
			XAIE_MEM_TILE_EVENT_BROADCAST_B_BLOCK_NORTH_SET : XAIE_MEM_TILE_EVENT_BROADCAST_A_BLOCK_NORTH_SET);
		_XAie_LPartWrite32(DevInst, RegAddr, BC_DIR_BLOCK); // Block North

		if (DevInst->AppMode != XAIE_DEVICE_DUAL_APP_MODE_B) {
			RegAddr = _XAie_LGetTileAddr(1, col) + XAIE_MEM_TILE_EVENT_BROADCAST_A_BLOCK_WEST_SET;
			_XAie_LPartWrite32(DevInst, RegAddr, BC_DIR_BLOCK); // Block West

			RegAddr = _XAie_LGetTileAddr(1, col) +  XAIE_MEM_TILE_EVENT_BROADCAST_A_BLOCK_EAST_SET;
			_XAie_LPartWrite32(DevInst, RegAddr, BC_DIR_BLOCK); // Block East
		}

		// Row-2 and Above : Core Tile
		// Configure Core-Tile to Block Events in West, North and East
		for (int row = XAIE_AIE_TILE_ROW_START; row < XAIE_NUM_ROWS; row++) {
			RegAddr = _XAie_LGetTileAddr(row, col) + XAIE_AIE_TILE_EVENT_BROADCAST_BLOCK_SOUTH_SET;
			_XAie_LPartWrite32(DevInst, RegAddr, BC_DIR_UNBLOCK); // UnBlock South

			RegAddr = _XAie_LGetTileAddr(row, col) + XAIE_AIE_TILE_EVENT_BROADCAST_BLOCK_WEST_SET;
			_XAie_LPartWrite32(DevInst, RegAddr, BC_DIR_BLOCK); // Block West

			RegAddr = _XAie_LGetTileAddr(row, col) + XAIE_AIE_TILE_EVENT_BROADCAST_BLOCK_NORTH_SET;
			_XAie_LPartWrite32(DevInst, RegAddr, BC_DIR_BLOCK); // Block North

			RegAddr = _XAie_LGetTileAddr(row, col) + XAIE_AIE_TILE_EVENT_BROADCAST_BLOCK_EAST_SET;
			_XAie_LPartWrite32(DevInst, RegAddr, BC_DIR_BLOCK); // Block East
		}
	}

	// Enable desired group errors for each tile and configure BC0 to propagate group error0 event.
	for (int col = 0; col < DevInst->NumCols; col++) {
		// Row-0 : Shim Tile
		RegAddr = _XAie_LGetTileAddr(0, col) + ((DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) ?
			XAIE_PL_MOD_GROUP_ERROR0_ENABLE_B : XAIE_PL_MOD_GROUP_ERROR0_ENABLE);
		_XAie_LPartWrite32(DevInst, RegAddr, XAIE_SHIM_TILE_GROUP_ERROR_VALUE);

		RegAddr = _XAie_LGetTileAddr(0, col) + ((DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) ?
			XAIE_PL_MOD_EVENT_BROADCAST_B_0 : XAIE_PL_MOD_EVENT_BROADCAST_A_0);
		_XAie_LPartWrite32(DevInst, RegAddr, XAIE_SHIM_TILE_GROUP_ERRORS_EVENT_NUM);

		// Row-1 : Mem Tile
		RegAddr = _XAie_LGetTileAddr(1, col) + ((DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) ?
			XAIE_MEM_TILE_GROUP_ERROR0_ENABLE_B : XAIE_MEM_TILE_GROUP_ERROR0_ENABLE);
		_XAie_LPartWrite32(DevInst, RegAddr, XAIE_MEM_TILE_GROUP_ERROR_VALUE);

		RegAddr = _XAie_LGetTileAddr(1, col) + ((DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) ?
			XAIE_MEM_TILE_EVENT_BROADCAST_B_0 : XAIE_MEM_TILE_EVENT_BROADCAST_A_0);
		_XAie_LPartWrite32(DevInst, RegAddr, XAIE_MEM_TILE_GROUP_ERRORS_EVENT_NUM);

		// Row-2 and Above : Core Tile
		for (int row = XAIE_AIE_TILE_ROW_START; row < XAIE_NUM_ROWS; row++) {
			RegAddr = _XAie_LGetTileAddr(row, col) + XAIE_CORE_MOD_GROUP_ERROR0_ENABLE;
			_XAie_LPartWrite32(DevInst, RegAddr, XAIE_CORE_TILE_GROUP_ERROR_VALUE);

			RegAddr = _XAie_LGetTileAddr(row, col) + XAIE_CORE_MOD_BASE_EVENT_BROADCAST;
			_XAie_LPartWrite32(DevInst, RegAddr, XAIE_CORE_TILE_GROUP_ERRORS0_EVENT_NUM);
		}
	}
#endif

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API sets the column clock control register. Its configuration affects
* (enable or disable) all tile's clock above the Shim tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE SHIM tile
* @param        Enable: XAIE_ENABLE to enable column global clock buffer,
*                       XAIE_DISABLE to disable.

* @note         It is internal function to this file
*
******************************************************************************/
static void _XAie_PrivilegeSetColClkBuf(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Enable)
{
	/* This Register address in in Shim Tile. So we should always pass
           Row Value as 0. If we use Loc.Row and user pass row value other
           then shim tile, then it can trigger HW error in AIE array */

        if (_XAie_LIsDeviceGenAIE4()) {
                _XAie_LColumnClkControl(DevInst, Loc, Enable);

        }else {
                u64 RegAddr;
                u32 FldVal;
                RegAddr = _XAie_LGetTileAddr(XAIE_SHIM_ROW, Loc.Col) +
                        XAIE_PL_MOD_COL_CLKCNTR_REGOFF;
                FldVal = XAie_SetField(Enable,
                                XAIE_PL_MOD_COL_CLKCNTR_CLKBUF_ENABLE_LSB,
                                XAIE_PL_MOD_COL_CLKCNTR_CLKBUF_ENABLE_MASK);
                _XAie_LPartMaskWrite32(DevInst, RegAddr,
                                XAIE_PL_MOD_COL_CLKCNTR_CLKBUF_ENABLE_MASK, FldVal);
        }
}

/*****************************************************************************/
/**
*
* This API set the tile columns clock buffer for every column in the partition
*
* @param	DevInst: Device Instance
* @param	Enable: XAIE_ENABLE to enable clock buffers, XAIE_DISABLE to
*			  disable clock buffers.
*
* @note		This function is internal to this file.
*
******************************************************************************/
static void _XAie_PrivilegeSetPartColClkBuf(XAie_DevInst *DevInst,
		u8 Enable)
{
	for(u8 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);

		_XAie_PrivilegeSetColClkBuf(DevInst, Loc, Enable);
	}
}

/*****************************************************************************/
/**
*
*  This API modifies column clock and shim clk control registers for the requested columns.
*  Caller may request subset of columns from a partition.
*
* @param        DevInst: Device Instance
* @param        StartCol: Starting column
* @param        NumCols: Number of columns
* @param        Enable: Enable/Disable
*
* @return       XAIE_OK
******************************************************************************/
AieRC XAie_SetColumnClk(XAie_DevInst *DevInst, u8 Enable)
{	
	/* The NPI open/close aperture is defeatured in AIE4*/
	if (!(_XAie_LIsDeviceGenAIE4())) {
		_XAie_LNpiSetPartProtectedReg(DevInst, XAIE_ENABLE);
	}
	for(u8 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);

		//This modifies column clk which affects all the aie and mem tile in the column.
		_XAie_PrivilegeSetColClkBuf(DevInst, Loc, Enable);
		/* This modifies the clk in the shim tile
		AIE4 device use XAie_TileClockControl API for modular clock control */
		if (!(_XAie_LIsDeviceGenAIE4())) {
			_XAie_PrivilegeSetShimClk(DevInst, Loc, Enable);
		}
	}
	/* The NPI open/close aperture is defeatured in AIE4*/
	if (!(_XAie_LIsDeviceGenAIE4())) {
		_XAie_LNpiSetPartProtectedReg(DevInst, XAIE_DISABLE);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
*  This API reads the register and returns the value of the register to IPU FW
*
* @param        RegAddr: Absolute address of the address to be read
*
* @return       Register Value
******************************************************************************/
u64 XAie_GenRead(u64 RegAddr) {
	return _XAie_LRawRead32(RegAddr);
}

/*****************************************************************************/
/**
*
*  This API writes value provided by IPU FW to the register
*
* @param        RegAddr: Absolute address of the address to be written
*
* @return       None
******************************************************************************/
void XAie_GenWrite(u64 RegAddr, u32 Value) {
	_XAie_LRawWrite32(RegAddr, Value);
}

/*****************************************************************************/
/**
*
*  This API generated NPI interrupts on the interrupt line provided by IPU FW
*
* @param        DevInst: Device Instance
* @param        IntLine: NPI Interrupt line to be used
*
* @return       0 to mark completion of API sequence
******************************************************************************/
AieRC XAie_GenNPIInterrupt(XAie_DevInst *DevInst, u8 IntLine) {

    AieRC RC;
    u64 RegAddr;
    u32 RegVal;

 #if DEV_GEN_AIE4
	// (0) Unlock
	 _XAie_LNpiWrite32(0x0C, 0xF9E8D7C6);

	// (1) LX7 enable AIE4 NPI interrupt (to program NPI register 0x3c)
	_XAie_LNpiWrite32(0x3C, (1U << IntLine)); // 0001b INTERRUPT1, 0010b INTERRUPT2, 0100b INTERRUPT3, 1000b INTERRUPT4

	RegAddr = _XAie_LGetTileAddr(0, 0) + \
					XAIE_NOC_MOD_INTR_L2_GLOBAL_ENABLE;
	if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {
		RegVal = XAie_SetField(1, XAIE_NOC_MOD_INTR_L2_APP_B_GE_LSB,
					XAIE_NOC_MOD_INTR_L2_APP_B_GE_MASK);
	} else {
		RegVal = XAie_SetField(1, XAIE_NOC_MOD_INTR_L2_APP_A_GE_LSB,
					XAIE_NOC_MOD_INTR_L2_APP_A_GE_MASK);
	}

	// (2) LX7 enable AIE4 Interrupr_controller_global_enable register (0xe8070)
	_XAie_LPartWrite32(DevInst, RegAddr, RegVal); // 0x1: Enable_App_A, 0x2: Enable_App_B

	RegAddr = _XAie_LGetTileAddr(0, 0) + \
					XAIE_NOC_MOD_INTR_L2_IRQ;
	if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {
		RegVal = XAie_SetField(IntLine,
					XAIE_NOC_MOD_INTR_L2_APP_B_IRQ_LSB, XAIE_NOC_MOD_INTR_L2_APP_B_IRQ_MASK);
    } else {
		RegVal = XAie_SetField(IntLine,
				XAIE_NOC_MOD_INTR_L2_IRQ_LSB, XAIE_NOC_MOD_INTR_L2_IRQ_MASK);
    }

	// (3) LX7 program AIE4 Interrupt_controller_interrupt_line register (0xe8074)
	if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {
		_XAie_LPartWrite32(DevInst, RegAddr, (IntLine << 7U));
	} else {
		_XAie_LPartWrite32(DevInst, RegAddr, IntLine);
	}
	 // 00b: npi 0, 01b: npi 0 & 1, 10b: npi 0 & 2, 11b: npi 0 & 3
	if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {
		RegAddr = XAIE_NOC_MOD_INTR_L2_APP_B_ENABLE;
	} else {
		RegAddr = XAIE_NOC_MOD_INTR_L2_ENABLE;
	}
	RegVal = 0x00010000;

	// (4) LX7 to program AIE4 Interrupt_controller_enable_A register (0x1c014) or _B (0x5c014)
	_XAie_LPartWrite32(DevInst, RegAddr, RegVal);

	// Lock NPI
	_XAie_LNpiWrite32(0x0C, 0x0);
#endif
    return 0;
}

/*****************************************************************************/
/**
*
*  This API clears NPI interrupts on the interrupt line provided by IPU FW
*
* @param        DevInst: Device Instance
* @param        IntLine: NPI Interrupt line to be used
*
* @return       0 to mark completion of API sequence
******************************************************************************/

AieRC XAie_ClearNPIInterrupt(XAie_DevInst *DevInst, u8 IntLine) {
	AieRC RC;
    u64 RegAddr;
    u32 RegVal;

 #if DEV_GEN_AIE4
	// (0) Unlock
	 _XAie_LNpiWrite32(0x0C, 0xF9E8D7C6);

	// (1) LX7 to program AIE4 Interrupt_controller_Status register App A (0x1C01C) or B (0x5C01C)
	if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {
		RegAddr = XAIE_NOC_MOD_INTR_L2_APP_B_STATUS;
	} else {
		RegAddr = XAIE_NOC_MOD_INTR_L2_STATUS;
	}
	RegVal = 0x00010000;
	_XAie_LPartWrite32(DevInst, RegAddr, RegVal);

	// (2) LX7 clear AIE4 NPI interrupt (to program NPI register 0x30)
	_XAie_LNpiWrite32(0x30, (1U << IntLine)); // 0001b INTERRUPT1, 0010b INTERRUPT2, 0100b INTERRUPT3, 1000b INTERRUPT4

	// Lock NPI
	_XAie_LNpiWrite32(0x0C, 0x0);
#endif
	return 0;
}

/*****************************************************************************/
/**
*
* This API set the tile column reset
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE SHIM tile
* @param	RstEnable: XAIE_ENABLE to assert reset, XAIE_DISABLE to
*			   deassert reset.
*
* @note		This function is internal to this file.
*
******************************************************************************/
static void _XAie_PrivilegeSetColReset(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 RstEnable)
{
	u32 FldVal;
	u64 RegAddr;

	RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
		XAIE_PL_MOD_COL_RST_REGOFF;
	FldVal = XAie_SetField(RstEnable, XAIE_PL_MOD_COL_RST_LSB,
			XAIE_PL_MOD_COL_RST_MASK);

	_XAie_LPartWrite32(DevInst, RegAddr, FldVal);
}

/*****************************************************************************/
/**
*
* This API set the tile columns reset for every column in the partition
*
* @param	DevInst: Device Instance
* @param	RstEnable: XAIE_ENABLE to assert reset, XAIE_DISABLE to
*			   deassert reset.
*
* @note		This function is internal to this file.
*
******************************************************************************/
static void _XAie_PrivilegeSetPartColRst(XAie_DevInst *DevInst, u8 RstEnable)
{
	for(u8 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);

		_XAie_PrivilegeSetColReset(DevInst, Loc, RstEnable);
	}
}

/*****************************************************************************/
/**
*
* This API reset all the resources for an App for the partition
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		This function asserts application reset, and then deassert it.
*		This function is internal to this file.
*
******************************************************************************/
static AieRC _XAie_PrivilegeApplicationReset(XAie_DevInst *DevInst)
{
	AieRC RC;
	u8 SecondAppMode;

	if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_A) {
		SecondAppMode = XAIE_DEVICE_DUAL_APP_MODE_B;
	} else if (DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {
		SecondAppMode = XAIE_DEVICE_DUAL_APP_MODE_A;
	} else {
		SecondAppMode = XAIE_DEVICE_SINGLE_APP_MODE;
	}

	// Pause DMAs for application which needs to be reset in the partition
	for(u8 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);
		_XAie_LSetPartDmaPause(DevInst,Loc,DevInst->AppMode,XAIE_ENABLE);
	}

	// Poll for Pending AXI-MM transactions for application to be reset
	for(u8 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);
		RC = _XAie_LPollAximmTransactions(DevInst,DevInst->AppMode,Loc);
		if(RC !=XAIE_OK) {
			XAIE_ERROR("Application Pending AXI-MM Transaction polling failed \n");
			return RC;
		}
	}

	// Pause DMas for second application if partition is in Dual App mode before calling Application reset assert
	if(SecondAppMode != XAIE_DEVICE_SINGLE_APP_MODE){
		for(u8 C = 0; C < DevInst->NumCols; C++) {
			XAie_LocType Loc = XAie_TileLoc(C, 0);
			_XAie_LSetPartDmaPause(DevInst,Loc,SecondAppMode,XAIE_ENABLE);
		}
	}

	// Poll for Pending AXI-MM transactions for second application before application reset
	if(SecondAppMode != XAIE_DEVICE_SINGLE_APP_MODE){
		for(u8 C = 0; C < DevInst->NumCols; C++) {
			XAie_LocType Loc = XAie_TileLoc(C, 0);
			RC = _XAie_LPollAximmTransactions(DevInst,SecondAppMode,Loc);
			if(RC !=XAIE_OK) {
				XAIE_ERROR("Application Pending AXI-MM Transaction polling failed \n");
				return RC;
			}
		}
	}

	// Assert Application reset
	for(u8 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);
		_XAie_LSetPartColAppReset(DevInst, Loc, XAIE_ENABLE);
	}

	// De-Assert Applicatoin reset
	for(u8 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);
		_XAie_LSetPartColAppReset(DevInst, Loc, XAIE_DISABLE);
	}
	
#ifdef __AIESIM__
	// Wait for Application reset to finish in Simnow
	usleep(100000);
#endif

	// Disable all DMAs pause after De-Asserting Application reset
	for(u8 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);
		_XAie_LSetPartDmaPause(DevInst,Loc,XAIE_DEVICE_SINGLE_APP_MODE,XAIE_DISABLE);
	}
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API reset all SHIMs in the AI engine partition
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		This function asserts reset, and then deassert it.
*		This function is internal to this file.
*
******************************************************************************/
static void _XAie_PrivilegeRstPartShims(XAie_DevInst *DevInst)
{
	for(u8 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);
		_XAie_LSetPartColShimReset(DevInst, Loc, XAIE_ENABLE);
	}

	if (!(_XAie_LIsDeviceGenAIE4())) {
		_XAie_LNpiSetShimReset(XAIE_ENABLE);
		_XAie_LNpiSetShimReset(XAIE_DISABLE);
	}

	//TODO : Note: Jignesh : Do we need to make outof reset using _XAie_LSetPartColShimReset(XAIE_DISABLE); ??
	for(u8 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);
		_XAie_LSetPartColShimReset(DevInst, Loc, XAIE_DISABLE);
	}
}

/*****************************************************************************/
/**
*
* This API sets to block NSU AXI MM slave error and decode error based on user
* inputs. If NSU errors is blocked, it will only generate error events.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE SHIM tile
* @param	BlockSlvEnable: XAIE_ENABLE to block NSU AXI MM Slave errors,
*				or XAIE_DISABLE to unblock.
* @param	BlockDecEnable: XAIE_ENABLE to block NSU AXI MM Decode errors,
*				or XAIE_DISABLE to unblock.
*
* @note		This function is internal to this file.
*
******************************************************************************/
static void _XAie_PrivilegeSetBlockAxiMmNsuErr(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 BlockSlvEnable, u8 BlockDecEnable)
{
	u32 FldVal;
	u64 RegAddr;

	RegAddr = XAIE_NOC_AXIMM_CONF_REGOFF +
		_XAie_LGetTileAddr(Loc.Row, Loc.Col);
	FldVal = XAie_SetField(BlockSlvEnable,
			XAIE_NOC_AXIMM_CONF_SLVERR_BLOCK_LSB,
			XAIE_NOC_AXIMM_CONF_SLVERR_BLOCK_MASK);
	FldVal |= XAie_SetField(BlockDecEnable,
			XAIE_NOC_AXIMM_CONF_DECERR_BLOCK_LSB,
			XAIE_NOC_AXIMM_CONF_DECERR_BLOCK_MASK);

	_XAie_LPartWrite32(DevInst, RegAddr, FldVal);
}

/*****************************************************************************/
/**
*
* This API sets to block the NSU AXI MM slave error and decode error config
* for all the SHIM NOCs in the full partition based on user inputs.
*
* @param	DevInst: Device Instance
* @param	BlockSlvEnable: XAIE_ENABLE to block NSU AXI MM Slave errors,
*				or XAIE_DISABLE to unblock.
* @param	BlockDecEnable: XAIE_ENABLE to block NSU AXI MM Decode errors,
*				or XAIE_DISABLE to unblock.
*
* @note		This function will do the following steps:
*		 * set AXI MM registers NSU errors fields in all SHIM NOC tiles
*		This function is internal to this file.
*
******************************************************************************/
static void _XAie_PrivilegeSetPartBlockAxiMmNsuErr(XAie_DevInst *DevInst,
		u8 BlockSlvEnable, u8 BlockDecEnable)
{
	XAie_LocType Loc = XAie_LPartGetNextNocTile(DevInst,
			XAie_TileLoc(0, 0));

	for(; Loc.Col < DevInst->NumCols;
		Loc = XAie_LPartGetNextNocTile(DevInst, Loc)) {
		_XAie_PrivilegeSetBlockAxiMmNsuErr(DevInst, Loc,
				BlockSlvEnable, BlockDecEnable);
	}
}

#if DEV_GEN_AIE4
/*****************************************************************************/
/**
*
* This API enables generation of HW Error interrupt for desired applications
* HW Error interrupt handler by configuring that desired HW errors to be flaged.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	HwErrCfg: HW Error Config bitmap 
*               		0 - AXI ERROR    
*				1 - HW CE ERRORS
*				2 - HW UC Errors
*
* @note		None.
*
******************************************************************************/
static void _XAie_PrivilegeConfigureHwErrIrq(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_HwErrCfg HwErrCfg)
{
	u64 RegAddr;
	u16 RegVal;

	// Based on the Application Mode, Calculate the register address.
	if (DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {

		RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + \
			  XAIE_PL_MOD_INTR_CNTRL_HW_ERR_ENABLE_B;
	} else {

		RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + \
			  XAIE_PL_MOD_INTR_CNTRL_HW_ERR_ENABLE_A;
	}

	// Calculate the reg value based on requested HW Err Config.
	RegVal = XAie_SetField(HwErrCfg.cfg_val,
			XAIE_PL_MOD_INTR_CNTRL_HW_ERR_CFG_LSB,
			XAIE_PL_MOD_INTR_CNTRL_HW_ERR_CFG_MASK);

	_XAie_LPartWrite32(DevInst, RegAddr, RegVal);
}

/*****************************************************************************/
/**
*
* This API sets NoC interrupt ID to which the interrupt from second level
* interrupt controller shall be driven to.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	NoCIrqId: NoC IRQ index on which the interrupt shall be
*			  driven.
*
* @note		None.
*
******************************************************************************/
static void _XAie_PrivilegeSetHwErrIrqId(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 NoCHwErrIrqId)
{
	u64 RegAddr;
	u16 RegVal;

	// Based on the Application Mode, Calculate the register address.
	if (DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {

		RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + \
			  XAIE_PL_MOD_INTR_CNTRL_HW_ERR_IRQ_B;
	} else {

		RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + \
			  XAIE_PL_MOD_INTR_CNTRL_HW_ERR_IRQ_A;
	}

	// Calculate the register value based on requested HW Err Config.
	RegVal = XAie_SetField(NoCHwErrIrqId, XAIE_PL_MOD_INTR_CNTRL_HW_ERR_CFG_LSB,\
			XAIE_PL_MOD_INTR_CNTRL_HW_ERR_CFG_MASK);

	_XAie_LPartWrite32(DevInst, RegAddr, RegVal);
}

/*****************************************************************************/
/**
*
* This is a warpper API that enables the HW Err interrupt handler and sets NoC 
* interrupt ID to which the error interrupts from HW Err interrupt controller
* shall be driven to. 
*
* This API configures all the HW Err interrupt controllers within a given
* partition in one go.
*
* @param	DevInst: Device Instance
*
* @note		None.
*
******************************************************************************/
static void _XAie_PrivilegeSetHwErrIrq(XAie_DevInst *DevInst,
					XAie_HwErrCfg HwErrCfg)
{
	XAie_LocType Rloc = {0, 0};

	for (;Rloc.Col < DevInst->NumCols; Rloc.Col++) {
		u8 TileType = _XAie_LGetShimTTypefromLoc(DevInst, Rloc);

		if (TileType != XAIEGBL_TILE_TYPE_SHIMNOC)
			continue;

		// Enable / Disable HW Err IRQ Generation
		_XAie_PrivilegeConfigureHwErrIrq(DevInst, Rloc, HwErrCfg);

		// Configure NPI IRQ Number only if request for enable
		if (HwErrCfg.cfg_val != XAIE_MASK) {
			_XAie_PrivilegeSetHwErrIrqId(DevInst, Rloc,
					_XAie_MapColToHWErrIrqId(DevInst, Rloc));
		}
	}
}

/*****************************************************************************/
/**
 *
 * This API enables the HW Err interrupt handler and sets NoC interrupt ID to
 * which the error interrupts from HW Err interrupt controller shall be driven
 * to.
 *
 * This API configures all the HW Err interrupt controllers within a given
 * partition in one go.
 *
 * @param	DevInst: Device Instance
 *
 * @note	None.
 *
 ******************************************************************************/
AieRC XAie_CfgPrivilegeHwErrIrq(XAie_DevInst *DevInst, XAie_HwErrCfg HwErrCfg)
{
	AieRC RC = XAIE_OK;

	if((DevInst == XAIE_NULL) ||
	   (DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}
	_XAie_PrivilegeSetHwErrIrq(DevInst, HwErrCfg);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API enables / disables generation of interrupt from desired application
* L2 interrupt handler.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	NoCIrqId: NoC IRQ index on which the interrupt shall be
*			  driven.
*
* @note		None.
*
******************************************************************************/
static void _XAie_PrivilegeConfigureL2Irq(XAie_DevInst *DevInst,
		XAie_LocType Loc,
		u8 Enable)
{
	u64 RegAddr;
	u16 RegVal;

	RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + \
		XAIE_NOC_MOD_INTR_L2_GLOBAL_ENABLE;

	/**
	 * Note:
	 * As per the agreement with MPNPU FW team, This API will be called :
	 * 		1) Only once per column irrespective of the no of partitions in column during init (Enable mode)
	 * 		2) But will be called once per partition per column during teardown (Disable mode).
	 */

	/**
	 * Mode == Enable 	and 	App Mode == Dual APP A
	 */
	if ((Enable == XAIE_ENABLE) && (DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_A)) {

		RegVal = XAie_SetField(Enable, XAIE_NOC_MOD_INTR_L2_APP_A_GE_LSB,
			XAIE_NOC_MOD_INTR_L2_APP_A_GE_MASK);
		RegVal |= XAie_SetField(Enable, XAIE_NOC_MOD_INTR_L2_APP_B_GE_LSB,
			XAIE_NOC_MOD_INTR_L2_APP_B_GE_MASK);
	}
	/**
	 * Mode == Enable 	and 	App Mode == Dual APP B
	 * Mode == Disable 	and 	App Mode == Dual APP B 
	 */ 
	else if (DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {

		RegVal = XAie_SetField(Enable, XAIE_NOC_MOD_INTR_L2_APP_B_GE_LSB,
			XAIE_NOC_MOD_INTR_L2_APP_B_GE_MASK);
	}
	/**
	 * Mode == Enable 	and 	App Mode == Single APP
	 * Mode == Disable 	and 	App Mode == Single APP
	 * Mode == Disable 	and 	App Mode == Dual App A
	 */ 
	else {

		RegVal = XAie_SetField(Enable, XAIE_NOC_MOD_INTR_L2_APP_A_GE_LSB,
			XAIE_NOC_MOD_INTR_L2_APP_A_GE_MASK);
	}

	_XAie_LPartWrite32(DevInst, RegAddr, RegVal);
}
#endif

/*****************************************************************************/
/**
*
* This API sets NoC interrupt ID to which the interrupt from second level
* interrupt controller shall be driven to.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	NoCIrqId: NoC IRQ index on which the interrupt shall be
*			  driven.
*
* @note		None.
*
******************************************************************************/
static void _XAie_PrivilegeSetL2IrqId(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 NoCIrqId)
{
	u16 RegVal;
	u64 RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) + \
				XAIE_NOC_MOD_INTR_L2_IRQ;

#if DEV_GEN_AIE4

	if (DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {

		RegVal = XAie_SetField(NoCIrqId,
				XAIE_NOC_MOD_INTR_L2_APP_B_IRQ_LSB,
				XAIE_NOC_MOD_INTR_L2_APP_B_IRQ_MASK);
	} else {

		RegVal = XAie_SetField(NoCIrqId,
				XAIE_NOC_MOD_INTR_L2_IRQ_LSB,
				XAIE_NOC_MOD_INTR_L2_IRQ_MASK);

	}
#else
	RegVal = NoCIrqId;
#endif

	_XAie_LPartWrite32(DevInst, RegAddr, RegVal);
}

/*****************************************************************************/
/**
*
* This is a warpper API that enables the L2 interrupt handler and sets NoC 
* interrupt ID to which the error interrupts from second level interrupt 
* controller shall be driven to. 
*
* This API configures all the second level interrupt controllers within a 
* given partition in one go.
*
* @param	DevInst: Device Instance
* @param        Enable: Flag indicating enable or disable
*
* @note		None.
*
******************************************************************************/
static void _XAie_PrivilegeSetL2ErrIrq(XAie_DevInst *DevInst, u8 Enable)
{
	XAie_LocType Rloc = {0, 0};

	for (;Rloc.Col < DevInst->NumCols; Rloc.Col++) {
		u8 TileType = _XAie_LGetShimTTypefromLoc(DevInst, Rloc);

		if (TileType != XAIEGBL_TILE_TYPE_SHIMNOC)
			continue;

		// Enable / Disable IRQ Generation
#if DEV_GEN_AIE4
		_XAie_PrivilegeConfigureL2Irq(DevInst, Rloc, Enable);
#endif

		// Configure NPI IRQ Number only if request was for enable.
		if (Enable != XAIE_DISABLE) {
			_XAie_PrivilegeSetL2IrqId(DevInst, Rloc,
					_XAie_MapColToIrqId(DevInst, Rloc));
		}
	}
}

/*****************************************************************************/
/**
* This API initializes the AI engine partition
*
* @param	DevInst: AI engine partition device instance pointer
* @param	Opts: Initialization options
*
* @return       XAIE_OK
*
* @note		This operation does the following steps to initialize an AI
*		engine partition:
*		- Clock gate all columns
*		- Reset Columns
*		- Ungate all Columns
*		- Remove columns reset
*		- Reset shims
*		- Setup AXI MM not to return errors for AXI decode or slave
*		  errors, raise events instead.
*		- ungate all columns
*		- Setup partition isolation.
*		- zeroize memory if it is requested
*
*******************************************************************************/
AieRC XAie_PartitionInitialize(XAie_DevInst *DevInst, XAie_PartInitOpts *Opts)
{
	AieRC RC = XAIE_OK;
	u32 OptFlags;
	u8 AppMode;

	XAIE_ERROR_RETURN((DevInst == NULL || DevInst->NumCols > XAIE_NUM_COLS),
		XAIE_INVALID_ARGS,
		XAIE_ERROR_MSG("Partition initialization failed, invalid partition instance\n"));

	if(Opts != NULL) {
		OptFlags = Opts->InitOpts;
		AppMode = (OptFlags & XAIE_PART_INIT_OPT_APP_MODE) >> XAIE_APP_MODE_SHIFT;

	} else {
		OptFlags = XAIE_PART_INIT_OPT_DEFAULT;
		AppMode = XAIE_DEVICE_SINGLE_APP_MODE;
	}

	/* Set Single or Dual App mode. In Dual App Mode set App A or App B */
        if (_XAie_LIsDeviceGenSupportDualApp()) {
                XAIE_ERROR_RETURN((AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) ||
                                  (AppMode == XAIE_DEVICE_INVALID_MODE),
                                XAIE_INVALID_ARGS,
                                XAIE_ERROR_MSG("This API should be called only for App A mode or single App Mode\n"));
                if(DevInst->NumCols == 1 && AppMode == XAIE_DEVICE_DUAL_APP_MODE_A) {
                        DevInst->AppMode = AppMode;
                } else if (DevInst->NumCols >= 1 && AppMode == XAIE_DEVICE_SINGLE_APP_MODE) {
			DevInst->AppMode = AppMode;
		} else {
                        XAIE_ERROR_RETURN((AppMode != XAIE_DEVICE_SINGLE_APP_MODE),
                                        XAIE_INVALID_ARGS,
                                        XAIE_ERROR_MSG("Partition has more than one column, so dual app is not supported\n"));
                }
        } else {
                DevInst->AppMode = XAIE_DEVICE_SINGLE_APP_MODE;
        }

	/* The NPI open/close aperture is defeatured in AIE4*/
	if (!(_XAie_LIsDeviceGenAIE4())) {
		_XAie_LNpiSetPartProtectedReg(DevInst, XAIE_ENABLE);
	}

	// Enable Column Clocks
	_XAie_PrivilegeSetPartColClkBuf(DevInst, XAIE_ENABLE);

	/* Set Dual App Mode Registers */
	/*TODO: Need to configure all dual app registers into single app mode for
	  single app partition. */
	if (_XAie_LIsDeviceGenSupportDualApp()) {
		if(DevInst->AppMode != XAIE_DEVICE_SINGLE_APP_MODE){
			if(Opts->Locs != NULL) {
				RC =_XAie_LSetDualAppModePrivileged(DevInst, Opts);
				if(RC != XAIE_OK) {
					XAIE_ERROR("Dual APP setting failed\n");
					return RC;
				}
			} else {
				XAIE_ERROR("Tile location in XAie_PartInitOpts cannot be NULL in Dual App Mode\n");
				return XAIE_INVALID_ARGS;
			}
		}
	}

	/* Column Reset is replaced by Application reset in AIE4 devices */
	if (((OptFlags & XAIE_PART_INIT_OPT_COLUMN_RST) != 0) &&
			(!(_XAie_LIsDeviceGenAIE4()))) {
		/* Always gate all tiles before resetting columns */
		_XAie_PrivilegeSetPartColClkBuf(DevInst, XAIE_DISABLE);
		_XAie_PrivilegeSetPartColRst(DevInst, XAIE_ENABLE);
		_XAie_PrivilegeSetPartColClkBuf(DevInst, XAIE_ENABLE);
		_XAie_PrivilegeSetPartColRst(DevInst, XAIE_DISABLE);
	}

	/* As per AIE4 MAS Application reset is not required in AIE4 device during Partition Init */
	if((OptFlags & XAIE_PART_INIT_OPT_SHIM_RST) != 0) {
		if (!(_XAie_LIsDeviceGenAIE4())) {
			_XAie_PrivilegeRstPartShims(DevInst);
		}
	}

	if((OptFlags & XAIE_PART_INIT_OPT_BLOCK_NOCAXIMMERR) != 0) {
		_XAie_PrivilegeSetPartBlockAxiMmNsuErr(DevInst,
			XAIE_ENABLE, XAIE_ENABLE);
	}

	if ((OptFlags & XAIE_PART_INIT_OPT_L2_SPLIT) != 0U) {
		if(DevInst->AppMode != XAIE_DEVICE_SINGLE_APP_MODE) {
			DevInst->L2Split = (OptFlags & XAIE_PART_INIT_OPT_L2_SPLIT) >> XAIE_L2_SPLIT_SHIFT;
			_XAie_LSetPartL2Split(DevInst);
		}
	}

	if ((OptFlags & XAIE_PART_INIT_OPT_ISOLATE) != 0) {
		_XAie_LSetPartIsolationAfterRst(DevInst);
	}

	/* As per AIE4 MAS Zeroization is not required in AIE4 device during Partition Init */
	if ((OptFlags & XAIE_PART_INIT_OPT_ZEROIZEMEM) != 0) {
		if (!(_XAie_LIsDeviceGenAIE4())) {
			_XAie_LPartMemZeroInit(DevInst);
		}
	}

	if ((OptFlags & XAIE_PART_INIT_OPT_UC_MEM_PRIV)) {
		if (_XAie_LIsDeviceGenAIE4()) {
			_XAie_PrivilegeSetUCMemoryPrivileged(DevInst, XAIE_ENABLE);
                }
        }


	/* Gate all the tiles and ungate the requested tiles*/
	if(Opts != NULL)
	{
		/* Disable all the column clock and enable only the requested column clock */
		_XAie_PrivilegeSetPartColClkBuf(DevInst, XAIE_DISABLE);

		/* Ungate the tiles that is requested */
		for(u32 i = 0; i < Opts->NumUseTiles; i++)
		{
			if(Opts->Locs[i].Col >= DevInst->NumCols || Opts->Locs[i].Row >= DevInst->NumRows) {
				XAIE_ERROR("Invalid Tile Location\n");
				return XAIE_INVALID_TILE;
			}

			if(Opts->Locs[i].Row == 0) {
				continue;
			}

			/*
			* Check if column clock buffer is already enabled and continue
			*/
			_XAie_PrivilegeSetColClkBuf(DevInst, Opts->Locs[i], XAIE_ENABLE);
		}
	} else {
		XAIE_DBG("XAie_PartInitOpts is NULL. Entire array will be initialized\n");
	}

	/**
	 * Enable L2 interrupt generation
	 */
	_XAie_PrivilegeSetL2ErrIrq(DevInst, XAIE_ENABLE);

	_XAie_DisableTlast(DevInst);

	/* The NPI open/close aperture is defeatured in AIE4*/
	if (!(_XAie_LIsDeviceGenAIE4())) {
		_XAie_LNpiSetPartProtectedReg(DevInst, XAIE_DISABLE);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API tears down the AI engine partition
*
* @param	DevInst: AI engine partition device instance pointer
* @param	Opts: Initialization options
*
* @return       XAIE_OK on success, error code on failure
*
* @note		This operation does the following steps to initialize an AI
*		engine partition:
*		- Clock gate all columns
*		- Reset Columns
*		- Ungate all columns
*		- Reset shims
*		- Ungate all columns
*		- Zeroize memories
*		- Clock gate all columns
*
*******************************************************************************/
AieRC XAie_PartitionTeardown(XAie_DevInst *DevInst)
{
	AieRC RC;

	XAIE_ERROR_RETURN((DevInst == NULL || DevInst->NumCols > XAIE_NUM_COLS),
		XAIE_INVALID_ARGS,
		XAIE_ERROR_MSG("Partition teardown failed, invalid partition instance\n"));


	/* Clearing core registers before disabling clock - This is for only phoenix and strix */
	if (!(_XAie_LIsDeviceGenAIE4())) {
		XAie_ClearCoreReg(DevInst);
	}

	/* The NPI open/close aperture is defeatured in AIE4*/
	if (!(_XAie_LIsDeviceGenAIE4())) {
		_XAie_LNpiSetPartProtectedReg(DevInst, XAIE_ENABLE);
	}

	/* Column Reset is replaced by Application reset in AIE4 devices */
	if (!(_XAie_LIsDeviceGenAIE4())) {

		_XAie_PrivilegeSetPartColClkBuf(DevInst, XAIE_DISABLE);

		_XAie_PrivilegeSetPartColRst(DevInst, XAIE_ENABLE);

		_XAie_PrivilegeSetPartColClkBuf(DevInst, XAIE_ENABLE);

		_XAie_PrivilegeSetPartColRst(DevInst, XAIE_DISABLE);
	}

	// AIE4 MAS doesn't mentioned to enable clock before Applicatio reset in Partition Teardown
	if (!(_XAie_LIsDeviceGenAIE4())) {
		_XAie_PrivilegeSetPartColClkBuf(DevInst, XAIE_ENABLE);
	}

	if (!(_XAie_LIsDeviceGenAIE4())) {
		_XAie_PrivilegeRstPartShims(DevInst);
	} else {
		RC = _XAie_PrivilegeApplicationReset(DevInst);
		if(RC != XAIE_OK){
			return RC;
		}
	}

	_XAie_LPartMemZeroInit(DevInst);

	_XAie_PrivilegeSetPartColClkBuf(DevInst, XAIE_DISABLE);

	/* The NPI open/close aperture is defeatured in AIE4*/
	if (!(_XAie_LIsDeviceGenAIE4())) {
		_XAie_LNpiSetPartProtectedReg(DevInst, XAIE_DISABLE);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API checks if all the shim dmas in the paritition are idle.
*
* @param	DevInst: AI engine partition device instance pointer
*
* @return       XAIE_OK on success, error code on failure
*
* @note		Internal only.
*
*******************************************************************************/
static inline AieRC _XAie_LPartIsShimDmaIdle(XAie_DevInst *DevInst)
{
	for(u8 C = 0U; C < DevInst->NumCols; C++) {
		AieRC RC;
		u8 TType;

		TType = _XAie_LGetTTypefromLoc(DevInst, XAie_TileLoc(C, 0));
		if(TType == XAIEGBL_TILE_TYPE_SHIMNOC) {
			RC = _XAie_LIsShimDmaIdle(DevInst, XAie_TileLoc(C, 0));
			if(RC != XAIE_OK)
				return RC;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API check if all the resources in the partition are idle. Currently,
* we check only for dma status.
*
* @param	DevInst: AI engine partition device instance pointer
*
* @return       XAIE_OK on success, error code on failure
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_IsPartitionIdle(XAie_DevInst *DevInst)
{
	AieRC RC = XAIE_OK;

	XAIE_ERROR_RETURN((DevInst == NULL || DevInst->NumCols > XAIE_NUM_COLS),
		XAIE_INVALID_ARGS,
		"Partition idle check failed, invalid partition instance\n");

	RC = _XAie_LPartIsDmaIdle(DevInst);
	XAIE_ERROR_RETURN(RC == XAIE_OK, RC, "DMAs are not idle\n");

	RC = _XAie_LPartIsShimDmaIdle(DevInst);
	XAIE_ERROR_RETURN(RC == XAIE_OK, RC, "Shim DMA is not idle\n");

	return RC;
}

/*****************************************************************************/
/**
* This API resets all modules, clears registers needed for context switching.
*
* @param	DevInst: AI engine partition device instance pointer
*
* @return       XAIE_OK on success, error code on failure
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_ClearPartitionContext(XAie_DevInst *DevInst)
{
	AieRC RC;

	/* Clearing core registers before disabling clock - This is for only phoenix and strix */
	if (!(_XAie_LIsDeviceGenAIE4())) {
		XAie_ClearCoreReg(DevInst);
	}
	
	/* The NPI open/close aperture is defeatured in AIE4*/
	if (!(_XAie_LIsDeviceGenAIE4())) {
		_XAie_LNpiSetPartProtectedReg(DevInst, XAIE_ENABLE);
	}

	/* Column Reset is replaced by Application reset in AIE4 devices */
	if (!(_XAie_LIsDeviceGenAIE4())) {
		_XAie_PrivilegeSetPartColClkBuf(DevInst, XAIE_DISABLE);
		_XAie_PrivilegeSetPartColRst(DevInst, XAIE_ENABLE);
		_XAie_PrivilegeSetPartColClkBuf(DevInst, XAIE_ENABLE);
		_XAie_PrivilegeSetPartColRst(DevInst, XAIE_DISABLE);
	}

	/* Block AxiMM NSU Error,Column clock reset, Clear Isolation should not be called
	 * for devices which support Dual App and are operating in Dual App mode for
	 * ClearPartitionContext.
	 * Note: To control individual Tile Clock use API - XAie_TileClockControl*/
	if (!(_XAie_LIsDeviceGenAIE4())) {
		_XAie_PrivilegeSetPartBlockAxiMmNsuErr(DevInst, XAIE_ENABLE,
					XAIE_ENABLE);
		_XAie_PrivilegeSetPartColClkBuf(DevInst, XAIE_ENABLE);
		_XAie_LSetPartIsolationAfterRst(DevInst);
	}

	/* Application reset is supported in AIE4 devices only.
	* For Legacy devices Column reset is enough during application
	* context switch */
	if (_XAie_LIsDeviceGenAIE4()) {
		RC =_XAie_PrivilegeApplicationReset(DevInst);
		if(RC != XAIE_OK){
			return RC;
		}
	}

	/* Zeroize Application memory resources */
	RC = _XAie_LPartDataMemZeroInit(DevInst);
	if (RC != XAIE_OK)
		return RC;

	/**
	 * Disable Hardware L2 interrupt generation
	 * Kotesh(TODO): Check why in previous generation code
	 *               This is enable and not disbale.
	 */
	_XAie_PrivilegeSetL2ErrIrq(DevInst, XAIE_DISABLE);

	/* The NPI open/close aperture is defeatured in AIE4*/
	if (!(_XAie_LIsDeviceGenAIE4())) {
		_XAie_LNpiSetPartProtectedReg(DevInst, XAIE_DISABLE);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API open/close the NPI aperture with protected register.
*
* @param	DevInst: AI engine partition device instance pointer
* @param	enable: To enable or disable the NPI protected register
*
* @return       XAIE_OK on success, error code on failure
*
* @note		For AIE4, Driver will not open/close NPI aperture when programming
			privilege registers, the eHypervisior will use AxPROT[1] for
			programming privilege registers.
			if a tall, for AIE4, if required to use NPI Aperture open/close,
			they this API can be called while programming privilege registers.
			for legacy devices, Driver will open/close NPI aperture programming
			privilege registers. This API is not allowed to call externally.
*
*******************************************************************************/
AieRC XAie_NpiSetPartProtectedReg(XAie_DevInst *DevInst, u8 enable)
{	
	if (_XAie_LIsDeviceGenAIE4()) {
		return XAIE_NOT_SUPPORTED;
	}
	_XAie_LNpiSetPartProtectedReg(DevInst, enable);
	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API configures the AIE array following POR sequence
*
* @param	DevInst: AI engine partition device instance pointer
* @param	PorOptions: To configure
*
* @return   XAIE_OK on success, error code on failure
*
* @note		Initial support for this API is only for AIE4 devices. It doesn't support
* 			Legacy devices.
*
*******************************************************************************/

AieRC XAie_PowerOnReset(XAie_DevInst *DevInst, XAie_PartPorOpts *PorOptions) {
	AieRC RC;
	if (!_XAie_LIsDeviceGenAIE4()) {
			return XAIE_NOT_SUPPORTED;
	}
	RC = _XAie_LAiePorConfiguration(DevInst, PorOptions);
	if (RC != XAIE_OK) {
		XAIE_ERROR("AIE array POR configuration failed\n");
		return RC;
	} else {
		return XAIE_OK;
	}
}

/*****************************************************************************/
/**
* This API Enables/Disables clock in Individual AIE Tiles
*
* @param	DevInst: AI engine partition device instance pointer
* @param	Locs: Locations of Tiles to Enable/Disable Module Clock
* @param	NumTiles: Number of tiles for clock Gate/Ungate
* @param	Enable: Enable/Disable Tile Module clock
*
* @return   XAIE_OK on success, error code on failure
*
* @note		Initial support for this API is only for AIE4 devices. It doesn't support
* 			Legacy devices.
*
*******************************************************************************/
AieRC XAie_TileClockControl(XAie_DevInst *DevInst, XAie_LocType *Loc,u8 NumTiles, u8 Enable) {
	AieRC RC;
	u32 FldVal;
	u64 RegAddr;
	if (!_XAie_LIsDeviceGenAIE4()) {
			return XAIE_NOT_SUPPORTED;
	}

	RC = _XAie_LTileClockControl(DevInst, Loc,NumTiles,Enable);
	if (RC != XAIE_OK) {
		XAIE_ERROR("AIE tile clock control failed\n");
		return RC;
	} else {
		return XAIE_OK;
	}
}

/*****************************************************************************/
/**
* This API Sets SMID, AxUSER_A or B and Trusted_Keys registers in Shim Tile for each
* application
*
* @param	DevInst: AI engine partition device instance pointer
* @param	XAie_ShimOpts: Values for SMID, AxUSER_A or B, Trusted_Keys
*
* @return   XAIE_OK on success, error code on failure
*
* @note		Initial support for this API is only for AIE4 devices. It doesn't support
* 			Legacy devices.
*
*******************************************************************************/
AieRC XAie_ConfigureShimDmaRegisters(XAie_DevInst *DevInst, XAie_ShimOpts *ShimOptions) {
	AieRC RC;

	if (!_XAie_LIsDeviceGenAIE4()) {
			return XAIE_NOT_SUPPORTED;
	}

	RC = _XAie_LConfigureShimDmaRegisters(DevInst, ShimOptions);
	if (RC != XAIE_OK) {
		XAIE_ERROR("AIE array Shim DMA configuration failed\n");
		return RC;
	} else {
		return XAIE_OK;
	}
}

#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE && XAIE_FEATURE_LITE */
/** @} */
