# Repository Guidelines

## Project Structure & Module Organization
- Source: `src/core/*`, `src/interfaces/*`, `src/utils/*`
- CLI tools: `cli/` (e.g., `experiment_runner.cpp`, `keypoint_manager.cpp`)
- Legacy/bridging components: `descriptor_compare/`
- Descriptors: `keypoints/`
- Public headers: `include/thesis_project/`
- Configs: `config/experiments/*.yaml`
- Data/results: `data/`, `results/` (created by builds)
- Tests: `tests/unit/**`, `tests/unit/database/**`, `tests/unit/integration/**`

## Build, Test, and Development Commands
- Configure + build (native):
  - `mkdir -p build && cd build`
  - `cmake .. -DUSE_SYSTEM_PACKAGES=ON -DBUILD_DATABASE=ON -DCMAKE_BUILD_TYPE=Debug`
  - `make -j$(nproc)`
- Run CLIs (from `build/`):
  - `./experiment_runner ../config/experiments/sift_baseline.yaml`
  - `./keypoint_manager generate ../data`
- Tests:
  - `ctest --output-on-failure` (in `build/`)
  - Targets are auto-created by CMake if test sources exist.
- Docker dev shell (reproducible env):
  - `docker-compose -f docker-compose.dev.yml up -d`
  - `docker-compose -f docker-compose.dev.yml exec descriptor-dev bash`

## Coding Style & Naming Conventions
- Language: C++17; prefer 4-space indentation, no tabs.
- Files: lower_snake_case for filenames; headers in `include/` mirror structure.
- Types/enums: PascalCase types; UPPER_SNAKE for enum values.
- Functions/vars: camelCase to match existing code (e.g., `setDescriptorType`).
- Tools: `clang-format` and `clang-tidy` are available in the dev Docker image.
  - Example: `clang-format -i src/core/**/*.cpp descriptor_compare/*.hpp`.
- Match adjacent style where inconsistencies exist; avoid sweeping reformat PRs.

## Testing Guidelines
- Framework: CTest; GoogleTest used when available, else legacy mains.
- Location/patterns: `tests/unit/*.cpp`, `tests/unit/database/*.cpp` (e.g., `test_yaml_config_gtest.cpp`).
- Add focused unit tests per module; keep runtime < 60s (CTest timeouts).
- Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build`.

## Commit & Pull Request Guidelines
- Commits: short, imperative summaries (e.g., "Add CLI support", "Remove CSV deps").
- Scope PRs narrowly; include:
  - What/why, affected modules, flags (e.g., `-DBUILD_DATABASE`), and migration notes.
  - Test evidence: `ctest` output; example CLI invocations.
- Link related issues; attach screenshots/metrics only when UI/plots are affected.

## Security & Configuration Tips
- Database: default `-DBUILD_DATABASE=ON`; SQLite file lives in `build/experiments.db`.
- Do not commit datasets or generated results; keep `data/` and `results/` local.
- Prefer Docker for consistent OpenCV/TBB/Boost toolchains; native builds should use `USE_SYSTEM_PACKAGES=ON` unless using Conan.


## Session Log — 2025-08-31

Summary:
- Dropped backward compatibility; new pipeline is now the default.
- experiment_runner uses new extractors + pooling directly; no migration toggle.
- Stricter YAML Schema v1; added config/defaults/* templates.
- Removed migration bridge/toggle and related tests.

How to run (from `build/`):
- Generate locked-in keypoints: `./keypoint_manager generate-projected ../data sift_locked_proj`
- SIFT baseline (new pipeline): `./experiment_runner ../config/experiments/sift_baseline.yaml`
- DSP (gaussian weights) example: `./experiment_runner ../config/defaults/dsp_gaussian.yaml`

Next focus:
- Design new experiments and run across descriptors/pooling.
