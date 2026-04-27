// Kernel
// Author: Evan Cassidy
// Date: 4/27/2026

#include "kernel.h"

// Intercepts Kernel calls by the CPU
void HandleKernelCall(CPU *cpu, RAM *ram) {
    uint32_t ordinal = cpu->eip - 0x80000000;
    printf("Kernel Call: %d\n", ordinal);

    switch (ordinal) {

        // PsCreateSystemThreadEx
        case 255:
            cpu->eax = 0;
            break;

        // Inknown Kernel Call
        default:
            printf("Unknown kernel call %d\n", ordinal);
            State(cpu);
            exit(1);

    }

    // RET
    uint32_t ret = Read32(ram, cpu->esp);
    cpu->esp += 4;
    cpu->eip = ret;
}