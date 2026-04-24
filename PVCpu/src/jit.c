#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h> // For realloc, since pheonix stdlib links fragmented memory in ralloc
#include <stdio.h>

#include <pvcpu-isa.h>
#include <pvcpu-jit.h>

#include <pvcpu-x64-jit.h>

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

int jit_convert_program(RuntimeContext *ctx) {
    if (ctx->memlimit > 0 && ctx->memlimit < JIT_PAGE) return -1;

    jit->capacity = JIT_PAGE;
    jit->data = exalloc(jit->capacity, __MEMTYPE_EXEC__ | __MEMTYPE_READ__ | __MEMTYPE_WRITE__, NULL);
    jit->size = 0;
    if (!jit->data) return -1;

    size_t offset = 0;

    jit_setup_x64(state, jit);
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
        printf("Opcode: 0x%X RDST: %u RSRC: %u Mode: %u\n", inst.opcode, inst.dest, inst.src, inst.mode);
        int out = jit_translate_x64(state, jit, &inst);
        if (out != 0) {
            CLEANUP();
            return out;
        }
    }

    ctx->exec_mem = jit->data;
    ctx->exec_size = jit->size;

    return 0;
}