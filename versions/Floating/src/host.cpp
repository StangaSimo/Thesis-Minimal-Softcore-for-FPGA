#include "xcl2.hpp"
#include <vector>

#define DATA_SIZE 256

extern "C"
{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

	struct Registers
	{
		float r[32];
		int16_t im; /* for imm instruction */
		bool c : 1;
		bool i : 1;
		int32_t pc;
	};

	struct Memory
	{
		int8_t instr[32][4];
		float data[1024];
		int32_t size;
	};

	struct Registers *inizialize_registers(struct Registers *reg)
	{
		for (int i = 0; i < 32; i++)
		{
			reg->r[i] = 0;
		}
		reg->c = false;
		reg->i = false;
		reg->pc = 0;
		reg->im = 0;
		return reg;
	}

	int8_t **get_instructions_from_file(char const *file_name, int32_t *size)
	{
		FILE *file = fopen(file_name, "rb");
		if (file == NULL)
		{
			fprintf(stderr, "Error: cannot open the file '%s'.\n", file_name);
			exit(1);
		}

		int8_t **instr = (int8_t **)malloc(sizeof(int8_t *));

		fseek(file, 0, SEEK_SET);
		while (true)
		{
			int8_t *buffer = (int8_t *)malloc(sizeof(int8_t) * 4);
			int num_bytes = fread(buffer, sizeof(int8_t), 4, file);
			if (num_bytes > 0)
			{
				*size = *size + 1;
				instr = (int8_t **)realloc(instr, *size * sizeof(int8_t *));
				instr[*size - 1] = buffer;
			}
			else
			{
				break;
			}
		}
		fclose(file);
		return instr;
	}
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
		return EXIT_FAILURE;
	}

	std::string binaryFile = argv[1];

	cl_int err;
	cl::CommandQueue q;
	cl::Context context;
	cl::Kernel krnl_vadd;
	auto out_size = 10;

	std::vector<int, aligned_allocator<int>> source_hw_results(out_size);

	// Create the test data and Software Result
	for (int i = 0; i < 10; i++)
	{
		source_hw_results[i] = 0;
	}

	// OPENCL HOST CODE AREA START
	// Create Program and Kernel
	auto devices = xcl::get_xil_devices();

	// read_binary_file() is a utility API which will load the binaryFile
	// and will return the pointer to file buffer.
	auto fileBuf = xcl::read_binary_file(binaryFile);
	cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
	bool valid_device = false;

	for (unsigned int i = 0; i < devices.size(); i++)
	{
		auto device = devices[i];
		// Creating Context and Command Queue for selected Device
		OCL_CHECK(err, context = cl::Context(device, nullptr, nullptr, nullptr, &err));
		OCL_CHECK(err, q = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err));

		std::cout << "Trying to program device[" << i << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
		cl::Program program(context, {device}, bins, nullptr, &err);
		if (err != CL_SUCCESS)
		{
			std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
		}
		else
		{
			std::cout << "Device[" << i << "]: program successful!\n";
			OCL_CHECK(err, krnl_vadd = cl::Kernel(program, "vadd", &err)); // modified
			valid_device = true;
			break; // we break because we found a valid device
		}
	}
	if (!valid_device)
	{
		std::cout << "Failed to program any device found, exit!\n";
		exit(EXIT_FAILURE);
	}

	/****** ME ******/

	char const *file = "add.text";
	// char const* file = "sub.text";
	// char const* file = "cmp.text";
	//char const* file = "data.text";
	// char const* file = "bitop.text";
	//char const* file = "absolute_branch.text";
	// char const* file = "branch.text";

	int32_t instr_size = 0;
	int8_t **prova_vector = get_instructions_from_file(file, &instr_size);
	// std::cout << "-------  " <<  instr_size << " instr_size!\n" << sizeof(int8_t**)*instr_size*sizeof(int8_t*) << " sizeof\n";
	auto mysize = instr_size;

	struct Memory *data = (struct Memory *)malloc(sizeof(struct Memory));

	struct Registers *reg = (struct Registers *)malloc(sizeof(struct Registers));
	reg = inizialize_registers(reg);
	// reg->r[2] = 29;

	// initialize memory
	for (int i = 0; i < 1024; i++)
	{
		data->data[i] = 0;
	}

	uint32_t res = 0;
	for (int i = 0; i < mysize; i++)
	{
		data->instr[i][0] = prova_vector[i][0];
		data->instr[i][1] = prova_vector[i][1];
		data->instr[i][2] = prova_vector[i][2];
		data->instr[i][3] = prova_vector[i][3];
		res = res + data->instr[i][0];
	}

	// Allocate Buffer in Global Memory
	OCL_CHECK(err, cl::Buffer buffer_w(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, out_size, source_hw_results.data(), &err));

	OCL_CHECK(err, cl::Buffer buffer_prova(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(struct Memory), data, &err));
	OCL_CHECK(err, cl::Buffer buffer_reg(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(struct Registers), reg, &err));

	/**** END ME ****/

	// Set the Kernel Arguments
	OCL_CHECK(err, err = krnl_vadd.setArg(0, buffer_prova));
	OCL_CHECK(err, err = krnl_vadd.setArg(1, buffer_reg));
	OCL_CHECK(err, err = krnl_vadd.setArg(2, buffer_w));

	OCL_CHECK(err, err = krnl_vadd.setArg(3, mysize));

	// Copy input data to device global memory
	OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_prova, buffer_reg}, 0 /* 0 means from host*/));

	// Launch the Kernel
	OCL_CHECK(err, err = q.enqueueTask(krnl_vadd));

	// Copy Result from Device Global Memory to Host Local Memory
	OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_w}, CL_MIGRATE_MEM_OBJECT_HOST));
	OCL_CHECK(err, err = q.finish());

	// std::cout << "\n\n MY   RESULT  " << res << " op_1: " << (int16_t)op_1 << " \n\n";
	std::cout << "\n\n FPGA RESULT " << source_hw_results[1] << "\n\n";

	return EXIT_SUCCESS;

	// OPENCL HOST CODE AREA END

	// Compare the results of the Device to the simulation
	// int match = 0;
	// for (int i = 0; i < size; i++) {
	//    if (source_hw_results[i] != source_sw_results[i]) {
	//        std::cout << "Error: Result mismatch" << std::endl;
	//        std::cout << "i = " << i << " Software result = " << source_sw_results[i]
	//            << " Device result = " << source_hw_results[i] << std::endl;
	//        match = 1;
	//        break;
	//    }
	//}

	// std::cout << "TEST " << (match ? "FAILED" : "PASSED") << std::endl;
	// return (match ? EXIT_FAILURE : EXIT_SUCCESS);
}
