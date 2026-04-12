# Environment Setup (macOS + Windows + Linux)

This project uses environment variables in VS Code debug/settings for the current GCC flow. Additional environment variables are only referenced by CMake if you switch to the optional `starm-clang.cmake` + `STARM_HYBRID` flow.

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
    - `cmake/starm-clang.cmake` (`--gcc-toolchain="$ENV{GCC_TOOLCHAIN_ROOT}/.."`)
  - Set this to the GCC toolchain `bin` directory, not the toolchain root directory itself.
  - Example:
    - macOS/Linux: `$CUBE_BUNDLE_PATH/gnu-tools-for-stm32/14.3.1+st.2/bin`
    - Windows: `%CUBE_BUNDLE_PATH%\gnu-tools-for-stm32\14.3.1+st.2\bin`

## Recommended Values

Set `CUBE_BUNDLE_PATH` to the STM32CubeIDE external-tools `tools` directory under `plugins/.../tools` (the folder that contains `STLink-gdb-server`, `STM32CubeProgrammer`, and `gnu-tools-for-stm32`), not the top-level STM32CubeIDE install root.

### macOS example

- `CUBE_BUNDLE_PATH=/Applications/STMicroelectronics/STM32Cube/STM32CubeIDE_1.18.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.mac64_2.2.0.202501201501/tools`
- `ARM_NONE_EABI_GDB=$CUBE_BUNDLE_PATH/gnu-tools-for-stm32/14.3.1+st.2/bin/arm-none-eabi-gdb`

### Windows example

- `CUBE_BUNDLE_PATH=C:\ST\STM32CubeIDE_1.18.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.win32_2.2.0.202501201501\tools`
- `ARM_NONE_EABI_GDB=%CUBE_BUNDLE_PATH%\gnu-tools-for-stm32\14.3.1+st.2\bin\arm-none-eabi-gdb.exe`

### Linux example

- `CUBE_BUNDLE_PATH=~/st/stm32cubeide_1.18.0/plugins/com.st.stm32cube.ide.mcu.externaltools.linux64_2.2.0.202501201501/tools`
- `ARM_NONE_EABI_GDB=$CUBE_BUNDLE_PATH/gnu-tools-for-stm32/14.3.1+st.2/bin/arm-none-eabi-gdb`

Adjust version folder names to match what is installed on each machine.

## Set Variables Persistently

### macOS (zsh)

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

### Linux (bash)

Append these lines to `~/.bashrc` (or `~/.bash_profile`):

```sh
export CUBE_BUNDLE_PATH="$HOME/st/stm32cubeide_1.18.0/plugins/com.st.stm32cube.ide.mcu.externaltools.linux64_2.2.0.202501201501/tools"
export ARM_NONE_EABI_GDB="$CUBE_BUNDLE_PATH/gnu-tools-for-stm32/14.3.1+st.2/bin/arm-none-eabi-gdb"
```

If using `STARM_HYBRID`, also set:

```sh
export CLANG_GCC_CMSIS_COMPILER="$CUBE_BUNDLE_PATH/st-arm-clang/18.0.1+st.4/bin"
export GCC_TOOLCHAIN_ROOT="$CUBE_BUNDLE_PATH/gnu-tools-for-stm32/14.3.1+st.2/bin"
```

Then reload your shell:

```sh
source ~/.bashrc
```

### Windows (PowerShell, user-level persistent)

Run once in PowerShell, replacing the path with your actual install location:

```powershell
$cubePath = "C:\ST\STM32CubeIDE_1.18.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.win32_2.2.0.202501201501\tools"
[Environment]::SetEnvironmentVariable("CUBE_BUNDLE_PATH", $cubePath, "User")
[Environment]::SetEnvironmentVariable("ARM_NONE_EABI_GDB", "$cubePath\gnu-tools-for-stm32\14.3.1+st.2\bin\arm-none-eabi-gdb.exe", "User")
```

If using `STARM_HYBRID`, also run:

```powershell
[Environment]::SetEnvironmentVariable("CLANG_GCC_CMSIS_COMPILER", "$cubePath\st-arm-clang\18.0.1+st.4\bin", "User")
[Environment]::SetEnvironmentVariable("GCC_TOOLCHAIN_ROOT", "$cubePath\gnu-tools-for-stm32\14.3.1+st.2\bin", "User")
```

After setting variables, restart VS Code so the integrated terminal/debugger picks up new values.

## Verify On Each Machine

### macOS

```sh
echo "$CUBE_BUNDLE_PATH"
echo "$ARM_NONE_EABI_GDB"
"$ARM_NONE_EABI_GDB" --version
ls "$CUBE_BUNDLE_PATH/STLink-gdb-server/bin/ST-LINK_gdbserver"
ls "$CUBE_BUNDLE_PATH/STM32CubeProgrammer/bin/STM32_Programmer_CLI"
```

### Linux

```sh
echo "$CUBE_BUNDLE_PATH"
echo "$ARM_NONE_EABI_GDB"
"$ARM_NONE_EABI_GDB" --version
ls "$CUBE_BUNDLE_PATH/STLink-gdb-server/bin/ST-LINK_gdbserver"
ls "$CUBE_BUNDLE_PATH/STM32CubeProgrammer/bin/STM32_Programmer_CLI"
```

### Windows (PowerShell)

```powershell
echo $env:CUBE_BUNDLE_PATH
echo $env:ARM_NONE_EABI_GDB
& $env:ARM_NONE_EABI_GDB --version
Test-Path "$env:CUBE_BUNDLE_PATH\STLink-gdb-server\bin\ST-LINK_gdbserver.exe"
Test-Path "$env:CUBE_BUNDLE_PATH\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
```

## Notes

- Your active `CMakePresets.json` uses `cmake/gcc-arm-none-eabi.cmake`, so `CLANG_GCC_CMSIS_COMPILER` and `GCC_TOOLCHAIN_ROOT` are not needed unless you intentionally switch toolchains.
- On Linux, the STM32CubeIDE installer typically places files under `~/st/stm32cubeide_<version>/`. Adjust the path to match your actual install location.
