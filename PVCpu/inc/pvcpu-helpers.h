// Author: Pheonix Studios/AkshuDev
#pragma once

#include <stdint.h>

#include <pvcpu-jit.h>

void emit_u8(Jit_Buf* buf, uint8_t v);
void emit_u16(Jit_Buf* buf, uint16_t v);
void emit_u32(Jit_Buf* buf, uint32_t v);
void emit_u64(Jit_Buf* buf, uint64_t v);
