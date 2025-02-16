#ifndef __ISA_H__
#define __ISA_H__

/*===================================
|           INCLUDES                |
====================================*/
#include "global.h"
#include "core.h"

/*===================================
|            ENUMS                  |
====================================*/
typedef enum {
	OPCODE_NOP,
	OPCODE_ADD,
	OPCODE_SUB,
	OPCODE_AND,
	OPCODE_OR,
	OPCODE_XOR,
	OPCODE_MUL,
	OPCODE_SLL,
	OPCODE_SRA,
	OPCODE_SRL,
	OPCODE_BEQ,
	OPCODE_BNE,
	OPCODE_BLT,
	OPCODE_BGT,
	OPCODE_BLE,
	OPCODE_BGE,
	OPCODE_JAL,
	OPCODE_LW,
	OPCODE_SW,
	OPCODE_SC,
	OPCODE_HALT,
	NUM_OF_OPCODES
} OpcodeCommand;

/*===================================
|        FUNCTIONS PROTOTYPE        |
====================================*/
void nop (SimulatorCore* core);
void add (SimulatorCore* core);
void sub (SimulatorCore* core);
void and (SimulatorCore* core);
void or  (SimulatorCore* core);
void xor (SimulatorCore* core);
void mul (SimulatorCore* core);
void sll (SimulatorCore* core);
void sra (SimulatorCore* core);
void srl (SimulatorCore* core);
void beq (SimulatorCore* core);
void bne (SimulatorCore* core);
void blt (SimulatorCore* core);
void bgt (SimulatorCore* core);
void ble (SimulatorCore* core);
void bge (SimulatorCore* core);
void jal (SimulatorCore* core);
void lw  (SimulatorCore* core);
void sw  (SimulatorCore* core);
void halt(SimulatorCore* core);

// Define a function signature
typedef void (*opcode_function) (SimulatorCore* core);
static const opcode_function branch_functions [NUM_OF_OPCODES] = { nop, nop, nop, nop, nop, nop, nop, nop, nop, nop, beq, bne, blt, bgt, ble, bge, jal, nop, nop, nop, nop  };
static const opcode_function execute_functions[NUM_OF_OPCODES] = { nop, add, sub, and, or , xor, mul, sll, sra, srl, nop, nop, nop, nop, nop, nop, nop, lw,   sw, nop, halt };


#endif // __ISA_H__