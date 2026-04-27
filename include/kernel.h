// Kernel
// Author: Evan Cassidy
// Date: 4/27/2026
#ifndef KERNEL_H
#define KERNEL_H

#include "CPU.h"
#include "RAM.h"

void HandleKernelCall(CPU *cpu, RAM *ram);

#endif