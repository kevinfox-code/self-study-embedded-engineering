

## Example: GPIO Peripheral Structure

Volitile is used to tell the compiler that the value of the variable can change at any time without any action being taken by the code the compiler finds nearby. This is important for hardware registers, as their values can change due to external events (like hardware signals) or internal processes (like timers).

Uint32_t is a standard integer type defined in the C standard library (specifically in stdint.h) that represents an unsigned 32-bit integer. This means it can hold values from 0 to 4,294,967,295 (4 bytes) keeping alingnment in mind.

``` c
typedef struct {
    volitile uint32_t MODER; // Offset 0x00
    volitile uint32_t OTYPER; // Offset 0x04
    volitile uint32_t OSPEEDR; // Offset 0x08
    volitile uint32_t PUPDR; // Offset 0x0C
    volitile uint32_t IDR; // Offset 0x10
    volitile uint32_t ODR; // Offset 0x14
    volitile uint32_t BSRR; // Offset 0x18
    volitile uint32_t LCKR; // Offset 0x1C
    volitile uint32_t AFR[2]; // Offset 0x20,
} GPIO_TypeDef;
```
