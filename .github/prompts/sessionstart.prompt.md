---
agent: ask
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

## When creating a new week from previous work

When the user asks to start a new week and wants to reuse the previous week's code as a starting point, follow these steps and confirm with the user before making changes:

- Identify the target course (one of `ESE-301`, `ESE-311`, `ECE-452`).
- Find the most recent `Week{NN}` folder for that course (highest NN present).
- Copy the entire folder contents of that week into a new folder named `Week{NN+1}` (zero-padded to two digits).
- Inside the new `Week{NN+1}` folder, rename the top-level code subfolder (if present) to reflect the new topic name the user provided. Update the `README.md` in the new week to describe the new topic and include an **AI Assistance** section noting Copilot's role if applicable.
- Preserve vendor folders like `Drivers/` and `Core/` where appropriate, but do not copy build artifacts or derived files (e.g., `build/`, `*.o`, `*.elf`, `Debug/`, `Release/`). Remove or clean those directories in the copied folder.
- Ensure all filenames, folder names, and branch names follow the conventions in `.github/copilot-instructions.md` (course dir, `Week{NN}`, branch `{course-lowercase}-week{NN}`).
- If any files must not be duplicated (license, large vendor binaries), prompt the user before copying.

Before performing the copy, ask the user to confirm these details:
- Which course (ESE-301 | ESE-311 | ECE-452)
- Which existing week to copy (source `Week{NN}`)
- The new week number (target `Week{NN+1}`) or allow auto-increment
- The desired new topic name (used to rename the code folder and update README)

If the user confirms, perform the filesystem copy and renaming, then:
- Remove common build artifacts from the new folder (`build/`, `*.o`, `*.elf`, etc.).
- Update the new week's `README.md` with the new topic title, objectives, and an **AI Assistance** statement (e.g., "Copilot assisted with scaffolding; maintainer validated and tested.").
- Create a new branch following the branch naming conventions ready for the user's edits.

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

Then wait for the user's task and confirm copy details if they want to start a new week from previous code.
