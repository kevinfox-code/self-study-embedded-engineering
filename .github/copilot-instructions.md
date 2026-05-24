# GitHub Copilot Instructions — Self-Study Embedded Engineering

## Project Overview

Three-course self-study embedded systems curriculum. Work is organized by course and week. Each week produces source files, a README, and (where applicable) a Makefile or CMakeLists.txt for standalone builds.

| Directory | Course | Focus |
|-----------|--------|-------|
| `ESE-301/` | Making Embedded Systems | Design patterns, FSM, HAL abstraction |
| `ESE-311/` | Bare-Metal ARM C Programming | Registers, linker, startup, CMSIS |
| `ECE-452/` | Electric Motor Drives / FOC | Clarke/Park, SVPWM, sensorless FOC |

## Naming Conventions

Always follow these. They are enforced by the PR checklist.

| Item | Convention | Example |
|------|-----------|---------|
| Course directories | `UPPER-###` | `ESE-301`, `ECE-452` |
| Week directories | `Week{NN}` zero-padded | `Week07`, `Week12` |
| Document files | `COURSEID_Descriptive-Name.ext` | `ESE-311_Startup-Code.md` |
| Root/shared files | lowercase kebab-case | `embedded-engineering-course-schedule.ics` |
| C source files | lowercase snake_case | `clarke.c`, `park_transform.h` |
| No Unicode punctuation in filenames | ASCII-safe only | — |

## Branch Naming

`{course-lowercase}-week{NN}` — always zero-padded to two digits.

```
ese311-week07    ese301-week06    ece452-week04
```

Never commit directly to `main`. Create the branch before starting a new week.

## Commit Messages

- Imperative mood, present tense, ≤72 chars
- Describe what changed and why — not how
- Append AI disclosure when AI substantially contributed

```
Add Clarke and Park Transform Implementation and SVPWM Module
ESE-301 Week06: LED state machine pattern (Making Embedded Systems p. 279)
Add GPIO interrupt handler and debounce logic

AI-assisted: implementation drafted with Copilot, validated and tested by maintainer.
```

Do not include AI tool names (Copilot, Claude, GPT) in the first commit line — put disclosure in the body.

## Pull Requests

- Always use `.github/pull_request_template.md` — fill every checkbox
- **AI assistance disclosure is required** when applicable (see CONTRIBUTING.md)
- Licensing checklist must be complete before merge
- Target: `main`

## Build Systems

### ECE-452 Motor Math — Makefile/gcc
```bash
cd ECE-452/Week{NN}
make        # build
make run    # build and execute tests
make clean  # remove output
```
Flags: `-std=c11 -Wall -Wextra -O2 -lm`

### ESE-301 Tests/Sandbox — CMake
```bash
cd ESE-301/Week{NN}
mkdir -p build && cd build && cmake .. && make && ctest
```

### ESE-311 / STM32CubeIDE Embedded Projects
- Do **not** modify files under `Drivers/` (vendor HAL/CMSIS)
- Do **not** touch `.project`, `.cproject`, or linker scripts unless intentional
- These projects require STM32CubeIDE to build and flash

## Code Style

- C standard: C11
- Style: follow the surrounding file's conventions (brace placement, indentation)
- Prefer explicit register-level code in ESE-311; prefer HAL abstractions in ESE-301
- No dynamic memory allocation in bare-metal code (no malloc/free)
- Use fixed-width types (`uint32_t`, `int16_t`) for hardware-facing code

## Do Not Generate or Commit

- Compiled artifacts: `*.o`, `*.elf`, `*.bin`, `*.hex`, `*.map`, `*.d`
- `Debug/` or `Release/` directories
- PDF copies of textbooks or paywalled content
- `.DS_Store` or other OS metadata files
- Unreviewed AI-generated boilerplate without human validation

## AI Transparency Policy

This project requires AI transparency per `NOTICE.md` and `CONTRIBUTING.md`:

- Disclose AI assistance in the PR description (required field in PR template)
- Add disclosure to the commit message body when AI substantially drafted content
- Validate all AI-generated code before committing — do not commit unreviewed output
- Do not embed proprietary content or reproduce copyrighted textbook material

## Week README Template

Every new week folder must include a `README.md`. Use this structure:

```markdown
# {COURSE} Week{NN}: {Topic Title}

## Objectives

- ...

## What Was Built

- ...

## Key Concepts

- ...

## References

- {Textbook}, Ch. {N}
- {any datasheets, application notes, or online references}

## AI Assistance

{None | Brief description of scope: e.g., "Copilot used for initial FSM scaffold, reviewed and corrected by maintainer."}
```

## Useful Prompt Files

Reusable agent prompts are in `.github/prompts/`. Open them in Copilot Chat with the `#` file reference or the prompt picker:

- `.github/prompts/new-week.prompt.md` — scaffold a new week folder
- `.github/prompts/build-check.prompt.md` — detect and run the correct build system
- `.github/prompts/pr-prep.prompt.md` — prepare a PR description from recent commits
