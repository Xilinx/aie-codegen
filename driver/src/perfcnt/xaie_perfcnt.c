/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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
#include "xaie_feature_config.h"
#include "xaie_perfcnt.h"
#include "xaie_events.h"
#include "xaie_helper_internal.h"

#ifdef XAIE_FEATURE_PERFCOUNT_ENABLE

/*****************************************************************************/
/***************************** Macro Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API provides information whether a AIE4 device running in single app or not?
*
* @param	DevGen: device generation value.
* @param	AppMode: current device appmode.
*
* @return	TRUE : if the device supports dual application mode
* 			FALSE : if the device doesn't support dual application mode
*
* @note
*
*******************************************************************************/
static inline u8 _XAie4_DeviceInSingleAppMode(u8 DevGen, u8 AppMode)
{
	if ((_XAie_IsDeviceGenAIE4(DevGen) && AppMode == XAIE_DEVICE_SINGLE_APP_MODE))
		return true;
	else
		return false;
}
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
	u8 MaxCounterVal;
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
			CounterRegOffset = C * PerfMod->PerfCounterOffsetAdd;
			RC |= XAie_Read32(DevInst, CounterBaseAddr + CounterRegOffset, CounterVal + C);
		}
		
		/* when device is in single app mode and tile is mem/shim tile*/
		if (_XAie_IsTileResourceInSharedAddrSpace(DevInst->DevProp.DevGen, TileType) &&
					(_XAie4_DeviceInSingleAppMode(DevInst->DevProp.DevGen, DevInst->AppMode))) {		
			CounterBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterBaseAddr);
			CounterVal += PerfMod->MaxCounterVal;
			for (u8 C = 0; C < PerfMod->MaxCounterVal; C++) {
				/* Add offset address based on Counter and read */
				CounterRegOffset = C * PerfMod->PerfCounterOffsetAdd;
				RC |= XAie_Read32(DevInst, CounterBaseAddr + CounterRegOffset, CounterVal + C);
			}
		}
	} else {
		if(_XAie4_DeviceInSingleAppMode(DevInst->DevProp.DevGen, DevInst->AppMode)) {
			if(Counter >= PerfMod->MaxCounterVal) {
				CounterBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterBaseAddr);
				Counter -= PerfMod->MaxCounterVal;
			}
		}
		
		CounterRegOffset = Counter * PerfMod->PerfCounterOffsetAdd;
		
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
	u32 MaxCounterVal;
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
	
	if(_XAie4_DeviceInSingleAppMode(DevInst->DevProp.DevGen, DevInst->AppMode)) {
		if(Counter >= PerfMod->MaxCounterVal){
			CounterBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterBaseAddr);
			Counter -= PerfMod->MaxCounterVal;
		}		
	}
	
	*Offset = CounterBaseAddr + (Counter * PerfMod->PerfCounterOffsetAdd);
		
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
	u32 MaxCounterVal;
	const XAie_PerfMod *PerfMod;
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
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[0U];
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	} else {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[Module];
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	}

	/* Check for perf counter support for tiletype and module*/
	if(PerfMod->MaxCounterVal == 0) {
		XAIE_ERROR("Perf counters are not suported for tile type %d and module %d\n", TileType, Module);
		return XAIE_INVALID_ARGS;
	}

	/* check if the event passed as input is corresponding to the module */
	if(StartEvent < EvntMod->EventMin || StartEvent > EvntMod->EventMax ||
		StopEvent < EvntMod->EventMin || StopEvent > EvntMod->EventMax) {
		XAIE_ERROR("Invalid Event id\n");
		return XAIE_INVALID_ARGS;
	}

	/* Subtract the module offset from event number */
	StartEvent -= EvntMod->EventMin;
	StopEvent -= EvntMod->EventMin;

	/* Getting the true event number from the enum to array mapping */
	IntStartEvent = EvntMod->XAie_EventNumber[StartEvent];
	IntStopEvent = EvntMod->XAie_EventNumber[StopEvent];

	/*checking for valid true event number */
	if(IntStartEvent == XAIE_EVENT_INVALID ||
			IntStopEvent == XAIE_EVENT_INVALID) {
		XAIE_ERROR("Invalid Event id\n");
		return XAIE_INVALID_ARGS;
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
	if(_XAie4_DeviceInSingleAppMode(DevInst->DevProp.DevGen, DevInst->AppMode)) {
		if(Counter >= PerfMod->MaxCounterVal){
			CtrlBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CtrlBaseAddr);
			Counter -= PerfMod->MaxCounterVal;
		}
	}
	
	CtrlRegOffset = (Counter / 2U * PerfMod->PerfCtrlOffsetAdd);

	/* Compute mask for performance control register */
	FldMask = (PerfMod->Start.Mask | PerfMod->Stop.Mask) <<
				(PerfMod->StartStopShift * (Counter % 2U));

	/* Compute value to be written to the performance control register */
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
	u32 MaxCounterVal;	
	const XAie_PerfMod *PerfMod;
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
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[0U];
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	} else {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[Module];
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	}

	/* Check for perf counter support for tiletype and module*/
	if(PerfMod->MaxCounterVal == 0) {
		XAIE_ERROR("Perf counters are not suported for tile type %d and module %d\n", TileType, Module);
		return XAIE_INVALID_ARGS;
	}

	/* check if the event passed as input is corresponding to the module */
	if(ResetEvent < EvntMod->EventMin || ResetEvent > EvntMod->EventMax) {
		XAIE_ERROR("Invalid Event id: %d\n", ResetEvent);
		return XAIE_INVALID_ARGS;
	}

	/* Subtract the module offset from event number */
	ResetEvent -= EvntMod->EventMin;

	/* Getting the true event number from the enum to array mapping */
	IntResetEvent = EvntMod->XAie_EventNumber[ResetEvent];

	/*checking for valid true event number */
	if(IntResetEvent == XAIE_EVENT_INVALID) {
		XAIE_ERROR("Invalid Event id: %d\n", ResetEvent);
		return XAIE_INVALID_ARGS;
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
	if(_XAie4_DeviceInSingleAppMode(DevInst->DevProp.DevGen, DevInst->AppMode)) {
		if(Counter >= PerfMod->MaxCounterVal){
			ResetBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, ResetBaseAddr);
			Counter -= PerfMod->MaxCounterVal;
		}
	}
	
	ResetRegOffset = (Counter / 4U * PerfMod->PerfResetOffsetAdd);

	/* Compute mask for performance control register */
	ResetFldMask = PerfMod->Reset.Mask <<
					(PerfMod->ResetShift * (Counter % 4U));
	/* Compute value to be written to the performance control register */
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
	u32 MaxCounterVal;
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
	if(_XAie4_DeviceInSingleAppMode(DevInst->DevProp.DevGen, DevInst->AppMode)) {
		if(Counter >= PerfMod->MaxCounterVal){
			CounterBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterBaseAddr);
			Counter -= PerfMod->MaxCounterVal;
		}
	}
	
	CounterRegOffset = Counter * PerfMod->PerfCounterOffsetAdd;

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
	u32 MaxCounterVal;
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
	if(_XAie4_DeviceInSingleAppMode(DevInst->DevProp.DevGen, DevInst->AppMode)) {
		if(Counter >= PerfMod->MaxCounterVal){
			CounterEvtValBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterEvtValBaseAddr);
			Counter -= PerfMod->MaxCounterVal;
		}		
	}
	
	CounterEvtValRegOffset = Counter * PerfMod->PerfCounterOffsetAdd;

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
	u32 MaxCounterVal;
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
	if(_XAie4_DeviceInSingleAppMode(DevInst->DevProp.DevGen, DevInst->AppMode)) {
		if(Counter >= PerfMod->MaxCounterVal){
			StartStopRegAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, StartStopRegAddr);
			Counter -= PerfMod->MaxCounterVal;
		}
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
	RC = XAie_EventPhysicalToLogicalConv(DevInst, Loc, Module, (u8)RegEvent,
			StartEvent);
	if (RC != XAIE_OK) {
		return RC;
	}

	RegEvent = (StartStopEvent & PerfMod->Stop.Mask) >>
			PerfMod->StartStopShift / 2U;
	RC = XAie_EventPhysicalToLogicalConv(DevInst, Loc, Module, (u8)RegEvent,
			StopEvent);
	if (RC != XAIE_OK) {
		return RC;
	}

	/* Compute absolute address and read the reset event register */
	/* Compute register address without offset */
	ResetRegAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
						PerfMod->PerfCtrlResetBaseAddr;		

	/* Get offset address based on Counter */
	if(_XAie4_DeviceInSingleAppMode(DevInst->DevProp.DevGen, DevInst->AppMode)) {
		if(Counter >= PerfMod->MaxCounterVal){
			ResetRegAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, ResetRegAddr);
			Counter -= PerfMod->MaxCounterVal;
		}
	}
	
	ResetRegOffset = (Counter / 4U *  PerfMod->PerfResetOffsetAdd);

	RC = XAie_Read32(DevInst, ResetRegAddr + ResetRegOffset, &RegEvent);
	if(RC != XAIE_OK) {
		return RC;
	}

	/* Get reset event for given counter and store in the event pointer */
	RegEvent >>= PerfMod->ResetShift * (Counter % 4U);
	RegEvent &= PerfMod->Reset.Mask;
	RC = XAie_EventPhysicalToLogicalConv(DevInst, Loc, Module, (u8)RegEvent,
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
	u8 MaxCounterVal;
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
			CounterSsRegOffset = C * PerfMod->PerfCounterOffsetAdd;
			RC |= XAie_Read32(DevInst, CounterSsBaseAddr + CounterSsRegOffset, CounterVal + C);
		}

		/* when device is in single app mode and tile is mem/shim tile*/
		if (_XAie_IsTileResourceInSharedAddrSpace(DevInst->DevProp.DevGen, TileType) &&
					(_XAie4_DeviceInSingleAppMode(DevInst->DevProp.DevGen, DevInst->AppMode))) {
			CounterSsBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterSsBaseAddr);
			CounterVal += PerfMod->MaxCounterVal;
			for (u8 C = 0; C < PerfMod->MaxCounterVal; C++) {
				/* Add offset address based on Counter and read */
				CounterSsRegOffset = C * PerfMod->PerfCounterOffsetAdd;
				RC |= XAie_Read32(DevInst, CounterSsBaseAddr + CounterSsRegOffset, CounterVal + C);
			}
		}
	} else {
		if(_XAie4_DeviceInSingleAppMode(DevInst->DevProp.DevGen, DevInst->AppMode)) {
			if(Counter >= PerfMod->MaxCounterVal){
				CounterSsBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterSsBaseAddr);
				Counter -= PerfMod->MaxCounterVal;
			}				
		} 
			
		CounterSsRegOffset = Counter * PerfMod->PerfCounterOffsetAdd;

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
	u32 MaxCounterVal;
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
	RC = _XAie_CheckModule(DevInst, Loc, Module);
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
	if(_XAie4_DeviceInSingleAppMode(DevInst->DevProp.DevGen, DevInst->AppMode)) {
		if(Counter >= PerfMod->MaxCounterVal){
			CounterSsBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterSsBaseAddr);
			Counter -= PerfMod->MaxCounterVal;
		}
	}
	
	CounterSsRegOffset = Counter * PerfMod->PerfCounterOffsetAdd;
	
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
		XAie_ModuleType Module, u32 EventVal)
{	
	u64 CounterSsLoadEvtBaseAddr;
	u8 TileType;
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
	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[0U];
	} else {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[Module];
	}
	
	/* Compute register address without offset */
	CounterSsLoadEvtBaseAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
						PerfMod->PerfCounterSsLoadEvttBaseAddr;	

	RC |= XAie_Write32(DevInst, CounterSsLoadEvtBaseAddr, EventVal);

	/* if device in single app and tile is mem/shim tile*/
	if(_XAie_IsTileResourceInSharedAddrSpace(DevInst->DevProp.DevGen, TileType)  &&
			(_XAie4_DeviceInSingleAppMode(DevInst->DevProp.DevGen, DevInst->AppMode))) {
		CounterSsLoadEvtBaseAddr = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, CounterSsLoadEvtBaseAddr);

		RC |= XAie_Write32(DevInst, CounterSsLoadEvtBaseAddr, EventVal);
	}

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
	return XAie_PerfCounterSnapshotLoadEventSet(DevInst, Loc, Module, 0U);
}


#endif /* XAIE_FEATURE_PERFCOUNT_ENABLE */
