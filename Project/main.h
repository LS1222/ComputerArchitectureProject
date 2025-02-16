#ifndef __MAIN_H__
#define __MAIN_H__

/*===================================
|           DEFINES                 |
====================================*/
#define _CRT_SECURE_NO_WARNINGS

/*===================================
|           INCLUDES                |
====================================*/
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "core.h"
#include "bus.h"
#include "simulator.h"

/*===================================
|        FUNCTIONS PROTOTYPE        |
====================================*/
void InitializeCore     (SimulatorCore* core);
void InitializeSimulator(Simulator* sim);
void splitFileExtention (const char* file_name, char* name, char* ext);
void checkCLIfile       (Simulator* sim, const char* act_file, const char* exp_file);
void ParseCLI           (int argc, char* argv[], Simulator* sim);
void LoadMemory         (SimFile* filehandle, WORD* mem);
void LoadMemories       (Simulator* sim);
void PrintMemory        (Simulator* sim);
void PrintCache         (SimulatorCore* core);
void PrintRegisters     (SimulatorCore* core);
void PrintStats         (SimulatorCore* core);
void SimulationCycle    (Simulator* sim);
void FinalizeSimulation (Simulator* sim);

#endif // __MAIN_H__