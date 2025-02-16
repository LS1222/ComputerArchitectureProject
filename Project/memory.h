#ifndef __MEMORY_H__
#define __MEMORY_H__

/*===================================
|           INCLUDES                |
====================================*/
#include <stdlib.h>
#include "global.h"

/*===================================
|           DEFINES                 |
====================================*/
#define DATA_WIDTH 32

// Registers.
#define REGISTER_PC_WIDTH 10
#define REGISTER_DEFAULT 0

// Instruction SRAM.
#define SRAM_SIZE (1 << REGISTER_PC_WIDTH)

// Cache.
#define CACHE_BLOCK_SIZE 4
#define CACHE_BLOCKS 64
#define OFFSET_MASK 0x000C0000
#define INDEX_MASK  0x0003F000
#define TAG_MASK    0x00000FFF

// RAM.
#define ADDR_WIDTH 20
#define MAX_MEM_LINE_LENGTH 8
#define RAM_SIZE (1 << ADDR_WIDTH)

/*===================================
|            ENUMS                  |
====================================*/
typedef enum {
	REGISTER_ZERO,           // $zero
	REGISTER_IMMIDIATE,      // $imm
	REGISTER_RESULT,         // $v0
	REGISTER_ARGUMENT_0,     // $a0
	REGISTER_ARGUMENT_1,     // $a1
	REGISTER_ARGUMENT_2,     // $a2
	REGISTER_ARGUMENT_3,     // $a3
	REGISTER_TEMPORARY_0,    // $t0
	REGISTER_TEMPORARY_1,    // $t1
	REGISTER_TEMPORARY_2,    // $t2
	REGISTER_SAVED_0,        // $s0
	REGISTER_SAVED_1,        // $s1
	REGISTER_SAVED_2,        // $s2
	REGISTER_GLOBAL_POINTER, // $gp
	REGISTER_STACK_POINTER,  // $sp
	REGISTER_RETURN_ADDRESS, // $ra
	NUM_OF_REGISTERS
} Registers;

typedef enum {
	CACHE_HIT,
	CACHE_MISS,
	CACHE_DIRTY,
	CACHE_FETCH,
	CACHE_UPDATE,
	NUM_OF_CACHE_RESULTS
} CacheResults;

/*===================================
|            STRUCTS                |
====================================*/
typedef struct {
	unsigned int imm    : 12;
	unsigned int rt     : 4;
	unsigned int rs     : 4;
	unsigned int rd     : 4;
	unsigned int opcode : 8;
} Register;

typedef struct {
	unsigned int value : DATA_WIDTH;
} WORD;

typedef struct {
	WORD data[CACHE_BLOCK_SIZE];
} CacheLine;

typedef struct {
	unsigned int tag  : 12;
	unsigned int MESI : 2;
} TSRAMLine;

typedef struct {
	CacheLine    dsram[CACHE_BLOCKS];
	TSRAMLine    tsram[CACHE_BLOCKS];
	CacheResults status;
	unsigned int base_address : ADDR_WIDTH;
} Cache;

typedef struct {
	WORD mem[RAM_SIZE];
} SimulatorMemory;

#endif // __MEMORY_H__