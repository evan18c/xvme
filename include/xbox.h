// Xbox
// Author: Evan Cassidy
// Date: 4/22/2026
#ifndef XBOX_H
#define XBOX_H

#include "CPU.h"
#include "RAM.h"

typedef struct Xbox {
    CPU *cpu;
    RAM *ram;
} Xbox;

Xbox *XboxNew();
void XboxRun(Xbox *xbox);

#endif