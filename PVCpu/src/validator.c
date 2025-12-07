// Author: Pheonix Studios/AkshuDev

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <pvcpu-isa.h>

bool pvcpu_validate_inst(const PVCpu_Inst* inst, uint64_t value, size_t allocated_size, size_t vaddr) {
    if (!(inst->flags & PVCPU_FLAGS_BP_VALID)) return false; // Invalid Instruction
    if (inst->opcode > 0xFFF) return false; // Out of opcode range
    // Supported Category check
    if (inst->opcode > 0xFF) return false; 
    if (inst->src > 33 || inst->dest > 33) return false; // No Access to internal registers
    
    if (inst->flags & PVCPU_FLAGS_BP_EXT) return false; // No Extended flags support yet
    
    // Memory
    if (inst->opcode > 0xFF && inst->opcode < 0x1FF) {
        // Out of Memory operations
        if (inst->flags & PVCPU_FLAGS_BP_IMM) {
            if (value > allocated_size) return false;
        }
    }
    if (inst->flags & PVCPU_FLAGS_BP_DISP64) {
        if (value + vaddr > allocated_size) return false;
    }

    return true;
}
