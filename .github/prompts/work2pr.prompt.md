x---
agent: ask
description: Turn current work into a branch, commit, push, and PR using repo conventions
argument-hint: Describe the completed work and preferred scope (for example: ADC now working, include only Src changes)
---

# Work To PR

Take the completed local work and publish it as a clean pull request.

## Inputs

- Work summary from the prompt arguments (what changed and why)
- Optional scope constraints (which files or folders to include/exclude)
- Optional branch override (if omitted, derive branch from course/week convention)

## Required workflow

1. Inspect repository state.
   - Run:
     - `git branch --show-current`
     - `git status --short`
     - `git log main..HEAD --oneline`
   - If unrelated local edits are present, keep PR scope limited to the requested work.

2. Determine branch strategy.
   - Branch naming must follow `{course-lowercase}-week{NN}`.
   - If currently on `main`, create and switch to a compliant branch.
   - If a compliant branch already exists for this work, switch to it.
   - If the active branch name is non-compliant, warn and propose a compliant replacement before proceeding.

3. Validate commit scope before staging.
   - Exclude generated/build artifacts and system files.
   - Warn if any of these appear in scope: `*.o`, `*.elf`, `*.bin`, `*.hex`, `*.map`, `Debug/`, `Release/`, `.DS_Store`, IDE/build outputs.
   - If a large vendor/library folder is new, explicitly confirm intent from current instructions before including it.

4. Stage and commit.
   - Stage only the intended files.
   - Create a commit subject line in imperative mood, present tense, <=72 chars.
   - Add a commit body that explains why and includes AI disclosure when AI substantially assisted.

5. Validate before push.
   - Run the most relevant local build/test command for the modified project when feasible.
   - If validation cannot be run, state why.

6. Push branch.
   - Push to origin and set upstream if needed.
   - Capture resulting branch and remote status.

7. Create pull request.
   - Base branch: `main`.
   - Use `.github/pull_request_template.md` structure.
   - Fill Summary, Change Type, AI Disclosure, Licensing, Naming/Structure, Process Notes, and Validation.
   - Keep PR limited to committed changes only.

8. Return results.
   - Output:
     - Branch name used
     - Commit hash and commit message
     - Validation commands and outcomes
     - PR URL
     - Any warnings or follow-up actions

## Repository conventions to enforce

- Course directories: `UPPER-###`
- Week directories: `Week{NN}`
- Branch pattern: `{course-lowercase}-week{NN}`
- Source file names: lowercase snake_case
- Document naming: `COURSEID_Descriptive-Name.ext`

## Safety rules

- Do not revert unrelated local changes unless explicitly instructed.
- Do not use destructive git commands such as hard reset.
- If unexpected deletions appear, pause and ask how to proceed.
