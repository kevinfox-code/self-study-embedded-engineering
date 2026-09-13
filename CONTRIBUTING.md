# Contributing

## Purpose

This repository documents a self-study embedded engineering curriculum and the
project artifacts that come out of it. Contributions should keep it readable as a
portfolio: accurate, consistently named, and free of stale scaffolding.

## Repository Conventions

These are enforced by review, not by tooling. They apply to original content only —
vendor and generated code (CMSIS, STM32 HAL, CubeMX output) is left as-is.

- **Course directories**: `AAA-###_Subject-Name` — the course identifier followed
  by a short subject name, e.g. `ESE-311_Bare-Metal-C`. The subject name names
  the engineering topic, not the textbook, so it survives a change of text.
- **Week directories**: `Week##-Topic-Name` — `Week12-SPI`
- **Document files**: `COURSEID_Descriptive-Name.ext` — `ESE-311_References.md`
- **Project directories**: name after what they build, matching the CMake target
  where practical
- **Filenames**: ASCII-safe; no Unicode punctuation
- **Part numbers** in filenames and identifiers must match the manufacturer's
  spelling — `adxl345`, not `adx1345`

## Documentation Standards

Every week folder has a `README.md` opening with
`# COURSE-ID Week NN — Topic`, and covering:

1. **Objectives** — what the week set out to teach
2. **What Was Built** — the actual artifacts, linked
3. **Key Concepts** — the specific, non-obvious things learned, not generic
   background
4. **Build and Run** — commands that work as written, from the stated directory
5. **References** — the course text plus the relevant vendor documents
6. **AI Assistance** — see below

Write for a reader who is not you and is not in the conversation the document came
out of. In particular:

- No offers, questions, or next-step suggestions addressed to a reader
  ("If you'd like, I can also…"). Documentation states what is; it does not
  negotiate.
- No unfilled templates. A README with `[Objective 1]` in it is worse than no
  README.
- No placeholder links (`[Datasheet](#)`). Link the real document or drop the line.
- Paths, target names, and launch configurations named in a README must exist.
  Check them before committing.

## Source File Headers

Hand-written C files carry:

```c
/*
 * SPDX-License-Identifier: MIT
 * Author:      <name>
 * Book:        <course text, where the file follows one>
 * Description: <what this file does>
 */
```

Week and course attribution belongs in the week README, not in each file header —
driver files get copied forward between weeks, and per-file week numbers go stale
the moment they do.

## AI Transparency Requirement

Every week README ends with an **AI Assistance** section. Use `None.` when there
was none; otherwise name the tool and what it did, in one or two sentences.

Contributions generated or substantially drafted by AI must also say so in the
commit message or pull request body, for example:

```
AI-assisted: initial draft generated with Claude, validated and edited by maintainer.
```

## Licensing Checklist

Before committing:

- Added content is original, public domain, or licensed for redistribution.
- Attributions for reused material are preserved.
- No vendor PDFs, paywalled content, or copied textbook chapters are committed.
  Link to the official download instead and add it to the course's
  `COURSEID_References.md`.

## Pull Request Checklist

- [ ] Naming follows the conventions above
- [ ] Affected projects build (`cmake --build build`, `make`, or `ctest`)
- [ ] README paths, target names, and commands were checked against the tree
- [ ] No unfilled templates, placeholder links, or reader-directed AI text
- [ ] Licensing and attribution are clean
- [ ] AI assistance is disclosed
