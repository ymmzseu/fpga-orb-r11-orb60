# Public FPGA repository facts

- Board/device/top: ALINX ACZ7015, xc7z015clg485-2, next_system_wrapper, FCLK0 100 MHz.
- Vivado HLS/Vivado/SDK 2018.3; native host MinGW 14.2.0, CMake/Ninja. Use isolated process profiles; no global PATH edits or WSL.
- frozen/R11_DEFAULT/inputs is authoritative for the published default ELF. FPGA_ORB_HLS is a later authored development snapshot, not identical release compilation inputs.
- Read-only publication check: pwsh -NoProfile -File tools/verify_public.ps1; add -AssetDir to check downloaded paired firmware.
- Host command with external vectors restored: pwsh -NoProfile -File FPGA_ORB_HLS/fastv2/next/scripts/build_and_test.ps1. See docs/BUILD_PUBLIC_ZH.md for fixture and tool-path prerequisites.
- Authored C/C++/Tcl/PowerShell/Python inputs are checked in. Build/job directories, SDK/BSP, HLS RTL/IP, backend_tables.h, full_reference.inc and profile ROM are generated: change generators, not generated outputs.
- Acceptance: CPU exact comparison, FAST11/resize13/Gaussian14 RTL transactions, firmware/source pairing, routed setup/hold and constraint coverage. Production is COMPLETE_UNVERIFIED, never exact PASS.
- Publishing/copying is not a new build or board test. No automatic JTAG/programming or nonvolatile writes.

## ORB60 branch addition

orb60/ is the independent 2024.2, xc7z015clg485-2, s_wrapper development
scope. Its 100 MHz two-core hardware is separate from the historical
R11 three-core bit/ELF. Use orb60/README_ZH.md and scripts/project_env.ps1.
Frozen 100 MHz text inputs and evidence are immutable; modify authored
optimized sources/generators. No new PS ELF or board result is claimed.
