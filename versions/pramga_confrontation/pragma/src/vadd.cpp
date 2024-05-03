
#define TYPE_A 1
#define TYPE_B 0
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "ap_int.h"

extern "C"
{

	struct Registers
	{
		int32_t r[32];
		int16_t im;
		bool c : 1;
		bool i : 1;
		int32_t pc;
	};

	struct Memory
	{
		int8_t instr[32][4];
		int32_t data[1024];
		int32_t size;
	};

	struct Instruction
	{
		int8_t type : 1; /* 1 type A, 0 type B */
		int8_t opcode : 6;
		int8_t rd : 5;
		int8_t ra : 5;
		int8_t rb : 5;
		int32_t im : 32;
	};

	int32_t add_Check_Overflow(int32_t a, int32_t b, bool *c)
	{
		int64_t res = (int64_t)a + (int64_t)b; /* only last 32 bit*/
		if (res > INT32_MAX)
		{
			*c = true;
			res = INT32_MAX;
		}
		else if (res < INT32_MIN) /* underflow */
		{

			*c = true;
			res = INT32_MIN;
		}
		else
		{
			*c = false;
		}

		return (int32_t)res;
	}

	void update_PC(struct Registers *reg, int32_t n, bool delay)
	{
		if (!delay) /* Note: if the instruction is in a delay slot, it shouldn't modify the pc reg*/
			reg->pc = reg->pc + n / 4;
	}

	int8_t conv_reg(int8_t n)
	{
		return n & 0b00011111;
	}

	struct Instruction *parse_instruction(int8_t *instr, int8_t type, struct Instruction *res, int16_t *im)
	{
		if (type) /* Type A */
		{
			res->type = type;
			res->rd = (instr[0] << 3) + ((instr[1] >> 5) & 0b00000111);
			res->ra = instr[1] & 0b00011111;
			res->rb = (instr[2] >> 3) & 0b00011111;
		}
		else /* Type B */
		{
			res->type = type;
			res->rd = ((instr[0] << 3) & 0b00011000) + ((instr[1] >> 5) & 0b00000111);
			res->ra = instr[1] & 0b00011111;
			int16_t n = instr[2];
			n = (n << 8) + (((int16_t)instr[3]) & 0b0000000011111111);

			if (*im) /* imm istruction before */
			{
				res->im = (*im << 16) + ((int32_t)n & 0b00000000000000001111111111111111);
				*im = 0;
			}
			else
				res->im = (int32_t)n;
		}
		return res;
	}

	void run_instruction(int8_t *instruction, struct Memory *data, struct Registers *reg, int8_t instructions[32][4], bool delay)
	{
		struct Instruction instr_data;
		struct Instruction *instr = &instr_data;
		bool carry = false; // carry
		int8_t op_code = (instruction[0] >> 2) & 0b00111111; /* remove last 11 bitfield is not working for the == operand*/
		instr->opcode = (instruction[0] >> 2) & 0b00111111;
		int32_t delayed_instruction, addr;
		int8_t branch_type, is_delayed, is_absolute, is_link;

		switch (op_code)
		{
		case 0x0:
		{ /* ADD 000000 */
			instr = parse_instruction(instruction, TYPE_A, instr, &reg->im);
			reg->r[instr->rd] = add_Check_Overflow(reg->r[instr->ra], reg->r[instr->rb], &carry);
			update_PC(reg, 4, delay);
			break;
		}

		case 0x4:
		{ /* ADDK 000100 */
			instr = parse_instruction(instruction, TYPE_A, instr, &reg->im);
			reg->r[instr->rd] = add_Check_Overflow(reg->r[instr->ra], reg->r[instr->rb], &carry);
			reg->c = carry;
			update_PC(reg, 4, delay);
			break;
		}

		case 0x2:
		{ /* ADDC 000010 */
			instr = parse_instruction(instruction, TYPE_A, instr, &reg->im);
			reg->r[instr->rd] = add_Check_Overflow(reg->r[instr->ra], reg->r[instr->rb] + reg->c, &carry);
			update_PC(reg, 4, delay);
			break;
		}

		case 0x6:
		{ /* ADDCK 000110 */
			instr = parse_instruction(instruction, TYPE_A, instr, &reg->im);
			reg->r[instr->rd] = add_Check_Overflow(reg->r[instr->ra], reg->r[instr->rb] + reg->c, &carry);
			reg->c = carry;
			update_PC(reg, 4, delay);
			break;
		}

		case 0x8:
		{ /* ADDI 001000 */
			instr = parse_instruction(instruction, TYPE_B, instr, &reg->im);
			reg->r[instr->rd] = add_Check_Overflow(instr->im, reg->r[instr->ra], &carry);
			update_PC(reg, 4, delay);
			break;
		}

		case 0xA:
		{ /* ADDIC 001010 */
			instr = parse_instruction(instruction, TYPE_B, instr, &reg->im);
			reg->r[instr->rd] = add_Check_Overflow(instr->im, reg->r[instr->ra] + reg->c, &carry);
			update_PC(reg, 4, delay);
			break;
		}

		case 0xC:
		{ /* ADDIK 001100 */
			instr = parse_instruction(instruction, TYPE_B, instr, &reg->im);
			reg->r[instr->rd] = add_Check_Overflow(instr->im, reg->r[instr->ra], &carry);
			reg->c = carry;
			update_PC(reg, 4, delay);
			break;
		}

		case 0xE:
		{ /* ADDICK 001110 */
			instr = parse_instruction(instruction, TYPE_B, instr, &reg->im);
			reg->r[instr->rd] = add_Check_Overflow(instr->im, reg->r[instr->ra] + reg->c, &carry);
			reg->c = carry;
			update_PC(reg, 4, delay);
			break;
		}
		case 0x2C:
		{ /* IMM 101100 */
			instr = parse_instruction(instruction, TYPE_B, instr, &reg->im);
			reg->im = instr->im;
			update_PC(reg, 4, delay);
			break;
		}

		default:
		{
			/* unknown istruction */
			break;
		}
		}
	}

	void vadd(struct Memory *mem, struct Registers *reg, uint32_t *out, ap_uint<32> my_size)
	{
#pragma HLS INTERFACE m_axi port = mem bundle = gmem
#pragma HLS INTERFACE m_axi port = reg bundle = gmem
#pragma HLS INTERFACE m_axi port = out bundle = gmem
#pragma HLS INTERFACE ap_ctrl_hs port = return

		struct Registers reg_copy;
		struct Memory mem_copy;

#pragma HLS bind_storage variable=reg_copy type=RAM_1P impl=bram
#pragma HLS bind_storage variable=mem_copy type=RAM_1P impl=bram

		for (int i = 0; i < 32; i++)
			reg_copy.r[i] = reg->r[i];

		reg_copy.c = reg->c;
		reg_copy.i = reg->i;
		reg_copy.pc = reg->pc;
		reg_copy.im = reg->im;

		for (int i = 0; i < 1024; i++)
			mem_copy.data[i] = mem->data[i];

		for (int i = 0; i < 32; i++)
			for (int j = 0; j < 4; j++)
				mem_copy.instr[i][j] = mem->instr[i][j];

		struct Registers *reg_copy_pointer = &reg_copy;
		struct Memory *mem_copy_pointer = &mem_copy;

		while (reg_copy_pointer->pc < my_size)
			run_instruction(mem_copy_pointer->instr[reg_copy_pointer->pc], 
												mem_copy_pointer, 
													reg_copy_pointer, 
														mem_copy_pointer->instr, 
															false);

		out[1] = reg_copy_pointer->r[1];
	}
}