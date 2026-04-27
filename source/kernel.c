// Kernel
// Author: Evan Cassidy
// Date: 4/27/2026

#include "kernel.h"

// Intercepts Kernel calls by the CPU
void HandleKernelCall(CPU *cpu, RAM *ram) {

    // Ordinal
    uint32_t ordinal = cpu->eip - 0x80000000;

    // Return Address
    uint32_t ret = Read32(ram, cpu->esp);
    cpu->esp += 4;

    switch (ordinal) {

        // __stdcall PsCreateSystemThreadEx (10 args)
        case 255: {
            printf("PsCreateSystemThreadEx();\n");
            uint32_t StartRoutine = Read32(ram, cpu->esp + 4 * 5);
            printf("StartRoutine: %08X\n", StartRoutine);
            cpu->esp += 4 * 10;
            cpu->eax = 0;
            break;
        }

        // Inknown Kernel Call
        default:
            printf("Unknown kernel call %d\n", ordinal);
            State(cpu);
            exit(1);

    }

    // Return
    cpu->eip = ret;
}