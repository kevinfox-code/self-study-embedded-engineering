# ESE-301 Week 05 — Interrupt Design Patterns

## Objectives

- Decouple interrupt producers from consumers with a publish/subscribe bus.
- Run multi-rate periodic work from a single tick without an RTOS.
- Make the interrupt-driven design testable on the host.

## What Was Built

- [`ESE-301_hw5`](ESE-301_hw5) — a pub/sub bus wired to a tick scheduler and a
  sensor module:
  - [`Core/Src/pubsub.c`](ESE-301_hw5/Core/Src/pubsub.c) — topic-indexed
    subscriber table with `PUBSUB_Subscribe` / `PUBSUB_Publish`
  - [`Core/Src/scheduler.c`](ESE-301_hw5/Core/Src/scheduler.c) — tick counter
    publishing 1 ms, 100 ms, and 1 s topics
  - [`Core/Src/sensor.c`](ESE-301_hw5/Core/Src/sensor.c) — temperature source
    publishing to its own topic
  - [`Tests/test_pubsub.c`](ESE-301_hw5/Tests/test_pubsub.c) — host-side tests
- [`ESE-301_hw5/README.md`](ESE-301_hw5/README.md) — the UML class diagram for
  the bus.

## Key Concepts

- **Publish/subscribe decouples rate from handler.** The scheduler does not know
  who consumes a tick, so adding a consumer does not touch the ISR path.
- **Topic tables are bounded.** Subscriber slots are fixed at compile time, and
  `PUBSUB_Subscribe` returns `PUBSUB_ERR_FULL` rather than allocating.
- **A union payload keeps the message flat.** One message type carries a tick,
  a button state, or a temperature without dynamic dispatch.
- **Multi-rate from one tick.** Dividing a single 1 ms tick avoids a second
  timer and keeps every rate phase-locked.

## Build and Run (host tests)

```bash
cd ESE-301/Week05-Interrupt-Design-Patterns/ESE-301_hw5/Tests
make
./test_pubsub
```

## References

- Elecia White, *Making Embedded Systems* — interrupt and task-management chapters
- See [ESE-301_References.md](../ESE-301_References.md) for download links

## AI Assistance

None.
