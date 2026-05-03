# AGENTS.md -- DrumGizmo

DrumGizmo is a multichannel, multilayered, cross-platform drum plugin
and standalone application written in C++11. It uses CMake as the build
system. Version: 0.9.20 (pre-1.0).

## Project Structure

```
src/            Core engine library (libdg) -- no namespace
dggui/          GUI framework (namespace dggui)
plugingui/      Plugin GUI application (namespace GUI)
plugin/         LV2/VST plugin wrappers
drumgizmo/      CLI application
test/           Unit tests (doctest framework)
```

External libraries are fetched by CPM at configure time and are not stored as
git submodules in this repository.

## Build Commands

### Configure and Build
```sh
cmake -S . -B build -DDG_WITH_DEBUG=ON
cmake --build build -j$(nproc)
```

Useful options for parity testing:
```sh
cmake -S . -B build \
  -DDG_WITH_DEBUG=ON \
  -DDG_NUM_CHANNELS=16 \
  -DDG_ENABLE_CLI=ON \
  -DDG_ENABLE_LV2=ON \
  -DDG_GUI_BACKEND=x11
```

Additional plugin options:
```sh
cmake -S . -B build \
  -DDG_ENABLE_VST=ON \
  -DDG_VST_SDK_PATH=/path/to/vstsdk24
```

Notes:
- LV2/VST builds require the `plugin/plugingizmo` dependency fetched by CPM.
- GUI/plugin builds require `getoptpp` and `lodepng`; pugl backends also require `pugl` (all fetched by CPM).

### Install
```sh
cmake --install build --prefix "$PWD/install"
```

### Run All Tests
```sh
ctest --test-dir build --output-on-failure
```

### Run a Single Test
```sh
ctest --test-dir build -R <testname> --output-on-failure
# Example:
ctest --test-dir build -R randomtest --output-on-failure
```

Available test targets: `resource`, `enginetest`, `paintertest`, `configfile`,
`audiocache`, `audiocachefile`, `audiocacheidmanager`, `audiocacheeventhandler`,
`randomtest`, `atomictest`, `syncedsettingstest`, `imagecachetest`,
`semaphoretest`, `drumkitcreatortest`, `bytesizeparsertest`, `notifiertest`,
`dgxmlparsertest`, `domloadertest`, `configparsertest`, `midimapparsertest`,
`eventsdstest`, `powermaptest`, `midimappertest`, `nativewindow_x11`.

## Mandatory Pre-Commit Verification

Before every **code-affecting** commit, run all four stages below in order.
If any stage fails, fix the issues and re-run before committing.

Docs-only commits (changes only to `*.md` or documentation files, no C/C++
source changes) are exempt from this mandatory verification.

### Stage 1 — clang-format

Format every changed or newly added C/C++ file (`.c`, `.cc`, `.cpp`, `.h`,
`.hh`, `.hpp`) according to `.clang-format`:

```sh
clang-format -i path/to/changed_file.cc path/to/changed_file.h
```

Run in dry-run mode first to preview changes:
```sh
clang-format --dry-run --Werror path/to/changed_file.cc
```

### Stage 2 — Static analysis (cppcheck + clang-tidy)

First, (re)configure the build with `compile_commands.json` required by
clang-tidy:
```sh
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DDG_WITH_DEBUG=ON
```

Then run **cppcheck** on every changed/new C/C++ file:
```sh
cppcheck \
  --error-exitcode=1 \
  --enable=warning,style,performance,portability \
  --language=c++ \
  --std=c++11 \
  --suppress=missingIncludeSystem \
  --suppress=toomanyconfigs \
  --suppress=unusedStructMember \
  --suppress=noExplicitConstructor \
  --suppress=missingOverride \
  --suppress=passedByValue \
  --suppress=constParameterReference \
  --suppress=constVariableReference \
  --suppress=shadowFunction \
  --suppress=shadowVariable \
  --suppress=duplicateExpression \
  --suppress=useStlAlgorithm \
  --suppress=knownConditionTrueFalse \
  --suppress=unreadVariable \
  --suppress=uselessCallsSubstr \
  --suppress=useInitializationList \
  --suppress=stlcstrParam \
  --suppress=uselessOverride \
  --suppress=virtualCallInConstructor \
  --suppress=funcArgOrderDifferent \
  --suppress=signConversion \
  --suppress=selfAssignment \
  --suppress=ignoredReturnValue \
  --suppress=redundantAssignment \
  --suppress=shiftTooManyBitsSigned \
  --suppress=uninitvar \
  --suppress=constVariablePointer \
  --inline-suppr \
  path/to/changed_file.cc
```

For C files, use `--language=c --std=c11` and omit C++-only suppressions.

Then run **clang-tidy** on every changed/new source file (`.c`, `.cc`, `.cpp`)
that appears in `build/compile_commands.json`:
```sh
clang-tidy -p build path/to/changed_file.cc
```
Note: Header-only changes do not need a separate clang-tidy pass, but the
changed source files that include them must pass clang-tidy.

### Stage 3 — Build with compiler warnings-as-errors

Debug builds enable `-Wall -Werror -Wextra`. The build must complete with zero
warnings:
```sh
cmake -S . -B build -DDG_WITH_DEBUG=ON
cmake --build build -j$(nproc)
```

### Stage 4 — Tests

```sh
ctest --test-dir build --output-on-failure
```

If any required stage fails, do **not** commit until it is fixed.

## SonarCloud Best Practices

The CI pipeline runs SonarCloud analysis on every push and pull request.
To avoid SonarCloud warnings and errors, follow these additional guidelines:

- **Initialize all member variables** — Use C++11 in-class initializers or
  constructor initializer lists for every member:
  ```cpp
  int count{0};
  std::string name;
  ```
- **Use `override`** — Every virtual function that overrides a base class
  function must be explicitly marked `override`.
- **Prefer `= default` and `= delete`** over empty or private-unimplemented
  special member functions.
- **Use `nullptr`** — Never use `NULL` or `0` for null pointers.
- **Use `explicit`** on single-argument constructors and conversion operators:
  ```cpp
  explicit MyClass(int value);
  ```
- **No commented-out code** — Remove dead/commented-out code before committing.
- **Use `empty()` instead of `size() == 0`** on STL containers.
- **Use range-based for loops** when iterating over entire containers.
- **Avoid C-style casts** — Use `static_cast<>`, `const_cast<>`, etc.
- **Mark member functions `const`** where possible.
- **Prefer `emplace_back()` over `push_back()`** when constructing in place.
- **Do not use `m_` prefix** on member variables — use the snake_case
  convention defined in this project.
- **Avoid `goto` and unconditional `break`/`continue`** where structured
  alternatives exist.
- **No public data members** — Use accessor functions.
- **Return values from all non-void functions** on all code paths.
- **Ensure names are consistent** — for example `assert()` argument order
  should match the natural expression being tested.

## Code Style (enforced by .clang-format)

- **Brace style**: Allman -- opening braces on their own line
- **Indentation**: Tabs (width 4)
- **Namespace indentation**: None (contents not indented)
- **Pointer alignment**: Left (`int* ptr`, not `int *ptr`)
- **Always use braces** for if/for/while, even single-line bodies
- **No single-line functions**: Bodies always expanded
- **No space before parens**: `if(cond)` not `if (cond)`
- **Constructor initializer lists**: Comma-first, one per line:
  ```cpp
  MyClass::MyClass()
      : foo(1)
      , bar(2)
      , baz(3)
  ```

## Header Files

- **Always use `#pragma once`** (never `#ifndef` guards)
- Start every file with the Emacs modeline and license header block
- **Include order**:
  1. Own header (`#include "myclass.h"`)
  2. Standard library (`#include <string>`, `#include <vector>`)
  3. External libraries (`#include <hugin.hpp>`, `#include <config.h>`)
  4. Project cross-directory (`#include <settings.h>` with angle brackets)
  5. Same-directory project (`#include "localfile.h"` with quotes)
- Use `""` for same-directory includes, `<>` for cross-directory and system

## Text Encoding

- Save source and documentation files as **UTF-8** (without BOM)
- Preserve author names exactly as written in license headers (do not transliterate)
- If a file contains non-ASCII characters (for example `ö`), keep them intact and avoid mojibake replacements

## Naming Conventions

| Element            | Style                    | Examples                                  |
|--------------------|--------------------------|-------------------------------------------|
| Classes/Structs    | PascalCase               | `DrumGizmo`, `AudioCache`, `Widget`       |
| Methods/Functions  | camelCase                | `getLatency()`, `setSamplerate()`         |
| Member variables   | snake_case               | `frame_size`, `audio_cache`, `chunk_size` |
| Private members    | `_prefix` (some files)   | `_name`, `_file`, `_visible`              |
| Type aliases       | PascalCase (`using`)     | `AudioFiles`, `Instruments`, `InstrumentID` |
| Low-level types    | snake_case_t             | `sample_t`, `channel_t`, `cacheid_t`      |
| constexpr          | snake_case               | `velocity_modifier_falloff_default`       |
| #define macros     | ALL_CAPS                 | `NUM_CHANNELS`, `MAX_RESAMPLER_BUFFER_SIZE` |
| enum class         | PascalCase name          | `enum class LoadStatus { Idle, Done }`    |
| Files              | all lowercase            | `drumgizmo.h`, `audiocache.cc`            |
| Test files         | `<name>test.cc`          | `randomtest.cc`, `enginetest.cc`          |

## Namespaces

- `src/` engine code: **no namespace** (global)
- `dggui/`: `namespace dggui { }` -- close with `} // dggui::`
- `plugingui/`: `namespace GUI { }` -- close with `} // GUI::`

## Error Handling

- **No exceptions** -- the engine is real-time safe; never use `throw`
- Return `bool` (`false` on failure) from init/load/parse functions
- Use `assert()` liberally for preconditions and invariants
- Log with hugin macros: `DEBUG(category, "fmt", ...)`,
  `ERR(category, ...)`, `WARN(category, ...)`, `INFO(category, ...)`
- Prefer graceful degradation (return silence buffer on cache miss, etc.)

## Memory Management

- Prefer `std::unique_ptr` for owned resources
- Use raw pointers only for **non-owning references**
- `std::shared_ptr` is rare (only GUI event queues)
- Use C++11 in-class member initialization: `int count{0};`

## Documentation / Comments

- Use `//!` for Doxygen doc comments (not `/** */`):
  `//! Get the current engine latency in samples.`
- Use `//! \brief`, `//! \param`, `//! \return` for detailed docs
- Use `//` for inline implementation comments; `// TODO:` / `// FIXME:` for tracked issues

## Unit Tests (doctest Framework)

Tests use the [doctest](https://github.com/doctest/doctest) framework (fetched by CPM).

### Writing a Test

```cpp
#include <doctest/doctest.h>
#include "module_under_test.h"

TEST_CASE("basic operation")
{
    MyModule m;
    CHECK_UNARY(m.isValid());
    CHECK_EQ(42, m.getValue());
}

// Use a fixture struct for shared state across test cases
struct MyModuleFixture
{
    MyModule m;
};

TEST_CASE_FIXTURE(MyModuleFixture, "edge case")
{
    CHECK_EQ(0, m.edgeCase());
}
```

### Available Assertions

| Macro                       | Purpose                                        |
|-----------------------------|------------------------------------------------|
| `CHECK(expr)`               | Assert truthiness (non-fatal)                  |
| `CHECK_EQ(exp, act)`        | Assert equality (non-fatal)                    |
| `CHECK_UNARY(expr)`         | Assert compound boolean expressions (non-fatal)|
| `REQUIRE(expr)`             | Assert truthiness (fatal — stops the test)     |
| `REQUIRE_EQ(exp, act)`      | Assert equality (fatal)                        |

## C++ Standard and Compiler Flags

- **C++11** (`-std=c++11`); debug builds: `-Wall -Werror -g`
- SSE auto-detected; logging disabled in release: `-DDISABLE_HUGIN`
