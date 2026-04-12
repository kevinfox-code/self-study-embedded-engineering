# blinky (STM32U575)

## Setup

- [ENVIRONMENT_SETUP.md](ENVIRONMENT_SETUP.md): environment variables required for VS Code debug and toolchain integration on macOS and Windows

## Build

```sh
cmake --preset Debug
cmake --build build/Debug
```

## Debug

Use the VS Code launch configuration:

- `STM32Cube: Launch ST-Link GDB Server`

If debug launch fails, verify all variables and paths from [ENVIRONMENT_SETUP.md](ENVIRONMENT_SETUP.md).
