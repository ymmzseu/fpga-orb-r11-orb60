# R11 ORB：4PPC／100MHz整合优化

开发分支`codex/orb60-4ppc`当前交付**通过完整电脑门禁的4PPC／100MHz版本**。固定960×720、8层、xc7z015clg485-2、Vivado/Vitis HLS2024.2；像素、点排序、方向和256bit BRIEF保持逐字节一致。PS、ELF/XSA与板测暂缓；原R11三核ELF不配对本两核bitstream。

默认分支`main`与[原100MHz产物](orb60/artifacts/100mhz/)保留原发布版。本文数据属于新的整合版本，不混用历史报告。

- [本版源码](orb60/optimized/FPGA_ORB_HLS/fastv2/next/src/)、[逐核冻结输入](orb60/frozen/integrated4_100mhz/)。
- [新100MHz bitstream](orb60/artifacts/integrated4_100mhz/orb60_100mhz.bit)、[源码／RTL／帧／检查点配对](orb60/artifacts/integrated4_100mhz/joint_validation.json)。
- [完整时序](orb60/evidence/integrated4_100mhz/route/post_route_timing.rpt)、[资源](orb60/evidence/integrated4_100mhz/route/post_route_utilization.rpt)、[分层资源](orb60/evidence/integrated4_100mhz/route/post_route_hierarchy.rpt)、[200端点／50高扇出诊断](orb60/evidence/integrated4_100mhz/route/post_route_diagnostics/)。
- [12帧周期／AXI／FIFO统计](orb60/evidence/integrated4_100mhz/native/performance.json)、[精确HLS门禁](orb60/evidence/integrated4_100mhz/hls/)、[随机AXI／复位](orb60/evidence/integrated4_100mhz/protocol/)、[四候选长背压单元](orb60/evidence/integrated4_100mhz/merge4/)。
- [优化结果与当前瓶颈](orb60/docs/INTEGRATED4_IO_RESULTS_ZH.md)、[第一版4PPC对照](orb60/docs/INTEGRATED4_RESULTS_ZH.md)、[计划](orb60/docs/NEXT_OPTIMIZATION_PLAN_ZH.md)。

## 完整路由结果

| 资源／时序 | 本版4PPC／100MHz |
|---|---:|
| LUT | 35347／46200，76.51% |
| FF | 32979／92400，35.69% |
| BRAM tile | 74／95，77.89% |
| DSP | 44／160，27.50% |
| Slice | **11405／11550，98.74%** |
| 控制集 | 1104 |
| WNS／TNS | **+0.188ns／0** |
| WHS／THS | **+0.044ns／0** |

Setup/hold、约束覆盖、路由及严格DRC检查通过。最差路径是复位寄存器到BRAM FIFO复位，主要为布线延迟。WNS满足硬门槛，略低于争取的+0.2ns；Slice接近容量上限。新增双块搬运并未减少总Slice，其价值来自实测整帧收益。

## 整帧RTL性能

实际两核RTL共享DDR模型；HP上限800MB/s，聚合预算800／1600MB/s，初始读资格延迟0~50ns／400~600ns，写响应20~310ns。逐字节golden、DDR守卫区、4KB边界、三raw／两blur槽、追加顺序与最终ap_done响应屏障均通过。

| DDR模型 | 普通图280点 | 压力图942点 | 空图 |
|---|---:|---:|---:|
| 1600MB/s、无随机停顿、低延迟 | 8.32935ms／120.06FPS | 9.09975ms／109.89FPS | 8.18185ms |
| 800MB/s、27%随机停顿、低延迟 | 10.69234ms／93.52FPS | **12.59037ms／79.43FPS** | 9.96998ms |
| 1600MB/s、无随机停顿、高延迟 | 8.32430ms／120.13FPS | 9.05809ms／110.40FPS | 8.18601ms |
| 800MB/s、27%随机停顿、高延迟 | 10.50938ms／95.15FPS | 12.40847ms／80.59FPS | 9.78945ms |

四组模型最大耗时12.59037ms，达到14ms工程目标。它们是三种输入、12帧仿真结果，尚不能证明板上P99/max≤16.67ms或持续60FPS。首读较慢的用例偶尔略快是PRNG事件节奏差异，不能推断DDR延迟越高越好。

## 复现与校验

Bit SHA256：`46c36be6a8758926f576fe6761434adddbf646ab2ed745709b1c0513b101abe2`。

```powershell
# 只读校验整个分发包
pwsh -NoProfile -File tools/verify_integrated4.ps1

# 新建本项目CPython3.12.12环境；不要复制旧venv
uv venv .venv --python 3.12.12
uv pip install --python .venv/Scripts/python.exe -r requirements.txt

# 导入独立图像golden，从原constants.hpp重算全部方向／描述子
pwsh -NoProfile -File orb60/scripts/reproduce_integrated4.ps1 -Mode Inputs

# 源码重建全部电脑门禁及配对bit；耗时较长
pwsh -NoProfile -File orb60/scripts/reproduce_integrated4.ps1 -Mode All
```

Inputs入口已在分发目录的新venv通过。各硬件门禁已逐项通过；本轮没有重复执行All串联完整重建。安装路径通过`scripts/project_env.ps1`和忽略的`scripts/local_env.ps1`配置；需要Vivado/Vitis HLS2024.2、ModelSim许可证、MinGW/CMake/Ninja及Verilator。

图像核开关：`LayerPpc4, FastStaged, LayerIoOverlap, LayerBramFifos`；描述子：`DescriptorWidthMode2, DescriptorPrefetch8, DescriptorPatch32`；HLS目标10ns。默认编译参数仍保留旧2PPC行为，必须使用明确开关。外部寄存器映射不变，4PPC统计版本为0x00600003，legacy pairs字段按quad／可变像素包解释。

新125MHz对照已从8ns HLS重新生成并通过C/RTL；其完整路由和归一化DDR测试正在进行，尚未作为可用硬件版本交付。旧125MHz−0.034ns属于历史设计，不能代替本次结果。

原R11历史输入及许可见[历史说明](docs/R11_ARCHIVE_README.md)、[GPL/BSD许可与来源](orb60/optimized/FPGA_ORB_SHARED_v1.0/NOTICE.md)。发布不包含任何JTAG或非易失编程操作。
