# PVCpu – **Pheonix Virtual CPU JIT Runtime**

A high-performance, cross-platform Just-In-Time (JIT) execution environment for the **PVCpu** and **PVCpu-Compressed** architectures. PVCpu allows binaries built for the Pheonix Virtual Architecture to run natively on real machines through dynamic code translation, advanced validation, and runtime optimization.

PVCpu is a core component of the **Pheonix Ecosystem**, enabling a unified execution model across Windows, Linux, and AOS++.

---

## Overview

PVCpu is a lightweight but powerful virtual CPU that executes machine code for the **PVCpu** and **PVCpu-Compressed (PVCu-C)** architectures.
Instead of interpreting instructions, PVCpu dynamically translates them into the host’s native machine instructions (x86-64, ARM64, etc.) at runtime. This provides:

* Near-native performance
* Advanced runtime safety checks
* Binary portability across platforms
* Support for custom executable formats like **AOSF** and **CAOSF**

The full instruction set is documented in **ISA.md**.

---

## Key Features

### Full JIT Compilation

PVCpu reads PVCpu instructions embedded inside supported executable formats and translates them to native host instructions on the fly. The JIT engine performs:

* Instruction validation
* Register mapping
* Memory & safety checks
* Dynamic block generation
* Host execution with low overhead

### Multi-Format Executable Support

PVCpu can run binaries embedded inside:

* **ELF** (Executable and Linkable Format) - Linux
* **PE / EXE** (Portable Executable) - Windows
* **AOSF** (Aftergreat Operating System Executable Format) - AOS++
* **CAOSF** (Compressed Aftergreat Operating System Executable Format) - AOS++

All formats may contain PVCpu or PVCpu-Compressed sections, which are extracted and executed.

### PVCpu-Compressed Support

PVCpu-Compressed (PVCpu-C) binaries are smaller and optimized. PVCpu automatically:

* Decompresses instructions
* Validates the compressed format
* Expands into executable PVCpu blocks
* Compiles them in the JIT pipeline

### Part of the Pheonix Ecosystem

PVCpu is designed to work alongside other Pheonix tools:

* **PAC (Pheonix Assembler Collection)**

  * Multi-architecture assembler
  * Can compile PVCpu binaries, but also target many real architectures
  * Fully separate project with optional integration
  * [PAC Github](https://github.com/AkshuDev/PAC)
  * [PAC Download](https://pheonix-studios-git.github.io/PPI)

* **PDumper (Pheonix Dumper)**

  * Disassembler
  * Debugger
  * Memory inspector
  * Register viewer

* Additional Pheonix tools (runtime, build systems, CLI utilities)

### Cross-Platform Execution

PVCpu binaries run identically across:

* Windows
* Linux
* AOS++
* Any platform supported by the host JIT backend

## How the JIT Pipeline Works

1. **Executable Loading**
   PVCpu reads the ELF/PE/AOSF/CAOSF file and extracts the relevant PVCpu sections (**.text**) if the Executable specifies PVCpu format, else it throws an error.

3. **Instruction Extraction**
   A PVCpu or PVCpu-Compressed stream is decoded.

4. **Validation Pass**
   Ensures the instruction stream is well-formed, safe, and ISA-compliant.

5. **Translation Pass**
   PVCpu instructions are converted into host-native micro-operations.

6. **JIT Compilation**
   Micro-ops are combined into optimized native code blocks.

7. **Execution**
   Code is executed with optional debugging hooks through PDumper.

8. **Runtime Optimization**
   Hot paths may be recompiled for better performance.

---

## Build Instructions

### **Dependencies**

* A Compiler that supports PAC (Example: AOL-Compiler)
* PAC (For Assembling & Linking)

### **Linux**

```bash
# Compile, for example AOL (.pasm stands for Pheonix Assembly)
aol --compile-only myfile.aol -o myfile.pasm
aol --compile-only anotherfile.aol -o anotherfile.pasm
# Assemble and Link for PVCpu
pac myfile.pasm anotherfile.pasm -o myprogram.elf --arch pvcpu --bits 64

# AOL Actually uses PAC so doing this also will work -
aol myfile.aol anotherfile.aol -o myprogram.elf --arch pvpcu --bits 64
```

### **Windows (MSVC)**

```bash
# Compile, for example AOL (.pasm stands for Pheonix Assembly)
aol --compile-only myfile.aol -o myfile.pasm
aol --compile-only anotherfile.aol -o anotherfile.pasm
# Assemble and Link for PVCpu
pac myfile.pasm anotherfile.pasm -o myprogram.exe --arch pvcpu --bits 64 --format win64

# AOL Actually uses PAC so doing this also will work -
aol myfile.aol anotherfile.aol -o myprogram.exe --arch pvpcu --bits 64 --format win64
```

### **AOS++**
***NOTE: Please use ASH, here BASH is used for an example***

```bash
# Compile, for example AOL (.pasm stands for Pheonix Assembly)
aol --compile-only myfile.aol -o myfile.pasm
aol --compile-only anotherfile.aol -o anotherfile.pasm
# Assemble and Link for PVCpu
pac myfile.pasm anotherfile.pasm -o myprogram.aosf --arch pvcpu --bits 64 --format aosf

# AOL Actually uses PAC so doing this also will work -
aol myfile.aol anotherfile.aol -o myprogram.aosf --arch pvpcu --bits 64 --format aosf
```

---

## Running a PVCpu Program
***NOTE: PVCpu JIT uses Pheonix Studios Argument System***

```sh
pvcpu run:myprogram
```

Verbose mode for debugging:

```sh
pvcpu run:myprogram:debug
```

---

## Debugging with PDumper

```sh
pdumper dump:myprogram
```

Features:

* Disassemble PVCpu and PVCpu-Compressed binaries
* Breakpoints
* Step-through execution
* Inspect registers & memory
* Export instruction dumps
* Visualize JIT-generated native code (optional)

---

## Cross-Platform Goals

PVCpu is engineered to allow:

* One binary → Runs everywhere
* No recompilation needed
* PVCpu as a universal executable target

This makes it the backbone of binary portability inside the Pheonix ecosystem.

---

## License

MIT License

Copyright (c) 2025 AkshuDev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
