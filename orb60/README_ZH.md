本开发分支当前通过版本为4PPC／100MHz，见[新结果](docs/INTEGRATED4_IO_RESULTS_ZH.md)与仓库根README。以下原说明保留历史2PPC内容。

# ORB60：100 MHz 2PPC + 精确 PL 描述子开发版本

本目录在原 R11 源码之外增加独立的 2024.2 开发实现。固定目标是
`xc7z015clg485-2`、960×720、八层金字塔、100 MHz；不减少层数或特征预算，
保持像素、选点、排序、方向及 256-bit BRIEF 逐字节一致。

本次交付 **HLS 源码、生成器、构建/验证脚本、冻结编译输入、说明、验证证据和
100 MHz bitstream**。源码发布或 bitstream 生成均不等于上板验收。

## 架构与改动

```text
DDR raw -> 一次 64-bit 读入 -> 2PPC 分流
                           +-> FAST / NMS / 候选筛选 -> DDR 16-byte points
                           +-> Gaussian             -> DDR blur
                           +-> Resize               -> DDR next raw

DDR raw + blur + points -> 方向矩 / 原 CORDIC-A32 / BRIEF -> DDR 64-byte features
                         descriptor(n) 与 image(n+1) 重叠
```

- FAST 分数采用平衡比较树和流水边界；候选更新 II=1，真实处理连续同格依赖。
- 两像素候选保序合并，支持同拍双候选和背压；各算子使用符合其访问方式的局部行缓存。
- 图像搬运与计算通过 DATAFLOW/FIFO 重叠，四个 64-bit `m_axi` 接口直接接 DDR。
- Resize 保持原整数插值、奇数行宽和跨行紧密打包；Gaussian 保持原系数、舍入及 reflect-101。
- 方向和描述子迁入自定义 HLS；raw/blur patch 双缓冲。压缩为八档基础旋转表，精确还原全部 32 档。
- PS7、控制/存储 AXI 仲裁和协议转换、时钟与复位使用 AMD IP；不新增 DMA 或 PL 层序列器。

## 源码、产物与配对依据

| 路径 | 内容 |
|---|---|
| `optimized/FPGA_ORB_HLS/fastv2/next/src/` | `layer2_hls.cpp`、`front2_hls.cpp`、`fastv2_hls.cpp`、`descriptor_hls.cpp` 等作者源码 |
| `optimized/FPGA_ORB_HLS/fastv2/next/include/` | 接口、记录、流和生成的 ROM；修改 ROM 应修改生成器 |
| `optimized/FPGA_ORB_HLS/fastv2/scripts/generate_profile_rom.py` | Resize/前端坐标 ROM 生成器 |
| `scripts/generate_descriptor_rom.py` | 八档基础旋转表生成及全表核对 |
| `optimized/FPGA_ORB_SHARED_v1.0/` | 独立 C++ golden、原常量及许可证 |
| `frozen/100mhz/{layer2,descriptor}/inputs/` | 本次 bitstream 两个核的实际冻结编译输入；优先作为配对源码依据 |
| `scripts/`、`tests/`、`vivado/` | HLS、golden、AXI、Verilator、Vivado 与发布入口 |
| `evidence/100mhz/` | C/RTL、实际 II、AXI 定向测试、完整路由和整帧仿真证据 |
| `artifacts/100mhz/` | `orb60_100mhz.bit`、生成记录、SHA256 和接口头 |

`PUBLICATION_MANIFEST.json` 核对分发文件字节。`EXTERNAL_FIXTURES.json` 列出未公开的
原图、测试二进制及原冻结清单中的少量缓存。GitHub 源码分支保留其哈希；本地完整 ZIP 会还原这些输入，
并用 `ZIP_CONTENT_MANIFEST.json` 核对完整冻结输入。不包含临时工程、工具安装或凭据。

## 实际 100 MHz 路由结果

完整顶层是 `s_wrapper`，包括两个 HLS 核及 AMD PS7/AXI/时钟/复位基础设施。

| 项目 | 完整 routed top |
|---|---:|
| LUT | 27,033 / 46,200，58.51% |
| FF | 27,169 / 92,400，29.40% |
| BRAM tile | 68 / 95，71.58%（59 RAMB36 + 18 RAMB18） |
| DSP | 40 / 160，25% |
| Setup WNS / TNS | +0.238 ns / 0 |
| Hold WHS / THS | +0.035 ns / 0 |

50,347/50,347 可路由网络全部布通；无路由错误、无未覆盖时序检查，DRC 无
Error/Critical Warning。原厂 Warning 保留在报告中。最差 setup 路径在 FAST lane1，
13 级逻辑、9.522 ns（逻辑 3.990 ns，布线 5.532 ns）。未施加 post-route physopt。
这些数值来自完整路由，不能用 HLS/OOC 估计相加替代。

## 正确性和性能证据

保留原 17 项主机基线和 FAST11/Resize13/Gaussian14 RTL 事务；新增共享图像核
21 个、精确描述子 38 个 C/RTL 事务。定向随机 AXI gate 通过描述子六项、图像核三项，
覆盖 4 KB 边界、奇数宽度、容量/错误、随机停顿、延迟写响应、复位恢复及实际 ap_done 的响应屏障。

双核原生 Verilator 使用共同 DDR 地址空间，只注入每帧 layer0；描述子读取实际
上游输出。三 raw 槽、两 blur 槽按所有权复用，每层按顺序追加到容量 1000 的整帧缓冲。
两个 DDR 配置共六帧、48 个图像核调用、48 个描述子调用和 2444 条完整特征全部精确。

| 输入 | 每拍最多两组 64-bit DDR 服务 | 每拍最多一组，附加 27% 随机停顿 |
|---|---:|---:|
| 固定普通图，280 点 | 11.37272 ms / 87.93 FPS | 12.71175 ms / 78.67 FPS |
| 真缩放噪声金字塔，942 点 | 13.28620 ms / 75.27 FPS | 15.81607 ms / 63.23 FPS |
| 空图，0 点 | 11.20667 ms / 89.23 FPS | 12.04355 ms / 83.03 FPS |

以上按 100 MHz 换算，包含 AXI-Lite 配置、完整 PL 描述子和最终写响应。
两个核在四组 HP 端口配对上争用同一 DDR；模型还使用首读 0～5 拍、写响应 2～31 拍延迟。
总 DDR R+W 分别为 6,544,712 / 8,462,552 / 5,731,128 bytes/frame。

这是模型假设下的实际 RTL 周期结果，尚未测量真实 AMD interconnect/DDR PHY、PS 缓存
可见性和板上视频吞吐。三个输入、两个种子不能证明代表性变化视频的 P99/max。
受限模型的压力图距 16.67 ms 剩约 0.85 ms，仍未达到 14 ms 工程目标。

## 接口与 PS 边界

控制地址：图像引擎 `0x43C00000`，描述子 `0x43C10000`；均为 32-bit DDR 地址、
64-bit AXI 数据。寄存器定义见 `evidence/100mhz/hls/*/xfastv2_*_hw.h`，不要硬猜偏移。
points 为 16 字节，summary/统计为 64 字节，features 为 64 字节，描述子部分为 256 bit。
每层预算依次为 217、181、151、126、105、87、73、60。

**PS 固件尚未迁移。这个新 bitstream 与历史 R11 ELF 的三核控制接口不配对；
不能把历史 ELF 当成本版本的新固件。** 当前没有新配对 ELF，也没有新上板测试。
后续需要 PS 调度、缓存可见性处理、配对 XSA/ELF 和实体板单独验收。

## 重建与复核

使用 Vivado/Vitis HLS 2024.2、项目 CPython 3.12.12、CMake/Ninja、发现于 PATH 的
GNU C/C++ 编译器；大批量使用原生 Verilator，随机 AXI gate 使用 ModelSim+cocotb。
本机设置放到根 `scripts/local_env.ps1`，不要修改源码中的用户路径或全局环境。

```powershell
# 完整 ZIP 已提供所需原图/二进制；源码分支须按 EXTERNAL_FIXTURES.json 还原。
# 先确认 py -3.12 对应 CPython 3.12.12。
py -3.12 -m venv .venv
./.venv/Scripts/python.exe -m pip install -r ./orb60/requirements.txt
. ./scripts/project_env.ps1
./orb60/scripts/host.ps1 -Variant optimized
./orb60/scripts/hls.ps1 -Variant optimized -Kernel layer2 -Period 10 -Cosim `
  -SourceJob ./orb60/frozen/100mhz/layer2

# 精确描述子 vectors 由独立 Python 从真正八层输入生成，随后运行 descriptor HLS。
# 具体参数与下一层门禁见 scripts/hls.ps1、tests/prepare_descriptor.py、WORKFLOW.md。
./tools/verify_orb60.ps1
```

完整重建需重新生成 HLS IP，再运行 `scripts/orb_timing.ps1`；现有 100 MHz bitstream
由已经验收的 routed DCP 生成，检查点 SHA256 在 artifact manifest 中。
125 MHz 是另外的 8 ns 有界实验，不能用本 10 ns IP 或旧周期直接声称 125 MHz FPS。

## 许可

原参考的 GPL-3.0-or-later 和各文件 GPL/BSD 来源声明保留；见
`optimized/FPGA_ORB_SHARED_v1.0/LICENSE`、`NOTICE.md`、`constants.hpp`。
此分发不为所有原始/派生文件统一重新授予其他许可。

## 125 MHz 对照结果

实际 8 ns HLS 重建和两轮物理优化后，最终 WNS −0.034 ns、WHS +0.050 ns，仍未通过 setup。100 MHz bit 与配对源码保持不变，PS 工作暂缓。完整记录见 [125 MHz 对照说明](docs/125MHZ_COMPARISON_ZH.md)。
