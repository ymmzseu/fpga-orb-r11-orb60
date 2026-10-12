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
// 0x18 : Data signal of raw
//        bit 31~0 - raw[31:0] (Read/Write)
// 0x1c : reserved
// 0x20 : Data signal of blurred
//        bit 31~0 - blurred[31:0] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of points
//        bit 31~0 - points[31:0] (Read/Write)
// 0x2c : reserved
// 0x30 : Data signal of features
//        bit 31~0 - features[31:0] (Read/Write)
// 0x34 : reserved
// 0x38 : Data signal of stats
//        bit 31~0 - stats[31:0] (Read/Write)
// 0x3c : reserved
// 0x40 : Data signal of width
//        bit 31~0 - width[31:0] (Read/Write)
// 0x44 : reserved
// 0x48 : Data signal of height
//        bit 31~0 - height[31:0] (Read/Write)
// 0x4c : reserved
// 0x50 : Data signal of count
//        bit 31~0 - count[31:0] (Read/Write)
// 0x54 : reserved
// 0x58 : Data signal of level
//        bit 31~0 - level[31:0] (Read/Write)
// 0x5c : reserved
// 0x60 : Data signal of frame_offset
//        bit 31~0 - frame_offset[31:0] (Read/Write)
// 0x64 : reserved
// 0x68 : Data signal of capacity
//        bit 31~0 - capacity[31:0] (Read/Write)
// 0x6c : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XFASTV2_DESCRIPTOR_HLS_CONTROL_ADDR_AP_CTRL           0x00
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_ADDR_GIE               0x04
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_ADDR_IER               0x08
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_ADDR_ISR               0x0c
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_ADDR_AP_RETURN         0x10
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_BITS_AP_RETURN         32
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_ADDR_RAW_DATA          0x18
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_BITS_RAW_DATA          32
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_ADDR_BLURRED_DATA      0x20
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_BITS_BLURRED_DATA      32
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_ADDR_POINTS_DATA       0x28
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_BITS_POINTS_DATA       32
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_ADDR_FEATURES_DATA     0x30
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_BITS_FEATURES_DATA     32
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_ADDR_STATS_DATA        0x38
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_BITS_STATS_DATA        32
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_ADDR_WIDTH_DATA        0x40
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_BITS_WIDTH_DATA        32
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_ADDR_HEIGHT_DATA       0x48
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_BITS_HEIGHT_DATA       32
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_ADDR_COUNT_DATA        0x50
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_BITS_COUNT_DATA        32
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_ADDR_LEVEL_DATA        0x58
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_BITS_LEVEL_DATA        32
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_ADDR_FRAME_OFFSET_DATA 0x60
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_BITS_FRAME_OFFSET_DATA 32
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_ADDR_CAPACITY_DATA     0x68
#define XFASTV2_DESCRIPTOR_HLS_CONTROL_BITS_CAPACITY_DATA     32

