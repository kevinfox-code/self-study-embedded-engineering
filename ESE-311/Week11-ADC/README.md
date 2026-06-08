# ADC Module

## Objectives

- Implement ADC drivers for the STM32U575 family
- Configure single-shot and continuous conversions
- Understand sampling time, resolution, and data alignment
- Use DMA for continuous sampling (optional)
- Validate readings with a simple test harness

## What Was Built

- ADC initialization and clock configuration
- Channel selection and sampling-time configuration
- Single conversion and continuous conversion routines
- Optional DMA integration example
- A small test program that logs conversion results over UART

## Key Concepts

- ADC clock sources and prescalers
- Sampling time vs input impedance and source driving
- Resolution (12/10/8/6-bit) and data alignment
- Calibration and voltage reference (VREF+ / VREF-)
- Using DMA to stream ADC samples without CPU polling

## Example Build (Local)

```bash
cd ESE-311/Week11-ADC-Driver/ADC
rm -rf build
cmake -B build
cmake --build build
```

## Build, Flash and Run

To build and flash the demo (uses the `flash` target configured for your board):

```bash
cmake --build build --target flash
```

Then open a serial terminal (115200, 8N1) to observe ADC readings printed by the test program.

## VS Code Debug

1. Install the Cortex-Debug extension.
2. Ensure `arm-none-eabi-gdb` and `openocd` are on your `PATH`.
3. Connect the board with ST-LINK.
4. Open the Run and Debug view and select the ADC debug configuration.

The debug launch uses `build/adc_demo.elf` (adjust target name if different).

## Exercises

1. Configure and read a single ADC channel (single conversion). Verify with a known voltage.
2. Switch the ADC resolution to 10-bit and compare scaling in software.
3. Implement continuous conversion mode and sample averaging to reduce noise.
4. (Advanced) Configure DMA to transfer conversions into a circular buffer and plot results.

## Verification

- Use a voltage divider or potentiometer on an analog-capable pin to exercise the ADC.
- Confirm raw counts map to expected voltages using the reference voltage and resolution.
- Add simple unit tests for scaling/math functions where practical (host-side tests).

## References

- Bare-Metal Embedded C Programming, ADC chapter
- STM32U5 Reference Manual (ADC section)
- STM32CubeU5 HAL drivers and application notes

## Notes

- Keep analog input sources low impedance or increase sampling time.
- Calibrate and verify VREF if precise voltage measurements are required.
- Update `CMakeLists.txt` or build targets if your board or demo target name differs.

---

If you'd like, I can also:

- add a small `adc_demo.c` example in this folder,
- add a CI step that builds the demo, or
- commit these README changes to a branch and open a PR.

File: ESE-311/Week11-ADC-Driver/ADC/README.md
