// Author: Pheonix Studios/AkshuDev

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

#include <elf.h>
#include <elfutils.h>
#include <pecoff.h>
#include <pecoffutils.h>

#include <reader.h>

static bool validate_elf(char* src, size_t size, char* error_info) {
    if (size < EI_NIDENT) {
        strcpy(error_info, "File size is less than minimum required!");
        return false;
    }

    const Elf64_Ehdr* eh = (const Elf64_Ehdr*)src;
    if (
        eh->e_ident[EI_MAG0] != ELFMAG0 ||
        eh->e_ident[EI_MAG1] != ELFMAG1 ||
        eh->e_ident[EI_MAG2] != ELFMAG2 ||
        eh->e_ident[EI_MAG3] != ELFMAG3
    ) {
        strcpy(error_info, "Magic bytes don't match ELF magic!");
        return false;
    }

    if (eh->e_ident[EI_CLASS] != ELFCLASS32 && eh->e_ident[EI_CLASS] != ELFCLASS64) {
        strcpy(error_info, "ELF Class doesn't specify 32-bit or even 64-bit!");
        return false;
    }

    if (eh->e_ident[EI_DATA] != ELFDATA2LSB && eh->e_ident[EI_DATA] != ELFDATA2MSB) {
        strcpy(error_info, "ELF Data Type doesn't specify Little-Endian or even Big-Endian!");
        return false;
    }

    return true;
}

static bool validate_pe(char* src, size_t size, char* error_info) {
    if (size < sizeof(DOS_Hdr)) {
        strcpy(error_info, "(DOS Header) File size is less than minimum required!");
        return false;
    }

    if (src[0] != 'M' || src[1] != 'Z') {
        strcpy(error_info, "DOS Magic bytes don't match DOS magic!");
        return false;
    }

    const DOS_Hdr* doshdr = (const DOS_Hdr*)(src);

    size_t pe_offset = (size_t)(doshdr->e_lfanew);
    if (pe_offset + 4 >= size) {
        strcpy(error_info, "(PE Header) File size is less than minimum required!");
        return false;
    }

    if (memcmp(&src[pe_offset], PE_MAGIC, 4) != 0) {
        strcpy(error_info, "PE Magic bytes don't match PE magic!");
        return false;
    }

    const Optional_Hdr_64* ohdr = (const Optional_Hdr_64*)(src + doshdr->e_lfanew + sizeof(PE_Hdr));
    if (ohdr->magic != Optional_Hdr_32_Magic && ohdr->magic != Optional_Hdr_32p_Magic) {
        strcpy(error_info, "PE Optional Header doesn't contain valid type (Valid types: PE32, PE32+)!");
        return false;
    }

    return true;
}

static bool is_pe(char* src, size_t size) {
    if (size < 2) return false;
    if (src[0] == 'M' && src[1] == 'Z') return true;
    return false;
}

static void print_elf_header64(const Elf64_Ehdr* ehdr) {
    printf("===== ELF Header =====\n");

    printf("Class:            %s\n",
        ehdr->e_ident[EI_CLASS] == ELFCLASS64 ? "ELF64" : "ELF32");

    printf("Endianness:       %s\n",
        ehdr->e_ident[EI_DATA] ? "Little Endian" : "Big Endian");

    printf("Type:             0x%x\n", ehdr->e_type);
    printf("Machine:          %s\n", elf_machine_to_string((int)ehdr->e_machine));
    printf("Version:          0x%x\n", ehdr->e_version);
    printf("Entry Point:      0x%lx\n", (unsigned long)ehdr->e_entry);

    printf("Program Hdr Off:  0x%lx\n", (unsigned long)ehdr->e_phoff);
    printf("Section Hdr Off:  0x%lx\n", (unsigned long)ehdr->e_shoff);

    printf("PH Count:         %u\n", ehdr->e_phnum);
    printf("SH Count:         %u\n", ehdr->e_shnum);
    printf("SH String Index:  %u\n", ehdr->e_shstrndx);
}

static void print_elf_header32(const Elf32_Ehdr* ehdr) {
    printf("===== ELF Header =====\n");

    printf("Class:            %s\n",
        ehdr->e_ident[EI_CLASS] == ELFCLASS64 ? "ELF64" : "ELF32");

    printf("Endianness:       %s\n",
        ehdr->e_ident[EI_DATA] ? "Little Endian" : "Big Endian");

    printf("Type:             0x%x\n", ehdr->e_type);
    printf("Machine:          %s\n", elf_machine_to_string((int)ehdr->e_machine));
    printf("Version:          0x%x\n", ehdr->e_version);
    printf("Entry Point:      0x%lx\n", (unsigned long)ehdr->e_entry);

    printf("Program Hdr Off:  0x%lx\n", (unsigned long)ehdr->e_phoff);
    printf("Section Hdr Off:  0x%lx\n", (unsigned long)ehdr->e_shoff);

    printf("PH Count:         %u\n", ehdr->e_phnum);
    printf("SH Count:         %u\n", ehdr->e_shnum);
    printf("SH String Index:  %u\n", ehdr->e_shstrndx);
}

static void print_elf_program_headers64(const Elf64_Ehdr* ehdr, char* src, size_t size) {
    printf("\n===== Program Headers =====\n");

    if ((size_t)(ehdr->e_phoff) >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    printf("%-5s %-10s %-10s %-16s %-16s %-16s %-16s %-8s\n", "Idx", "Type", "Flags", "Offset", "VirtAddr", "PhysAddr", "FileSz", "MemSz");
    printf("----------------------------------------------------------------------------------------------------------------------------\n");

    for (size_t i = 0; i < ehdr->e_phnum; i++) {
        const Elf64_Phdr* ph = (const Elf64_Phdr*)(src + ehdr->e_phoff + i * ehdr->e_phentsize);
        char fbuf[4];
        elf_better_ph_flags(ph->p_flags, fbuf, sizeof(fbuf));

        printf(
            "%-5zu %-10s %-10s 0x%014lx 0x%014lx 0x%014lx 0x%014lx 0x%08lx\n",
            i, elf_ph_type_to_string(ph->p_type), fbuf,
            ph->p_offset, ph->p_vaddr, ph->p_paddr, ph->p_filesz, ph->p_memsz
        );
    }
}

static void print_elf_program_headers32(const Elf32_Ehdr* ehdr, char* src, size_t size) {
    printf("\n===== Program Headers =====\n");

    if ((size_t)(ehdr->e_phoff) >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    printf("%-5s %-10s %-8s %-10s %-10s %-10s %-10s %-8s\n", "Idx", "Type", "Flags", "Offset", "VirtAddr", "PhysAddr", "FileSz", "MemSz");
    printf("---------------------------------------------------------------------------------------------------------------------------\n");

    for (size_t i = 0; i < ehdr->e_phnum; i++) {
        const Elf32_Phdr* ph = (const Elf32_Phdr*)(src + ehdr->e_phoff + i * ehdr->e_phentsize);
        char fbuf[4];
        elf_better_ph_flags(ph->p_flags, fbuf, sizeof(fbuf));

        printf(
            "%-5zu %-10s %-8s 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
            i, elf_ph_type_to_string(ph->p_type), fbuf,
            ph->p_offset, ph->p_vaddr, ph->p_paddr, ph->p_filesz, ph->p_memsz
        );
    }
}

static void print_elf_section_headers64(const Elf64_Ehdr* ehdr, char* src, size_t size) {
    printf("\n===== Section Headers =====\n");

    if ((size_t)(ehdr->e_shoff) >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    printf("%-4s %-20s %-12s %-16s %-16s %-16s %-6s %-6s %-6s %-8s\n", "Idx", "Name", "Type", "Addr", "Offset", "Size", "ES", "Flg", "Lk", "Inf");
    printf("-----------------------------------------------------------------------------------------------------------------------------------\n");

    const Elf64_Shdr* sh_table = (const Elf64_Shdr*)(src + ehdr->e_shoff);
    const char* shstrtab = src + sh_table[ehdr->e_shstrndx].sh_offset;

    for (size_t i = 0; i < ehdr->e_shnum; i++) {
        const Elf64_Shdr* sh = &sh_table[i];
        printf(
            "%-4zu %-20s %-12s 0x%014lx 0x%014lx 0x%014lx %-6lu %-6lx %-6u %-8u\n",
            i, shstrtab + sh->sh_name, elf_sh_type_to_string(sh->sh_type),
            sh->sh_addr, sh->sh_offset, sh->sh_size,
            sh->sh_entsize, sh->sh_flags, sh->sh_link, sh->sh_info
        );
    }
}

static void print_elf_section_headers32(const Elf32_Ehdr* ehdr, char* src, size_t size) {
    printf("\n===== Section Headers =====\n");

    if ((size_t)(ehdr->e_shoff) >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    printf("%-4s %-20s %-12s %-8s %-8s %-8s %-6s %-6s %-6s %-8s\n", "Idx", "Name", "Type", "Addr", "Offset", "Size", "ES", "Flg", "Lk", "Inf");
    printf("--------------------------------------------------------------------------------------------------------------------------------\n");

    const Elf32_Shdr* sh_table = (const Elf32_Shdr*)(src + ehdr->e_shoff);
    const char* shstrtab = src + sh_table[ehdr->e_shstrndx].sh_offset;

    for (size_t i = 0; i < ehdr->e_shnum; i++) {
        const Elf32_Shdr* sh = &sh_table[i];
        printf(
            "%-4zu %-20s %-12s 0x%08x 0x%08x 0x%08x %-6u %-6x %-6u %-8u\n",
            i, shstrtab + sh->sh_name, elf_sh_type_to_string(sh->sh_type),
            sh->sh_addr, sh->sh_offset, sh->sh_size,
            sh->sh_entsize, sh->sh_flags, sh->sh_link, sh->sh_info
        );
    }
}

static void print_all_elf64(const Elf64_Ehdr* ehdr, char* src, size_t size) {
    print_elf_header64(ehdr);
    print_elf_program_headers64(ehdr, src, size);
    print_elf_section_headers64(ehdr, src, size);
}

static void print_all_elf32(const Elf32_Ehdr* ehdr, char* src, size_t size) {
    print_elf_header32(ehdr);
    print_elf_program_headers32(ehdr, src, size);
    print_elf_section_headers32(ehdr, src, size);
}

static void print_dos_header(const DOS_Hdr* dos) {
    printf("===== DOS Header =====\n");
    printf("Magic:            %-2s\n", (char*)&dos->e_magic);
    printf("PE Header Offset: 0x%08x\n", dos->e_lfanew);
}

static void print_pe_header(const DOS_Hdr* doshdr, char* src, size_t size) {
    if ((size_t)(doshdr->e_lfanew) >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    const PE_Hdr* pe_hdr = (const PE_Hdr*)(src + doshdr->e_lfanew);
    printf("\n===== PE File Header =====\n");
    printf("%-20s %-20s %-20s\n", "Machine", "Sections", "TimeDateStamp");
    time_t ts = pe_hdr->time_date_stamp;
    struct tm* tm_info = localtime(&ts);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    printf(
        "%-20s\t%u\t%s\n", pecoff_machine_to_string(pe_hdr->machine), 
        pe_hdr->no_of_sections, 
        buf
    );
    
    printf(
        "\nPointerToSymbolTable: 0x%08x\nNumberOfSymbols: %u\nSizeOfOptionalHeader: %u\n",
        pe_hdr->ptr_to_symtab, pe_hdr->no_of_symbols, pe_hdr->size_of_optional_header
    );
    
    printf("Characteristics: %s\n", pecoff_characteristics_to_string(pe_hdr->characteristics));
}

static void print_pe_optional_header32(const DOS_Hdr* doshdr, char* src, size_t size) {
    if ((size_t)(doshdr->e_lfanew) >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    const Optional_Hdr_32* oh = (const Optional_Hdr_32*)(src + doshdr->e_lfanew + sizeof(PE_Hdr));
    printf("\n===== Optional Header (PE32) =====\n");
    printf("%-20s %-16s %-16s %-16s %-16s\n", "Magic", "EntryPoint", "ImageBase", "SectionAlign", "FileAlign");
    printf(
        "%-20s\t0x%08x\t0x%08x\t0x%08x\t0x%08x\n",
        pe_magic_to_string(oh->magic), oh->addr_of_entry, oh->image_base, oh->section_alignment, oh->file_alignment
    );
    printf("\nSizeOfImage: 0x%08x\nSizeOfHeaders: 0x%08x\nSubsystem: 1x%04x\n", oh->size_of_image, oh->size_of_headers, oh->subsys);
}

static void print_pe_optional_header(const DOS_Hdr* doshdr, char* src, size_t size) {
    if ((size_t)(doshdr->e_lfanew) >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    const Optional_Hdr_64* oh = (const Optional_Hdr_64*)(src + doshdr->e_lfanew + sizeof(PE_Hdr));
    if (oh->magic == Optional_Hdr_32_Magic) {
        print_pe_optional_header32(doshdr, src, size);
        return;
    }

    printf("\n===== Optional Header (PE32+) =====\n");
    printf("%-20s %-16s %-16s %-16s %-16s\n", "Magic", "EntryPoint", "ImageBase", "SectionAlign", "FileAlign");
    printf(
        "%-20s\t0x%08x\t0x%08lx\t0x%08x\t0x%08x\n",
        pe_magic_to_string(oh->magic), oh->addr_of_entry, (unsigned long int)oh->image_base, oh->section_alignment, oh->file_alignment
    );
    printf("\nSizeOfImage: 0x%08x\nSizeOfHeaders: 0x%08x\nSubsystem: 0x%04x\n", oh->size_of_image, oh->size_of_headers, oh->subsys);
}

static void print_pe_sections(const DOS_Hdr* doshdr, char* src, size_t size) {
    if ((size_t)(doshdr->e_lfanew) >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    const PE_Hdr* pe_hdr = (const PE_Hdr*)(src + doshdr->e_lfanew);
    Section_Hdr* sections = (Section_Hdr*)(src + doshdr->e_lfanew + sizeof(PE_Hdr) + pe_hdr->size_of_optional_header);
    int count = pe_hdr->no_of_sections;

    printf("\n===== Section Headers =====\n");
    printf("%-8s %-10s %-10s %-10s %-10s %-10s %-10s %-10s\n", "Name", "VirtSize", "VirtAddr", "SizeRaw", "PtrRaw", "RelocPtr", "LinenumPtr", "Flags");
    printf("----------------------------------------------------------------------------------------------------------------------------------------\n");

    for(int i = 0; i < count; i++) {
        const Section_Hdr* sh = &sections[i];
        printf(
            "%-8s 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
            (char*)&sh->name, sh->virtual_size, sh->virtual_addr,
            sh->size_of_raw_data, sh->ptr_to_raw_data,
            sh->ptr_to_relocs, sh->ptr_to_line_no, sh->characteristics
        );
    }
}

static void print_all_pe(const DOS_Hdr* doshdr, char* src, size_t size) {
    print_dos_header(doshdr);
    print_pe_header(doshdr, src, size);
    print_pe_optional_header(doshdr, src, size);
    print_pe_sections(doshdr, src, size);
}

bool r_info_basic(char* src, size_t size) {
    if (is_pe(src, size)) {
        char error_info[128];
        if (!validate_pe(src, size, error_info)) {
            fprintf(stderr, "Error: %s\n", error_info);
            return false;
        }

        const DOS_Hdr* doshdr = (const DOS_Hdr*)(src);
        print_dos_header(doshdr);
        print_pe_header(doshdr, src, size);
        print_pe_optional_header(doshdr, src, size);
    } else {
        // ELF
        char error_info[128];
        if (!validate_elf(src, size, error_info)) {
            fprintf(stderr, "Error: %s\n", error_info);
            return false;
        }

        const Elf64_Ehdr* ehdr = (const Elf64_Ehdr*)(src);
        if (ehdr->e_ident[EI_CLASS] == ELFCLASS32) {
            const Elf32_Ehdr* ehdr32 = (const Elf32_Ehdr*)(src);
            print_elf_header32(ehdr32);
        } else {
            print_elf_header64(ehdr);
        }
    }

    return true;
}

bool r_info_sections(char* src, size_t size) {
    if (is_pe(src, size)) {
        char error_info[128];
        if (!validate_pe(src, size, error_info)) {
            fprintf(stderr, "Error: %s\n", error_info);
            return false;
        }

        const DOS_Hdr* doshdr = (const DOS_Hdr*)(src);
        print_pe_sections(doshdr, src, size);
    } else {
        // ELF
        char error_info[128];
        if (!validate_elf(src, size, error_info)) {
            fprintf(stderr, "Error: %s\n", error_info);
            return false;
        }

        const Elf64_Ehdr* ehdr = (const Elf64_Ehdr*)(src);
        if (ehdr->e_ident[EI_CLASS] == ELFCLASS32) {
            const Elf32_Ehdr* ehdr32 = (const Elf32_Ehdr*)(src);
            print_elf_section_headers32(ehdr32, src, size);
        } else {
            print_elf_section_headers64(ehdr, src, size);
        }
    }

    return true;
}

bool r_info_program_headers(char* src, size_t size) {
    if (is_pe(src, size)) {
        fprintf(stderr, "PE files don't have program headers!\n");
        return false;
    } else {
        // ELF
        char error_info[128];
        if (!validate_elf(src, size, error_info)) {
            fprintf(stderr, "Error: %s\n", error_info);
            return false;
        }

        const Elf64_Ehdr* ehdr = (const Elf64_Ehdr*)(src);
        if (ehdr->e_ident[EI_CLASS] == ELFCLASS32) {
            const Elf32_Ehdr* ehdr32 = (const Elf32_Ehdr*)(src);
            print_elf_program_headers32(ehdr32, src, size);
        } else {
            print_elf_program_headers64(ehdr, src, size);
        }
    }

    return true;
}

bool r_info_all(char* src, size_t size) {
    if (is_pe(src, size)) {
        char error_info[128];
        if (!validate_pe(src, size, error_info)) {
            fprintf(stderr, "Error: %s\n", error_info);
            return false;
        }

        const DOS_Hdr* doshdr = (const DOS_Hdr*)(src);
        print_all_pe(doshdr, src, size);
    } else {
        // ELF
        char error_info[128];
        if (!validate_elf(src, size, error_info)) {
            fprintf(stderr, "Error: %s\n", error_info);
            return false;
        }

        const Elf64_Ehdr* ehdr = (const Elf64_Ehdr*)(src);
        if (ehdr->e_ident[EI_CLASS] == ELFCLASS32) {
            const Elf32_Ehdr* ehdr32 = (const Elf32_Ehdr*)(src);
            print_all_elf32(ehdr32, src, size);
        } else {
            print_all_elf64(ehdr, src, size);
        }
    }

    return true;
}

Architecture r_get_arch(char* src, size_t size) {
    if (is_pe(src, size)) {
        char error_info[128];
        if (!validate_pe(src, size, error_info)) {
            fprintf(stderr, "Error: %s\n", error_info);
            return UnknownArch;
        }

        const DOS_Hdr* doshdr = (const DOS_Hdr*)(src);
        const PE_Hdr* pehdr = (const PE_Hdr*)(src + doshdr->e_lfanew);
        switch (pehdr->machine) {
            case IMAGE_FILE_MACHINE_I386: return x86;
            case IMAGE_FILE_MACHINE_AMD64: return x64;
            default: return UnknownArch;
        }
    } else {
        // ELF
        char error_info[128];
        if (!validate_elf(src, size, error_info)) {
            fprintf(stderr, "Error: %s\n", error_info);
            return UnknownArch;
        }

        const Elf64_Ehdr* ehdr = (const Elf64_Ehdr*)(src);
        if (ehdr->e_ident[EI_CLASS] == ELFCLASS32) {
            const Elf32_Ehdr* ehdr32 = (const Elf32_Ehdr*)(src);
            switch (ehdr32->e_machine) {
                case EM_PVCPU: return PVCpu;
                case EM_PVCPUC: return PVCpuC;
                case EM_X86_64: return x64;
                case EM_386: return x86;
                default: return UnknownArch;
            }
        } else {
            switch (ehdr->e_machine) {
                case EM_PVCPU: return PVCpu;
                case EM_PVCPUC: return PVCpuC;
                case EM_X86_64: return x64;
                case EM_386: return x86;
                default: return UnknownArch;
            }
        }
    }

    return UnknownArch;
}

size_t r_get_codeoff(char* src, size_t size, size_t* code_size, size_t* svaddr) {
    if (is_pe(src, size)) {
        char error_info[128];
        if (!validate_pe(src, size, error_info)) {
            fprintf(stderr, "Error: %s\n", error_info);
            *code_size = 0;
            *svaddr = 0;
            return 0;
        }

        const DOS_Hdr* doshdr = (const DOS_Hdr*)(src);
        const PE_Hdr* pehdr = (const PE_Hdr*)(src + doshdr->e_lfanew);
        Section_Hdr* sections = (Section_Hdr*)(src + doshdr->e_lfanew + sizeof(PE_Hdr) + pehdr->size_of_optional_header);

        for (size_t i = 0; i < pehdr->no_of_sections; i++) {
            Section_Hdr* section = (Section_Hdr*)&sections[i];
            if (section->characteristics & IMAGE_SCN_CNT_CODE) {
                *code_size = (size_t)section->size_of_raw_data;
                *svaddr = (size_t)section->virtual_addr;
                return (size_t)section->ptr_to_raw_data;
            }
        }
    } else {
        // ELF
        char error_info[128];
        if (!validate_elf(src, size, error_info)) {
            fprintf(stderr, "Error: %s\n", error_info);
            *code_size = 0;
            *svaddr = 0;
            return 0;
        }

        const Elf64_Ehdr* ehdr = (const Elf64_Ehdr*)(src);
        if (ehdr->e_ident[EI_CLASS] == ELFCLASS32) {
            const Elf32_Ehdr* ehdr32 = (const Elf32_Ehdr*)(src);
            Elf32_Shdr* sections = (Elf32_Shdr*)(src + ehdr32->e_shoff);
            for (size_t i = 0; i < (size_t)ehdr32->e_shnum; i++) {
                Elf32_Shdr* section = (Elf32_Shdr*)&sections[i];
                if (section->sh_flags == SHF_EXECINSTR && section->sh_type == SHT_PROGBITS) {
                    *code_size = (size_t)section->sh_size;
                    *svaddr = (size_t)section->sh_addr;
                    return (size_t)section->sh_offset;
                }
            }
        } else {
            Elf64_Shdr* sections = (Elf64_Shdr*)(src + ehdr->e_shoff);
            for (size_t i = 0; i < (size_t)ehdr->e_shnum; i++) {
                Elf64_Shdr* section = (Elf64_Shdr*)&sections[i];
                if (section->sh_flags & SHF_EXECINSTR && section->sh_type == SHT_PROGBITS) {
                    *code_size = (size_t)section->sh_size;
                    *svaddr = (size_t)section->sh_addr;
                    return (size_t)section->sh_offset;
                }
            }
        }
    }

    fprintf(stderr, "Error: Possibly a file without any code OR unsupported file?\n");
    *code_size = 0;
    *svaddr = 0;
    return 0;
}
