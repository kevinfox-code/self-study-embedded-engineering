---
mode: agent
description: Detect and run the correct build system for the current or specified directory
---

# Build Check

Detect and run the right build system for this embedded engineering repo.

## What to ask me first (if not already provided)

- Which directory should I build? (current, or give a path like `ECE-452/Week02`)

## Build system detection rules

Check for files in this priority order:

1. **Makefile present** → ECE-452 motor math or similar native build
   ```bash
   make clean && make && make run
   ```
   - Flag any compiler warnings (`-Wall -Wextra` is standard here)
   - Report pass/fail and binary name

2. **CMakeLists.txt present** → ESE-301 sandbox or test suite
   ```bash
   mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug && make && ctest --output-on-failure
   ```
   - Report test results individually
   - Flag any failing tests with the test name and output

3. **`.project` file present** (STM32CubeIDE) → embedded hardware project
   - Do **not** attempt to build from the command line
   - Report: "This is a STM32CubeIDE project — open in STM32CubeIDE to build and flash"
   - List the source files in `Core/Src/` so I can review them

4. **None of the above** → no build system
   - List all `.c` and `.h` files present
   - Ask me whether to create a Makefile or CMakeLists.txt

## After a successful build

- Confirm the output binary name and location
- Check for any staged build artifacts (`.o`, `.elf`, `.bin`) and remind me not to commit them
- Suggest running `make clean` or deleting `build/` before committing

## After a failed build

- Show the full compiler error output
- Identify the file and line number of the first error
- Suggest a fix if the cause is clear (missing include, type mismatch, undefined symbol, etc.)
- Ask before making any code changes
