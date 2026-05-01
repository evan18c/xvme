// Xbox
// Author: Evan Cassidy
// Date: 4/22/2026
#ifndef XBOX_H
#define XBOX_H

// Forward
typedef struct CPU CPU;
typedef struct RAM RAM;

typedef struct Xbox {
    CPU *cpu;
    RAM *ram;
} Xbox;

Xbox *XboxNew();
void XboxLoadXBE(Xbox *xbox, const char *xbe);
void XboxRun(Xbox *xbox);

#endif