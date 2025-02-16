#ifndef __BUS_C__
#define __BUS_C__

#include "bus.h"

bool RoundRobin(SimulatorBus* bus) {
	unsigned int curr = bus->rr_last;

	for (int i = 0; i < NUM_OF_CORES; i++) {
		curr = (curr + 1) % NUM_OF_CORES;
		if (bus->waiting[curr]) {
			bus->rr_grant = curr;
			return TRUE;
		}
	}

	return FALSE;
}

void MemoryRequest(SimulatorBus* bus) {
	bus->latency = MEMORY_DELAY + CACHE_BLOCK_SIZE;
	bus->address = bus->bus_addr;
	bus->data    = bus->bus_data;
	bus->write   = bus->bus_cmd == BUS_READX_COMMAND;
}

void MemoryResponse(SimulatorBus* bus, SimulatorMemory* ram) {
	bus->latency--;

	// First response cycle.
	if (bus->latency == CACHE_BLOCK_SIZE) {
		if (bus->write) {
			ram->mem[bus->address].value = bus->data;
		}
		bus->bus_cmd    = BUS_FLUSH_COMMAND;
		bus->bus_addr   = bus->address;
		bus->bus_data   = ram->mem[bus->address].value;
		bus->bus_shared = FALSE;
		bus->bus_origid = MEMORY_ID;
	}

	// Memory access finished or cache Xfers.
	if (bus->latency == 0 || bus->bus_cmd == BUS_FLUSH_COMMAND) {
		bus->active  = FALSE;
		bus->rr_last = bus->rr_grant;
	}
}

void MemoryCycle(SimulatorBus* bus, SimulatorMemory* ram) {
	if (!bus->active && RoundRobin(bus)) {
		bus->active = TRUE;
	}

	if (bus->active && bus->rr_grant == bus->bus_origid) {
		MemoryRequest(bus);
	}

	// Implement read from memory with 16 cycles of delay and round robin.
	if (bus->active && MEMORY_ID == bus->bus_origid) {
		MemoryResponse(bus, ram);
	}

	if (bus->bus_cmd != BUS_NO_COMMAND) {
		fprintf(bus->bustrace.handle, "%08X %1X %1X %5X %8X %1X\n", bus->cycles++, bus->bus_origid, bus->bus_cmd, bus->bus_addr, bus->bus_data, bus->bus_shared);
	}
}

#endif // __BUS_C__