# Author: Pheonix Studios / AkshuDev
# PVCpu cross-platform build system

# CONFIG

BUILD ?= debug
OS := $(shell uname | tr '[:upper:]' '[:lower:]')
ARCH := $(shell uname -m)
CC := gcc
CFLAGS_RELEASE := -Wall -Wextra -std=c17 -I inc
CFLAGS_DEBUG := -Wall -Wextra -std=c17 -g -fsanitize=address,undefined -I inc
LDFLAGS := 

ifeq ($(BUILD),release)
	CFLAGS := $(CFLAGS_RELEASE)
else
	CFLAGS := $(CFLAGS_DEBUG)
endif

# Cross-build for Windows (mingw)
ifeq ($(BUILD_WIN),1)
    CC := x86_64-w64-mingw32-gcc
    OS := windows
endif

# Source files
SRCS := $(wildcard src/*.c)
OBJS := $(patsubst src/%.c,build/%_$(OS)_$(ARCH).o,$(SRCS))

# Output binary
BIN := bin/$(OS)/$(ARCH)/pvcpu

# Separator for inspect target
INSPECT_VAR ?= 

# DEFAULT TARGET
.PHONY: all
all: $(BIN)

# BUILD RULES
build/%_$(OS)_$(ARCH).o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJS)
	@mkdir -p bin/$(OS)/$(ARCH)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Built $(BIN) [$(BUILD)]"

# SPECIAL TARGETS

.PHONY: clean
clean:
	rm -rf build bin
	@echo "Cleaned build and bin directories"

.PHONY: help
help:
	@echo "PVCpu Makefile Help"
	@echo ""
	@echo "Targets:"
	@echo "  all             Build default target (debug build)"
	@echo "  build_win=1     Build for Windows using mingw-gcc"
	@echo "  clean           Remove build/ and bin/ directories"
	@echo "  inspect VAR=... Print the value of VAR"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD=debug|release    (default debug)"
	@echo "  INSPECT_VAR=<value>    Used with inspect target"

.PHONY: inspect
inspect:
ifeq ($(INSPECT_VAR),)
	@echo "Please set INSPECT_VAR=<value>"
else
	@echo "INSPECT_VAR=$(INSPECT_VAR)"
endif

