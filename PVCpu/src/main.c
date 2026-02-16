// Author: Pheonix Studios/AkshuDev

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <pvcpu-isa.h>
#include <pvcpu-jit.h>
#include <pvcpu-validator.h>
#include <pvcpu-helpers.h>

#include <extra.h>

#define PVCPU_USAGE "Usage: pvcpu command <OPTIONAL:input> <OPTIONAL:--[OPTIONS]>"

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
    printf(PVCPU_USAGE "\n\nCommands:\n");
    printf("run <file>      - Run a PVCpu binary\n");
    printf("check <value>   - Debug command\n");
    printf("help            - Display this message\n");
}

typedef struct {
    bool help;
    bool run;
    bool check;
    bool error;

    char* run_input;
    char* check_input;
} Args_t;

static void parse_args(Args_t* args, int argc, char** argv) {
    memset(args, 0, sizeof(Args_t));

    if (argc < 2) {
        args->error = true;
        return;
    }

    char* cmd = argv[1];

    if (!strcmp(cmd, "help")) {
        args->help = true;
        return;
    }
    else if (!strcmp(cmd, "run")) {
        args->run = true;
        if (argc < 3) {
            fprintf(stderr, "run requires <file>\n");
            args->error = true;
            return;
        }
        args->run_input = argv[2];
    }
    else if (!strcmp(cmd, "check")) {
        args->check = true;
        if (argc < 3) {
            fprintf(stderr, "check requires <value>\n");
            args->error = true;
            return;
        }
        args->check_input = argv[2];
    }
    else {
        fprintf(stderr, "Unknown command '%s'\n", cmd);
        args->error = true;
    }
}

int main(int argc, char** argv) {

    if (argc < 2) {
        fprintf(stderr, PVCPU_USAGE "\nExample: run:100:arg1,arg_with_value-hi\n\tSeperators: ':', ',', '-'\n");
        return 1;
    }

    Args_t args = {0};
    parse_args(&args, argc, argv);
    if (args.error) {
        return 2;
    }

    // Route functions
    if (args.run) {
        size_t file_size = 0;
        uint8_t* program = read_file(args.run_input, &file_size);
        if (!program) {
            fprintf(stderr, "Error: Could not read file [%s]\n", args.run_input);
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
    else if (args.check) {
        printf("Checking something %s\n", args.check_input);
    }
    else {
        fprintf(stderr, "Unknown command.\n");
        return 7;
    }

    return 0;
}
