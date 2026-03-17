// Author: Pheonix Studios/AkshuDev
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define PVCPU_FLAGS_BP_Valid 0b0001 // PVCpu Flags Bit Position - Valid
#define PVCPU_FLAGS_BP_ImmDispAbs 0b0010
#define PVCPU_FLAGS_BP_ExtSize 0b0100
#define PVCPU_FLAGS_BP_ExtFlags 0b1000

typedef struct {
    uint16_t opcode:12; // Actually 12bits, use lower
    uint8_t mode:4; // Actually 4bits, use lower
    uint8_t src:6; // Actually 6bits, use lower
    uint8_t dest:6; // Actually 6bits, use lower
    uint8_t flags:4; // Actually 4bits, use lower

    uint64_t imm;
    uint64_t ext_flags;
    uint8_t ext_size;
} __attribute__((packed)) PVCpu_Inst;

typedef enum {
    // ALU
    OP_ADD = 0,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_AND,
    OP_OR,
    OP_NOR,
    OP_XOR,
    OP_XNOR,
    OP_NOT,
    OP_NAND,
    OP_CMP,
    OP_UCMP,
    OP_TEST,
    OP_RSHIFT,
    OP_LSHIFT,
    OP_ARSHIFT,
    OP_ARLSHIFT,
    OP_ROTR,
    OP_ROTL,
    // Memory
    OP_LOAD = 0x100,
    OP_STORE,
    // Registers
    OP_MOV = 0x150
} Opcodes;

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

typedef struct {
    uint64_t regs[40]; // NULL, G0-G30, LR, SF, SP, PC (Internal), I0-I3 (Internal), TR (Internal)
    uint8_t* memory;
    size_t memsize;
} PVCpu_State;

typedef struct {
    uint8_t* program;
    size_t program_size;

    void* exec_mem;
    size_t exec_size;

    void* data_mem;
    size_t data_size;

    size_t memlimit;
} RuntimeContext;

typedef struct {
    uint16_t opcode; // Actually 12bits, use lower
    uint8_t extender; // Actually 4bits, use lower
} PVCpuC_Inst; // PVCpu-Compressed

inline static uint32_t pvpcu_pack_inst(PVCpu_Inst inst) {
    uint32_t packed_inst = 0;
    packed_inst |= (inst.opcode & 0xFFF) << 20;
    packed_inst |= (inst.mode & 0xF) << 16;
    packed_inst |= (inst.src & 0x3F) << 10;
    packed_inst |= (inst.dest & 0x3F) << 4;
    packed_inst |= (inst.flags & 0xF);
    return packed_inst;
}

inline static size_t pvcpu_unpack_inst(const void* buf, size_t len, PVCpu_Inst* out) {
    if (len < 4) return 0;

    uint32_t w;
    memcpy(&w, buf, 4);
    out->opcode = (w >> 20) & 0xFFF;
    out->mode = (w >> 16) & 0xF;
    out->src = (w >> 10) & 0x3F;
    out->dest = (w >> 4) & 0x3F;
    out->flags = w & 0xF;

    size_t off = 4;

    if (!(out->flags & PVCPU_FLAGS_BP_Valid)) return 4;
    if (out->flags & PVCPU_FLAGS_BP_ImmDispAbs) {
        uint8_t size = 4;
        if (out->flags & PVCPU_FLAGS_BP_ExtSize) size = 8;
        if (off + size > len) return 0;
        out->ext_size = size * 8;
        memcpy(&out->imm, buf + off, size);
        off += size;
    } else if (out->flags & PVCPU_FLAGS_BP_ExtFlags) {
        uint8_t size = 4;
        if (out->flags & PVCPU_FLAGS_BP_ExtSize) size = 8;
        if (off + size > len) return 0;
        out->ext_size = size * 8;
        
        memcpy(&out->ext_flags, buf + off, size);

        if (!(out->ext_flags & PVCPU_FLAGS_BP_Valid)) return 0;
    }

    return off;
}

inline static uint16_t pvpcu_c_pack_inst(PVCpuC_Inst inst) {
    uint16_t packed_inst = 0;
    packed_inst |= (inst.opcode & 0xFFF) << 4;
    packed_inst |= (inst.extender & 0xF);
    return packed_inst;
}

int pvcpu_run(uint8_t* run_code, size_t run_code_size, size_t memlimit);
