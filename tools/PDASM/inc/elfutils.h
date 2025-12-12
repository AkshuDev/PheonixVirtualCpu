// Author: Pheonix Studios/AkshuDev

#pragma once

#include <elf.h>
#include <stddef.h>

#define EM_PVCPU 0x5650
#define EM_PVCPUC 0x5651 // Compressed version
#define R_PVCPU_64 0x80001000
#define R_PVCPU_32 0x80001001
#define R_PVCPU_16 0x80001002
#define R_PVCPU_8 0x80001003

#define EM_INTEL205 205
#define EM_INTEL206 206
#define EM_INTEL207 207
#define EM_INTEL208 208
#define EM_INTEL209 209
#define EM_ARC_COMPACT2 195
#define EM_ECOG1 96
#define EM_HEXAGON 164
#define EM_KMX16 210
#define EM_KMX8 211
#define EM_LANAI 212
#define EM_CEVA 213
#define EM_CEVA_X2 214
#define EM_GRAPHCORE_IPU 215
#define EM_MCST_ELBRUS_E2K 216
#define EM_INTEL_GT 217
#define EM_INTEL_GTP 218
#define EM_ARC_COMPACT3 219
#define EM_MIPS_LEGACY 220
#define EM_MIPS_RS4_BE 221
#define EM_WDC65816 222

typedef struct {
    int value;
    const char* name;
} EMEntry;

static const EMEntry em_table[] = {
    {EM_NONE, "None (Unknown)"},
    {EM_M32, "AT&T WE 32100 (32-Bit)"},
    {EM_SPARC, "SPARC (32-Bit)"},
    {EM_386, "Intel 80386 (32-Bit)"},
    {EM_68K, "Motorola 68000 (32-Bit)"},
    {EM_88K, "Motorola 88000 (32-Bit)"},
    {EM_IAMCU, "Intel MCU (8-Bit)"},
    {EM_860, "Intel 80860 (32-Bit)"},
    {EM_MIPS, "MIPS I/II (32-Bit)"},
    {EM_S370, "IBM System/370 (32-Bit)"},
    {EM_MIPS_RS3_LE, "MIPS RS3 Little-Endian (32-Bit)"},
    {EM_PARISC, "HP PA-RISC (32-Bit)"},
    {EM_VPP500, "Fujitsu VPP500 (32-Bit)"},
    {EM_SPARC32PLUS, "SPARC V8+ (32-Bit)"},
    {EM_960, "Intel 960 (32-Bit)"},
    {EM_PPC, "PowerPC (32-Bit)"},
    {EM_PPC64, "PowerPC 64-Bit"},
    {EM_S390, "IBM S390 (32-Bit)"},
    {EM_SPU, "Cell SPU (32-Bit)"},
    {EM_V800, "NEC V800 (16-Bit)"},
    {EM_FR20, "Fujitsu FR20 (32-Bit)"},
    {EM_RH32, "TRW RH-32 (32-Bit)"},
    {EM_RCE, "Motorola RCE (32-Bit)"},
    {EM_ARM, "ARM (32-Bit)"},
    {EM_ALPHA, "DEC Alpha (64-Bit)"},
    {EM_SH, "SuperH (32-Bit)"},
    {EM_SPARCV9, "SPARC V9 (64-Bit)"},
    {EM_TRICORE, "Infineon TriCore (32-Bit)"},
    {EM_ARC, "ARC (32-Bit)"},
    {EM_H8_300, "Hitachi H8/300 (16-Bit)"},
    {EM_H8_300H, "Hitachi H8/300H (16-Bit)"},
    {EM_H8S, "Hitachi H8S (16-Bit)"},
    {EM_H8_500, "Hitachi H8/500 (16-Bit)"},
    {EM_IA_64, "Intel Itanium (64-Bit)"},
    {EM_MIPS_X, "MIPS-X (32-Bit)"},
    {EM_COLDFIRE, "Motorola ColdFire (32-Bit)"},
    {EM_68HC12, "Motorola 68HC12 (16-Bit)"},
    {EM_MMA, "Fujitsu Multimedia Accelerator (32-Bit)"},
    {EM_PCP, "Siemens PCP (32-Bit)"},
    {EM_NCPU, "Sony nCPU (32-Bit)"},
    {EM_NDR1, "Sony NDR1 (32-Bit)"},
    {EM_STARCORE, "Motorola StarCore DSP (32-Bit)"},
    {EM_ME16, "Trebia ME16 (16-Bit)"},
    {EM_ST100, "ST100 (16-Bit)"},
    {EM_TINYJ, "Advanced TinyJ (8-Bit)"},
    {EM_X86_64, "x64 (AMD64, 64-Bit)"},
    {EM_PDSP, "Polar Semiconductor DSP (32-Bit)"},
    {EM_PDP10, "DEC PDP-10 (36-Bit)"},
    {EM_PDP11, "DEC PDP-11 (16-Bit)"},
    {EM_FX66, "Siemens FX66 (16-Bit)"},
    {EM_ST9PLUS, "ST9+ (8-Bit)"},
    {EM_ST7, "ST7 (8-Bit)"},
    {EM_68HC16, "Motorola 68HC16 (16-Bit)"},
    {EM_68HC11, "Motorola 68HC11 (8-Bit)"},
    {EM_68HC08, "Motorola 68HC08 (8-Bit)"},
    {EM_68HC05, "Motorola 68HC05 (8-Bit)"},
    {EM_SVX, "Seiko SVX (16-Bit)"},
    {EM_ST19, "ST19 (16-Bit)"},
    {EM_VAX, "DEC VAX (32-Bit)"},
    {EM_CRIS, "Axis Communications CRIS (32-Bit)"},
    {EM_JAVELIN, "Javelin (32-Bit)"},
    {EM_FIREPATH, "Firepath (32-Bit)"},
    {EM_ZSP, "ZSP DSP (32-Bit)"},
    {EM_MMIX, "Donald Knuth MMIX (64-Bit)"},
    {EM_HUANY, "Huany Processor (32-Bit)"},
    {EM_PRISM, "Prism (32-Bit)"},
    {EM_AVR, "Atmel AVR 8-Bit MCU"},
    {EM_FR30, "Fujitsu FR30 (32-Bit)"},
    {EM_D10V, "NEC D10V (32-Bit)"},
    {EM_D30V, "NEC D30V (32-Bit)"},
    {EM_V850, "NEC V850 (32-Bit)"},
    {EM_M32R, "Renesas M32R (32-Bit)"},
    {EM_MN10300, "Panasonic MN10300 (32-Bit)"},
    {EM_MN10200, "Panasonic MN10200 (32-Bit)"},
    {EM_PJ, "PicoJava (32-Bit)"},
    {EM_OPENRISC, "OpenRISC 32-Bit"},
    {EM_ARC_COMPACT, "ARC Compact (32-Bit)"},
    {EM_XTENSA, "Xtensa (32-Bit)"},
    {EM_VIDEOCORE, "VideoCore (32-Bit)"},
    {EM_TMM_GPP, "TMM General-Purpose Processor (32-Bit)"},
    {EM_NS32K, "NS32K (32-Bit)"},
    {EM_TPC, "TPC (32-Bit)"},
    {EM_SNP1K, "S1NP1K (32-Bit)"},
    {EM_ST200, "ST200 (32-Bit)"},
    {EM_IP2K, "IP2K (32-Bit)"},
    {EM_MAX, "MAX (Unknown)"},
    {EM_CR, "CR (Unknown)"},
    {EM_F2MC16, "F2MC-16 (16-Bit)"},
    {EM_MSP430, "TI MSP430 (16-Bit)"},
    {EM_BLACKFIN, "Analog Devices Blackfin (32-Bit)"},
    {EM_SE_C33, "SE-C33 (32-Bit)"},
    {EM_SEP, "SEP (32-Bit)"},
    {EM_ARCA, "ARCA (32-Bit)"},
    {EM_UNICORE, "Unicore (32-Bit)"},
    {EM_EXCESS, "Excess (32-Bit)"},
    {EM_DXP, "DXP DSP (32-Bit)"},
    {EM_ALTERA_NIOS2, "Altera Nios II (32-Bit)"},
    {EM_CRX, "CRX (32-Bit)"},
    {EM_XGATE, "XGATE (16-Bit)"},
    {EM_C166, "Infineon C166 (16-Bit)"},
    {EM_M16C, "Renesas M16C (16-Bit)"},
    {EM_DSPIC30F, "Microchip dsPIC30F (16-Bit)"},
    {EM_CE, "CE (32-Bit)"},
    {EM_M32C, "Renesas M32C (16-Bit)"},
    {EM_TSK3000, "TSK3000 (32-Bit)"},
    {EM_RS08, "RS08 (8-Bit)"},
    {EM_SHARC, "Analog Devices SHARC DSP (32-Bit)"},
    {EM_ECOG2, "eCOG2 (8-Bit)"},
    {EM_SCORE7, "Score7 DSP (32-Bit)"},
    {EM_DSP24, "DSP24 (24-Bit)"},
    {EM_VIDEOCORE3, "VideoCore III (32-Bit)"},
    {EM_LATTICEMICO32, "LatticeMico32 (32-Bit)"},
    {EM_SE_C17, "SE-C17 (32-Bit)"},
    {EM_TI_C6000, "TI C6000 DSP (32-Bit)"},
    {EM_TI_C2000, "TI C2000 MCU (32-Bit)"},
    {EM_TI_C5500, "TI C5500 DSP (32-Bit)"},
    {EM_MMDSP_PLUS, "MMDSP+ DSP (32-Bit)"},
    {EM_CYPRESS_M8C, "Cypress M8C (8-Bit)"},
    {EM_R32C, "R32C (32-Bit)"},
    {EM_TRIMEDIA, "Trimedia (32-Bit)"},
    {EM_HEXAGON, "Qualcomm Hexagon DSP (32-Bit)"},
    {EM_8051, "Intel 8051 (8-Bit)"},
    {EM_STXP7X, "STxP7x (32-Bit)"},
    {EM_NDS32, "Andes NDS32 (32-Bit)"},
    {EM_ECOG1, "eCOG1 (8-Bit)"},
    {EM_ECOG1X, "eCOG1X (8-Bit)"},
    {EM_MAXQ30, "MaxQ30 (16-Bit)"},
    {EM_XIMO16, "XIMO16 (16-Bit)"},
    {EM_MANIK, "Manik (32-Bit)"},
    {EM_CRAYNV2, "Cray NV2 (64-Bit)"},
    {EM_RX, "Renesas RX (32-Bit)"},
    {EM_METAG, "Imagination MetaG (32-Bit)"},
    {EM_MCST_ELBRUS, "MCST Elbrus (64-Bit)"},
    {EM_ECOG16, "eCOG16 (16-Bit)"},
    {EM_CR16, "CR16 (16-Bit)"},
    {EM_ETPU, "Enhanced Time Processor Unit (32-Bit)"},
    {EM_SLE9X, "SLE9X (32-Bit)"},
    {EM_L10M, "L10M (32-Bit)"},
    {EM_K10M, "K10M (32-Bit)"},
    {EM_AARCH64, "ARM64 (AArch64, 64-Bit)"},
    {EM_AVR32, "Atmel AVR32 (32-Bit)"},
    {EM_STM8, "STMicro STM8 (8-Bit)"},
    {EM_TILE64, "Tilera TILE64 (32-Bit)"},
    {EM_TILEPRO, "Tilera TILEPro (32-Bit)"},
    {EM_MICROBLAZE, "Xilinx MicroBlaze (32-Bit)"},
    {EM_CUDA, "NVIDIA CUDA (32-Bit)"},
    {EM_TILEGX, "Tilera TILE-Gx (32-Bit)"},
    {EM_CLOUDSHIELD, "CloudShield (32-Bit)"},
    {EM_COREA_1ST, "CoreA 1st Gen (32-Bit)"},
    {EM_COREA_2ND, "CoreA 2nd Gen (32-Bit)"},
    {EM_ARC_COMPACT2, "ARC Compact2 (32-Bit)"},
    {EM_OPEN8, "Open8 (8-Bit)"},
    {EM_RL78, "Renesas RL78 (16-Bit)"},
    {EM_VIDEOCORE5, "VideoCore V (32-Bit)"},
    {EM_78KOR, "Renesas 78KOR (8-Bit)"},
    {EM_56800EX, "Freescale 56800EX (16-Bit)"},
    {EM_BA1, "Beyond BA1 (32-Bit)"},
    {EM_BA2, "Beyond BA2 (32-Bit)"},
    {EM_XCORE, "XMOS XCore (32-Bit)"},
    {EM_MCHP_PIC, "Microchip PIC (8-Bit)"},
    {EM_INTEL205, "Intel 205 (8-Bit)"},
    {EM_INTEL206, "Intel 206 (8-Bit)"},
    {EM_INTEL207, "Intel 207 (8-Bit)"},
    {EM_INTEL208, "Intel 208 (8-Bit)"},
    {EM_INTEL209, "Intel 209 (8-Bit)"},
    {EM_KM32, "KM32 (32-Bit)"},
    {EM_KMX32, "KMX32 (32-Bit)"},
    {EM_KMX16, "KMX16 (16-Bit)"},
    {EM_KMX8, "KMX8 (8-Bit)"},
    {EM_KVARC, "KVARC (32-Bit)"},
    {EM_CDP, "CDP (32-Bit)"},
    {EM_COGE, "COGE (32-Bit)"},
    {EM_COOL, "COOL (32-Bit)"},
    {EM_NORC, "NORC (32-Bit)"},
    {EM_CSR_KALIMBA, "CSR Kalimba (32-Bit)"},
    {EM_Z80, "Zilog Z80 (8-Bit)"},
    {EM_VISIUM, "Visium (32-Bit)"},
    {EM_FT32, "FT32 (32-Bit)"},
    {EM_MOXIE, "Moxie (32-Bit)"},
    {EM_AMDGPU, "AMD GPU (64-Bit)"},
    {EM_RISCV, "RISC-V (32-Bit)"},
    {EM_LANAI, "Lanai (32-Bit)"},
    {EM_CEVA, "CEVA DSP (32-Bit)"},
    {EM_CEVA_X2, "CEVA-X2 DSP (32-Bit)"},
    {EM_BPF, "eBPF Virtual Machine (64-Bit)"},
    {EM_GRAPHCORE_IPU, "Graphcore IPU (32-Bit)"},
    {EM_MCST_ELBRUS_E2K, "MCST Elbrus E2K (64-Bit)"},
    {EM_INTEL_GT, "Intel GT Graphics (64-Bit)"},
    {EM_INTEL_GTP, "Intel GTP Graphics (64-Bit)"},
    {EM_ARC_COMPACT3, "ARC Compact3 (32-Bit)"},
    {EM_MIPS_LEGACY, "MIPS Legacy (32-Bit)"},
    {EM_MIPS_RS4_BE, "MIPS RS4 Big-Endian (32-Bit)"},
    {EM_WDC65816, "Western Design Center 65816 (16-Bit)"},
    {EM_PVCPU, "Pheonix Virtual CPU (64-Bit)"},
    {EM_PVCPUC, "Pheonix Virtual CPU - Compressed (64-Bit)"}
};

static const size_t em_table_count = sizeof(em_table) / sizeof(em_table[0]);

static inline const char *elf_machine_to_string(int mach) {
    for (size_t i = 0; i < em_table_count; i++) {
        if (em_table[i].value == mach)
            return em_table[i].name;
    }
    return "Unknown Machine";
}

static inline const char* elf_ph_type_to_string(uint32_t type) {
    switch(type) {
        case PT_NULL: return "NULL";
        case PT_LOAD: return "LOAD";
        case PT_DYNAMIC: return "DYNAMIC";
        case PT_INTERP: return "INTERP";
        case PT_NOTE: return "NOTE";
        case PT_SHLIB: return "SHLIB";
        case PT_PHDR: return "PHDR";
        case PT_TLS: return "TLS";
        default: return "UNKNOWN";
    }
}

static inline void elf_better_ph_flags(uint32_t flags, char* buf, size_t bufsize) {
    snprintf(
        buf,
        bufsize,
        "%c%c%c",
        (flags & PF_R) ? 'R' : '-',
        (flags & PF_W) ? 'W' : '-',
        (flags & PF_X) ? 'X' : '-'
    );
}

static inline const char* elf_sh_type_to_string(uint32_t type) {
    switch(type) {
        case SHT_NULL: return "NULL";
        case SHT_PROGBITS: return "PROGBITS";
        case SHT_SYMTAB: return "SYMTAB";
        case SHT_STRTAB: return "STRTAB";
        case SHT_RELA: return "RELA";
        case SHT_HASH: return "HASH";
        case SHT_DYNAMIC: return "DYNAMIC";
        case SHT_NOTE: return "NOTE";
        case SHT_NOBITS: return "NOBITS";
        case SHT_REL: return "REL";
        case SHT_SHLIB: return "SHLIB";
        case SHT_DYNSYM: return "DYNSYM";
        default: return "UNKNOWN";
    }
}

