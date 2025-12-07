// Author: Pheonix Studios/AkshuDev

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <pvcpu-jit.h>
#include <pvcpu-helpers.h>

void emit_u32(Jit_Buf* buf, uint32_t v) {
    if (buf->size + 4 > buf->capacity) return;
    memcpy(buf->data + buf->size, &v, 4);
    buf->size += 4;
}

void emit_u64(Jit_Buf* buf, uint64_t v) {
    if (buf->size + 8 > buf->capacity) return;
    memcpy(buf->data + buf->size, &v, 8);
    buf->size += 8;
}
