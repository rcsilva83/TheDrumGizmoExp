# Contributing to DrumGizmo

Thank you for your interest in contributing! This guide covers the essentials
for getting started and working within the project's conventions.

## Building from source

```sh
cmake -S . -B build
cmake --build build -j$(nproc)
```

See the `README` and `AGENTS.md` for the full list of CMake options (LV2, VST,
GUI backend, NLS, etc.).

## Running the tests

```sh
cmake -S . -B build -DDG_ENABLE_TESTS=ON
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

Individual tests can be targeted with `-R <name>`:

```sh
ctest --test-dir build -R enginetest --output-on-failure
```

See `AGENTS.md` for the full list of test names.

## Code style

DrumGizmo uses **Allman braces**, **tabs for indentation** (width 4), and
**no space before parentheses** — `if(cond)` not `if (cond)`. The authoritative
style is enforced by `.clang-format`:

```sh
clang-format -i path/to/changed_file.cc
```

See `AGENTS.md` for the complete naming conventions, header-file rules, and
error-handling guidelines.

## Writing tests

Tests use the [doctest](https://github.com/doctest/doctest) framework and live
in `test/`. New tests are added with the `dg_add_doctest_test()` helper in
`test/CMakeLists.txt`. See an existing test such as `test/randomtest.cc` as a
starting point.

## Coverage Goals

DrumGizmo tracks test coverage via `gcovr` on every CI run. The full baseline
measurements and target thresholds are documented in
[docs/coverage.md](docs/coverage.md).

**Short-term targets** (line coverage):

| Module       | Target |
| ------------ | -----: |
| Overall      |   40%  |
| `src/`       |   60%  |
| `dggui/`     |   15%  |
| `plugingui/` |   10%  |
| `drumgizmo/` |   15%  |

When adding new features to `src/` please include unit tests that keep the
`src/` line coverage at or above the current baseline (54%). When fixing bugs,
add a regression test that would have caught the bug.

To generate a local coverage report see the instructions in
[docs/coverage.md](docs/coverage.md).
