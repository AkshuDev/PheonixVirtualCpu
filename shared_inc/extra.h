// Author: Pheonix Studios/AkshuDev

#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint64_t key;
    char* value;
} HashMap64;

typedef struct {
    uint32_t key;
    char* value;
} HashMap32;

typedef struct {
    uint16_t key;
    char* value;
} HashMap16;

typedef struct {
    uint8_t key;
    char* value;
} HashMap8;

static inline char* get_value_hashmap8(HashMap8 map[], uint8_t key, size_t total_count) {
    for (size_t i = 0; i < total_count; i++) {
        HashMap8 hm = map[i];
        if (hm.key == key)
            return hm.value;
    }
    return NULL;
}

static inline char* get_value_hashmap16(HashMap16 map[], uint16_t key, size_t total_count) {
    for (size_t i = 0; i < total_count; i++) {
        HashMap16 hm = map[i];
        if (hm.key == key)
            return hm.value;
    }
    return NULL;
}

static inline char* get_value_hashmap32(HashMap32 map[], uint32_t key, size_t total_count) {
    for (size_t i = 0; i < total_count; i++) {
        HashMap32 hm = map[i];
        if (hm.key == key)
            return hm.value;
    }
    return NULL;
}

static inline char* get_value_hashmap64(HashMap64 map[], uint64_t key, size_t total_count) {
    for (size_t i = 0; i < total_count; i++) {
        HashMap64 hm = map[i];
        if (hm.key == key)
            return hm.value;
    }
    return NULL;
}

// Colors ANSI
// Normal text
#define C_BLACK "\e[0;30m"
#define C_RED "\e[0;31m"
#define C_GREEN "\e[0;32m"
#define C_YELLLOW "\e[0;33m"
#define C_BLUE "\e[0;34m"
#define C_MAGENTA "\e[0;35m"
#define C_CYAN "\e[0;36m"
#define C_WHITE "\e[0;37m"

// Regular bold text
#define CB_BLACK "\e[1;30m"
#define CB_RED "\e[1;31m"
#define CB_GREEN "\e[1;32m"
#define CB_YELLOW "\e[1;33m"
#define CB_BLUE "\e[1;34m"
#define CB_MAGENTA "\e[1;35m"
#define CB_CYAN "\e[1;36m"
#define CB_WHITE "\e[1;37m"

// Regular underline text
#define CU_BLACK "\e[4;30m"
#define CU_RED "\e[4;31m"
#define CU_GREEN "\e[4;32m"
#define CU_YELLOW "\e[4;33m"
#define CU_BLUE "\e[4;34m"
#define CU_MAGENTA "\e[4;35m"
#define CU_CYAN "\e[4;36m"
#define CU_WHITE "\e[4;37m"

// Regular background
#define CBG_BLACK "\e[40m"
#define CBG_RED "\e[41m"
#define CBG_GREEN "\e[42m"
#define CBG_YELLOW "\e[43m"
#define CBG_BLUE "\e[44m"
#define CBG_MAGENTA "\e[45m"
#define CBG_CYAN "\e[46m"
#define CBG_WHITE "\e[47m"

// High intensty background 
#define CHBG_BLACK "\e[0;100m"
#define CHBG_RED "\e[0;101m"
#define CHBG_GREEN "\e[0;102m"
#define CHBG_YELLOW "\e[0;103m"
#define CHBG_BLUE "\e[0;104m"
#define CHBG_MAGENTA "\e[0;105m"
#define CHBG_CYAN "\e[0;106m"
#define CHBG_WHITE "\e[0;107m"

// High intensty text
#define CH_BLACK "\e[0;90m"
#define CH_RED "\e[0;91m"
#define CH_GREEN "\e[0;92m"
#define CH_YELLOW "\e[0;93m"
#define CH_BLUE "\e[0;94m"
#define CH_MAGENTA "\e[0;95m"
#define CH_CYAN "\e[0;96m"
#define CH_WHITE "\e[0;97m"

// Bold high intensity text
#define CBH_BLACK "\e[1;90m"
#define CBH_RED "\e[1;91m"
#define CBH_GREEN "\e[1;92m"
#define CBH_YELLOW "\e[1;93m"
#define CBH_BLUE "\e[1;94m"
#define CBH_MAGENTA "\e[1;95m"
#define CBH_CYAN "\e[1;96m"
#define CBH_WHITE "\e[1;97m"

// Special
#define CS_RESET "\e[0m"
