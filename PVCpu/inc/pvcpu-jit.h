// Author: Pheonix Studios/AkshuDev

#pragma once

#include <stdint.h>
#include <stddef.h>

#include <pvcpu-isa.h>

typedef struct {
    uint8_t* data;
    size_t size;
    size_t capacity;
} Jit_Buf;

int jit_convert_program(RuntimeContext* ctx);

typedef void (*JitFn)(void* cpu_state);