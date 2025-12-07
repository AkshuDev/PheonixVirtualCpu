// Author: Pheonix Studios/AkshuDev
#pragma once

#include <stdint.h>
#include <string.h>

#include <pvcpu-jit.h>

#define PVCPU_FLAGS_BP_VALID 0b0001 // PVCpu Flags Bit Position - Valid
#define PVCPU_FLAGS_BP_IMM 0b0010
#define PVCPU_FLAGS_BP_DISP64 0b0100
#define PVCPU_FLAGS_BP_EXT 0b1000

typedef struct {
    uint16_t opcode; // Actually 12bits, use lower
    uint8_t mode; // Actually 4bits, use lower
    uint8_t src; // Actually 6bits, use lower
    uint8_t dest; // Actually 6bits, use lower
    uint8_t flags; // Actually 4bits, use lower
} PVCpu_Inst;

typedef struct {
    uint64_t regs[40]; // NULL, G0-G30, LR, SF, SP, PC (Internal), I0-I3 (Internal), IP (Internal)
    uint8_t* memory;
    size_t memsize;
} Cpu_State;

typedef struct {
    uint16_t opcode; // Actually 12bits, use lower
    uint8_t extender; // Actually 4bits, use lower
} PVCpuC_Inst; // PVCpu-Compressed

inline static uint32_t pvpcu_pack_inst(PVCpu_Inst inst) {
    uint32_t packed_inst;
    packed_inst |= (inst.opcode & 0xFFF) << 20;
    packed_inst |= (inst.mode & 0xF) << 16;
    packed_inst |= (inst.src & 0x3F) << 10;
    packed_inst |= (inst.dest & 0x3F) << 4;
    packed_inst |= (inst.flags & 0xF);
    return packed_inst;
}

inline static size_t pvcpu_unpack_inst(const uint8_t* buf, size_t len, PVCpu_Inst* out, uint64_t* val_out, uint64_t* extflags_out, int* extflag_count) {
    if (len < 4) return 0;

    uint32_t w;
    memcpy(&w, buf, 4);
    out->opcode = (w >> 20) & 0xFFF;
    out->mode = (w >> 16) & 0xF;
    out->src = (w >> 10) & 0x3F;
    out->dest = (w >> 4) & 0x3F;
    out->flags = w & 0xF;

    size_t off = 4;

    if (!(out->flags & PVCPU_FLAGS_BP_VALID)) return 4;
    if (out->flags & PVCPU_FLAGS_BP_IMM) {
        if (off + 8 > len) return 0;
        memcpy(val_out, buf + off, 8);
        off += 8;
    } else if (out->flags & PVCPU_FLAGS_BP_DISP64) {
        if (off + 8 > len) return 0;
        memcpy(val_out, buf + off, 8);
        off += 8;
    }
    if (out->flags & PVCPU_FLAGS_BP_EXT) {
        while (off < len) {
            if (off + 8 > len) return 0;
            uint64_t mask;
            memcpy(&mask, buf + off, 8);

            extflags_out[*extflag_count] = mask;
            (*extflag_count)++;

            if (!(mask & PVCPU_FLAGS_BP_VALID)) break;
        }
    }

    return off;
}

inline static uint16_t pvpcu_c_pack_inst(PVCpuC_Inst inst) {
    uint16_t packed_inst;
    packed_inst |= (inst.opcode & 0xFFF) << 4;
    packed_inst |= (inst.extender & 0xF);
    return packed_inst;
}

