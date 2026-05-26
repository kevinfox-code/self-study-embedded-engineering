---
name: lesson-content-standards
description: "Use when creating, updating, or reviewing lesson content in this repository; ensure course/week headers are correct and code comments teach the concept clearly."
---

# Lesson Content Standards

Use this skill for ESE-301, ESE-311, and ECE-452 lesson files, especially READMEs, C sources, headers, and small examples.

## Goal

Keep lesson material aligned with the current course, week, and topic while making the code and notes readable as teaching material.

## Workflow

1. Identify the owning course, week, and lesson topic from the folder path and surrounding files.
2. Check the file header, title, and first comments for the correct course/week label.
3. For code files, add or fix a short top-of-file header comment when the lesson expects one.
4. Make comments instructional: explain why a step exists, what the hardware or math is doing, and what the learner should notice.
5. Remove stale references to a different week, module, board, or exercise.
6. Keep examples small and compileable so the teaching point stays focused.

## Commenting Rules

- Prefer comments that explain intent, tradeoffs, or hardware behavior.
- Do not narrate obvious syntax.
- Add comments around register setup, timing-sensitive logic, transforms, or other non-obvious steps.
- Keep the wording concise and classroom-friendly.

## Header Rules

- Use the correct course identifier and zero-padded week number when a header references a lesson week.
- Match the module or exercise name to the current folder.
- If the file is a lesson README, make the title and overview match the actual exercise or driver.
- If the file is a code sample, make sure the first block of comments describes the teaching goal.

## Completion Check

- The file name, header, and comments all point to the same lesson.
- No leftover references to a previous module or week remain.
- The file teaches the concept clearly enough for a learner to follow without outside context.
