# ESE-311 Week07: GPIO Input & Output

## Objectives

- Understand GPIO configuration on STM32U5 (input, output, pull-up/down).
- Learn simple debouncing and edge-detection for button inputs.
- Build and run a small example blinking LED + button input.

## What Was Built

- `GpioInput-Output/gpio_example`: a minimal CMake STM32 project demonstrating GPIO input and output, debouncing, and an example main loop.

## Key Concepts

- GPIO modes: input, output, analog, alternate function.
- Pull resistors and signal stability.
- Debounce strategies (software tick-based debounce).
- Basic `volatile` register access and CMSIS startup flow.

## References

- Example project: [GpioInput-Output/gpio_example](GpioInput-Output/gpio_example/README.md#L1)
- STM32U5 reference manuals and CMSIS device headers in the `CMSIS` folder.

## Build / Run (host)

1. Create a `build` directory and run CMake:

```bash
cd ESE-311/Week07/GpioInput-Output/gpio_example
mkdir -p build && cd build
cmake ..
make
```

2. Use your usual flashing tools (STM32CubeProgrammer / OpenOCD) to program the device.

## AI Assistance

Copilot-assisted: README scaffolded with AI and reviewed by maintainer.
