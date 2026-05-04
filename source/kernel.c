// Kernel
// Author: Evan Cassidy
// Date: 4/27/2026

#include "kernel.h"
#include "xbox.h"
#include "CPU.h"
#include "RAM.h"

// Intercepts Kernel calls by the CPU
void HandleKernelCall(Xbox *xbox, CPU *cpu, RAM *ram) {

    // Ordinal
    uint32_t ordinal = cpu->eip - 0x80000000;

    // Return Address
    uint32_t ret = RAMRead32(ram, cpu->esp);
    cpu->esp += 4;

    switch (ordinal) {

        // __stdcall ExQueryNonVolatileSetting (5 args)
        case 24: {
            printf("ExQueryNonVolatileSetting();\n");
            cpu->esp += 4 * 5;
            cpu->eax = 0;
            break;
        }

        // __stdcall HalRegisterShutdownNotification (2 args)
        case 47: {
            printf("HalRegisterShutdownNotification();\n");
            cpu->esp += 4 * 2;
            break;
        }

        // __stdcall HalReturnToFirmware (1 arg)
        case 49: {
            printf("HalReturnToFirmware();\n");
            cpu->esp += 4 * 1;
            break;
        }

        // __stdcall KeInitializeDpc (3 args)
        case 107: {
            printf("KeInitializeDpc();\n");
            cpu->esp += 4 * 3;
            break;
        }

        // __stdcall KeInitializeTimerEx (2 args)
        case 113: {
            printf("KeInitializeTimerEx();\n");
            cpu->esp += 4 * 2;
            break;
        }

        // __stdcall KeSetTimer (4 args)
        case 149: {
            printf("KeSetTimer();\n");
            cpu->esp += 4 * 4;
            cpu->eax = 0;
            break;
        }

        // __stdcall MmPersistContiguousMemory (3 args)
        case 178: {
            printf("MmPersistContiguousMemory();\n");
            cpu->esp += 4 * 3;
            break;
        }

        // __stdcall NtAllocateVirtualMemory (5 args)
        case 184: {
            printf("NtAllocateVirtualMemory();\n");
            cpu->esp += 4 * 5;
            cpu->eax = 0;
            break;
        }

        // __stdcall NtClose (1 arg)
        case 187: {
            printf("NtClose();\n");
            cpu->esp += 4 * 1;
            break;
        }

        // __stdcall PsCreateSystemThreadEx (10 args)
        case 255: {
            printf("PsCreateSystemThreadEx();\n");
            uint32_t StartRoutine = RAMRead32(ram, cpu->esp + 4 * 5);
            cpu->esp += 4 * 10;
            cpu->eax = 0;
            XboxCreateThread(xbox, StartRoutine);
            break;
        }

        // __stdcall RtlInitializeCriticalSection (1 arg)
        case 291: {
            printf("RtlInitializeCriticalSection();\n");
            cpu->esp += 4 * 1;
            break;
        }

        // __stdcall RtlNtStatusToDosError (1 arg)
        case 301: {
            printf("RtlNtStatusToDosError();\n");
            cpu->esp += 4 * 1;
            break;
        }

        // Unknown Kernel Call
        default:
            printf("Unknown kernel call %d\n", ordinal);
            CPUState(cpu);
            exit(1);

    }

    // Return
    cpu->eip = ret;
}