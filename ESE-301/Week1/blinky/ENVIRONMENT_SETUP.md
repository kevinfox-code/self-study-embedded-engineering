# Environment Setup (macOS + Windows)

This project references environment variables in VS Code debug settings and toolchain CMake files.

## Variables Used In This Project

### Required For Current VS Code Debug + GCC Flow

- `CUBE_BUNDLE_PATH`
  - Used by:
    - `.vscode/launch.json` (ST-Link GDB server and STM32CubeProgrammer CLI)
    - `.vscode/settings.json` (`--query-driver` for arm-none-eabi toolchain binaries)

- `ARM_NONE_EABI_GDB`
  - Used by:
    - `.vscode/launch.json` (`gdb` executable path)

### Optional (Only If You Switch To `starm-clang.cmake` With `STARM_HYBRID`)

- `CLANG_GCC_CMSIS_COMPILER`
  - Used by:
    - `cmake/starm-clang.cmake` (`multilib.gnu_tools_for_stm32.yaml`)

- `GCC_TOOLCHAIN_ROOT`
  - Used by:
    - `cmake/starm-clang.cmake` (`--gcc-toolchain` root)

## Recommended Values

Use your STM32CubeIDE bundle install root as `CUBE_BUNDLE_PATH`.

### macOS example

- `CUBE_BUNDLE_PATH=/Applications/STMicroelectronics/STM32Cube/STM32CubeIDE_1.18.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.mac64_2.2.0.202501201501/tools`
- `ARM_NONE_EABI_GDB=$CUBE_BUNDLE_PATH/gnu-tools-for-stm32/14.3.1+st.2/bin/arm-none-eabi-gdb`

### Windows example

- `CUBE_BUNDLE_PATH=C:\ST\STM32CubeIDE_1.18.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.win32_2.2.0.202501201501\tools`
- `ARM_NONE_EABI_GDB=%CUBE_BUNDLE_PATH%\gnu-tools-for-stm32\14.3.1+st.2\bin\arm-none-eabi-gdb.exe`

Adjust version folder names to match what is installed on each machine.

## Set Variables Persistently

## macOS (zsh)

Append these lines to `~/.zshrc`:

```sh
export CUBE_BUNDLE_PATH="/Applications/STMicroelectronics/STM32Cube/STM32CubeIDE_1.18.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.mac64_2.2.0.202501201501/tools"
export ARM_NONE_EABI_GDB="$CUBE_BUNDLE_PATH/gnu-tools-for-stm32/14.3.1+st.2/bin/arm-none-eabi-gdb"
```

If using `STARM_HYBRID`, also set:

```sh
export CLANG_GCC_CMSIS_COMPILER="$CUBE_BUNDLE_PATH/st-arm-clang/18.0.1+st.4/bin"
export GCC_TOOLCHAIN_ROOT="$CUBE_BUNDLE_PATH/gnu-tools-for-stm32/14.3.1+st.2/bin"
```

Then reload your shell:

```sh
source ~/.zshrc
```

## Windows (PowerShell, user-level persistent)

Run once in PowerShell:

```powershell
[Environment]::SetEnvironmentVariable("CUBE_BUNDLE_PATH", "C:\ST\STM32CubeIDE_1.18.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.win32_2.2.0.202501201501\tools", "User")
[Environment]::SetEnvironmentVariable("ARM_NONE_EABI_GDB", "%CUBE_BUNDLE_PATH%\gnu-tools-for-stm32\14.3.1+st.2\bin\arm-none-eabi-gdb.exe", "User")
```

If using `STARM_HYBRID`, also run:

```powershell
[Environment]::SetEnvironmentVariable("CLANG_GCC_CMSIS_COMPILER", "%CUBE_BUNDLE_PATH%\st-arm-clang\18.0.1+st.4\bin", "User")
[Environment]::SetEnvironmentVariable("GCC_TOOLCHAIN_ROOT", "%CUBE_BUNDLE_PATH%\gnu-tools-for-stm32\14.3.1+st.2\bin", "User")
```

After setting variables, restart VS Code so the integrated terminal/debugger picks up new values.

## Verify On Each Machine

## macOS

```sh
echo "$CUBE_BUNDLE_PATH"
echo "$ARM_NONE_EABI_GDB"
"$ARM_NONE_EABI_GDB" --version
ls "$CUBE_BUNDLE_PATH/STLink-gdb-server/bin/ST-LINK_gdbserver"
ls "$CUBE_BUNDLE_PATH/STM32CubeProgrammer/bin/STM32_Programmer_CLI"
```

## Windows (PowerShell)

```powershell
echo $env:CUBE_BUNDLE_PATH
echo $env:ARM_NONE_EABI_GDB
& $env:ARM_NONE_EABI_GDB --version
Test-Path "$env:CUBE_BUNDLE_PATH\STLink-gdb-server\bin\ST-LINK_gdbserver.exe"
Test-Path "$env:CUBE_BUNDLE_PATH\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
```

## Notes

- Your active `CMakePresets.json` uses `cmake/gcc-arm-none-eabi.cmake`, so `CLANG_GCC_CMSIS_COMPILER` and `GCC_TOOLCHAIN_ROOT` are not needed unless you intentionally switch toolchains.
- If your Windows user-level variables do not expand `%CUBE_BUNDLE_PATH%` inside another variable as expected, set `ARM_NONE_EABI_GDB` to a fully expanded absolute path.
