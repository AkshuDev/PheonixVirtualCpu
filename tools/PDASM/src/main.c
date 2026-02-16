// Author: Pheonix Studios/AkshuDev

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <reader.h>
#include <extra.h>
#include <decoder_pvcpu.h>
#include <decoder_x86.h>

#define SArchs_Print "x86, x64/x86_64, pvcpu, pvcpuc/pvcpu_c"
#define SArchsEX_Print "x86, x64/x86_64, pvcpu (Pheonix Virtual Cpu), pvcpuc/pvcpu_c (Pheonix Virtual Cpu - Compressed)"
#define PDASM_USAGE "Usage: pdasm <command> <OPTIONAL:input> <OPTIONAL:[OPTIONS]>\nExample: disassemble myfile\n"

static uint8_t* read_file(const char* filename, size_t* out_size) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("Error opening file");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t* buf = malloc(size);
    if (!buf) {
        perror("Memory allocation failed");
        fclose(f);
        return NULL;
    }

    if (fread(buf, 1, size, f) != size) {
        perror("Error reading file");
        free(buf);
        fclose(f);
        return NULL;
    }

    fclose(f);
    *out_size = size;
    return buf;
}

static void print_help() {
    printf(PDASM_USAGE "\n\nCommands:\n");
    
    printf("info <file>        -  Shows information about specified executable file. Subcommands:\n");
    printf("\t--all (default)    -  Shows all available information\n");
    printf("\t--sections         -  Shows only section information\n");
    printf("\t--symbols          -  Shows symbol information\n");
    printf("\t--relocs           -  Shows relocation information\n");
    printf("\t--program          -  Shows program information, such as loading\n");
    printf("\t--basic            -  Shows basic information\n");
    
    printf("disassemble <file> -  Disassembles the file, using the executable information. Subcommands:\n");
    printf("\t--binary              -  The file is a binary, when using this, please also specify architecture\n");
    printf("\t--arch <architecture> - Incase the file is a binary, use this to specify the architecture. Available architectures:\n\t\t" SArchsEX_Print "\n");

    printf("\nhelp             - Display this message\n");
}

typedef struct {
    bool help;
    bool info;
    bool disassemble;

    bool error;
    
    char* info_input;
    bool info_all;
    bool info_sections;
    bool info_symbols;
    bool info_relocs;
    bool info_program;
    bool info_basic;
    
    char* disassemble_input;
    bool disassemble_binary;
    Architecture disassemble_arch;
} Args_t;

static Architecture str_to_arch(char* s) {
    if (strcmp(s, "x86") == 0 || strcmp(s, "i386") == 0) {
        return Arch_x86;
    } else if (strcmp(s, "x64") == 0 || strcmp(s, "x86_64") == 0 || strcmp(s, "amd64") == 0) {
        return Arch_x64;
    } else if (strcmp(s, "pvcpu") == 0) {
        return Arch_PVCpu;
    } else if (strcmp(s, "pvcpuc") == 0 || strcmp(s, "pvcpu-c") == 0 || strcmp(s, "c-pvcpu") == 0) {
        return Arch_PVCpuC;
    } else {
        return Arch_Unknown;
    }
}

static const char* arch_to_str(Architecture a) {
    switch (a) {
        case Arch_x86: return "x86";
        case Arch_x64: return "x64";
        case Arch_PVCpu: return "PVCpu";
        case Arch_PVCpuC: return "PVCpu-C";
        default: return "Unknown";
    }
}

static void parse_args(Args_t* args, int argc, char** argv) {
    memset(args, 0, sizeof(Args_t));
    
    char* cmd = argv[1];
    if (strcmp(cmd, "help") == 0) {
        args->help = true;
        return;
    } else if (strcmp(cmd, "info") == 0) {
        args->info = true;
        if (argc < 3) {
            printf(CB_RED "Command '%s' requires an argument <input>!\n\t" CB_CYAN "Tip: Try using the 'help' command!\n" CS_RESET, cmd);
            args->error = true;
            return;
        }
        args->info_input = argv[2];
        for (int i = 3; i < argc; i++) {
            char* opt = argv[i];
            if (strcmp(opt, "--all") == 0) {
                args->info_all = true;
            } else if (strcmp(opt, "--all") == 0) {
                args->info_all = true;
            } else if (strcmp(opt, "--sections") == 0) {
                args->info_sections = true;
            } else if (strcmp(opt, "--symbols") == 0) {
                args->info_symbols = true;
            } else if (strcmp(opt, "--relocs") == 0) {
                args->info_relocs = true;
            } else if (strcmp(opt, "--program") == 0) {
                args->info_program = true;
            } else if (strcmp(opt, "--basic") == 0) {
                args->info_basic = true;
            } else {
                printf(CB_RED "Invalid Option for '%s' command: %s\n\t" CB_CYAN "Tip: Try 'help' command!\n" CS_RESET, cmd);
                args->error = true;
                return;
            }
        }
    } else if (strcmp(cmd, "disassemble") == 0 || strcmp(cmd, "dasm") == 0) {
        args->disassemble = true;
        if (argc < 3) {
            printf(CB_RED "Command '%s' requires an argument <input>!\n\t" CB_CYAN "Tip: Try using the 'help' command!\n" CS_RESET, cmd);
            args->error = true;
            return;
        }
        args->disassemble_input = argv[2];
        for (int i = 3; i < argc; i++) {
            char* opt = argv[i];
            if (strcmp(opt, "--binary") == 0) {
                args->disassemble_binary = true;
            } else if (strcmp(opt, "--arch") == 0) {
                if (i + 1 >= argc) {
                    printf(CB_RED "Required argument <architecture> for option '--arch' of command '%s': %s\n\t" CB_CYAN "Tip: Try 'help' command!\n" CS_RESET, cmd, opt);
                    args->error = true;
                    return;
                }
                args->disassemble_arch = str_to_arch(argv[++i]);
            } else {
                printf(CB_RED "Invalid Option for '%s' command: %s\n\t" CB_CYAN "Tip: Try 'help' command!\n" CS_RESET, cmd, opt);
                args->error = true;
                return;
            }
        }
    } else {
        printf(CB_RED "Unknown command '%s'!\n\t" CB_CYAN "Tip: Try 'help' command!\n" CS_RESET, cmd);
        args->error = true;
        return;
    }
}

static void decode(char* src, size_t size, Architecture arch, size_t svaddr, size_t soff, size_t code_size) {
    if (soff > size) {
        printf(CB_RED "File truncated, maybe?\n" CS_RESET);
        return;
    }

    size_t off = soff;
    size_t last_off = soff;
    size_t vaddr = svaddr;
    char out[256];

    switch (arch) {
        case Arch_x86:
        case Arch_x64:
            while (off < (code_size + soff)) {
                decode_x86((uint8_t*)src, &off, vaddr, out, sizeof(out));
                printf(C_CYAN "0x%08lx: %s\n" CS_RESET, vaddr, out);

                vaddr += (off - last_off);
                last_off = off;
                out[0] = '\0';
            }
            break;
        case Arch_PVCpu:
            while (off < (code_size + soff)) {
                decode_pvcpu((uint8_t*)src, &off, vaddr, out, sizeof(out));
                printf(C_CYAN "0x%08lx: %s\n" CS_RESET, vaddr, out);

                vaddr += (off - last_off);
                last_off = off;
                out[0] = '\0';
            }
            break;
        default:
            printf(CB_RED "Unsupported/Unknown Architecture!\n" CS_RESET);
            break;
    }
}

int main(int argc, char** argv) {
    setvbuf(stdout, NULL, _IONBF, 0); // Disable buffering for stdout
    if (argc < 2) {
        fprintf(stderr, CB_RED PDASM_USAGE CS_RESET);
        return 1;
    }

    Args_t args = {0};
    parse_args(&args, argc, argv);
    if (args.error) {
        return 2;
    }

    if (args.info) {
        size_t size = 0;
        char* src = (char*)read_file(args.info_input, &size);
        if (size == 0 || src == NULL) {
            fprintf(stderr, CB_RED "Error: Could not read file properly!\n" CS_RESET);
            return 4;
        }
        
        if (args.info_basic) r_info_basic(src, size);
        if (args.info_program) r_info_program(src, size);
        if (args.info_sections) r_info_sections(src, size);
        if (args.info_symbols) r_info_symbols(src, size);
        if (args.info_relocs) r_info_relocs(src, size);
        if (args.info_all) r_info_all(src, size);

        free(src);
    } else if (args.disassemble) {
        if (!args.disassemble_binary && args.disassemble_arch != Arch_Unknown) {
            printf(CB_YELLOW "Warning: Can only specify architecture when using binary files! Defaulting to file specified architecture\n" CS_RESET);
            args.disassemble_arch = Arch_Unknown;
        }

        size_t size = 0;
        char* src = (char*)read_file(args.disassemble_input, &size);
        if (size == 0 || src == NULL) {
            fprintf(stderr, CB_RED "Error: Could not read file properly!\n" CS_RESET);
            return 4;
        }
        
        if (!args.disassemble_binary) {
            r_info_basic(src, size);
            Architecture arch = r_get_arch(src, size);
            size_t svaddr = 0;
            size_t code_size = 0;
            size_t text_off = r_get_codeoff(src, size, &code_size, &svaddr);

            decode(src, size, arch, svaddr, text_off, code_size);
        } else {
            if (args.disassemble_arch == Arch_Unknown) {
                fprintf(stderr, CB_RED "Error: Please specify architecture when using binary files!\n" CS_RESET);
                free(src);
                return 5;
            }
            decode(src, size, args.disassemble_arch, 0, 0, size);
        }

        free(src);
    } else if (args.help) {
        print_help();
    }

    return 0;
}
