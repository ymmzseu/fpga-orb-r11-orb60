# FPGA ORB R11：源码与配对固件

此仓库采用直接公开源码的方式，不再上传完整交接 ZIP。
源码、构建脚本、测试代码与说明可在线浏览；已验收的配对固件在
[Release](https://github.com/sunsn061128/fpga-orb-r11/releases/tag/r11-handoff-20261011)
单独下载。完整测试素材、SDK/BSP、生成工程和全部历史 job 仍保存在本地交接包。

## 目录与版本边界

| 路径 | 内容与边界 |
|---|---|
| `FPGA_ORB_HLS/` | 作者源码、HLS/PS/Vivado 脚本及测试；含旧实现和后续开发快照 |
| `FPGA_ORB_HLS/fastv2/next/` | 三核 mixed-PPC 实现及 PS 后端，开发快照含未启用的缓存实验 |
| `FPGA_ORB_SHARED_v1.0/` | CPU 权威参考源码、规范和原许可证；不含完整 vectors |
| `frozen/R11_DEFAULT/inputs/` | 已交付默认 R11 ELF 的冻结编译源；以此为固件源码依据 |
| `evidence/` | 默认 R11 主机记录、源码/产物清单及复用 R8 路由报告 |
| `docs/` | 构建前提、固件下载说明、公开文件溯源清单 |

开发快照不等于默认 ELF 的全部冻结编译源，不能把新缓存实验称为已上板版本。
源码复制保留字节和原目录关系；只新增公开分发说明，不修改算法。

## 已归档效果

- ACZ7015，`xc7z015clg485-2`，`next_system_wrapper`，PL 100 MHz。
- R11 默认 ELF 复用已验收 R8 bit/HDF；不是本次重新综合或上板。
- 最新 quiet 复测：Verified Pipeline 100/100 CPU 精确 PASS，摘要 `a1967dad68429025`。
- Production 固定 960×720 输入、八层、100 帧：平均 30.862 ms，最大 30.874 ms，约 32.402 FPS；终态是 `COMPLETE_UNVERIFIED`，不是逐帧精确 PASS。
- 路由 WNS +0.177 ns、WHS +0.051 ns、TNS/THS 0；LUT 20452、FF 23621、BRAM36 tile 61、DSP 16；仍有 44 条 DRC Warning。
- 已有真实 RTL：FAST 11、resize 13、Gaussian 14 事务通过；完整仿真 job 留在本地归档，不随源码仓库上传。

固定图短组结果不代表变化视频长期稳定 30 FPS、60 FPS、视频解码或完整 SLAM。
详细历史及失败/恢复边界保留在 next 目录的中文报告中。

## 使用

建议克隆到短路径，例如 `C:/h/orb`。版本保持 Vivado HLS/Vivado/SDK 2018.3，
主机 MinGW 14.2.0 与 CMake/Ninja。见 [构建前提](docs/BUILD_PUBLIC_ZH.md)
和 [固件说明](docs/FIRMWARE_ZH.md)。

没有完整外部测试输入时，不能宣称执行了全套回归。本文不授权自动 JTAG、
下载、复位或 Flash/SD/eFUSE 写入。

## 许可

CPU 参考保留 `FPGA_ORB_SHARED_v1.0/LICENSE`（GPL-3.0-or-later）及
`NOTICE.md`；`constants.hpp` 中保留原 GPL/BSD 来源声明。此整理不为全部
内容重新授予 MIT 等统一许可；使用和再分发须遵守各文件原许可。
第三方 SDK、手册和完整素材不在本源码仓库中。

## 本分支新增：ORB60 100 MHz 开发版本

新增共享读入的 2PPC 图像引擎和精确 A32 PL 描述子，使用 Vivado/Vitis HLS 2024.2。源码、冻结编译输入、完整路由和双核整帧仿真证据见 [ORB60 说明](orb60/README_ZH.md)。生成的 100 MHz bitstream 在 `orb60/artifacts/100mhz/`。当前 PS 固件未适配新控制接口；该开发 bitstream 不与历史 R11 ELF 配对。原有 R11 源码、默认固件说明及历史验收内容保留。
