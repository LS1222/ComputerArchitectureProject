#ifndef __BUS_H__
#define __BUS_H__

/*===================================
|           INCLUDES                |
====================================*/
#include "global.h"
#include "memory.h"

/*===================================
|           DEFINES                 |
====================================*/
#define MEMORY_DELAY 16
#define MEMORY_ID    4

/*===================================
|            ENUMS                  |
====================================*/
typedef enum {
	BUS_NO_COMMAND,
	BUS_READ_COMMAND,
	BUS_READX_COMMAND,
	BUS_FLUSH_COMMAND,
	NUM_OF_BUS_COMMANDS
} BusCommand;

typedef enum {
	INVALID,
	SHARED,
	EXCLUSIVE,
	MODIFIED,
	NUM_OF_MESI_STATES
} MESI;

/*===================================
|            STRUCTS                |
====================================*/
typedef struct {
	unsigned int cycles;
	SimFile      bustrace;

	unsigned int latency;
	unsigned int address : ADDR_WIDTH;
	unsigned int data    : DATA_WIDTH;
	bool         write;

	// Round Robin.
	bool         waiting[NUM_OF_CORES];
	bool         active;
	unsigned int rr_grant;
	unsigned int rr_last;

	// Bus signals.
	unsigned int bus_origid : 3;
	unsigned int bus_cmd    : 2;
	unsigned int bus_addr   : ADDR_WIDTH;
	unsigned int bus_data   : DATA_WIDTH;
	unsigned int bus_shared : 1;
} SimulatorBus;

/*===================================
|        FUNCTIONS PROTOTYPE        |
====================================*/
bool RoundRobin    (SimulatorBus* bus);
void MemoryRequest (SimulatorBus* bus);
void MemoryResponse(SimulatorBus* bus, SimulatorMemory* ram);
void MemoryCycle   (SimulatorBus* bus, SimulatorMemory* ram);

#endif // __BUS_H__