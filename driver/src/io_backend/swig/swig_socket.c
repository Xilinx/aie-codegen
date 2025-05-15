/******************************************************************************
* Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file swig_socket.c
* @{
*
* This file contains swig interface APIs for socket backend of AIE drivers.
*
******************************************************************************/

#ifdef __SWIGINTERFACE__
#include "swig_socket_interface.h"
#endif

#ifdef __SWIGINTERFACE__
/*****************************************************************************/
/**
*
* This is the public interface memory IO function to initialize the global IO instance
*
* @param	None.
*
* @return	None.
*
* @note		The global IO instance is a singleton and any further attempt
* to initialize just increments the reference count. Internal only.
*
*******************************************************************************/
int XAie_SocketIO_swig_Init(Swig_DevInst *DevInst) {
	AieRC RC;
	RC = XAie_SocketIO_Init((XAie_DevInst *)DevInst);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface memory IO function to write 32bit data to the specified address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Value: 32-bit data to be written.
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
int XAie_SocketIO_swig_Write32(Swig_SocketIO IOInst, u64 RegOff, u32 Value) {
	AieRC RC;
	void *IOInst_swig = &IOInst;
	RC = XAie_SocketIO_Write32(IOInst_swig,RegOff,Value);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface memory IO function to read 32bit data from the specified address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: u32 Data variable
*
* @return	Data read from the register
*
* @note		Internal only.
*
*******************************************************************************/
int XAie_SocketIO_swig_Read32(Swig_SocketIO IOInst, u64 RegOff, u32 Data) {
	AieRC RC;
	void *IOInst_swig = &IOInst;
	RC = XAie_SocketIO_Read32(IOInst_swig,RegOff,&Data);
	(void)RC;
	return Data;
}

/*****************************************************************************/
/**
*
* This is the public interface memory IO function to free the global IO instance
*
* @param	IOInst: IO Instance pointer.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The global IO instance is a singleton and freed when
* the reference count reaches a zero. Internal only.
*
*******************************************************************************/
int XAie_SocketIO_swig_Finish(Swig_SocketIO IOInst) {
	AieRC RC;
	void *IOInst_swig = &IOInst;
	RC = XAie_SocketIO_Finish(IOInst_swig);
	return RC;
}

#endif // __SWIGINTERFACE__
