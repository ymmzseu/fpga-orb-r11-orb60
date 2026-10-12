# Copy to local_env.ps1 and override only installed tool paths.
$env:FPGA_VIVADO_ROOT = 'D:\Xilinx\Vivado\2024.2'
$env:FPGA_VITIS_HLS_ROOT = 'D:\Xilinx\Vitis_HLS\2024.2'
# FPGA_BASE_PYTHON: installed CPython 3.12.12; create this checkout's .venv.
