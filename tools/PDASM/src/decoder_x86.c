// Author: Pheonix Studios/AkshuDev

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <decoder_x86.h>
#include <extra.h>

typedef struct {
    bool rex_present;
    bool b16_prefix;
    bool opcode16;
    uint8_t rex_w;
    uint8_t rex_r;
    uint8_t rex_x;
    uint8_t rex_b;
    uint16_t opcode;
    uint8_t modrm;
    uint8_t sib;
    uint64_t disp;
    uint64_t imm;
} x64_inst;

typedef enum {
    OP_RM_REG, // 01, 29, 89 etc (r/m <- reg)
    OP_REG_RM, // 03, 8B etc (reg <- r/m)
} op_form;

static HashMap8 opcodes8[] = {
    {0x00,"add"}, {0x01,"add"}, {0x02,"add"}, {0x03,"add"}, {0x04,"add"}, {0x05,"add"},
    {0x06,"push"}, {0x07,"pop"}, {0x08,"or"}, {0x09,"or"}, {0x0A,"or"}, {0x0B,"or"},
    {0x0C,"or"}, {0x0D,"or"}, {0x0E,"push"}, {0x10,"adc"}, {0x11,"adc"},
    {0x12,"adc"}, {0x13,"adc"}, {0x14,"adc"}, {0x15,"adc"}, {0x16,"push"}, {0x17,"pop"},
    {0x18,"sbb"}, {0x19,"sbb"}, {0x1A,"sbb"}, {0x1B,"sbb"}, {0x1C,"sbb"}, {0x1D,"sbb"},
    {0x1E,"push"}, {0x1F,"pop"}, {0x20,"and"}, {0x21,"and"}, {0x22,"and"}, {0x23,"and"},
    {0x24,"and"}, {0x25,"and"}, {0x26,"es"}, {0x27,"daa"}, {0x28,"sub"}, {0x29,"sub"},
    {0x2A,"sub"}, {0x2B,"sub"}, {0x2C,"sub"}, {0x2D,"sub"}, {0x2E,"cs"}, {0x2F,"das"},
    {0x30,"xor"}, {0x31,"xor"}, {0x32,"xor"}, {0x33,"xor"}, {0x34,"xor"}, {0x35,"xor"},
    {0x36,"ss"}, {0x37,"aaa"}, {0x38,"cmp"}, {0x39,"cmp"}, {0x3A,"cmp"}, {0x3B,"cmp"},
    {0x3C,"cmp"}, {0x3D,"cmp"}, {0x3E,"ds"}, {0x3F,"aas"}, {0x40,"inc"}, {0x41,"inc"},
    {0x42,"inc"}, {0x43,"inc"}, {0x44,"inc"}, {0x45,"inc"}, {0x46,"inc"}, {0x47,"inc"},
    {0x48,"dec"}, {0x49,"dec"}, {0x4A,"dec"}, {0x4B,"dec"}, {0x4C,"dec"}, {0x4D,"dec"},
    {0x4E,"dec"}, {0x4F,"dec"}, {0x50,"push"}, {0x51,"push"}, {0x52,"push"}, {0x53,"push"},
    {0x54,"push"}, {0x55,"push"}, {0x56,"push"}, {0x57,"push"}, {0x58,"pop"}, {0x59,"pop"},
    {0x5A,"pop"}, {0x5B,"pop"}, {0x5C,"pop"}, {0x5D,"pop"}, {0x5E,"pop"}, {0x5F,"pop"},
    {0x60,"pusha"}, {0x61,"popa"}, {0x62,"bound"}, {0x63,"arpl"}, {0x64,"fs"}, {0x65,"gs"},
    {0x66,"operand-size"}, {0x67,"address-size"}, {0x68,"push"}, {0x69,"imul"}, {0x6A,"push"},
    {0x6B,"imul"}, {0x6C,"insb"}, {0x6D,"insw"}, {0x6E,"outsb"}, {0x6F,"outsw"}, {0x70,"jo"},
    {0x71,"jno"}, {0x72,"jb"}, {0x73,"jnb"}, {0x74,"jz"}, {0x75,"jnz"}, {0x76,"jbe"}, {0x77,"jnbe"},
    {0x78,"js"}, {0x79,"jns"}, {0x7A,"jp"}, {0x7B,"jnp"}, {0x7C,"jl"}, {0x7D,"jge"}, {0x7E,"jle"}, {0x7F,"jg"},
    {0x80,"grp1"}, {0x81,"grp1"}, {0x82,"grp1"}, {0x83,"grp1"}, {0x84,"test"}, {0x85,"test"}, {0x86,"xchg"}, {0x87,"xchg"},
    {0x88,"mov"}, {0x89,"mov"}, {0x8A,"mov"}, {0x8B,"mov"}, {0x8C,"mov"}, {0x8D,"lea"}, {0x8E,"mov"}, {0x8F,"pop"},
    {0x90,"nop"}, {0x91,"xchg"}, {0x92,"xchg"}, {0x93,"xchg"}, {0x94,"xchg"}, {0x95,"xchg"}, {0x96,"xchg"}, {0x97,"xchg"},
    {0x98,"cbw"}, {0x99,"cwd"}, {0x9A,"callf"}, {0x9B,"fwait"}, {0x9C,"pushf"}, {0x9D,"popf"}, {0x9E,"sahf"}, {0x9F,"lahf"},
    {0xA0,"mov"}, {0xA1,"mov"}, {0xA2,"mov"}, {0xA3,"mov"}, {0xA4,"movs"}, {0xA5,"movs"}, {0xA6,"cmps"}, {0xA7,"cmps"},
    {0xA8,"test"}, {0xA9,"test"}, {0xAA,"stos"}, {0xAB,"stos"}, {0xAC,"lods"}, {0xAD,"lods"}, {0xAE,"scas"}, {0xAF,"scas"},
    {0xB0,"mov"}, {0xB1,"mov"}, {0xB2,"mov"}, {0xB3,"mov"}, {0xB4,"mov"}, {0xB5,"mov"}, {0xB6,"mov"}, {0xB7,"mov"},
    {0xB8,"mov"}, {0xB9,"mov"}, {0xBA,"mov"}, {0xBB,"mov"}, {0xBC,"mov"}, {0xBD,"mov"}, {0xBE,"mov"}, {0xBF,"mov"},
    {0xC0,"shift"}, {0xC1,"shift"}, {0xC2,"ret"}, {0xC3,"ret"}, {0xC4,"les"}, {0xC5,"lds"}, {0xC6,"mov"}, {0xC7,"mov"},
    {0xC8,"enter"}, {0xC9,"leave"}, {0xCA,"retf"}, {0xCB,"retf"}, {0xCC,"int3"}, {0xCD,"int"}, {0xCE,"into"}, {0xCF,"iret"},
    {0xD0,"shift"}, {0xD1,"shift"}, {0xD2,"shift"}, {0xD3,"shift"}, {0xD4,"aam"}, {0xD5,"aad"}, {0xD6,"salc"}, {0xD7,"xlat"},
    {0xD8,"fpu"}, {0xD9,"fpu"}, {0xDA,"fpu"}, {0xDB,"fpu"}, {0xDC,"fpu"}, {0xDD,"fpu"}, {0xDE,"fpu"}, {0xDF,"fpu"},
    {0xE0,"loopne"}, {0xE1,"loope"}, {0xE2,"loop"}, {0xE3,"jcxz"}, {0xE4,"in"}, {0xE5,"in"}, {0xE6,"out"}, {0xE7,"out"},
    {0xE8,"call"}, {0xE9,"jmp"}, {0xEA,"jmpf"}, {0xEB,"jmp"}, {0xEC,"in"}, {0xED,"in"}, {0xEE,"out"}, {0xEF,"out"},
    {0xF0,"lock"}, {0xF1,"int1"}, {0xF2,"repne"}, {0xF3,"repe"}, {0xF4,"hlt"}, {0xF5,"cmc"}, {0xF6,"grp3"}, {0xF7,"grp3"},
    {0xF8,"clc"}, {0xF9,"stc"}, {0xFA,"cli"}, {0xFB,"sti"}, {0xFC,"cld"}, {0xFD,"std"}, {0xFE,"grp4"}, {0xFF,"grp5"}
};

static size_t opcodes8_count = sizeof(opcodes8)/sizeof(opcodes8[0]);

static HashMap8 opcodes16[] = {
    {0x00,"sldt"}, {0x01,"vmcall"}, {0x02,"lar"}, {0x03,"lsl"}, {0x04,"?04"}, {0x05,"syscall"},
    {0x06,"clts"}, {0x07,"sysret"}, {0x08,"invd"}, {0x09,"wbinvd"}, {0x0A,"?0A"}, {0x0B,"ud2"},
    {0x0C,"?0C"}, {0x0D,"nop"}, {0x0E,"?0E"},
    {0x10,"movups"}, {0x11,"movups"}, {0x12,"movvhlps"}, {0x13,"movlps"}, {0x14,"unpcklps"}, {0x15,"unpckhps"}, {0x16,"movlhps"}, {0x17,"movhps"},
    {0x18,"prefetchnta"}, {0x19,"hint_nop"}, {0x1A,"hint_nop"}, {0x1B,"hint_nop"}, {0x1C,"hint_nop"}, {0x1D,"hint_nop"}, {0x1E,"hint_nop"}, {0x1F,"nop"},
    {0x20,"mov"}, {0x21,"mov"}, {0x22,"mov"}, {0x23,"mov"}, {0x24,"?24"}, {0x25,"?25"}, {0x26,"?26"}, {0x27,"?27"},
    {0x28,"movaps"}, {0x29,"movaps"}, {0x2A,"cvtpi2ps"}, {0x2B,"movntps"}, {0x2C,"cvtpi2ps"}, {0x2D,"cvtps2si"}, {0x2E,"ucomiss"}, {0x2F,"comiss"},
    {0x30,"wrmsr"}, {0x31,"rdtsc"}, {0x32,"rdmsr"}, {0x33,"rdpmc"}, {0x34,"sysenter"}, {0x35,"sysexit"}, {0x36,"?36"}, {0x37,"getsec"},
    {0x38,"?38"}, {0x39,"?39"}, {0x3A,"?3A"}, {0x3B,"?3B"}, {0x3C,"?3C"}, {0x3D,"?3D"}, {0x3E,"?3E"}, {0x3F,"?3F"},
    {0x40,"cmovo"}, {0x41,"cmovno"}, {0x42,"cmovb"}, {0x43,"cmovnb"}, {0x44,"cmovz"}, {0x45,"cmovnz"}, {0x46,"cmovbe"}, {0x47,"cmovnbe"},
    {0x48,"cmovs"}, {0x49,"cmovns"}, {0x4A,"cmovp"}, {0x4B,"cmovnp"}, {0x4C,"cmovl"}, {0x4D,"cmovnl"}, {0x4E,"cmovle"}, {0x4F,"cmovnle"},
    {0x50,"movmskps"}, {0x51,"sqrtps"}, {0x52,"rsqrtps"}, {0x53,"rcpps"}, {0x54,"andps"}, {0x55,"andnps"}, {0x56,"orps"}, {0x57,"xorps"},
    {0x58,"addps"}, {0x59,"mulps"}, {0x5A,"cv2pspd"}, {0x5B,"cvtdq2ps"}, {0x5C,"pop"}, {0x5D,"pop"}, {0x5E,"pop"}, {0x5F,"pop"},
    {0x60,"paddb"}, {0x61,"paddw"}, {0x62,"?62"}, {0x63,"movsxd"}, {0x64,"?64"}, {0x65,"?65"}, {0x66,"operand_size"}, {0x67,"address_size"},
    {0x68,"push"}, {0x69,"imul"}, {0x6A,"push"}, {0x6B,"imul"}, {0x6C,"ins"}, {0x6D,"ins"}, {0x6E,"outs"}, {0x6F,"outs"},
    {0x70,"jo"}, {0x71,"jno"}, {0x72,"jb"}, {0x73,"jnb"}, {0x74,"jz"}, {0x75,"jnz"}, {0x76,"jbe"}, {0x77,"jnbe"},
    {0x78,"js"}, {0x79,"jns"}, {0x7A,"jp"}, {0x7B,"jnp"}, {0x7C,"jl"}, {0x7D,"jge"}, {0x7E,"jle"}, {0x7F,"jg"},
    {0x80,"grp1"}, {0x81,"grp1"}, {0x82,"?82"}, {0x83,"grp1"}, {0x84,"test"}, {0x85,"test"}, {0x86,"xchg"}, {0x87,"xchg"},
    {0x88,"mov"}, {0x89,"mov"}, {0x8A,"mov"}, {0x8B,"mov"}, {0x8C,"mov"}, {0x8D,"lea"}, {0x8E,"mov"}, {0x8F,"pop"},
    {0x90,"nop"}, {0x91,"xchg"}, {0x92,"xchg"}, {0x93,"xchg"}, {0x94,"xchg"}, {0x95,"xchg"}, {0x96,"xchg"}, {0x97,"xchg"},
    {0x98,"cbw/cwde"}, {0x99,"cwd/cdq"}, {0x9A,"call"}, {0x9B,"fwait"}, {0x9C,"pushf"}, {0x9D,"popf"}, {0x9E,"sahf"}, {0x9F,"lahf"},
    {0xA0,"mov"}, {0xA1,"mov"}, {0xA2,"mov"}, {0xA3,"mov"}, {0xA4,"movs"}, {0xA5,"movs"}, {0xA6,"cmps"}, {0xA7,"cmps"},
    {0xA8,"test"}, {0xA9,"test"}, {0xAA,"stos"}, {0xAB,"stos"}, {0xAC,"lods"}, {0xAD,"lods"}, {0xAE,"scas"}, {0xAF,"scas"},
    {0xB0,"mov"}, {0xB1,"mov"}, {0xB2,"mov"}, {0xB3,"mov"}, {0xB4,"mov"}, {0xB5,"mov"}, {0xB6,"movzx"}, {0xB7,"movzx"},
    {0xB8,"mov"}, {0xB9,"mov"}, {0xBA,"mov"}, {0xBB,"mov"}, {0xBC,"bsf"}, {0xBD,"bsr"}, {0xBE,"movsx"}, {0xBF,"movsx"},
    {0xC0,"rol"}, {0xC1,"rol"}, {0xC2,"ret"}, {0xC3,"ret"}, {0xC4,"les"}, {0xC5,"lds"}, {0xC6,"mov"}, {0xC7,"mov"},
    {0xC8,"enter"}, {0xC9,"leave"}, {0xCA,"retf"}, {0xCB,"retf"}, {0xCC,"int3"}, {0xCD,"int"}, {0xCE,"into"}, {0xCF,"iret"},
    {0xD0,"rol"}, {0xD1,"rol"}, {0xD2,"rol"}, {0xD3,"rol"}, {0xD4,"aam"}, {0xD5,"aad"}, {0xD6,"?D6"}, {0xD7,"xlat"},
    {0xD8,"fp"}, {0xD9,"fp"}, {0xDA,"fp"}, {0xDB,"fp"}, {0xDC,"fp"}, {0xDD,"fp"}, {0xDE,"fp"}, {0xDF,"fp"},
    {0xE0,"loopne"}, {0xE1,"loope"}, {0xE2,"loop"}, {0xE3,"jcxz"}, {0xE4,"in"}, {0xE5,"in"}, {0xE6,"out"}, {0xE7,"out"},
    {0xE8,"call"}, {0xE9,"jmp"}, {0xEA,"jmp"}, {0xEB,"jmp"}, {0xEC,"in"}, {0xED,"in"}, {0xEE,"out"}, {0xEF,"out"},
    {0xF0,"lock"}, {0xF1,"int1"}, {0xF2,"repne"}, {0xF3,"repe"}, {0xF4,"hlt"}, {0xF5,"cmc"}, {0xF6,"grp3"}, {0xF7,"grp3"},
    {0xF8,"clc"}, {0xF9,"stc"}, {0xFA,"cli"}, {0xFB,"sti"}, {0xFC,"cld"}, {0xFD,"std"}, {0xFE,"grp4"}, {0xFF,"grp5"}
};

static size_t opcodes16_count = sizeof(opcodes16)/sizeof(opcodes16[0]);

static const char* decode_register(uint8_t opcode, bool rex, bool b16_prefix, bool rexW, bool rexB, uint8_t reg_code) {
    (void)rex;

    uint8_t reg = reg_code;

    switch(opcode) {
        case 0x40 ... 0x4F:
            reg = opcode & 7;
            break;

        // PUSH rAX..rDI (50–57)
        case 0x50 ... 0x57:
            reg = opcode & 7;
            break;

        // POP rAX..rDI (58–5F)
        case 0x58 ... 0x5F:
            reg = opcode & 7;
            break;

        // MOV r64, imm (B8–BF)
        case 0xB8 ... 0xBF:
            reg = opcode & 7;
            break;

        // XCHG rAX, r (90–97)
        case 0x90 ... 0x97:
            reg = opcode & 7;
            break;

        default:
            break;
    }

    if (rexW && !rexB && !b16_prefix) {
        switch (reg) {
            case 0: return "%rax";
            case 1: return "%rcx";
            case 2: return "%rdx";
            case 3: return "%rbx";
            case 4: return "%rsp";
            case 5: return "%rbp";
            case 6: return "%rsi";
            case 7: return "%rdi"; 
            default: return "(invalid)";
        }
    } else if (rexW && rexB && !b16_prefix) {
        switch (reg) {
            case 0: return "%r8";
            case 1: return "%r9";
            case 2: return "%r10";
            case 3: return "%r11";
            case 4: return "%r12";
            case 5: return "%r13";
            case 6: return "%r14";
            case 7: return "%r15";
            default: return "(invalid)";
        }
    } else if (!rexW && rexB && !b16_prefix) {
        switch (reg) {
            case 0: return "%r8d";
            case 1: return "%r9d";
            case 2: return "%r10d";
            case 3: return "%r11d";
            case 4: return "%r12d";
            case 5: return "%r13d";
            case 6: return "%r14d";
            case 7: return "%r15d";
            default: return "(invalid)";
        }
    } else if (!rexW && !rexB && !b16_prefix) {
        switch (reg) {
            case 0: return "%eax";
            case 1: return "%ecx";
            case 2: return "%edx";
            case 3: return "%ebx";
            case 4: return "%esp";
            case 5: return "%ebp";
            case 6: return "%esi";
            case 7: return "%edi";
            default: return "(invalid)";
        }
    } else if (!rexW && !rexB && b16_prefix) {
        switch (reg) {
            case 0: return "%ax";
            case 1: return "%cx";
            case 2: return "%dx";
            case 3: return "%bx";
            case 4: return "%sp";
            case 5: return "%bp";
            case 6: return "%si";
            case 7: return "%di";
            default: return "(invalid)";
        }
    } else if (!rexW && rexB && b16_prefix) {
        switch (reg) {
            case 0: return "%r8w";
            case 1: return "%r9w";
            case 2: return "%r10w";
            case 3: return "%r11w";
            case 4: return "%r12w";
            case 5: return "%r13w";
            case 6: return "%r14w";
            case 7: return "%r15w";
            default: return "(invalid)";
        }
    } else {
        return "(invalid?)";
    }
}

static bool opcode_needs_modrm(uint8_t op, bool is_0f_prefix, uint8_t op_0f) {
    if (is_0f_prefix) {
        // 0F extended opcodes that need ModRM
        switch (op_0f) {
            case 0x01: case 0x20: case 0x21: case 0x22: case 0x23:
            case 0xBA: case 0xAF: case 0xB6: case 0xB7:
                return true;
            default: return false;
        }
    }

    // 1-byte opcodes needing ModRM
    switch (op) {
        case 0x00: case 0x01: case 0x02: case 0x03:
        case 0x08: case 0x09: case 0x0A: case 0x0B:
        case 0x10: case 0x11: case 0x12: case 0x13:
        case 0x20: case 0x21: case 0x22: case 0x23:
        case 0x28: case 0x29: case 0x2A: case 0x2B:
        case 0x30: case 0x31: case 0x32: case 0x33:
        case 0x38: case 0x39: case 0x3A: case 0x3B:
        case 0x63: // movsxd
        case 0x69: case 0x6B: // imul r,r/m,imm
        case 0x80: case 0x81: case 0x82: case 0x83: // grp1 imm
        case 0x84: case 0x85: // test r/m,r
        case 0x88: case 0x89: case 0x8A: case 0x8B: // mov
        case 0x8D: // lea
        case 0xC6: case 0xC7: // mov r/m, imm
        case 0xF6: case 0xF7: // grp3
        case 0xFF: // grp5
            return true;
        case 0x0F: // two-byte prefix
            return true;
        default:
            return false;
    }
}

static bool opcode_needs_imm(uint8_t op, bool rex_w, bool op16, size_t* immsize, bool is_0f_prefix, uint8_t op_0f) {
    if (is_0f_prefix) {
        // Jcc near 0F 8x
        if (op_0f >= 0x80 && op_0f <= 0x8F) {
            *immsize = 4; // near jump offset
            return true;
        }
        return false;
    }

    // 1-byte opcodes
    switch (op) {
        case 0x80: *immsize = 1; return true;
        case 0x81: *immsize = op16 ? 2 : 4; return true;
        case 0x83: *immsize = 1; return true;
        case 0xC6: *immsize = 1; return true;
        case 0xC7: *immsize = rex_w ? 4 : op16 ? 2 : 4; return true;
        case 0xC0: case 0xC1: *immsize = 1; return true;
        case 0xEB: *immsize = 1; return true;
        case 0xE8: case 0xE9: *immsize = 4; return true;
        case 0x6A: *immsize = 1; return true;
        case 0x68: *immsize = 4; return true;
        case 0x69: *immsize = op16 ? 2 : 4; return true;
        case 0x6B: *immsize = 1; return true;
        case 0xA9: *immsize = op16 ? 2 : 4; return true;
        case 0x04: *immsize = 1; return true;
        case 0x05: *immsize = 4; return true;
        case 0x0C: *immsize = 1; return true;
        case 0x0D: *immsize = 4; return true;
        case 0x14: *immsize = 1; return true;
        case 0x15: *immsize = 4; return true;
        case 0x1C: *immsize = 1; return true;
        case 0x1D: *immsize = 4; return true;
        case 0x24: *immsize = 1; return true;
        case 0x25: *immsize = 4; return true;
        case 0x2C: *immsize = 1; return true;
        case 0x2D: *immsize = 4; return true;
        case 0x34: *immsize = 1; return true;
        case 0x35: *immsize = 4; return true;
        case 0x3C: *immsize = 1; return true;
        case 0x3D: *immsize = 4; return true;
    }

    // B8–BF mov r, imm
    if ((op & 0xF8) == 0xB8) {
        if (rex_w) *immsize = 8;
        else if (op16) *immsize = 2;
        else *immsize = 4;
        return true;
    }

    return false;
}

static bool opcode_register_extension(uint8_t opcode, bool rex_present, bool b16_prefix, bool rex_w, bool rex_b, char* out, size_t outsz, bool is_0f_prefix, uint8_t op_0f) {
    uint8_t base = opcode & 0xF8;
    uint8_t r = opcode & 0x07;
    bool matched = false;

    if (!is_0f_prefix) {
        if (base == 0xB8) matched = true; // mov r, imm
        else if (base == 0x50) matched = true; // push r
        else if (base == 0x40) matched = true; // inc/dec r
        else if (base == 0x90) matched = true; // xchg rAX,r*
    } else {
        // 0F extended opcodes that embed register
        if (op_0f == 0xB6 || op_0f == 0xB7) matched = true; // MOVZX
        else if (op_0f == 0xBE || op_0f == 0xBF) matched = true; // MOVSX
        else if (op_0f >= 0x40 && op_0f <= 0x4F) matched = true; // CMOVcc
        else if (op_0f == 0xAF) matched = true; // IMUL r,r/m
    }

    if (!matched) return false;

    uint8_t reg = r;
    if (rex_b) reg |= 0x8;

    const char* regname = decode_register(opcode, rex_present, b16_prefix, rex_w, rex_b, reg);
    size_t off = strlen(out);
    snprintf(out + off, outsz - off, CB_CYAN " %s," C_WHITE, regname);
    return true;
}

static op_form get_op_form(uint8_t opcode) {
    switch(opcode) {
        case 0x01: case 0x09: case 0x11: case 0x19:
        case 0x21: case 0x29: case 0x31: case 0x39:
        case 0x89:
            return OP_RM_REG;

        case 0x03: case 0x0B: case 0x13: case 0x1B:
        case 0x23: case 0x2B: case 0x33: case 0x3B:
        case 0x8B:
            return OP_REG_RM;
    }
    return OP_RM_REG;
}

void decode_x86(uint8_t* data, size_t max_size, size_t* offset, size_t cvaddr, char* out, size_t outsz) {
    size_t og_offset = *offset;
    x64_inst inst = {0};

    out[0] = '\0';

    bool prefix_done = false;
    while (!prefix_done) {
        if (*offset + 1 > max_size) { *offset += 2; return; }
        uint8_t b = data[*offset];
        if (b >= 0x40 && b <= 0x4F) {
            inst.rex_present = true;
            inst.rex_w = (b >> 3) & 1;
            inst.rex_r = (b >> 2) & 1;
            inst.rex_x = (b >> 1) & 1;
            inst.rex_b = (b >> 0) & 1;
            (*offset)++;
        } else if (b == 0x66) {
            inst.b16_prefix = true;
            (*offset)++;
        } else {
            prefix_done = true;
        }
    }

    uint8_t primary_opcode = 0;
    uint8_t secondary_opcode = 0;
    
    bool is_0f = (data[*offset] == 0x0F);
    if (!is_0f) {
        if (*offset + 1 > max_size) { *offset += 2; return; }
        inst.opcode = (uint8_t)(data[*offset]);
        secondary_opcode = inst.opcode;
        sprintf(out, CB_RED "%s", get_value_hashmap8(opcodes8, inst.opcode, opcodes8_count));
        *offset += 1;
    } else {
        if (*offset + 2 > max_size) { *offset += 3; return; }
        primary_opcode = 0x0F;
        secondary_opcode = data[*offset + 1];

        inst.opcode = secondary_opcode;
        inst.opcode16 = true;

        sprintf(out, CB_RED "%s", get_value_hashmap8(opcodes16, secondary_opcode, opcodes16_count));
        *offset += 2;
    }
    
    bool has_modrm = opcode_needs_modrm(inst.opcode, inst.opcode16, secondary_opcode); 
    if (has_modrm) {
        if (*offset + 1 > max_size) { *offset += 2; return; }
        inst.modrm = data[*offset];
        (*offset)++;

        uint8_t mod = (inst.modrm & 0b11000000) >> 6;
        uint8_t rm  = (inst.modrm & 0b00000111);
        uint8_t reg = (inst.modrm & 0b00111000) >> 3;

        op_form form = get_op_form(inst.opcode);
        const char *dst;
        const char *src;

        if (form == OP_RM_REG) {
            dst = decode_register(secondary_opcode, inst.rex_present, inst.b16_prefix, inst.rex_w, inst.rex_b, rm);
            src = decode_register(secondary_opcode, inst.rex_present, inst.b16_prefix, inst.rex_w, inst.rex_b, reg);
        } else {
            dst = decode_register(secondary_opcode, inst.rex_present, inst.b16_prefix, inst.rex_w, inst.rex_b, reg);
            src = decode_register(secondary_opcode, inst.rex_present, inst.b16_prefix, inst.rex_w, inst.rex_b, rm);
        }

        if (rm == 4 && mod != 3) { // SIB present
            if (*offset + 1 > max_size) { *offset += 2; return; }
            inst.sib = data[*offset];
            uint8_t scale = (inst.sib & 0b11000000) >> 6;
            uint8_t index = (inst.sib & 0b00111000) >> 3;
            uint8_t base = (inst.sib & 0b00000111);

            if (inst.rex_present) {
                index |= inst.rex_x << 3;
                base  |= inst.rex_b << 3;
            }

            const char* base_str = decode_register(secondary_opcode, inst.rex_present, inst.b16_prefix, inst.rex_w, inst.rex_b, base);
            const char* index_str = (index == 4) ? "" : decode_register(secondary_opcode, inst.rex_present, inst.b16_prefix, inst.rex_w, inst.rex_b, index); // 4 = no index
            int64_t disp_val = 0;
            (*offset)++;

            if (mod == 0 && base == 5) { // disp32
                if (*offset + 4 > max_size) { *offset += 5; return; }
                int32_t disp32;
                memcpy(&disp32, data + *offset, sizeof(disp32));
                disp_val = disp32;
                *offset += 4;
            } else if (mod == 1) { // disp8
                if (*offset + 1 > max_size) { *offset += 2; return; }
                int8_t disp8;
                memcpy(&disp8, data + *offset, sizeof(disp8));
                disp_val = disp8;
                *offset += 1;
            } else if (mod == 2) { // disp32
                if (*offset + 4 > max_size) { *offset += 5; return; }
                int32_t disp32;
                memcpy(&disp32, data + *offset, sizeof(disp32));
                disp_val = disp32;
                *offset += 4;
            }

            size_t out_off = strlen(out);
            if (index == 4)
                snprintf(out + out_off, outsz - out_off, CB_CYAN " %s, [" CB_WHITE "%s + 0x%lx]" C_WHITE, dst, base_str, disp_val);
            else
                snprintf(out + out_off, outsz - out_off, CB_CYAN " %s, [" CB_WHITE "%s + %s*%d + 0x%lx]" C_WHITE, dst, base_str, index_str, 1 << scale, disp_val);
        }
        // else if (mod == 0 && rm != 5) {
        //     if (*offset + 4 > max_size) { *offset += 5; return; }
        //     // Direct memory
        //     uint32_t imm32;
        //     memcpy(&imm32, data + *offset, sizeof(imm32));
        //     inst.imm = imm32;

        //     *offset += 4;
        //     size_t out_off = strlen(out);
        //     snprintf(out + out_off, outsz - out_off, CB_CYAN " %s, " CB_WHITE "[0x%lx] ", dst, inst.imm);
        // }
        else if (mod == 0 && rm == 5) {
            if (*offset + 4 > max_size) { *offset += 5; return; }
            // RIP Disp32
            int32_t disp32;
            memcpy(&disp32, data + *offset, sizeof(disp32));
            inst.disp = disp32;

            *offset += 4;
            size_t out_off = strlen(out);
            size_t next_ip = cvaddr + (*offset - og_offset) + 4;
            snprintf(out + out_off, outsz - out_off, CB_CYAN " %s, " CB_WHITE "[0x%lx]" C_WHITE " // RIP disp32, 0x%lx ", dst, inst.disp, (size_t)(next_ip + inst.disp));
        }
        else if (mod == 1) {
            if (*offset + 1 > max_size) { *offset += 2; return; }
            // Disp8
            int8_t disp32;
            memcpy(&disp32, data + *offset, sizeof(disp32));
            inst.disp = disp32;

            *offset += 1;
            size_t out_off = strlen(out);
            size_t next_ip = cvaddr + (*offset - og_offset) + 4;
            snprintf(out + out_off, outsz - out_off, CB_CYAN " %s, " CB_WHITE "[0x%lx]" C_WHITE " // disp8, 0x%lx ", dst, inst.disp, (size_t)(next_ip + inst.disp));
        }
        else if (mod == 2) {
            if (*offset + 4 > max_size) { *offset += 5; return; }
            // Disp32
            int32_t disp32;
            memcpy(&disp32, data + *offset, sizeof(disp32));
            inst.disp = disp32;

            *offset += 4;
            size_t out_off = strlen(out);
            size_t next_ip = cvaddr + (*offset - og_offset) + 4;
            snprintf(out + out_off, outsz - out_off, CB_CYAN " %s, " CB_WHITE "[0x%lx]" C_WHITE " // disp32, 0x%lx ", dst, inst.disp, (size_t)(next_ip + inst.disp));
        } else if (mod == 3) {
            // Register
            size_t out_off = strlen(out);
            snprintf(out + out_off, outsz - out_off, CB_CYAN " %s, " CB_WHITE "%s ", dst, src);
        }
    }

    opcode_register_extension(inst.opcode, inst.rex_present, inst.b16_prefix, inst.rex_w, inst.rex_b, out, outsz, inst.opcode16, secondary_opcode);

    size_t immsize = 4;
    if (opcode_needs_imm(inst.opcode, inst.rex_w, inst.b16_prefix, &immsize, inst.opcode16, secondary_opcode)) {
        if (immsize == 4) {
            int32_t imm32;
            if (*offset + sizeof(imm32) > max_size) { *offset += sizeof(imm32) + 1; return; }
            memcpy(&imm32, data + *offset, sizeof(imm32));
            inst.imm = imm32;
        } else if (immsize == 2) {
            int16_t imm16;
            if (*offset + sizeof(imm16) > max_size) { *offset += sizeof(imm16) + 1; return; }
            memcpy(&imm16, data + *offset, sizeof(imm16));
            inst.imm = imm16;
        } else if (immsize == 1) {
            int8_t imm8;
            if (*offset + sizeof(imm8) > max_size) { *offset += sizeof(imm8) + 1; return; }
            memcpy(&imm8, data + *offset, sizeof(imm8));
            inst.imm = imm8;
        } else if (immsize == 8) {
            int64_t imm64;
            if (*offset + sizeof(imm64) > max_size) { *offset += sizeof(imm64) + 1; return; }
            memcpy(&imm64, data + *offset, sizeof(imm64));
            inst.imm = imm64;
        }

        *offset += immsize;

        size_t out_off = strlen(out);
        snprintf(out + out_off, outsz - out_off, CB_CYAN " 0x%lx ", inst.imm);
    }

    if (out[0] == '\0') {
        strcpy(out, "Could not decode!");
    }

    size_t out_off = strlen(out);

    snprintf(out + out_off, outsz - out_off, C_WHITE "\t\t\t - %02X  ", (uint8_t)data[og_offset]);
    for (size_t i = 1; i < (*offset - og_offset); i++) {
        out_off = strlen(out);
        snprintf(out + out_off, outsz - out_off, "%02X  ", (uint8_t)data[og_offset + i]);
    }

    return;
}
