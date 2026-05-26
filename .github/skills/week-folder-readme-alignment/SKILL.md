---
name: week-folder-readme-alignment
description: "Use when maintaining a course week folder so the folder contents, week README, and commit message all stay aligned."
---

# Week Folder README Alignment

Use this skill when you add, remove, rename, or revise material inside an ESE-301, ESE-311, or ECE-452 week folder.

## Goal

Keep each week folder self-consistent: the files in the folder should match the week README, and the commit message should describe the full set of changes.

## Workflow

1. Identify the course, week number, and intended lesson scope from the folder path.
2. List the actual files and subfolders being changed in that week.
3. Compare those changes against the week README and update the README so it describes the real contents of the folder.
4. Remove stale README references to files, exercises, or topics that no longer exist.
5. Add missing README entries for new lesson content, build instructions, tests, or support files.
6. Check that file names, headings, and references use the correct course identifier and zero-padded week number.
7. Draft a commit message that includes all meaningful details of the change set, not just the headline feature.

## Decision Points

- If the folder adds a new exercise or support target, document it in the week README.
- If the folder removes content, delete or rewrite the corresponding README entry so the README does not overstate what is present.
- If the folder contains build or test instructions, keep them accurate and specific to the current week.
- If changes span more than one lesson topic, describe each topic explicitly instead of collapsing them into a vague summary.

## Quality Checks

- Every file mentioned in the week README exists in the folder.
- Every notable file or exercise in the folder appears in the week README.
- Course and week labels are consistent across filenames, headings, and references.
- Commit messages are detailed enough that another maintainer can understand the full scope without reading the diff first.

## Completion Criteria

The week folder, its README, and the commit summary all tell the same story.
