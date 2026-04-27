// RAM
// Author: Evan Cassidy
// Date: 4/24/2026
#ifndef RAM_H
#define RAM_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    uint32_t base;
    uint32_t size;
    uint8_t *data;
} Region;

typedef struct {
    Region regions[64];
    uint8_t region_count;
} RAM;

void AddRegion(RAM *ram, uint32_t base, uint32_t size);

uint8_t ReadByte(RAM *ram, uint32_t addr);
uint32_t Read32(RAM *ram, uint32_t addr);
void WriteByte(RAM *ram, uint32_t addr, uint8_t byte);
void Write32(RAM *ram, uint32_t addr, uint32_t data);

// Virtual Address -> Raw Address
void *RawPointer(RAM *ram, uint32_t addr);

#endif