// Author: Pheonix Studios/AkshuDev
#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t* data;
    size_t size;
    size_t capacity;
} Jit_Buf;

static inline void jit_init(Jit_Buf* buf, size_t cap) {
    buf->data = (uint8_t*)malloc(cap);
    buf->size = 0;
    buf->capacity = cap;
}
