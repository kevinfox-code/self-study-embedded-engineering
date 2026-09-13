# Peripheral Struct Overlays

## Why `volatile`

`volatile` tells the compiler that the value at this address can change without
anything in the nearby code having written it, and that a write has an effect
beyond the stored value. That matters for hardware registers, whose contents are
changed by the peripheral itself (status flags, data registers) and whose writes
trigger hardware behaviour. Without the qualifier the compiler may cache a
register in a CPU register, reorder accesses, or delete a write whose result is
never read.

The qualifier belongs on each *member*, not on the pointer, so that every access
through the struct is a real load or store.

## Why `uint32_t`

`uint32_t` from `<stdint.h>` is exactly 32 bits wide and unsigned. Fixed width is
what makes the struct a faithful overlay: each member advances the offset by
exactly 4 bytes, so `MODER` lands at `+0x00`, `OTYPER` at `+0x04`, and so on.
`int` or `unsigned` would be implementation-defined in width, and the offsets
could drift on a different target.

## Example: GPIO Peripheral Structure

```c
typedef struct {
    volatile uint32_t MODER;    // Offset 0x00  mode select, 2 bits per pin
    volatile uint32_t OTYPER;   // Offset 0x04  push-pull vs open-drain
    volatile uint32_t OSPEEDR;  // Offset 0x08  output slew rate
    volatile uint32_t PUPDR;    // Offset 0x0C  pull-up / pull-down
    volatile uint32_t IDR;      // Offset 0x10  input data (read-only)
    volatile uint32_t ODR;      // Offset 0x14  output data
    volatile uint32_t BSRR;     // Offset 0x18  atomic bit set / reset
    volatile uint32_t LCKR;     // Offset 0x1C  configuration lock
    volatile uint32_t AFR[2];   // Offset 0x20, 0x24  alternate function low/high
} GPIO_TypeDef;
```

Cast onto the port's base address, the struct replaces every hand-built address
macro:

```c
#define GPIOA ((GPIO_TypeDef *)GPIOA_BASE)

GPIOA->MODER &= ~(3U << (2 * 5));   // clear PA5 mode field
GPIOA->MODER |=  (1U << (2 * 5));   // general-purpose output
GPIOA->BSRR   =  (1U << 5);         // set PA5, atomically
```

This is exactly what the CMSIS device header provides — writing it once by hand
is what makes the generated header readable afterwards.

## Reserved Gaps

Where the register map has a hole, the struct needs an explicit padding member:

```c
volatile uint32_t RESERVED0[2];   // 0x28 - 0x2C not implemented
```

Omitting it shifts every member after the gap, and the resulting bug looks like a
peripheral that ignores writes rather than like a struct that is wrong.
