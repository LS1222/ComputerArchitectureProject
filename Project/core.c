#ifndef __CORE_C__
#define __CORE_C__

/*===================================
|           INCLUDES                |
====================================*/
#include "core.h"
#include "isa.h"

/*===================================
|           FUNCTIONS               |
====================================*/
void CoreCycle(SimulatorCore* core) {

	// Execute pipeline stages.
	for (int stage = WRITEBACK; stage >= FETCH; stage--) {
		stage_functions[stage](core);
	}
	Snooping(core);

	// End cycle.
	core->active >>= 1;
	memcpy(core->regs, core->regs_new, sizeof(Register) * NUM_OF_REGISTERS);

	// Print core trace.
	fprintf(core->files.trace.handle, "%0d ", ++core->stats.cycles);                                     // CYCLES.
	fprintf(core->files.trace.handle, (core->active & 0b10000 ? "%03X " : "--- "), core->PC);            // FETCH.
	fprintf(core->files.trace.handle, (core->active & 0b01000 ? "%03X " : "--- "), core->IF_ID.PC);      // DECODE.
	fprintf(core->files.trace.handle, (core->active & 0b00100 ? "%03X " : "--- "), core->ID_EX.PC);      // EXECUTE.
	fprintf(core->files.trace.handle, (core->active & 0b00010 ? "%03X " : "--- "), core->EX_MEM.PC);     // MEMORY.
	fprintf(core->files.trace.handle, (core->active & 0b00001 ? "%03X " : "--- "), core->MEM_WB.PC);     // WRITEBACK.
	for (int reg = 2; reg < NUM_OF_REGISTERS; reg++) {
		fprintf(core->files.trace.handle, "%08X ", *(int*)&core->regs[reg]);
	}
	fprintf(core->files.trace.handle, "\n");
	fflush(core->files.trace.handle);
}

void Fetch(SimulatorCore* core) {
	if (core->stall_decode || core->stall_memory) {
		return;
	}

	// Fetch instruction.
	if (core->PC < core->files.imem.w_len) {
		memcpy(&core->IF_ID.IR, &core->imem[core->PC], sizeof(Register));
		core->stats.instructions++;
		core->active |= (1 << NUM_OF_STAGES);
	}

	// Next instruction.
	core->IF_ID.PC  = core->PC;
	core->IF_ID.NPC = core->PC + 1;
	if (core->ID_EX.cond && core->ID_EX.IR.opcode >= OPCODE_BEQ && core->ID_EX.IR.opcode <= OPCODE_JAL) {
		core->IF_ID.NPC = core->ID_EX.ALUOutput;
	}

	// Advance the PC register.
	core->PC = core->IF_ID.NPC;
}

void Decode(SimulatorCore* core) {
	if (core->stall_memory) {
		return;
	}

	core->ID_EX.A   = *(int*)&core->regs[core->IF_ID.IR.rs];
	core->ID_EX.B   = *(int*)&core->regs[core->IF_ID.IR.rt];
	core->ID_EX.PC  = core->IF_ID.PC;
	core->ID_EX.NPC = core->IF_ID.NPC;
	core->ID_EX.IR  = core->IF_ID.IR;
	core->ID_EX.imm = core->IF_ID.IR.imm; // TODO Fix sign extension.

	// Data Hazards.
	//if (core->IF_ID.IR.opcode < OPCODE_BEQ && (core->ID_EX.IR.rd == core->IF_ID.IR.rs || core->ID_EX.IR.rd == core->IF_ID.IR.rt)) {
	//	core->ID_EX.IR.opcode = OPCODE_NOP;
	//}

	//if (core->IF_ID.IR.opcode < OPCODE_BEQ && (core->ID_EX.IR.rt == core->IF_ID.IR.rs || core->ID_EX.IR.rt == core->IF_ID.IR.rt)) {
	//	core->ID_EX.IR.opcode = OPCODE_NOP;
	//}

	// Branch resolution.
	branch_functions[core->ID_EX.IR.opcode](core);
}

void Execute(SimulatorCore* core) {
	if (core->stall_memory) {
		return;
	}

	core->EX_MEM.IR  = core->ID_EX.IR;
	core->EX_MEM.PC  = core->ID_EX.PC;
	core->EX_MEM.NPC = core->ID_EX.NPC;
	execute_functions[core->ID_EX.IR.opcode](core);
}

void Memory(SimulatorCore* core) {
	core->MEM_WB.PC        = core->EX_MEM.PC;
	core->MEM_WB.NPC       = core->EX_MEM.NPC;
	core->MEM_WB.IR        = core->EX_MEM.IR;
	core->MEM_WB.ALUOutput = core->EX_MEM.ALUOutput;

	// Cache read.
	if (OPCODE_LW == core->MEM_WB.IR.opcode) {
		CacheRead(core);
	}

	// Cache write.
	if (OPCODE_SW == core->MEM_WB.IR.opcode) {
		CacheWrite(core);
	}

	// Wait for cache HIT.
	core->stall_memory = (CACHE_HIT != core->cache.status);
	if (core->stall_memory) core->stats.mem_stall++;
}

void WriteBack(SimulatorCore* core) {

	// Load operation.
	if (core->MEM_WB.IR.opcode == OPCODE_LW) {
		core->regs_new[core->MEM_WB.IR.rt] = *(Register*)&core->MEM_WB.LMD;
	}

	// Arithmetic operations.
	if (core->MEM_WB.IR.opcode < OPCODE_BEQ) {
		core->regs_new[core->MEM_WB.IR.rd].opcode = (core->MEM_WB.ALUOutput & OPCODE_MASK) >> 24;
		core->regs_new[core->MEM_WB.IR.rd].rd     = (core->MEM_WB.ALUOutput & RD_MASK)     >> 20;
		core->regs_new[core->MEM_WB.IR.rd].rs     = (core->MEM_WB.ALUOutput & RS_MASK)     >> 16;
		core->regs_new[core->MEM_WB.IR.rd].rt     = (core->MEM_WB.ALUOutput & RT_MASK)     >> 12;
		core->regs_new[core->MEM_WB.IR.rd].imm    = (core->MEM_WB.ALUOutput & IMM_MASK)    >>  0;
	}
}

void CacheRead(SimulatorCore* core) {
	unsigned int tag    = (core->MEM_WB.ALUOutput & TAG_MASK)    >> 8;
	unsigned int index  = (core->MEM_WB.ALUOutput & INDEX_MASK)  >> 2;
	unsigned int offset = (core->MEM_WB.ALUOutput & OFFSET_MASK) >> 0;

	// Cache hit.
	if (core->cache.tsram[index].tag == tag && core->cache.tsram[index].MESI != INVALID) {
		core->MEM_WB.LMD = core->cache.dsram[index].data[offset];
		core->cache.status = CACHE_HIT;
		core->stats.cache_read_hit++;

		return;
	}

	core->bus->waiting[core->id] = TRUE;
	if (core->bus->rr_grant != core->id) {
		return;
	}

	// Cache miss - Request.
	if (core->cache.status != CACHE_FETCH) {
		core->cache.status       = CACHE_FETCH;
		core->cache.base_address = (core->MEM_WB.ALUOutput & ~0xF) >> 2;
		core->bus->bus_cmd       = BUS_READ_COMMAND;
		core->bus->bus_origid    = core->id;
		core->bus->bus_addr      = core->cache.base_address;
		core->stats.cache_read_miss++;
	}

	// Cache miss - Response.
	else if (BUS_FLUSH_COMMAND == core->bus->bus_cmd) {
		core->cache.dsram[index].data[core->bus->bus_addr - core->cache.base_address].value = core->bus->bus_data;

		// Last operation is when the last word in the block have been read.
		if (core->bus->bus_addr == (core->cache.base_address + CACHE_BLOCK_SIZE - 1)) {
			core->MEM_WB.LMD             = core->cache.dsram[index].data[offset];
			core->cache.status           = CACHE_HIT;
			core->bus->waiting[core->id] = FALSE;

			// Update cache.
			core->cache.tsram[index].tag  = tag;
			core->cache.tsram[index].MESI = core->bus->bus_shared ? SHARED : EXCLUSIVE;
		}
	}
}

void CacheWrite(SimulatorCore* core) {
	unsigned int tag    = (core->MEM_WB.ALUOutput & TAG_MASK)    >> 8;
	unsigned int index  = (core->MEM_WB.ALUOutput & INDEX_MASK)  >> 2;
	unsigned int offset = (core->MEM_WB.ALUOutput & OFFSET_MASK) >> 0;

	// Dirty cacheline - write to memory.
	if (core->cache.tsram[index].tag != tag && core->cache.tsram[index].MESI == MODIFIED) {

		core->bus->waiting[core->id] = TRUE;
		if (core->bus->rr_grant != core->id) {
			return;
		}

		// Request.
		if (core->cache.status != CACHE_DIRTY) {
			core->cache.status       = CACHE_DIRTY;
			core->cache.base_address = (tag << 8) | (index << 2);
			core->bus->bus_cmd       = BUS_READX_COMMAND;
			core->bus->bus_addr      = core->cache.base_address;
			core->bus->bus_origid    = core->id;
		}

		// Response.
		if (CACHE_DIRTY == core->cache.status && BUS_FLUSH_COMMAND == core->bus->bus_cmd) {
			core->cache.dsram[index].data[core->bus->bus_addr - core->cache.base_address].value = core->bus->bus_data;

			// Last operation is when the last word in the block have been read.
			if (core->bus->bus_addr == (core->cache.base_address + CACHE_BLOCK_SIZE - 1)) {
				core->cache.status            = CACHE_HIT;
				core->cache.tsram[index].MESI = INVALID;
				core->bus->waiting[core->id]  = FALSE;
			}
		}
	}

	// Fetch block from memory.
	else if ((core->cache.tsram[index].MESI == INVALID && core->cache.tsram[index].tag == tag) || core->cache.tsram[index].tag != tag) {

		core->bus->waiting[core->id] = TRUE;
		if (core->bus->rr_grant != core->id) {
			return;
		}

		// Request.
		if (CACHE_FETCH != core->cache.status) {
			core->cache.status       = CACHE_FETCH;
			core->cache.base_address = (core->MEM_WB.ALUOutput & ~0xF) >> 2;
			core->bus->bus_origid    = core->id;
			core->bus->bus_cmd       = BUS_READX_COMMAND;
			core->bus->bus_addr      = core->cache.base_address;
			core->stats.cache_write_miss++;
		}

		// Response.
		if (CACHE_FETCH == core->cache.status && BUS_FLUSH_COMMAND == core->bus->bus_cmd) {
			core->cache.dsram[index].data[core->bus->bus_addr - core->cache.base_address].value = core->bus->bus_data;

			// Last operation is when the last word in the block have been read.
			if (core->bus->bus_addr == (core->cache.base_address + CACHE_BLOCK_SIZE - 1)) {
				core->cache.status = CACHE_HIT;
				core->cache.dsram[index].data[offset].value = core->MEM_WB.B;

				// Update cache.
				core->cache.tsram[index].tag  = tag;
				core->cache.tsram[index].MESI = core->bus->bus_shared ? SHARED : EXCLUSIVE;
				core->bus->waiting[core->id]  = FALSE;
			}
		}
	}

	// Update cache data only.
	else {
		core->cache.status = CACHE_HIT;
		core->cache.dsram[index].data[offset].value = core->MEM_WB.B;
		core->stats.cache_write_hit++;
	}
}

void Snooping(SimulatorCore* core) {
	unsigned int tag    = (core->bus->bus_addr & TAG_MASK) >> 8;
	unsigned int index  = (core->bus->bus_addr & INDEX_MASK) >> 2;
	unsigned int offset = (core->bus->bus_addr & OFFSET_MASK) >> 0;

	// Flush to nearby caches.
	if (core->bus->bus_origid != core->id && core->bus->bus_cmd == BUS_READ_COMMAND) {
		if (core->cache.tsram[index].MESI != INVALID && core->cache.tsram[index].tag == tag) {

			core->bus->waiting[core->id] = TRUE;
			if (core->bus->rr_grant != core->id) {
				return;
			}

			core->bus->bus_origid         = core->id;
			core->bus->bus_cmd            = BUS_FLUSH_COMMAND;
			core->bus->bus_data           = core->cache.dsram[index].data[offset].value;
			core->bus->bus_shared         = TRUE;
			core->cache.tsram[index].MESI = SHARED;
			core->bus->waiting[core->id]  = FALSE;
		}
	}
}

#endif // __CORE_C__