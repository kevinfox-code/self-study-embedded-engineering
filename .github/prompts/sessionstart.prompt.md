---
mode: ask
description: Orient Copilot to this repo at the start of a session
---

# Session Start — Self-Study Embedded Engineering

You are helping me work in my self-study embedded engineering repository. Read the files below to orient yourself before I give you a task.

## Files to read first

- `#file:.github/copilot-instructions.md` — full project conventions (naming, branches, commits, PRs, build systems)
- `#file:CONTRIBUTING.md` — AI transparency and licensing policy
- `#file:README.md` — project overview

## What this repo contains

Three parallel embedded engineering courses, each progressing week by week:

- **ESE-301** (`ESE-301/`) — Making Embedded Systems: design patterns, FSMs, HAL abstraction
- **ESE-311** (`ESE-311/`) — Bare-Metal ARM C: registers, linker scripts, startup code, CMSIS
- **ECE-452** (`ECE-452/`) — Motor Drives / FOC: Clarke/Park transforms, SVPWM, sensorless FOC on STM32

## Ground rules for this session

1. **Naming** — follow the conventions in `copilot-instructions.md` exactly. Course dirs are `UPPER-###`, week dirs are `Week{NN}` zero-padded, source files are `snake_case.c`.
2. **Branches** — new work goes on `{course-lowercase}-week{NN}` branches, never directly on `main`.
3. **Commits** — imperative, descriptive, ≤72 chars. Append AI disclosure to the body when you helped substantially.
4. **PRs** — always use `.github/pull_request_template.md`. Every checkbox must be filled.
5. **Build systems** — ECE-452 math uses `make`, ESE-301 tests use `cmake + ctest`, STM32CubeIDE projects open in the IDE.
6. **No build artifacts** — never stage `*.o`, `*.elf`, `*.bin`, `Debug/`, `Release/`.
7. **AI transparency** — I must disclose your assistance in PR descriptions and commit bodies per `CONTRIBUTING.md`.
8. **CI checks** — this repo includes an `Agentic CI` workflow that runs on PRs and pushes to `main`; ensure commits pass local build/tests before opening a PR.

## Useful prompt files

These are available in `.github/prompts/` — reference them with `#file:` or use the Copilot prompt picker:

| Prompt | Purpose |
|--------|---------|
| `new-week.prompt.md` | Scaffold a new week folder, branch, and README |
| `build-check.prompt.md` | Detect and run the right build system |
| `pr-prep.prompt.md` | Draft a complete PR description from recent commits |

## Ready

Once you have read the orientation files, confirm with:
> "Ready — I've read the project conventions. What are we working on today?"

Then wait for my task.
