/******************************************************************************
* Copyright (C) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_interrupt_control.c
* @{
*
* This file implements routine for disabling AIE interrupts.
*
*
* Note:
* In AIE4 switching between the two instances of L2 Interrupt Controller
* hardware based on application A or B in dual mode of operation. Is
* managed by the use of partition baseaddress member of DevInst structure
* representing the partition, Since all the functions in this file are
* invoked without reference to partition info (DevInst). The responsibility
* to access the correct configuration registers based on application A or B
* is with driver API. Hence the functions in this file are handling the same.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Nishad  08/20/2021  Initial creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaie_feature_config.h"
#include "xaie_interrupt.h"
#include "xaie_lite.h"
#include "xaie_lite_io.h"

#if defined(XAIE_FEATURE_INTR_CTRL_ENABLE) && defined(XAIE_FEATURE_LITE)

/************************** Constant Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API returns the status of second-level interrupt controller.
*
* @param	Loc: Location of AIE tile.
*
* @return	Status: Status second-level interrupt controller.
*
* @note		Internal only.
*
******************************************************************************/
u32 _XAie_LIntrCtrlL2Status(XAie_LocType Loc, XAie_AppIndex Id)
{
	u64 RegAddr;

#if DEV_GEN_AIE4
	/**
	 * Based on the DevGen and Application Mode select the correct
	 * register to read the L2 IRQ status
	 */
	if (Id == APPLICATION_B) {
		/**
		 * Dev-Gen: AIE4 and derivatives
		 * App-Mode: DUAL APP MODE B
		 */
		RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
				XAIE_NOC_MOD_INTR_L2_APP_B_STATUS;
	} else {
#endif
		/**
		 * Dev-Gen: AIE AIE2 AIE2P AIE4 and derivatives
		 * App-Mode: SINGLE APP MODE (or) DUAL APP MODE A
		 */
		RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
				XAIE_NOC_MOD_INTR_L2_STATUS;
#if DEV_GEN_AIE4
	}
#else
(void)Id;
#endif

	return _XAie_LRead32(RegAddr);
}

/*****************************************************************************/
/**
*
* This API clears the status of interrupts in the second-level interrupt
* controller.
*
* @param	Loc: Location of AIE tile.
* @param	ChannelBitMap: Bitmap of channels to be acknowledged. Writing a
*				value of 1 to the register field clears the
*				corresponding interrupt channel.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
void _XAie_LIntrCtrlL2Ack(XAie_LocType Loc, XAie_AppIndex Id,
					u32 ChannelBitMap)
{
	u64 RegAddr;

#if DEV_GEN_AIE4
	/**
	 * Based on the DevGen and Application Mode select the correct
	 * register to clear the L2 IRQ status
	 */
	if (Id == APPLICATION_B) {
		/**
		 * Dev-Gen: AIE4 and derivatives
		 * App-Mode: DUAL APP MODE B
		 */
		RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
				XAIE_NOC_MOD_INTR_L2_APP_B_STATUS;
	} else {
#endif
		/**
		 * Dev-Gen: AIE AIE2 AIE2P AIE4 and derivatives
		 * App-Mode: SINGLE APP MODE (or) DUAL APP MODE A
		 */
		RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
				XAIE_NOC_MOD_INTR_L2_STATUS;
#if DEV_GEN_AIE4
	}
#else
(void)Id;
#endif

	_XAie_LWrite32(RegAddr, ChannelBitMap);
}

/*****************************************************************************/
/**
*
* This API disables interrupts to second level interrupt controller.
*
* @param	Loc: Location of AIE Tile
* @param	ChannelBitMap: Interrupt Bitmap.
*
* @return	None.
*
* @note		Internal Only.
*
******************************************************************************/
static inline void _XAie_LIntrCtrlL2Disable(XAie_LocType Loc, XAie_AppIndex Id,
					    u32 ChannelBitMap)
{
	u64 RegAddr;

#if DEV_GEN_AIE4
	/**
	 * Based on the DevGen and Application Mode select the correct
	 * register to disable the L2 IRQ status
	 */
	if (Id == APPLICATION_B) {
		/**
		 * Dev-Gen: AIE4 and derivatives
		 * App-Mode: DUAL APP MODE B
		 */
		RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
				XAIE_NOC_MOD_INTR_L2_APP_B_DISABLE;
	} else {
#endif
		/**
		 * Dev-Gen: AIE AIE2 AIE2P AIE4 and derivatives
		 * App-Mode: SINGLE APP MODE (or) DUAL APP MODE A
		 */
		RegAddr = _XAie_LGetTileAddr(Loc.Row, Loc.Col) +
				XAIE_NOC_MOD_INTR_L2_DISABLE;
#if DEV_GEN_AIE4
	}
#else
(void)Id;
#endif

	_XAie_LWrite32(RegAddr, ChannelBitMap);
}

/*****************************************************************************/
/**
*
* This API disables all second-level interrupt controllers reporting errors.
*
* @param	IrqId: Zero indexed IRQ ID. Valid values corresponds to the
*		       number of AIE IRQs mapped to the processor.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XAie_DisableErrorInterrupts(u8 IrqId)
{
	XAie_Range Cols;

	XAie_MapIrqIdToCols(IrqId, &Cols);

	XAie_LocType Loc = XAie_TileLoc(Cols.Start, XAIE_SHIM_ROW);

	if (!IS_TILE_NOC_TILE(Loc)) {
		UPDT_NEXT_NOC_TILE_LOC(Loc);
	}

	while (Loc.Col < Cols.Start + Cols.Num) {
		u32 Status;

		/**
		 * Dev-Gen: AIE AIE2 AIE2P AIE4 and derivatives
		 * App-Mode: SINGLE APP MODE (or) DUAL APP MODE A
		 */
		Status = _XAie_LIntrCtrlL2Status(Loc, APPLICATION_A);

		/* Only disable L2s that are reporting errors. */
		if (Status) {
			_XAie_LIntrCtrlL2Disable(Loc, APPLICATION_A, Status);
		}
#if DEV_GEN_AIE4
		/**
		 * Dev-Gen: AIE4 and derivatives
		 * App-Mode: DUAL APP MODE B
		 */
		Status = _XAie_LIntrCtrlL2Status(Loc, APPLICATION_B);

		/* Only disable L2 IRQ lines that are reporting errors. */
		if (Status) {
			_XAie_LIntrCtrlL2Disable(Loc, APPLICATION_B, Status);
		}
#endif
		UPDT_NEXT_NOC_TILE_LOC(Loc);
	}
}

/*****************************************************************************/
/**
*
* This API Clears all second-level interrupt controllers reporting errors.
*
* @param	IrqId: Zero indexed IRQ ID. Valid values corresponds to the
*		       number of AIE IRQs mapped to the processor.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XAie_ClearErrorInterrupts(u8 ColNum)
{
	XAie_LocType Loc = XAie_TileLoc(ColNum, XAIE_SHIM_ROW);

	if (!IS_TILE_NOC_TILE(Loc)) {
		UPDT_NEXT_NOC_TILE_LOC(Loc);
	}

	u32 Status;

	/**
	 * Dev-Gen: AIE AIE2 AIE2P AIE4 and derivatives
	 * App-Mode: SINGLE APP MODE (or) DUAL APP MODE A
	 */
	Status = _XAie_LIntrCtrlL2Status(Loc, APPLICATION_A);

	/* Only Clear L2s that are reporting errors. */
	if (Status) {
		_XAie_LIntrCtrlL2Ack(Loc, APPLICATION_A, Status);
	}

#if DEV_GEN_AIE4
		/**
		 * Dev-Gen: AIE4 and derivatives
		 * App-Mode: DUAL APP MODE B
		 */
		Status = _XAie_LIntrCtrlL2Status(Loc, APPLICATION_B);

		/* Only disable L2 IRQ lines that are reporting errors. */
		if (Status) {
			_XAie_LIntrCtrlL2Ack(Loc, APPLICATION_B, Status);
		}
#endif
}

#endif /* XAIE_FEATURE_INTR_CTRL_ENABLE */

/** @} */
