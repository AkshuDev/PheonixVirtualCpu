// Author: Pheonix Studios/AkshuDev

#include <stdio.h> // More Native and widely supported
#include <string.h> // PStdlib currently doesn't provide pure ASM implementation

#include <pstdlib.h>

#include <pvcpu-isa.h>

#include <extra.h>

#define PVCPU_USAGE "Usage: pvcpu command <OPTIONAL:input> <OPTIONAL:--[OPTIONS]>\n"

static uint8_t* read_file(const char* filename, size_t* out_size) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("Error opening file");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    usize_t size = (usize_t)ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t* buf = alloc(size);
    if (!buf) {
        perror("Memory allocation failed");
        fclose(f);
        return NULL;
    }

    if (fread(buf, 1, size, f) != size) {
        perror("Error reading file");
        dealloc(buf);
        fclose(f);
        return NULL;
    }

    fclose(f);
    *out_size = size;
    return buf;
}

static void print_help() {
    printf(PVCPU_USAGE "\nCommands:\n");
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
        fprintf(stderr, PVCPU_USAGE);
        return 1;
    }

    Args_t args = {0};
    parse_args(&args, argc, argv);
    if (args.error) {
        return 2;
    }

    // Route functions
    if (args.help) {
        print_help();
    } else if (args.run) {
        size_t data_size = 0;
        uint8_t* data = read_file(args.run_input, &data_size);
        if (!data || data_size == 0) {
            printf("Failed!\n");
            return 3;
        }

        return pvcpu_run(data, data_size, 0); // 0 = Unlimited Memory Limit
    } else if (args.check) {
        printf("Checking something %s\n", args.check_input);
    } else {
        fprintf(stderr, "Unknown command.\n");
        return 7;
    }

    return 0;
}
