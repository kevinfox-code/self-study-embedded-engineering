# Makefiles — Hand-Written Build (STM32U575)

The same firmware as the CMake projects, built from a hand-written Makefile so
the compile, assemble, link, and objcopy steps are all visible. Week context:
[../README.md](../README.md).

## Build

```bash
make
```

## Flash

```bash
make load
```

Requires `arm-none-eabi-gcc` and `openocd` on `PATH`. `make clean` removes the
object files and the ELF.
