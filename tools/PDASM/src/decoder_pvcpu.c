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

HashMap8 opcodes[] = {
    {0x0, "nop"},
    {0x1, "add"}
};

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

void decode_pvcpu(uint8_t* data, size_t* offset, size_t cvaddr, char* out) {
    Inst inst = {0};

    uint16_t op_mode;
    memcpy(&op_mode, &data[*offset], sizeof(op_mode));
    uint16_t opcode = (op_mode & 0b1111111111110000) >> 4;
    uint8_t mode = (op_mode & 0b0000000000001111);
    inst.opcode = opcode;
    inst.mode = mode;

    *offset += 2;

    uint16_t reg_flags;
    memcpy(&reg_flags, &data[*offset], sizeof(reg_flags));
    uint8_t src = (reg_flags & 0b1111110000000000) >> 10;
    uint8_t dest = (reg_flags & 0b0000001111110000) >> 4;
    uint8_t flags = (reg_flags & 0b0000000000001111);

    inst.src = src;
    inst.dest = dest;
    inst.flags = flags;

    *offset += 2;

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
    switch (mode) {
        case MODE_REGREG: snprintf(out + out_off, sizeof(out) - out_off, CB_CYAN "%s, %s", decode_reg(dest), decode_reg(src));
    }
}
