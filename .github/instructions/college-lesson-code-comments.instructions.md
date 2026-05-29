---
description: Use when writing or editing embedded C lesson code for ESE-301, ESE-311, or ECE-452. Enforce college-level file headers and instructional comments that teach intent, hardware behavior, and tradeoffs.
applyTo: ["ESE-301/**/*.{c,cpp,h}", "ESE-311/**/*.{c,cpp,h}", "ECE-452/**/*.{c,cpp,h}"]
---

# College-Level Lesson Code Headers And Comments

Use this instruction for course code that should read like a college software/embedded engineering lab submission.

## Required Header Template

Use this header style for new or updated `.c`, `.cpp`, and `.h` lesson files:

```c
/*
 * SPDX-License-Identifier: <license>
 * Author:      <name>
 * Book:        <textbook or source reference>
 *              <author/publisher line if needed>
 * Description: <what this file implements and what students should learn>
 */
```

Header rules:

- Keep SPDX, Author, Book, and Description aligned with the current lesson context.
- Keep wording technical and concise.
- Update header fields when file behavior changes.
- Do not rewrite untouched legacy files only to force this header format.

## Instructional Comment Expectations

- Write comments to teach reasoning, not syntax.
- Prefer comments that explain:
  - why a register write, timing value, or sequence is required
  - what hardware behavior is expected
  - what tradeoff is being made (accuracy, speed, power, simplicity)
- Add comments around non-obvious logic, especially:
  - clock and peripheral setup
  - calibration and startup sequences
  - ISR behavior and shared-state assumptions
  - fixed-point or control/math transforms

Complexity guidance (college-level):

- For simple utility code, 1-2 instructional comments per logical block is sufficient.
- For register-level initialization and control flow, comment each major phase (clocking, mode config, enable/start, read/verify/error path).
- For safety-critical or timing-sensitive paths, include brief rationale for timeout values and failure handling.

## Avoid These Patterns

- Do not narrate obvious code line-by-line.
- Do not leave placeholder comments that add no technical meaning.
- Do not use comments that conflict with actual implementation.
- Do not use assembly-specific guidance here; this instruction is intentionally limited to `.c`, `.cpp`, and `.h`.

## Style Guidance

- Keep comments concise and classroom-friendly.
- Prefer clear, technical wording over marketing language.
- Update comments when code changes so teaching intent stays aligned.

## Quick Self-Check Before Finalizing

- Would a student understand why the key steps exist?
- Does the file include the required SPDX/Author/Book/Description header fields with accurate values?
- Are comments focused on intent and behavior, not trivial syntax?
