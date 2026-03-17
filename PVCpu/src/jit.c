#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h> // For realloc, since pheonix stdlib links fragmented memory in ralloc

#include <pvcpu-isa.h>
#include <pvcpu-jit.h>

#include <pstdlib.h> // For exalloc, since stdlib doesn't support it

#define JIT_PAGE 0x400

static Jit_Buf _jit = {0};
static Jit_Buf* jit = &_jit;

static PVCpu_State _state = {0};
static PVCpu_State* state = &_state;

#define CLEANUP() \
    dealloc(jit->data); \
    jit->capacity = 0; \
    jit->size = 0;

static inline int emit_u8(uint8_t v) {
    if (jit->size >= jit->capacity) return -1;
    jit->data[jit->size++] = v;
    return 0;
}

static inline int emit_u32(uint32_t v) {
    if (jit->size + 4 > jit->capacity) return -1;
    *(uint32_t*)(jit->data + jit->size) = v;
    jit->size += 4;
    return 0;
}

static int load_pvcpu_reg(uint8_t host_reg, uint8_t pvcpu_reg) {
#ifdef __x86_64__
    if (pvcpu_reg < 8) {
        // mov host_reg, [rdi + pvcpu_reg*8] ; rdi points to PVCpu_State*
        
        emit_u8(0x48); // REX.W
        emit_u8(0x8B); // mov r64, r/m64
        emit_u8(0x87 + (host_reg << 3)); // modrm
        emit_u32(pvcpu_reg * 8); 
    } else {
        // spill to stack / memory
    }
    return 0;
#endif
}

static int store_pvcpu_reg(uint8_t pvcpu_reg, uint8_t host_reg) {
#ifdef __x86_64__
    if (pvcpu_reg < 8) {
        emit_u8(0x48); // REX.W
        emit_u8(0x89); // mov r/m64, r64
        emit_u8(0x87 + (host_reg << 3));
        emit_u32(pvcpu_reg * 8);
    } else {
        // spill to stack / memory
    }
    return 0;
#endif
}

static int jit_emit_add(PVCpu_Inst* inst) {
#ifdef __x86_64
    switch (inst->mode) {
        case REG_REG: {
            // Find register dest
            if (inst->dest > 40) {
                // Stack
            } else {
                // Use corrosponding host register
            }
        }
    }
#endif
    return 0;
}

static int jit_translate(PVCpu_Inst* inst) {
    switch (inst->opcode) {
        case OP_ADD: return jit_emit_add(inst);
        default: return -1;
    }
}

int jit_convert_program(RuntimeContext *ctx) {
    if (ctx->memlimit > 0 && ctx->memlimit < JIT_PAGE) return -1;

    jit->capacity = JIT_PAGE;
    jit->data = exalloc(jit->capacity, __MEMTYPE_EXEC__ | __MEMTYPE_READ__ | __MEMTYPE_WRITE__, NULL);
    jit->size = 0;
    if (!jit->data) return -1;

    size_t offset = 0;

    while (offset < ctx->program_size) {
        // Unpack instruction (validates the instruction via its flags)
        PVCpu_Inst inst = {0};
        size_t off = pvcpu_unpack_inst(ctx->program + offset, ctx->program_size - offset, &inst);
        if (off < 4) {
            CLEANUP();
            return -1;
        }
        offset += off;

        // Run jit of the inst
        int out = jit_translate(&inst);
        if (out != 0) {
            CLEANUP();
            return out;
        }
    }

    ctx->exec_mem = jit->data;
    ctx->exec_size = jit->size;

    return 0;
}