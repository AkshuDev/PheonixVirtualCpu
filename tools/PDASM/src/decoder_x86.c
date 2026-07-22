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

// Full Support Complete
static HashMap8 opcodes8[] = {
    {0x00,"add"}, {0x01,"add"}, {0x02,"add"}, {0x03,"add"}, {0x04,"add"}, {0x05,"add"},
    {0x08,"or"}, {0x09,"or"}, {0x0A,"or"}, {0x0B,"or"}, {0x0C,"or"}, {0x0D,"or"}, 
	{0x10,"adc"}, {0x11,"adc"}, {0x12,"adc"}, {0x13,"adc"}, {0x14,"adc"}, {0x15,"adc"},
    {0x18,"sbb"}, {0x19,"sbb"}, {0x1A,"sbb"}, {0x1B,"sbb"}, {0x1C,"sbb"}, {0x1D,"sbb"},
    {0x20,"and"}, {0x21,"and"}, {0x22,"and"}, {0x23,"and"}, {0x24,"and"}, {0x25,"and"}, {0x26,"null"}, 
	{0x28,"sub"}, {0x29,"sub"}, {0x2A,"sub"}, {0x2B,"sub"}, {0x2C,"sub"}, {0x2D,"sub"}, {0x2E,"null"},
    {0x30,"xor"}, {0x31,"xor"}, {0x32,"xor"}, {0x33,"xor"}, {0x34,"xor"}, {0x35,"xor"},
    {0x36,"null"}, {0x38,"cmp"}, {0x39,"cmp"}, {0x3A,"cmp"}, {0x3B,"cmp"}, {0x3C,"cmp"}, {0x3D,"cmp"}, {0x3E,"null"}, 
	
	{0x50,"push"}, {0x51,"push"}, {0x52,"push"}, {0x53,"push"}, {0x54,"push"}, {0x55,"push"}, {0x56,"push"}, {0x57,"push"},
	{0x58,"pop"}, {0x59,"pop"}, {0x5A,"pop"}, {0x5B,"pop"}, {0x5C,"pop"}, {0x5D,"pop"}, {0x5E,"pop"}, {0x5F,"pop"},
    
	{0x63,"movsxd"}, {0x68,"push"}, {0x69,"imul"}, {0x6A,"push"}, {0x6B,"imul"},
	
	{0x6C,"insb"}, {0x6D,"insw"}, {0x6E,"outsb"}, {0x6F,"outsw"},
	
	{0x70,"jo"}, {0x71,"jno"}, {0x72,"jc"}, {0x73,"jnc"}, {0x74,"je"}, {0x75,"jne"}, {0x76,"jbe"}, {0x77,"jnbe"},
    {0x78,"js"}, {0x79,"jns"}, {0x7A,"jp"}, {0x7B,"jnp"}, {0x7C,"jl"}, {0x7D,"jge"}, {0x7E,"jle"}, {0x7F,"jg"},
    
	{0x84,"test"}, {0x85,"test"}, {0x86,"xchg"}, {0x87,"xchg"},
    {0x88,"mov"}, {0x89,"mov"}, {0x8A,"mov"}, {0x8B,"mov"}, {0x8C,"mov"},
	{0x8D,"lea"}, {0x8E,"mov"}, {0x8F,"pop"},

    {0x90,"nop"}, {0x91,"xchg"}, {0x92,"xchg"}, {0x93,"xchg"}, {0x94,"xchg"}, {0x95,"xchg"}, {0x96,"xchg"}, {0x97,"xchg"},
    {0x98,"cbw"}, {0x99,"cwd"}, {0x9B,"fwait"}, {0x9C,"pushf"}, {0x9D,"popf"}, {0x9E,"sahf"}, {0x9F,"lahf"},
    {0xA0,"mov"}, {0xA1,"mov"}, {0xA2,"mov"}, {0xA3,"mov"}, {0xA4,"movsb"}, {0xA5,"movsw"}, {0xA6,"cmpsb"}, {0xA7,"cmpsw"},
    
	{0xA8,"test"}, {0xA9,"test"}, {0xAA,"stosb"}, {0xAB,"stosw"}, {0xAC,"lodsb"}, {0xAD,"lodsw"}, {0xAE,"scasb"}, {0xAF,"scasw"},

    {0xB0,"mov"}, {0xB1,"mov"}, {0xB2,"mov"}, {0xB3,"mov"}, {0xB4,"mov"}, {0xB5,"mov"}, {0xB6,"mov"}, {0xB7,"mov"},
    {0xB8,"mov"}, {0xB9,"mov"}, {0xBA,"mov"}, {0xBB,"mov"}, {0xBC,"mov"}, {0xBD,"mov"}, {0xBE,"mov"}, {0xBF,"mov"},

    {0xC2,"ret"}, {0xC3,"ret"}, {0xC6,"mov"}, {0xC7,"mov"},
    {0xC8,"enter"}, {0xC9,"leave"}, {0xCA,"retf"}, {0xCB,"retf"}, {0xCC,"int3"}, {0xCD,"int"}, {0xCE,"into"}, {0xCF,"iret"},
    
	{0xD7,"xlat"},
    {0xE0,"loopne"}, {0xE1,"loope"}, {0xE2,"loop"}, {0xE3,"jecxz"}, {0xE4,"in"}, {0xE5,"in"}, {0xE6,"out"}, {0xE7,"out"},
    {0xE8,"call"}, {0xE9,"jmp"}, {0xEB,"jmp"}, {0xEC,"in"}, {0xED,"in"}, {0xEE,"out"}, {0xEF,"out"},

    {0xF1,"int1"}, {0xF2,"repne"}, {0xF3,"repe"}, {0xF4,"hlt"}, {0xF5,"cmc"},

    {0xF8,"clc"}, {0xF9,"stc"}, {0xFA,"cli"}, {0xFB,"sti"}, {0xFC,"cld"}, {0xFD,"std"},
};

static size_t opcodes8_count = sizeof(opcodes8)/sizeof(HashMap8);

// Full Support Complete
static HashMap8 opcodes16[] = {
    {0x02,"lar"}, {0x03,"lsl"}, {0x05,"syscall"}, {0x06,"clts"}, {0x07,"sysret"},
	{0x08,"invd"}, {0x09,"wbinvd"}, {0x0B,"ud2"}, {0x0D,"nop"},

    {0x10,"movups"}, {0x11,"movups"}, {0x12,"movlps"}, {0x13,"movlps"}, {0x14,"unpcklps"}, {0x15,"unpckhps"}, {0x16,"movhps"}, {0x17,"movhps"},

    {0x18,"prefetchnta"}, {0x19,"hint_nop"}, {0x1A,"hint_nop"}, {0x1B,"hint_nop"}, {0x1C,"hint_nop"}, {0x1D,"hint_nop"}, {0x1E,"hint_nop"}, {0x1F,"nop"},
    
	{0x20,"mov"}, {0x21,"mov"}, {0x22,"mov"}, {0x23,"mov"},
    {0x28,"movaps"}, {0x29,"movaps"}, {0x2A,"cvtpi2ps"}, {0x2B,"movntps"}, {0x2C,"cvttps2pi"}, {0x2D,"cvtps2pi"}, {0x2E,"ucomiss"}, {0x2F,"comiss"},
    {0x30,"wrmsr"}, {0x31,"rdtsc"}, {0x32,"rdmsr"}, {0x33,"rdpmc"}, {0x34,"sysenter"}, {0x35,"sysexit"}, {0x37,"getsec"},

    {0x40,"cmovo"}, {0x41,"cmovno"}, {0x42,"cmovb"}, {0x43,"cmovnb"}, {0x44,"cmovz"}, {0x45,"cmovnz"}, {0x46,"cmovbe"}, {0x47,"cmovnbe"},
    {0x48,"cmovs"}, {0x49,"cmovns"}, {0x4A,"cmovp"}, {0x4B,"cmovnp"}, {0x4C,"cmovl"}, {0x4D,"cmovnl"}, {0x4E,"cmovle"}, {0x4F,"cmovnle"},
    {0x50,"movmskps"}, {0x51,"sqrtps"}, {0x52,"rsqrtps"}, {0x53,"rcpps"}, {0x54,"andps"}, {0x55,"andnps"}, {0x56,"orps"}, {0x57,"xorps"},
    {0x58,"addps"}, {0x59,"mulps"}, {0x5A,"cvtps2pd"}, {0x5B,"cvtdq2ps"}, {0x5C,"subps"}, {0x5D,"minps"}, {0x5E,"divps"}, {0x5F,"maxps"},
    {0x60,"punpcklbw"}, {0x61,"punpcklwd"}, {0x62,"punpckldq"}, {0x63,"packsswb"}, {0x64,"pcmpgtb"}, {0x65,"pcmpgtw"}, {0x66,"pcmpgtd"}, {0x67,"packuswb"},
    {0x68,"punpckhbw"}, {0x69,"punpckhwd"}, {0x6A,"punpckhdq"}, {0x6B,"packssdw"}, {0x6C,"punpcklqdq"}, {0x6D,"punpckhqdq"}, {0x6E,"movd"}, {0x6F,"movq"},
    {0x70,"pshufw"}, {0x74,"pcmpeqb"}, {0x75,"pcmpeqw"}, {0x76,"pcmpeqd"}, {0x77,"emms"},
    {0x78,"vmread"}, {0x79,"vmwrite"}, {0x7C,"haddpd"}, {0x7D,"hsubpd"}, {0x7E,"movd"}, {0x7F,"movq"},
    
	{0x80,"jo"}, {0x81,"jno"}, {0x82,"jc"}, {0x83,"jnc"}, {0x84,"je"}, {0x85,"jne"}, {0x86,"jbe"}, {0x87,"jnbe"},
    {0x88,"js"}, {0x89,"jns"}, {0x8A,"jp"}, {0x8B,"jnp"}, {0x8C,"jl"}, {0x8D,"jge"}, {0x8E,"jle"}, {0x8F,"jg"},
    
	{0x90,"seto"}, {0x91,"setno"}, {0x92,"setc"}, {0x93,"setnc"}, {0x94,"setz"}, {0x95,"setnz"}, {0x96,"setbe"}, {0x97,"setnbe"},
    {0x98,"sets"}, {0x99,"setns"}, {0x9A,"setp"}, {0x9B,"setnp"}, {0x9C,"setl"}, {0x9D,"setge"}, {0x9E,"setle"}, {0x9F,"setg"},
    
	{0xA0,"push"}, {0xA1,"pop"}, {0xA2,"cpuid"}, {0xA3,"bt"}, {0xA4,"shld"}, {0xA5,"shld"},
    {0xA8,"push"}, {0xA9,"pop"}, {0xAA,"rsm"}, {0xAB,"bts"}, {0xAC,"shrd"}, {0xAD,"shrd"}, {0xAF,"imul"},

    {0xB0,"cmpxchg"}, {0xB1,"cmpxchg"}, {0xB2,"lss"}, {0xB3,"btr"}, {0xB4,"lfs"}, {0xB5,"lgs"}, {0xB6,"movzx"}, {0xB7,"movzx"},
    {0xB8,"popcnt"}, {0xB9,"ud"}, {0xBA,"bt"}, {0xBB,"btc"}, {0xBC,"bsf"}, {0xBD,"bsr"}, {0xBE,"movsx"}, {0xBF,"movsx"},
    {0xC0,"xadd"}, {0xC1,"xadd"}, {0xC2,"cmpps"}, {0xC3,"movnti"}, {0xC4,"pinsrw"}, {0xC5,"pextrw"}, {0xC6,"shufps"}, {0xC7,"cmpxchg8b"},
    {0xC8,"bswap"}, {0xC9,"bswap"}, {0xCA,"bswap"}, {0xCB,"bswap"}, {0xCC,"bswap"}, {0xCD,"bswap"}, {0xCE,"bswap"}, {0xCF,"bswap"},
    
	{0xD0,"addsubpd"}, {0xD1,"psrlw"}, {0xD2,"psrld"}, {0xD3,"psrlq"}, {0xD4,"paddq"}, {0xD5,"pmullw"}, {0xD6,"movq2dq"}, {0xD7,"pmovmskb"},
    {0xD8,"psubusb"}, {0xD9,"psubusw"}, {0xDA,"pminub"}, {0xDB,"pand"}, {0xDC,"paddusb"}, {0xDD,"paddusd"}, {0xDE,"pmaxub"}, {0xDF,"pandn"},
    
	{0xE0,"pavgb"}, {0xE1,"psraw"}, {0xE2,"psrad"}, {0xE3,"pavgw"}, {0xE4,"pmulhuw"}, {0xE5,"pmulhw"}, {0xE6,"cvtpd2dq"}, {0xE7,"movntq"},
    {0xE8,"psubsb"}, {0xE9,"psubsw"}, {0xEA,"pminsw"}, {0xEB,"por"}, {0xEC,"paddsb"}, {0xED,"paddsw"}, {0xEE,"pmaxsw"}, {0xEF,"pxor"},
    
	{0xF0,"lddqu"}, {0xF1,"psllw"}, {0xF2,"pslld"}, {0xF3,"psllq"}, {0xF4,"pmuludq"}, {0xF5,"pmaddwd"}, {0xF6,"psadbw"}, {0xF7,"maskmovq"},
    {0xF8,"psubb"}, {0xF9,"psubw"}, {0xFA,"psubd"}, {0xFB,"psubq"}, {0xFC,"paddb"}, {0xFD,"paddw"}, {0xFE,"paddd"}
};

static size_t opcodes16_count = sizeof(opcodes16)/sizeof(HashMap8);

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

// Full Support Complete
static bool opcode_needs_modrm(uint8_t op, bool is_0f_prefix, uint8_t op_0f) {
    if (is_0f_prefix) {
        // 0F extended opcodes that need ModRM
        switch (op_0f) {
            // Syscall and more Ops
			case 0x5: case 0x6: case 0x7: case 0x8:
			case 0x9: case 0xB:

			// System Ops
			case 0x30: case 0x31: case 0x32: case 0x33:
			case 0x34: case 0x35: case 0x36: case 0x37:
			case 0x77:

			// Branching Ops
			case 0x80: case 0x81: case 0x82: case 0x83:
			case 0x84: case 0x85: case 0x86: case 0x87:
			case 0x88: case 0x89: case 0x8A: case 0x8B:
			case 0x8C: case 0x8D: case 0x8E: case 0x8F:

			// More? Ops?
			case 0xA0: case 0xA1: case 0xA2: case 0xA8:
			case 0xA9: case 0xAA:

			// BSWAP
			case 0xC8: case 0xC9: case 0xCA: case 0xCB:
			case 0xCC: case 0xCD: case 0xCE: case 0xCF:

			// These do return false
			return false;

            default: return true;
        }
    }

    // 1-byte opcodes not needing ModRM
    switch (op) {
		// ALU Ops
        case 0x4: case 0x5: case 0xC: case 0xD:
		case 0x14: case 0x15: case 0x1C: case 0x1D:
		case 0x24: case 0x25: case 0x2C: case 0x2D:
		case 0x34: case 0x35: case 0x3C: case 0x3D:

		// Push/Pop Ops
		case 0x50: case 0x51: case 0x52: case 0x53:
		case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5A: case 0x5B:
		case 0x5C: case 0x5D: case 0x5E: case 0x5F:
		case 0x68: case 0x6A:

		// Branching Ops
		case 0x70: case 0x71: case 0x72: case 0x73:
		case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7A: case 0x7B:
		case 0x7C: case 0x7D: case 0x7E: case 0x7F:

		// More Ops ig
		case 0x90: case 0x98: case 0x99: case 0x9B:
		case 0x9C: case 0x9D: case 0x9E: case 0x9F:

		// Test Ops
		case 0xA8: case 0xA9:

		// Mov Ops
		case 0xB0: case 0xB1: case 0xB2: case 0xB3:
		case 0xB4: case 0xB5: case 0xB6: case 0xB7:
		case 0xB8: case 0xB9: case 0xBA: case 0xBB:
		case 0xBC: case 0xBD: case 0xBE: case 0xBF:

		// Ret Ops
		case 0xC2: case 0xC3:
		
		// Ret, Enter, Int Ops
		case 0xC8: case 0xC9: case 0xCA: case 0xCB:
		case 0xCC: case 0xCD: case 0xCE: case 0xCF:

		// Branching and more Ops
		case 0xE0: case 0xE1: case 0xE2: case 0xE3:
		case 0xE4: case 0xE5: case 0xE6: case 0xE7:
		case 0xE8: case 0xE9: case 0xEB: case 0xEC:
		case 0xED: case 0xEE: case 0xEF:
		
		// More Ops?
		case 0xF1: case 0xF2: case 0xF3: case 0xF4:
		case 0xF5: case 0xF8: case 0xF9: case 0xFA:
		case 0xFB: case 0xFC: case 0xFD:

		// They will do false return
		return false;

		default: return true;
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
        if (((b >> 4) & 0xF) == 0b0100) {
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
		uint8_t reg = (inst.modrm & 0b00111000) >> 3;
        uint8_t rm  = (inst.modrm & 0b00000111);

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
	while (out_off < 96) {
		out[out_off++] = ' ';
	}
	out[out_off] = '\0';

	if (out_off >= outsz) return;
    snprintf(out + out_off, outsz - out_off, C_WHITE "- %02X ", (uint8_t)data[og_offset]);
    for (size_t i = 1; i < (*offset - og_offset); i++) {
		out_off = strlen(out);
		if (out_off >= outsz) return;
        snprintf(out + out_off, outsz - out_off, "%02X ", (uint8_t)data[og_offset + i]);
    }

    return;
}
