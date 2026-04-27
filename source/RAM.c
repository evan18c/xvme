// RAM
// Author: Evan Cassidy
// Date: 4/24/2026

#include "RAM.h"

void AddRegion(RAM *ram, uint32_t base, uint32_t size) {
    Region *region = &ram->regions[ram->region_count++];
    region->base = base;
    region->size = size;
    region->data = calloc(size, 1);
}

uint8_t ReadByte(RAM *ram, uint32_t addr) {
    for (int i = 0; i < ram->region_count; i++) {
        Region *region = &ram->regions[i];
        if (addr >= region->base && addr < (region->base + region->size)) {
            return region->data[addr - region->base];
        }
    }
    printf("ReadByte: Segfault at %08x!\n", addr);
    exit(1);
}

uint32_t Read32(RAM *ram, uint32_t addr) {
    for (int i = 0; i < ram->region_count; i++) {
        Region *region = &ram->regions[i];
        if (addr >= region->base && addr < (region->base + region->size)) {
            uint32_t raw_addr = addr - region->base;
            return region->data[raw_addr] | (region->data[raw_addr + 1] << 8) | (region->data[raw_addr + 2] << 16) | (region->data[raw_addr + 3] << 24);
        }
    }
    printf("Read32: Segfault at %08x!\n", addr);
    exit(1);
}

void WriteByte(RAM *ram, uint32_t addr, uint8_t byte) {
    for (int i = 0; i < ram->region_count; i++) {
        Region *region = &ram->regions[i];
        if (addr >= region->base && addr < (region->base + region->size)) {
            region->data[addr - region->base] = byte;
            return;
        }
    }
    printf("WriteByte: Segfault at %08x!\n", addr);
    exit(1);
}

void Write32(RAM *ram, uint32_t addr, uint32_t data) {
    for (int i = 0; i < ram->region_count; i++) {
        Region *region = &ram->regions[i];
        if (addr >= region->base && addr < (region->base + region->size)) {
            uint32_t raw_addr = addr - region->base;
            region->data[raw_addr] = data & 0xFF;
            region->data[raw_addr + 1] = (data >> 8) & 0xFF;
            region->data[raw_addr + 2] = (data >> 16) & 0xFF;
            region->data[raw_addr + 3] = (data >> 24) & 0xFF;
            return;
        }
    }
    printf("Write32: Segfault at %08x!\n", addr);
    exit(1);
}

void *RawPointer(RAM *ram, uint32_t addr) {
    for (int i = 0; i < ram->region_count; i++) {
        Region *region = &ram->regions[i];
        if (addr >= region->base && addr < (region->base + region->size)) {
            return &region->data[addr - region->base];
        }
    }
    printf("RawPointer: Segfault at %08x!\n", addr);
    exit(1);
}