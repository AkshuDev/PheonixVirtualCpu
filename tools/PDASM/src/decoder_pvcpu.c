// Author: Pheonix Studios/AkshuDev

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

#include <extra.h>
#include <decoder_pvcpu.h>

#define FLAGS_VALID 0b0001
#define FLAGS_IMM 0b0010
#define FLAGS_DISP64 0b0100
#define FLAGS_EXTFLAGS 0b1000

typedef struct {
    uint16_t opcode;
    uint8_t mode;
    uint8_t src;
    uint8_t dest;
    uint8_t flags;

    uint64_t imm;
    uint64_t disp64;
    uint64_t extflags;
    uint64_t extextflags;
    uint64_t advflags;
} Inst;

typedef enum {
    NULL_MODE = 0, // No mode
    REG_REG, // dest = src
    REG_IMM, // src is actually a imm! dest is a reg (dest = src (as imm))
    REG_EXTIMM, // Allows use of Bit 1 of Flags (dest = imm)
    REG_DISP, // Allows use of Bit 2 of Flags (dest = mem[disp + PC])
    LOAD_REGADDR, // dest = mem[src]
    LOAD_IMMADDR, // dest = mem[imm]
    LOAD_PC_REL, // dest = mem[src (as offset) + PC]
    STORE_REGADDR, // mem[dest] = src
    STORE_IMMADDR, // mem[imm] = src
    STORE_PC_REL, // mem[dest (as offset) + PC] = src
    // Special
    SYSCALL_REG, // syscall(src)
    SYSCALL_IMM, // syscall(imm)
} Modes;

static HashMap16 opcodes[] = {
    {0x0, "nop"},
    // ALU
    {0x1, "add"}, {0x2, "sub"}, {0x3, "mul"}, {0x4, "div"}, {0x5, "cmp"},
    // Memory
    {0x100, "load"}, {0x101, "store"},
    // Movement
    {0x150, "mov"}
};

static size_t opcodes_count = sizeof(opcodes) / sizeof(opcodes[0]);

static const char* decode_reg(uint8_t reg) {
    switch (reg) {
        case 0: return "NULL";
        case 1: return "%g0";
        case 2: return "%g1";
        case 3: return "%g2";
        case 4: return "%g3";
        case 5: return "%g4";
        case 6: return "%g5";
        case 7: return "%g6";
        case 8: return "%g7";
        case 9: return "%g8";
        case 10: return "%g9";
        case 11: return "%g10";
        case 12: return "%g11";
        case 13: return "%g12";
        case 14: return "%g13";
        case 15: return "%g14";
        case 16: return "%g15";
        case 17: return "%g16";
        case 18: return "%g17";
        case 19: return "%g18";
        case 20: return "%g19";
        case 21: return "%g20";
        case 22: return "%g21";
        case 23: return "%g22";
        case 24: return "%g23";
        case 25: return "%g24";
        case 26: return "%g25";
        case 27: return "%g26";
        case 28: return "%g27";
        case 29: return "%g28";
        case 30: return "%g29";
        case 31: return "%g30";
        case 32: return "%lr";
        case 33: return "%sf";
        case 34: return "%sp";
        case 35: return "%pc";
        case 36: return "%i0";
        case 37: return "%i1";
        case 38: return "%i2";
        case 39: return "%ip";
        default: return "(invalid)";
    }
}

void decode_pvcpu(uint8_t* data, size_t* offset, size_t cvaddr, char* out, size_t outsz) {
    size_t og_offset = *offset;

    Inst inst = {0};
    uint32_t inst_raw;
    memcpy(&inst_raw, &data[*offset], sizeof(inst_raw));

    uint16_t op_mode = (inst_raw & 0b11111111111111110000000000000000) >> 16;
    uint16_t opcode = (op_mode & 0b1111111111110000) >> 4;
    uint8_t mode = (op_mode & 0b0000000000001111);
    inst.opcode = opcode;
    inst.mode = mode;

    uint16_t reg_flags = (inst_raw & 0b00000000000000001111111111111111);
    uint8_t src = (reg_flags & 0b1111110000000000) >> 10;
    uint8_t dest = (reg_flags & 0b0000001111110000) >> 4;
    uint8_t flags = (reg_flags & 0b0000000000001111);

    inst.src = src;
    inst.dest = dest;
    inst.flags = flags;

    *offset += 4;

    if (!(flags & FLAGS_VALID)) {
        strcpy(out, CB_RED "(invalid) ");
    } else {
        if (flags & FLAGS_IMM) {
            uint64_t imm;
            memcpy(&imm, &data[*offset], sizeof(imm));
            inst.imm = imm;
            *offset += 8;
        }
        if (flags & FLAGS_DISP64) {
            uint64_t disp64;
            memcpy(&disp64, &data[*offset], sizeof(disp64));
            inst.disp64 = disp64;
            *offset += 8;
        }
        if (flags & FLAGS_EXTFLAGS) {
            uint64_t extflags;
            memcpy(&extflags, &data[*offset], sizeof(extflags));
            inst.extflags = extflags;
            *offset += 8;
        }
    }

    size_t out_off = strlen(out);
    snprintf(out + out_off, outsz - out_off, CB_RED "%s ", get_value_hashmap16(opcodes, inst.opcode, opcodes_count));
    out_off = strlen(out);
    switch (mode) {
        case NULL_MODE: break;
        case REG_REG: snprintf(out + out_off, outsz - out_off, CB_CYAN "%s, %s ", decode_reg(inst.dest), decode_reg(inst.src)); break;
        case REG_IMM: snprintf(out + out_off, outsz - out_off, CB_CYAN "%s, %d ", decode_reg(inst.dest), (int)inst.src); break;
        case REG_EXTIMM: snprintf(out + out_off, outsz - out_off, CB_CYAN "%s, %lld ", decode_reg(inst.dest), (long long int)inst.imm); break;
        case REG_DISP: snprintf(out + out_off, outsz - out_off, CB_CYAN "%s, [0x%llx] // Disp64, 0x%llx ", decode_reg(inst.dest), (unsigned long long int)((size_t)inst.disp64 + cvaddr), (unsigned long long int)inst.disp64); break;
        case LOAD_REGADDR: snprintf(out + out_off, outsz - out_off, CB_CYAN "%s, [%x] ", decode_reg(inst.dest), (unsigned int)inst.src); break;
        case LOAD_IMMADDR: snprintf(out + out_off, outsz - out_off, CB_CYAN "%s, [%llx] ", decode_reg(inst.dest), (unsigned long long int)inst.imm); break;
        case LOAD_PC_REL: snprintf(out + out_off, outsz - out_off, CB_CYAN "%s, [%x + %llx] ", decode_reg(inst.dest), (int)inst.src, (unsigned long long int)cvaddr); break;
        case STORE_REGADDR: snprintf(out + out_off, outsz - out_off, CB_CYAN "[%x], %s ", (unsigned int)inst.src, decode_reg(inst.src)); break;
        case STORE_IMMADDR: snprintf(out + out_off, outsz - out_off, CB_CYAN "[%llx], %s ", (unsigned long long int)inst.imm, decode_reg(inst.src)); break;
        case STORE_PC_REL: snprintf(out + out_off, outsz - out_off, CB_CYAN "[%x + %llx], %s ", (int)inst.src, (unsigned long long int)cvaddr, decode_reg(inst.src)); break;
        case SYSCALL_REG: snprintf(out + out_off, outsz - out_off, CB_CYAN "syscall %u ", (unsigned int)inst.src); break;
        case SYSCALL_IMM: snprintf(out + out_off, outsz - out_off, CB_CYAN "syscall %llu ", (unsigned long long int)inst.imm); break;
        default: snprintf(out + out_off, outsz - out_off, CB_CYAN "(invalid) "); break;
    }

    out_off = strlen(out);

    snprintf(out + out_off, outsz - out_off, C_WHITE "\t\t\t - %02X  ", (uint8_t)data[og_offset]);
    for (size_t i = 1; i < (*offset - og_offset); i++) {
        out_off = strlen(out);
        snprintf(out + out_off, outsz - out_off, "%02X  ", (uint8_t)data[og_offset + i]);
    }

    return;
}
