#! /bin/bash

source /tools/Xilinx/Vitis/2023.1/settings64.sh
source /opt/xilinx/xrt/setup.sh
export PLATFORM_REPO_PATHS=/opt/xilinx/platforms/xilinx_u50_gen3x16_xdma_5_202210_1/
export LIBRARY_PATH=/usr/lib/x86_64-linux-gnu
./app.exe vadd.xclbin
