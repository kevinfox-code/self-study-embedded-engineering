---
name: scaffold-driver
description: Generate a complete STM32 HAL peripheral driver from a specification
---

Generate a complete peripheral driver based on the following specification. The driver must follow all project coding standards (see `.github/copilot-instructions.md` and `.github/instructions/c-firmware.instructions.md`).

## Input (provide these details)

- **Peripheral**: (e.g., USART2, SPI1, I2C1, TIM2, ADC1)
- **Purpose**: (e.g., "Debug console UART", "IMU SPI interface", "Motor PWM output")
- **Pins**: (e.g., PA2/PA3)
- **Configuration**: (e.g., 115200 baud 8N1, 10MHz CPOL=0 CPHA=0, 20kHz PWM)
- **DMA**: Yes/No — if yes, specify stream/channel
- **Interrupt**: Yes/No — if yes, specify which interrupt(s)

## Output Structure

Generate the following files:

### 1. `<peripheral>.h` — Public header

- Include guards with `extern "C"` wrappers
- Doxygen module description
- Public type definitions (status enums, config structs)
- Public function declarations with Doxygen comments:
  - `<peripheral>_init()` — Initialize peripheral and GPIO
  - `<peripheral>_deinit()` — De-initialize and release resources
  - Operational functions specific to the peripheral type
  - Error/status query function

### 2. `<peripheral>.c` — Implementation

- Static peripheral handle (`static <Handle>_HandleTypeDef h<periph>;`)
- Static buffers with named sizes
- `<peripheral>_init()`:
  - Enable GPIO and peripheral clocks (`__HAL_RCC_*_CLK_ENABLE()`)
  - Configure GPIO alternate function
  - Configure peripheral parameters
  - Configure DMA if applicable
  - Configure NVIC if applicable
  - Call `HAL_*_Init()`
  - Check return value
- Operational functions with HAL return value checking
- ISR handler (if interrupt-driven): short, flag-based
- HAL callbacks (if DMA/interrupt): `HAL_*_CpltCallback`, `HAL_*_ErrorCallback`
- Static helper functions for internal logic

### 3. Verification checklist

After generating, confirm:
- [ ] All HAL return values checked
- [ ] All buffers statically allocated with named sizes
- [ ] `volatile` on ISR-shared variables
- [ ] GPIO clock enabled before GPIO init
- [ ] Peripheral clock enabled before peripheral init
- [ ] DMA stream/channel matches Reference Manual mapping
- [ ] NVIC priority set appropriately (lower numeric = higher priority)
- [ ] No dynamic memory allocation
- [ ] Doxygen on all public functions
