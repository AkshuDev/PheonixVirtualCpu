# PVCpu Instruction Set Architecture (ISA)

*PVCpu* and *PVCpu-C* can be categorised as *MHSA* **(Modern Huge Simple Architecture, not offically recognised)** rather than *RISC* or *CISC*

## Overview

The **PVCpu Architecture** (Pheonix Virtual CPU Architecture) defines a modular instruction set with extensibility and efficient decoding as core design goals.

Two architectural variants exist:

- **PVCpu Architecture**  
  The standard fixed-width instruction format, using a 32-bit base instruction with optional 64/32-bit extensions.

- **PVCpu-C Architecture**  
  A compressed instruction format optimized for reduced code size and higher instruction density.  
  PVCpu-C modifies instruction fetching and decoding behavior while preserving the logical execution model.

This document provides a high-level architectural description. Detailed opcode definitions, instruction modes, register encodings, and flag semantics are specified in **`Docs/PVCpu.md`**.

---

## PVCpu Instruction Format (Standard)

### Instruction Size and Layout

All standard PVCpu instructions are **32 bits** wide and follow a fixed layout:

[ opcode: 12 bits ][ mode: 4 bits ][ src: 6 bits ][ dest: 6 bits ][ flags: 4 bits ]

### Flags

The flags field is a static bitmask:

- **Bit 0 — Valid Instruction**  
  Marks the instruction as valid and executable.

- **Bit 1 — Immediate / Absolute / Displacement Extension**  
  The next 64/32 bits represent an immediate value or an absolute memory address.

- **Bit 2 — Extension width**  
  If marked 1, then *Extensions* are 64-bit else they are 32-bit *Extensions*.

- **Bit 3 — Extended Flags Present**  
  The next 64/32 bits contain extended flags.

### Extended Flags

Extended flags are static bitmasks that refine instruction behavior.  
They may be chained up to two times, forming the following hierarchy:

Flags → Extended Flags → Extended-Extended Flags → Advanced Flags

---

## Register Architecture

### Register Count

- **Total Registers**: 40
- **Program-Accessible Registers**: 34

### Program-Accessible Registers

- **NULL**  
  Always resolves to zero when read. Writes are ignored.

- **G0 – G30**  
  General-purpose registers used for computation, addressing, and data movement.

- **LR (Link Register)**  
  Holds the return address for control-transfer operations.

- **SF (Stack Frame Register)**  
  Points to the base of the current stack frame.

- **SP (Stack Pointer)**  
  Points to the top of the active stack.

### Internal Registers

- **PC (Program Counter)**  
  Holds the address of the current instruction.

- **I0 – I2 (Internal0 - Internal2)**  
  Internal registers reserved for operating system use.

- **TR (Timing Register)**  
  Tracks timings, useful for accurate delays/timers within the OS

---

## PVCpu-C Architecture (Compressed Format)

### Overview

**PVCpu-C** is a compressed instruction encoding designed to reduce instruction size and improve code density.  
While execution semantics remain consistent with the standard PVCpu architecture, instruction fetching and decoding differ to support variable-length instructions.

PVCpu-C enables instructions to shrink from a minimum of **4 bytes to as little as 2 bytes**, particularly for operations that do not require operands.

**NOTE: PVCpu-C is not used for execution within any *Pheonix-CPUs***

---

### Base Instruction Format

PVCpu-C instructions begin with a compact header:

[ opcode: 12 bits ][ extender: 4 bits ]

The **extender** field is a control bitmask that determines which optional components follow in the instruction stream.

---

### Extender Bitmask

- **Bit 0 — Valid Instruction**  
  Marks the instruction as valid and executable.

- **Bit 1 — Mode / Register Field Size Present**  
  When set, the next 1 byte is fetched in the following format:

[ RegFieldSize: 4 bits ][ mode: 4 bits ]

- **RegFieldSize** selects the register encoding width and may be 1, 2, 4, or 6 bits.
- **Mode** defines the instruction mode.  
  When this extender bit is inactive, the mode implicitly defaults to 1.

- **Bit 2 — Registers Present**  
When set, the next `(RegFieldSize × 2)` bits encode the registers, with the source register first and the destination register second.

- **Bit 3 — Flags Present**  
When set, the next 4 bits encode the instruction flags.

---

### Extended Flags in PVCpu-C

In PVCpu-C, extended flags differ from the standard format:

- Extended flags are **32 bits wide**
- They may be **chained up to four times**
- This allows access to all instruction fields, advanced behaviors, and modifiers within the compressed format

Immediate values and displacement values are also **32 bits** wide in PVCpu-C.

---

### Architectural Impact

The PVCpu-C format introduces several important effects:

- **Reduced Code Size**  
Instructions without operands or extensions may occupy as little as 2 bytes, significantly improving code density.

- **Flexible Operand Encoding**  
Variable register field sizes allow balancing compactness and expressiveness on a per-instruction basis.

- **Lower Memory Bandwidth Usage**  
Smaller instruction footprints reduce instruction fetch pressure and improve cache efficiency.

- **Increased Decoder Complexity**  
Variable-length instructions and chained extensions require more sophisticated fetching and decoding logic.

Despite these differences, PVCpu-C preserves the logical execution model of the standard PVCpu architecture.

---

## Further Specification

The following components are defined in detail in **`Docs/PVCpu.md`**:

- Opcode definitions and semantics  
- Instruction modes  
- Flag, extended flag, extended-extended flag, and advanced flag layouts  
- Register encodings  
- Operand interpretation rules  
- PVCpu-C detailed decoding rules and extension behavior
- Page Tables and P-XPV
- CPU Info structures

This document serves as the architectural overview and entry point for the PVCpu Instruction Set Architecture.
