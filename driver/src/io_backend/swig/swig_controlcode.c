/******************************************************************************
* Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file swig_controlcode.c
* @{
*
* This file contains swig interface APIs for controlcode backend of AIE drivers.
*
******************************************************************************/

#ifdef __SWIGINTERFACE__
#include "swig_controlcode_interface.h"
#endif

#ifdef __SWIGINTERFACE__
/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to initialize the
* global IO instance
*
* @param	DevInst: Dev Instance Pointer.
*
* @return	AIE-RT API execution status
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_Init(Swig_DevInst *DevInst) {
	AieRC RC;
	// Set Backend in DevInst to control code
	DevInst->Backend = &ControlCodeBackend;
	RC = XAie_ControlCodeIO_Init((XAie_DevInst *)DevInst);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to config the Mode
* Select
*
* @param	DevInst: Dev Instance Pointer.
* @param	Mode: Mode value to be configured.
*
* @return	AIE-RT API execution status
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_ConfigMode(Swig_DevInst *DevInst, XAie_ModeSelect Mode){
	AieRC RC;
	void *IOInst_swig = DevInst->IOInst;
	RC = XAie_ConfigMode(IOInst_swig, Mode);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to get the current
* Config Mode Selection
*
* @param	DevInst: Dev Instance Pointer.
*
* @return	Current configured config mode.
*
*******************************************************************************/
XAie_ModeSelect XAie_ControlCodeIO_swig_GetConfigMode(Swig_DevInst *DevInst){
	void *IOInst_swig = DevInst->IOInst;
	XAie_ModeSelect Mode = XAie_GetConfigMode(IOInst_swig);
	return Mode;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to write 32bit
* data to the specified address.
*
* @param	DevInst: Dev instance pointer
* @param	RegOff: Register offset to read from.
* @param	Value: 32-bit data to be written.
*
* @return	AIE-RT API execution status
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_Write32(Swig_DevInst *DevInst, u64 RegOff,
	u32 Value)
{
	AieRC RC;
	void *IOInst_swig = DevInst->IOInst;
	RC = XAie_ControlCodeIO_Write32(IOInst_swig,RegOff,Value);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to read 32bit
* data from the specified address.
*
* @param	DevInst: Dev instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: u32 Data variable
*
* @return	AIE-RT API execution status
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_Read32(Swig_DevInst *DevInst, u64 RegOff,
	u32 *Data)
{
	AieRC RC;
	void *IOInst_swig = DevInst->IOInst;
	RC = XAie_ControlCodeIO_Read32(IOInst_swig,RegOff,Data);
    return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to write 32bit
* data to the specified address.
*
* @param	DevInst: Dev instance pointer
* @param	RegOff: Register offset to read from.
* @param	Mask: Mask value to be used for reg write.
* @param	Value: 32-bit data to be written.
*
* @return	AIE-RT API execution status
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_MaskWrite32(Swig_DevInst *DevInst, u64 RegOff,
	u32 Mask, u32 Value)
{
	AieRC RC;
	void *IOInst_swig = DevInst->IOInst;
	RC = XAie_ControlCodeIO_MaskWrite32(IOInst_swig,RegOff,Mask,Value);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to poll on
* data to the specified address.
*
* @param	DevInst: Dev instance pointer
* @param	RegOff: Register offset to read from.
* @param	Mask: Mask value to be used for reg write.
* @param	Value: 32-bit data to be written.
* @param	TimeOutUs: Timeout in micro seconds.
*
* @return	AIE-RT API execution status
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_MaskPoll(Swig_DevInst *DevInst, u64 RegOff,
	u32 Mask, u32 Value, u32 TimeOutUs)
{
	AieRC RC;
	void *IOInst_swig = DevInst->IOInst;
	RC = XAie_ControlCodeIO_MaskPoll(IOInst_swig,RegOff,Mask,Value,TimeOutUs);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to write multi
* word data to the specified address.
*
* @param	DevInst: Dev instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Pointer to the multi word data to be written.
* @param	Size: No of words to be written.
*
* @return	AIE-RT API execution status
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_BlockWrite32(Swig_DevInst *DevInst, u64 RegOff,
	u32 *Data, u32 DataSize)
{
	AieRC RC;
	void *IOInst_swig = DevInst->IOInst;
	RC = XAie_ControlCodeIO_BlockWrite32(IOInst_swig,RegOff,Data,DataSize);
	/**
	 * ReSet the IsShimBd flag of ControlCodeIO inst structure to :
	 * Use Block Write backend API to write SHIM BD when using Swig Interface.
	 * 
	 * Note: In ideal case each address patch command will be followed by SHIM
	 *       BD write only. In case of SHIM BD chaining Multiple address patch
	 *       commands will be followed by one WRITE DESC command that chains
	 *       multiple SHIM BDs as UCBDs.
	 * 
	 * Current logic doesnot handle SHIM BD chaining. Will be handled later.
	 */
	((Swig_ControlCodeIO *)IOInst_swig)->IsShimBd = 0;
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to set multi
* word starting from the specified address to a desired value.
*
* @param	DevInst: Dev instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: The data to be written.
* @param	Size: No of words to be written.
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_BlockSet32(Swig_DevInst *DevInst, u64 RegOff,
	u32 Data, u32 DataSize)
{
	AieRC RC;
	void *IOInst_swig = DevInst->IOInst;
	RC = XAie_ControlCodeIO_BlockSet32(IOInst_swig,RegOff,Data,DataSize);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to patch the host
* buffer address in SHIM DMA BD(s) based on argument index.
*
* @param	DevInst: Dev instance pointer
* @param	Arg_Index: XRT kernel argument index to be used for patching
*			host buffer address used in SHIM DMA BD.
* @param	Num_BDs: No of SHIM DMA BDs to be updated using this argument.
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_AddressPatching(Swig_DevInst *DevInst,
	u32 Arg_Index, u8 Num_BDs)
{
	AieRC RC;
	void *IOInst_swig = DevInst->IOInst;
	RC = XAie_ControlCodeIO_AddressPatching(IOInst_swig,Arg_Index,Num_BDs);
	/**
	 * Set the IsShimBd flag of ControlCodeIO inst structure to :
	 * Use Block Write backend API to write SHIM BD when using Swig Interface.
	 * 
	 * 	 * Note: In ideal case each address patch command will be followed by SHIM
	 *       BD write only. In case of SHIM BD chaining Multiple address patch
	 *       commands will be followed by one WRITE DESC command that chains
	 *       multiple SHIM BDs as UCBDs. 
	 */
	((Swig_ControlCodeIO *)IOInst_swig)->IsShimBd = 1;
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to insert Wait UC
* DMA cert command.
*
* @param	DevInst: Dev instance pointer
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_WaitUcDMA(Swig_DevInst *DevInst)
{
	AieRC RC;
	void *IOInst_swig = DevInst->IOInst;
	RC = XAie_ControlCodeIO_WaitUcDMA(IOInst_swig);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to execute RunOps.
*
* @param	DevInst: Dev Instance Pointer.
* @param	Op: XAie_BackendOpCode opcode.
* @param	Arg: Pointer to arguments.
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_RunOp(Swig_DevInst *DevInst, XAie_BackendOpCode Op, void *Arg)
{
	AieRC RC;
	void *IOInst_swig = DevInst->IOInst;
	RC = XAie_ControlCodeIO_RunOp(IOInst_swig, (XAie_DevInst *)DevInst, Op, Arg);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to add wait for
* TCT cert opcode.
*
* @param	DevInst: Dev Instance pointer
* @param	Column: Column Number
* @param	Row: Row Number
* @param	Channel: Channel Number
* @param	DevInst: Dev Instance pointer
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_WaitTaskCompleteToken(Swig_DevInst *DevInst,
	u16 Column, u16 Row, u32 Channel, u8 NumTokens)
{
	AieRC RC;
	RC = XAie_WaitTaskCompleteToken((XAie_DevInst *)DevInst, Column, Row,
		 Channel, NumTokens);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to add save
* timestamp cert opcode.
*
* @param	DevInst: Device instance pointer
* @param	Timestamp: Time stamp value
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_SaveTimestamp(Swig_DevInst *DevInst, u32 Timestamp)
{
	AieRC RC;
	RC = XAie_ControlCodeSaveTimestamp((XAie_DevInst *)DevInst, Timestamp);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to open a new ASM
* file.
*
* @param	DevInst: Device instance pointer.
* @param	FileName: Name of the ASM file to be created.
* @param	PageSize: Page Size when created ASM file.
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_OpenControlCodeFile(Swig_DevInst *DevInst,
	const char *FileName, u32 PageSize)
{
	AieRC RC;
	RC = XAie_OpenControlCodeFile((XAie_DevInst *)DevInst, FileName, PageSize);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to create new job
* cert command in ASM file.
*
* @param	DevInst: Device instance pointer.
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_StartNewJob(Swig_DevInst *DevInst)
{
	AieRC RC;
	RC = XAie_StartNewJob((XAie_DevInst *)DevInst);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to create end job
* cert command in ASM file.
*
* @param	DevInst: Device instance pointer.
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_EndJob(Swig_DevInst *DevInst)
{
	AieRC RC;
	RC = XAie_EndJob((XAie_DevInst *)DevInst);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to create end page
* cert command in ASM file.
*
* @param	DevInst: Device instance pointer.
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_EndPage(Swig_DevInst *DevInst)
{
	AieRC RC;
	RC = XAie_EndPage((XAie_DevInst *)DevInst);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to create end page
* cert command in ASM file.
*
* @param	DevInst: Device instance pointer.
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_ControlCodeAddAnnotation(Swig_DevInst *DevInst,
	u32 Id, const char *Name, const char *Description)
{
	AieRC RC;
	RC = XAie_ControlCodeAddAnnotation((XAie_DevInst *)DevInst, Id, Name,
		 Description);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to set scratchpad
* cert command in ASM file.
*
* @param	DevInst: Device instance pointer.
* @param	Scrachpad: Scratchpad.
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_ControlCodeSetScrachPad(Swig_DevInst *DevInst,
	const char *Scrachpad)
{
	AieRC RC;
	RC = XAie_ControlCodeSetScrachPad((XAie_DevInst *)DevInst, Scrachpad);
	return RC;
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to close ASM file.
*
* @param	DevInst: Device instance pointer.
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
void XAie_ControlCodeIO_swig_CloseControlCodeFile(Swig_DevInst *DevInst)
{
	XAie_CloseControlCodeFile((XAie_DevInst *)DevInst);
}

/*****************************************************************************/
/**
*
* This is the public interface swig controlcode IO function to free the global
* IO instance
*
* @param	IOInst: IO Instance pointer.
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
int XAie_ControlCodeIO_swig_Finish(Swig_ControlCodeIO IOInst) {
	AieRC RC;
	void *IOInst_swig = &IOInst;
	RC = XAie_ControlCodeIO_Finish(IOInst_swig);
	return RC;
}
#endif // __SWIGINTERFACE__
