# FPGA ORB R11 / ORB60：源码、100 MHz bitstream 与路由报告

本仓库保存 R11 的独立 ORB60 开发版本：**960×720、8 层金字塔、2PPC 图像引擎、精确 A32 PL 描述子**，器件 `xc7z015clg485-2`，工具链 Vivado / Vitis HLS 2024.2。像素、选点顺序、方向和 256-bit BRIEF 保持原算法逐字节一致。

当前通过的硬件版本为 **100 MHz**。实际 125 MHz 对照仍有 setup 违例；4PPC、8 bank 筛选器及描述子跨点流水属于下一阶段开发计划，尚未作为通过版本交付。PS 固件暂未迁移，新 bitstream 与旧 R11 三核 ELF 不配对。

## 源码与 bitstream

- [ORB60 源码](orb60/optimized/FPGA_ORB_HLS/fastv2/next/src/)、[完整架构与复现说明](orb60/README_ZH.md)。
- [100 MHz bitstream](orb60/artifacts/100mhz/orb60_100mhz.bit)、[产物与检查点来源](orb60/artifacts/100mhz/artifact_manifest.json)。
- [100 MHz 冻结编译输入](orb60/frozen/100mhz/)，作为 bitstream 两个 HLS 核的配对源码依据。
- [构建和验证入口](orb60/scripts/)、[独立 golden 与 RTL 测试](orb60/tests/)、[AMD IP 顶层](orb60/vivado/)。

Bitstream SHA256：`a80fe3fdff930a3266b2d8701a9dfbc36319010e56744ba0d20245eda7c1303d`。

## 当前完整顶层资源与时序

以下包含两个 HLS 核及 PS7 / AXI / 时钟 / 复位基础设施，来自 **完整 routed top**。

| 项目 | 100 MHz 已通过版本 | 125 MHz 最终失败对照 |
|---|---:|---:|
| LUT | 27,033 / 46,200，58.51% | 27,826，60.23% |
| FF | 27,169 / 92,400，29.40% | 29,048，31.44% |
| BRAM tile | 68 / 95，71.58% | 68 / 95，71.58% |
| DSP | 40 / 160，25.00% | 40 / 160，25.00% |
| Slice | 9,633 / 11,550，83.40% | 10,091，87.37% |
| Setup WNS / TNS | **+0.238 ns / 0** | **−0.034 ns / −0.187 ns** |
| Hold WHS / THS | +0.035 ns / 0 | +0.050 ns / 0 |
| Setup 失败端点 | 0 | 10 |

100 MHz 最差路径在 FAST lane1：13 级逻辑，数据延迟 9.522 ns，其中逻辑 3.990 ns、布线 5.532 ns。50,347 个可路由网络全部布通，无路由错误、无未覆盖时序检查，DRC 无 Error / Critical Warning；原厂 Warning 保留在报告中。

- [100 MHz 时序摘要](orb60/evidence/100mhz/route/post_route_timing.rpt)、[最差 setup 路径](orb60/evidence/100mhz/route/post_route_setup_paths.rpt)、[hold 路径](orb60/evidence/100mhz/route/post_route_hold_paths.rpt)。
- [资源利用率](orb60/evidence/100mhz/route/post_route_utilization.rpt)、[分层资源](orb60/evidence/100mhz/route/post_route_hierarchy.rpt)、[拥塞](orb60/evidence/100mhz/route/post_route_congestion.rpt)、[DRC](orb60/evidence/100mhz/route/post_route_drc.rpt)。
- [125 MHz 对照说明](orb60/docs/125MHZ_COMPARISON_ZH.md)、[最终完整报告](orb60/evidence/125mhz/20261012_174304_physopt_125mhz_17bedb6f/reports/)。

125 MHz 从实际 8 ns HLS 重建起步，完整路由 WNS −0.341 ns，经两轮有界物理优化改善到 −0.034 ns，仍未签核。

## 整帧 RTL 仿真性能

真实生成 RTL 在原生 Verilator 中访问同一 DDR 模型。只注入每帧第 0 层输入，描述子使用实际上游输出，并检查三 raw / 两 blur 槽所有权、整帧特征追加、4 KB 边界和最终 AXI 写响应。

| 输入 | 两组 64-bit DDR 服务 / 拍 | 一组服务 / 拍 + 27% 随机停顿 |
|---|---:|---:|
| 普通图片，280 点 | 11.37272 ms / 87.93 FPS | 12.71175 ms / 78.67 FPS |
| 压力图片，942 点 | 13.28620 ms / 75.27 FPS | 15.81607 ms / 63.23 FPS |
| 空图，0 点 | 11.20667 ms / 89.23 FPS | 12.04355 ms / 83.03 FPS |

[逐层周期与流量](orb60/evidence/100mhz/native/performance.json)记录了上述 6 帧、48 次图像核调用、48 次描述子调用和 2,444 条精确特征。图像核 21 个、描述子 38 个 C/RTL 事务通过；定向随机 AXI gate 及整帧独立审计证据均保留在 [evidence/100mhz](orb60/evidence/100mhz/)。

这些是所列 DDR 服务条件下的实际 RTL 周期结果，**尚未上板测量**。真实 DDR 带宽、PS 缓存可见性、视频变化输入的 P99 / 最大耗时和持续 60 FPS 仍需板级验收。报告中的 scope 字段描述报告生成时的门禁进度；累计状态以本说明和各后续 gate 记录为准。

## 复现、历史版本与许可

运行 `pwsh -NoProfile -File tools/verify_orb60.ps1` 可只读核对分发文件及 bitstream 哈希；这不是重新综合或板级测试。源码仓库保留外部测试素材的哈希清单，完整复现需还原 [外部输入](orb60/docs/EXTERNAL_FIXTURES.json)，具体环境和命令见 [ORB60 说明](orb60/README_ZH.md)。

原始 R11 源码和冻结 PS 输入保留在根目录 `FPGA_ORB_HLS/`、`FPGA_ORB_SHARED_v1.0/`、`frozen/`，其 Vivado 2018.3、旧 bit / ELF 和历史板测数据见 [R11 历史说明](docs/R11_ARCHIVE_README.md)。它们与本次 ORB60 版本分开验收。

保留各文件的 GPL-3.0-or-later / BSD 来源及原许可证：[参考 LICENSE](orb60/optimized/FPGA_ORB_SHARED_v1.0/LICENSE)、[NOTICE](orb60/optimized/FPGA_ORB_SHARED_v1.0/NOTICE.md)。发布源码或 bitstream 不授权自动 JTAG 或非易失编程。
