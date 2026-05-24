---
mode: agent
description: Prepare a complete PR description from recent commits on the current branch
---

# PR Preparation

Generate a complete pull request description for the current branch, ready to paste into GitHub.

## Steps to perform

1. **Read the current branch name** and validate it follows `{course}-week{NN}` convention.
   - If it does not match, warn me and ask before proceeding.

2. **Get recent commits** on this branch (since diverging from `main`):
   ```bash
   git log main..HEAD --oneline
   ```

3. **Get the full diff summary**:
   ```bash
   git diff main..HEAD --stat
   ```

4. **Check for any staged build artifacts** that should not be committed:
   ```bash
   git status
   ```
   Warn me if `.o`, `.elf`, `.bin`, `.hex`, `.map`, `Debug/`, or `Release/` appear staged.

5. **Generate a PR description** using `.github/pull_request_template.md` as the structure.
   Fill in:
   - **Summary**: what changed and why (1–3 sentences, factual)
   - **Change Type**: check the correct box based on the commits
   - **AI Assistance Disclosure**: ask me whether AI was used and to what extent before filling this in
   - **Licensing and Attribution**: pre-check boxes that clearly apply; flag any uncertain ones
   - **Naming and Structure Checklist**: verify against repo conventions before checking
   - **Validation**: list what was tested (e.g., `make run` output, observed behavior)

6. **Output the complete PR description** in a code block so I can copy-paste it directly into GitHub.

## Naming validation rules

Before checking the "Naming and Structure" boxes, verify:

- Week directory is `Week{NN}` (capital W, two-digit number)
- Course directory is `UPPER-###` format
- Document files use `COURSEID_Descriptive-Name.ext`
- Source files are lowercase snake_case
- No Unicode punctuation in any filename

## AI disclosure guidance

Ask me explicitly:
- "Was Copilot or another AI tool used to draft any code or documentation in this PR?"
- If yes: "What was the scope? (drafting, editing, code generation, restructuring)"
- "What human validation was performed?"

Then fill in the disclosure section accordingly. Never pre-fill "No AI assistance" without confirming with me.
