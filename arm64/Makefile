# === CONFIG ===
BASE_DIR := $(CURDIR)
SRC_DIR := $(BASE_DIR)/src-arm64
BUILD_DIR := $(BASE_DIR)/build-arm64
BIN_DIR := $(BASE_DIR)/bin-arm64
ASSEMBLER := aarch64-linux-gnu-as
LINKER := aarch64-linux-gnu-ld

ENTRY := $(SRC_DIR)/main.s
OUTPUT := $(BUILD_DIR)/pvcpu

# === More Files ===
STRUCTS := $(SRC_DIR)/structs.s

# === FLAGS ===
ASFLAGS := -g -I$(SRC_DIR)
LDFLAGS := -nostdlib

# === OBJECT FILES ===
OBJS := $(BIN_DIR)/pvcpu.o \
	$(BIN_DIR)/structs.o

# === TARGETS ===
.PHONY: all clean

all: dirs $(OUTPUT)

# Create needed dirs
dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

# Assemble Source
$(BIN_DIR)/pvcpu.o: $(ENTRY)
	$(ASSEMBLER) $(ASFLAGS) -o $@ $<

$(BIN_DIR)/structs.o: $(STRUCTS)
	$(ASSEMBLER) $(ASFLAGS) -o $@ $<

# Link
$(OUTPUT): $(OBJS)
	$(LINKER) $(LDFLAGS) -o $@ $^

# Clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
