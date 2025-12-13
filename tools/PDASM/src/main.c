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


const char* SEPS = ":,-";

// Split arguments using ANY of the separators
static int split_args(char* input, char* out[], int max_parts) {
    int count = 0;

    char* tok = strtok(input, SEPS);
    while (tok && count < max_parts) {
        out[count++] = tok;
        tok = strtok(NULL, SEPS);
    }
    return count;
}

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
        fprintf(stderr, CB_RED "Usage: pdasm [func]<sep>[value]<sep>[args]\nExample: disassemble:myfile:arg1,arg_with_value-hi\n\tSeperators: ':', ',', '-'\n" CS_RESET);
        return 1;
    }

    // Copy to buffer
    char buf[1024];
    strncpy(buf, argv[1], sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';

    char* parts[32];
    int count = split_args(buf, parts, 32);

    if (count < 1) {
        fprintf(stderr, CB_RED "Invalid input.\n" CS_RESET);
        return 2;
    }

    const char* func = parts[0];
    const char* value = (count > 1 ? parts[1] : NULL);
    if (!value) {
        fprintf(stderr, CB_RED "Please pass value to function [%s]\n" CS_RESET, func);
        return 3;
    }

    if (strcmp(func, "info") == 0) {
        size_t size = 0;
        char* src = (char*)read_file(value, &size);
        if (size == 0 || src == NULL) {
            fprintf(stderr, CB_RED "Error: Could not read file properly!\n" CS_RESET);
            return 4;
        }
        r_info_all(src, size);

        free(src);
    } else if (strcmp(func, "disassemble") == 0 || strcmp(func, "dasm") == 0) {
        bool binary;
        Architecture arch = Arch_Unknown;

        if (count > 2) {
            for (int i = 2; i < count; i++) {
                char* arg = parts[i];
                if (strcmp(arg, "binary") == 0) {
                    binary = true;
                } else if (strcmp(arg, "arch") == 0) {
                    if (i + 1 < count) {
                        i++;
                        char* a = parts[i];

                        if (strcmp(a, "x86") == 0)
                            arch = Arch_x86;
                        else if (strcmp(a, "x64") == 0 || strcmp(a, "x86_64") == 0)
                            arch = Arch_x64;
                        else if (strcmp(a, "pvcpu") == 0)
                            arch = Arch_PVCpu;
                        else if (strcmp(a, "pvcpuc") == 0 || strcmp(a, "pvcpu_c") == 0)
                            arch = Arch_PVCpuC;
                        else {
                            fprintf(stderr, CB_RED "Unknown Architecture: %s\n\tAvailable: [" SArchs_Print "]\n" CS_RESET, a);
                            return 6;
                        }
                    } else {
                        fprintf(stderr, CB_RED "Expected an Architecture?\n\tAvailable: [" SArchs_Print "]\n" CS_RESET);
                        return 7;
                    }
                } else {
                    printf(CB_YELLOW "Warning: Unknown argument - %s\n" CS_RESET, arg);
                }
            }
        }

        if (!binary && arch != Arch_Unknown) {
            printf(CB_YELLOW "Warning: Can only specify architecture when using binary files! Defaulting to file specified architecture\n" CS_RESET);
            arch = Arch_Unknown;
        }

        size_t size = 0;
        char* src = (char*)read_file(value, &size);
        if (size == 0 || src == NULL) {
            fprintf(stderr, CB_RED "Error: Could not read file properly!\n" CS_RESET);
            return 4;
        }
        
        if (!binary) {
            r_info_basic(src, size);
            Architecture arch = r_get_arch(src, size);
            size_t svaddr = 0;
            size_t code_size = 0;
            size_t text_off = r_get_codeoff(src, size, &code_size, &svaddr);

            decode(src, size, arch, svaddr, text_off, code_size);
        } else {
            if (arch == Arch_Unknown) {
                fprintf(stderr, CB_RED "Error: Please specify architecture when using binary files!\n" CS_RESET);
                free(src);
                return 5;
            }
            decode(src, size, arch, 0, 0, size);
        }

        free(src);
    }

    return 0;
}
