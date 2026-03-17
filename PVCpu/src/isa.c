// Author: Pheonix Studios/AkshuDev

#include "pvcpu-jit.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <pstdlib.h>

#include <pvcpu-isa.h>

static RuntimeContext _ctx = {0};
static RuntimeContext* ctx = &_ctx;

int pvcpu_run(uint8_t* run_code, size_t run_code_size, size_t memlimit) {
    if (!run_code || run_code_size == 0) {
        fprintf(stderr, "Invalid program input\n");
        return -1;
    }

    ctx->program = run_code;
    ctx->program_size = run_code_size;
    ctx->memlimit = memlimit;

    int out = jit_convert_program(ctx);
    if (out != 0) return out;

    return 0;
}