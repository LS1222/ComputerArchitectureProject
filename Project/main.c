/*===================================
|           INCLUDES                |
====================================*/
#include "main.h"

/*===================================
|           FUNCTIONS               |
====================================*/
void InitializeCore(SimulatorCore* core) {
	memset(core, 0, sizeof(SimulatorCore));
	core->status = NO_ERROR;

	for (int reg = REGISTER_ZERO; reg < NUM_OF_REGISTERS; reg++) {
		core->regs[reg].opcode = REGISTER_DEFAULT & OPCODE_MASK;
		core->regs[reg].rd     = REGISTER_DEFAULT & RD_MASK;
		core->regs[reg].rs     = REGISTER_DEFAULT & RS_MASK;
		core->regs[reg].rt     = REGISTER_DEFAULT & RT_MASK;
		core->regs[reg].imm    = REGISTER_DEFAULT & IMM_MASK;
	}

	for (int block = 0; block < CACHE_BLOCKS; block++) {

		// DSRAM.
		for (int offset = 0; offset < CACHE_BLOCK_SIZE; offset++) {
			core->cache.dsram[block].data[offset].value = 0;
		}

		// TSRAM.
		core->cache.tsram[block].MESI = INVALID;
		core->cache.tsram[block].tag = 0;
	}
}

void InitializeSimulator(Simulator* sim) {
	memset(sim, 0, sizeof(Simulator));
	sim->status = NO_ERROR;

	// Allocate memory, bus and cores.
	sim->ram = (SimulatorMemory*)calloc(1, sizeof(SimulatorMemory));
	if (NULL == sim->ram) {
		sim->status = MEMORY_ALLOCATION_ERROR;
		return;
	}

	sim->bus = (SimulatorBus*)calloc(1, sizeof(SimulatorBus));
	if (NULL == sim->bus) {
		sim->status = MEMORY_ALLOCATION_ERROR;
		return;
	}

	sim->cpu = (SimulatorCore*)calloc(NUM_OF_CORES, sizeof(SimulatorCore));
	if (NULL == sim->cpu) {
		sim->status = MEMORY_ALLOCATION_ERROR;
		return;
	}

	// Initialize core values.
	for (int core = 0; core < NUM_OF_CORES; core++) {
		InitializeCore(&sim->cpu[core]);
		sim->cpu[core].id           = core;
		sim->cpu[core].bus          = sim->bus;
		sim->cpu[core].cache.status = CACHE_HIT;
	}
}

void splitFileExtention(const char* file_name, char* name, char* ext) {
	int   sub_index = 0;
	char* substring = NULL;

	// Check pointers.
	if (NULL == file_name) {
		return;
	}

	substring = name;
	for (int i = 0; i < strlen(file_name); i++) {

		// When finding a '.' delimiter, start collecting the extension.
		if (file_name[i] == '.') {
			substring[sub_index] = '\0';
			sub_index = 0;
			substring = ext;
			continue;

			// TODO: Corner case - file contains multiple delimiters.
		}

		// Get the character into the substring [filename/extension].
		substring[sub_index++] = file_name[i];
	}

	// Add the NULL terminator to the string.
	substring[sub_index] = '\0';
}

void checkCLIfile(Simulator* sim, const char* act_file, const char* exp_file) {
	char act_name[MAX_FILENAME_SIZE + 1] = { 0 };
	char exp_name[MAX_FILENAME_SIZE + 1] = { 0 };
	char act_ext [MAX_FILENAME_SIZE + 1] = { 0 };
	char exp_ext [MAX_FILENAME_SIZE + 1] = { 0 };

	splitFileExtention(act_file, act_name, act_ext);
	splitFileExtention(exp_file, exp_name, exp_ext);

	// Check file name.
	if (strcmp(act_name, exp_name)) {
		printf("Incorrect filename. Expected %s but got %s\n", exp_name, act_name);
		sim->status = FILE_BAD_NAME;
		return;
	}

	// Check file extension.
	if (strcmp(act_ext, exp_ext)) {
		printf("Bad extension. Expected %s.%s but got %s.%s\n", exp_name, exp_ext, act_name, act_ext);
		sim->status = FILE_BAD_EXTENSION;
		return;
	}
}

void ParseCLI(int argc, char* argv[], Simulator* sim) {
	char filename[MAX_FILENAME_SIZE + 1] = { 0 };

	if (argc != (NUM_OF_CLI_ARGS + 1)) {
		printf("Incorrect number of arguments.\nExpected %0d | Actual: %0d\n", NUM_OF_CLI_ARGS, argc - 1);

		// Return an error code.
		sim->status = INCORRECT_CLI_ARGUMENTS;
		return;
	}

	// Open the files.
	for (int core = 0; core < NUM_OF_CORES; core++) {
		(void)snprintf(filename, MAX_FILENAME_SIZE, INSTRUCTION_INPUT_FILE, core);
		SIMFILE_OPEN(sim->cpu[core].files.imem,   argv[core +  1], filename, "r", sim);
		(void)snprintf(filename, MAX_FILENAME_SIZE, REGISTER_OUTPUT_FILE, core);
		SIMFILE_OPEN(sim->cpu[core].files.regout, argv[core +  7], filename, "w", sim);
		(void)snprintf(filename, MAX_FILENAME_SIZE, TRACE_OUTPUT_FILE, core);
		SIMFILE_OPEN(sim->cpu[core].files.trace,  argv[core + 11], filename, "w", sim);
		(void)snprintf(filename, MAX_FILENAME_SIZE, DSRAM_OUTPUT_FILE, core);
		SIMFILE_OPEN(sim->cpu[core].files.dsram,  argv[core + 16], filename, "w", sim);
		(void)snprintf(filename, MAX_FILENAME_SIZE, TSRAM_OUTPUT_FILE, core);
		SIMFILE_OPEN(sim->cpu[core].files.tsram,  argv[core + 20], filename, "w", sim);
		(void)snprintf(filename, MAX_FILENAME_SIZE, STATS_OUTPUT_FILE, core);
		SIMFILE_OPEN(sim->cpu[core].files.stats,  argv[core + 24], filename, "w", sim);
	}
	SIMFILE_OPEN(sim->memin,    argv[5 ], RAM_INPUT_FILE,       "r", sim);
	SIMFILE_OPEN(sim->memout,   argv[6 ], RAM_OUTPUT_FILE,      "w", sim);
	SIMFILE_OPEN(sim->bustrace, argv[15], BUSTRACE_OUTPUT_FILE, "w", sim);
}

void LoadMemory(SimFile* filehandle, WORD* mem) {
	char line[MAX_MEM_LINE_LENGTH + 1] = { 0 };
	unsigned int address = 0;
	unsigned int value   = 0;

	// Initialize RAM memory from file.
	filehandle->w_len = filehandle->len / MAX_MEM_LINE_LENGTH;
	while (!filehandle->empty) {

		// Read the memory line.
		fgets(line, MAX_MEM_LINE_LENGTH + 1, filehandle->handle);

		// Read the \n.
		fgetc(filehandle->handle);

		if (feof(filehandle->handle)) {
			return;
		}

		// Covert to number and store in memory.
		(void)sscanf(line, "%X", &value);
		mem[address++].value = value;
	}
}

void LoadMemories(Simulator* sim) {
	LoadMemory(&sim->memin, sim->ram->mem);
	for (int core = 0; core < NUM_OF_CORES; core++) {
		LoadMemory(&sim->cpu[core].files.imem, sim->cpu[core].imem);
	}
}

void PrintMemory(Simulator* sim) {
	char line[MAX_MEM_LINE_LENGTH + 2] = { 0 };
	char line_format[MAX_MEM_LINE_LENGTH + 4] = { 0 };
	(void)sprintf(line_format + 2, "%0dX\n", MAX_MEM_LINE_LENGTH);
	line_format[0] = '%';
	line_format[1] = '0';

	// Check if the file is closed.
	if (feof(sim->memout.handle)) {
		sim->status = FILE_CLOSED;
		return;
	}

	// Write all the memory to the output file.
	for (int address = 0; address < RAM_SIZE; address++) {

		// Convert the memory word to a string line.
		sprintf(line, line_format, sim->ram->mem[address].value);

		// Override case for last line.
		if (RAM_SIZE - 1 == address) {
			(void)sprintf(line_format + 2, "%0dX", MAX_MEM_LINE_LENGTH);
			line_format[0] = '%';
			line_format[1] = '0';
			sprintf(line, line_format, sim->ram->mem[address].value);
		}

		// Write to file.
		fputs(line, sim->memout.handle);
	}
}

void PrintCache(SimulatorCore* core) {
	char line[MAX_MEM_LINE_LENGTH + 2] = { 0 };

	// Check if the DSRAM file is closed.
	if (feof(core->files.dsram.handle)) {
		core->status = FILE_CLOSED;
		return;
	}

	// Check if the TSRAM file is closed.
	if (feof(core->files.tsram.handle)) {
		core->status = FILE_CLOSED;
		return;
	}

	// Write all the memory to the output file.
	for (int index = 0; index < CACHE_BLOCKS; index++) {
		for (int offset = 0; offset < CACHE_BLOCK_SIZE; offset++) {

			// Write each DSRAM word to file.
			sprintf(line, "%08X\n", core->cache.dsram[index].data[offset].value);
			fputs(line, core->files.dsram.handle);
		}

		// Write TSRAM cacheline to file.
		sprintf(line, "%08X\n", *(int*)&core->cache.tsram[index]);
		fputs(line, core->files.tsram.handle);
	}
}

void PrintRegisters(SimulatorCore* core) {

	// Check if the file is closed.
	if (feof(core->files.regout.handle)) {
		core->status = FILE_CLOSED;
		return;
	}

	// Print all the registers without registers 0 and 1 (zero and imm).
	for (int reg = 2; reg < NUM_OF_REGISTERS; reg++) {
		fprintf(core->files.regout.handle, "%08X\n", (*(int*)&core->regs[reg]));
	}
}

void PrintStats(SimulatorCore* core) {

	// Check if the file is closed.
	if (feof(core->files.stats.handle)) {
		core->status = FILE_CLOSED;
		return;
	}

	fprintf(core->files.stats.handle, "cycles %0d\n",       core->stats.cycles);
	fprintf(core->files.stats.handle, "instructions %0d\n", core->stats.instructions);
	fprintf(core->files.stats.handle, "read_hit %0d\n",     core->stats.cache_read_hit);
	fprintf(core->files.stats.handle, "write_hit %0d\n",    core->stats.cache_write_hit);
	fprintf(core->files.stats.handle, "read_miss %0d\n",    core->stats.cache_read_miss);
	fprintf(core->files.stats.handle, "write_miss %0d\n",   core->stats.cache_write_miss);
	fprintf(core->files.stats.handle, "decode_stall %0d\n", core->stats.decode_stall);
	fprintf(core->files.stats.handle, "mem_stall %0d\n",    core->stats.mem_stall);
}

void SimulationCycle(Simulator* sim) {
	unsigned int active_cores = NUM_OF_CORES;

	for (int core = 0; core < NUM_OF_CORES; core++) {
		if (HALT_CORE == sim->cpu[core].status) {
			active_cores--;
		} else {
			CoreCycle(&sim->cpu[core]);
		}
	}

	// RAM cycle.
	MemoryCycle(sim->bus, sim->ram);

	// Halt simualtion.
	if (active_cores == 0) {
		sim->status = HALT_SIMULATION;
	}
}

void FinalizeSimulation(Simulator* sim) {

	// Print to files all the cycle information.
	PrintMemory(sim);
	for (int core = 0; core < NUM_OF_CORES; core++) {
		PrintRegisters(&sim->cpu[core]);
		PrintStats(&sim->cpu[core]);
		PrintCache(&sim->cpu[core]);
	}

	// Close all the files.
	for (int core = 0; core < NUM_OF_CORES; core++) {
		CLOSE_FILE(sim->cpu[core].files.imem.handle);
		CLOSE_FILE(sim->cpu[core].files.regout.handle);
		CLOSE_FILE(sim->cpu[core].files.trace.handle);
		CLOSE_FILE(sim->cpu[core].files.dsram.handle);
		CLOSE_FILE(sim->cpu[core].files.tsram.handle);
		CLOSE_FILE(sim->cpu[core].files.stats.handle);
	}
	CLOSE_FILE(sim->memin.handle);
	CLOSE_FILE(sim->memout.handle);
	CLOSE_FILE(sim->bustrace.handle);

	// Free the memory.
	if (NULL != sim->cpu)     { free(sim->cpu);     }
	if (NULL != sim->ram->mem) { free(sim->ram->mem); }
}

int main(int argc, char* argv[]) {
	Simulator sim;

	// Initialize simulator.
	InitializeSimulator(&sim);

	// Check the command line arguments.
	if (NO_ERROR == sim.status) {
		ParseCLI(argc, argv, &sim);
	}

	// Load memories with data from files.
	if (NO_ERROR == sim.status) {
		LoadMemories(&sim);
	}

	while (NO_ERROR == sim.status) {
		SimulationCycle(&sim);
	}

	// Graceful simulation finalization.
	FinalizeSimulation(&sim);
	if (HALT_SIMULATION == sim.status) { return (NO_ERROR); }
	return (sim.status);
}