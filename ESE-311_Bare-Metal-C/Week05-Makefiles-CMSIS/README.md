# ESE-311 Week 05 — Makefiles

## Objectives

- Replace the generated build with a hand-written Makefile so every compiler,
  assembler, and linker invocation is explicit.
- Understand the cross-compilation flags the target needs: CPU, FPU, ABI,
  and the linker script.
- Add a flash target that drives OpenOCD from the same Makefile.

## What Was Built

- [`Makefiles`](Makefiles) — the same firmware as the CMake projects, built with
  pattern rules for `.c` and `.s`, an explicit link step, and `make load` to
  flash the board.

## Key Concepts

- **Target flags travel together.** `-mcpu=cortex-m33 -mthumb` plus the FPU and
  float-ABI flags must match between every compile and the link, or the linker
  rejects objects with incompatible attributes.
- **`-T` selects the linker script.** Without it the toolchain links against a
  host default and produces an image that will not boot.
- **`--specs=nano.specs` / `nosys.specs`.** Picks the reduced newlib and stubs
  out the syscalls a bare-metal target has no OS for.
- **Objcopy produces the loadable image.** The ELF carries symbols and section
  metadata; the `.bin` or `.hex` is what gets written to flash.

## Build and Run

```bash
cd ESE-311_Bare-Metal-C/Week05-Makefiles-CMSIS/Makefiles
make
make load
```

## References

- Israel Gbati, *Bare-Metal Embedded C Programming* — Makefiles chapter
- Arm GNU Toolchain documentation for the `-mcpu` / `-mfpu` / `-mfloat-abi` set
- See [ESE-311_References.md](../ESE-311_References.md) for download links

## AI Assistance

None.
