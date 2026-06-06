---
applyTo: "**/*.c,**/*.h,**/*.cpp,**/*.hpp,**/*.ld,**/Makefile,**/CMakeLists.txt,**/*.s"
---

# STM32 Target-Specific Instructions

> This file documents the exact MCU, clock configuration, peripheral assignments, and build setup for this project. It is loaded whenever you edit firmware source, build files, or linker scripts. Keep it in sync with the actual code in `Src/` and the linker script.

> Maintenance rule: whenever you change firmware that affects pin use, peripheral ownership, clocks, interrupts, DMA, or linker placement, update this file before committing. The peripheral allocation table is the source of truth and must stay aligned with the code.

## MCU Details

- **Part**: STM32U575ZITxQ (NUCLEO-U575ZI-Q board, STLINK-V3E onboard debugger)
- **Core**: ARM Cortex-M33 with single-precision FPU, DSP extensions, and TrustZone-M
- **Max clock**: 160 MHz. **This project runs on HSI16 (16 MHz) with no PLL** — SYSCLK = 16 MHz.
- **Flash**: 2 MB (dual-bank), **RAM**: 784 KB (768 KB SRAM1/2/3 at `0x20000000` + 16 KB SRAM4 at `0x28000000`)
- **No HAL**: this is a bare-metal CMSIS register-level project. Peripherals are driven directly via CMSIS device headers (`CMSIS/Device/ST/STM32U5xx/`), not STM32Cube HAL.
- **TrustZone**: registers are accessed through the **non-secure aliases** (`RCC_NS`, `GPIOA_NS`, `USART1_NS`, `ADC1_NS`, `PWR_NS`, …). Always use the `_NS` aliases when adding peripheral code unless you have a specific reason to touch the secure alias.
- **Toolchain**: `arm-none-eabi-gcc` (Cortex-M33, `-mcpu=cortex-m33 -mthumb`), `--specs=nosys.specs`, Newlib.

## Clock Configuration

```
HSI16 (16 MHz) → SYSCLK = 16 MHz   (no PLL configured)
  → AHB    = 16 MHz
  → APB1   = 16 MHz   (TIM2 etc.)
  → APB2   = 16 MHz   (USART1)
  → ADC kernel clock = HSI16 (CCIPR3.ADCDACSEL), then /8 prescaler → 2 MHz
  → SysTick = 16 MHz core clock (16000 cycles per millisecond)
```

Because there is no PLL and all buses are at 16 MHz, peripheral timing math is straightforward (no APB ×2 timer-clock quirk to account for at the current prescalers). If you introduce a PLL or change APB prescalers, recompute every dependent value: UART BRR, TIM PSC/ARR, ADC prescaler, and the SysTick `ONE_MSEC_LOAD` constant in [systick.c](PROJECT_NAME/Src/systick.c).

## Peripheral Allocation

| Peripheral | Usage              | Pins                          | Config Notes |
|------------|--------------------|-------------------------------|--------------|
| USART1     | Debug console (STLINK-V3E VCP) | PA9 (TX) / PA10 (RX), AF7 | 115200 8N1, blocking; kernel clock = APB2 16 MHz; BRR = fCK/baud (OVER16). Optional interrupt-driven ring-buffer mode via `FEATURE_UART_DMA`. |
| GPIOC      | Green LED (LD1)    | PC7                           | Push-pull, medium speed, no pull |
| GPIOB      | Blue LED (LD2)     | PB7                           | Push-pull, medium speed, no pull |
| GPIOG      | Red LED (LD3)      | PG2                           | Push-pull, medium speed, no pull |
| GPIOC      | User button (B1)   | PC13                          | Input, pull-down, treated active-high in code |
| ADC1       | Analog input       | PA3 (ADC1_IN8, A0 on Zio)     | 12-bit, continuous + OVRMOD, /8 prescaler (2 MHz), 19.5-cycle sample time; needs `PWR_NS->SVMCR ASV` set |
| TIM2       | 1 Hz software tick | —                             | APB1; PSC = 4000-1, ARR = 1000-1 → 16 MHz/4000/1000 = 1 Hz; polled via `UIF` |
| SysTick    | Blocking ms delay  | —                             | Core clock 16 MHz, polls COUNTFLAG |

> All GPIO/peripheral clocks are enabled on **`RCC_NS->AHB2ENR1`** (GPIO, ADC12), **`RCC_NS->APB2ENR`** (USART1), **`RCC_NS->APB1ENR1`** (TIM2), and **`RCC_NS->AHB3ENR`** (PWR). Match this register naming when adding peripherals.

## DMA / Interrupt-Driven I/O

This project does **not** use the STM32U5 GPDMA controller. The `FEATURE_UART_DMA` toggle (despite the name) enables an **interrupt-driven** USART1 path backed by software ring buffers ([ringbuffer.c](PROJECT_NAME/Src/ringbuffer.c)), not hardware DMA:

```
USART1_IRQn (NVIC priority 1):
  RXNE → push byte into rxRingBuffer
  TXE  → pop byte from txRingBuffer (TX interrupt disabled when buffer drains)
```

If you add real GPDMA channels later, document the channel→request-line mapping here. Two peripherals must not share the same GPDMA channel.

## Linker Script Notes

See [STM32U575ZITXQ_FLASH.ld](PROJECT_NAME/STM32U575ZITXQ_FLASH.ld).

- **FLASH**: 2048 KB at `0x08000000`
- **RAM**: 768 KB at `0x20000000` (SRAM1/2/3, contiguous) — primary RAM region (`_estack = ORIGIN(RAM) + LENGTH(RAM)`)
- **SRAM4**: 16 KB at `0x28000000` — separate region (low-power domain), not currently used by any section
- Stack size: `_Min_Stack_Size = 0x400;` (1 KB)
- Heap size: `_Min_Heap_Size = 0x200;` (512 B) — used by Newlib `_sbrk` via [sysmem.c](PROJECT_NAME/Src/sysmem.c)
- `libc.a`, `libm.a`, `libgcc.a` contents are placed in `/DISCARD/` — keep an eye on this if you start pulling in libc functions that fail to link.

## Known Errata

Consult the **STM32U575/585 errata sheet (ES0499)** for your exact silicon revision before relying on a peripheral edge case. Verify the revision printed on your part rather than assuming.

- Check ES0499 for ADC, USART, and GPDMA limitations relevant to the peripherals above.
- The ADC analog supply must be validated (`PWR_NS->SVMCR |= PWR_SVMCR_ASV`) before calibration, or `ADCAL` never completes — already handled in [adc.c](PROJECT_NAME/Src/adc.c); preserve this when refactoring.
- Errata and documentation index: https://www.st.com/en/microcontrollers-microprocessors/stm32u575zi.html#documentation

## Build Commands

CMake-based, cross-compiling with `arm-none-eabi-gcc`. See [CMakeLists.txt](PROJECT_NAME/CMakeLists.txt).

```bash
# Configure (Debug is the default build type)
cmake -B build -S PROJECT_NAME -DCMAKE_BUILD_TYPE=Debug

# Build (produces bin/, hex/, map/ artifacts in build/)
cmake --build build -j

# Build with feature toggles
cmake -B build -S PROJECT_NAME -DDEBUG_ENABLED=ON -DFEATURE_UART_DMA=ON
cmake --build build -j

# Flash via OpenOCD (STLINK-V3E)
cmake --build build --target flash
# equivalently:
openocd -f interface/stlink.cfg -f target/stm32u5x.cfg \
        -c "program build/bin/project_name.elf verify reset exit"

# Flash a raw binary via st-flash
st-flash write build/bin/project_name.bin 0x08000000

# Strict static-analysis build
cmake --build build --target lint
```

## Debugging Tips

- printf-style output goes over **USART1 → STLINK-V3E Virtual COM Port** at 115200 8N1 (`__io_putchar` is routed to `uart_send_char`). Open the VCP in a serial terminal.
- SWO/ITM trace is available on the Cortex-M33 if you configure it:
  ```c
  // ITM_SendChar is available via CMSIS when SWO is configured
  int _write(int file, char *ptr, int len) {
      for (int i = 0; i < len; i++) ITM_SendChar(*ptr++);
      return len;
  }
  ```
- `arm-none-eabi-objdump -d -C build/bin/project_name.elf` — inspect generated assembly for performance-critical sections.
- `arm-none-eabi-size build/bin/project_name.elf` — monitor flash/RAM usage after each change.
- `arm-none-eabi-nm --size-sort build/bin/project_name.elf` — find the largest symbols.
