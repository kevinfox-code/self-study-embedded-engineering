# blinky (STM32U575)

## Setup

- [ESE-301_Environment-Setup.md](ESE-301_Environment-Setup.md): environment variables required for VS Code debug and toolchain integration on macOS, Windows, and Linux

## Build

```sh
cmake --preset Debug
cmake --build build/Debug
```

## Debug

Use the VS Code launch configuration:

- `STM32Cube: Launch ST-Link GDB Server`

If debug launch fails, verify all variables and paths from [ESE-301_Environment-Setup.md](ESE-301_Environment-Setup.md).
