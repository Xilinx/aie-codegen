/******************************************************************************
* Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_perfcnt.c
* @{
*
* This file contains routines for AIE performance counters
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Dishita 11/22/2019  Initial creation
* 1.1   Tejus   04/13/2020  Remove use of range in apis
* 1.2   Dishita 04/16/2020  Fix compiler warnings
* 1.3   Dishita 05/04/2020  Added Module argument to all apis
* 1.4   Tejus   06/10/2020  Switch to new io backend apis.
* 1.5   Dishita 09/15/2020  Add api to read perf counter control configuration.
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_events.h"
#include "xaie_feature_config.h"
#include "xaie_perfcnt.h"
#include "xaie_helper_internal.h"

#ifdef XAIE_FEATURE_PERFCOUNT_ENABLE

/*****************************************************************************/
/***************************** Macro Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/* This API reads the given performance counter for the given tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Pl or Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
* @param	Counter:Performance Counter. If value is MaxCounterVal,
*                       all counter values will be returned
* @param	CounterVal: Pointer to store Counter Value.
*                           If Counter is MaxCounterVal, CounterVal pointer is
*                           expected to be large to store all counter values
* @return	XAIE_OK on success
*		XAIE_INVALID_ARGS if any argument is invalid
*		XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterGet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter, u32 *CounterVal)
{
	u64 CounterBaseAddr;
	u32 CounterRegOffset;
	u8 TileType;
	AieRC RC;
	u16 MaxCounterVal;
	const XAie_PerfMod *PerfMod;

	if((DevInst == XAIE_NULL) || (CounterVal == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance or CounterVal\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[0U];
	} else {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[Module];
	}

	/* Check for perf counter support for tiletype and module*/
	if(PerfMod->MaxCounterVal == 0) {
		XAIE_ERROR("Perf counters are not suported for tile type %d and module %d\n", TileType, Module);
		return XAIE_INVALID_ARGS;
	}

	/* Checking for valid Counter */
	MaxCounterVal = _XAie_GetMaxElementValue(DevInst->DevProp.DevGen, TileType, DevInst->AppMode, PerfMod->MaxCounterVal);
	if(Counter > MaxCounterVal) {
		XAIE_ERROR("Invalid Counter number: %d\n", Counter);
		return XAIE_INVALID_ARGS;
	}

	/* Compute register address without offset */
	CounterBaseAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
					PerfMod->PerfCounterBaseAddr;	
	
	/* If Counter is MaxCounterVal, return all counter values */
	if (Counter == MaxCounterVal) {			
		for (u8 C = 0; C < PerfMod->MaxCounterVal; C++) {
			/* Add offset address based on Counter and read */
			CounterRegOffset = C * (u32)PerfMod->PerfCounterOffsetAdd;
			RC |= XAie_Read32(DevInst, CounterBaseAddr + CounterRegOffset, CounterVal + C);
		}
		
		/* when device is in single app mode and tile is mem/shim tile*/
		if (_XAie_IsTileResourceInSharedAddrSpace(DevInst->DevProp.DevGen, DevInst->AppMode, TileType)) {
			CounterBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterBaseAddr);
			CounterVal += PerfMod->MaxCounterVal;
			for (u8 C = 0; C < PerfMod->MaxCounterVal; C++) {
				/* Add offset address based on Counter and read */
				CounterRegOffset = C * (u32)PerfMod->PerfCounterOffsetAdd;
				RC |= XAie_Read32(DevInst, CounterBaseAddr + CounterRegOffset, CounterVal + C);
			}
		}
	} else {
		//For AIE4 device in single app mode, change the register space
		if(Counter >= PerfMod->MaxCounterVal) {
			CounterBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterBaseAddr);
			Counter -= PerfMod->MaxCounterVal;
		}
		CounterRegOffset = Counter * (u32)PerfMod->PerfCounterOffsetAdd;
		
		RC |= XAie_Read32(DevInst, CounterBaseAddr + CounterRegOffset, CounterVal);
	}

	return RC;
}

/*****************************************************************************/
/* This API returns the given performance counter offset for the given tile
 * and counter.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Pl or Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
* @param	Counter: Performance Counter.
* @param	Offset: Pointer to store Offset Value.
* @return	XAIE_OK on success
*		XAIE_INVALID_ARGS if any argument is invalid
*		XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note		None.
*
******************************************************************************/
AieRC XAie_PerfCounterGetOffset(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter, u64 *Offset)
{
	u64 CounterBaseAddr;
	u8 TileType;
	u16 MaxCounterVal;
	AieRC RC;
	const XAie_PerfMod *PerfMod;

	if((DevInst == XAIE_NULL) || (Offset == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance or Offset\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[0U];
	} else {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[Module];
	}	

	/* Check for perf counter support for tiletype and module*/
	if(PerfMod->MaxCounterVal == 0) {
		XAIE_ERROR("Perf counters are not suported for tile type %d and module %d\n", TileType, Module);
		return XAIE_INVALID_ARGS;
	}

	/* Checking for valid Counter */
	MaxCounterVal = _XAie_GetMaxElementValue(DevInst->DevProp.DevGen, TileType, DevInst->AppMode, PerfMod->MaxCounterVal);
	if(Counter >= MaxCounterVal) {
		XAIE_ERROR("Invalid Counter number: %d\n", Counter);
		return XAIE_INVALID_ARGS;
	}

	/* Compute register address without offset */
	CounterBaseAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
						PerfMod->PerfCounterBaseAddr;	
	
	//For AIE4 device in single app mode, change the register space
	if(Counter >= PerfMod->MaxCounterVal){
		CounterBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterBaseAddr);
		Counter -= PerfMod->MaxCounterVal;
	}
	
	*Offset = CounterBaseAddr + (Counter * (u32)PerfMod->PerfCounterOffsetAdd);
		
	return RC;
}

/*****************************************************************************/
/* This API configures the control registers corresponding to the counters
*  with the start and stop event for the given tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Pl or Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
* @param	Counter:Performance Counter
* @param	StartEvent:Event that triggers start to the counter
* @Param	StopEvent: Event that triggers stop to the counter
* @return	XAIE_OK on success
*		XAIE_INVALID_ARGS if any argument is invalid
*		XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterControlSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter,
		XAie_Events StartEvent, XAie_Events StopEvent)
{
	u32 CtrlRegOffset, FldVal, FldMask;
	u64 CtrlBaseAddr;
	u8 TileType;
	u16 IntStartEvent, IntStopEvent;
	AieRC RC;
	u16 MaxCounterVal;
	const XAie_PerfMod *PerfMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[0U];
	} else {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[Module];
	}

	/* Check for perf counter support for tiletype and module*/
	if(PerfMod->MaxCounterVal == 0) {
		XAIE_ERROR("Perf counters are not suported for tile type %d and module %d\n", TileType, Module);
		return XAIE_INVALID_ARGS;
	}
	
	/* Getting the true event number from the logical event mapping */
	RC |= XAie_EventLogicalToPhysicalConv(DevInst, Loc, Module, StartEvent, &IntStartEvent);
	RC |= XAie_EventLogicalToPhysicalConv(DevInst, Loc, Module, StopEvent, &IntStopEvent);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Counld not get physical event number\n");
		return XAIE_ERR;
	}

	/* Checking for valid Counter */
	MaxCounterVal = _XAie_GetMaxElementValue(DevInst->DevProp.DevGen, TileType, DevInst->AppMode, PerfMod->MaxCounterVal);
	if(Counter >= MaxCounterVal) {
		XAIE_ERROR("Invalid Counter number: %d\n", Counter);
		return XAIE_INVALID_ARGS;
	}

	/* Compute register address without offset */
	CtrlBaseAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
						PerfMod->PerfCtrlBaseAddr;		

	/* Get offset address based on Counter */
	
	//For AIE4 device in single app mode, change the register space
	if(Counter >= PerfMod->MaxCounterVal){
		CtrlBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CtrlBaseAddr);
		Counter -= PerfMod->MaxCounterVal;
	}

	CtrlRegOffset = (Counter / 2U * PerfMod->PerfCtrlOffsetAdd);

	/* Compute mask for performance control register */
	if (_XAie_CheckPrecisionExceeds((PerfMod->StartStopShift * (Counter % 2U)),
			_XAie_MaxBitsNeeded(PerfMod->Start.Mask | PerfMod->Stop.Mask),
			MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	FldMask = (PerfMod->Start.Mask | PerfMod->Stop.Mask) <<
				(PerfMod->StartStopShift * (Counter % 2U));

	/* Compute value to be written to the performance control register */
	if ((_XAie_CheckPrecisionExceeds(
			PerfMod->Start.Lsb + (PerfMod->StartStopShift * (Counter % 2U)),
			_XAie_MaxBitsNeeded(IntStartEvent), MAX_VALID_AIE_REG_BIT_INDEX))  ||
		(_XAie_CheckPrecisionExceeds((PerfMod->StartStopShift * (Counter % 2U)),
			_XAie_MaxBitsNeeded(PerfMod->Start.Mask), MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(
			PerfMod->Stop.Lsb + (PerfMod->StartStopShift * (Counter % 2U)),
			_XAie_MaxBitsNeeded(IntStopEvent), MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(PerfMod->StartStopShift * (Counter % 2U),
			_XAie_MaxBitsNeeded(PerfMod->Stop.Mask), MAX_VALID_AIE_REG_BIT_INDEX))){
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	FldVal = XAie_SetField(IntStartEvent,
		PerfMod->Start.Lsb + (PerfMod->StartStopShift * (Counter % 2U)),
		PerfMod->Start.Mask << (PerfMod->StartStopShift * (Counter % 2U)))|
		XAie_SetField(IntStopEvent,
		PerfMod->Stop.Lsb + (PerfMod->StartStopShift * (Counter % 2U)),
		PerfMod->Stop.Mask << (PerfMod->StartStopShift * (Counter % 2U)));
	
	return XAie_MaskWrite32(DevInst, CtrlBaseAddr + CtrlRegOffset, FldMask, FldVal);
}

/*****************************************************************************/
/* This API configures the control registers corresponding to the counter
*  with the reset event for the given tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Pl or Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
* @param	Counter:Performance Counter
* @param	ResetEvent:Event that triggers reset to the counter
* @return	XAIE_OK on success
*		XAIE_INVALID_ARGS if any argument is invalid
*		XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterResetControlSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter,
		XAie_Events ResetEvent)
{
	u32 ResetRegOffset, ResetFldVal, ResetFldMask;
	u64 ResetBaseAddr;
	u8 TileType;
	u16 IntResetEvent;
	AieRC RC;
	u16 MaxCounterVal;	
	const XAie_PerfMod *PerfMod;
	
	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[0U];
	} else {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[Module];
	}

	/* Check for perf counter support for tiletype and module*/
	if(PerfMod->MaxCounterVal == 0) {
		XAIE_ERROR("Perf counters are not suported for tile type %d and module %d\n", TileType, Module);
		return XAIE_INVALID_ARGS;
	}

	RC = XAie_EventLogicalToPhysicalConv(DevInst, Loc, Module, ResetEvent, &IntResetEvent);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Counld not get physical event number\n");
		return XAIE_ERR;
	}

	/* Checking for valid Counter */
	MaxCounterVal = _XAie_GetMaxElementValue(DevInst->DevProp.DevGen, TileType, DevInst->AppMode, PerfMod->MaxCounterVal);
	if(Counter >= MaxCounterVal) {
		XAIE_ERROR("Invalid Counter number: %d\n", Counter);
		return XAIE_INVALID_ARGS;
	}

	/* Compute register address without offset */
	ResetBaseAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
						PerfMod->PerfCtrlResetBaseAddr;		

	/* Get offset address based on Counter */
	//For AIE4 device in single app mode, change the register space
	if(Counter >= PerfMod->MaxCounterVal){
		ResetBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, ResetBaseAddr);
		Counter -= PerfMod->MaxCounterVal;
	}
	
	ResetRegOffset = (Counter / 4U * PerfMod->PerfResetOffsetAdd);

	/* Compute mask for performance control register */
	if (_XAie_CheckPrecisionExceeds(PerfMod->ResetShift * (Counter % 4U),
			_XAie_MaxBitsNeeded(PerfMod->Reset.Mask),MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	ResetFldMask = PerfMod->Reset.Mask <<
					(PerfMod->ResetShift * (Counter % 4U));
	/* Compute value to be written to the performance control register */
	if ((_XAie_CheckPrecisionExceeds(
			PerfMod->Reset.Lsb + (PerfMod->ResetShift * (Counter % 4U)),
			_XAie_MaxBitsNeeded(IntResetEvent), MAX_VALID_AIE_REG_BIT_INDEX))  ||
		(_XAie_CheckPrecisionExceeds((PerfMod->ResetShift * (Counter % 4U)),
			_XAie_MaxBitsNeeded(PerfMod->Reset.Mask), MAX_VALID_AIE_REG_BIT_INDEX))){
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	ResetFldVal = XAie_SetField(IntResetEvent,
		PerfMod->Reset.Lsb + (PerfMod->ResetShift * (Counter % 4U)),
		PerfMod->Reset.Mask << (PerfMod->ResetShift * (Counter % 4U)));
	

	return XAie_MaskWrite32(DevInst, ResetBaseAddr + ResetRegOffset, ResetFldMask,
			ResetFldVal);
}

/*****************************************************************************/
/* This API sets the performance counter value for the given tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of Tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Pl or Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
*
* @param	Counter:Performance Counter
* @param	CounterVal:Performance Counter Value
* @return	XAIE_OK on success
*		XAIE_INVALID_ARGS if any argument is invalid
*		XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter,
		u32 CounterVal)
{
	u32 CounterRegOffset;
	u64 CounterBaseAddr;
	u8 TileType;
	AieRC RC;
	u16 MaxCounterVal;
	const XAie_PerfMod *PerfMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[0U];
	} else {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[Module];
	}

	/* Check for perf counter support for tiletype and module*/
	if(PerfMod->MaxCounterVal == 0) {
		XAIE_ERROR("Perf counters are not suported for tile type %d and module %d\n", TileType, Module);
		return XAIE_INVALID_ARGS;
	}

	/* Checking for valid Counter */
	MaxCounterVal = _XAie_GetMaxElementValue(DevInst->DevProp.DevGen, TileType, DevInst->AppMode, PerfMod->MaxCounterVal);
	if(Counter >= MaxCounterVal) {
		XAIE_ERROR("Invalid Counter number: %d\n", Counter);
		return XAIE_INVALID_ARGS;
	}

	/* Compute register address without offset */
	CounterBaseAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
						PerfMod->PerfCounterBaseAddr;	

	/* Get offset address based on Counter */
	//For AIE4 device in single app mode, change the register space
	if(Counter >= PerfMod->MaxCounterVal){
		CounterBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterBaseAddr);
		Counter -= PerfMod->MaxCounterVal;
	}
	
	CounterRegOffset = Counter * (u32)PerfMod->PerfCounterOffsetAdd;

	return XAie_Write32(DevInst, CounterBaseAddr + CounterRegOffset, CounterVal);
}
/*****************************************************************************/
/* This API sets the performance counter event value for the given tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Pl or Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
* @param	Counter:Performance Counter
* @param	EventVal:Event value to set
* @return	XAIE_OK on success
*		XAIE_INVALID_ARGS if any argument is invalid
*		XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterEventValueSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter, u32 EventVal)
{
	u32 CounterEvtValRegOffset;
	u64 CounterEvtValBaseAddr;
	u8 TileType;
	AieRC RC;
	u16 MaxCounterVal;
	const XAie_PerfMod *PerfMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[0U];
	} else {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[Module];
	}
	
	/* Check for perf counter support for tiletype and module*/
	if(PerfMod->MaxCounterVal == 0) {
		XAIE_ERROR("Perf counters are not suported for tile type %d and module %d\n", TileType, Module);
		return XAIE_INVALID_ARGS;
	}

	/* Checking for valid Counter */
	MaxCounterVal = _XAie_GetMaxElementValue(DevInst->DevProp.DevGen, TileType, DevInst->AppMode, PerfMod->MaxCounterVal);
	if(Counter >= MaxCounterVal) {
		XAIE_ERROR("Invalid Counter number: %d\n", Counter);
		return XAIE_INVALID_ARGS;
	}

	/* Compute register address without offset */
	CounterEvtValBaseAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
						PerfMod->PerfCounterEvtValBaseAddr;	

	/* Get offset address based on Counter */
	//For AIE4 device in single app mode, change the register space
	if(Counter >= PerfMod->MaxCounterVal){
		CounterEvtValBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterEvtValBaseAddr);
		Counter -= PerfMod->MaxCounterVal;
	}
	
	CounterEvtValRegOffset = Counter * (u32)PerfMod->PerfCounterOffsetAdd;

	return XAie_Write32(DevInst, CounterEvtValBaseAddr + CounterEvtValRegOffset, EventVal);
}

/*****************************************************************************/
/* This API resets the performance counter event value for the given tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE tile
* @param        Module: Module of tile.
*                       For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*                       For Pl or Shim tile - XAIE_PL_MOD,
*                       For Mem tile - XAIE_MEM_MOD.
* @param        Counter:Performance Counter
* @return       XAIE_OK on success
*               XAIE_INVALID_ARGS if any argument is invalid
*               XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterEventValueReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter)
{
	return XAie_PerfCounterEventValueSet(DevInst, Loc, Module, Counter, 0U);
}

/*****************************************************************************/
/* This API resets the performance counter value for the given tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of Tile
* @param        Module: Module of tile.
*                       For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*                       For Pl or Shim tile - XAIE_PL_MOD,
*                       For Mem tile - XAIE_MEM_MOD.
*
* @param        Counter:Performance Counter
* @return       XAIE_OK on success
*               XAIE_INVALID_ARGS if any argument is invalid
*               XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter)
{
	return XAie_PerfCounterSet(DevInst, Loc, Module, Counter, 0U);
}

/*****************************************************************************/
/* This API resets configuration for the control registers corresponding to the
*  counter with the NONE as reset event for the given tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE tile
* @param        Module: Module of tile.
*                       For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*                       For Pl or Shim tile - XAIE_PL_MOD,
*                       For Mem tile - XAIE_MEM_MOD.
* @param        Counter:Performance Counter
* @return       XAIE_OK on success
*               XAIE_INVALID_ARGS if any argument is invalid
*               XAIE_INVALID_TILE if tile type from Loc is invalid
*
*
******************************************************************************/
AieRC XAie_PerfCounterResetControlReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		                XAie_ModuleType Module, u8 Counter)
{
	AieRC RC;
	u8 TileType;
	u32 ResetEvent;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	} else {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	}

	if(EvntMod->EventMin > XAIE_EVENT_USER_EVENT_7_MEM_TILE){
		XAIE_ERROR("Invalid Event type\n");
		return XAIE_ERR;
	}

	/* Since first event of all modules is NONE event, using it to reset */
	ResetEvent = EvntMod->EventMin;
	/*
	 * Currently calling the external api, later it can be factorized to
	 * remove redundant checks.
	 */
	return XAie_PerfCounterResetControlSet(DevInst, Loc, Module, Counter,
			(XAie_Events)ResetEvent);
}

/*****************************************************************************/
/* This API resets configuration of the control registers corresponding to the
*  counters with the start and stop event as NONE for the given tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of the tile
* @param        Module: Module of tile.
*                       For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*                       For Pl or Shim tile - XAIE_PL_MOD,
*                       For Mem tile - XAIE_MEM_MOD.
* @param        Counter:Performance Counter
* @return       XAIE_OK on success
*               XAIE_INVALID_ARGS if any argument is invalid
*               XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterControlReset(XAie_DevInst *DevInst, XAie_LocType Loc,
	XAie_ModuleType Module, u8 Counter)
{
	AieRC RC;
	u8 TileType;
	XAie_Events StartStopEvent;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	} else {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	}


	if(EvntMod->EventMin > XAIE_EVENT_USER_EVENT_7_MEM_TILE){
		XAIE_ERROR("Invalid Event type\n");
		return XAIE_ERR;
	}

	/* Since first event of all modules is NONE event, using it to reset */
	StartStopEvent = (XAie_Events)EvntMod->EventMin;

	/*
	 * Currently calling the external api, later it can be factorized to
	 * remove redundant checks.
	 */
	return XAie_PerfCounterControlSet(DevInst, Loc, Module, Counter,
		StartStopEvent, StartStopEvent);
}

/*****************************************************************************/
/* This API reads the performance counter configuration for the given counter
 * and tile location.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of the tile
* @param        Module: Module of tile i.e.
*                       XAIE_MEM_MOD, XAIE_CORE_MOD, XAIE_PL_MOD
* @param        Counter: Performance Counter
* @param        StartEvent: Pointer to store start event
* @param        StopEvent: Pointer to store stop event
* @param        ResetEvent: Pointer to store reset event
*
* @return       XAIE_OK on success
*               XAIE_INVALID_ARGS if any argument is invalid
*               XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterGetControlConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter, XAie_Events *StartEvent,
		XAie_Events *StopEvent, XAie_Events *ResetEvent)
{
	u32 StartStopRegOffset, ResetRegOffset, StartStopEvent, RegEvent;
	u64 StartStopRegAddr, ResetRegAddr;
	u8 TileType;
	AieRC RC;
	u16 MaxCounterVal;
	const XAie_PerfMod *PerfMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(StartEvent == XAIE_NULL || StopEvent == XAIE_NULL ||
			ResetEvent == XAIE_NULL) {
		XAIE_ERROR("Invalid pointers to store Events\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[0U];
	} else {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[Module];
	}

	/* Check for perf counter support for tiletype and module*/
	if(PerfMod->MaxCounterVal == 0) {
		XAIE_ERROR("Perf counters are not suported for tile type %d and module %d\n", TileType, Module);
		return XAIE_INVALID_ARGS;
	}

	/* Checking for valid Counter */
	MaxCounterVal = _XAie_GetMaxElementValue(DevInst->DevProp.DevGen, TileType, DevInst->AppMode, PerfMod->MaxCounterVal);
	if(Counter >= MaxCounterVal) {
		XAIE_ERROR("Invalid Counter number: %d\n", Counter);
		return XAIE_INVALID_ARGS;
	}	

	/* Compute absolute address and read the start stop event register */
	/* Compute register address without offset */
	StartStopRegAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
						PerfMod->PerfCtrlBaseAddr;		

	/* Get offset address based on Counter */
	//For AIE4 device in single app mode, change the register space
	if(Counter >= PerfMod->MaxCounterVal){
		StartStopRegAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, StartStopRegAddr);
		Counter -= PerfMod->MaxCounterVal;
	}
	
	StartStopRegOffset = (Counter / 2U * PerfMod->PerfCtrlOffsetAdd);

	RC = XAie_Read32(DevInst, StartStopRegAddr + StartStopRegOffset , &StartStopEvent);
	if(RC != XAIE_OK) {
		return RC;
	}

	/* Get both start and stop event for given counter */
	StartStopEvent >>= PerfMod->StartStopShift * (Counter % 2U);

	/* Get start and stop event individually and store in event pointer */
	RegEvent = StartStopEvent & PerfMod->Start.Mask;
	if(RegEvent > UINT16_MAX){
		XAIE_ERROR("Invalid RegEvent\n");
		return XAIE_ERR;
	}
	RC = XAie_EventPhysicalToLogicalConv(DevInst, Loc, Module, (u16)RegEvent,
			StartEvent);
	if (RC != XAIE_OK) {
		return RC;
	}

	if (_XAie_CheckPrecisionExceeds(PerfMod->StartStopShift / 2U,
			_XAie_MaxBitsNeeded(StartStopEvent & PerfMod->Stop.Mask), MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RegEvent = (StartStopEvent & PerfMod->Stop.Mask) >>
			PerfMod->StartStopShift / 2U;
	if(RegEvent > UINT16_MAX){
		XAIE_ERROR("Invalid RegEvent\n");
		return XAIE_ERR;
	}
	RC = XAie_EventPhysicalToLogicalConv(DevInst, Loc, Module, (u16)RegEvent,
			StopEvent);
	if (RC != XAIE_OK) {
		return RC;
	}

	/* Compute absolute address and read the reset event register */
	/* Compute register address without offset */
	ResetRegAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
						PerfMod->PerfCtrlResetBaseAddr;		

	/* Get offset address based on Counter */
	//For AIE4 device in single app mode, change the register space
	if(Counter >= PerfMod->MaxCounterVal){
		ResetRegAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, ResetRegAddr);
		Counter -= PerfMod->MaxCounterVal;
	}
	
	ResetRegOffset = (Counter / 4U *  PerfMod->PerfResetOffsetAdd);

	RC = XAie_Read32(DevInst, ResetRegAddr + ResetRegOffset, &RegEvent);
	if(RC != XAIE_OK) {
		return RC;
	}

	/* Get reset event for given counter and store in the event pointer */
	RegEvent >>= PerfMod->ResetShift * (Counter % 4U);
	RegEvent &= PerfMod->Reset.Mask;
	if(RegEvent > UINT16_MAX){
		XAIE_ERROR("Invalid RegEvent\n");
		return XAIE_ERR;
	}
	RC = XAie_EventPhysicalToLogicalConv(DevInst, Loc, Module, (u16)RegEvent,
			ResetEvent);
	if (RC != XAIE_OK) {
		return RC;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the perf counter event based on the tile location
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD.
* @param	Event: Base event of tile
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None
******************************************************************************/
AieRC XAie_PerfCounterGetEventBase(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events *Event)
{
	AieRC RC;
	u8 TileType;
	const XAie_EvntMod *EventMod;

	if((DevInst == XAIE_NULL) || (Event == NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	RC = XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if (TileType == XAIEGBL_TILE_TYPE_AIETILE) {
		EventMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	} else {
		EventMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	}

	if(EventMod->PerfCntEventBase > XAIE_EVENT_USER_EVENT_7_MEM_TILE){
		XAIE_ERROR("Invalid Event type\n");
		return XAIE_ERR;
	}
	*Event = (XAie_Events)EventMod->PerfCntEventBase;

	return RC;
}

/*****************************************************************************/
/* This API reads the given performance counter snapshot for the given tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Pl or Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
* @param	Counter:Performance Counter. If value is MaxCounterVal,
*                       all counter values will be returned
* @param	CounterVal: Pointer to store Counter Value.
*                           If Counter is MaxCounterVal, CounterVal pointer is
*                           expected to be large to store all counter values
* @return	XAIE_OK on success
*		XAIE_INVALID_ARGS if any argument is invalid
*		XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterSnapshotGet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter, u32 *CounterVal)
{
	u64 CounterSsBaseAddr;
	u32 CounterSsRegOffset;
	u8 TileType;
	AieRC RC;
	u16 MaxCounterVal;
	const XAie_PerfMod *PerfMod;

	if((DevInst == XAIE_NULL) || (CounterVal == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance or CounterVal\n");
		return XAIE_INVALID_ARGS;
	}

	if(!_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
		XAIE_ERROR("Performance snapshot registers are supported from AIE4 devices only\n");
		return XAIE_INVALID_DEVICE;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[0U];
	} else {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[Module];
	}

	if(PerfMod->MaxCounterVal == 0) {
		XAIE_ERROR("Perf counters are not suported for tile type %d and module %d\n", TileType, Module);
		return XAIE_INVALID_ARGS;
	}

	/* Checking for valid Counter */
	MaxCounterVal = _XAie_GetMaxElementValue(DevInst->DevProp.DevGen, TileType, DevInst->AppMode, PerfMod->MaxCounterVal);
	if(Counter > MaxCounterVal) {
		XAIE_ERROR("Invalid Counter number: %d\n", Counter);
		return XAIE_INVALID_ARGS;
	}

	/* Compute register address without offset */
	CounterSsBaseAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
					PerfMod->PerfCounterSsBaseAddr;

	/* If Counter is MaxCounterVal, return all counter values */
	if (Counter == MaxCounterVal) {
		/* Read all counters from app A space and app B space */
		/* Read all counters from app A space */
		for (u8 C = 0; C < PerfMod->MaxCounterVal; C++) {
			/* Add offset address based on Counter and read */
			CounterSsRegOffset = C * (u32)PerfMod->PerfCounterOffsetAdd;
			RC |= XAie_Read32(DevInst, CounterSsBaseAddr + CounterSsRegOffset, CounterVal + C);
		}

		/* when device is in single app mode and tile is mem/shim tile*/
		if (_XAie_IsTileResourceInSharedAddrSpace(DevInst->DevProp.DevGen, DevInst->AppMode, TileType)) {
			CounterSsBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterSsBaseAddr);
			CounterVal += PerfMod->MaxCounterVal;
			for (u8 C = 0; C < PerfMod->MaxCounterVal; C++) {
				/* Add offset address based on Counter and read */
				CounterSsRegOffset = C * (u32)PerfMod->PerfCounterOffsetAdd;
				RC |= XAie_Read32(DevInst, CounterSsBaseAddr + CounterSsRegOffset, CounterVal + C);
			}
		}
	} else {
		//For AIE4 device in single app mode, change the register space
		if(Counter >= PerfMod->MaxCounterVal){
			CounterSsBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterSsBaseAddr);
			Counter -= PerfMod->MaxCounterVal;
		}

		CounterSsRegOffset = Counter * (u32)PerfMod->PerfCounterOffsetAdd;

		RC |= XAie_Read32(DevInst, CounterSsBaseAddr + CounterSsRegOffset, CounterVal);
	}

	return RC;
}

/*****************************************************************************/
/* This API sets the performance counter snapshot value for the given tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of Tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Pl or Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
*
* @param	Counter:Performance Counter
* @param	CounterVal:Performance Counter Value
* @return	XAIE_OK on success
*		XAIE_INVALID_ARGS if any argument is invalid
*		XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterSnapshotSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter,
		u32 CounterVal)
{
	u32 CounterSsRegOffset;
	u64 CounterSsBaseAddr;
	u8 TileType;
	AieRC RC;
	u16 MaxCounterVal;
	const XAie_PerfMod *PerfMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(!_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
		XAIE_ERROR("Performance snapshot registers are supported from AIE4 devices only\n");
		return XAIE_INVALID_DEVICE;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[0U];
	} else {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[Module];
	}

	/* Check for perf counter support for tiletype and module*/
	if(PerfMod->MaxCounterVal == 0) {
		XAIE_ERROR("Perf counters are not suported for tile type %d and module %d\n", TileType, Module);
		return XAIE_INVALID_ARGS;
	}
	
	/* Checking for valid Counter */
	MaxCounterVal = _XAie_GetMaxElementValue(DevInst->DevProp.DevGen, TileType, DevInst->AppMode, PerfMod->MaxCounterVal);
	if(Counter >= MaxCounterVal) {
		XAIE_ERROR("Invalid Counter number: %d\n", Counter);
		return XAIE_INVALID_ARGS;
	}

	/* Compute register address without offset */
	CounterSsBaseAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
						PerfMod->PerfCounterSsBaseAddr;	

	/* Get offset address based on Counter */
	//For AIE4 device in single app mode, change the register space
	if(Counter >= PerfMod->MaxCounterVal){
		CounterSsBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterSsBaseAddr);
		Counter -= PerfMod->MaxCounterVal;
	}
	
	CounterSsRegOffset = Counter * (u32)PerfMod->PerfCounterOffsetAdd;
	
	return XAie_Write32(DevInst, CounterSsBaseAddr + CounterSsRegOffset, CounterVal);
}

/*****************************************************************************/
/* This API resets the performance counter Snapshot value for the given tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of Tile
* @param        Module: Module of tile.
*                       For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*                       For Pl or Shim tile - XAIE_PL_MOD,
*                       For Mem tile - XAIE_MEM_MOD.
*
* @param        Counter:Performance Counter
* @return       XAIE_OK on success
*               XAIE_INVALID_ARGS if any argument is invalid
*               XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterSnapshotReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter)
{
	return XAie_PerfCounterSnapshotSet(DevInst, Loc, Module, Counter, 0U);
}

/*****************************************************************************/
/* This API sets the performance counter snapshot load event for the given tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Pl or Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
* @param	EventVal:Event value to set
* @return	XAIE_OK on success
*		XAIE_INVALID_ARGS if any argument is invalid
*		XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterSnapshotLoadEventSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events SSLoadEventVal)
{	
	u64 CounterSsLoadEvtBaseAddr;
	u8 TileType;
	u16 IntSSLoadEventVal;
	AieRC RC;
	const XAie_PerfMod *PerfMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(!_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
		XAIE_ERROR("Performance snapshot registers are supported from AIE4 devices only\n");
		return XAIE_INVALID_DEVICE;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[0U];
	} else {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[Module];
	}

	/* Check for perf counter support for tiletype and module*/
	if(PerfMod->MaxCounterVal == 0) {
		XAIE_ERROR("Perf counters are not suported for tile type %d and module %d\n", TileType, Module);
		return XAIE_INVALID_ARGS;
	}
	
	/* Getting the true event number from the logical event mapping */
	RC |= XAie_EventLogicalToPhysicalConv(DevInst, Loc, Module, SSLoadEventVal, &IntSSLoadEventVal);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Counld not get physical event number\n");
		return XAIE_ERR;
	}
	
	/* Compute register address without offset */
	CounterSsLoadEvtBaseAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
						PerfMod->PerfCounterSsLoadEvttBaseAddr;	

	RC |= XAie_Write32(DevInst, CounterSsLoadEvtBaseAddr, IntSSLoadEventVal);

	return RC;
	
}

/*****************************************************************************/
/* This API resets the performance counter snapshot load event for the given tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE tile
* @param        Module: Module of tile.
*                       For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*                       For Pl or Shim tile - XAIE_PL_MOD,
*                       For Mem tile - XAIE_MEM_MOD.
* @return       XAIE_OK on success
*               XAIE_INVALID_ARGS if any argument is invalid
*               XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterSnapshotLoadEventReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module)
{
	AieRC RC;
	u8 TileType;
	u32 ResetEvent;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	} else {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	}

	/* CERT-C Check*/
	if(EvntMod->EventMin > XAIE_EVENT_USER_EVENT_7_MEM_TILE){
		XAIE_ERROR("Invalid Event type\n");
		return XAIE_ERR;
	}

	/* Since first event of all modules is NONE event, using it to reset */
	ResetEvent = EvntMod->EventMin;

	return XAie_PerfCounterSnapshotLoadEventSet(DevInst, Loc, Module, (XAie_Events)ResetEvent);
}


/*****************************************************************************/
/**
*
* This API returns the whole set of performance counter values in the shim
* uC MDM module. Counters are read sequentially, first event counters then
* latency counters which store more than one value.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of SHIM Tile
* @param	CounterVal: Pointer to store counter values, expected to be
*			    large enough for all counter values.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Counter widths of greater than 32 bits not supported
******************************************************************************/
AieRC XAie_MdmPerfCounterGet(XAie_DevInst *DevInst, XAie_LocType Loc,
	u32 *CounterVal)
{
	AieRC RC;
	u64 Offset;
	u8 TileType, Index;
	const XAie_UcMdm *UcMdm;

	if((DevInst == XAIE_NULL) || (CounterVal == NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(XAie_IsUcModulePresent(DevInst, TileType) == 0U) {
		XAIE_ERROR("Tile does not have uC module\n");
		return XAIE_INVALID_TILE;
	}

	UcMdm = DevInst->DevProp.DevMod[TileType].UcMod->UcMdm;
	if(UcMdm->CounterWidth != 32U) {
		XAIE_ERROR("Only counter widths of 32 bits supported\n");
		return XAIE_ERR;
	}

	/* Reset counter access to first counter */
	Offset = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		UcMdm->PerfCtrl->RegOff;

	/* covertiy CERT-C check*/
	if (_XAie_CheckPrecisionExceeds(UcMdm->PerfCtrl->Reset.Lsb ,
			_XAie_MaxBitsNeeded(1U), MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}	
	RC = XAie_Write32(DevInst, Offset, (1U << UcMdm->PerfCtrl->Reset.Lsb) &
			UcMdm->PerfCtrl->Reset.Mask) ;
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to reset register access\n");
		return RC;
	}

	/* Read event counters */
	Index = 0;
	Offset = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		UcMdm->PerfCntReadRegOff;
	for(u8 i = 0; i < UcMdm->NumEventCounters; i++) {
		RC = XAie_Read32(DevInst, Offset, CounterVal + Index);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to read event register %d\n", i);
			return RC;
		}
		Index++;
	}

	/* Read latency counters */
	for(u8 i = 0; i < UcMdm->NumLatencyCounters; i++) {
		for (u8 j = 0; j < 4U; j ++) {
			RC = XAie_Read32(DevInst, Offset, CounterVal + Index);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to read %d register latency counter %d\n",
						j, i);
				return RC;
			}
			Index++;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the configuration of the MDM performance counters including
* how many latency and event counters and counter width
*
* @param	DevInst: Device Instance
* @param	Loc: Location of SHIM Tile
* @param	NumEvntCounter: Pointer to store number of event counters
* @param	NumLatCounter: Pointer to store number of latency counters
* @param	CounterWidth: Pointer to store counter width
*
* @return	XAIE_OK on success, error code on failure.
******************************************************************************/
AieRC XAie_MdmPerfCounterGetConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
	u8 *NumEvntCounter, u8 *NumLatCounter, u8 *CounterWidth)
{
	u8 TileType;
	const XAie_UcMdm *UcMdm;

	if((DevInst == XAIE_NULL) || (NumEvntCounter == NULL) ||
			(NumLatCounter == NULL) || (CounterWidth == NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(XAie_IsUcModulePresent(DevInst, TileType) == 0U) {
		XAIE_ERROR("Tile does not have uC module\n");
		return XAIE_INVALID_TILE;
	}

	UcMdm = DevInst->DevProp.DevMod[TileType].UcMod->UcMdm;
	*NumEvntCounter = UcMdm->NumEventCounters;
	*NumLatCounter = UcMdm->NumLatencyCounters;
	*CounterWidth = UcMdm->CounterWidth;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the whole set of performance counter statuses in the shim
* uC MDM module. Counters are read sequentially, first event counters then
* latency counters.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of SHIM Tile
* @param	CounterStatus: Pointer to store counter status, expected to be
*			       large enough for all counter statuses.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Counter widths of greater than 32 bits not supported
******************************************************************************/
AieRC XAie_MdmPerfCounterGetStatus(XAie_DevInst *DevInst, XAie_LocType Loc,
	u8 *CounterStatus)
{
	AieRC RC;
	u64 Offset;
	u8 TileType, TotalCounters;
	const XAie_UcMdm *UcMdm;

	if((DevInst == XAIE_NULL) || (CounterStatus == NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(XAie_IsUcModulePresent(DevInst, TileType) == 0U) {
		XAIE_ERROR("Tile does not have uC module\n");
		return XAIE_INVALID_TILE;
	}

	/* Reset counter access to first counter */
	UcMdm = DevInst->DevProp.DevMod[TileType].UcMod->UcMdm;
	Offset = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		UcMdm->PerfCtrl->RegOff;

	/* covertiy CERT-C check*/
	if (_XAie_CheckPrecisionExceeds(UcMdm->PerfCtrl->Reset.Lsb ,
			_XAie_MaxBitsNeeded(1U), MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RC = XAie_Write32(DevInst, Offset, (1U << UcMdm->PerfCtrl->Reset.Lsb) &
			UcMdm->PerfCtrl->Reset.Mask) ;
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to reset register access\n");
		return RC;
	}

	/* Read counter status */
	TotalCounters = (UcMdm->NumEventCounters + UcMdm->NumLatencyCounters) & 0xFFU;
	Offset = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		UcMdm->PerfSts->RegOff;
	for(u8 i = 0; i < TotalCounters; i++) {
		u32 RegVal;

		RC = XAie_Read32(DevInst, Offset, &RegVal);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to read status of register %d\n", i);
			return RC;
		}

		CounterStatus[i] = (u8)(RegVal & 0xFFU);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API sets the whole set of performance counter event control registers in
* uC MDM module. Counters are written to sequentially, first event counters then
* latency counters.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of SHIM Tile
* @param	UcEvents: Pointer to array of events to program counters
* @param	Reset: If reset is true, all control registers will be reset
*		       - XAIE_DISABLE to disable reset
*		       - XAIE_ENABLE to enable reset
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Internal only
******************************************************************************/
AieRC _XAie_MdmPerfCounterControlConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
	u8 *UcEvents, u8 Reset)
{
	AieRC RC;
	u64 Offset;
	u8 TileType, TotalCounters;
	const XAie_UcMdm *UcMdm;

	/* Expect caller to check valid tiletype */
	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	UcMdm = DevInst->DevProp.DevMod[TileType].UcMod->UcMdm;

	/* Reset counter access to first counter */
	Offset = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		UcMdm->PerfCtrl->RegOff;

	/* covertiy CERT-C check*/
	if (_XAie_CheckPrecisionExceeds(UcMdm->PerfCtrl->Reset.Lsb,
			_XAie_MaxBitsNeeded(1U), MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	RC = XAie_Write32(DevInst, Offset, (1U << UcMdm->PerfCtrl->Reset.Lsb) &
			UcMdm->PerfCtrl->Reset.Mask) ;
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to reset register access\n");
		return RC;
	}

	/* Write counter event control registers */
	Offset = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		UcMdm->PerfEvents->RegOff;
	TotalCounters = (UcMdm->NumEventCounters + UcMdm->NumLatencyCounters) & 0xFFU;
	for(u8 i = 0; i < TotalCounters; i++) {
		if (Reset == XAIE_ENABLE) {
			RC = XAie_Write32(DevInst, Offset, 0U);
		} else {
			RC = XAie_Write32(DevInst, Offset, UcEvents[i]);
		}
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to write event control for register %d\n",
					i);
			return RC;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API sets the whole set of performance counter event control registers in
* uC MDM module. Counters are written to sequentially, first event counters then
* latency counters.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of SHIM Tile
* @param	UcEvents: Pointer to array of events to program counters
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Counter widths of greater than 32 bits not supported
******************************************************************************/
AieRC XAie_MdmPerfCounterControlSet(XAie_DevInst *DevInst, XAie_LocType Loc,
	u8 *UcEvents)
{
	u8 TileType, TotalCounters;
	const XAie_UcMdm *UcMdm;

	if((DevInst == XAIE_NULL) || (UcEvents == NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(XAie_IsUcModulePresent(DevInst, TileType) == 0U) {
		XAIE_ERROR("Tile does not have uC module\n");
		return XAIE_INVALID_TILE;
	}

	UcMdm = DevInst->DevProp.DevMod[TileType].UcMod->UcMdm;
	TotalCounters = (UcMdm->NumEventCounters + UcMdm->NumLatencyCounters) & 0xFFU;
	for(u8 i = 0; i < TotalCounters; i++) {
		if (UcEvents[i] > UcMdm->PerfEvents->MaxEventId) {
			XAIE_ERROR("Invalid uC Event ID for counter %d\n", i);
			return XAIE_INVALID_ARGS;
		}
	}

	return _XAie_MdmPerfCounterControlConfig(DevInst, Loc, UcEvents,
			XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API resets the whole set of performance counter event control registers in
* uC MDM module. Counters are written to sequentially, first event counters then
* latency counters.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of SHIM Tile
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Counter widths of greater than 32 bits not supported
******************************************************************************/
AieRC XAie_MdmPerfCounterControlReset(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(XAie_IsUcModulePresent(DevInst, TileType) == 0U) {
		XAIE_ERROR("Tile does not have uC module\n");
		return XAIE_INVALID_TILE;
	}

	return _XAie_MdmPerfCounterControlConfig(DevInst, Loc, NULL,
			XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API sets the whole set of performance counter registers in uC MDM module.
* Counters are written to sequentially, first event counters then latency
* counters.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of SHIM Tile
* @param	CounterVal: Pointer to array of counter valuues
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Counter widths of greater than 32 bits not supported
******************************************************************************/
AieRC XAie_MdmPerfCounterSet(XAie_DevInst *DevInst, XAie_LocType Loc,
	u32 *CounterVal)
{
	AieRC RC;
	u64 Offset;
	u8 TileType, Index;
	const XAie_UcMdm *UcMdm;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(XAie_IsUcModulePresent(DevInst, TileType) == 0U) {
		XAIE_ERROR("Tile does not have uC module\n");
		return XAIE_INVALID_TILE;
	}

	UcMdm = DevInst->DevProp.DevMod[TileType].UcMod->UcMdm;
	if(UcMdm->CounterWidth != 32U) {
		XAIE_ERROR("Only counter widths of 32 bits supported\n");
		return XAIE_ERR;
	}

	/* Reset counter access to first counter */
	Offset = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		UcMdm->PerfCtrl->RegOff;

	/* covertiy CERT-C check*/
	if (_XAie_CheckPrecisionExceeds(UcMdm->PerfCtrl->Reset.Lsb,
			_XAie_MaxBitsNeeded(1U),MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RC = XAie_Write32(DevInst, Offset, (1U << UcMdm->PerfCtrl->Reset.Lsb) &
			UcMdm->PerfCtrl->Reset.Mask) ;
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to reset register access\n");
		return RC;
	}

	/* Write event counter registers */
	Index = 0;
	Offset = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		UcMdm->PerfEvents->RegOff;
	for(u8 i = 0; i < UcMdm->NumEventCounters; i++) {
		RC = XAie_Write32(DevInst, Offset, CounterVal[Index]);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to write to event counter register %d\n",
					i);
			return RC;
		}
		Index++;
	}

	/* Write latency counter registers */
	for(u8 i = 0U; i < UcMdm->NumLatencyCounters; i++) {
		for(u8 j = 0U; j < 4U; j++) {
			RC = XAie_Write32(DevInst, Offset, CounterVal[Index]);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to write to latency counter register %d\n",
						i);
				return RC;
			}
			Index++;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API resets the whole set of performance counter registers in uC MDM module.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of SHIM Tile
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Counter widths of greater than 32 bits not supported
******************************************************************************/
AieRC XAie_MdmPerfCounterReset(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u64 Offset;
	u8 TileType;
	const XAie_UcMdm *UcMdm;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(XAie_IsUcModulePresent(DevInst, TileType) == 0U) {
		XAIE_ERROR("Tile does not have uC module\n");
		return XAIE_INVALID_TILE;
	}

	/* Reset counters through control register */
	UcMdm = DevInst->DevProp.DevMod[TileType].UcMod->UcMdm;
	Offset = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		UcMdm->PerfCtrl->RegOff;
	
	/* covertiy CERT-C check*/
	if (_XAie_CheckPrecisionExceeds(UcMdm->PerfCtrl->Clear.Lsb,
			_XAie_MaxBitsNeeded(1U), MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	return XAie_Write32(DevInst, Offset, (1U << UcMdm->PerfCtrl->Clear.Lsb) &
			UcMdm->PerfCtrl->Clear.Mask);
}

/*****************************************************************************/
/**
*
* This API returns the values of the event control registers.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of SHIM Tile
* @param	UcEvents: Pointer to store configuration of event control registers.
*			  Expected to be large enough to store for all counters.
*
* @return	XAIE_OK on success, error code on failure.
******************************************************************************/
AieRC XAie_MdmPerfCounterGetControlConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
	u8 *UcEvents)
{
	AieRC RC;
	u64 Offset;
	u8 TileType, TotalCounters;
	const XAie_UcMdm *UcMdm;

	if((DevInst == XAIE_NULL) || (UcEvents == NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(XAie_IsUcModulePresent(DevInst, TileType) == 0U) {
		XAIE_ERROR("Tile does not have uC module\n");
		return XAIE_INVALID_TILE;
	}

	/* Reset counter access to first counter */
	UcMdm = DevInst->DevProp.DevMod[TileType].UcMod->UcMdm;
	Offset = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		UcMdm->PerfCtrl->RegOff;

	/* covertiy CERT-C check*/
	if (_XAie_CheckPrecisionExceeds(UcMdm->PerfCtrl->Reset.Lsb,
			_XAie_MaxBitsNeeded(1U), MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RC = XAie_Write32(DevInst, Offset, (1U << UcMdm->PerfCtrl->Reset.Lsb) &
			UcMdm->PerfCtrl->Reset.Mask) ;
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to reset register access\n");
		return RC;
	}

	/* Read counter event control registers */
	Offset = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		UcMdm->PerfEvents->RegOff;
	TotalCounters = (UcMdm->NumEventCounters + UcMdm->NumLatencyCounters) & 0xFFU;
	for(u8 i = 0; i < TotalCounters; i++) {
		u32 RegVal;

		RC = XAie_Read32(DevInst, Offset, &RegVal);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to read event control for register %d\n",
					i);
			return RC;
		}
		UcEvents[i] = (u8)(RegVal & 0xFFU);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API sets the start bit in the uC MDM performance command register.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of SHIM Tile
*
* @return	XAIE_OK on success, error code on failure.
******************************************************************************/
AieRC XAie_MdmPerfCounterStart(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u64 Offset;
	u8 TileType;
	const XAie_UcMdm *UcMdm;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(XAie_IsUcModulePresent(DevInst, TileType) == 0U) {
		XAIE_ERROR("Tile does not have uC module\n");
		return XAIE_INVALID_TILE;
	}

	/* Start counters using the control register */
	UcMdm = DevInst->DevProp.DevMod[TileType].UcMod->UcMdm;
	Offset = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		UcMdm->PerfCtrl->RegOff;
	
	/* covertiy CERT-C check*/
	if (_XAie_CheckPrecisionExceeds(UcMdm->PerfCtrl->Start.Lsb,
		_XAie_MaxBitsNeeded(1U), MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	return XAie_Write32(DevInst, Offset, (1U << UcMdm->PerfCtrl->Start.Lsb) &
			UcMdm->PerfCtrl->Start.Mask);
}

/*****************************************************************************/
/**
*
* This API sets the stop bit in the uC MDM performance command register. If
* SampleEnable is also set, sample bit will be set.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of SHIM Tile
* @param	SampleEnable: If XAIE_ENABLE command will also be issued
*			      to sample counters. XAIE_DISABLE to just stop
*			      counters.
*
* @return	XAIE_OK on success, error code on failure.
******************************************************************************/
AieRC XAie_MdmPerfCounterStop(XAie_DevInst *DevInst, XAie_LocType Loc,
	u8 SampleEnable)
{
	u64 Offset;
	u8 TileType;
	u32 FldVal, Mask;
	const XAie_UcMdm *UcMdm;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(XAie_IsUcModulePresent(DevInst, TileType) == 0U) {
		XAIE_ERROR("Tile does not have uC module\n");
		return XAIE_INVALID_TILE;
	}

	/* Stop counters using the control register */
	UcMdm = DevInst->DevProp.DevMod[TileType].UcMod->UcMdm;
	Offset = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		UcMdm->PerfCtrl->RegOff;

	/* covertiy CERT-C check*/
	if (_XAie_CheckPrecisionExceeds(UcMdm->PerfCtrl->Stop.Lsb ,
			_XAie_MaxBitsNeeded(1U), MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	Mask = UcMdm->PerfCtrl->Stop.Mask;
	FldVal = 1U << UcMdm->PerfCtrl->Stop.Lsb;
	/* If sample enabled, also set sample bit */
	if (SampleEnable) {
		/* covertiy CERT-C check*/
		if (_XAie_CheckPrecisionExceeds(UcMdm->PerfCtrl->Sample.Lsb ,
				_XAie_MaxBitsNeeded(1U), MAX_VALID_AIE_REG_BIT_INDEX)) {
			XAIE_ERROR("Check Precision Exceeds Failed\n");
			return XAIE_ERR;
		}
		
		Mask |= UcMdm->PerfCtrl->Sample.Mask;
		FldVal |= 1U << UcMdm->PerfCtrl->Sample.Lsb;
	}

	return XAie_Write32(DevInst, Offset, FldVal & Mask);
}

/*****************************************************************************/
/**
*
* This API sets the sample bit in the uC MDM performance command register.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of SHIM Tile
*
* @return	XAIE_OK on success, error code on failure.
******************************************************************************/
AieRC XAie_MdmPerfCounterSample(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u64 Offset;
	u8 TileType;
	const XAie_UcMdm *UcMdm;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(XAie_IsUcModulePresent(DevInst, TileType) == 0U) {
		XAIE_ERROR("Tile does not have uC module\n");
		return XAIE_INVALID_TILE;
	}

	/* Stop counters using the control register */
	UcMdm = DevInst->DevProp.DevMod[TileType].UcMod->UcMdm;
	Offset = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		UcMdm->PerfCtrl->RegOff;
	
	
	/* covertiy CERT-C check*/
	if (_XAie_CheckPrecisionExceeds(UcMdm->PerfCtrl->Sample.Lsb,
		_XAie_MaxBitsNeeded(1U), MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	return XAie_Write32(DevInst, Offset, (1U << UcMdm->PerfCtrl->Sample.Lsb) &
			UcMdm->PerfCtrl->Sample.Mask);
}


#endif /* XAIE_FEATURE_PERFCOUNT_ENABLE */
