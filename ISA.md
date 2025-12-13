# PVCpu Instruction Set Architecture (ISA)

## Overview

The **PVCpu Architecture** (Pheonix Virtual CPU Architecture) defines a 32-bit fixed-width instruction set designed for simplicity, extensibility, and operating-system-level control.

Two architectural variants exist:
- **PVCpu Architecture**: The standard, uncompressed instruction format described in this document.
- **PVCpu-C Architecture**: A compressed instruction format intended for higher density and complexity. The PVCpu-C format is more complex and is described separately below.

This document provides a high-level architectural description. Detailed opcode definitions, register encodings, instruction modes, and extended flag semantics are specified in **`Docs/PVCpu.md`**.

---

## Instruction Size and Layout

All PVCpu instructions are **32 bits** wide and follow a fixed layout:

[ opcode: 12 bits ][ mode: 4 bits ][ src: 6 bits ][ dest: 6 bits ][ flags: 4 bits ]

### Field Descriptions

- **Opcode (12 bits)**  
  Identifies the operation to be executed by the processor.

- **Mode (4 bits)**  
  Determines the instruction’s addressing mode, operand interpretation, and execution behavior.

- **Source (6 bits)**  
  Encodes the source register or operand identifier.

- **Destination (6 bits)**  
  Encodes the destination register or operand identifier.

- **Flags (4 bits)**  
  A static bitmask controlling instruction validity and extension behavior.

---

## Instruction Flags

The **Flags** field is a static bitmask with the following meaning:

- **Bit 0 — Valid Instruction**  
  Indicates whether the instruction is valid and executable. If unset, the instruction is treated as invalid.

- **Bit 1 — Immediate / Absolute Extension**  
  Specifies that the next 64 bits in the instruction stream represent an immediate value or an absolute memory address.

- **Bit 2 — Displacement Extension**  
  Specifies that the next 64 bits in the instruction stream represent a signed or unsigned 64-bit displacement value.

- **Bit 3 — Extended Flags Present**  
  Specifies that the next 64 bits contain an extended flags structure.

---

## Extended Flags

Extended flags form a static bitmask used to further refine instruction behavior.

Extended flags may be **chained up to two times**, producing the following hierarchy:

Flags → Extended Flags → Extended-Extended Flags → Advanced Flags

Each level is represented by an additional 64-bit extension in the instruction stream. The semantics and layout of each extension level are defined in **`Docs/PVCpu.md`**.

---

## Register Architecture

### Register Count

- **Total Registers**: 40  
- **Program-Accessible Registers**: 34  

Registers are encoded using 6-bit identifiers.

---

## Program-Accessible Registers

### NULL Register

- **NULL**  
  A constant-value register that always resolves to zero when read.  
  Writes to this register are discarded and have no effect.  
  It is commonly used for operand suppression, comparisons, and placeholder destinations.

### General-Purpose Registers

- **G0 – G30 (General0 – General30)**  
  General-purpose registers used for arithmetic operations, data movement, addressing, and intermediate values.  
  These registers have no implicit behavior and may be freely used by programs.

### Special Registers

- **LR (Link Register)**  
  Holds the return address for control-transfer instructions.  
  For correctness and safety, return addresses are also pushed onto the stack, ensuring stack-based validation of control flow.

- **SF (Stack Frame Register)**  
  Points to the base of the current stack frame.  
  Used to access local variables, function arguments, and saved state.

- **SP (Stack Pointer)**  
  Points to the top of the active stack.  
  Automatically updated by stack-related instructions.

---

## Internal Registers

Internal registers are reserved for processor control, execution flow, and system-level operations.

- **PC (Program Counter)**  
  Holds the address of the currently executing instruction.  
  Updated automatically during instruction fetch and control flow changes.

- **IP (Instruction Pointer)**  
  Tracks the current position within the instruction stream and assists with multi-word instruction decoding.

- **I0 – I3 (Internal0 – Internal3)**  
  Internal-purpose registers reserved for operating system use.  
  These registers are not accessible to user programs and are intended for low-level control, scheduling, and exception handling.

---

## Further Specification

The following components are defined in detail in **`Docs/PVCpu.md`**:

- Opcode definitions and semantics  
- Instruction modes  
- Flag, extended flag, extended-extended flag, and advanced flag layouts  
- Register encodings  
- Operand interpretation rules  
- PVCpu-C compressed instruction format  

This document serves as the architectural overview and entry point for the PVCpu ISA.
