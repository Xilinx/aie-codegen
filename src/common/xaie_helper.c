/******************************************************************************
* Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_helper.c
* @{
*
* This file contains inline helper functions for AIE drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   09/24/2019  Fix range check logic for shim row
* 1.2   Tejus   01/04/2020  Cleanup error messages
* 1.3   Tejus   04/13/2020  Add api to get tile type from Loc
* 1.4   Tejus   04/13/2020  Remove helper functions for range apis
* 1.5   Dishita 04/29/2020  Add api to check module & tile type combination
* 1.7   Nishad  07/24/2020  Add _XAie_GetFatalGroupErrors() helper function.
* 1.8   Dishita 08/10/2020  Add api to get bit position from tile location
* 1.9   Nishad  08/26/2020  Fix tiletype check in XAie_CheckModule()
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_helper.h"
#include "xaie_helper_internal.h"
#include "xaie_reset_aie.h"
#include "xaie_core.h"
#include "xaie_txn.h"
#include "xaie_instbuf.h"

#ifdef __AIESOCKET__
	#define XAIE4_APP_B_OFFSET  0x08000000U
#else
	#define XAIE4_APP_B_OFFSET 0x0U
#endif

/************************** Constant Definitions *****************************/
#define XAIE_DEV_GEN_AIE4_AIE_TILE_SHIFT_OFFSET 4U
#define XAIE_DEV_GEN_AIE4_MEM_TILE_SHIFT_OFFSET 5U

#define U64_MAX 0xFFFFFFFFFFFFFFFFU
/************************** Variable Definitions *****************************/

/***************************** Macro Definitions *****************************/
/************************** Function Definitions *****************************/
/******************************************************************************/
/**
*
* This is the function used to get the tile type for a given device instance
* and tile location.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @return	TileType (AIETILE/MEMTILE/SHIMPL/SHIMNOC on success and MAX on
*		error)
*
* @note		Internal API only.
*
******************************************************************************/
u8 XAie_GetTileTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 ColType;

	if(Loc.Col >= DevInst->NumCols) {
		XAIE_ERROR("Invalid column: %d\n", Loc.Col);
		return XAIEGBL_TILE_TYPE_MAX;
	}

	if(Loc.Row == 0U) {
		ColType = Loc.Col % 4U;
		if((ColType == 0U) || (ColType == 1U)) {
			return XAIEGBL_TILE_TYPE_SHIMPL;
		}

		return XAIEGBL_TILE_TYPE_SHIMNOC;

	} else if(Loc.Row >= DevInst->MemTileRowStart &&
			(Loc.Row < (DevInst->MemTileRowStart +
				     DevInst->MemTileNumRows))) {
		return XAIEGBL_TILE_TYPE_MEMTILE;
	} else if (Loc.Row >= DevInst->AieTileRowStart &&
			(Loc.Row < (DevInst->AieTileRowStart +
				     DevInst->AieTileNumRows))) {
		return XAIEGBL_TILE_TYPE_AIETILE;
	}

	XAIE_ERROR("Cannot find Tile Type\n");

	return XAIEGBL_TILE_TYPE_MAX;
}

/*****************************************************************************/
/**
*
* This is the function used to get the tile type for a given device instance
* and tile location. This function will be removed once other teams migrate
* to the new global function
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @return	TileType (AIETILE/MEMTILE/SHIMPL/SHIMNOC on success and MAX on
*		error)
*
* @note		Internal API only.
*
******************************************************************************/

u8 _XAie_GetTileTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	return XAie_GetTileTypefromLoc(DevInst, Loc);
}

/*******************************************************************************/
/**
* This function is used to check for module and tiletype combination.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of the AIE tile.
* @param	Module:	XAIE_MEM_MOD - memory module
* 			XAIE_CORE_MOD - core module
* 			XAIE_PL_MOD - pl module
* @return       XAIE_OK for correct combination of Module and tile type
* 		XAIE_INVALID_ARGS for incorrect combination of module and tile
* 		type
*
* @note         Internal API only.
*
*******************************************************************************/

AieRC XAie_CheckModule(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Module)
{
	u8 TileType;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_AIETILE && Module > XAIE_CORE_MOD) {
		XAIE_ERROR("Invalid Module\n");
		return XAIE_INVALID_ARGS;
	}

	if((TileType == XAIEGBL_TILE_TYPE_SHIMPL ||
	    TileType == XAIEGBL_TILE_TYPE_SHIMNOC) && Module != XAIE_PL_MOD) {
		XAIE_ERROR("Invalid Module\n");
		return XAIE_INVALID_ARGS;
	}

	if(TileType == XAIEGBL_TILE_TYPE_MEMTILE &&
		Module != XAIE_MEM_MOD) {
		XAIE_ERROR("Invalid Module\n");
		return XAIE_INVALID_ARGS;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This function is used to check for module and tiletype combination.
* This function will be removed once other teams migrate to the new global function
*
* @param        DevInst: Device Instance
* @param        Loc: Location of the AIE tile.
* @param	Module:	XAIE_MEM_MOD - memory module
* 			XAIE_CORE_MOD - core module
* 			XAIE_PL_MOD - pl module
* @return       XAIE_OK for correct combination of Module and tile type
* 		XAIE_INVALID_ARGS for incorrect combination of module and tile
* 		type
*
* @note         Internal API only.
*
*******************************************************************************/
AieRC _XAie_CheckModule(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Module)
{
	return XAie_CheckModule(DevInst, Loc, Module);
}

/*******************************************************************************/
/**
* This function is used to get no. of rows for the given tiletype.
*
* @param        DevInst: Device Instance
* @param        TileType: Type of tile
*
* @return       BitmapNumRows: Number of rows for given tiletype
*
* @note         Internal API only.
*
*******************************************************************************/
u32 XAie_GetNumRows(XAie_DevInst *DevInst, u8 TileType)
{
	u32 NumRows;

	switch(TileType) {
	case XAIEGBL_TILE_TYPE_SHIMNOC:
	case XAIEGBL_TILE_TYPE_SHIMPL:
	{       NumRows = 1U;
		break;
	}
	case XAIEGBL_TILE_TYPE_AIETILE:
	{	NumRows = DevInst->AieTileNumRows;
		break;
	}
	case XAIEGBL_TILE_TYPE_MEMTILE:
	{	NumRows = DevInst->MemTileNumRows;
		break;
	}
	default:
	{
		XAIE_ERROR("Invalid Tiletype\n");
		return 0;
	}
	}

	return NumRows;
}

/*****************************************************************************/
/**
* This function is used to get no. of rows for the given tiletype.
* This function will be removed once other teams migrate to the new global function
*
* @param        DevInst: Device Instance
* @param        TileType: Type of tile
*
* @return       BitmapNumRows: Number of rows for given tiletype
*
* @note         Internal API only.
*
*******************************************************************************/
u32 _XAie_GetNumRows(XAie_DevInst *DevInst, u8 TileType)
{
	return XAie_GetNumRows(DevInst, TileType);
}


/*******************************************************************************/
/**
* This function is used to get start row for the given tiletype.
*
* @param        DevInst: Device Instance
* @param        TileType: Type of tile
*
* @return       StartRow: Start row for given tiletype
*
* @note         Internal API only.
*
*******************************************************************************/
u32 XAie_GetStartRow(XAie_DevInst *DevInst, u8 TileType)
{
	u32 StartRow;

	switch(TileType) {
	case XAIEGBL_TILE_TYPE_SHIMNOC:
	case XAIEGBL_TILE_TYPE_SHIMPL:
	{	StartRow = DevInst->ShimRow;
		break;
	}
	case XAIEGBL_TILE_TYPE_AIETILE:
	{
		StartRow = DevInst->AieTileRowStart;
		break;
	}
	case XAIEGBL_TILE_TYPE_MEMTILE:
	{
		StartRow = DevInst->MemTileRowStart;
		break;
	}
	default:
	{
		XAIE_ERROR("Invalid Tiletype\n");
		return 0;
	}
	}

	return StartRow;
}

/*****************************************************************************/
/**
* This function is used to get start row for the given tiletype.
* This function will be removed once other teams migrate to the new global function
*
* @param        DevInst: Device Instance
* @param        TileType: Type of tile
*
* @return       StartRow: Start row for given tiletype
*
* @note         Internal API only.
*
*******************************************************************************/
u32 _XAie_GetStartRow(XAie_DevInst *DevInst, u8 TileType)
{
	return XAie_GetStartRow(DevInst, TileType);
}

/*****************************************************************************/
/**
*
* This API returns the default value of group errors marked as fatal.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
*
* @return	Default value of group fatal errors.
*
* @note		Internal API only.
*
******************************************************************************/
u32 _XAie_GetFatalGroupErrors(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module)
{
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if (Module == XAIE_PL_MOD) {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	} else {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	}

	return EvntMod->DefaultGroupErrorMask;
}

void XAie_Log(FILE *Fd, const char *prefix, const char *func, u32 line,
		const char *Format, ...)
{
	va_list ArgPtr;
	va_start(ArgPtr, Format);
	if (fprintf(Fd, "%s %s():%u: ", prefix, func, line) == -1) {
		va_end(ArgPtr);
		return;
	}
	if (vfprintf(Fd, Format, ArgPtr) == -1){
		va_end(ArgPtr);
		return;
	}
	va_end(ArgPtr);
}

/*****************************************************************************/
/**
* This is an internal API to get bit position corresponding to tile location in
* bitmap. This bitmap does not represent Shim tile so this API
* only accepts AIE tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE tile
* @return       Bit position in the TilesInUse bitmap
*
* @note         None
*
******************************************************************************/
u32 _XAie_GetTileBitPosFromLoc(XAie_DevInst *DevInst, XAie_LocType Loc)
{
    return (u32)((Loc.Col + DevInst->StartCol) * (DevInst->NumRows - 1U) + Loc.Row - 1U);
}

/*******************************************************************************/
/**
 * This API populates ungated tiles of partition to Locs list.
 *
 * @param        DevInst: Device Instance
 * @param        NumTiles: Size of Locs array.
 * @param        Locs: Pointer to tile locations list
 *
 * @note         NumTiles pointer is used to indicate the size of Locs as input
 *               when passed by the caller. The same pointer gets updated to
 *               indicate the return locs list size.
 *
 *******************************************************************************/
AieRC XAie_GetUngatedLocsInPartition(XAie_DevInst *DevInst, u32 *NumTiles,
		XAie_LocType *Locs)
{
	u32 Index = 0;

	/* Add clock enabled tiles of the partition to Rscs */
	for(u8 Col = 0; Col < DevInst->NumCols; Col++) {
		for(u8 Row = 0; Row < DevInst->NumRows; Row++) {
			XAie_LocType Loc = XAie_TileLoc(Col, Row);

			if(_XAie_PmIsTileRequested(DevInst, Loc) == XAIE_ENABLE) {
				if(Index >= *NumTiles) {
					XAIE_ERROR("Invalid NumTiles: %d\n",
							*NumTiles);
					return XAIE_INVALID_ARGS;
				}

				Locs[Index] = Loc;
				Index++;
			}
		}
	}

	/* Update NumTiles to size equal to ungated tiles in partition */
	*NumTiles = Index;
	return XAIE_OK;
}

/*****************************************************************************/
/**
 * This API populates ungated tiles of partition to Locs list.
* This function will be removed once other teams migrate to the new global function
 *
 * @param        DevInst: Device Instance
 * @param        NumTiles: Size of Locs array.
 * @param        Locs: Pointer to tile locations list
 *
 * @note         NumTiles pointer is used to indicate the size of Locs as input
 *               when passed by the caller. The same pointer gets updated to
 *               indicate the return locs list size.
 *
 *******************************************************************************/
AieRC _XAie_GetUngatedLocsInPartition(XAie_DevInst *DevInst, u32 *NumTiles,
		XAie_LocType *Locs)
{
	return XAie_GetUngatedLocsInPartition(DevInst, NumTiles, Locs);
}

/*****************************************************************************/
/**
* This API sets given number of bits from given start bit in the given bitmap.
*
* @param        Bitmap: bitmap to be set (NULL if bitmap tracking disabled)
* @param        StartSetBit: Bit position in the bitmap
* @param        NumSetBit: Number of bits to be set.
*
* @return       none
*
* @note         This API is internal, hence all the argument checks are taken
*               care of in the caller API.
*               For device generations that don't require bitmap tracking,
*               Bitmap will be NULL and this function safely returns without
*               accessing memory. This handles all bitmap sizes: NULL (disabled),
*               1 word, 32 words, or any other valid size.
*
******************************************************************************/
void _XAie_SetBitInBitmap(u32 *Bitmap, u32 StartSetBit,
		u32 NumSetBit)
{
	/*
	 * Handle NULL bitmap (tracking disabled) or zero bits to set.
	 * For device generations like AIE2P/AIE2IPU that don't need bitmap
	 * tracking, the DevOps pointers are set to NULL explicitly.
	 */
	if ((Bitmap == NULL) || (NumSetBit == 0U)) {
		return;
	}

	for(u32 i = StartSetBit; i < StartSetBit + NumSetBit; i++) {
		u32 WordIndex = i / (sizeof(Bitmap[0]) * 8U);
		u32 BitIndex = i % (sizeof(Bitmap[0]) * 8U);
		Bitmap[WordIndex] |= (u32)(1U << BitIndex);
	}
}

/*****************************************************************************/
/**
* This API clears number of bits from given start bit in the given bitmap.
*
* @param        Bitmap: bitmap to be cleared (NULL if bitmap tracking disabled)
* @param        StartBit: Bit position in the bitmap
* @param        NumBit: Number of bits to be cleared.
*
* @return       None
*
* @note         This API is internal, hence all the argument checks are taken
*               care of in the caller API.
*               For device generations that don't require bitmap tracking,
*               Bitmap will be NULL and this function safely returns without
*               accessing memory. This handles all bitmap sizes: NULL (disabled),
*               1 word, 32 words, or any other valid size.
*
******************************************************************************/
void _XAie_ClrBitInBitmap(u32 *Bitmap, u32 StartBit, u32 NumBit)
{
	/*
	 * Handle NULL bitmap (tracking disabled) or zero bits to clear.
	 * For device generations like AIE2P/AIE2IPU that don't need bitmap
	 * tracking, the DevOps pointers are set to NULL explicitly.
	 */
	if ((Bitmap == NULL) || (NumBit == 0U)) {
		return;
	}

	for(u32 i = StartBit; i < StartBit + NumBit; i++) {
		u32 WordIndex = i / (sizeof(Bitmap[0]) * 8U);
		u32 BitIndex = i % (sizeof(Bitmap[0]) * 8U);
		Bitmap[WordIndex] &= ~(u32)(1U << BitIndex);
	}
}

AieRC XAie_Write32(XAie_DevInst *DevInst, u64 RegOff, u32 Value)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		return XAie_Txn_Write32(DevInst, RegOff, Value);
	}

	if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {
		RegOff|= XAIE4_APP_B_OFFSET;
	}

	if (_XAie_IsInstBufRecording(DevInst)) {
		return _XAie_Write32InstBuf(DevInst, RegOff, Value);
	}

	return Backend->Ops.Write32((void*)(DevInst->IOInst), RegOff, Value);
}

AieRC XAie_Read32(XAie_DevInst *DevInst, u64 RegOff, u32 *Data)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		return XAie_Txn_Read32(DevInst, RegOff, Data);
	}

	if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {
		RegOff|= XAIE4_APP_B_OFFSET;
	}

	if (_XAie_IsInstBufRecording(DevInst)) {
		return _XAie_Read32InstBuf(DevInst, RegOff, Data);
	}

	return Backend->Ops.Read32((void*)(DevInst->IOInst), RegOff, Data);
}

AieRC XAie_MaskWrite32(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		return XAie_Txn_MaskWrite32(DevInst, RegOff, Mask, Value);
	}

	if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {
			RegOff|= XAIE4_APP_B_OFFSET;
	}

	if (_XAie_IsInstBufRecording(DevInst)) {
		return _XAie_MaskWrite32InstBuf(DevInst, RegOff, Mask, Value);
	}

	return Backend->Ops.MaskWrite32((void *)(DevInst->IOInst), RegOff, Mask,
			Value);
}

AieRC XAie_MaskPoll(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		return XAie_Txn_MaskPoll(DevInst, RegOff, Mask, Value, TimeOutUs);
	}

	if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {
			RegOff|= XAIE4_APP_B_OFFSET;
	}
	return Backend->Ops.MaskPoll((void*)(DevInst->IOInst), RegOff, Mask,
			Value, TimeOutUs);
}

AieRC XAie_MaskPollBusy(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		return XAie_Txn_MaskPollBusy(DevInst, RegOff, Mask, Value, TimeOutUs);
	}
	if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {
			RegOff|= XAIE4_APP_B_OFFSET;
	}
	return Backend->Ops.MaskPoll((void*)(DevInst->IOInst), RegOff, Mask,
			Value, TimeOutUs);
}

AieRC XAie_BlockWrite32(XAie_DevInst *DevInst, u64 RegOff, const u32 *Data, u32 Size)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		return XAie_Txn_BlockWrite32(DevInst, RegOff, Data, Size);
	}

	if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {
			RegOff|= XAIE4_APP_B_OFFSET;
	}
	return Backend->Ops.BlockWrite32((void *)(DevInst->IOInst), RegOff,
			Data, Size);
}

AieRC XAie_BlockSet32(XAie_DevInst *DevInst, u64 RegOff, u32 Data, u32 Size)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		return XAie_Txn_BlockSet32(DevInst, RegOff, Data, Size);
	}

	if(DevInst->AppMode == XAIE_DEVICE_DUAL_APP_MODE_B) {
			RegOff|= XAIE4_APP_B_OFFSET;
	}
	return Backend->Ops.BlockSet32((void *)(DevInst->IOInst), RegOff, Data,
			Size);
}

AieRC XAie_CmdWrite(XAie_DevInst *DevInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		/**
		 * This function is only used in XAie_LoadElf() on AIESIM platform.
		 * In the past the elf loading was done via XCLBIN (PDI) but not
		 * via TXN binary. Hence this unwanted TXN implementation did not have
		 * any side effects. But when elf loading is attempted via TXN flow 
		 * this needs to be made a NOOP else it fails. Hence Making 
		 * XAIe_CmdWrite no-op for transaction mode.
		 **/
		return XAIE_OK;
	}
	return Backend->Ops.CmdWrite((void *)(DevInst->IOInst), Col, Row,
			Command, CmdWd0, CmdWd1, CmdStr);
}

AieRC XAie_RunOp(XAie_DevInst *DevInst, XAie_BackendOpCode Op, void *Arg)
{
	AieRC RC;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		RC = XAie_Txn_RunOp(DevInst, (u8)Op == XAIE_BACKEND_OP_CONFIG_SHIMDMABD, Arg);
		if (RC == XAIE_NOT_SUPPORTED)
			return Backend->Ops.RunOp(DevInst->IOInst, DevInst, Op, Arg);
		else
			return RC;
	}

	return Backend->Ops.RunOp(DevInst->IOInst, DevInst, Op, Arg);
}

AieRC XAie_WaitTct(XAie_DevInst *DevInst, uint16_t Column, uint16_t Row, uint32_t Channel, uint8_t NumTokens)
{
	const XAie_Backend *Backend = DevInst->Backend;
	if( Backend->Ops.WaitTaskCompleteToken )
	{
		return Backend->Ops.WaitTaskCompleteToken(DevInst, Column, Row, Channel, NumTokens);
	}
	XAIE_ERROR("Wait TCT function pointer points to NULL\n");
	return XAIE_NOT_SUPPORTED;
}

AieRC XAie_AddressPatching(XAie_DevInst *DevInst, u16 Arg_Offset, u8 Num_BDs)
{
	const XAie_Backend *Backend = DevInst->Backend;
	
	if (Backend->Ops.AddressPatching != NULL) {
		return Backend->Ops.AddressPatching((void *)DevInst->IOInst, Arg_Offset, Num_BDs);
	} else {
		XAIE_ERROR("Address Patching function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}
}

AieRC XAie_SetPadInteger(XAie_DevInst *DevInst, char* BuffName, u32 BuffSize)
{
	const XAie_Backend *Backend = DevInst->Backend;
	
	if (Backend->Ops.SetPadInteger != NULL) {
		return Backend->Ops.SetPadInteger((void *)DevInst->IOInst, BuffName, BuffSize);
	} else {
		XAIE_ERROR("SetPadInteger function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}
}

AieRC XAie_SetPadString(XAie_DevInst *DevInst, char* BuffName, char* BuffBlobPath)
{
	const XAie_Backend *Backend = DevInst->Backend;
	
	if (Backend->Ops.SetPadString != NULL) {
		return Backend->Ops.SetPadString((void *)DevInst->IOInst, BuffName, BuffBlobPath);
	} else {
		XAIE_ERROR("SetPadString function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}
}

AieRC XAie_WaitUCDMA(XAie_DevInst *DevInst)
{
	const XAie_Backend *Backend = DevInst->Backend;
	
	if (Backend->Ops.WaitUcDMA != NULL) {
		return Backend->Ops.WaitUcDMA((void *)DevInst->IOInst);
	} else {
		XAIE_ERROR("WaitUCDMA function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}
}

AieRC XAie_ModeConfig(XAie_DevInst *DevInst, XAie_ModeSelect Mode)
{
	const XAie_Backend *Backend = DevInst->Backend;
	
	if (Backend->Ops.ConfigMode != NULL) {
		return Backend->Ops.ConfigMode((void *)DevInst->IOInst, Mode);
	} else {
		XAIE_ERROR("ConfigMode function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}
}

XAie_ModeSelect XAie_GetModeConfig(XAie_DevInst *DevInst)
{
	const XAie_Backend *Backend = DevInst->Backend;
	
	if (Backend->Ops.GetConfigMode != NULL) {
		return Backend->Ops.GetConfigMode((void *)DevInst->IOInst);
	} else {
		XAIE_ERROR("GetConfigMode function pointer points to NULL, hence returned mode is XAIE_INVALID_MODE\n");
		return XAIE_INVALID_MODE;
	}
}

AieRC XAie_Preempt(XAie_DevInst *DevInst, u16 PreemptId, char* SaveLabel, char* RestoreLabel, u32* HintMap, u32 HintMapSizeInWords)
{
	const XAie_Backend *Backend = DevInst->Backend;
	if (Backend->Ops.Preempt != NULL)
	{
		return Backend->Ops.Preempt((void *)DevInst->IOInst, PreemptId, SaveLabel, RestoreLabel, HintMap, HintMapSizeInWords);
	}
	else
	{
		XAIE_ERROR("Preempt function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}
}

AieRC XAie_AttachToGroup(XAie_DevInst *DevInst, uint8_t UcIndex)
{
	const XAie_Backend *Backend = DevInst->Backend;
	if (Backend->Ops.AttachToGroup != NULL)
	{
		return Backend->Ops.AttachToGroup((void *)DevInst->IOInst, UcIndex);
	}
	else
	{
		XAIE_ERROR("AttachToGroup function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}
}

AieRC XAie_RemoteBarrier(XAie_DevInst *DevInst,  uint8_t RbId, uint32_t UcMask)
{
	const XAie_Backend *Backend = DevInst->Backend;
	if (Backend->Ops.RemoteBarrier != NULL)
	{
		return Backend->Ops.RemoteBarrier((void *)DevInst->IOInst, RbId, UcMask);
	}
	else
	{
		XAIE_ERROR("RemoteBarrier function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}
}

AieRC XAie_SaveRegister(XAie_DevInst *DevInst, u32 RegOff, u32 Id)
{
	const XAie_Backend *Backend = DevInst->Backend;
	if (Backend->Ops.SaveRegister != NULL)
	{
		return Backend->Ops.SaveRegister((void *)DevInst->IOInst, RegOff, Id);
	}
	else
	{
		XAIE_ERROR("SaveRegister function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}
}

AieRC XAie_Nop(XAie_DevInst *DevInst)
{
	const XAie_Backend *Backend = DevInst->Backend;
	if (Backend->Ops.Nop != NULL)
	{
		return Backend->Ops.Nop((void *)DevInst->IOInst);
	}
	else
	{
		XAIE_ERROR("Nop function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}
}

AieRC XAie_LoadCoresStart(XAie_DevInst *DevInst, u32 UniqueCoreElfId, const char* Label) {
	const XAie_Backend *Backend = DevInst->Backend;
	if (Backend->Ops.LoadCoresStart != NULL)
	{
		return Backend->Ops.LoadCoresStart((void *)DevInst->IOInst, UniqueCoreElfId, Label);
	}
	else
	{
		XAIE_ERROR("LoadCoresStart function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}
}

AieRC XAie_LoadCoresEnd(XAie_DevInst *DevInst) {
	const XAie_Backend *Backend = DevInst->Backend;
	if (Backend->Ops.LoadCoresEnd != NULL)
	{
		return Backend->Ops.LoadCoresEnd((void *)DevInst->IOInst);
	}
	else
	{
		XAIE_ERROR("LoadCoresEnd function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}
}

AieRC XAie_LoadCoresCPStart(XAie_DevInst *DevInst, u32 UniqueCoreElfId) {
	const XAie_Backend *Backend = DevInst->Backend;
	if (Backend->Ops.LoadCoresCPStart != NULL)
	{
		return Backend->Ops.LoadCoresCPStart((void *)DevInst->IOInst, UniqueCoreElfId);
	}
	else
	{
		XAIE_ERROR("LoadCoresCPStart function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}
}

AieRC XAie_LoadCoresCPEnd(XAie_DevInst *DevInst) {
	const XAie_Backend *Backend = DevInst->Backend;
	if (Backend->Ops.LoadCoresCPEnd != NULL)
	{
		return Backend->Ops.LoadCoresCPEnd((void *)DevInst->IOInst);
	}
	else
	{
		XAIE_ERROR("LoadCoresCPEnd function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}
}

/*****************************************************************************/
/**
*
* This API returns the Aie Tile Core status for a particular column and row.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Loc: Location of AIE tile
*
* @return	XAIE_OK for success and error code otherwise.
*
* @note	Internal only.
*
******************************************************************************/
static AieRC _XAie_CoreStatusDump(XAie_DevInst *DevInst,
		XAie_ColStatus *Status, XAie_LocType Loc)
{
	u32 RegVal;
	AieRC RC;
	u8 TileType, TileStart, Index;

	TileStart = DevInst->AieTileRowStart;
	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);

	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		/* Core status is only applicable for AIE tiles, skip others */
		return XAIE_OK;
	}

	/* core status */
	RC = XAie_CoreGetStatus(DevInst, Loc, &RegVal);
	if (RC != XAIE_OK) {
		return RC;
	}
	if(Loc.Row < TileStart){
		XAIE_ERROR("Loc.Row should not be less than TileStart\n");
		return XAIE_ERR;
	}else {
		Index = Loc.Row - TileStart;
	}
	Status[Loc.Col].CoreTile[Index].CoreStatus = RegVal;

	/* program counter */
	RC = XAie_CoreGetPCValue(DevInst, Loc, &RegVal);
	if (RC != XAIE_OK) {
		return RC;
	}
	Status[Loc.Col].CoreTile[Index].ProgramCounter = RegVal;

	/* stack pointer */
	RC = XAie_CoreGetSPValue(DevInst, Loc, &RegVal);
	if (RC != XAIE_OK) {
		return RC;
	}
	Status[Loc.Col].CoreTile[Index].StackPtr = RegVal;

	/* link register */
	RC = XAie_CoreGetLRValue(DevInst, Loc, &RegVal);
	if (RC != XAIE_OK) {
		return RC;
	}
	Status[Loc.Col].CoreTile[Index].LinkReg = RegVal;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the All Tile DMA Channel status for a particular column
* and row.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Loc: Location of AIE tile
*
* @return	XAIE_OK for success and error code otherwise.
*
* @note	Internal only.
*
******************************************************************************/
static AieRC _XAie_DmaStatusDump(XAie_DevInst *DevInst,
		XAie_ColStatus *Status, XAie_LocType Loc)
{
	u32 RegVal;
	u8 TileType, AieTileStart, MemTileStart;
	u8 Index;
	AieRC RC;

	const XAie_DmaMod *DmaMod;
	XAie_CoreTileStatus *CoreTile;
	XAie_ShimTileStatus *ShimTile;
	XAie_MemTileStatus *MemTile;

	AieTileStart = DevInst->AieTileRowStart;
	MemTileStart = DevInst->MemTileRowStart;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if((TileType >= XAIEGBL_TILE_TYPE_MAX) ||
			(TileType == XAIEGBL_TILE_TYPE_SHIMPL)) {
		/* DMA status not applicable for this tile type, skip */
		return XAIE_OK;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(DmaMod == NULL) {
		/* DMA module not present for this tile type */
		return XAIE_OK;
	}

	CoreTile = Status[Loc.Col].CoreTile;
	MemTile = Status[Loc.Col].MemTile;
	ShimTile = Status[Loc.Col].ShimTile;

	if(!_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
		/* iterate all tile dma channels */
		for (u8 Chan = 0; Chan < DmaMod->NumChannels; Chan++) {

			/* read s2mm channel status */
			RC = XAie_DmaGetChannelStatus(DevInst, Loc, Chan,
					DMA_S2MM, &RegVal);
			if (RC != XAIE_OK) {
				return RC;
			}

			if(TileType == XAIEGBL_TILE_TYPE_AIETILE) {
				Index = Loc.Row - AieTileStart;
				CoreTile[Index].Dma[Chan].S2MMStatus = RegVal;
			} else if(TileType == XAIEGBL_TILE_TYPE_MEMTILE) {
				Index = Loc.Row - MemTileStart;
				MemTile[Index].Dma[Chan].S2MMStatus = RegVal;
			} else {
				ShimTile[Loc.Row].Dma[Chan].S2MMStatus = RegVal;
			}

			/* read mm2s channel status */
			RC = XAie_DmaGetChannelStatus(DevInst, Loc, Chan,
					DMA_MM2S, &RegVal);
			if (RC != XAIE_OK) {
				return RC;
			}

			if(TileType == XAIEGBL_TILE_TYPE_AIETILE) {
				Index = Loc.Row - AieTileStart;
				CoreTile[Index].Dma[Chan].MM2SStatus = RegVal;
			} else if(TileType == XAIEGBL_TILE_TYPE_MEMTILE) {
				Index = Loc.Row - MemTileStart;
				MemTile[Index].Dma[Chan].MM2SStatus = RegVal;
			} else {
				ShimTile[Loc.Row].Dma[Chan].MM2SStatus = RegVal;
			}
		}
	} else {
		/* iterate all s2mm dma channels */
		for (u8 Chan = 0; Chan < DmaMod->NumChannels; Chan++) {

			/* read s2mm channel status */
			RC = XAie_DmaGetChannelStatus(DevInst, Loc, Chan,
					DMA_S2MM, &RegVal);
			if (RC != XAIE_OK) {
				return RC;
			}

			if(TileType == XAIEGBL_TILE_TYPE_AIETILE) {
				Index = Loc.Row - AieTileStart;
				CoreTile[Index].Dma[Chan].S2MMStatus = RegVal;
			} else if(TileType == XAIEGBL_TILE_TYPE_MEMTILE) {
				Index = Loc.Row - MemTileStart;
				MemTile[Index].Dma[Chan].S2MMStatus = RegVal;
			} else {
				ShimTile[Loc.Row].Dma[Chan].S2MMStatus = RegVal;
			}			
		}
		/* iterate all mm2s dma channels */
		for (u8 Chan = 0; Chan < DmaMod->NumMm2sChannels; Chan++) {			

			/* read mm2s channel status */
			RC = XAie_DmaGetChannelStatus(DevInst, Loc, Chan,
					DMA_MM2S, &RegVal);
			if (RC != XAIE_OK) {
				return RC;
			}

			if(TileType == XAIEGBL_TILE_TYPE_AIETILE) {
				Index = Loc.Row - AieTileStart;
				CoreTile[Index].Dma[Chan].MM2SStatus = RegVal;
			} else if(TileType == XAIEGBL_TILE_TYPE_MEMTILE) {
				Index = Loc.Row - MemTileStart;
				MemTile[Index].Dma[Chan].MM2SStatus = RegVal;
			} else {
				ShimTile[Loc.Row].Dma[Chan].MM2SStatus = RegVal;
			}
		}
	}	
	
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the Aie Lock status for a particular column and row.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Loc: Location of AIE tile
*
* @return	XAIE_OK for success and error code otherwise.
*
* @note	Internal only.
*
******************************************************************************/
static AieRC _XAie_LockValueStatusDump(XAie_DevInst *DevInst,
		XAie_ColStatus *Status, XAie_LocType Loc)
{
	u32 RegVal;
	u8 TileType, AieTileStart, MemTileStart;
	u8 Index;
	AieRC RC;
	XAie_Lock Lock = {0,0};

	const XAie_LockMod *LockMod;
	XAie_CoreTileStatus *CoreTile;
	XAie_ShimTileStatus *ShimTile;
	XAie_MemTileStatus *MemTile;

	AieTileStart = DevInst->AieTileRowStart;
	MemTileStart = DevInst->MemTileRowStart;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if((TileType >= XAIEGBL_TILE_TYPE_MAX) ||
			(TileType == XAIEGBL_TILE_TYPE_SHIMPL)) {
		/* Lock status not applicable for this tile type, skip */
		return XAIE_OK;
	}

	LockMod = DevInst->DevProp.DevMod[TileType].LockMod;
	if(LockMod == NULL) {
		/* Lock module not present for this tile type */
		return XAIE_OK;
	}

	CoreTile = Status[Loc.Col].CoreTile;
	MemTile = Status[Loc.Col].MemTile;
	ShimTile = Status[Loc.Col].ShimTile;

	/* iterate all lock value registers */
	for(u32 LockCnt = 0; LockCnt < LockMod->NumLocks; LockCnt++) {

		/* read lock value */
		Lock.LockId = (u8)LockCnt;
		RC = XAie_LockGetValue(DevInst, Loc, Lock, &RegVal);
		if (RC != XAIE_OK) {
			return RC;
		}

		if(RegVal > UINT8_MAX){
			XAIE_ERROR("Invalid Lock value\n");
			return XAIE_ERR;
		}

		if(TileType == XAIEGBL_TILE_TYPE_AIETILE) {
			Index = Loc.Row - AieTileStart;
			CoreTile[Index].LockValue[LockCnt] = (u8)RegVal;
		} else if(TileType == XAIEGBL_TILE_TYPE_MEMTILE) {
			Index = Loc.Row - MemTileStart;
			MemTile[Index].LockValue[LockCnt] = (u8)RegVal;
		} else {
			ShimTile[Loc.Row].LockValue[LockCnt] = (u8)RegVal;
		}
	}
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the Tile Event Status for a particular column and row.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Loc: Location of AIE tile
*
* @return	XAIE_OK for success and error code otherwise.
*
* @note	Internal only.
*
******************************************************************************/
static AieRC _XAie_EventStatusDump(XAie_DevInst *DevInst,
		XAie_ColStatus *Status, XAie_LocType Loc)
{
	u32 RegVal;
	AieRC RC;
	u8 TileType, AieTileStart, MemTileStart;
	u8 Index;
	u8 NumEventReg;

	const XAie_EvntMod *EvntCoreMod, *EvntMod;
	XAie_CoreTileStatus *CoreTile;
	XAie_ShimTileStatus *ShimTile;
	XAie_MemTileStatus *MemTile;
	const XAie_TileMod *DevMod;

	AieTileStart = DevInst->AieTileRowStart;
	MemTileStart = DevInst->MemTileRowStart;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType >= XAIEGBL_TILE_TYPE_MAX) {
		/* Invalid tile type, skip */
		return XAIE_OK;
	}

	CoreTile = Status[Loc.Col].CoreTile;
	MemTile = Status[Loc.Col].MemTile;
	ShimTile = Status[Loc.Col].ShimTile;
	DevMod = DevInst->DevProp.DevMod;

	if(TileType == XAIEGBL_TILE_TYPE_AIETILE) {
		EvntCoreMod = &DevMod[TileType].EvntMod[XAIE_CORE_MOD];
		Index = Loc.Row - AieTileStart;
		/* iterate all event status registers */
		NumEventReg = EvntCoreMod->NumEventReg;
		for(u32 EventReg = 0; EventReg < NumEventReg; EventReg++) {
			/* read event status register and store in output
			 * buffer */
			RC = XAie_EventRegStatus(DevInst, Loc, XAIE_CORE_MOD,
					(u8)EventReg, &RegVal);
			if (RC != XAIE_OK) {
				return RC;
			}

			CoreTile[Index].EventCoreModStatus[EventReg] = RegVal;

			/* For AIE4 devices, skip memory module event status for core tiles */
			if(!_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
				/* read event status register for memory module */
				RC = XAie_EventRegStatus(DevInst, Loc, XAIE_MEM_MOD,
						(u8)EventReg, &RegVal);
				if (RC != XAIE_OK) {
					return RC;
				}
				CoreTile[Index].EventMemModStatus[EventReg] = RegVal;
			}
		}
	} else if(TileType == XAIEGBL_TILE_TYPE_MEMTILE) {
		EvntMod = &DevMod[TileType].EvntMod[XAIE_MEM_MOD];
		Index = Loc.Row - MemTileStart;
		NumEventReg = EvntMod->NumEventReg;
		for(u32 EventReg = 0; EventReg < NumEventReg; EventReg++) {
			RC = XAie_EventRegStatus(DevInst, Loc, XAIE_MEM_MOD,
					(u8)EventReg, &RegVal);
			if (RC != XAIE_OK) {
				return RC;
			}
			MemTile[Index].EventStatus[EventReg] = RegVal;
		}
	} else {
		EvntMod = &DevMod[TileType].EvntMod[0U];
		NumEventReg = EvntMod->NumEventReg;
		for(u32 EventReg = 0; EventReg < NumEventReg; EventReg++) {
			RC = XAie_EventRegStatus(DevInst, Loc, XAIE_PL_MOD,
					(u8)EventReg, &RegVal);
			if (RC != XAIE_OK) {
				return RC;
			}
			ShimTile[Loc.Row].EventStatus[EventReg] = RegVal;
		}
	}
	return XAIE_OK;
}


/*****************************************************************************/
/**
*
* This API returns the column status for N number of colums.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
*
* @return	XAIE_OK for success and error code otherwise.
*
* @note	None.
*
******************************************************************************/
AieRC XAie_StatusDump(XAie_DevInst *DevInst, XAie_ColStatus *Status)
{
	AieRC RC = XAIE_OK;
	u8 StartCol = DevInst->StartCol;
	u8 NumCols  = DevInst->NumCols;
	u8 NumRows  = DevInst->NumRows;
	XAie_LocType Loc;

	if(Status == NULL) {
		return XAIE_ERR;
	}

	/* iterate specified columns */
	for(u32 Col = StartCol; Col < ((u32)StartCol + NumCols); Col++) {
		for(u32 Row = 0; Row < NumRows; Row++) {
			Loc.Row = (u8)Row;
			Loc.Col = (u8)Col;
			RC |= (u32)_XAie_CoreStatusDump(DevInst, Status, Loc);
			RC |= (u32)_XAie_DmaStatusDump(DevInst, Status, Loc);
			RC |= (u32)_XAie_LockValueStatusDump(DevInst, Status, Loc);
			RC |= (u32)_XAie_EventStatusDump(DevInst, Status, Loc);
		}
	}
	return (AieRC)RC;
}

/*****************************************************************************/
/**
*
* This API provides information whether a particular device is AIE4 device or not
*
* @param	DevGen: device generation value.
*
* @return	true : if the device is AIE4 device
* 			false : if the device is legacy device.
*
* @note
*
*******************************************************************************/
u8 _XAie_IsDeviceGenAIE4(u8 DevGen)
{
	switch(DevGen) {
	case XAIE_DEV_GEN_AIE4_GENERIC:
	case XAIE_DEV_GEN_AIE4:
	case XAIE_DEV_GEN_AIE4_A:
		return true;
	default:
		return false;
	}
}

/*****************************************************************************/
/**
*
* This API provides information whether a particular device support dual application
* mode at column level or not?
*
* @param	DevGen: device generation value.
*
* @return	true : if the device supports dual application mode
* 			false : if the device doesn't support dual application mode
*
* @note
*
*******************************************************************************/
u8 _XAie_IsDeviceGenSupportDualApp(u8 DevGen)
{
	if(_XAie_IsDeviceGenAIE4(DevGen)) {
		switch(DevGen){
		case XAIE_DEV_GEN_AIE4_GENERIC:
		case XAIE_DEV_GEN_AIE4:
		case XAIE_DEV_GEN_AIE4_A:
			return true;
		default:
			return false;
		}
	} else {
		return false;
	}
}

/*****************************************************************************/
/**
*
* This API provides information whether a particular tile resources are present in dual address space
*
* @param	DevInst: device generation value.
* @param	TileType: Type of the Tile
*
* @return	true : if the tile resources are present in dual address space
* 			false : Otherwise
*
* @note		This api doesn't check any input parameter for the invalid values.
* 			It is assumed that all parameters are valid for this.
*
*******************************************************************************/
u8 _XAie_IsTileResourceInSharedAddrSpace(u8 DevGen, u8 AppMode, u8 TileType)
{
	if ((_XAie_IsDeviceGenAIE4(DevGen) &&
				AppMode == XAIE_DEVICE_SINGLE_APP_MODE)) {
		switch(TileType) {
			case XAIEGBL_TILE_TYPE_AIETILE:
				return false;

			case XAIEGBL_TILE_TYPE_SHIMNOC:
			case XAIEGBL_TILE_TYPE_MEMTILE:
				return true;

			default:
				return false;
		}
	} else {
		return false;
	}
}

/*****************************************************************************/
/**
*
* This API provides the maximum number of elements a particular device supports
* for given tile type and module type
*
* @param	DevGen: device generation value.
* @param	TileType: Type of tile aiecore tile, memtile or shimnoc tile
* @param	elementValue: resource value for corresponding tile
* @param	AppMode: device appmode value.

* @return	None.
*
* @note		This api doesn't check any input parameter for the invalid values.
* 			It is asssumed that all parameters are valid for this function
*
*******************************************************************************/
u16 _XAie_GetMaxElementValue(u8 DevGen, u8 TileType, u8 AppMode, u16 elementValue)
{
	if(elementValue > UINT16_MAX/2 ) {
		XAIE_ERROR("Invalid elementValue \n");
		return XAIE_ERR;
	}
	if(_XAie_IsTileResourceInSharedAddrSpace(DevGen, AppMode, TileType))
		return elementValue * 2;
	else
		return elementValue;
}

/*****************************************************************************/
/**
*
* This API provides the maximum number of elements a particular device supports
* for given tile type and module type
*
* @param	DevGen: device generation value.
* @param	TileType: Type of tile aiecore tile, memtile or shimnoc tile
* @param	elementValue: resource value for corresponding tile
* @param	AppMode: device appmode value.

* @return	None.
*
* @note		This api doesn't check any input parameter for the invalid values.
* 			It is asssumed that all parameters are valid for this function
*
*******************************************************************************/
u16 XAie_GetMaxElementValue(u8 DevGen, u8 TileType, u8 AppMode, u16 elementValue)
{
	return _XAie_GetMaxElementValue(DevGen, TileType, AppMode, elementValue);
}

/*****************************************************************************/
/**
*
* This API provides the mask value of the register space for App B space for
* AIE4+ architecture
* This api returns zero, if the device generation doesn't support
* dual address space
*
* @param	devGen t: device generation/name
*
* @return	Mask value required to modify the register offset.
*
* @note
*
*******************************************************************************/
static inline u32 XAie_Mask_Value(u8 devGen)
{
	switch(devGen) {
	case XAIE_DEV_GEN_AIE4_GENERIC:
	case XAIE_DEV_GEN_AIE4:
	case XAIE_DEV_GEN_AIE4_A:
		return XAIE4_MASK_VALUE_APP_B;

	default:
		return 0;
	}
}

/*****************************************************************************/
/**
*
* This API provides the register offset in App B space for AIE4+ architecture
* This api returns the same offset, if the device generation doesn't support
* dual adress space
*
* @param	devGen t: device generation/name
* @param	regOffset: Type of tile aiecore tile, memtile or shimnoc tile
*
* @return	regOffset: Modified or same register offset
*
* @note
*
*******************************************************************************/
u64 _XAie_ChangeRegisterSpace(u8 devGen, u64 regOffset)
{
	return (regOffset | XAie_Mask_Value(devGen));
}

/*****************************************************************************/
/**
* This function is used to check for invalid configuration of 
* module and tiletype combination for AIE4 devices before calling eventmodule .
*
* @param        DevInst: Device Instance
* @param        Loc: Location of the AIE tile.
* @param	Module:	XAIE_MEM_MOD - memory module
* 			XAIE_CORE_MOD - core module
* 			XAIE_PL_MOD - pl module
* @return       XAIE_OK for correct combination of Module and tile type
* 		XAIE_INVALID_ARGS for incorrect combination of module and tile
* 		type
*
* @note         Internal API only.
*
*******************************************************************************/
u8 XAie_IsTileTypeAndModuleSupportForEvents(XAie_DevInst* DevInst,
	XAie_LocType Loc, XAie_ModuleType Module)
{
	u8 TileType;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen) &&
		 ((TileType == XAIEGBL_TILE_TYPE_AIETILE) && (Module == XAIE_MEM_MOD)))
		 return false;

	return true;
}

/*****************************************************************************/
/**
* This function is used to check whether device supports L1 interrupt or not .
*
* @param    DevGen: Device generation
* @return   true:  device supports L1 interrupt
*	 		false: device doesnt supports L1 interrupt 
*
*******************************************************************************/
u8 XAie_IsDeviceSupportsL1Interrupt(u8 DevGen) {
	if (_XAie_IsDeviceGenAIE4(DevGen))
		return false;
	else
		return true;
}

/*****************************************************************************/
/**
* This function is used to check whether device supports AIE4 Features or not .
*
* @param    DevGen: Device generation
* @param    Feature:  AIE4 Features
* @return   true:  device supports AIE4 features
*			false: device doesnt supports AIE4 features
*
*******************************************************************************/
u8 XAie_IsFeatureSupportCheck(u8 DevGen, u8 Feature)
{
	if (_XAie_IsDeviceGenAIE4(DevGen)){
		switch(Feature) {
			case NO_L1_INTERRUPT_SUPPORT:
			case PERFORMANCE_SNAPSHOT_SUPPORT:
			case BITMAP64_GROUPEVENT_SUPPORT:
			case NO_MEM_MOD_IN_AIE_TILE:
				return true;
			default:
				return false;
		}
	}
	else
		return false;
}

/*****************************************************************************/
/**
* This function is used to get the number of combo events.
*
* @param    DevInst: Device Instance
* @param    TileType:  AIE tile type
* @param	Module:	module type
* @return   u8 value: number of combo events supported
*
*******************************************************************************/
u8 XAie_GetComboEventsNumber(XAie_DevInst* DevInst, u8 TileType, XAie_ModuleType Module) {
	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
		if((TileType == XAIEGBL_TILE_TYPE_AIETILE) && (Module == XAIE_MEM_MOD))
			return 0;
		else		
			return XAIE4_COMBO_PER_MOD;
	}
	else
		return XAIE_COMBO_PER_MOD;
}

/*****************************************************************************/
/**
 *
 * Calculates the Tile Address from Row, Col of the AIE array/partition
 *
 * @param	DevInst: Device Instance
 * @param	R: Row
 * @param	C: Column
 * @return	TileAddr
 *
 * @note		Internal API only.
 *
 ******************************************************************************/
u64 XAie_GetTileAddr(XAie_DevInst *DevInst, u8 R, u8 C)
{
	XAie_LocType Loc = { R, C };
	if ((DevInst->DevProp.RowShift > 55) || (DevInst->DevProp.ColShift > 55)){
		XAIE_ERROR("Invalid shift value pair\n");
		return XAIE_ERR;
	}
	u8 TileType = XAie_GetTileTypefromLoc(DevInst,Loc);
	if (_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) {
		switch(TileType) {
		case XAIEGBL_TILE_TYPE_SHIMNOC:
		case XAIEGBL_TILE_TYPE_SHIMPL:	
			/* TBD - Implement Shim Row South and North Handling */
			break;
		case XAIEGBL_TILE_TYPE_AIETILE:
			R += ((DevInst->MemTileNumRows * XAIE_DEV_GEN_AIE4_AIE_TILE_SHIFT_OFFSET) & 0xFFU);
			break;
		case XAIEGBL_TILE_TYPE_MEMTILE:
			if(R != DevInst->MemTileRowStart) {
				R = (R * XAIE_DEV_GEN_AIE4_MEM_TILE_SHIFT_OFFSET) - XAIE_DEV_GEN_AIE4_AIE_TILE_SHIFT_OFFSET;
			}
			break;
		default:
			XAIE_ERROR("Invalid Tiletype\n");
			return 0;
		}
	}
	return (((u64)(R & 0xFFU)) << DevInst->DevProp.RowShift) |
			(((u64)(C & 0xFFU)) << DevInst->DevProp.ColShift);
}

/*****************************************************************************/
/**
 *
 * Calculates the Tile Address from Row, Col of the AIE array/partition
 *
 * @param	DevInst: Device Instance
 * @param	R: Row
 * @param	C: Column
 * @return	TileAddr
 *
 * @note		Internal API only.
 *
 ******************************************************************************/
u64 _XAie_GetTileAddr(XAie_DevInst *DevInst, u8 R, u8 C)
{
	return XAie_GetTileAddr(DevInst, R, C);
}


/*****************************************************************************/
/**
*
* This routine is used to check if the shim tile has uc module.
*
* @param	DevInst: Device Instance.
* @param	TileType: Type of the tile.
*
* @return	1 if uc module is present and 0 otherwise.
*
* @note		Internal API only.
*
*******************************************************************************/
u8 XAie_IsUcModulePresent(XAie_DevInst* DevInst, u8 TileType) {
	if ((_XAie_IsDeviceGenAIE4(DevInst->DevProp.DevGen)) &&
			TileType == XAIEGBL_TILE_TYPE_SHIMNOC) {
		return 1;
	}
	return 0;
}

/*****************************************************************************/
/**
*
* This routine is used to check if given lsb & value pair exceeds precision.
*
* @param	Lsb				: Shift Value
* @param    ValueBitCount	: Value to be shifted
* @param    MaxValidBitPos	: Max desired precision post shift
*
* @return	0 - if precision intact else precision exceeds
*
* @note		Internal API only.
*
*******************************************************************************/
u8 _XAie_CheckPrecisionExceeds(u32 Lsb, u8 ValueBitCount, u8 MaxValidBitPos)
{
	if ((Lsb + ValueBitCount) > MaxValidBitPos) {
		return 1;
	}
	return 0;
}

/*****************************************************************************/
/**
*
* This routine is used calculate the max bits needed for a given integer number.
*
* @param    Value		: Value
*
* @return	No of bits needed to represent the value.
*
* @note		Internal API only.
*
*******************************************************************************/
u8 _XAie_MaxBitsNeeded(u64 value)
{
    u8 bits = 0;
    while (value) {
        bits++;
        value >>= 1; // Right shift by 1 (equivalent to dividing by 2)
    }
    return bits;
}

/*****************************************************************************/
/**
*
* This routine is used to check if given lsb & Mask pair exceeds precision
* when a masked value is right shift by lsb
*
* @param	Lsb				: Shift Value
* @param    Mask			: Value to be shifted
*
* @return	0 - if precision intact else precision exceeds
*
* @note		Internal API only.
*
*******************************************************************************/
u8 _XAie_CheckPrecisionExceedsForRightShift(u32 Lsb, u32 Mask)
{
	if (Lsb  > _XAie_CountTrailingZeros(Mask)) {
		return 1;
	}
	return 0;
}

/*****************************************************************************/
/**
*
* This routine is used calculate the trailing zeros needed for a given 
* integer number in  binary form
*
* @param    Value		: Value
*
* @return	No of trailing zeros in the given number.
*
* @note		Internal API only.
*
*******************************************************************************/
u8 _XAie_CountTrailingZeros(u32 value)
{
	u8 count = 0;
    while ((value & 1) == 0 && value > 0)  {
		value >>= 1;
        count++; // Right shift by 1 (equivalent to dividing by 2)
    }
    return count;
}

/*****************************************************************************/
/**
*
* This routine is used reset the desired bits of the TilesInUse, MemInUse and
* CoreInUse global bitmaps based on the partition dimensions defined in DevInst
* structure like startcol and numcols.
*
* @param    DevInst		: Device Instance
*
* @return	None
*
* @note		Internal API only.
*		For device generations that don't require bitmap tracking (AIE2P,
*		AIE2IPU), the bitmap pointers are NULL and this function returns
*		early without any operations.
*
*******************************************************************************/
void _XAie_ResetInUseBitMaps(XAie_DevInst *DevInst)
{
	u8 startCol = DevInst->StartCol;
	u8 numCols = DevInst->NumCols;

	/*
	 * Skip if bitmaps are not allocated (NULL indicates bitmap tracking
	 * is disabled for this device generation).
	 */
	if ((DevInst->DevOps->TilesInUse == NULL) ||
	    (DevInst->DevOps->MemInUse == NULL) ||
	    (DevInst->DevOps->CoreInUse == NULL)) {
		return;
	}

	// Calculate the start bit position for each column and reset the bits in
	// the global bitmaps.
	for (int col = startCol; col < (startCol + numCols); col++) {
		XAie_LocType ColStartLoc = { 0, col };
		u32 ColStartBitPos = _XAie_GetTileBitPosFromLoc(DevInst, ColStartLoc);
		_XAie_ClrBitInBitmap(DevInst->DevOps->TilesInUse, ColStartBitPos, \
			DevInst->NumRows);
		_XAie_ClrBitInBitmap(DevInst->DevOps->MemInUse, ColStartBitPos, \
			DevInst->NumRows);
		_XAie_ClrBitInBitmap(DevInst->DevOps->CoreInUse, ColStartBitPos, \
			DevInst->NumRows);
	}
}

/** @} */
