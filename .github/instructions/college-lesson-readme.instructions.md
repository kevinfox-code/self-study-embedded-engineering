---
description: Use when creating or updating course/week lesson README files for ESE-301, ESE-311, or ECE-452. Enforce college-level structure, instructional clarity, and accurate course/week context.
applyTo: ["ESE-301/**/README.md", "ESE-311/**/README.md", "ECE-452/**/README.md"]
---

# College-Level Lesson README Standards

Apply this instruction to lesson and week README files so documentation reads like a college engineering lab handout.

## Required Section Structure

Each lesson README should include these sections in this order:

1. Title with course/week/module context
2. Objectives
3. What Was Built
4. Key Concepts
5. Build and Run (or Validation)
6. References
7. AI Assistance

## Title Requirements

- Use a precise title: `<COURSE> WeekNN: <Topic>`.
- Match directory context exactly.
- Remove stale references to other weeks/modules.

## Content Quality Requirements

- Objectives should be specific and measurable.
- What Was Built should describe actual artifacts, not intentions.
- Key Concepts should explain why the implementation works.
- Build/Validation should include concrete commands or observable outcomes.
- References should be legitimate sources (book chapter, datasheet, official docs).

## Instructional Tone

- Write clearly for a student audience at college level.
- Prefer concise technical language over generic prose.
- Explain tradeoffs and assumptions where relevant.

## Avoid These Patterns

- Do not leave placeholder sections.
- Do not include commands that are not valid for this project.
- Do not claim validation that was not performed.
- Do not omit AI disclosure section.

## Quick Self-Check Before Finalizing

- Does the title and section content match course/week/module path?
- Could a student reproduce the build/validation from this README alone?
- Are references and AI disclosure present and accurate?
