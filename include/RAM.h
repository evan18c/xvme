// RAM
// Author: Evan Cassidy
// Date: 4/24/2026
#ifndef RAM_H
#define RAM_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct Region {
    uint32_t base;
    uint32_t size;
    uint8_t *data;
} Region;

typedef struct RAM {
    Region regions[64];
    uint8_t region_count;
} RAM;

void RAMAddRegion(RAM *ram, uint32_t base, uint32_t size);

uint8_t RAMReadByte(RAM *ram, uint32_t addr);
uint32_t RAMRead32(RAM *ram, uint32_t addr);
void RAMWriteByte(RAM *ram, uint32_t addr, uint8_t byte);
void RAMWrite32(RAM *ram, uint32_t addr, uint32_t data);

// Virtual Address -> Raw Address
void *RAMRawPointer(RAM *ram, uint32_t addr);

#endif