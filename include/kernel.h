// Kernel
// Author: Evan Cassidy
// Date: 4/27/2026
#ifndef KERNEL_H
#define KERNEL_H

// Forward
typedef struct Xbox Xbox;
typedef struct CPU CPU;
typedef struct RAM RAM;

void HandleKernelCall(Xbox *xbox, CPU *cpu, RAM *ram);

#endif