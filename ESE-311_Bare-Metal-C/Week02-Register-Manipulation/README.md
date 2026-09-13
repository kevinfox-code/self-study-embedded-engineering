# ESE-311 Week 02 — Register Manipulation

## Objectives

- Build peripheral register addresses from bus base plus offset, by hand.
- Use `volatile`-qualified pointer casts to read and write memory-mapped
  registers.
- Apply the set / clear / toggle bit patterns that every later driver relies on.

## What Was Built

- [`RegisterManipulation`](RegisterManipulation) — enables the GPIOB clock,
  configures PB7 as a general-purpose output, and drives it high, using nothing
  but address macros and bitwise operations.

## Contents

- [`RegisterManipulation/README.md`](RegisterManipulation/README.md) — the
  address arithmetic used by the code, and why `volatile` and the `UL` suffix
  matter.
- [`RegisterManipulation/HW2_System-Architecture.md`](RegisterManipulation/HW2_System-Architecture.md)
- [`RegisterManipulation/HW2_Volatile_Register_Accessor_Macros.md`](RegisterManipulation/HW2_Volatile_Register_Accessor_Macros.md)

## Key Concepts

- **Offset vs bit position.** The register offset is added to the base address;
  the bit number is only ever used inside the shift.
- **`volatile` is not optional.** Without it the compiler is free to cache or
  drop a register access that has a hardware side effect.
- **Two bits per pin in `MODER`.** Clear the field before setting it.

## References

- Israel Gbati, *Bare-Metal Embedded C Programming* — Ch. 2
- RM0456 §RCC and §GPIO register descriptions
- See [ESE-311_References.md](../ESE-311_References.md) for download links

## AI Assistance

None.
