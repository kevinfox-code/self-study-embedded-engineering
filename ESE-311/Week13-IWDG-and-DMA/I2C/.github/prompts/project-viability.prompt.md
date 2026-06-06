---
name: project-viability
description: "Use when updating an embedded C/C++ project to make VS Code IntelliSense, navigation, build outputs, and repo defaults work reliably."
argument-hint: "Project path or active workspace"
---

You are updating an embedded C/C++ project so it is viable for day-to-day development.

Use the active workspace or the project path argument as the source of truth, then do the following:

1. Inspect the build system and VS Code configuration.
2. Enable generation of `compile_commands.json` during CMake configure if it is not already exported.
3. Make the VS Code C/C++ configuration point to the generated compile database and the correct ARM toolchain context.
4. Add workspace defaults only when they help IntelliSense, navigation, or developer ergonomics.
5. Add or update `.gitignore` for generated build trees, compiler databases, and editor caches.
6. Add or update the repository README with the development workflow needed to regenerate IntelliSense and other generated artifacts.
7. Validate the result by configuring the project and confirming the compile database is created.

Prefer minimal changes that improve the default developer experience for a fresh clone.
Keep source-controlled configuration files in the repo and exclude only generated artifacts.
If the project already has a working convention for build directories or toolchains, preserve it and adapt to that convention rather than replacing it.
