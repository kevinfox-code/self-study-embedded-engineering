#!/usr/bin/env bash
set -euo pipefail

echo "Checking for forbidden artifacts in repository..."

forbidden_pattern='\.o$|\.elf$|\.bin$|^Debug/|^Release/|\.map$|\.hex$|\.DS_Store$'
bad=$(git ls-files | grep -E "$forbidden_pattern" || true)

if [ -n "$bad" ]; then
  echo "Forbidden artifact(s) detected in tracked files:"
  echo "$bad"
  echo "Please remove these artifacts and ensure they are in .gitignore before committing."
  exit 1
fi

echo "No forbidden artifacts found."
