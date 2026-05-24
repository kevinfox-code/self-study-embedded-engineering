---
mode: agent
description: Scaffold a new week folder with README and branch setup
---

# New Week Scaffold

Scaffold a new week for this embedded engineering curriculum.

## What to ask me first (if not already provided)

1. Which course? (`ESE-301`, `ESE-311`, or `ECE-452`)
2. Which week number? (e.g., `8` → padded to `Week08`)
3. What is the topic/title for this week?
4. Does this week need a standalone build? (Makefile for ECE-452 math, CMake for ESE-301 tests, or none for CubeIDE projects)

## Steps to perform

1. **Create the branch** (if not already on one):
   ```
   git checkout -b {course-lowercase}-week{NN}
   ```
   Example: `ese311-week08`

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

6. **Report what was created** and remind me to:
   - Add source files with correct naming (snake_case for C, `COURSEID_` prefix for docs)
   - Fill in the README objectives before the first commit
   - Stage and commit once files are ready

## Naming rules to enforce

- Week directory: `Week{NN}` — always two digits, capital W
- Course directory: uppercase with dash — `ESE-301`, `ESE-311`, `ECE-452`
- Source files: lowercase snake_case — `timer_config.c`, `clarke.h`
- Document files: `COURSEID_Title-With-Dashes.ext` — e.g., `ESE-311_Memory-Map.md`
