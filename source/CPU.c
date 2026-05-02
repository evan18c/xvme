// CPU Structure
// Author: Evan Cassidy
// Date: 4/22/2026

#include "CPU.h"
#include "xbox.h"
#include "RAM.h"
#include "kernel.h"

// Private function prototypes
uint32_t ALU(CPU *cpu, uint8_t op, uint32_t a, uint32_t b);
uint32_t read_rm32(CPU *cpu, RAM *ram, uint32_t *reg_ptrs[8], uint8_t mod, uint8_t rm);
void write_rm32(CPU *cpu, RAM *ram, uint32_t *reg_ptrs[8], uint8_t mod, uint8_t rm, uint32_t val);
uint32_t calc_addr(CPU *cpu, RAM *ram, uint32_t *reg_ptrs[8], uint8_t mod, uint8_t rm);
uint8_t *get_reg8_ptr(CPU *cpu, uint8_t rm);
void write_rm8(CPU *cpu, RAM *ram, uint32_t *reg_ptrs[8], uint8_t mod, uint8_t rm, uint8_t val);
void write_rm16(CPU *cpu, RAM *ram, uint32_t *reg_ptrs[8], uint8_t mod, uint8_t rm, uint16_t val);
uint16_t read_rm16(CPU *cpu, RAM *ram, uint32_t *reg_ptrs[8], uint8_t mod, uint8_t rm);

void CPUState(CPU *cpu) {
    printf("EAX: 0x%08X EBX: 0x%08X ECX: 0x%08X EDX: 0x%08X\n", cpu->eax, cpu->ebx, cpu->ecx, cpu->edx);
    printf("ESI: 0x%08X EDI: 0x%08X\n", cpu->esi, cpu->edi);
    printf("EBP: 0x%08X ESP: 0x%08X\n", cpu->ebp, cpu->esp);
    printf("EIP: 0x%08X\n", cpu->eip);
    printf("ZF: %hhX SF: %hhX CF: %hhX OF: %hhX\n", cpu->ZF, cpu->SF, cpu->CF, cpu->OF);
    printf("FS: 0x%08X GS: 0x%08X\n", cpu->FS, cpu->GS);
    printf("COUNTER: %llu\n", cpu->counter);
}

void CPURun(Xbox *xbox, CPU *cpu, RAM *ram) {

    // Init
    uint32_t *reg_ptrs[8] = {&cpu->eax, &cpu->ecx, &cpu->edx, &cpu->ebx, &cpu->esp, &cpu->ebp, &cpu->esi, &cpu->edi};
    uint8_t modrm, mod, reg, rm;
    uint32_t a, b, temp;
    uint32_t imm32;
    uint32_t addr;
    int32_t rel32;
    uint8_t imm8;
    int32_t rel8;
    uint32_t ret;
    uint32_t dest;
    uint16_t imm16;

    // Prefixes
    uint8_t prefix_rep;
    uint8_t prefix_fs;

    while (1) {

        // Process Exit
        if (!cpu->eip) {
            printf("Thread finished.\n");
            CPUState(cpu);
            break;
        }

        // Kernel Call
        if (cpu->eip >= 0x80000000 && cpu->eip < 0x80000200) {
            HandleKernelCall(xbox, cpu, ram);
            continue;
        }

        // Prefixes
        prefix_rep = 0;
        prefix_fs = 0;
        while (1) {
            uint8_t prefix = RAMReadByte(ram, cpu->eip);

            if (prefix == 0x64) {
                prefix_fs = 1;
                cpu->eip++;
                continue;
            }

            if (prefix == 0xF3) {
                prefix_rep = 1;
                cpu->eip++;
                continue;
            }

            break;
        }

        // Standard Opcode
        uint8_t opcode = RAMReadByte(ram, cpu->eip++);
        printf("%08X: %hhX\n", cpu->eip - 1, opcode);

        switch (opcode) {

            // ADD r32, r/m32
            case 0x03:
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                a = *reg_ptrs[reg];
                b = read_rm32(cpu, ram, reg_ptrs, mod, rm);

                *reg_ptrs[reg] = ALU(cpu, 0, a, b);
                break;

            // ADD EAX, imm32
            case 0x05:
                a = cpu->eax;
                b = RAMRead32(ram, cpu->eip);
                cpu->eip += 4;
                cpu->eax = ALU(cpu, 0, a, b);
                break;

            // 2 byte opcodes
            case 0x0F: {

                uint8_t opcode2 = RAMReadByte(ram, cpu->eip++);

                switch (opcode2) {

                    // SETNZ
                    case 0x95:
                        modrm = RAMReadByte(ram, cpu->eip++);
                        mod = (modrm >> 6) & 3;
                        reg = (modrm >> 3) & 7;
                        rm = modrm & 7;

                        write_rm8(cpu, ram, reg_ptrs, mod, rm, !cpu->ZF);
                        break;

                    // MOVZX r32, r/m16
                    case 0xB7:
                        modrm = RAMReadByte(ram, cpu->eip++);
                        mod = (modrm >> 6) & 3;
                        reg = (modrm >> 3) & 7;
                        rm = modrm & 7;

                        *reg_ptrs[reg] = (uint32_t)read_rm16(cpu, ram, reg_ptrs, mod, rm);
                        break;

                    // Unsupported
                    default:
                        printf("Unknown Two-Byte Instruction 0x0F 0x%hhX at 0x%08X\n", opcode2, cpu->eip - 1);
                        CPUState(cpu);
                        exit(1);
                }

            }
            break;

            // AND AL, imm8
            case 0x24: {
                uint8_t result = *get_reg8_ptr(cpu, 0) & RAMReadByte(ram, cpu->eip++);
                *get_reg8_ptr(cpu, 0) = result;

                cpu->ZF = (result == 0);
                cpu->SF = (result >> 7) & 1;
                cpu->CF = 0;
                cpu->OF = 0;
                break;
            }

            // SUB r32, r/m32
            case 0x2B:
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                a = *reg_ptrs[reg];
                b = read_rm32(cpu, ram, reg_ptrs, mod, rm);

                *reg_ptrs[reg] = ALU(cpu, 5, a, b);

                break;

            // XOR r32, r/m32
            case 0x33:
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                a = *reg_ptrs[reg];
                b = read_rm32(cpu, ram, reg_ptrs, mod, rm);

                *reg_ptrs[reg] = ALU(cpu, 6, a, b);
                break;

            // CMP r/m32, r32
            case 0x39:
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                a = read_rm32(cpu, ram, reg_ptrs, mod, rm);
                b = *reg_ptrs[reg];

                ALU(cpu, 7, a, b);

                break;
            
            // CMP r32, r/m32
            case 0x3B:
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                a = *reg_ptrs[reg];
                b = read_rm32(cpu, ram, reg_ptrs, mod, rm);

                ALU(cpu, 7, a, b);

                break;

            // INC r32
            case 0x40:
            case 0x41:
            case 0x42:
            case 0x43:
            case 0x44:
            case 0x45:
            case 0x46:
            case 0x47:
                a = *reg_ptrs[opcode - 0x40];
                b = 1;
                temp = a + b;
                *reg_ptrs[opcode - 0x40] = temp;
                cpu->ZF = (temp == 0);
                cpu->SF = (temp >> 31) & 1;
                cpu->OF = ((~(a ^ b) & (a ^ temp)) >> 31) & 1;
                break;
            
            // DEC r32
            case 0x48:
            case 0x49:
            case 0x4A:
            case 0x4B:
            case 0x4C:
            case 0x4D:
            case 0x4E:
            case 0x4F:
                a = *reg_ptrs[opcode - 0x48];
                b = 1;
                temp = a - b;
                *reg_ptrs[opcode - 0x48] = temp;
                cpu->ZF = (temp == 0);
                cpu->SF = (temp >> 31) & 1;
                cpu->OF = (((a ^ b) & (a ^ temp)) >> 31) & 1;
                break;

            // PUSH r32
            case 0x50:
            case 0x51:
            case 0x52:
            case 0x53:
            case 0x54:
            case 0x55:
            case 0x56:
            case 0x57:
                cpu->esp -= 4;
                RAMWrite32(ram, cpu->esp, *reg_ptrs[opcode - 0x50]);
                break;

            // POP r32
            case 0x58:
            case 0x59:
            case 0x5A:
            case 0x5B:
            case 0x5C:
            case 0x5D:
            case 0x5E:
            case 0x5F:
                *reg_ptrs[opcode - 0x58] = RAMRead32(ram, cpu->esp);
                cpu->esp += 4;
                break;

            // PUSH imm32
            case 0x68:
                imm32 = RAMRead32(ram, cpu->eip);
                cpu->eip += 4;
                cpu->esp -= 4;
                RAMWrite32(ram, cpu->esp, imm32);
                break;

            // PUSH imm8
            case 0x6A: {
                imm8 = RAMReadByte(ram, cpu->eip++);
                cpu->esp -= 4;
                int32_t val = (int8_t)imm8;
                RAMWrite32(ram, cpu->esp, val);
                break;
            }

            // JB rel8
            // JC rel8
            // JNAE rel8
            case 0x72:
                rel8 = (int8_t)RAMReadByte(ram, cpu->eip++);
                if (cpu->CF) cpu->eip += rel8;
                break;

            // JAE rel8
            // JNB rel8
            // JNC rel8
            case 0x73:
                rel8 = (int8_t)RAMReadByte(ram, cpu->eip++);
                if (!cpu->CF) cpu->eip += rel8;
                break;

            // JE rel8
            // JZ rel8
            case 0x74:
                rel8 = (int8_t)RAMReadByte(ram, cpu->eip++);
                if (cpu->ZF) cpu->eip += rel8;
                break;

            // JNE rel8
            // JNZ rel8
            case 0x75:
                rel8 = (int8_t)RAMReadByte(ram, cpu->eip++);
                if (!cpu->ZF) cpu->eip += rel8;
                break;
            
            // JL rel8
            case 0x7C:
                rel8 = (int8_t)RAMReadByte(ram, cpu->eip++);
                if (cpu->SF != cpu->OF) cpu->eip += rel8;
                break;

            // JGE rel8
            case 0x7D:
                rel8 = (int8_t)RAMReadByte(ram, cpu->eip++);
                if (cpu->SF == cpu->OF) cpu->eip += rel8;
                break;

            // JLE rel8
            case 0x7E:
                rel8 = (int8_t)RAMReadByte(ram, cpu->eip++);
                if (cpu->ZF || cpu->SF != cpu->OF) cpu->eip += rel8;
                break;
            
            // JG rel8
            case 0x7F:
                rel8 = (int8_t)RAMReadByte(ram, cpu->eip++);
                if (!cpu->ZF && cpu->SF == cpu->OF) cpu->eip += rel8;
                break;

            // ADD, OR, ADC, SBB, AND, SUB, XOR, CMP r/m32, imm32
            case 0x81:
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                addr = 0;
                if (mod != 3) addr = calc_addr(cpu, ram, reg_ptrs, mod, rm);

                a = (mod == 3) ? *reg_ptrs[rm] : RAMRead32(ram, addr);

                b = RAMRead32(ram, cpu->eip);
                cpu->eip += 4;

                temp = ALU(cpu, reg, a, b);

                if (reg != 7) {
                    if (mod == 3) *reg_ptrs[rm] = temp;
                    else RAMWrite32(ram, addr, temp);
                }

                break;

            // ADD, SUB, CMP, AND, OR, XOR r/m32, imm8
            case 0x83: {
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                addr = 0;
                if (mod != 3) {
                    addr = calc_addr(cpu, ram, reg_ptrs, mod, rm);
                }

                a = (mod == 3) ? *reg_ptrs[rm] : RAMRead32(ram, addr);
                b = (int32_t)(int8_t)RAMReadByte(ram, cpu->eip++);

                temp = ALU(cpu, reg, a, b);

                // not CMP
                if (reg != 7) {
                    if (mod == 3) *reg_ptrs[rm] = temp;
                    else RAMWrite32(ram, addr, temp);
                }

                break;
            }

            // TEST r/m32, r32
            case 0x85:
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                a = read_rm32(cpu, ram, reg_ptrs, mod, rm);
                b = *reg_ptrs[reg];

                ALU(cpu, 4, a, b); // AND
                
                break;
            
            // MOV r/m32, r32
            case 0x89:
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                write_rm32(cpu, ram, reg_ptrs, mod, rm, *reg_ptrs[reg]);

                break;

            // MOV r32, r/m32
            case 0x8B:
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                *reg_ptrs[reg] = read_rm32(cpu, ram, reg_ptrs, mod, rm);

                break;
            
            // LEA r32, m
            case 0x8D:
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                addr = calc_addr(cpu, ram, reg_ptrs, mod, rm);
                *reg_ptrs[reg] = addr;
                break;

            // NOP
            case 0x90:
                break;
            
            // CWD CDQ
            case 0x99:
                cpu->edx = (cpu->eax & 0x80000000) ? 0xFFFFFFFF : 0x00000000;
                break;

            // MOV EAX, moffs32
            case 0xA1:
                imm32 = RAMRead32(ram, cpu->eip);
                cpu->eip += 4;
                cpu->eax = RAMRead32(ram, imm32);
                break;
            
            // MOV moffs32, EAX
            case 0xA3:
                imm32 = RAMRead32(ram, cpu->eip);
                cpu->eip += 4;
                RAMWrite32(ram, imm32, cpu->eax);
                break;

            // MOVSD
            case 0xA5:
                if (prefix_rep) {
                    while (cpu->ecx != 0) {
                        RAMWrite32(ram, cpu->edi, RAMRead32(ram, cpu->esi));
                        cpu->edi += 4;
                        cpu->esi += 4;
                        cpu->ecx--;
                    }
                } else {
                    RAMWrite32(ram, cpu->edi, RAMRead32(ram, cpu->esi));
                    cpu->edi += 4;
                    cpu->esi += 4;
                }
                break;
            
            // STOSD
            case 0xAB:
                if (prefix_rep) {
                    while (cpu->ecx != 0) {
                        RAMWrite32(ram, cpu->edi, cpu->eax);
                        cpu->edi += 4;
                        cpu->ecx--;
                    }
                } else {
                    RAMWrite32(ram, cpu->edi, cpu->eax);
                    cpu->edi += 4;
                }
                break;

            // MOV r32, imm32
            case 0xB8:
            case 0xB9:
            case 0xBA:
            case 0xBB:
            case 0xBC:
            case 0xBD:
            case 0xBE:
            case 0xBF:
                imm32 = RAMRead32(ram, cpu->eip);
                cpu->eip += 4;
                *reg_ptrs[opcode - 0xB8] = imm32;
                break;

            // SHR r/m32, imm8
            case 0xC1: {
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                addr = 0;
                if (mod != 3) addr = calc_addr(cpu, ram, reg_ptrs, mod, rm);

                uint32_t val = (mod == 3) ? *reg_ptrs[rm] : RAMRead32(ram, addr);
                uint8_t imm8 = RAMReadByte(ram, cpu->eip++) & 0x1F;
                uint32_t result = val;

                switch (reg) {

                    // SHR
                    case 5:
                        if (imm8 != 0) {
                            result = val >> imm8;
                            cpu->CF = (val >> (imm8 - 1)) & 1;
                            cpu->ZF = (result == 0);
                            cpu->SF = (result >> 31) & 1;
                        }
                        break;

                    default:
                        printf("Unspported 0xC1 /%hhx\n", reg);
                        CPUState(cpu);
                        exit(1);

                }

                if (mod == 3) *reg_ptrs[rm] = result;
                else RAMWrite32(ram, addr, result);

                break;

            }

            // RET imm16
            case 0xC2:
                imm16 = RAMReadByte(ram, cpu->eip) | (RAMReadByte(ram, cpu->eip + 1) << 8);
                cpu->eip += 2;

                ret = RAMRead32(ram, cpu->esp);
                cpu->esp += 4;

                cpu->esp += imm16;
                cpu->eip = ret;
                break;

            // RET
            case 0xC3:
                ret = RAMRead32(ram, cpu->esp);
                cpu->esp += 4;
                cpu->eip = ret;
                break;

            // MOV r/m32, imm32
            case 0xC7:
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                addr = 0;
                if (mod != 3) addr = calc_addr(cpu, ram, reg_ptrs, mod, rm);

                imm32 = RAMRead32(ram, cpu->eip);
                cpu->eip += 4;

                if (mod == 3) *reg_ptrs[rm] = imm32;
                else RAMWrite32(ram, addr, imm32);
                break;
            
            // LEAVE
            case 0xC9:
                cpu->esp = cpu->ebp;
                cpu->ebp = RAMRead32(ram, cpu->esp);
                cpu->esp += 4;
                break;

            // INT3
            case 0xCC:
                printf("INT3\n");
                CPUState(cpu);
                exit(0);
            
            // SHR r/m32, 1
            case 0xD1: {
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                addr = 0;
                if (mod != 3) addr = calc_addr(cpu, ram, reg_ptrs, mod, rm);

                uint32_t val = (mod == 3) ? *reg_ptrs[rm] : RAMRead32(ram, addr);
                uint32_t result = val;

                switch (reg) {

                    // SHR
                    case 5:
                        result = val >> 1;
                        cpu->CF = val & 1;
                        cpu->ZF = (result == 0);
                        cpu->SF = (result >> 31) & 1;
                        break;

                    default:
                        printf("Unspported 0xC1 /%hhx\n", reg);
                        CPUState(cpu);
                        exit(1);

                }

                if (mod == 3) *reg_ptrs[rm] = result;
                else RAMWrite32(ram, addr, result);

                break;

            }

            // CALL rel32
            case 0xE8:
                rel32 = (int32_t)RAMRead32(ram, cpu->eip);
                cpu->eip += 4;
                cpu->esp -= 4;
                RAMWrite32(ram, cpu->esp, cpu->eip);
                cpu->eip += rel32;
                break;

            // JMP rel32
            case 0xE9:
                rel32 = (int32_t)RAMRead32(ram, cpu->eip);
                cpu->eip += 4;
                cpu->eip += rel32;
                break;

            // JMP rel8
            case 0xEB:
                rel8 = (int8_t)RAMReadByte(ram, cpu->eip++);
                cpu->eip += rel8;
                break;
            
            // TEST, NOT, NEG, MUL, IMUL, DIV, IDIV r/m8
            case 0xF6:
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                switch (reg) {

                    // TEST
                    case 0:
                        if (mod == 3) {
                            uint8_t a = *get_reg8_ptr(cpu, rm);
                            uint8_t b = RAMReadByte(ram, cpu->eip++);
                            uint8_t result = a & b;
                            cpu->ZF = (result == 0);
                            cpu->SF = (result >> 7) & 1;
                            cpu->CF = 0;
                            cpu->OF = 0;
                        } else {
                            uint32_t addr = calc_addr(cpu, ram, reg_ptrs, mod, rm);
                            uint8_t a = RAMReadByte(ram, addr);
                            uint8_t b = RAMReadByte(ram, cpu->eip++);
                            uint8_t result = a & b;
                            cpu->ZF = (result == 0);
                            cpu->SF = (result >> 7) & 1;
                            cpu->CF = 0;
                            cpu->OF = 0;
                        }
                        break;

                    // NOT
                    case 2:
                        if (mod == 3) {
                            *get_reg8_ptr(cpu, rm) ^= 0xFF;
                        } else {
                            addr = calc_addr(cpu, ram, reg_ptrs, mod, rm);
                            RAMWriteByte(ram, addr, RAMReadByte(ram, addr) ^ 0xFF);
                        }
                        break;
                    
                    default:
                        printf("Unsupported 0xF6 /%hhX\n", reg);
                        CPUState(cpu);
                        exit(1);
                }

                break;


            // TEST, TEST, NOT, NEG, MUL, IMUL, DIV, IDIV
            case 0xF7:
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                addr = 0;
                if (mod != 3) addr = calc_addr(cpu, ram, reg_ptrs, mod, rm);
                a = (mod == 3) ? *reg_ptrs[rm] : RAMRead32(ram, addr);

                switch (reg) {

                    // NEG
                    case 3:
                        temp = 0 - a;
                        if (mod == 3) *reg_ptrs[rm] = temp;
                        else RAMWrite32(ram, addr, temp);
                        cpu->ZF = (temp == 0);
                        cpu->SF = (temp >> 31) & 1;
                        cpu->CF = (a != 0);
                        cpu->OF = (a == 0x80000000);
                        break;

                    // IDIV
                    case 7: {
                        int64_t dividend = ((int64_t)(int32_t)cpu->edx << 32) | (uint32_t)cpu->eax;
                        int32_t divisor = (int32_t)a;

                        int64_t quotient = dividend / divisor;
                        int64_t remainder = dividend % divisor;

                        cpu->eax = (uint32_t)(int32_t)quotient;
                        cpu->edx = (uint32_t)(int32_t)remainder;

                        break;
                    }


                    default:
                        printf("Unspported 0xF7 /%hhX\n", reg);
                        CPUState(cpu);
                        exit(1);

                }
                break;

            // INC, DEC, CALL, CALLF, JMP, JMPF, PUSH r/m32
            case 0xFF:
                modrm = RAMReadByte(ram, cpu->eip++);
                mod = (modrm >> 6) & 3;
                reg = (modrm >> 3) & 7;
                rm = modrm & 7;

                switch (reg) {

                    // CALL
                    case 2:
                        dest = read_rm32(cpu, ram, reg_ptrs, mod, rm);
                        cpu->esp -= 4;
                        RAMWrite32(ram, cpu->esp, cpu->eip);
                        cpu->eip = dest;
                        break;
                    
                    // JMP
                    case 4:
                        dest = read_rm32(cpu, ram, reg_ptrs, mod, rm);
                        cpu->eip = dest;
                        break;

                    // PUSH
                    case 6: {
                        uint32_t val = read_rm32(cpu, ram, reg_ptrs, mod, rm);
                        cpu->esp -= 4;
                        RAMWrite32(ram, cpu->esp, val);
                        break;
                    }

                    default:
                        printf("Unsupported 0xFF /%hhX\n", reg);
                        CPUState(cpu);
                        exit(1);

                }
                break;

            // UNKNOWN
            default:
                printf("Unknown Instruction 0x%hhX at 0x%08X\n", opcode, cpu->eip - 1);
                CPUState(cpu);
                exit(1);

        }

        // Instruction Counter
        cpu->counter++;
    }

}

uint32_t ALU(CPU *cpu, uint8_t op, uint32_t a, uint32_t b) {
    uint32_t temp = 0;

    switch (op) {

        // ADD
        case 0:
            temp = a + b;
            cpu->ZF = (temp == 0);
            cpu->SF = (temp >> 31) & 1;
            cpu->CF = (temp < a);
            cpu->OF = ((~(a ^ b) & (a ^ temp)) >> 31) & 1;
            break;

        // OR
        case 1:
            temp = a | b;
            cpu->ZF = (temp == 0);
            cpu->SF = (temp >> 31) & 1;
            cpu->CF = 0;
            cpu->OF = 0;
            break;

        // AND
        case 4:
            temp = a & b;
            cpu->ZF = (temp == 0);
            cpu->SF = (temp >> 31) & 1;
            cpu->CF = 0;
            cpu->OF = 0;
            break;

        // SUB
        case 5:
            temp = a - b;
            cpu->ZF = (temp == 0);
            cpu->SF = (temp >> 31) & 1;
            cpu->CF = (a < b);
            cpu->OF = (((a ^ b) & (a ^ temp)) >> 31) & 1;
            break;

        // XOR
        case 6:
            temp = a ^ b;
            cpu->ZF = (temp == 0);
            cpu->SF = (temp >> 31) & 1;
            cpu->CF = 0;
            cpu->OF = 0;
            break;

        // CMP
        case 7:
            temp = a - b;
            cpu->ZF = (temp == 0);
            cpu->SF = (temp >> 31) & 1;
            cpu->CF = (a < b);
            cpu->OF = (((a ^ b) & (a ^ temp)) >> 31) & 1;
            break;
        
        // Unsupported
        default:
            printf("Unsupported ALU operation %hhX\n", op);
            CPUState(cpu);
            exit(1);
    }

    return temp;
}

// Reads from R/M address
uint32_t read_rm32(CPU *cpu, RAM *ram, uint32_t *reg_ptrs[8], uint8_t mod, uint8_t rm) {
    if (mod == 3) {
        return *reg_ptrs[rm];
    }

    uint32_t addr = calc_addr(cpu, ram, reg_ptrs, mod, rm);
    return RAMRead32(ram, addr);
}

// Writes to R/M address
void write_rm32(CPU *cpu, RAM *ram, uint32_t *reg_ptrs[8], uint8_t mod, uint8_t rm, uint32_t val) {
    if (mod == 3) {
        *reg_ptrs[rm] = val;
        return;
    }

    uint32_t addr = calc_addr(cpu, ram, reg_ptrs, mod, rm);

    RAMWrite32(ram, addr, val);
}

// Calculates address for R/M when mod != 3
uint32_t calc_addr(CPU *cpu, RAM *ram, uint32_t *reg_ptrs[8], uint8_t mod, uint8_t rm) {

    // SIB byte
    if (rm == 4) {

        // sib
        uint8_t sib = RAMReadByte(ram, cpu->eip++);

        // decode
        uint8_t scale = (sib >> 6) & 3;
        uint8_t index = (sib >> 3) & 7;
        uint8_t base = sib & 7;

        // scale
        uint32_t scale_val = 1 << scale;

        // index
        uint32_t index_val;
        if (index == 4) {
            index_val = 0;
        } else {
            index_val = *reg_ptrs[index];
        }

        // base
        uint32_t base_val;
        if (mod == 0 && base == 5) {
            uint32_t udisp = RAMRead32(ram, cpu->eip);
            cpu->eip += 4;
            base_val = (int32_t)udisp;
        } else {
            base_val = *reg_ptrs[base];
        }

        // disp
        int32_t disp = 0;
        if (mod == 1) {
            disp = (int8_t)RAMReadByte(ram, cpu->eip++);
        } else if (mod == 2) {
            uint32_t udisp = RAMRead32(ram, cpu->eip);
            cpu->eip += 4;
            disp = (int32_t)udisp;
        }

        // calculate
        return base_val + index_val * scale_val + disp;
    }

    if (mod == 0) {

        // disp32
        if (rm == 5) {
            int32_t disp32 = (int32_t)RAMRead32(ram, cpu->eip);
            cpu->eip += 4;
            return disp32;
        }

        return *reg_ptrs[rm];

    }

    // disp8
    if (mod == 1) {
        int8_t disp8 = (int8_t)RAMReadByte(ram, cpu->eip++);
        return *reg_ptrs[rm] + disp8;
    }

    // disp32
    if (mod == 2) {
        int32_t disp32 = (int32_t)RAMRead32(ram, cpu->eip);
        cpu->eip += 4;
        return *reg_ptrs[rm] + disp32; 
    }

    printf("Invalid mod in calc_addr\n");
    exit(1);
}

// rm -> reg *
uint8_t *get_reg8_ptr(CPU *cpu, uint8_t rm) {
    switch (rm) {
        case 0: return (uint8_t *)&cpu->eax; // AL
        case 1: return (uint8_t *)&cpu->ecx; // CL
        case 2: return (uint8_t *)&cpu->edx; // DL
        case 3: return (uint8_t *)&cpu->ebx; // BL

        case 4: return ((uint8_t *)&cpu->eax) + 1; // AH
        case 5: return ((uint8_t *)&cpu->ecx) + 1; // CH
        case 6: return ((uint8_t *)&cpu->edx) + 1; // DH
        case 7: return ((uint8_t *)&cpu->ebx) + 1; // BH
    }
}

// Writes to R/M address (1 byte)
void write_rm8(CPU *cpu, RAM *ram, uint32_t *reg_ptrs[8], uint8_t mod, uint8_t rm, uint8_t val) {
    if (mod == 3) {
        *get_reg8_ptr(cpu, rm) = val;
        return;
    }
    uint32_t addr = calc_addr(cpu, ram, reg_ptrs, mod, rm);
    RAMWriteByte(ram, addr, val);
}

// Writes to R/M address (2 byte)
void write_rm16(CPU *cpu, RAM *ram, uint32_t *reg_ptrs[8], uint8_t mod, uint8_t rm, uint16_t val) {
    if (mod == 3) {
        *(uint16_t *)reg_ptrs[rm] = val;
        return;
    }
    uint32_t addr = calc_addr(cpu, ram, reg_ptrs, mod, rm);
    RAMWriteByte(ram, addr, val & 255);
    RAMWriteByte(ram, addr + 1, (val >> 8) & 255);
}

// Reads from R/M address
uint16_t read_rm16(CPU *cpu, RAM *ram, uint32_t *reg_ptrs[8], uint8_t mod, uint8_t rm) {
    if (mod == 3) {
        return *(uint16_t *)reg_ptrs[rm];
    }

    uint32_t addr = calc_addr(cpu, ram, reg_ptrs, mod, rm);
    return RAMReadByte(ram, addr) | (RAMReadByte(ram, addr + 1) << 8);
}