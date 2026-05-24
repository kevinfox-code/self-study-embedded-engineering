# Agent Roles and Using Prompts

This repository provides a set of Copilot/agent prompts and a minimal CI workflow to support an interactive, agent-assisted development flow.

Agent roles

- Scaffolder: runs `new-week.prompt.md` to create project scaffolding for a course week.
- Build Checker: runs `build-check.prompt.md` to detect appropriate build systems and provide build commands.
- PR Preparer: runs `pr-prep.prompt.md` to draft PR descriptions from recent commits and ensure AI disclosure is included.
- Session Guide: use `sessionstart.prompt.md` at session start to load conventions and ground rules.

How to use

1. Create a branch named using the pattern from [.github/copilot-instructions.md](.github/copilot-instructions.md).
2. Open Copilot Chat and run `sessionstart.prompt.md` to orient the agent.
3. Use `new-week.prompt.md` to scaffold files as needed.
4. Locally build and run tests (see `ECE-452/` and `ESE-301/` subfolders for examples).
5. Commit with an AI disclosure entry in the commit body if the agent substantially helped.
6. Push and open a PR; the `Agentic CI` workflow will run on push and PRs to `main`.

CI expectations

- The workflow runs on `ubuntu-latest` and will attempt to build detected `Makefile` and `CMakeLists.txt` projects.
- The workflow also runs a forbidden-artifact check and basic script linting (`shellcheck`).

Notes

- This file is intentionally minimal; expand with project-specific agent responsibilities as needed.
