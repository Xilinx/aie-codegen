#ifndef XAIE_LITE_AIE2P_SSW_AND_DMA_H
#define XAIE_LITE_AIE2P_SSW_AND_DMA_H

#include "xaiegbl_defs.h"
// Stream_Switch_Master_Config_South2
typedef union {
    uint32_t value;
    struct {
        uint32_t Configuration  : 7;  // [6:0]
        uint32_t Drop_Header    : 1;  // [7]
        uint32_t reserved0      : 22; // [8:29]
        uint32_t Packet_Enable  : 1;  // [30]
        uint32_t Master_Enable  : 1;  // [31]
    } bits;
} SSwitch_Master_Port_Config_t;

// Stream_Switch_Slave_Config_South_3
typedef union {
    uint32_t value;
    struct {
        uint32_t reserved0      : 30; // [0:29]
        uint32_t Packet_Enable  : 1;  // [30]
        uint32_t Slave_Enable   : 1;  // [31]
    } bits;
} SSwitch_Slave_Port_Config_t;

// Mux_Config
typedef union {
    uint32_t value;
    struct {
        uint32_t reserved0 : 8;   // [0:7]
        uint32_t South2    : 2;   // [8:9]
        uint32_t South3    : 2;   // [10:11]
        uint32_t South6    : 2;   // [12:13]
        uint32_t South7    : 2;   // [14:15]
        uint32_t reserved1 : 16;  // [16:31]
    } bits;
} Mux_Config_t;

// Demux_Config
typedef union {
    uint32_t value;
    struct {
        uint32_t reserved0 : 4;   // [0:3]
        uint32_t South2    : 2;   // [4:5]
        uint32_t South3    : 2;   // [6:7]
        uint32_t South4    : 2;   // [8:9]
        uint32_t South5    : 2;   // [10:11]
        uint32_t reserved1 : 20;  // [12:31]
    } bits;
} Demux_Config_t;

// DMA_S2MM_0_Task_Queue & DMA_MM2S_0_Task_Queue
typedef union {
    uint32_t value;
    struct {
        uint32_t Start_BD_ID        : 4;   // [0:3]
        uint32_t reserved0          : 12;  // [4:15]
        uint32_t Repeat_Count       : 8;   // [16:23]
        uint32_t reserved1          : 7;   // [24:30]
        uint32_t Enable_Token_Issue : 1;   // [31]
    } bits;
} DMA_Task_Queue_t;

typedef struct {
    union {
        uint32_t value;
        struct {
            uint32_t Buffer_Length : 32; // [31:0]
        } bits;
    } reg0; // DMA_BD0_0

    union {
        uint32_t value;
        struct {
            uint32_t reserved0        : 2;  // [1:0]
            uint32_t Base_Address_Low : 30; // [31:2]
        } bits;
    } reg1; // DMA_BD0_1

    union {
        uint32_t value;
        struct {
            uint32_t Base_Address_High   : 16; // [15:0]
            uint32_t Packet_Type         : 3;  // [18:16]
            uint32_t Packet_ID           : 5;  // [23:19]
            uint32_t Out_Of_Order_BD_ID  : 6;  // [29:24]
            uint32_t Enable_Packet       : 1;  // [30]
            uint32_t reserved0           : 1;  // [31]
        } bits;
    } reg2; // DMA_BD0_2

    union {
        uint32_t value;
        struct {
            uint32_t D0_Stepsize   : 20; // [19:0]
            uint32_t D0_Wrap       : 10; // [29:20]
            uint32_t Secure_Access : 1;  // [30]
            uint32_t reserved0     : 1;  // [31]
        } bits;
    } reg3; // DMA_BD0_3

    union {
        uint32_t value;
        struct {
            uint32_t D1_Stepsize  : 20; // [19:0]
            uint32_t D1_Wrap      : 10; // [29:20]
            uint32_t Burst_Length : 2;  // [31:30]
        } bits;
    } reg4; // DMA_BD0_4

    union {
        uint32_t value;
        struct {
            uint32_t D2_Stepsize : 20; // [19:0]
            uint32_t AxQoS       : 4;  // [23:20]
            uint32_t AxCache     : 4;  // [27:24]
            uint32_t SMID        : 4;  // [31:28]
        } bits;
    } reg5; // DMA_BD0_5

    union {
        uint32_t value;
        struct {
            uint32_t Iteration_Stepsize : 20; // [19:0]
            uint32_t Iteration_Wrap     : 6;  // [25:20]
            uint32_t Iteration_Current  : 6;  // [31:26]
        } bits;
    } reg6; // DMA_BD0_6

    union {
        uint32_t value;
        struct {
            uint32_t Lock_Acq_ID     : 4;  // [3:0]
            uint32_t Lock_Acq_Value  : 7;  // [11:5]
            uint32_t Lock_Acq_Enable : 1;  // [12]
            uint32_t Lock_Rel_ID     : 4;  // [16:13]
            uint32_t Lock_Rel_Value  : 7;  // [24:18]
            uint32_t Valid_BD        : 1;  // [25]
            uint32_t Use_Next_BD     : 1;  // [26]
            uint32_t Next_BD         : 4;  // [30:27]
            uint32_t TLAST_Suppress  : 1;  // [31]
        } bits;
    } reg7; // DMA_BD0_7

} DMA_SHIM_BD_t;

#endif