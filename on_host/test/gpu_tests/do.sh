/tools/Xilinx/Vivado/2023.1/gnu/microblaze/lin/bin/mb-gcc -o test.o -c main.s
/tools/Xilinx/Vivado/2023.1/gnu/microblaze/lin/bin/mb-objdump -dS test.o > gpu.txt
objcopy -j .text -O binary -I elf32-little test.o gpu.text
