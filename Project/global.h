#ifndef __GLOBAL_H__
#define __GLOBAL_H__

/*===================================
|           INCLUDES                |
====================================*/
#include <stdlib.h>
#include <stdio.h>

/*===================================
|           DEFINES                 |
====================================*/
#define MAX_FILENAME_SIZE 50
#define NUM_OF_CORES      4
#define NUM_OF_CLI_ARGS   (3+6*NUM_OF_CORES)
#define TRUE              1
#define FALSE             0

// Input Files.
#define RAM_INPUT_FILE "memin.txt"
#define INSTRUCTION_INPUT_FILE "imem%0d.txt"

// Output Files.
#define RAM_OUTPUT_FILE "memout.txt"
#define REGISTER_OUTPUT_FILE "regout%0d.txt"
#define TRACE_OUTPUT_FILE "core%0dtrace.txt"
#define BUSTRACE_OUTPUT_FILE "bustrace.txt"
#define DSRAM_OUTPUT_FILE "dsram%0d.txt"
#define TSRAM_OUTPUT_FILE "tsram%0d.txt"
#define STATS_OUTPUT_FILE "stats%0d.txt"

/*===================================
|             MACROS                |
====================================*/
#define STR_(X) #X
#define STR(X) STR_(X)

#define SIMFILE_INIT(SIMFILE, SIMULATOR) \
	do { \
		SIMFILE = (SimFile*)malloc(sizeof(SimFile)); \
		if (NULL == SIMFILE) { SIMULATOR.status = MEMORY_ALLOCATION_ERROR; return; } \
		SIMFILE->name_length = 0; \
		for (int i = 0; i < MAX_FILENAME_SIZE; i++) { SIMFILE->name[i] = '\0'; } \
	} while (0)

#define SIMFILE_OPEN(HANDLE, ARG, NAME, ACCESS, SIMULATOR) \
    do { \
		checkCLIfile(SIMULATOR, ARG, NAME); \
		if (SIMULATOR->status != NO_ERROR) { return; } \
		strcpy(HANDLE.name,  ARG); \
		HANDLE.name_length = strlen(HANDLE.name); \
		HANDLE.handle = fopen(HANDLE.name, ACCESS); \
		if (NULL == HANDLE.handle) { SIMULATOR->status = INPUT_FILE_DOES_NOT_EXISTS; return; } \
		fseek(HANDLE.handle, 0, SEEK_END); \
		HANDLE.len = ftell(HANDLE.handle); \
		HANDLE.empty = (HANDLE.len == 0); \
		rewind(HANDLE.handle); \
	} while (0)

#define CLOSE_FILE(HANDLE) do { if (NULL != HANDLE) { fclose(HANDLE); HANDLE = NULL; } } while (0)

/*===================================
|          ERROR CODES              |
====================================*/
typedef enum {
	NO_ERROR = 0,
	INCORRECT_CLI_ARGUMENTS,
	INPUT_FILE_DOES_NOT_EXISTS,
	FILE_BAD_NAME,
	FILE_BAD_EXTENSION,
	MEMORY_ALLOCATION_ERROR,
	FILE_CLOSED,
	HALT_CORE,
	HALT_SIMULATION,
	NUM_OF_STATUS
} ErrorCode;

/*===================================
|           TYPEDEFS                |
====================================*/
typedef char unsigned bool;

/*===================================
|            STRUCTS                |
====================================*/
typedef struct {
	char   name[MAX_FILENAME_SIZE + 1];
	size_t name_length;
	FILE*  handle;
	bool   empty;
	size_t len;   // Length in bytes.
	size_t w_len; // Length in words.
} SimFile;

typedef struct {
	SimFile imem;
	SimFile regout;
	SimFile trace;
	SimFile dsram;
	SimFile tsram;
	SimFile stats;
} CoreFiles;

#endif // __GLOBAL_H__