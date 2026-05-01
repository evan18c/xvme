// CPU Structure
// Author: Evan Cassidy
// Date: 4/22/2026
#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Forward
typedef struct Xbox Xbox;
typedef struct RAM RAM;

typedef struct CPU {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi;
    uint32_t ebp, esp;
    uint32_t eip;
    uint8_t CF, PF, AF, ZF, SF, TF, IF, DF, OF, RF;
    uint32_t CS, DS, SS, ES, FS, GS;
    uint64_t counter;
} CPU;

void CPUState(CPU *cpu);

void CPURun(Xbox *xbox, CPU *cpu, RAM *ram);

#endif