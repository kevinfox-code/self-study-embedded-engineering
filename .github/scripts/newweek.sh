#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 3 ]; then
  echo "Usage: $0 <COURSE> <WEEK_NUMBER> <TOPIC_TITLE> [Makefile|CMake|None]"
  exit 2
fi

COURSE="$1"            # e.g. ESE-301
WEEKNUM="$2"          # e.g. 8 or 08
TOPIC="$3"            # e.g. "Clarke and Park"
BUILDSYS="${4:-None}" # Makefile, CMake, or None

NN=$(printf "%02d" "$WEEKNUM")
# branch name: remove non-alnum, lowercase, then -weekNN (e.g. ese311-week08)
BRANCH_BASE=$(echo "$COURSE" | tr -cd '[:alnum:]' | tr '[:upper:]' '[:lower:]')
BRANCH="${BRANCH_BASE}-week${NN}"

# 1) create/switch branch only if not already on it
current_branch=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "")
if [ "$current_branch" = "$BRANCH" ]; then
  echo "On branch $BRANCH — skipping branch creation."
else
  echo "Creating and checking out branch: $BRANCH"
  git checkout -b "$BRANCH"
fi

# 2) create week directory
WEEK_DIR="${COURSE}/Week${NN}"
mkdir -p "$WEEK_DIR"

# 3) create README.md using template
cat > "$WEEK_DIR/README.md" <<EOF
# ${COURSE} Week${NN}: ${TOPIC}

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
EOF

# 4) create Makefile or CMake if requested
if [ "$BUILDSYS" = "Makefile" ]; then
  cat > "$WEEK_DIR/Makefile" <<EOF
CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -O2
LDFLAGS = -lm

TARGET  = week${NN}_test
SRCS    = main.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
EOF
elif [ "$BUILDSYS" = "CMake" ]; then
  cat > "$WEEK_DIR/CMakeLists.txt" <<EOF
cmake_minimum_required(VERSION 3.16)
project(week${NN} C)
set(CMAKE_C_STANDARD 11)

enable_testing()

add_executable(week${NN}_main main.c)
EOF
fi

# 5) summary
echo "Created: $WEEK_DIR/README.md"
if [ "$BUILDSYS" != "None" ]; then
  if [ "$BUILDSYS" = "Makefile" ]; then
    echo "Created: $WEEK_DIR/Makefile"
  else
    echo "Created: $WEEK_DIR/CMakeLists.txt"
  fi
fi
echo "Next: add source files, edit README, stage and commit when ready."
