// Author: Pheonix Studios/AkshuDev

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pvcpu-jit.h>
#include <pvcpu-helpers.h>
#include <pvcpu-isa.h>

typedef void (*PVCpu_Handler)(Jit_Buf* buf, PVCpu_Inst* inst, uint64_t value, uint64_t* extflags, int extflag_count);
static PVCpu_Handler handlers[4096]; // 12-bits

typedef void (*JitFn)(PVCpu_State*);

static void load_pvcpu_reg(Jit_Buf* buf, int host_reg, int pvcpu_reg) {
    #ifdef __x86_64__
        uint32_t offset = offsetof(PVCpu_State, regs) + pvcpu_reg * 8;

        emit_u8(buf, 0b01001000); // REX.W = 1
        emit_u8(buf, 0x8B); // mov r64, r/m64
        #ifdef _WIN32
        uint8_t modrm = 0b10000001 | ((host_reg & 7) << 3); // RCX
        #else
        uint8_t modrm = 0b10000111 | ((host_reg & 7) << 3); // RDI
        #endif
        emit_u8(buf, modrm); // modrm [rdi + disp32]
        emit_u32(buf, offset);
    #endif
}

static void store_pvcpu_reg(Jit_Buf* buf, int pvcpu_reg, int host_reg) {
    #ifdef __x86_64__
        uint32_t offset = offsetof(PVCpu_State, regs) + pvcpu_reg * 8;

        emit_u8(buf, 0b01001000); // REX.W = 1
        emit_u8(buf, 0x89); // mov r/m64, r64
        #ifdef _WIN32
        uint8_t modrm = 0b10000001 | ((host_reg & 7) << 3); // RCX
        #else
        uint8_t modrm = 0b10000111 | ((host_reg & 7) << 3); // RDI
        #endif
        emit_u8(buf, modrm);
        emit_u32(buf, offset);
    #endif
}

static void op_add(Jit_Buf* buf, PVCpu_Inst* inst, uint64_t value, uint64_t* extflags, int extflag_count) {
    // add [dest], [src] & add [mem], [src] & add [dest], [mem] & add [dest], imm
    if (inst->mode == REG_REG) {
        #ifdef __x86_64__
            load_pvcpu_reg(buf, 0, inst->src);
            load_pvcpu_reg(buf, 3, inst->dest);
            emit_u8(buf, 0b01001000); // REX.W = 1
            emit_u8(buf, 0x01); // add r/m64, r64
            emit_u8(buf, 0b11011000); // modrm:rbx, rax

            store_pvcpu_reg(buf, inst->dest, 3);
        #endif
    }
}

static void init_handlers() {
    for (int i = 0; i < 4096; i++) {
        handlers[i] = NULL;
    }

    handlers[OP_ADD] = op_add;
}

void pvcpu_run(PVCpu_Inst* insts, uint64_t* values, uint64_t** extflags, int* extflag_count, size_t inst_num, size_t inst_cap, size_t memsize, size_t instsize, uint8_t run_code) {
    init_handlers();
    
    Jit_Buf buf = {0};
    PVCpu_State cpu_state = {0};
    jit_init(&buf, instsize);
    if (buf.data == NULL) {
        perror("Error: Instruction memory allocation failed!");
        return;
    }
    cpu_state.memory = (uint8_t*)malloc(memsize);
    if (cpu_state.memory == NULL) {
        perror("Error: Memory allocation failed!");
        free(buf.data);
        return;
    }
    cpu_state.memsize = memsize;

    for (size_t i = 0; i < inst_num; i++) {
        PVCpu_Inst* inst = &insts[i];
        if (inst->opcode > 4096) continue; // Just continue so the code doesn't break incase they run it! (Though validator will catch it)
        handlers[inst->opcode](&buf, inst, values[i], extflags[i], extflag_count[i]);

        if (buf.size > buf.capacity) {
            fprintf(stderr, "Error: Not enough memory to store instructions, maybe try allocating a bit more?\n");
            free(buf.data);
            free(cpu_state.memory);
            return;
        }
    }

    JitFn entry = (JitFn)buf.data;
    
    #ifdef __x86_64__
        emit_u8(&buf, 0xC3); // ret just incase
    #endif

    if (run_code == 1) entry(&cpu_state);
    else {
        printf("Host Code generation completed!\n");
        printf("Dumping Code : \n");
        for (size_t i = 0; i < buf.size; i++) {
            printf("%02x ", buf.data[i]);
        }
        printf("\n");
    }

    free(cpu_state.memory);
    free(buf.data);
}
