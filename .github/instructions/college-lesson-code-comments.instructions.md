---
description: Use when writing or editing embedded C lesson code for ESE-301, ESE-311, or ECE-452. Enforce college-level file headers and instructional comments that teach intent, hardware behavior, and tradeoffs.
applyTo: ["ESE-301/**/*.{c,cpp,h}", "ESE-311/**/*.{c,cpp,h}", "ECE-452/**/*.{c,cpp,h}"]
---

# College-Level Lesson Code Headers And Comments

Use this instruction for course code that should read like a college software/embedded engineering lab submission.

## Required Header Template

Use this exact structure at the top of each `.c`, `.cpp`, and `.h` lesson file, adapted to the file context:

```c
/*
 * File:        <filename>
 * Course:      <ESE-301 | ESE-311 | ECE-452>
 * Week:        <WeekNN>
 * Module:      <module or lab name>
 * Purpose:     <what this file implements>
 * Learning:    <what a student should learn from this file>
 * Dependencies:<key headers/peripherals/assumptions>
 */
```

Header rules:

- Keep all fields present. Do not omit lines.
- Keep wording technical and concise.
- Match the folder context exactly (course/week/module).
- Update header fields when file behavior changes.

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
- Does the file include the full required header template with accurate values?
- Do header and comments match the course/week/module in this folder?
- Are comments focused on intent and behavior, not trivial syntax?
