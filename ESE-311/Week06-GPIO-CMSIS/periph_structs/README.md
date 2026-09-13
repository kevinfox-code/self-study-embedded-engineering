# periph_structs — Peripheral Struct Overlays (STM32U575)

Builds the peripheral access pattern by hand: a `volatile`-qualified struct whose
member offsets mirror the register map, cast onto the peripheral base address.
This is what the CMSIS device header does for you. Week context:
[../README.md](../README.md).

See [../ESE-311_Peripheral-Structs.md](../ESE-311_Peripheral-Structs.md) for the
walkthrough of why each member is `volatile` and why `uint32_t` keeps the offsets
aligned.

## Build

```bash
cmake -B build
cmake --build build
```

## Flash

```bash
cmake --build build --target flash
```
