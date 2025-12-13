// Author: Pheonix Studios/AkshuDev

#pragma once

#include <stdint.h>

#include <pecoff.h>

static inline const char* pecoff_machine_to_string(uint16_t machine) {
    switch(machine) {
        case 0x014c: return "Intel 386";
        case 0x8664: return "x64";
        case 0x01c0: return "ARM";
        case 0xaa64: return "ARM64";
        case 0x01c4: return "ARM Thumb-2";
        case 0x0200: return "Intel Itanium";
        default: return "UNKNOWN";
    }
}

static inline const char* pecoff_characteristics_to_string(uint16_t flags) {
    static char buf[64];
    buf[0] = 0;
    if(flags & 0x0002) strcat(buf, "EXECUTABLE ");
    if(flags & 0x2000) strcat(buf, "DLL ");
    if(flags & 0x0001) strcat(buf, "RELOCS_STRIPPED ");
    return buf;
}

static inline const char* pe_magic_to_string(uint16_t magic) {
    switch(magic) {
        case 0x10b: return "PE32";
        case 0x20b: return "PE32+";
        default: return "UNKNOWN";
    }
}

