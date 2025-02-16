#ifndef __ISA_C__
#define __ISA_C__

/*===================================
|           INCLUDES                |
====================================*/
#include "isa.h"

/*===================================
|           FUNCTIONS               |
====================================*/
void nop(SimulatorCore* core) { }

void add(SimulatorCore* core) {
	core->EX_MEM.ALUOutput = core->ID_EX.A + core->ID_EX.B;
	if (core->ID_EX.IR.rt == REGISTER_IMMIDIATE) {
		core->EX_MEM.ALUOutput = core->ID_EX.A + core->ID_EX.imm; 
	}
}

void sub(SimulatorCore* core) {
	core->EX_MEM.ALUOutput = core->ID_EX.A - core->ID_EX.B;
	if (core->ID_EX.IR.rt == REGISTER_IMMIDIATE) {
		core->EX_MEM.ALUOutput = core->ID_EX.A - core->ID_EX.imm;
	}
}

void mul(SimulatorCore* core) {
	core->EX_MEM.ALUOutput = core->ID_EX.A * core->ID_EX.B;
	if (core->ID_EX.IR.rt == REGISTER_IMMIDIATE) {
		core->EX_MEM.ALUOutput = core->ID_EX.A * core->ID_EX.imm;
	}
}

void and (SimulatorCore* core) {
	core->EX_MEM.ALUOutput = core->ID_EX.A & core->ID_EX.B;
	if (core->ID_EX.IR.rt == REGISTER_IMMIDIATE) {
		core->EX_MEM.ALUOutput = core->ID_EX.A & core->ID_EX.imm;
	}
}

void or (SimulatorCore* core) {
	core->EX_MEM.ALUOutput = core->ID_EX.A | core->ID_EX.B;
	if (core->ID_EX.IR.rt == REGISTER_IMMIDIATE) {
		core->EX_MEM.ALUOutput = core->ID_EX.A | core->ID_EX.imm;
	}
}

void xor (SimulatorCore* core) {
	core->EX_MEM.ALUOutput = core->ID_EX.A ^ core->ID_EX.B;
	if (core->ID_EX.IR.rt == REGISTER_IMMIDIATE) {
		core->EX_MEM.ALUOutput = core->ID_EX.A ^ core->ID_EX.imm;
	}
}

void sll(SimulatorCore* core) {
	core->EX_MEM.ALUOutput = core->ID_EX.A << core->ID_EX.B;
	if (core->ID_EX.IR.rt == REGISTER_IMMIDIATE) {
		core->EX_MEM.ALUOutput = core->ID_EX.A << core->ID_EX.imm;
	}
}

void sra(SimulatorCore* core) {
	// TODO: SRA properly.
	core->EX_MEM.ALUOutput = (int)core->ID_EX.A - (int)core->ID_EX.B;
	if (core->ID_EX.IR.rt == REGISTER_IMMIDIATE) {
		core->EX_MEM.ALUOutput = (int)core->ID_EX.A - core->ID_EX.imm;
	}
}

void srl(SimulatorCore* core) {
	core->EX_MEM.ALUOutput = core->ID_EX.A >> core->ID_EX.B;
	if (core->ID_EX.IR.rt == REGISTER_IMMIDIATE) {
		core->EX_MEM.ALUOutput = core->ID_EX.A >> core->ID_EX.imm;
	}
}

void beq(SimulatorCore* core) {
	core->EX_MEM.cond = (core->ID_EX.A == 0);
	if (core->ID_EX.A == core->ID_EX.B) {
		core->EX_MEM.ALUOutput = core->ID_EX.NPC + (core->ID_EX.imm << 2);
	}
}

void bne(SimulatorCore* core) {
	core->EX_MEM.cond = (core->ID_EX.A == 0);
	if (core->ID_EX.A != core->ID_EX.B) {
		core->EX_MEM.ALUOutput = core->ID_EX.NPC + (core->ID_EX.imm << 2);
	}
}

void blt(SimulatorCore* core) {
	core->EX_MEM.cond = (core->ID_EX.A == 0);
	if (core->ID_EX.A < core->ID_EX.B) {
		core->EX_MEM.ALUOutput = core->ID_EX.NPC + (core->ID_EX.imm << 2);
	}
}

void bgt(SimulatorCore* core) {
	core->EX_MEM.cond = (core->ID_EX.A == 0);
	if (core->ID_EX.A > core->ID_EX.B) {
		core->EX_MEM.ALUOutput = core->ID_EX.NPC + (core->ID_EX.imm << 2);
	}
}

 void ble(SimulatorCore* core) {
	 core->EX_MEM.cond = (core->ID_EX.A == 0);
	 if (core->ID_EX.A <= core->ID_EX.B) {
		 core->EX_MEM.ALUOutput = core->ID_EX.NPC + (core->ID_EX.imm << 2);
	 }
}

void bge(SimulatorCore* core) {
	core->EX_MEM.cond = (core->ID_EX.A == 0);
	if (core->ID_EX.A >= core->ID_EX.B) {
		core->EX_MEM.ALUOutput = core->ID_EX.NPC + (core->ID_EX.imm << 2);
	}
}

void jal(SimulatorCore* core) {
	core->EX_MEM.cond = (core->ID_EX.A == 0);
	
	core->regs_new[15].imm = core->ID_EX.imm;
	core->PC = core->regs[core->ID_EX.IR.rd].imm;
}

void lw(SimulatorCore* core) {
	core->EX_MEM.ALUOutput = core->ID_EX.A + core->ID_EX.imm;
	core->EX_MEM.B = core->ID_EX.B;
}

void sw(SimulatorCore* core) {
	core->EX_MEM.ALUOutput = core->ID_EX.A + core->ID_EX.imm;
	core->EX_MEM.B = core->ID_EX.B;
}

void halt(SimulatorCore* core) {
	core->status = HALT_CORE;
}

#endif // __ISA_C__