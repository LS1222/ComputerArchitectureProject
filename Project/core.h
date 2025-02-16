#ifndef __CORE_H__
#define __CORE_H__

/*===================================
|           INCLUDES                |
====================================*/
#include <string.h>
#include "global.h"
#include "memory.h"
#include "bus.h"

/*===================================
|           DEFINES                 |
====================================*/
#define OPCODE_MASK 0xFF000000
#define RD_MASK     0x00F00000
#define RS_MASK     0x000F0000
#define RT_MASK     0x0000F000
#define IMM_MASK    0x00000FFF

/*===================================
|           ENUMS                   |
====================================*/
typedef enum {
	FETCH,
	DECODE,
	EXECUTE,
	MEMORY,
	WRITEBACK,
	NUM_OF_STAGES
} CoreStages;

/*===================================
|            STRUCTS                |
====================================*/
typedef struct {
	unsigned int cycles;
	unsigned int instructions;
	unsigned int cache_read_hit;
	unsigned int cache_write_hit;
	unsigned int cache_read_miss;
	unsigned int cache_write_miss;
	unsigned int decode_stall;
	unsigned int mem_stall;
} CoreStats;

typedef struct {
	Register IR;
	unsigned int PC  : REGISTER_PC_WIDTH;
	unsigned int NPC : REGISTER_PC_WIDTH;
} FetchSignals;

typedef struct {
	Register IR;
	unsigned int PC        : REGISTER_PC_WIDTH;
	unsigned int NPC       : REGISTER_PC_WIDTH;
	unsigned int A         : DATA_WIDTH;
	unsigned int B         : DATA_WIDTH;
	signed int   imm       : 12;
	unsigned int ALUOutput : DATA_WIDTH;
	bool cond;
} DecodeSignals;

typedef struct {
	Register IR;
	unsigned int PC        : REGISTER_PC_WIDTH;
	unsigned int NPC       : REGISTER_PC_WIDTH;
	unsigned int A         : DATA_WIDTH;
	unsigned int B         : DATA_WIDTH;
	signed int   imm       : 12;
	unsigned int ALUOutput : DATA_WIDTH;
	bool cond;
} ExecuteSignals;

typedef struct {
	Register IR;
	unsigned int PC        : REGISTER_PC_WIDTH;
	unsigned int NPC       : REGISTER_PC_WIDTH;
	unsigned int A         : DATA_WIDTH;
	unsigned int B         : DATA_WIDTH;
	signed int   imm       : 12;
	unsigned int ALUOutput : DATA_WIDTH;
	bool cond;
	WORD LMD;
} MemorySignals;

typedef struct {
	ErrorCode     status;
	CoreFiles     files;
	CoreStats     stats;
	SimulatorBus* bus;

	// Components of the core.
	WORD     imem[SRAM_SIZE];
	Cache    cache;
	Register regs    [NUM_OF_REGISTERS];
	Register regs_new[NUM_OF_REGISTERS];
	unsigned int PC : REGISTER_PC_WIDTH;

	// Pipeline signals.
	bool           active;
	FetchSignals   IF_ID;
	DecodeSignals  ID_EX;
	ExecuteSignals EX_MEM;
	MemorySignals  MEM_WB;

	// Help variables.
	unsigned int id;
	bool         stall_decode;
	bool         stall_memory;
} SimulatorCore;

/*===================================
|        FUNCTIONS PROTOTYPE        |
====================================*/
void CoreCycle (SimulatorCore* core);
void Fetch     (SimulatorCore* core);
void Decode    (SimulatorCore* core);
void Execute   (SimulatorCore* core);
void Memory    (SimulatorCore* core);
void WriteBack (SimulatorCore* core);
void CacheRead (SimulatorCore* core);
void CacheWrite(SimulatorCore* core);
void Snooping  (SimulatorCore* core);

// Define a function signature
typedef void (*stage_function) (SimulatorCore* core);
static const stage_function stage_functions[NUM_OF_STAGES] = { Fetch, Decode, Execute, Memory, WriteBack };

#endif // __CORE_H__