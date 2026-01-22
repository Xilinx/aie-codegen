/******************************************************************************
* Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_locks_aieml.c
* @{
*
* This file contains routines for AIE4 locks.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   03/17/2020  Initial creation
* 1.1   Tejus   03/23/2020  fix check of return value from mask poll for acquire
*			    api
* 1.2   Tejus   06/10/2020  Switch to new io backend.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_locks.h"
#include "xaiegbl_defs.h"

#ifdef XAIE_FEATURE_LOCK_ENABLE
#include "xaie_helper_internal.h"
/************************** Constant Definitions *****************************/
#define XAIE4_LOCK_VALUE_MASK		0x7FU
#define XAIE4_LOCK_VALUE_SHIFT		0x2U

#define XAIE4_LOCK_RESULT_SUCCESS	1U
#define XAIE4_LOCK_RESULT_LSB		0x0U
#define XAIE4_LOCK_RESULT_MASK		0x1U

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API is used to release the specified lock with or without value. This
* API can be blocking or non-blocking based on the TimeOut value. If the
* TimeOut is 0us, the API behaves in a non-blocking fashion and returns
* immediately after the first lock release request. If TimeOut > 0, then the
* API is a blocking call and will issue lock release request until the release
* is successful or it times out, whichever occurs first.
*
* @param	DevInst: Device Instance
* @param	LockMod: Internal lock module data structure.
* @param	Loc: Location of AIE Tile
* @param	Lock: Lock data structure with LockId and LockValue.
* @param	TimeOut: Timeout value for which the release request needs to be
*		repeated. Value in usecs.
*
* @return	XAIE_OK if Lock Release, else XAIE_LOCK_RESULT_FAILED.
*
* @note 	Internal API for AIE4. This API should not be called directly.
*		It is invoked only using the function pointer part of the lock
*		module data structure.
*
******************************************************************************/
AieRC _XAie4_LockRelease(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
		XAie_LocType Loc, XAie_Lock Lock, u32 TimeOut, u8 BusyPoll)
{
	u64 RegAddr;
	u64 RegOff = 0;
	AieRC Status = XAIE_OK;
	u8 LockVal;

	if((DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) && (Lock.LockId >= LockMod->NumLocks)) {
		RegOff = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, RegOff);
		Lock.LockId -= LockMod->NumLocks;
	}
	if(Lock.LockVal < 0)
		LockVal = (u8)(Lock.LockVal);
	else
		LockVal = Lock.LockVal;

	RegOff |= LockMod->BaseAddr + (Lock.LockId * (u64)LockMod->LockIdOff) +
		((LockVal & XAIE4_LOCK_VALUE_MASK) <<
		 XAIE4_LOCK_VALUE_SHIFT);


	RegAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOff;

	if (BusyPoll != XAIE_ENABLE) {
		Status = XAie_MaskPoll(DevInst, RegAddr, XAIE4_LOCK_RESULT_MASK,
					(XAIE4_LOCK_RESULT_SUCCESS <<
					 XAIE4_LOCK_RESULT_LSB), TimeOut);
		if (Status != XAIE_OK) {
			XAIE_DBG("Wait for lock release MaskPoll timed out : %d\n", TimeOut);
			return XAIE_LOCK_RESULT_FAILED;
		}
	} else {
		Status = XAie_MaskPollBusy(DevInst, RegAddr, XAIE4_LOCK_RESULT_MASK,
					(XAIE4_LOCK_RESULT_SUCCESS <<
					 XAIE4_LOCK_RESULT_LSB), TimeOut);
		if (Status != XAIE_OK) {
			XAIE_DBG("Wait for lock release MaskPollBusy timed out : %d\n", TimeOut);
			return XAIE_LOCK_RESULT_FAILED;
		}
	}	

	return Status;
}

/*****************************************************************************/
/**
*
* This API is used to acquire the specified lock and value. This API can be
* blocking or non-blocking based on the TimeOut value. If the TimeOut is 0us,
* the API behaves in a non-blocking fashion and returns immediately after the
* first lock acquire request. If TimeOut > 0, then the API is a blocking call
* and will issue lock acquire request until the acquire is successful or it
* times out, whichever occurs first.
*
* @param	DevInst: Device Instance
* @param	LockMod: Internal lock module data structure.
* @param	Loc: Location of AIE Tile
* @param	Lock: Lock data structure with LockId and LockValue.
* @param	TimeOut: Timeout value for which the acquire request needs to be
*		repeated. Value in usecs.
*
* @return	XAIE_OK if Lock Acquired, else XAIE_LOCK_RESULT_FAILED.
*
* @note 	Internal API for AIE4. This API should not be called directly.
*		It is invoked only using the function pointer part of the lock
*		module data structure.
*
******************************************************************************/
AieRC _XAie4_LockAcquire(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
		XAie_LocType Loc, XAie_Lock Lock, u32 TimeOut, u8 BusyPoll)
{
	u64 RegAddr;
	u64 RegOff = 0;
	AieRC Status = XAIE_OK;
	u8 LockVal;

	if((DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) && (Lock.LockId >= LockMod->NumLocks)) {
		RegOff = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, RegOff);
		Lock.LockId -= LockMod->NumLocks;
	}
	if(Lock.LockVal < 0)
		LockVal = (u8)(Lock.LockVal);
	else
		LockVal = Lock.LockVal;
	RegOff |= LockMod->BaseAddr + (Lock.LockId * (u64)LockMod->LockIdOff) +
		(LockMod->RelAcqOff) + ((LockVal &
					XAIE4_LOCK_VALUE_MASK) <<
				XAIE4_LOCK_VALUE_SHIFT);

	RegAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOff;

	if (BusyPoll != XAIE_ENABLE) {
		Status = XAie_MaskPoll(DevInst, RegAddr, XAIE4_LOCK_RESULT_MASK,
					(XAIE4_LOCK_RESULT_SUCCESS <<
					 XAIE4_LOCK_RESULT_LSB), TimeOut);
		if (Status != XAIE_OK) {
			XAIE_DBG("Wait for lock acquire MaskPoll timed out : %d\n", TimeOut);
			return XAIE_LOCK_RESULT_FAILED;
		}
	} else {
		Status = XAie_MaskPollBusy(DevInst, RegAddr, XAIE4_LOCK_RESULT_MASK,
					(XAIE4_LOCK_RESULT_SUCCESS <<
					 XAIE4_LOCK_RESULT_LSB), TimeOut);
		if (Status != XAIE_OK) {
			XAIE_DBG("Wait for lock acquire MaskPollBusy timed out : %d\n", TimeOut);
			return XAIE_LOCK_RESULT_FAILED;
		}
	}	

	return Status;
}

/*****************************************************************************/
/**
*
* This API is used to initalize a lock with given value.
*
* @param	DevInst: Device Instance
* @param	LockMod: Internal lock module data structure.
* @param	Loc: Location of AIE Tile
* @param	Lock: Lock data structure with LockId and LockValue.
*
* @return	XAIE_OK if Lock Release, else error code.
*
* @note 	Internal only.
*
******************************************************************************/
AieRC _XAie4_LockSetValue(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
		XAie_LocType Loc, XAie_Lock Lock)
{
	u64 RegAddr;
	u32 RegVal;
	u64 RegOff;

        if(Lock.LockVal < 0) {
                XAIE_ERROR("Lock Value needs to be positive\n");
                return XAIE_ERR;
        }

	RegOff = LockMod->LockSetValBase;

	if((DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) && (Lock.LockId >= LockMod->NumLocks)) {
		RegOff = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, RegOff);
		Lock.LockId -= LockMod->NumLocks;
	}

	RegAddr = (u64)(RegOff +
		LockMod->LockSetValOff * (u64)Lock.LockId) +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

        if ((_XAie_CheckPrecisionExceeds(LockMod->LockInit->Lsb,
                        _XAie_MaxBitsNeeded(Lock.LockVal),MAX_VALID_AIE_REG_BIT_INDEX))) {
                XAIE_ERROR("Check Precision Exceeds Failed\n");
                return XAIE_ERR;
        }

	RegVal = XAie_SetField(Lock.LockVal, LockMod->LockInit->Lsb,
			LockMod->LockInit->Mask);

	return XAie_Write32(DevInst, RegAddr, RegVal);
}

/*****************************************************************************/
/**
*
* This API is used to get Lock Status Value.
*
* @param	DevInst: Device Instance
* @param	LockMod: Internal lock module data structure.
* @param	Loc: Location of AIE Tile
* @param	Lock: Lock data structure with LockId and LockValue.
* @param	LockVal: Lock Value.
*
* @return	XAIE_OK if Lock Release, else error code.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAie4_LockGetValue(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
		XAie_LocType Loc, XAie_Lock Lock, u32 *LockVal)
{
	u64 RegAddr;
	u64 RegOff;
	AieRC RC;

	RegOff = LockMod->LockSetValBase;

	if((DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE) && (Lock.LockId >= LockMod->NumLocks)) {
		RegOff = _XAie_ChangeRegisterSpace(DevInst->DevProp.DevGen, RegOff);
		Lock.LockId -= LockMod->NumLocks;
	}

	RegAddr = Lock.LockId * LockMod->LockSetValOff +
		RegOff +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	RC = XAie_Read32(DevInst, RegAddr, LockVal);
	if (RC != XAIE_OK) {
		return RC;
	}

	return XAIE_OK;
}
#endif /* XAIE_FEATURE_LOCK_ENABLE */
/** @} */
