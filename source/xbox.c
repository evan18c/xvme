// Xbox
// Author: Evan Cassidy
// Date: 4/22/2026

#include "xbox.h"

#include "CPU.h"
#include "RAM.h"
#include "utils.h"
#include "xbe.h"

void xbox() {

    // Create CPU + RAM
    CPU *cpu = calloc(1, sizeof(CPU));
    RAM *ram = calloc(1, sizeof(RAM));

    // read xbe
    int size;
    char *xbe = ReadFile("Default.xbe", &size);

    // read header
    XBEImageHeader *header = (XBEImageHeader *)xbe;
    uint32_t BaseAddress = header->BaseAddress;
    printf("BaseAddress: 0x%08X\n", BaseAddress);

    // Load Header
    AddRegion(ram, BaseAddress, header->SizeOfHeaders);
    memcpy(RawPointer(ram, BaseAddress), xbe, header->SizeOfHeaders);

    // Load Sections
    printf("Number Of Sections: %d\n", header->NumberOfSections);
    XBESectionHeader *headers = (XBESectionHeader *)(xbe + (header->SectionHeaderAddress - BaseAddress));
    for (int i = 0; i < header->NumberOfSections; i++) {
        uint32_t VirtualAddress = headers[i].VirtualAddress;
        uint32_t VirtualSize = headers[i].VirtualSize;
        uint32_t RawAddress = headers[i].RawAddress;
        uint32_t RawSize = headers[i].RawSize;
        char *SectionName = xbe + (headers[i].SectionNameAddress - BaseAddress);

        printf("AddRegion (%s) at 0x%08X Size 0x%08X\n", SectionName, VirtualAddress, VirtualSize);
        AddRegion(ram, VirtualAddress, VirtualSize);
        memcpy(RawPointer(ram, VirtualAddress), xbe + RawAddress, RawSize);
    }

    // Entry Point
    uint32_t EntryPoint = header->EntryPoint ^ 0xA8FC57AB;
    printf("EntryPoint: 0x%08X\n", EntryPoint);

    // Running
    printf("\n");
    AddRegion(ram, 0x03F00000, 0x100000); // stack
    cpu->esp = 0x04000000;
    cpu->eip = EntryPoint;
    Run(cpu, ram);

    printf("End of xbox();\n");
    exit(0);
}
