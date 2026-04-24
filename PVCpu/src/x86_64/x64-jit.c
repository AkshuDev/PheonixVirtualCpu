#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h> // For realloc, since pheonix stdlib links fragmented memory in ralloc
#include <stdio.h>

#include <pvcpu-isa.h>
#include <pvcpu-jit.h>
#include <pvcpu-x64-jit.h>

#include <pstdlib.h> // For exalloc, since stdlib doesn't support it

typedef struct {
    PVCpu_Inst* cur_inst;
    PVCpu_Inst last_inst;

    uint8_t host_reg_connections[8];
} optimiser;

optimiser opt = {0};
bool reload_rdi = true;

static int emit_u8(Jit_Buf* jit, uint8_t v) {
    if (jit->size >= jit->capacity) return -1;
    jit->data[jit->size++] = v;
    return 0;
}

static int emit_u32(Jit_Buf* jit, uint32_t v) {
    if (jit->size + 4 > jit->capacity) return -1;
    memcpy(jit->data + jit->size, &v, 4);
    jit->size += 4;
    return 0;
}

static int emit_u64(Jit_Buf* jit, uint64_t v) {
    if (jit->size + 8 > jit->capacity) return -1;
    memcpy(jit->data + jit->size, &v, 8);
    jit->size += 8;
    return 0;
}

static uint8_t pvcpu_to_host_reg(uint16_t pvcpu_reg, bool* valid, bool* _64) {
    *valid = true;
    *_64 = false;
    switch (pvcpu_reg) {
        case 0: return 0;
        case 1: return 1;
        case 2: return 2;
        case 3: return 3;
        case 4: *_64 = true; return 0;
        case 5: *_64 = true; return 1;
        case 6: *_64 = true; return 2;
        case 7: *_64 = true; return 3;
        default: *valid = false; return 0;
    }
}

static uint8_t make_mod(uint8_t mod, uint8_t reg, uint8_t rm) {
    uint8_t modrm = rm & 0b00000111;
    modrm |= (reg << 3) & 0b00111000;
    modrm |= (mod << 6) & 0b11000000;
    return modrm;
}

static bool load_pvcpu_reg(Jit_Buf* jit, uint8_t* host_reg, bool* _64, uint16_t pvcpu_reg) {
    // rax, rcx, rdx, rbx, r8, r9, r10, r11
    if (pvcpu_reg < 8) { // rdi always points to 
        // mov host_reg, [rdi + pvcpu_reg*8] ; rdi points to PVCpu_State*
        
        bool valid = false;

        uint8_t reg = pvcpu_to_host_reg(pvcpu_reg, &valid, _64);
        if (!valid) return false;

        *host_reg = reg;
        if (opt.host_reg_connections[*_64 ? reg + 4 : reg] == pvcpu_reg) return true; // skip entirely

        if (*_64) emit_u8(jit, 0x4C); // REX.W+R
        else emit_u8(jit, 0x48); // REX.W
        emit_u8(jit, 0x8B); // mov r64, r/m64

        emit_u8(jit, make_mod(1, reg, 0b111)); // modrm (use 1 byte disp, and base reg rdi)
        emit_u8(jit, pvcpu_reg * 8);
        opt.host_reg_connections[*_64 ? reg + 4 : reg] = pvcpu_reg;
    } else {
        // spill to stack / memory
    }
    return true;
}

static int store_pvcpu_reg(Jit_Buf* jit, uint8_t pvcpu_reg, uint8_t host_reg, bool _64) {
    if (pvcpu_reg < 8) { // rdi always points to 
        // mov [rdi + pvcpu_reg*8], host_reg ; rdi points to PVCpu_State*

        if (_64) emit_u8(jit, 0x4C); // REX.W+R
        else emit_u8(jit, 0x48); // REX.W
        emit_u8(jit, 0x89); // mov r/m64, r64

        emit_u8(jit, make_mod(1, host_reg, 0b111)); // modrm (use 1 byte disp, and base reg rdi)
        emit_u8(jit, pvcpu_reg * 8);
    } else {
        // spill to stack / memory
    }
    return 0;
}

static int jit_emit_add(Jit_Buf* jit, PVCpu_Inst* inst) {
    switch (inst->mode) {
        case REG_REG: {
            uint8_t sreg = 0;
            uint8_t dreg = 0;
            bool s_64 = false;
            bool d_64 = false;
            load_pvcpu_reg(jit, &sreg, &s_64, inst->src);
            load_pvcpu_reg(jit, &dreg, &d_64, inst->dest);
            uint8_t rex = 0b01001000; // REX.W
            if (s_64) rex |= 0b00000001;
            if (d_64) rex |= 0b00000100;
            emit_u8(jit, rex); // REX.W?R?B
            emit_u8(jit, 0x1); // add r64, r64
            emit_u8(jit, make_mod(0b11, sreg, dreg));
            store_pvcpu_reg(jit, inst->dest, dreg, d_64);
        }
        case REG_IMM: {
            uint8_t dreg = 0;
            bool d_64 = false;
            load_pvcpu_reg(jit, &dreg, &d_64, inst->dest);
            emit_u8(jit, d_64 ? 0b01001001 : 0b01001000); // REX.W?B
            emit_u8(jit, 0x81); // add r64, imm32
            emit_u8(jit, make_mod(0b11, 0, dreg));
            emit_u32(jit, inst->src);
            store_pvcpu_reg(jit, inst->dest, dreg, d_64);
        }
        case REG_EXTIMM: {
            uint8_t dreg = 0;
            bool d_64 = false;
            load_pvcpu_reg(jit, &dreg, &d_64, inst->dest);
            if (inst->ext_size == 1) {
                emit_u8(jit, d_64 ? 0b01001001 : 0b01001000); // REX.W?B
                emit_u8(jit, 0x81); // add r64, imm32
                emit_u8(jit, make_mod(0b11, 0, dreg));
                emit_u32(jit, (uint32_t)inst->imm);
            } else {
                emit_u8(jit, 0b01001100); // REX.WR
                emit_u8(jit, 0xB8 + 4); // mov r12, imm64
                emit_u64(jit, inst->imm);
                emit_u8(jit, d_64 ? 0b01001101 : 0b01001100); // REX.WR?B
                emit_u8(jit, 0x1); // add r/m64, r12
                emit_u8(jit, make_mod(0b11, 4, dreg));
            }
            store_pvcpu_reg(jit, inst->dest, dreg, d_64);
        }
    }
    return 0;
}

void jit_setup_x64(PVCpu_State* state, Jit_Buf* jit) {
    for (size_t i = 0; i < sizeof(opt.host_reg_connections); i++) {
        opt.host_reg_connections[i] = 0xFF; // Reset
    }
}

int jit_translate_x64(PVCpu_State* state, Jit_Buf* jit, PVCpu_Inst* inst) {
    if (reload_rdi) {
        emit_u8(jit, 0x48); // REX.W
        emit_u8(jit, 0xBF); // mov rdi, state
        emit_u64(jit, (uint64_t)state);
        reload_rdi = false;
    }

    opt.cur_inst = inst;
    int ret = 0;
    switch (inst->opcode) {
        case OP_ADD: ret = jit_emit_add(jit, inst); break;
        default: ret = -1;
    }
    memcpy(&opt.last_inst, inst, sizeof(PVCpu_Inst));
    return ret;
}