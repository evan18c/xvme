// Xbox
// Author: Evan Cassidy
// Date: 4/22/2026

#include "xbox.h"
#include "CPU.h"
#include "RAM.h"
#include "utils.h"
#include "xbe.h"

// Initializes Xbox
Xbox *XboxNew() {

    // Create Xbox
    Xbox *xbox = calloc(1, sizeof(Xbox));

    // Create CPU + RAM
    xbox->cpu = calloc(1, sizeof(CPU));
    xbox->ram = calloc(1, sizeof(RAM));

    // Return
    return xbox;

}

// Loads XBE
void XboxLoadXBE(Xbox *xbox, const char *xbe) {

    // read xbe
    int size;
    char *data = ReadFile(xbe, &size);

    // read header
    XBEImageHeader *header = (XBEImageHeader *)data;
    uint32_t BaseAddress = header->BaseAddress;
    printf("BaseAddress: 0x%08X\n", BaseAddress);

    // Load Header
    RAMAddRegion(xbox->ram, BaseAddress, header->SizeOfHeaders);
    memcpy(RAMRawPointer(xbox->ram, BaseAddress), data, header->SizeOfHeaders);

    // Load Sections
    printf("Number Of Sections: %d\n", header->NumberOfSections);
    XBESectionHeader *headers = (XBESectionHeader *)(data + (header->SectionHeaderAddress - BaseAddress));
    for (int i = 0; i < header->NumberOfSections; i++) {
        uint32_t VirtualAddress = headers[i].VirtualAddress;
        uint32_t VirtualSize = headers[i].VirtualSize;
        uint32_t RawAddress = headers[i].RawAddress;
        uint32_t RawSize = headers[i].RawSize;
        char *SectionName = data + (headers[i].SectionNameAddress - BaseAddress);

        printf("RAMAddRegion (%s) at 0x%08X Size 0x%08X\n", SectionName, VirtualAddress, VirtualSize);
        RAMAddRegion(xbox->ram, VirtualAddress, VirtualSize);
        memcpy(RAMRawPointer(xbox->ram, VirtualAddress), data + RawAddress, RawSize);
    }

    // Entry Point
    uint32_t EntryPoint = header->EntryPoint ^ 0xA8FC57AB;
    printf("EntryPoint: 0x%08X\n", EntryPoint);
    xbox->cpu->eip = EntryPoint;

    // Kernel Imports
    uint32_t KernelImageThunkAddress = header->KernelImageThunkAddress ^ 0x5B6D40B6;
    printf("KernelImageThunkAddress: 0x%08X\n", KernelImageThunkAddress);

    // Kernel LaunchDataPage
    RAMAddRegion(xbox->ram, 0x81000000, 0x1000);

    // Kernel Imports
    RAMAddRegion(xbox->ram, 0x80000000, 512);
    *(uint32_t *)RAMRawPointer(xbox->ram, 0x800000A4) = 0x81000000; // xboxkrnl.exe::LaunchDataPage

}

// Runs XBE
void XboxRun(Xbox *xbox) {

    // Temp FS region, FS = 0
    RAMAddRegion(xbox->ram, 0x00000000, 0x1000);

    // Adding STACK
    RAMAddRegion(xbox->ram, 0x03F00000, 0x100000); // stack
    xbox->cpu->esp = 0x04000000;

    // Return Address
    xbox->cpu->esp -= 4;
    *(uint32_t *)RAMRawPointer(xbox->ram, xbox->cpu->esp) = 0x00000000;

    // Running
    printf("\n");
    CPURun(xbox, xbox->cpu, xbox->ram);
}