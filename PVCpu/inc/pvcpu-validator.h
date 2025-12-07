// Author: Pheonix Studios/AkshuDev
#pragma once

#include <stdbool.h>

#include <pvcpu-isa.h>

bool pvcpu_validate_inst(const PVCpu_Inst* inst, uint64_t value, size_t allocated_size, size_t vaddr);
