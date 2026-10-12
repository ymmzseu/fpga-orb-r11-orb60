# 125 MHz 有界对照实验：未通过

两个核均从配对源码重新按 8 ns 目标完成 HLS、精确 C/RTL co-sim 和 II/64-bit 接口检查，随后对 xc7z015clg485-2 的完整顶层进行实际布局布线。

| 阶段 | WNS/ns | TNS/ns | 失败端点 | WHS/ns | LUT |
|---|---:|---:|---:|---:|---:|
| 8 ns HLS 完整路由 | -0.341 | -6.608 | 56 | +0.050 | 27817 |
| AggressiveExplore | -0.039 | -0.467 | 17 | +0.050 | 27825 |
| Explore | -0.034 | -0.187 | 10 | +0.050 | 27826 |

最终 FF 29,048、BRAM tile 68/95、DSP 40/160；51,283 个可路由网络全部布通，无路由错误，12 类 check_timing 计数均为零，DRC 无 Error/Critical Warning。这些检查不能抵消负 WNS，125 MHz 仍未签核。

最后一轮只改善 5 ps，有界物理实验到此结束。热点仍是候选筛选器best_score 读选择、比较到另一分数寄存器 CE 的反馈路径。后续提频需另做 HLS 结构优化并重跑功能、协议和完整路由门禁。

本 ZIP 的可交付 bitstream 仍是原已通过的 100 MHz 版本，WNS +0.238 ns、WHS +0.035 ns，bit 和冻结配对源码字节不变。PS 工作按用户要求暂缓，没有配套新 ELF/XSA，也不能使用旧三核 ELF 配置新两核 bit。125 MHz 整帧和板上 FPS 未实测。

证据见 ../evidence/125mhz/；复现入口为 scripts/frequency_gate.ps1 和 scripts/post_route_physopt.ps1。各 job 下保留当次输入、真实报告和 FAILED 状态。
