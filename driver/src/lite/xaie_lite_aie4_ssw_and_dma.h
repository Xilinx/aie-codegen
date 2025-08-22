#ifndef XAIE_LITE_AIE4_SSW_AND_DMA_H
#define XAIE_LITE_AIE4_SSW_AND_DMA_H

#include "xaiegbl_defs.h"

typedef struct {
    union {
        uint32_t value;
        struct {
            uint32_t Base_Address_High : 25;
            uint32_t KeyIdx            : 3;
            uint32_t AxUSER            : 4;
        } bits;
    } reg0;

    union {
        uint32_t value;
        struct {
            uint32_t reserved0         : 2;
            uint32_t Base_Address_Low  : 30;
        } bits;
    } reg1;

    union {
        uint32_t value;
        struct {
            uint32_t Buffer_Length     : 32;
        } bits;
    } reg2;

    union {
        uint32_t value;
        struct {
            uint32_t Lock_Acq_ID       : 5;
            uint32_t Lock_Acq_Enable   : 1;
            uint32_t Burst_Length      : 2;
            uint32_t D1_Wrap           : 12;
            uint32_t D2_Wrap           : 12;
        } bits;
    } reg3;

    union {
        uint32_t value;
        struct {
            uint32_t D0_Wrap           : 12;
            uint32_t Lock_Rel_Value    : 7;
            uint32_t Lock_Rel_ID       : 5;
            uint32_t Lock_Acq_Value    : 7;
            uint32_t reserved0         : 1;
        } bits;
    } reg4;

    union {
        uint32_t value;
        struct {
            uint32_t Enable_Packet     : 1;
            uint32_t Next_BD           : 4;
            uint32_t Use_Next_BD       : 1;
            uint32_t AxQoS             : 4;
            uint32_t D1_Stepsize       : 22;
        } bits;
    } reg5;

    union {
        uint32_t value;
        struct {
            uint32_t Enable_Compression: 1;
            uint32_t TLAST_Suppress    : 1;
            uint32_t Iteration_Current : 6;
            uint32_t Data_Reuse        : 2;
            uint32_t D2_Stepsize       : 22;
        } bits;
    } reg6;

    union {
        uint32_t value;
        struct {
            uint32_t AxCache           : 4;
            uint32_t Iteration_Wrap    : 6;
            uint32_t Iteration_Stepsize: 22;
        } bits;
    } reg7;

    union {
        uint32_t value;
        struct {
            uint32_t D3_Stepsize       : 22;
            uint32_t IO_Coherence      : 1;
            uint32_t Out_Of_Order_BD_ID: 4;
            uint32_t Packet_ID         : 5;
        } bits;
    } reg8;
} DMA_SHIM_BD_t;

typedef union {
    uint32_t value;
    struct {
        uint32_t Start_BD_ID         : 4;  // Bits 0-3
        uint32_t reserved0           : 12; // Bits 4-15 (not used)
        uint32_t Repeat_Count        : 12; // Bits 16-27
        uint32_t reserved1           : 3;  // Bits 28-30 (not used)
        uint32_t Enable_Token_Issue  : 1;  // Bit 31
    } bits;
} DMA_Task_Queue_t;

typedef union {
    uint32_t value;
    struct {
        uint32_t Configuration  : 7;  // Bits 0-6
        uint32_t Drop_Header    : 1;  // Bit 7
        uint32_t reserved0      : 22; // Bits 8-29
        uint32_t Packet_Enable  : 1;  // Bit 30
        uint32_t Manager_Enable : 1;  // Bit 31
    } bits;
} SSwitch_Mgr_Port_Config_t;

typedef union {
    uint32_t value;
    struct {
        uint32_t reserved0           : 30; // Bits 0-29
        uint32_t Packet_Enable       : 1;  // Bit 30
        uint32_t Subordinate_Enable  : 1;  // Bit 31
    } bits;
} SSwitch_Sub_Port_Config_t;

#endif
