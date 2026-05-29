# Self-Study Embedded Engineering

Self-directed, college-level embedded systems curriculum with a focus on:

- practical firmware architecture
- bare-metal ARM/C development
- sensorless motor control implementation

The repository is organized for traceability and professional documentation standards.

## Contents

- [ESE-301](ESE-301): Embedded systems design and engineering patterns
- [ESE-311](ESE-311): Bare-metal embedded C programming on ARM Cortex-M
- [ESE-311/Week11/ADC](ESE-311/Week11/ADC): bare-metal STM32U575 ADC example with explicit sample-time setup
- [ECE-452](ECE-452): Electric motor drives and field-oriented control
- [embedded-engineering-course-schedule.ics](embedded-engineering-course-schedule.ics): course calendar

## Environment Setup

- [ESE-301/Week1/blinky/ESE-301_Environment-Setup.md](ESE-301/Week1/blinky/ESE-301_Environment-Setup.md): environment variable setup for STM32CubeIDE tools (macOS + Windows + Linux)

## Course Syllabus Files

The current syllabus documents are stored as Word (`.docx`) files. If they do not preview well in your browser, download and open them in Microsoft Word or LibreOffice. For easier review in pull requests, prefer exporting syllabus updates to PDF or Markdown alongside the `.docx` source when possible.
- [ESE-301/ESE-301_Making-Embedded-Systems_Syllabus.docx](ESE-301/ESE-301_Making-Embedded-Systems_Syllabus.docx)
- [ESE-311/ESE-311_Bare-Metal-Embedded-C-Programming_Syllabus.docx](ESE-311/ESE-311_Bare-Metal-Embedded-C-Programming_Syllabus.docx)
- [ECE-452/ECE-452_Sensorless-FOC-STM32-Implementation-in-C_Syllabus.docx](ECE-452/ECE-452_Sensorless-FOC-STM32-Implementation-in-C_Syllabus.docx)
- [ECE-452/ECE-452_Sensorless-FOC-STM32-Implementation-in-C_Syllabus.md](ECE-452/ECE-452_Sensorless-FOC-STM32-Implementation-in-C_Syllabus.md)

## Naming Conventions

This repository follows a simple, predictable naming scheme:

- Directories: `UPPER-###` for course identifiers (example: `ESE-301`)
- Document files: `COURSEID_Descriptive-Name.ext`
- Shared project files: lowercase kebab-case (example: `embedded-engineering-course-schedule.ics`)

## License And Attribution

- Project license: [LICENSE](LICENSE) (MIT)
- AI transparency and attribution policy: [NOTICE.md](NOTICE.md)
- Contribution expectations and content policy: [CONTRIBUTING.md](CONTRIBUTING.md)

## Notes

- This repository may include references to third-party books and course material for educational context.
- Rights for third-party content remain with original owners.

## Reusable Skills

- [lesson-content-standards](.github/skills/lesson-content-standards/SKILL.md): keep lesson READMEs, code headers, and teaching comments aligned with the correct course week.
- [week-folder-readme-alignment](.github/skills/week-folder-readme-alignment/SKILL.md): keep each week folder, its README, and commit summary aligned when content changes.

## Agentic Workflow (interactive)

This repository supports an interactive, agent-assisted workflow using the Copilot/VS Code prompts found under `.github/prompts/` and a lightweight CI workflow that validates builds and repository hygiene on PRs.

- **Local flow (typical)**: create a branch using the `{course-lowercase}-week{NN}` pattern, run `sessionstart.prompt.md` in Copilot to orient the agent, use `new-week.prompt.md` to scaffold work, build/tests locally, commit with AI disclosure in the commit body, push and open a PR.
- **Publish flow (work to PR)**: run `work2pr.prompt.md` to convert completed local work into a compliant branch, scoped commit, push, and PR using the repository template/checklists.
- **Automated checks**: the `Agentic CI` GitHub Actions workflow runs on `push` and `pull_request` to `main` and will attempt to build detected `Makefile` and `CMakeLists.txt` projects, run a forbidden-artifact check, and lint scripts.
- **Where to look**: See `AGENTS.md` for agent role descriptions and `.github/pull_request_template.md` for required PR disclosure.

Example minimal workflow (developer):

1. Create a branch: `git checkout -b ese301-week07`
2. Start a Copilot session: open Copilot Chat and run `sessionstart.prompt.md` (select the prompt from `.github/prompts/`).
3. Use prompts to generate or scaffold files (e.g., `new-week.prompt.md`).
4. Build and run tests locally (for CMake projects: `mkdir -p build && cd build && cmake .. && cmake --build . && ctest`; for Makefile projects: `make`).
5. Commit with AI disclosure in the commit body per [.github/copilot-instructions.md](.github/copilot-instructions.md).
6. Push and open a PR; CI will run and report build/test status.

