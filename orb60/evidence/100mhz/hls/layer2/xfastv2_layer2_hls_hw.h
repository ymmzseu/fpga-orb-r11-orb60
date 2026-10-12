// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
// control
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read/COR)
//        bit 7  - auto_restart (Read/Write)
//        bit 9  - interrupt (Read)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0 - enable ap_done interrupt (Read/Write)
//        bit 1 - enable ap_ready interrupt (Read/Write)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0 - ap_done (Read/TOW)
//        bit 1 - ap_ready (Read/TOW)
//        others - reserved
// 0x10 : Data signal of ap_return
//        bit 31~0 - ap_return[31:0] (Read)
// 0x18 : Data signal of image_r
//        bit 31~0 - image_r[31:0] (Read/Write)
// 0x1c : reserved
// 0x20 : Data signal of blurred
//        bit 31~0 - blurred[31:0] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of resized
//        bit 31~0 - resized[31:0] (Read/Write)
// 0x2c : reserved
// 0x30 : Data signal of selected
//        bit 31~0 - selected[31:0] (Read/Write)
// 0x34 : reserved
// 0x38 : Data signal of result
//        bit 31~0 - result[31:0] (Read/Write)
// 0x3c : reserved
// 0x40 : Data signal of stats
//        bit 31~0 - stats[31:0] (Read/Write)
// 0x44 : reserved
// 0x48 : Data signal of level
//        bit 31~0 - level[31:0] (Read/Write)
// 0x4c : reserved
// 0x50 : Data signal of budget
//        bit 31~0 - budget[31:0] (Read/Write)
// 0x54 : reserved
// 0x58 : Data signal of high_threshold
//        bit 31~0 - high_threshold[31:0] (Read/Write)
// 0x5c : reserved
// 0x60 : Data signal of low_threshold
//        bit 31~0 - low_threshold[31:0] (Read/Write)
// 0x64 : reserved
// 0x68 : Data signal of dual_threshold
//        bit 31~0 - dual_threshold[31:0] (Read/Write)
// 0x6c : reserved
// 0x70 : Data signal of selected_capacity
//        bit 31~0 - selected_capacity[31:0] (Read/Write)
// 0x74 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_AP_CTRL                0x00
#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_GIE                    0x04
#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_IER                    0x08
#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_ISR                    0x0c
#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_AP_RETURN              0x10
#define XFASTV2_LAYER2_HLS_CONTROL_BITS_AP_RETURN              32
#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_IMAGE_R_DATA           0x18
#define XFASTV2_LAYER2_HLS_CONTROL_BITS_IMAGE_R_DATA           32
#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_BLURRED_DATA           0x20
#define XFASTV2_LAYER2_HLS_CONTROL_BITS_BLURRED_DATA           32
#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_RESIZED_DATA           0x28
#define XFASTV2_LAYER2_HLS_CONTROL_BITS_RESIZED_DATA           32
#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_SELECTED_DATA          0x30
#define XFASTV2_LAYER2_HLS_CONTROL_BITS_SELECTED_DATA          32
#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_RESULT_DATA            0x38
#define XFASTV2_LAYER2_HLS_CONTROL_BITS_RESULT_DATA            32
#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_STATS_DATA             0x40
#define XFASTV2_LAYER2_HLS_CONTROL_BITS_STATS_DATA             32
#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_LEVEL_DATA             0x48
#define XFASTV2_LAYER2_HLS_CONTROL_BITS_LEVEL_DATA             32
#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_BUDGET_DATA            0x50
#define XFASTV2_LAYER2_HLS_CONTROL_BITS_BUDGET_DATA            32
#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_HIGH_THRESHOLD_DATA    0x58
#define XFASTV2_LAYER2_HLS_CONTROL_BITS_HIGH_THRESHOLD_DATA    32
#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_LOW_THRESHOLD_DATA     0x60
#define XFASTV2_LAYER2_HLS_CONTROL_BITS_LOW_THRESHOLD_DATA     32
#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_DUAL_THRESHOLD_DATA    0x68
#define XFASTV2_LAYER2_HLS_CONTROL_BITS_DUAL_THRESHOLD_DATA    32
#define XFASTV2_LAYER2_HLS_CONTROL_ADDR_SELECTED_CAPACITY_DATA 0x70
#define XFASTV2_LAYER2_HLS_CONTROL_BITS_SELECTED_CAPACITY_DATA 32

