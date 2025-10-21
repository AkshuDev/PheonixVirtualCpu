# =====================================
# AMD64 Cross-Platform NASM Makefile
# =====================================

# Detect OS (lowercased)
UNAME_S := $(shell uname -s | tr '[:upper:]' '[:lower:]')

# Set architecture folder
ARCH_DIR := amd64

# Base OS source directory
OS_SRC_DIR := $(ARCH_DIR)/$(UNAME_S)/src
INCLUDE_DIR := $(OS_SRC_DIR)/inc

# Output directories
BUILD_DIR := $(ARCH_DIR)/build
BIN_DIR := $(ARCH_DIR)/bin

# Binary name
BIN_NAME := pvcpu-$(UNAME_S)

# Find all .asm files in OS-specific src
ASM_FILES := $(wildcard $(OS_SRC_DIR)/*.asm)

# Convert .asm to .o for build
OBJ_FILES := $(patsubst $(OS_SRC_DIR)/%.asm,$(BUILD_DIR)/%-obj-$(UNAME_S).o,$(ASM_FILES))

# Compiler and linker
NASM := nasm
LD := ld

# Flags
NASM_FLAGS := -g -F dwarf -f elf64 -I $(INCLUDE_DIR)/
LD_FLAGS := -g

# Default target
all: prepare $(BIN_DIR)/$(BIN_NAME)

# Prepare folders
prepare:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

# Build object files
$(BUILD_DIR)/%-obj-$(UNAME_S).o: $(OS_SRC_DIR)/%.asm
	@echo "Assembling $< -> $@"
	@$(NASM) $(NASM_FLAGS) $< -o $@

# Link final binary
$(BIN_DIR)/$(BIN_NAME): $(OBJ_FILES)
	@echo "Linking $@"
	@$(LD) $(LD_FLAGS) -o $@ $^

# Clean build files
clean:
	@echo "Cleaning build and bin directories..."
	@rm -rf $(BUILD_DIR)/*
	@rm -rf $(BIN_DIR)/*

.PHONY: all clean prepare
