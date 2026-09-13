# ESE-311 Week 06 — GPIO via CMSIS

## Objectives

- Move from hand-written address macros to typed peripheral access.
- Build a peripheral struct overlay by hand, then compare it with the CMSIS
  device header that ships the same thing.
- Understand why every member is `volatile` and why the member types fix the
  offsets.

## What Was Built

- [`periph_structs`](periph_structs) — a hand-written `GPIO_TypeDef`-style struct
  cast onto the GPIO base address.
- [`cmsis_example`](cmsis_example) — the same LED toggle written against the
  CMSIS device header (`GPIOA->MODER`).
- [`chip_headers`](chip_headers) — the CMSIS Core and STM32U5 device headers the
  later weeks build against.

## Contents

- [`ESE-311_Peripheral-Structs.md`](ESE-311_Peripheral-Structs.md) — walkthrough
  of the struct overlay: why `volatile`, why `uint32_t`, and how the member order
  maps onto the register offsets.

## Key Concepts

- **Struct member order *is* the register order.** A missing or reordered member
  silently shifts every register after it.
- **`volatile` on each member.** The qualifier belongs on the members, so that
  each access through the pointer is a real load or store.
- **Fixed-width types keep alignment.** `uint32_t` guarantees the 4-byte stride
  the register map assumes; `int` or `unsigned` would be implementation-defined.
- **Reserved fields are declared, not skipped.** Gaps in the register map need
  explicit padding members to keep later offsets correct.

## Build and Run

Each subproject builds the same way:

```bash
cd ESE-311/Week06-GPIO-CMSIS/cmsis_example   # or periph_structs
cmake -B build
cmake --build build
cmake --build build --target flash
```

## References

- Israel Gbati, *Bare-Metal Embedded C Programming* — CMSIS chapter
- [CMSIS-Core documentation](https://arm-software.github.io/CMSIS_6/latest/Core/index.html)
- See [ESE-311_References.md](../ESE-311_References.md) for download links

## AI Assistance

None.
