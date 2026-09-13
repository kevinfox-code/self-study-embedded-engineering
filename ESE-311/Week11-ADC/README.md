# ESE-311 Week 11 — ADC

## Objectives

- Configure the STM32U575 ADC at the register level, without the HAL.
- Understand the sequence the U5 requires before a conversion will ever complete:
  clock enable, analog supply valid, calibration, then enable.
- Select a channel and set an explicit sample time rather than relying on reset defaults.
- Read conversions in a loop and report them over UART.

## What Was Built

- [`ADC/Src/adc.c`](ADC/Src/adc.c) — ADC1 driver. Enables the GPIOA and ADC12 clocks,
  asserts `PWR_SVMCR_ASV` so the analog supply is valid, runs `ADCAL`, selects
  ADC1_IN8 (PA3, the `AO` pin on the Zio connector), programs a 19.5-cycle
  sample time via `SMPR1.SMP8`, and runs in continuous mode with `OVRMOD` set so
  the latest sample always wins.
- [`ADC/Inc/adc.h`](ADC/Inc/adc.h) — `ADC_Init`, `Start_Conversion`, `ADC_Read`.
- [`ADC/Src/main.c`](ADC/Src/main.c) — starts a conversion and prints each raw count
  over UART every 250 ms while toggling the green LED.
- UART, SysTick, GPIO, and debug modules carried forward from Weeks 7–10.

## Key Concepts

- **The ASV gate.** On STM32U5 the analog isolation switch stays open until
  `PWR_SVMCR.ASV` is set. Without it the ADC has no analog power and `ADCAL`
  never completes — the single most common reason a U5 ADC port from an F4
  appears to hang.
- **Calibration ordering.** `ADCAL` must run with `ADEN = 0`; enabling the ADC
  first silently skips calibration.
- **Sample time vs source impedance.** The sampling capacitor has to charge
  through the source. A high-impedance source needs a longer `SMP` setting, or
  readings sag toward the previous conversion.
- **Continuous vs single conversion.** With `CONT` set the ADC free-runs, so
  `ADC_Read()` returns whatever is in `DR` rather than triggering a conversion.
  `OVRMOD` decides whether an unread sample is preserved or overwritten.
- **Resolution.** `CFGR1.RES` is two bits wide on this part and is left at its
  reset value here; changing it rescales every raw count in software.

## Build and Run

```bash
cd ESE-311/Week11-ADC/ADC
cmake -B build
cmake --build build
cmake --build build --target flash
```

Open a serial terminal on the ST-LINK virtual COM port at 115200 8N1 to see the
conversion values. Drive PA3 from a potentiometer between 3V3 and GND to sweep
the reading.

## VS Code Debug

Requires the Cortex-Debug extension, with `arm-none-eabi-gdb` and `openocd` on
`PATH`. Open the `ADC/` folder as the workspace and launch **Debug (OpenOCD)**,
which builds `build/adc.elf` via the `cmake: build` task.

## References

- Israel Gbati, *Bare-Metal Embedded C Programming* — ADC chapter
- RM0456 §ADC, and the `PWR_SVMCR` description in §PWR
- See [ESE-311_References.md](../ESE-311_References.md) for download links

## AI Assistance

Claude Code assisted with the STM32U5 ADC power and calibration sequence and
with drafting this README. Reviewed and edited by the maintainer.
