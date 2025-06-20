# === HOST CPU ===
ARCH := $(shell uname -m)
BASE_DIR := $(CURDIR)

# Normalize some common outputs
ifeq ($(ARCH),x86_64)
	CPU := amd64
else ifeq ($(ARCH),aarch64)
	CPU := arm64
else ifeq ($(ARCH),armv71)
	CPU := arm32
else
	CPU:= unknown
endif

# === Set vars based on CPU ===
ifeq ($(CPU),amd64)
	MAKEDIR := $(BASE_DIR)/amd64
else ifeq ($(CPU),arm64)
	MAKEDIR := $(BASE_DIR)/arm64
else ifeq ($(CPU),arm32)
	# Arm64 files currently are made in 32-bit (ARM32 support+)
	MAKEDIR := $(BASE_DIR)/arm64
else:
	MAKEDIR := unsupported
endif

MAKE := make

# === TARGETS ===
.PHONY: all clean info

all: info
	@echo "Running Build For: $(ARCH)"
	$(MAKE) -C $(MAKEDIR)

info:
	@echo "Detected Arch: $(ARCH)"
	@echo "Normalized CPU Type: $(CPU)"
	@echo "Build-Dir: $(MAKEDIR)"

clean:
	@echo "Running Clean for: $(ARCH)"
	$(MAKE) -C $(MAKEDIR) clean
