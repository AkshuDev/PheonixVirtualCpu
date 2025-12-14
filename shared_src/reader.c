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

static void print_elf_symbols64(const Elf64_Ehdr* ehdr, char* src, size_t size) {
    printf("\n===== Symbols =====\n");

    if ((size_t)(ehdr->e_shoff) >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    const Elf64_Shdr* sh_table = (const Elf64_Shdr*)(src + ehdr->e_shoff);
    char* strtab = NULL;
    char* shstrtab = (char*)(src + sh_table[ehdr->e_shstrndx].sh_offset);
    Elf64_Sym* symtab = NULL;
    size_t symcount = 0;

    for (size_t i = 0; i < ehdr->e_shnum; i++) {
        Elf64_Shdr* sec = (Elf64_Shdr*)&sh_table[i];
        if (sec->sh_type == SHT_STRTAB && i != ehdr->e_shstrndx) {
            strtab = (char*)(src + sec->sh_offset);
        } else if (sec->sh_type == SHT_SYMTAB) {
            symtab = (Elf64_Sym*)(src + sec->sh_offset);
            symcount = sec->sh_size / sec->sh_entsize;
        }
    }

    if (!symtab || !strtab) {
        printf("No symbol table found.\n");
        return;
    }

    printf("%-4s %-24s %-8s %-8s %-10s %-6s %-10s\n", "Idx", "Name", "Type", "Bind", "Value", "Size", "Section");
    printf("--------------------------------------------------------------------------------------------------\n");

    for (size_t i = 0; i < symcount; i++) {
        Elf64_Sym* sym = (Elf64_Sym*)&symtab[i];
        const char* type = elf_sym_type_to_string((uint32_t)ELF64_ST_TYPE(sym->st_info));
        const char* bind = elf_sym_bind_to_string((uint32_t)ELF64_ST_BIND(sym->st_info));
        char* name = strtab != NULL ? (char*)(strtab + sym->st_name) : "INVALID";
        char* secname;

        if (sym->st_shndx == SHN_UNDEF)
            secname = "UND";
        else if (sym->st_shndx == SHN_ABS)
            secname = "ABS";
        else
            secname = shstrtab + sh_table[sym->st_shndx].sh_name;

        printf("%-4zu %-24s %-8s %-8s 0x%08lx %-6lu %-10s\n", i, name, type, bind, sym->st_value, sym->st_size, secname);
    }
}

static void print_elf_relocations64(const Elf64_Ehdr* ehdr, char* src, size_t size) {
    printf("\n===== Relocations =====\n");

    if ((size_t)ehdr->e_shoff >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    const Elf64_Shdr* sh_table = (const Elf64_Shdr*)(src + ehdr->e_shoff);
    const char* shstrtab = src + sh_table[ehdr->e_shstrndx].sh_offset;

    for (size_t i = 0; i < ehdr->e_shnum; i++) {
        const Elf64_Shdr* sh = &sh_table[i];

        if (sh->sh_type != SHT_REL && sh->sh_type != SHT_RELA)
            continue;

        const char* secname = shstrtab + sh->sh_name;
        size_t count = sh->sh_size / sh->sh_entsize;

        Elf64_Sym* symtab = (Elf64_Sym*)(src + sh_table[sh->sh_link].sh_offset);
        const char* strtab = src + sh_table[sh_table[sh->sh_link].sh_link].sh_offset;

        printf("\n-- Relocation section '%s' (%zu entries) --\n", secname, count);
        printf("%-4s %-10s %-10s %-16s %-10s\n", "Idx", "Offset", "Type", "Symbol", "Addend");
        printf("---------------------------------------------------------------------------\n");

        for (size_t j = 0; j < count; j++) {
            if (sh->sh_type == SHT_REL) {
                Elf64_Rel rel;
                memcpy(&rel, src + sh->sh_offset + j * sizeof(Elf64_Rel), sizeof(Elf64_Rel));
                uint32_t sym_idx = ELF64_R_SYM(rel.r_info);
                uint32_t type = ELF64_R_TYPE(rel.r_info);

                const char* symname = sym_idx ? strtab + symtab[sym_idx].st_name : "<none>";

                printf("%-4zu 0x%08lx %-10u %-16s %-10s\n", j, rel.r_offset, type, symname, "—");
            } else {
                Elf64_Rela rela;
                memcpy(&rela, src + sh->sh_offset + j * sizeof(Elf64_Rela), sizeof(Elf64_Rela));
                uint32_t sym_idx = ELF64_R_SYM(rela.r_info);
                uint32_t type = ELF64_R_TYPE(rela.r_info);

                const char* symname = sym_idx ? strtab + symtab[sym_idx].st_name : "<none>";

                printf("%-4zu 0x%08lx %-10u %-16s %-8ld\n", j, rela.r_offset, type, symname, rela.r_addend);
            }
        }
    }
}

static void print_elf_symbols32(const Elf32_Ehdr* ehdr, char* src, size_t size) {
    printf("\n===== Symbols =====\n");

    if ((size_t)(ehdr->e_shoff) >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    const Elf32_Shdr* sh_table = (const Elf32_Shdr*)(src + ehdr->e_shoff);
    char* strtab = NULL;
    char* shstrtab = (char*)(src + sh_table[ehdr->e_shstrndx].sh_offset);
    Elf32_Sym* symtab = NULL;
    size_t symcount = 0;

    for (size_t i = 0; i < ehdr->e_shnum; i++) {
        Elf32_Shdr* sec = (Elf32_Shdr*)&sh_table[i];
        if (sec->sh_type == SHT_STRTAB && i != ehdr->e_shstrndx) {
            strtab = (char*)(src + sec->sh_offset);
        } else if (sec->sh_type == SHT_SYMTAB) {
            symtab = (Elf32_Sym*)(src + sec->sh_offset);
            symcount = sec->sh_size / sec->sh_entsize;
        }
    }

    if (!symtab || !strtab) {
        printf("No symbol table found.\n");
        return;
    }

    printf("%-4s %-24s %-8s %-8s %-10s %-6s %-10s\n", "Idx", "Name", "Type", "Bind", "Value", "Size", "Section");
    printf("--------------------------------------------------------------------------------------------------\n");

    for (size_t i = 0; i < symcount; i++) {
        Elf32_Sym* sym = (Elf32_Sym*)&symtab[i];
        const char* type = elf_sym_type_to_string((uint32_t)ELF32_ST_TYPE(sym->st_info));
        const char* bind = elf_sym_bind_to_string((uint32_t)ELF32_ST_BIND(sym->st_info));
        char* name = strtab != NULL ? (char*)(strtab + sym->st_name) : "INVALID";
        char* secname;

        if (sym->st_shndx == SHN_UNDEF)
            secname = "UND";
        else if (sym->st_shndx == SHN_ABS)
            secname = "ABS";
        else
            secname = shstrtab + sh_table[sym->st_shndx].sh_name;

        printf("%-4zu %-24s %-8s %-8s 0x%08x %-6u %-10s\n", i, name, type, bind, sym->st_value, sym->st_size, secname);
    }
}

static void print_elf_relocations32(const Elf32_Ehdr* ehdr, char* src, size_t size) {
    printf("\n===== Relocations =====\n");

    if ((size_t)ehdr->e_shoff >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    const Elf32_Shdr* sh_table = (const Elf32_Shdr*)(src + ehdr->e_shoff);
    const char* shstrtab = src + sh_table[ehdr->e_shstrndx].sh_offset;

    for (size_t i = 0; i < ehdr->e_shnum; i++) {
        const Elf32_Shdr* sh = &sh_table[i];

        if (sh->sh_type != SHT_REL && sh->sh_type != SHT_RELA)
            continue;

        const char* secname = shstrtab + sh->sh_name;
        size_t count = sh->sh_size / sh->sh_entsize;

        Elf32_Sym* symtab = (Elf32_Sym*)(src + sh_table[sh->sh_link].sh_offset);
        const char* strtab = src + sh_table[sh_table[sh->sh_link].sh_link].sh_offset;

        printf("\n-- Relocation section '%s' (%zu entries) --\n", secname, count);
        printf("%-4s %-10s %-10s %-16s %-10s\n", "Idx", "Offset", "Type", "Symbol", "Addend");
        printf("---------------------------------------------------------------------------\n");

        for (size_t j = 0; j < count; j++) {
            if (sh->sh_type == SHT_REL) {
                Elf32_Rel rel;
                memcpy(&rel, src + sh->sh_offset + j * sizeof(Elf32_Rel), sizeof(Elf32_Rel));
                uint32_t sym_idx = ELF32_R_SYM(rel.r_info);
                uint32_t type = ELF32_R_TYPE(rel.r_info);

                const char* symname = sym_idx ? strtab + symtab[sym_idx].st_name : "<none>";

                printf("%-4zu 0x%08x %-10u %-16s %-10s\n", j, rel.r_offset, type, symname, "—");
            } else {
                Elf32_Rela rela;
                memcpy(&rela, src + sh->sh_offset + j * sizeof(Elf32_Rela), sizeof(Elf32_Rela));
                uint32_t sym_idx = ELF32_R_SYM(rela.r_info);
                uint32_t type = ELF32_R_TYPE(rela.r_info);

                const char* symname = sym_idx ? strtab + symtab[sym_idx].st_name : "<none>";

                printf("%-4zu 0x%08x %-10u %-16s 0x%-8d\n", j, rela.r_offset, type, symname, rela.r_addend);
            }
        }
    }
}

static void print_all_elf64(const Elf64_Ehdr* ehdr, char* src, size_t size) {
    print_elf_header64(ehdr);
    print_elf_program_headers64(ehdr, src, size);
    print_elf_section_headers64(ehdr, src, size);
    print_elf_symbols64(ehdr, src, size);
    print_elf_relocations64(ehdr, src, size);
}

static void print_all_elf32(const Elf32_Ehdr* ehdr, char* src, size_t size) {
    print_elf_header32(ehdr);
    print_elf_program_headers32(ehdr, src, size);
    print_elf_section_headers32(ehdr, src, size);
    print_elf_symbols32(ehdr, src, size);
    print_elf_relocations32(ehdr, src, size);
}

static void print_dos_header(const DOS_Hdr* dos) {
    printf("===== DOS Header =====\n");
    printf("Magic:            %-2s\n", (char*)&dos->e_magic);
    printf("PE Header Offset: 0x%08x\n", dos->e_lfanew);
}

static void print_pe_header(const DOS_Hdr* doshdr, char* src, size_t size) {
    printf("\n===== PE File Header =====\n");

    if ((size_t)(doshdr->e_lfanew) >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    const PE_Hdr* pe_hdr = (const PE_Hdr*)(src + doshdr->e_lfanew);
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
    printf("\n===== Optional Header (PE32) =====\n");

    if ((size_t)(doshdr->e_lfanew) >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    const Optional_Hdr_32* oh = (const Optional_Hdr_32*)(src + doshdr->e_lfanew + sizeof(PE_Hdr));
    printf("%-20s %-16s %-16s %-16s %-16s\n", "Magic", "EntryPoint", "ImageBase", "SectionAlign", "FileAlign");
    printf(
        "%-20s\t0x%08x\t0x%08x\t0x%08x\t0x%08x\n",
        pe_magic_to_string(oh->magic), oh->addr_of_entry, oh->image_base, oh->section_alignment, oh->file_alignment
    );
    printf("\nSizeOfImage: 0x%08x\nSizeOfHeaders: 0x%08x\nSubsystem: 1x%04x\n", oh->size_of_image, oh->size_of_headers, oh->subsys);
}

static void print_pe_optional_header(const DOS_Hdr* doshdr, char* src, size_t size) {
    printf("\n===== Optional Header (PE32+) =====\n");

    if ((size_t)(doshdr->e_lfanew) >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    const Optional_Hdr_64* oh = (const Optional_Hdr_64*)(src + doshdr->e_lfanew + sizeof(PE_Hdr));
    if (oh->magic == Optional_Hdr_32_Magic) {
        print_pe_optional_header32(doshdr, src, size);
        return;
    }

    printf("%-20s %-16s %-16s %-16s %-16s\n", "Magic", "EntryPoint", "ImageBase", "SectionAlign", "FileAlign");
    printf(
        "%-20s\t0x%08x\t0x%08lx\t0x%08x\t0x%08x\n",
        pe_magic_to_string(oh->magic), oh->addr_of_entry, (unsigned long int)oh->image_base, oh->section_alignment, oh->file_alignment
    );
    printf("\nSizeOfImage: 0x%08x\nSizeOfHeaders: 0x%08x\nSubsystem: 0x%04x\n", oh->size_of_image, oh->size_of_headers, oh->subsys);
}

static void print_pe_sections(const DOS_Hdr* doshdr, char* src, size_t size) {
    printf("\n===== Section Headers =====\n");

    if ((size_t)(doshdr->e_lfanew) >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    const PE_Hdr* pe_hdr = (const PE_Hdr*)(src + doshdr->e_lfanew);
    Section_Hdr* sections = (Section_Hdr*)(src + doshdr->e_lfanew + sizeof(PE_Hdr) + pe_hdr->size_of_optional_header);
    int count = pe_hdr->no_of_sections;

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

static void print_pe_symbols(const DOS_Hdr* doshdr, char* src, size_t size) {
    printf("\n===== COFF Symbols =====\n");

    if ((size_t)doshdr->e_lfanew >= size) {
        printf("File truncated, maybe?\n");
        return;
    }

    const PE_Hdr* pe = (const PE_Hdr*)(src + doshdr->e_lfanew);

    if (pe->ptr_to_symtab == 0 || pe->no_of_symbols == 0) {
        printf("No symbol table present.\n");
        return;
    }

    const Section_Hdr* sectab = (const Section_Hdr*)((const char*)pe + sizeof(PE_Hdr) + pe->size_of_optional_header);

    const char* sym_base = src + pe->ptr_to_symtab;
    const char* strtab = sym_base + (pe->no_of_symbols * sizeof(Symbol_Entry));

    printf("%-4s %-24s %-10s %-10s %-8s\n", "Idx", "Name", "Value", "Section", "Type");
    printf("------------------------------------------------------------------------\n");

    for (size_t i = 0; i < pe->no_of_symbols; i++) {
        Symbol_Entry sym;
        memcpy(&sym, sym_base + i * sizeof(Symbol_Entry), sizeof(Symbol_Entry));

        const char* name;
        char namebuf[256];

        if (sym.name.zeros == 0) {
            uint32_t off = sym.name.offset;
            name = strtab + off;
        } else {
            memcpy(namebuf, (uint8_t*)&sym.name.short_name, 8);
            namebuf[8] = '\0';
            name = namebuf;
        }

        const char* secname = pecoff_section_index_to_string(sym.section_no);
        char secbuf[9];

        if (!secname && sym.section_no > 0 && sym.section_no <= pe->no_of_sections) {
            memcpy(secbuf, (uint8_t*)&sectab[sym.section_no - 1].name, 8);
            secbuf[8] = '\0';
            secname = secbuf;
        }
        if (!secname) secname = "INVALID";

        const char* type = (sym.type == 0x20) ? "FUNC" : (sym.type == 0x00) ? "NULL" : "OTHER";

        printf("%-4zu %-24s 0x%08x %-10s %-8s\n", i, name, sym.value, secname, type);

        i += sym.no_aux_symbols;
    }
}

static void print_all_pe(const DOS_Hdr* doshdr, char* src, size_t size) {
    print_dos_header(doshdr);
    print_pe_header(doshdr, src, size);
    print_pe_optional_header(doshdr, src, size);
    print_pe_sections(doshdr, src, size);
    print_pe_symbols(doshdr, src, size);
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

bool r_info_program(char* src, size_t size) {
    if (is_pe(src, size)) {
        const DOS_Hdr* doshdr = (const DOS_Hdr*)src;
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
            print_elf_program_headers32(ehdr32, src, size);
        } else {
            print_elf_program_headers64(ehdr, src, size);
        }
    }

    return true;
}

bool r_info_symbols(char* src, size_t size) {
    if (is_pe(src, size)) {
        const DOS_Hdr* doshdr = (const DOS_Hdr*)src;
        print_pe_symbols(doshdr, src, size);
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
            print_elf_symbols32(ehdr32, src, size);
        } else {
            print_elf_symbols64(ehdr, src, size);
        }
    }

    return true;
}

bool r_info_relocs(char* src, size_t size) {
    if (is_pe(src, size)) {
        const DOS_Hdr* doshdr = (const DOS_Hdr*)src;
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
            print_elf_relocations32(ehdr32, src, size);
        } else {
            print_elf_relocations64(ehdr, src, size);
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
            return Arch_Unknown;
        }

        const DOS_Hdr* doshdr = (const DOS_Hdr*)(src);
        const PE_Hdr* pehdr = (const PE_Hdr*)(src + doshdr->e_lfanew);
        switch (pehdr->machine) {
            case IMAGE_FILE_MACHINE_I386: return Arch_x86;
            case IMAGE_FILE_MACHINE_AMD64: return Arch_x64;
            default: return Arch_Unknown;
        }
    } else {
        // ELF
        char error_info[128];
        if (!validate_elf(src, size, error_info)) {
            fprintf(stderr, "Error: %s\n", error_info);
            return Arch_Unknown;
        }

        const Elf64_Ehdr* ehdr = (const Elf64_Ehdr*)(src);
        if (ehdr->e_ident[EI_CLASS] == ELFCLASS32) {
            const Elf32_Ehdr* ehdr32 = (const Elf32_Ehdr*)(src);
            switch (ehdr32->e_machine) {
                case EM_PVCPU: return Arch_PVCpu;
                case EM_PVCPUC: return Arch_PVCpuC;
                case EM_X86_64: return Arch_x64;
                case EM_386: return Arch_x86;
                default: return Arch_Unknown;
            }
        } else {
            switch (ehdr->e_machine) {
                case EM_PVCPU: return Arch_PVCpu;
                case EM_PVCPUC: return Arch_PVCpuC;
                case EM_X86_64: return Arch_x64;
                case EM_386: return Arch_x86;
                default: return Arch_Unknown;
            }
        }
    }

    return Arch_Unknown;
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
