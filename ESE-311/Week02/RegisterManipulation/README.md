# STM32U575 Register Manipulation Notes

This example matches the bare-metal logic in [Core/Src/main.c](RegisterManipulation/Core/Src/main.c): enable the GPIOB clock, configure PB7 as a general-purpose output, and drive PB7 high.

## Address Map Used By The Code

```c
PERIPH_BASE_NS       = 0x40000000
AHB2PERIPH_BASE_NS   = PERIPH_BASE_NS + 0x02020000 = 0x42020000
AHB3PERIPH_BASE_NS   = PERIPH_BASE_NS + 0x06020000 = 0x46020000
GPIOB_BASE           = AHB2PERIPH_BASE_NS + 0x0400 = 0x42020400
RCC_BASE             = AHB3PERIPH_BASE_NS + 0x0C00 = 0x46020C00
RCC_AHB2ENR1         = RCC_BASE + 0x008C          = 0x46020C8C
GPIOB_MODER          = GPIOB_BASE + 0x0000        = 0x42020400
GPIOB_ODR            = GPIOB_BASE + 0x0014        = 0x42020414
```

The important detail is that the register offset is added to the base address, while the bit number is used only inside the bitwise operation.

## What `main.c` Does

1. Enable the GPIOB peripheral clock with `RCC_AHB2EN_R |= GPIOB_EN;`.
1. Clear the PB7 mode bits with `GPIOB_MODE_R &= ~PB7_OUTPUT_MASK;`.
1. Set PB7 to output mode with `GPIOB_MODE_R |= PB7_OUTPUT_MODE;`.
1. Set PB7 high with `GPIOB_OD_R |= PIN7_ON;`.
1. Stay in an empty infinite loop.

## Bitwise Pattern

The code uses the usual register pattern:

```c
Register |= (1U << bit_position);   // set bit
Register &= ~(1U << bit_position);  // clear bit
```

For PB7, the mode register uses two bits per pin, so bits 14 and 15 control the mode. The example clears both bits first, then writes `01` to select output mode.

## Chapter 2 Notes

### Why the `UL` suffix matters

The `UL` suffix on constants such as `0x40000000UL` tells the compiler to treat the value as an unsigned long. That keeps the address literal in an unsigned form and avoids signed conversion issues when doing pointer arithmetic or building register addresses.

### Why the code uses `*(volatile unsigned int *)`

The register macros cast a raw memory address to a pointer, dereference it, and access the register value directly.

- The cast tells the compiler that the number is a memory-mapped address.
- The `*` dereference reads or writes the value at that address.
- `volatile` tells the compiler the value can change outside normal program flow, so it must not optimize away reads or writes.

### Common bare-metal terms

- Bit mask: a binary value used to set, clear, or toggle specific bits.
- Alias: a readable name given to a bit or group of bits for clarity in code.

## Result

After reset, the program directly configures PB7 as a digital output and drives it high. On this board, that corresponds to the blue LED being turned on when the pin is wired active-high.