# Pheonix Disassembler (PDASM)

PDASM is a cross-platform disassembler and executable inspection tool supporting multiple architectures including **x86, x64, and PVCpu (Pheonix Virtual CPU variants)**. It can analyze both native binaries (`.exe`, ELF) and raw binary input.

---

## Features

* Multi-architecture disassembly:

  * x86 (i386)
  * x64 / x86_64 / amd64
  * PVCpu (Pheonix Virtual CPU)
  * PVCpu-C (Pheonix Virtual CPU - Compressed Variant)

* File format support:

  * Windows PE (`.exe`)
  * Linux ELF
  * Raw binaries

* Deep executable analysis:

  * Sections
  * Symbols
  * Relocations
  * Program headers
  * Basic metadata

* Flexible disassembly modes:

  * Auto-detect from file
  * Manual binary mode with forced architecture

---

## Usage

```bash
pdasm <command> <OPTIONAL:input> <OPTIONAL:[OPTIONS]>
```

---

## Commands

### info

Display detailed information about an executable.

```bash
pdasm info <file> <OPTIONAL:[OPTIONS]>
```

Example

```bash
pdasm info myfile --all
```

#### Options:

* `--all` (default) -> Show everything
* `--basic` -> Basic file information
* `--sections` -> Section table
* `--symbols` -> Symbol table
* `--relocs` -> Relocation entries
* `--program` -> Program headers / load info

---

### disassemble

Disassemble a binary or executable.

```bash
pdasm disassemble <file> <OPTIONAL:[OPTIONS]>
```

or shorthand:

```bash
pdasm dasm <file> <OPTIONAL:[OPTIONS]>
```

Example

```bash
pdasm disassemble myfile -binary --arch pvcpu
```

#### Options:

* `--binary` -> Treat input as raw binary
* `--arch <architecture>` -> Force architecture (Required when using *--binary* flag)

#### Supported architectures:

* `x86` / `i386`
* `x64` / `x86_64` / `amd64`
* `pvcpu` -> Pheonix Virtual CPU
* `pvcpuc` / `pvcpu-c` -> Pheonix Virtual CPU - Compressed Variant

---

## Examples

### Inspect an executable

```bash
pdasm info myprogram.exe --sections
```

### Full analysis

```bash
pdasm info app.elf --all
```

### Auto disassemble

```bash
pdasm disassemble app.elf
```

### Raw binary with forced architecture

```bash
pdasm disassemble shellcode.bin --binary --arch x64
```

---

## Architecture Support

| Architecture | Description                               |
| ------------ | ----------------------------------------- |
| x86          | 32-bit Intel/AMD                          |
| x64          | 64-bit Intel/AMD                          |
| PVCpu        | Pheonix Virtual CPU                       |
| PVCpu-C      | Compressed Variant of Pheonix Virtual CPU |

---

## Notes

* If architecture is not provided in `--binary` mode, PDASM will error out.
* Non-binary mode auto-detects architecture from file metadata.
* Output is colorized in supported terminals.

---

## Limitations

* No GUI (CLI-only tool)