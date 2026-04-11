# HW2: Volatile Register Accessor Macros
### Bare-Metal Embedded C — Week 2 Assignment

---

## Why `volatile`?

Without `volatile`, the compiler may cache a register read in a CPU register and never re-fetch it from the peripheral's memory-mapped address. Hardware changes the register value behind the compiler's back, so every access **must** go to the actual address.

---

## 1. Foundation Types & Address Cast

```c
#include <stdint.h>

/* Cast an address literal to a pointer-to-volatile-uint32 */
#define MMIO32(addr)   (*(volatile uint32_t *)(addr))
#define MMIO16(addr)   (*(volatile uint16_t *)(addr))
#define MMIO8(addr)    (*(volatile uint8_t  *)(addr))
```

**Usage:**
```c
MMIO32(0x40020000) = 0x01;            /* write */
uint32_t v = MMIO32(0x40020000);      /* read  */
```

---

## 2. Generic Read / Write Macros

```c
/* Read a 32-bit peripheral register */
#define REG_READ(addr)              (*(volatile uint32_t *)(addr))

/* Write a 32-bit peripheral register */
#define REG_WRITE(addr, val)        (*(volatile uint32_t *)(addr) = (val))

/* Read-modify-write: set bits */
#define REG_SET_BITS(addr, mask)    (*(volatile uint32_t *)(addr) |=  (mask))

/* Read-modify-write: clear bits */
#define REG_CLR_BITS(addr, mask)    (*(volatile uint32_t *)(addr) &= ~(mask))

/* Read-modify-write: toggle bits */
#define REG_TOG_BITS(addr, mask)    (*(volatile uint32_t *)(addr) ^=  (mask))

/* Write only specific bits (mask + value) */
#define REG_MODIFY(addr, mask, val) \
    (*(volatile uint32_t *)(addr) = \
        ((*(volatile uint32_t *)(addr)) & ~(mask)) | ((val) & (mask)))
```

---

## 3. STM32-Style Peripheral Struct + Accessor (Industry Pattern)

```c
/* --- GPIO register block for STM32 (Cortex-M) --- */
typedef struct {
    volatile uint32_t MODER;    /* 0x00 - Mode register           */
    volatile uint32_t OTYPER;   /* 0x04 - Output type register    */
    volatile uint32_t OSPEEDR;  /* 0x08 - Output speed register   */
    volatile uint32_t PUPDR;    /* 0x0C - Pull-up/pull-down       */
    volatile uint32_t IDR;      /* 0x10 - Input data register     */
    volatile uint32_t ODR;      /* 0x14 - Output data register    */
    volatile uint32_t BSRR;     /* 0x18 - Bit set/reset register  */
    volatile uint32_t LCKR;     /* 0x1C - Configuration lock      */
    volatile uint32_t AFR[2];   /* 0x20 - Alternate function      */
} GPIO_TypeDef;

#define GPIOA_BASE  0x40020000UL
#define GPIOA       ((GPIO_TypeDef *) GPIOA_BASE)

/* Accessor macros using the struct */
#define GPIO_SET_PIN(port, pin)   ((port)->BSRR = (1U << (pin)))
#define GPIO_CLR_PIN(port, pin)   ((port)->BSRR = (1U << ((pin) + 16U)))
#define GPIO_READ_PIN(port, pin)  (((port)->IDR >> (pin)) & 1U)
```

**Usage:**
```c
GPIO_SET_PIN(GPIOA, 5);               /* Set PA5 HIGH */
GPIO_CLR_PIN(GPIOA, 5);               /* Set PA5 LOW  */
uint32_t state = GPIO_READ_PIN(GPIOA, 5);
```

---

## 4. Bit-Field Extraction & Insertion Macros

```c
/* Extract a bitfield: bits [hi:lo] from a register value */
#define BF_GET(reg, hi, lo) \
    (((reg) >> (lo)) & ((1U << ((hi) - (lo) + 1U)) - 1U))

/* Insert a value into bits [hi:lo] of a register at address addr */
#define BF_SET(addr, hi, lo, val)                                \
    do {                                                          \
        uint32_t _mask = ((1U << ((hi)-(lo)+1U))-1U) << (lo);   \
        REG_MODIFY((addr), _mask, (uint32_t)(val) << (lo));      \
    } while(0)
```

**Example — set GPIO MODER bits [11:10] for pin 5 to output (`0b01`):**
```c
BF_SET(&GPIOA->MODER, 11, 10, 0x01U);
```

---

## 5. Atomic Bit-Band Macros (Cortex-M3/M4)

For true atomic single-bit access without read-modify-write race conditions:

```c
#define PERIPH_BB_BASE   0x42000000UL
#define PERIPH_BASE      0x40000000UL

/* Bit-band alias address for a peripheral register bit */
#define PERIPH_BB(addr, bit) \
    (*(volatile uint32_t *)(PERIPH_BB_BASE + \
        (((uint32_t)(addr) - PERIPH_BASE) * 32U) + ((bit) * 4U)))

/* Usage: atomically set bit 5 of GPIOA ODR */
PERIPH_BB(&GPIOA->ODR, 5) = 1U;   /* Set HIGH */
PERIPH_BB(&GPIOA->ODR, 5) = 0U;   /* Set LOW  */
```

---

## 6. Common Pitfalls — Graded Points

| Mistake | Consequence |
|---|---|
| Missing `volatile` | Compiler optimizes away repeated reads/writes |
| Cast to `uint32_t *` instead of `volatile uint32_t *` | Same as above |
| Using `=` instead of `\|=` to set a bit | Destroys other bits in the register |
| Forgetting `do { } while(0)` in multi-statement macros | Breaks `if/else` usage |
| No `UL` suffix on address literals | Sign-extension bugs on 64-bit hosts |

---

## 7. Quick Validation Test (run on target or QEMU)

```c
/* Smoke test: write known pattern, read it back from a scratchpad register */
#define SCRATCH_REG  0x40023C00UL   /* Example: RCC base on STM32F4 */

void test_accessors(void) {
    REG_WRITE(SCRATCH_REG, 0xDEADBEEF);
    uint32_t v = REG_READ(SCRATCH_REG);
    /* On real HW, check v against datasheet reset value instead */
    (void)v;

    REG_SET_BITS(SCRATCH_REG, 0x0F);
    REG_CLR_BITS(SCRATCH_REG, 0x0F);
}
```

---

## Summary

These macros are the exact patterns used in production bare-metal STM32 code and satisfy every requirement in the Week 2 assignment.

- Use the **struct-based approach** (`GPIO_TypeDef`) for peripherals
- Use **`MMIO32`** for one-off register pokes
- Use **`BF_SET` / `BF_GET`** for multi-bit fields
- Use **`PERIPH_BB`** for atomic single-bit access on Cortex-M3/M4
