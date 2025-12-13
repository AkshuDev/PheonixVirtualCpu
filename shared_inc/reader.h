// Author Pheonix Studios/AkshuDev

#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    Arch_Unknown,
    Arch_x86,
    Arch_x64,
    Arch_PVCpu,
    Arch_PVCpuC
} Architecture;

bool r_info_all(char* src, size_t size);
bool r_info_symbols(char* src, size_t size);
bool r_info_sections(char* src, size_t size);
bool r_info_basic(char* src, size_t size);
bool r_info_program_headers(char* src, size_t size);
bool r_info_relocs(char* src, size_t size);
Architecture r_get_arch(char* src, size_t size);
size_t r_get_codeoff(char* src, size_t size, size_t* code_size, size_t* svaddr);
