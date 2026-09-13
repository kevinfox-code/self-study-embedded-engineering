# ESE-301 Week 06 — State Machines

## Objectives

- Express behaviour as an explicit finite state machine instead of scattered flags.
- Separate the FSM logic from the hardware it drives so it can be tested on the host.
- Exercise the same FSM against a mock HAL in tests and a host HAL in a sandbox.

## What Was Built

- [`src/led_fsm.c`](src/led_fsm.c) / [`src/led_fsm.h`](src/led_fsm.h) — the LED
  state machine, built as the `led_fsm` static library with no HAL implementation
  of its own.
- [`src/hal.h`](src/hal.h) — the HAL interface the FSM depends on. Each
  executable supplies its own implementation.
- [`sandbox/`](sandbox) — host HAL ([`hal_host.c`](sandbox/hal_host.c)) plus a
  driver program for interactive experimentation.
- [`test/`](test) — mock HAL ([`test/hal_mock.c`](test/hal_mock.c)) and
  [`test/test_led_fsm.c`](test/test_led_fsm.c), registered with CTest.

## Key Concepts

- **Depend on an interface, not an implementation.** `led_fsm` links against
  `hal.h` only; the mock and the host HAL are swapped at link time, which is what
  makes the FSM testable without hardware.
- **Explicit state beats implicit flags.** A single `state` variable and a
  transition function keep the reachable states enumerable and reviewable.
- **Transitions are events, not polling.** Each call feeds an event; the FSM
  decides whether that event causes a transition.

## Build and Run

```bash
cd ESE-301/Week06-State-Machines
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## References

- Elecia White, *Making Embedded Systems* — state machine chapter
- See [ESE-301_References.md](../ESE-301_References.md) for download links

## AI Assistance

None.
