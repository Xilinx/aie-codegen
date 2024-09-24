/******************************************************************************
* Copyright (C) 2021 - 2022 Xilinx, Inc.  All rights reserved.
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
#include "xaie_lite_internal.h"

/***************************** Macro Definitions *****************************/
#define XAIE_ISOLATE_EAST_MASK	(1U << 3)
#define XAIE_ISOLATE_NORTH_MASK	(1U << 2)
#define XAIE_ISOLATE_WEST_MASK	(1U << 1)
#define XAIE_ISOLATE_SOUTH_MASK	(1U << 0)
#define XAIE_ISOLATE_ALL_MASK	((1U << 4) - 1)

#define XAIE_APP_MODE_SHIFT     8U
#define XAIE_L2_SPLIT_SHIFT     10U

/************************** Function Definitions *****************************/
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
	u64 RegAddr;
	u32 FldVal;

	RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
		XAIE_PL_MOD_COL_CLKCNTR_REGOFF;
	FldVal = XAie_SetField(Enable,
			XAIE_PL_MOD_COL_CLKCNTR_CLKBUF_ENABLE_LSB,
			XAIE_PL_MOD_COL_CLKCNTR_CLKBUF_ENABLE_MASK);

	_XAie_LPartMaskWrite32(DevInst, RegAddr,
		XAIE_PL_MOD_COL_CLKCNTR_CLKBUF_ENABLE_MASK, FldVal);
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
		//This modifies the clk in the shim tile
		_XAie_PrivilegeSetShimClk(DevInst, Loc, Enable);
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
		if (HwErrCfg.cfg_val != XAIE_DISABLE) {
			_XAie_PrivilegeSetHwErrIrqId(DevInst, Rloc,
					_XAie_MapColToHWErrIrqId(DevInst, Rloc));
		}
	}
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

	if (DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {

		RegVal = XAie_SetField(Enable, XAIE_NOC_MOD_INTR_L2_APP_B_GE_LSB,
			XAIE_NOC_MOD_INTR_L2_APP_B_GE_MASK);
	} else {

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
		XAIE_ERROR_RETURN((appMode == XAIE_DEVICE_DUAL_APP_MODE_B) ||
				  (appMode == XAIE_DEVICE_INVALID_MODE),
				XAIE_INVALID_ARGS,
				XAIE_ERROR_MSG("This API should be called only for App A mode or single App Mode\n"));
		if(DevInst->NumCols == 1) {
			DevInst->AppMode = AppMode;
		} else {
			XAIE_ERROR_RETURN((appMode != XAIE_DEVICE_SINGLE_APP_MODE),
					XAIE_INVALID_ARGS,
					XAIE_ERROR_MSG("Partition has more than one column, so dual app is not supported\n"));
			DevInst->AppMode = XAIE_DEVICE_SINGLE_APP_MODE;
		}
	} else {
		DevInst->AppMode = XAIE_DEVICE_SINGLE_APP_MODE;
	}

	/* The NPI open/close aperture is defeatured in AIE4*/
	if (!(_XAie_LIsDeviceGenAIE4())) {
		_XAie_LNpiSetPartProtectedReg(DevInst, XAIE_ENABLE);
	}

	/* Set Dual App Mode Registers */
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

	if((OptFlags & XAIE_PART_INIT_OPT_SHIM_RST) != 0) {
		_XAie_PrivilegeRstPartShims(DevInst);
	}

	if((OptFlags & XAIE_PART_INIT_OPT_BLOCK_NOCAXIMMERR) != 0) {
		_XAie_PrivilegeSetPartBlockAxiMmNsuErr(DevInst,
			XAIE_ENABLE, XAIE_ENABLE);
	}

	_XAie_PrivilegeSetPartColClkBuf(DevInst, XAIE_ENABLE);

	if ((OptFlags & XAIE_PART_INIT_OPT_L2_SPLIT) != 0U) {
		if(DevInst->AppMode != XAIE_DEVICE_SINGLE_APP_MODE) {
			DevInst->L2Split = (OptFlags & XAIE_PART_INIT_OPT_L2_SPLIT) >> XAIE_L2_SPLIT_SHIFT;
			_XAie_LSetPartL2Split(DevInst);
		}
	}

	if ((OptFlags & XAIE_PART_INIT_OPT_ISOLATE) != 0) {
		_XAie_LSetPartIsolationAfterRst(DevInst);
	}

	if ((OptFlags & XAIE_PART_INIT_OPT_ZEROIZEMEM) != 0) {
		_XAie_LPartMemZeroInit(DevInst);
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

			if ((OptFlags & XAIE_PART_INIT_OPT_UC_MEM_PRIV)) {
				if (_XAie_LIsDeviceGenAIE4()) {
					_XAie_PrivilegeSetUCMemoryPrivileged(DevInst, XAIE_ENABLE);
				}
			}
		}
	} else {
		XAIE_DBG("XAie_PartInitOpts is NULL. Entire array will be initialized\n");
	}

	/**
         * Enable Hardware error interrupt generation
         */
#if DEV_GEN_AIE4

	XAie_HwErrCfg HwErrCfg;
	HwErrCfg.cfg_bits.axi_errors = XAIE_ENABLE;
	HwErrCfg.cfg_bits.hw_ce_errors = XAIE_ENABLE;
	HwErrCfg.cfg_bits.hw_uc_errors = XAIE_ENABLE;
	_XAie_PrivilegeSetHwErrIrq(DevInst, HwErrCfg);

#endif

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

	_XAie_PrivilegeRstPartShims(DevInst);

	_XAie_PrivilegeSetPartColClkBuf(DevInst, XAIE_ENABLE);

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
		_XAie_PrivilegeRstPartShims(DevInst);
	}

	/* Zeroize Application memory resources */
	RC = _XAie_LPartDataMemZeroInit(DevInst);
	if (RC != XAIE_OK)
		return RC;

	/**
         * Disable Hardware error interrupt generation
         */
#if DEV_GEN_AIE4

	XAie_HwErrCfg HwErrCfg;
	HwErrCfg.cfg_bits.axi_errors = XAIE_DISABLE;
	HwErrCfg.cfg_bits.hw_ce_errors = XAIE_DISABLE;
	HwErrCfg.cfg_bits.hw_uc_errors = XAIE_DISABLE;
	_XAie_PrivilegeSetHwErrIrq(DevInst, HwErrCfg);

#endif

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
