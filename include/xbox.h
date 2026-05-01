// Xbox
// Author: Evan Cassidy
// Date: 4/22/2026
#ifndef XBOX_H
#define XBOX_H
#include <stdint.h>

// Forward
typedef struct CPU CPU;
typedef struct RAM RAM;

typedef struct Xbox {
    CPU *threads[64];
    uint32_t thread_count;
    RAM *ram;
} Xbox;

Xbox *XboxNew();
void XboxLoadXBE(Xbox *xbox, const char *xbe);
void XboxRun(Xbox *xbox);
void XboxCreateThread(Xbox *xbox, uint32_t entry);

#endif