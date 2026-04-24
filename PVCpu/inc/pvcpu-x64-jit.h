#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <pvcpu-isa.h>
#include <pvcpu-jit.h>

void jit_setup_x64(PVCpu_State* state, Jit_Buf* jit);
int jit_translate_x64(PVCpu_State* state, Jit_Buf* buf, PVCpu_Inst* inst);