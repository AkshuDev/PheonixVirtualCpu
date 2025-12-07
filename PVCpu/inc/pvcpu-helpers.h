// Author: Pheonix Studios/AkshuDev
#pragma once

#include <stdint.h>

#include <pvcpu-jit.h>

void emit_u32(Jit_Buf* buf, uint32_t v);
void emit_u64(Jit_Buf* buf, uint64_t v);
