# ESE-301 Week 03 — Hardware and Debugging

## Objectives

- Read a schematic and a datasheet well enough to plan a driver before writing it.
- Practise the debugging tools available on a host before they are needed on target.
- Work through a design pattern on the host, where the edit-run loop is fast.

## What Was Built

- [`main.c`](main.c) — a host-side command-response test harness built around a
  function-pointer dispatch table: a sentinel-terminated array of
  `commandStruct` entries mapping a command name to a handler (`version`,
  flash test, LED blink, `help`). Built with CMake as the `CommandPattern`
  executable. Running on the host keeps the focus on the dispatch design rather
  than on flashing.

## Key Concepts

- **Table-driven dispatch.** A command table replaces a growing `if`/`strcmp`
  chain and keeps the name, the handler, and the help text next to each other,
  so adding a command touches one place.
- **Sentinel termination.** The table ends in a null entry rather than carrying a
  separate length, which keeps the table and its count from drifting apart.
- **Breakpoint intrinsics.** `BKPT()` wraps `__builtin_debugtrap` where the
  compiler has it and falls back to `__builtin_trap`, giving the same
  stop-in-debugger behaviour across toolchains.
- **Debug on the host first.** A design that is wrong on the host is wrong on
  target too, and the host tells you faster.

## Build and Run

```bash
cd ESE-301/Week03-Hardware-and-Debugging
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/CommandPattern
```

VS Code: run the **Debug CommandPattern (CodeLLDB)** launch configuration, which
builds via the `Build (Debug) Compile` task.

## References

- Elecia White, *Making Embedded Systems* — hardware and debugging chapters
- See [ESE-301_References.md](../ESE-301_References.md) for download links

## AI Assistance

None.
