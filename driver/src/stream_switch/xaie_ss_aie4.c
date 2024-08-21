/******************************************************************************
* Copyright (C) 2023 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_ss_aie4.c
* @{
*
* This file contains internal api implementations for AIE4 stream switch.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who         Date        Changes
* ----- ---------   ----------  -----------------------------------------------
* 1.0   jbaniset   11/21/2023  Initial creation
* </pre>
*
******************************************************************************/
#include "xaie_feature_config.h"
#include "xaie_helper.h"

#ifdef XAIE_FEATURE_SS_ENABLE
/****** Enum for AIE Tile PortType Base indexes ******/
typedef enum {
	AIETILE_SUBRDNT_CORE	= 0,
	AIETILE_SUBRDNT_MM2S	= 1,
	AIETILE_SUBRDNT_SOUTH	= 2,
	AIETILE_SUBRDNT_WEST	= 10,
	AIETILE_SUBRDNT_NORTH	= 12,
	AIETILE_SUBRDNT_EAST	= 16,
	AIETILE_SUBRDNT_SC	= 18,
	AIETILE_SUBRDNT_32b	= 20,
	AIETILE_SUBRDNT_MAX,
}_AieTile_SlvPortTypeBaseIdx;

typedef enum {
	AIETILE_MNGR_CORE	= 0,
	AIETILE_MNGR_S2MM	= 2,
	AIETILE_MNGR_SOUTH	= 4,
	AIETILE_MNGR_WEST	= 8,
	AIETILE_MNGR_NORTH	= 10,
	AIETILE_MNGR_EAST	= 18,
	AIETILE_MNGR_NC		= 20,
	AIETILE_MNGR_TC		= 22,
	AIETILE_MNGR_32b	= 23,
	AIETILE_MNGR_MAX
}_AieTile_MngrPortTypeBaseIdx;

typedef enum {
	AIETILE_DUALAPP_SUBRDNT_CORE	= 0,
	AIETILE_DUALAPP_SUBRDNT_MM2S	= 1,
	AIETILE_DUALAPP_SUBRDNT_SOUTH	= 2,
	AIETILE_DUALAPP_SUBRDNT_NORTH	= 7,
	AIETILE_DUALAPP_SUBRDNT_SC	= 9,
	AIETILE_DUALAPP_SUBRDNT_32b	= 10,
	AIETILE_DUALAPP_SUBRDNT_MAX
}_AieTile_DualAppSlvPortTypeBaseIdx;

typedef enum {
	AIETILE_DUALAPP_MNGR_CORE	= 0,
	AIETILE_DUALAPP_MNGR_S2MM	= 2,
	AIETILE_DUALAPP_MNGR_SOUTH	= 4,
	AIETILE_DUALAPP_MNGR_NORTH	= 6,
	AIETILE_DUALAPP_MNGR_NC		= 10,
	AIETILE_DUALAPP_MNGR_TC		= 11,
	AIETILE_DUALAPP_MNGR_32b	= 12,
	AIETILE_DUALAPP_MNGR_MAX,
}_AieTile_DualAppMngrPortTypeBaseIdx;

/****** Enum for MEM Tile PortType Base indexes ******/
typedef enum {
	MEMTILE_SUBRDNT_MM2S	= 0,
	MEMTILE_SUBRDNT_SOUTH	= 10,
	MEMTILE_SUBRDNT_NORTH	= 14,
	MEMTILE_SUBRDNT_SC	= 18,
	MEMTILE_SUBRDNT_32b	= 20,
	MEMTILE_SUBRDNT_MAX	= 22
}_MemTile_SlvPortTypeBaseIdx;

typedef enum {
	MEMTILE_MNGR_S2MM	= 0,
	MEMTILE_MNGR_SOUTH	= 8,
	MEMTILE_MNGR_NORTH	= 12,
	MEMTILE_MNGR_NC		= 20,
	MEMTILE_MNGR_TC		= 22,
	MEMTILE_MNGR_32b	= 24,
	MEMTILE_MNGR_MAX	= 26
}_MemTile_MngrPortTypeBaseIdx;

typedef enum {
	MEMTILE_DUALAPP_SUBRDNT_MM2S	= 0,
	MEMTILE_DUALAPP_SUBRDNT_SOUTH	= 5,
	MEMTILE_DUALAPP_SUBRDNT_NORTH	= 7,
	MEMTILE_DUALAPP_SUBRDNT_SC	= 9,
	MEMTILE_DUALAPP_SUBRDNT_32b	= 10,
	MEMTILE_DUALAPP_SUBRDNT_MAX
}_MemTile_DualAppSlvPortTypeBaseIdx;

typedef enum {
	MEMTILE_DUALAPP_MNGR_S2MM	= 0,
	MEMTILE_DUALAPP_MNGR_SOUTH	= 4,
	MEMTILE_DUALAPP_MNGR_NORTH	= 6,
	MEMTILE_DUALAPP_MNGR_NC		= 10,
	MEMTILE_DUALAPP_MNGR_TC		= 11,
	MEMTILE_DUALAPP_MNGR_32b	= 12,
	MEMTILE_DUALAPP_MNGR_MAX
}_MemTile_DualAppMngrPortTypeBaseIdx;

/****** Enum for SHIM Tile PortType Base indexes ******/
typedef enum {
	SHIMTILE_SUBRDNT_MM2S	= 0,
	SHIMTILE_SUBRDNT_WEST	= 4,
	SHIMTILE_SUBRDNT_NORTH	= 6,
	SHIMTILE_SUBRDNT_EAST	= 10,
	SHIMTILE_SUBRDNT_M2SCTL	= 12,
	SHIMTILE_SUBRDNT_32b	= 14,
	SHIMTILE_SUBRDNT_MAX	= 16
}_ShimTile_SlvPortTypeBaseIdx;

typedef enum {
	SHIMTILE_MNGR_S2MM	= 0,
	SHIMTILE_MNGR_WEST	= 2,
	SHIMTILE_MNGR_NORTH	= 4,
	SHIMTILE_MNGR_EAST	= 8,
	SHIMTILE_MNGR_NC	= 10,
	SHIMTILE_MNGR_TC	= 12,
	SHIMTILE_MNGR_32b	= 14,
	SHIMTILE_MNGR_MAX	= 16
}_ShimTile_MngrPortTypeBaseIdx;

typedef enum {
	SHIMTILE_DUALAPP_SUBRDNT_MM2S	= 0,
	SHIMTILE_DUALAPP_SUBRDNT_M2SCTL	= 2,
	SHIMTILE_DUALAPP_SUBRDNT_NORTH	= 3,
	SHIMTILE_DUALAPP_SUBRDNT_32b	= 5,
	SHIMTILE_DUALAPP_SUBRDNT_MAX	= 6
}_ShimTile_DualAppSlvPortTypeBaseIdx;

typedef enum {
	SHIMTILE_DUALAPP_MNGR_S2MM	= 0,
	SHIMTILE_DUALAPP_MNGR_NORTH	= 1,
	SHIMTILE_DUALAPP_MNGR_NC	= 3,
	SHIMTILE_DUALAPP_MNGR_TC	= 4,
	SHIMTILE_DUALAPP_MNGR_32b	= 5,
	SHIMTILE_DUALAPP_MNGR_MAX	= 6
}_ShimTile_DualAppMngrPortTypeBaseIdx;

/************************** Constant Definitions *****************************/
/*
 * C --> Core
 * M2S --> mm2s
 * S2M --> s2mm
 * S --> South
 * W --> West
 * N --> North
 * E --> East
 * SC --> South_control
 * NC --> North_control
 * TC --> Tile_control
 * 32b --> 32b_switch
 */
static const u8 _XAie4_AieTile_PortsConectivityMatrix[AIETILE_SUBRDNT_MAX][AIETILE_MNGR_MAX] = {
		     /*C0  C1  S2M0  S2M1  S0  S1  S2  S3  W0  W1  N0  N1  N2  N3  N4  N5  N6  N7  E0  E1  NC0  NC1  TC  32b */
	/*  C0  */ {0,  0,   0,    1,   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,   0,   1,  1    },
	/* M2S0 */ {1,  1,   1,    0,   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,   1,   1,  1    },
	/*  S0  */ {1,  1,   1,    1,   1,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,   0,   0,  1    },
	/*  S1  */ {1,  1,   1,    1,   0,  1,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,   0,   0,  1    },
	/*  S2  */ {1,  1,   1,    1,   0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,   0,   0,  1    },
	/*  S3  */ {1,  1,   1,    1,   0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,   0,   0,  1    },
	/*  S4  */ {1,  1,   1,    1,   0,  0,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,   0,   0,  1    },
	/*  S5  */ {1,  1,   1,    1,   0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,   0,   0,  1    },
	/*  S6  */ {1,  1,   1,    1,   0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,   0,   0,  1    },
	/*  S7  */ {1,  1,   1,    1,   0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,   0,   0,  1    },
	/*  W0  */ {1,  1,   1,    1,   1,  1,  1,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,   1,   1,  1    },
	/*  W1  */ {1,  1,   1,    1,   1,  1,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,   1,   1,  1    },
	/*  N0  */ {1,  1,   1,    1,   1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,   1,   1,  1    },
	/*  N1  */ {1,  1,   1,    1,   1,  1,  1,  1,  1,  1,  0,  1,  0,  0,  0,  0,  0,  0,  1,  1,  1,   1,   1,  1    },
	/*  N2  */ {1,  1,   1,    1,   1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1,  0,  0,  0,  1,  1,  1,   1,   1,  1    },
	/*  N3  */ {1,  1,   1,    1,   1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  1,  0,  0,  1,  1,  1,   1,   1,  1    },
	/*  E0  */ {1,  1,   1,    1,   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,   1,   1,  1    },
	/*  E1  */ {1,  1,   1,    1,   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,   1,   1,  1    },
	/*  SC0 */ {1,  0,   1,    0,   0,  0,  0,  0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,   0,   1,  1    },
	/*  SC1 */ {0,  1,   0,    1,   0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,   1,   1,  1    },
	/*  32b */ {1,  1,   1,    1,   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,   1,   1,  0    }
};

static const u8 _XAie4_AieTile_DualAppPortsConectivityMatrix[AIETILE_DUALAPP_SUBRDNT_MAX][AIETILE_DUALAPP_MNGR_MAX] = {
		  /*C0  C1  S2M0  S2M1  S0  S1  N0  N1  N2  N3  NC0 TC  32b */
	/*  C0  */ {0,  0,   0,    1,   1,  1,  1,  1,  1,  1,  1,  1,  1  },
	/* M2S0 */ {1,  1,   1,    0,   1,  1,  1,  1,  1,  1,  1,  1,  1  },
	/*  S0  */ {1,  1,   1,    1,   1,  0,  1,  1,  1,  1,  0,  0,  1  },
	/*  S1  */ {1,  1,   1,    1,   0,  1,  1,  1,  1,  1,  0,  0,  1  },
	/*  S2  */ {1,  1,   1,    1,   0,  0,  1,  1,  1,  1,  0,  0,  1  },
	/*  S3  */ {1,  1,   1,    1,   0,  0,  1,  1,  1,  1,  0,  0,  1  },
	/*  N0  */ {1,  1,   1,    1,   1,  1,  1,  0,  0,  0,  0,  0,  1  },
	/*  N1  */ {1,  1,   1,    1,   1,  1,  1,  1,  1,  1,  0,  0,  0  },
	/*  SC0 */ {1,  0,   1,    0,   0,  0,  0,  0,  0,  0,  1,  1,  1  },
	/*  32b */ {1,  1,   1,    1,   1,  1,  1,  1,  1,  1,  1,  1,  0  }
};

static const u8 _XAie4_MemTile_PortsConectivityMatrix[MEMTILE_SUBRDNT_MAX][MEMTILE_MNGR_MAX] = {
		      /*S2M0  S2M1  S2M2  S2M3  S2M4  S2M5  S2M6  S2M7  S0  S1  S2  S3  N0  N1  N2  N3  N4  N5  N6  N7  NC0  NC1  TC0  TC1  32b0  32b1 */
	/* M2S0 */ {1,     0,    0,    0,    0,    0,    0,    0,   1,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,   0,   0,   0,   0,   1,    1},
	/* M2S1 */ {0,     1,    0,    0,    0,    0,    0,    0,   0,  1,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,   0,   0,   0,   0,   1,    1},
	/* M2S2 */ {0,     0,    1,    0,    0,    0,    0,    0,   1,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,   0,   0,   0,   0,   1,    1},
	/* M2S3 */ {0,     0,    0,    1,    0,    0,    0,    0,   0,  1,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,   1,   0,   1,   0,   1,    1},
	/* M2S4 */ {0,     0,    0,    0,    0,    0,    0,    0,   1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,   1,   0,   1,   0,   1,    1},
	
	/* M2S6 */ {0,     0,    0,    0,    1,    0,    0,    0,   0,  0,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,   0,   0,   0,   0,   1,    1},
	/* M2S7 */ {0,     0,    0,    0,    0,    1,    0,    0,   0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,   0,   0,   0,   0,   1,    1},
	/* M2S8 */ {0,     0,    0,    0,    0,    0,    1,    0,   0,  0,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,   0,   0,   0,   0,   1,    1},
	/* M2S9 */ {0,     0,    0,    0,    0,    0,    0,    1,   0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,   0,   1,   0,   1,   1,    1},
	/* M2S10*/ {0,     0,    0,    0,    0,    0,    0,    0,   1,  1,  1,  1,  0,  0,  0,  0,  1,  0,  0,  0,   0,   1,   0,   1,   1,    1},
	
	/*  S0  */ {1,     0,    1,    1,    0,    0,    0,    0,   1,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,   0,   0,   0,   0,   1,    1},
	/*  S1  */ {0,     1,    1,    1,    0,    0,    0,    0,   0,  1,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,   0,   0,   0,   0,   1,    1},
	/*  S2  */ {0,     0,    0,    0,    1,    0,    1,    1,   0,  0,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,   0,   0,   0,   0,   1,    1},
	/*  S3  */ {0,     0,    0,    0,    0,    1,    1,    1,   0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,   0,   0,   0,   0,   1,    1},

	/*  N0  */ {1,     1,    1,    0,    0,    0,    0,    0,   1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,   0,   0,   0,   0,   1,    1},
	/*  N1  */ {1,     1,    0,    1,    0,    0,    0,    0,   1,  1,  1,  1,  0,  1,  0,  0,  0,  0,  0,  0,   0,   0,   1,   0,   1,    1},
	/*  N2  */ {1,     1,    0,    0,    1,    1,    1,    0,   1,  1,  1,  1,  0,  0,  0,  0,  1,  0,  0,  0,   0,   0,   0,   0,   1,    1},
	/*  N3  */ {1,     1,    0,    0,    1,    1,    0,    1,   1,  1,  1,  1,  0,  0,  0,  0,  0,  1,  0,  0,   0,   0,   1,   1,   1,    1},

	/*  SC0 */ {0,     0,    0,    1,    0,    0,    0,    0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   1,   0,   1,   0,   1,    1},
	/*  SC1 */ {0,     0,    0,    0,    0,    0,    0,    1,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,   1,   1,   1,   1,    1},
	/* 32b0 */ {1,     1,    1,    1,    1,    1,    1,    1,   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,   1,   0,   1,   0,   0,    0},
	/* 32b1 */ {1,     1,    1,    1,    1,    1,    1,    1,   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,   0,   1,   0,   1,   0,    0}
};

static const u8 _XAie4_MemTile_DualAppPortsConectivityMatrix[MEMTILE_DUALAPP_SUBRDNT_MAX][MEMTILE_DUALAPP_MNGR_MAX] = {
		      /*S2M0  S2M1  S2M2  S2M3   S0    S1    N0   N1  N2  N3  NC0  TC0  32b0 */
	/* M2S0 */ {1,     0,    0,    0,    1,    0,    1,   1,  1,  1,  0,   0,   1 },
	/* M2S1 */ {0,     1,    0,    0,    0,    1,    1,   1,  1,  1,  0,   0,   1 },
	/* M2S2 */ {0,     0,    1,    0,    1,    0,    1,   1,  1,  1,  0,   0,   1 },
	/* M2S3 */ {0,     0,    0,    1,    0,    1,    1,   1,  1,  1,  0,   0,   1 },
	/* M2S4 */ {0,     0,    0,    0,    1,    1,    1,   0,  0,  0,  1,   1,   1 },
	
	/*  S0  */ {1,     0,    1,    1,    1,    0,    1,   1,  1,  1,  0,   0,   1 },
	/*  S1  */ {0,     1,    1,    1,    0,    1,    1,   1,  1,  1,  0,   0,   1 },

	/*  N0  */ {1,     1,    1,    0,    1,    1,    1,   0,  0,  0,  0,   0,   1 },
	/*  N1  */ {1,     1,    0,    1,    1,    1,    0,   1,  0,  0,  0,   1,   1 },

	/*  SC0 */ {0,     0,    0,    1,    0,    0,    0,   0,  0,  0,  1,   1,   1 },
	/* 32b0 */ {1,     1,    1,    1,    1,    1,    1,   1,  1,  1,  1,   1,   0 }
};

static const u8 _XAie4_ShimTile_PortsConectivityMatrix[SHIMTILE_SUBRDNT_MAX][SHIMTILE_MNGR_MAX] = {
		      /*S2M0  S2M2  W0  W1  N0  N1  N2  N3  E0  E1  NC0  NC1  TC0  TC1  32b0  32b1*/
	/* M2S0 */ { 1,    0,   1,  1,  1,  1,  1,  1,  1,  1,  1,   0,   1,   0,    1,    1 },
	/* M2S1 */ { 0,    0,   1,  1,  1,  1,  1,  1,  1,  1,  1,   0,   1,   0,    1,    1 },
	/* M2S2 */ { 0,    1,   1,  1,  1,  1,  1,  1,  1,  1,  0,   1,   0,   1,    1,    1 },
	/* M2S3 */ { 0,    0,   1,  1,  1,  1,  1,  1,  1,  1,  0,   1,   0,   1,    1,    1 },
	/*  W0  */ { 1,    1,   1,  0,  1,  1,  1,  1,  1,  1,  1,   1,   1,   0,    1,    1 },
	/*  W1  */ { 1,    1,   0,  1,  1,  1,  1,  1,  1,  1,  1,   1,   1,   0,    1,    1 },
	/*  N0  */ { 1,    1,   1,  1,  1,  0,  0,  0,  1,  1,  0,   0,   0,   0,    1,    1 },
	/*  N1  */ { 1,    1,   1,  1,  0,  1,  0,  0,  1,  1,  1,   0,   1,   0,    1,    1 },
	/*  N2  */ { 1,    1,   1,  1,  0,  0,  1,  0,  1,  1,  0,   0,   0,   0,    1,    1 },
	/*  N3  */ { 1,    1,   1,  1,  0,  0,  0,  1,  1,  1,  1,   1,   1,   1,    1,    1 },
	/*  E0  */ { 1,    1,   1,  1,  1,  1,  1,  1,  1,  0,  1,   1,   1,   0,    1,    1 },
	/*  E1  */ { 1,    1,   1,  1,  1,  1,  1,  1,  0,  1,  1,   1,   1,   0,    1,    1 },
	/* M2SC0*/ { 0,    0,   1,  1,  1,  1,  0,  0,  1,  1,  1,   0,   1,   0,    1,    0 },
	/* M2SC1*/ { 0,    0,   0,  1,  0,  0,  1,  1,  0,  1,  0,   1,   1,   1,    0,    1 },
	/* 32b0 */ { 1,    1,   1,  1,  1,  1,  1,  1,  1,  1,  1,   1,   1,   0,    0,    0 },
	/* 32b1 */ { 1,    1,   1,  1,  1,  1,  1,  1,  1,  1,  1,   1,   0,   1,    0,    0 }
};

static const u8 _XAie4_ShimTile_DualAppPortsConectivityMatrix[SHIMTILE_DUALAPP_SUBRDNT_MAX][SHIMTILE_DUALAPP_MNGR_MAX] = {
		      /*S2M0  N0    N1  NC0  TC0  32b0*/
	/* M2S0 */ { 1,   1,    1,   1,  1,    1 },
	/* M2S1 */ { 0,   1,    1,   1,  1,    1 },
	/* M2SC0*/ { 0,   1,    1,   1,  1,    1 },
	/*  N0  */ { 1,   1,    0,   0,  0,    1 },
	/*  N1  */ { 1,   0,    1,   1,  1,    1 },
	/* 32b0 */ { 1,   1,    1,   1,  1,    0 }
};

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This api verifies if a stream switch(512b) connection exists within the ports.
* Numerous conditions exist to determine if the port combination is valid.
* The method assumes both ports exist, and the relevant tile is an AIE-Tile.
*
* @param	Slave: The type of the slave port.
* @param	SlvPortNum: The number of the slave port.
* @param	Master: The type of the master port.
* @param	MstrPortNum: The number of the master port.
*
* @return	XAIE_OK if a stream switch connection is possible.
*		XAIE_ERR_STREAM_PORT if the connection isn't possible.
*
* @note		Internal API for AIE4. This API shouldn't be called directly.
*		It is invoked using a function pointer within the Stream
*		Module data structure.
*
*****************************************************************************/
AieRC _XAie4_AieTile_StrmSwCheckPortValidity(XAie_DevInst *DevInst,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum)
{
	u8 SubrntePortIdx = 0;
	u8 MngrPortIdx = 0;
	u8 DualAppMode;

	if (DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE)
		DualAppMode = 0;
	else
		DualAppMode = 1;

	switch (Slave) {
	case CORE:
		SubrntePortIdx = DualAppMode ? AIETILE_DUALAPP_SUBRDNT_CORE : AIETILE_SUBRDNT_CORE;
		break;
	case DMA:
		SubrntePortIdx = DualAppMode ? AIETILE_DUALAPP_SUBRDNT_MM2S : AIETILE_SUBRDNT_MM2S;
		break;
	case SOUTH:
		SubrntePortIdx = DualAppMode ? AIETILE_DUALAPP_SUBRDNT_SOUTH: AIETILE_SUBRDNT_SOUTH;
		break;
	case WEST:
		if (DualAppMode)
			return XAIE_ERR_STREAM_PORT;
		SubrntePortIdx = AIETILE_SUBRDNT_WEST; break;
	case NORTH:
		SubrntePortIdx = DualAppMode ? AIETILE_DUALAPP_SUBRDNT_NORTH : AIETILE_SUBRDNT_NORTH;
		break;
	case EAST:
		if (DualAppMode)
			return XAIE_ERR_STREAM_PORT;
		SubrntePortIdx = AIETILE_SUBRDNT_EAST; break;
	case SOUTH_CTRL:
		SubrntePortIdx = DualAppMode ? AIETILE_DUALAPP_SUBRDNT_SC : AIETILE_SUBRDNT_SC;
		break;
	case SWITCH_32b:
		SubrntePortIdx = DualAppMode ? AIETILE_DUALAPP_SUBRDNT_32b : AIETILE_SUBRDNT_32b;
		break;
	default:
		/* Any port that is not shown in connectivity matrix is fully connected */
		return XAIE_OK;
	}
	SubrntePortIdx += SlvPortNum;

	switch (Master) {
	case CORE:
		MngrPortIdx = DualAppMode ? AIETILE_DUALAPP_MNGR_CORE : AIETILE_MNGR_CORE;
		break;
	case DMA:
		MngrPortIdx = DualAppMode ? AIETILE_DUALAPP_MNGR_S2MM : AIETILE_MNGR_S2MM;
		break;
	case SOUTH:
		MngrPortIdx = DualAppMode ? AIETILE_DUALAPP_MNGR_SOUTH : AIETILE_MNGR_SOUTH;
		break;
	case WEST:
		if (DualAppMode)
			return XAIE_ERR_STREAM_PORT;
		MngrPortIdx = AIETILE_MNGR_WEST; break;
	case NORTH:
		MngrPortIdx = DualAppMode ? AIETILE_DUALAPP_MNGR_NORTH : AIETILE_MNGR_NORTH;
		break;
	case EAST:
		if (DualAppMode)
			return XAIE_ERR_STREAM_PORT;
		MngrPortIdx = AIETILE_MNGR_EAST; break;
	case NORTH_CTRL:
		MngrPortIdx = DualAppMode ? AIETILE_DUALAPP_MNGR_NC : AIETILE_MNGR_NC;
		break;
	case CTRL:
		MngrPortIdx = DualAppMode ? AIETILE_DUALAPP_MNGR_TC : AIETILE_MNGR_TC;
		break;
	case SWITCH_32b:
		MngrPortIdx = DualAppMode ? AIETILE_DUALAPP_SUBRDNT_32b : AIETILE_SUBRDNT_32b;
		break;
	default:
		/* Any port that is not shown in connectivity matrix is fully connected */
		return XAIE_OK;
	}
	MngrPortIdx += MstrPortNum;

	if (DualAppMode) {
		/* Safe check to see if the indexes are out of connectivity matrix bound */
		if ((SubrntePortIdx >= AIETILE_DUALAPP_SUBRDNT_MAX) ||
				(MngrPortIdx >= AIETILE_DUALAPP_MNGR_MAX))
			return XAIE_ERR_STREAM_PORT;

		if (_XAie4_AieTile_DualAppPortsConectivityMatrix[SubrntePortIdx][MngrPortIdx])
			return XAIE_OK;
		else
			return XAIE_ERR_STREAM_PORT;

	} else {
		/* Safe check to see if the indexes are out of connectivity matrix bound */
		if ((SubrntePortIdx >= AIETILE_SUBRDNT_MAX) ||
			(MngrPortIdx >= AIETILE_MNGR_MAX))
			return XAIE_ERR_STREAM_PORT;

		if(_XAie4_AieTile_PortsConectivityMatrix[SubrntePortIdx][MngrPortIdx])
			return XAIE_OK;
		else
			return XAIE_ERR_STREAM_PORT;
	}
}

/**
*
* This api verifies if a stream switch connection exists within the ports.
* Less valid ports exist in the MemTile, but certain combinations exist.
* The method assumes both ports exist, and the relevant tile is an MEM-Tile.
*
* @param	Slave: The type of the slave port.
* @param	SlvPortNum: The number of the slave port.
* @param	Master: The type of the master port.
* @param	MstrPortNum: The number of the master port.
*
* @return	XAIE_OK if a stream switch connection is possible.
* 		XAIE_ERR_STREAM_PORT if the connection isn't possible.
*
* @note		Internal API for AIE4. This API shouldn't be called directly.
* 		It is invoked using a function pointer within the Stream
* 		Module data structure.
*****************************************************************************/
AieRC _XAie4_MemTile_StrmSwCheckPortValidity(XAie_DevInst *DevInst,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum)
{
	u8 SubrntePortIdx = 0;
	u8 MngrPortIdx = 0;
	u8 DualAppMode;
	u8 HoleInTheMatrixTable = 0;

	if (DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE)
		DualAppMode = 0;
	else
		DualAppMode = 1;

	switch (Slave) {
	case DMA:
		if (DualAppMode) {
			SubrntePortIdx = MEMTILE_DUALAPP_SUBRDNT_MM2S;
		} else {
			/* In v1.2 spec, MM2S5 & MM2S11 are removed but ports numbers are not
			 * adjusted in the spec, below logic is to validate that and calculate
			 * index based on that
			 */
			SubrntePortIdx = MEMTILE_SUBRDNT_MM2S;
			if (SlvPortNum == MEMTILE_SUBRDNT_SOUTH / 2)
				return XAIE_ERR_STREAM_PORT;
			else if (SlvPortNum > MEMTILE_SUBRDNT_SOUTH / 2)
				HoleInTheMatrixTable = 1;
		}
		break;
	case SOUTH:
		SubrntePortIdx = DualAppMode ? MEMTILE_DUALAPP_SUBRDNT_SOUTH : MEMTILE_SUBRDNT_SOUTH;
		break;
	case NORTH:
		SubrntePortIdx = DualAppMode ? MEMTILE_DUALAPP_SUBRDNT_NORTH : MEMTILE_SUBRDNT_NORTH;
		break;
	case SOUTH_CTRL:
		SubrntePortIdx = DualAppMode ? MEMTILE_DUALAPP_SUBRDNT_SC : MEMTILE_SUBRDNT_SC;
		break;
	case SWITCH_32b:
		SubrntePortIdx = DualAppMode ? MEMTILE_DUALAPP_SUBRDNT_32b : MEMTILE_SUBRDNT_32b;
		break;
	default:
		/* Any port that is not shown in connectivity matrix is fully connected */
		return XAIE_OK;
	}
	SubrntePortIdx += SlvPortNum - HoleInTheMatrixTable;

	switch (Master) {
	case DMA:
		MngrPortIdx = DualAppMode ? MEMTILE_DUALAPP_MNGR_S2MM : MEMTILE_MNGR_S2MM;
		break;
	case SOUTH:
		MngrPortIdx = DualAppMode ? MEMTILE_DUALAPP_MNGR_SOUTH : MEMTILE_MNGR_SOUTH;
		break;
	case NORTH:
		MngrPortIdx = DualAppMode ? MEMTILE_DUALAPP_MNGR_NORTH : MEMTILE_MNGR_NORTH;
		break;
	case NORTH_CTRL:
		MngrPortIdx = DualAppMode ? MEMTILE_DUALAPP_MNGR_NC : MEMTILE_MNGR_NC;
		break;
	case CTRL:
		MngrPortIdx = DualAppMode ? MEMTILE_DUALAPP_MNGR_TC : MEMTILE_MNGR_TC;
		break;
	case SWITCH_32b:
		MngrPortIdx = DualAppMode ? MEMTILE_DUALAPP_MNGR_32b : MEMTILE_MNGR_32b;
		break;
	default:
		/* Any port that is not shown in connectivity matrix is fully connected */
		return XAIE_OK;
	}
	MngrPortIdx += MstrPortNum;

	if (DualAppMode) {
		/* Safe check to see if the indexes are out of connectivity matrix bound */
		if ((SubrntePortIdx >= MEMTILE_DUALAPP_SUBRDNT_MAX) ||
				(MngrPortIdx >= MEMTILE_DUALAPP_MNGR_MAX))
			return XAIE_ERR_STREAM_PORT;

		if(_XAie4_MemTile_DualAppPortsConectivityMatrix[SubrntePortIdx][MngrPortIdx])
			return XAIE_OK;
		else
			return XAIE_ERR_STREAM_PORT;

	} else {
		/* Safe check to see if the indexes are out of connectivity matrix bound */
		if ((SubrntePortIdx >= MEMTILE_SUBRDNT_MAX) ||
			(MngrPortIdx >= MEMTILE_MNGR_MAX))
			return XAIE_ERR_STREAM_PORT;

		if(_XAie4_MemTile_PortsConectivityMatrix[SubrntePortIdx][MngrPortIdx])
			return XAIE_OK;
		else
			return XAIE_ERR_STREAM_PORT;
	}
}

/**
*
* This api verifies if a stream switch connection exists within the ports.
* Less types of stream switch port exist within the SHIM tile.
* The method assumes both ports exist, and the relevant tile is a SHIM-Tile.
*
* @param	Slave: The type of the slave port.
* @param	SlvPortNum: The number of the slave port.
* @param	Master: The type of the master port.
* @param	MstrPortNum: The number of the master port.
*
* @return	XAIE_OK if a stream switch connection is possible.
* 		XAIE_ERR_STREAM_PORT if the connection isn't possible.
*
* @note		Internal API for AIE-ML. This API shouldn't be called directly.
* 		It is invoked using a function pointer within the Stream
* 		Module data structure.
*
*****************************************************************************/
AieRC _XAie4_ShimTile_StrmSwCheckPortValidity(XAie_DevInst *DevInst,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum)
{
	u8 SubrntePortIdx = 0;
	u8 MngrPortIdx = 0;
	u8 DualAppMode;
	u8 HoleInTheMatrixTable = 0;

	if (DevInst->AppMode == XAIE_DEVICE_SINGLE_APP_MODE)
		DualAppMode = 0;
	else
		DualAppMode = 1;

	switch (Slave) {
	case DMA:
		SubrntePortIdx = DualAppMode ? SHIMTILE_DUALAPP_SUBRDNT_MM2S : SHIMTILE_SUBRDNT_MM2S;
		break;
	case WEST:
		if (DualAppMode)
			return XAIE_ERR_STREAM_PORT;
		SubrntePortIdx = SHIMTILE_SUBRDNT_WEST;
		break;
	case NORTH:
		SubrntePortIdx = DualAppMode ? SHIMTILE_DUALAPP_SUBRDNT_NORTH : SHIMTILE_SUBRDNT_NORTH;
		break;
	case EAST:
		if (DualAppMode)
			return XAIE_ERR_STREAM_PORT;
		SubrntePortIdx = SHIMTILE_SUBRDNT_EAST;
		break;
	case DMA_CTRL:
		SubrntePortIdx = DualAppMode ? SHIMTILE_DUALAPP_SUBRDNT_M2SCTL : SHIMTILE_SUBRDNT_M2SCTL;
		break;
	case SWITCH_32b:
		SubrntePortIdx = DualAppMode ? SHIMTILE_DUALAPP_SUBRDNT_32b : SHIMTILE_SUBRDNT_32b;
		break;
	default:
		/* Any port that is not shown in connectivity matrix is fully connected */
		return XAIE_OK;
	}
	SubrntePortIdx += SlvPortNum;

	switch (Master) {
	case DMA:
		if (DualAppMode) {
			MngrPortIdx = SHIMTILE_DUALAPP_MNGR_S2MM;
		} else {
			/* In v1.2 spec, S2MM1 & S2MM3 are removed but ports numbers are not
			 * adjusted in the spec, below logic is to validate that and calculate
			 * index based on that
			 */
			MngrPortIdx = SHIMTILE_MNGR_S2MM;
			if (MstrPortNum == SHIMTILE_MNGR_WEST / 2)
				return XAIE_ERR_STREAM_PORT;
			else if (MstrPortNum > SHIMTILE_MNGR_WEST / 2)
				HoleInTheMatrixTable = 1;
		}
		break;
	case WEST:
		if (DualAppMode)
			return XAIE_ERR_STREAM_PORT;
		MngrPortIdx = SHIMTILE_MNGR_WEST;
		break;
	case NORTH:
		MngrPortIdx = DualAppMode ? SHIMTILE_DUALAPP_MNGR_NORTH : SHIMTILE_MNGR_NORTH;
		break;
	case EAST:
		if (DualAppMode)
			return XAIE_ERR_STREAM_PORT;
		MngrPortIdx = SHIMTILE_MNGR_EAST;
		break;
	case NORTH_CTRL:
		MngrPortIdx = DualAppMode ? SHIMTILE_DUALAPP_MNGR_NC : SHIMTILE_MNGR_NC;
		break;
	case CTRL:
		MngrPortIdx = DualAppMode ? SHIMTILE_DUALAPP_MNGR_TC : SHIMTILE_MNGR_TC;
		break;
	case SWITCH_32b:
		MngrPortIdx = DualAppMode ? SHIMTILE_DUALAPP_MNGR_32b : SHIMTILE_MNGR_32b;
		break;
	default:
		/* Any port that is not shown in connectivity matrix is fully connected */
		return XAIE_OK;
	}
	MngrPortIdx += MstrPortNum - HoleInTheMatrixTable;

	if (DualAppMode) {
		/* Safe check to see if the indexes are out of connectivity matrix bound */
		if ((SubrntePortIdx >= SHIMTILE_DUALAPP_SUBRDNT_MAX) ||
				(MngrPortIdx >= SHIMTILE_DUALAPP_MNGR_MAX))
			return XAIE_ERR_STREAM_PORT;

		if(_XAie4_ShimTile_DualAppPortsConectivityMatrix[SubrntePortIdx][MngrPortIdx])
			return XAIE_OK;
		else
			return XAIE_ERR_STREAM_PORT;

	} else {
		/* Safe check to see if the indexes are out of connectivity matrix bound */
		if ((SubrntePortIdx >= SHIMTILE_SUBRDNT_MAX) ||
			(MngrPortIdx >= SHIMTILE_MNGR_MAX))
			return XAIE_ERR_STREAM_PORT;

		if(_XAie4_ShimTile_PortsConectivityMatrix[SubrntePortIdx][MngrPortIdx])
			return XAIE_OK;
		else
			return XAIE_ERR_STREAM_PORT;
	}
}

/*****************************************************************************/
/**
*
* This api verifies if a stream switch(32b) connection exists within the ports.
* Numerous conditions exist to determine if the port combination is valid.
* The method assumes both ports exist, and the relevant tile is an AIE-Tile.
*
* @param	Slave: The type of the slave port.
* @param	SlvPortNum: The number of the slave port.
* @param	Master: The type of the master port.
* @param	MstrPortNum: The number of the master port.
*
* @return	XAIE_OK if a stream switch connection is possible.
*		XAIE_ERR_STREAM_PORT if the connection isn't possible.
*
* @note		Internal API for AIE4. This API shouldn't be called directly.
*		It is invoked using a function pointer within the Stream
*		Module data structure.
*
*****************************************************************************/
AieRC _XAie4_StrmSw32bCheckPortValidity(XAie_DevInst *DevInst,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum)
{
	(void)DevInst;
	(void)Slave;
	(void)SlvPortNum;
	(void)Master;
	(void)MstrPortNum;
	return XAIE_OK;
}

#endif /* XAIE_FEATURE_SS_ENABLE */
