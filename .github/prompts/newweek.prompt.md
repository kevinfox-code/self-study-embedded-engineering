---
agent: ask
description: Scaffold a new week folder with README and branch setup
---

# New Week Scaffold

Scaffold a new week for this embedded engineering curriculum.

## Script arguments (no interactive prompts)

The script produced must be non-interactive and accept the following command-line arguments. If required arguments are missing or invalid, the script should exit with a usage message and non-zero status — it must not rely on external conversational prompts.

1. `COURSE` — one of `ESE-301`, `ESE-311`, or `ECE-452`
2. `WEEK_NUMBER` — integer (e.g., `8` or `08`) — the script must format this as `Week{NN}` where `NN` is zero-padded to two digits
3. `TOPIC_TITLE` — short descriptive title for the week
4. `BUILDSYS` (optional) — one of `Makefile`, `CMake`, or `None`. If omitted, treat as `None`.

Examples of invocation the script should accept:

```
./newweek.sh "ESE-301" 8 "Clarke and Park" Makefile
bash newweek.sh ECE-452 2 "SVPWM" None
```

## Steps to perform

> Constraint: Output a single, directly executable bash script (one copy-pasteable block) that:
> - checks the current git branch and only creates/checks out the new branch if not already on it,
> - creates the week directory and files using `mkdir -p` and `cat <<'EOF' > ...` here-documents,
> - does not include conversational text outside the script (no instructions or prompts before/after the block).
>
> Example (high-level):
> ```bash
> #!/usr/bin/env bash
> # (script checks current branch, conditionally runs git checkout -b, then uses here-docs)
> ```

1. **Create the branch**:
   - Include in the script a conditional branch check and creation so the script itself will only run `git checkout -b {course-lowercase}-week{NN}` when the current branch is different. Do not require a separate, manual git command outside the script.
   - Example branch name: `ese311-week08`

2. **Create the week directory**:
   ```
   {COURSE}/Week{NN}/
   ```
   Example: `ESE-311/Week08/`

3. **Create `README.md`** using this exact template:

   ```markdown
   # {COURSE} Week{NN}: {Topic Title}

   ## Objectives

   - 

   ## What Was Built

   - 

   ## Key Concepts

   - 

   ## References

   - 

   ## AI Assistance

   None
   ```

4. **If a Makefile is needed** (ECE-452 math/test exercises):

   ```makefile
   CC      = gcc
   CFLAGS  = -std=c11 -Wall -Wextra -O2
   LDFLAGS = -lm

   TARGET  = week{NN}_test
   SRCS    = main.c

   .PHONY: all clean run

   all: $(TARGET)

   $(TARGET): $(SRCS)
   	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

   run: $(TARGET)
   	./$(TARGET)

   clean:
   	rm -f $(TARGET)
   ```

5. **If CMakeLists.txt is needed** (ESE-301 sandbox/tests):

   ```cmake
   cmake_minimum_required(VERSION 3.16)
   project(week{NN} C)
   set(CMAKE_C_STANDARD 11)

   enable_testing()

   add_executable(week{NN}_main main.c)
   ```

6. **At the end of the script, add `echo` statements** that print:
   - Each file path that was created
   - A reminder to fill in the README objectives before the first commit
   - A reminder to stage and commit once source files are added

   These must be `echo` lines inside the script block — not prose outside it.

## Naming rules to enforce

- Week directory: `Week{NN}` — always two digits, capital W
- Course directory: uppercase with dash — `ESE-301`, `ESE-311`, `ECE-452`
- Source files: lowercase snake_case — `timer_config.c`, `clarke.h`
- Document files: `COURSEID_Title-With-Dashes.ext` — e.g., `ESE-311_Memory-Map.md`
   - Exception: repository-level README files for a week MUST be named `README.md` (use the template above). Other documents should follow the `COURSEID_Title-With-Dashes.ext` pattern.
