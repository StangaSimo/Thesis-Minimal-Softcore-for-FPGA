#! /bin/bash
# vitis_analyzer /home/stanganini/hardware/vadd.xclbin.link_summary


./clean.sh
#export XCL_EMULATION_MODE=hw_emu
source /tools/Xilinx/Vitis/2023.1/settings64.sh
source /opt/xilinx/xrt/setup.sh
export PLATFORM_REPO_PATHS=/opt/xilinx/platforms/xilinx_u50_gen3x16_xdma_5_202210_1/
export LIBRARY_PATH=/usr/lib/x86_64-linux-gnu
g++ -g -std=c++17 -Wall -O0 src/host.cpp src/xcl2.cpp -o ./app.exe -I$XILINX_XRT/include/ -L$XILINX_XRT/lib -lxrt_coreutil -pthread -lOpenCL
emconfigutil --platform xilinx_u50_gen3x16_xdma_5_202210_1
v++ -c -t hw --platform xilinx_u50_gen3x16_xdma_5_202210_1 --config conf.cfg -k vadd -I/src src/vadd.cpp -o ./vadd.xo
v++ -l -t hw --platform xilinx_u50_gen3x16_xdma_5_202210_1 --config conf.cfg ./vadd.xo -o ./vadd.xclbin
./app.exe vadd.xclbin
