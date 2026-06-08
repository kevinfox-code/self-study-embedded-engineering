# GPIO Input/Output Example

## Build

``` bash
rm -rf build
cmake -B build
cmake --build build
```

## Build and Flash

```bash
cmake --build build --target flash
```

## VS Code Debug

1. Install the Cortex-Debug extension.
2. Make sure `arm-none-eabi-gdb` and `openocd` are on your `PATH`.
3. Connect the board with ST-LINK.
4. Open the Run and Debug view and select `Debug Systick (OpenOCD)`.

The debug launch uses `build/systick.elf` and the same OpenOCD setup as the `flash` target.
