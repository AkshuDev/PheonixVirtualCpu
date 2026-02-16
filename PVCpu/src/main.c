// Author: Pheonix Studios/AkshuDev

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <pvcpu-isa.h>
#include <pvcpu-jit.h>
#include <pvcpu-validator.h>
#include <pvcpu-helpers.h>

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

int main(int argc, char** argv) {

    if (argc < 2) {
        fprintf(stderr, PVCPU_USAGE "\nExample: run:100:arg1,arg_with_value-hi\n\tSeperators: ':', ',', '-'\n");
        return 1;
    }

    // Copy to modifiable buffer
    char buf[1024];
    strncpy(buf, argv[1], sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';

    char* parts[32];
    int count = split_args(buf, parts, 32);

    if (count < 1) {
        fprintf(stderr, "Invalid input.\n");
        return 2;
    }

    const char* func  = parts[0];
    const char* value = (count > 1 ? parts[1] : NULL);
    if (!value) {
        fprintf(stderr, "Please pass value to function [%s]\n", func);
        return 3;
    }

    // Route functions
    if (strcmp(func, "run") == 0) {
        size_t file_size = 0;
        uint8_t* program = read_file(value, &file_size);
        if (!program) {
            fprintf(stderr, "Error: Could not read file [%s]\n", value);
            return 4;
        }

        size_t inst_cap = 2;
        size_t inst_count = 0;
        PVCpu_Inst* insts = calloc(inst_cap, sizeof(PVCpu_Inst));
        uint64_t* inst_values = calloc(inst_cap, sizeof(uint64_t));
        uint64_t** inst_extflags = calloc(inst_cap, sizeof(uint64_t*));
        int* inst_extflags_count = calloc(inst_cap, sizeof(int));

        size_t off = 0;
        size_t idx = 0;
        while (off < file_size) {
            if (inst_count + 1 > inst_cap) {
                inst_cap *= 2;
                insts = realloc(insts, inst_cap * sizeof(PVCpu_Inst));
                inst_values = realloc(inst_values, inst_cap * sizeof(uint64_t));
                inst_extflags = realloc(inst_extflags, inst_cap * sizeof(uint64_t*));
                inst_extflags_count = realloc(inst_extflags_count, inst_cap * sizeof(int));
            }
            inst_extflags[idx] = calloc(2, sizeof(uint64_t));
            size_t read_bytes = pvcpu_unpack_inst(program + off, file_size - off, &insts[idx], &inst_values[idx], inst_extflags[idx], &inst_extflags_count[idx]);
            if (read_bytes == 0 && file_size - off > 4) {
                fprintf(stderr, "Error: Instruction unpacking failed!\n");
                free(program);
                free(insts);
                free(inst_values);
                for (size_t i = 0; i < inst_cap; i++) {
                    free(inst_extflags[i]);
                }
                free(inst_extflags);
                free(inst_extflags_count);
                return 5;
            } else if (file_size - off < 4) {
                break;
            }
            off += read_bytes;
            inst_count += 1;
        }

        // Validate
        for (size_t i = 0; i < inst_count; i++) {
            if (!pvcpu_validate_inst(&insts[i], inst_values[i], inst_cap, 0)) {
                fprintf(stderr, "Validation Failed: This might be a harmful file, DO NOT RUN!\n");
                free(program);
                free(insts);
                free(inst_values);
                for (size_t i = 0; i < inst_cap; i++) {
                    free(inst_extflags[i]);
                }
                free(inst_extflags);
                free(inst_extflags_count);
                return 6;
            }
        }

        pvcpu_run(insts, inst_values, inst_extflags, inst_extflags_count, inst_count, inst_cap, 100, 128, 0);
    
        free(program);
        free(insts);
        free(inst_values);
        for (size_t i = 0; i < inst_cap; i++) {
            free(inst_extflags[i]);
        }
        free(inst_extflags);
        free(inst_extflags_count);
    }
    else if (strcmp(func, "check") == 0) {
        printf("Checking something %s\n", value);
    }
    else {
        fprintf(stderr, "Unknown command.\n");
        return 7;
    }

    return 0;
}
