# AGENTS.md -- DrumGizmo

DrumGizmo is a multichannel, multilayered, cross-platform drum plugin
and standalone application written in C++11. It now supports a CMake build
in parallel with the legacy Autotools flow. Version: 0.9.20 (pre-1.0).

## Project Structure

```
src/            Core engine library (libdg.la) -- no namespace
dggui/          GUI framework (namespace dggui)
plugingui/      Plugin GUI application (namespace GUI)
plugin/         LV2/VST plugin wrappers
drumgizmo/      CLI application
test/           Unit tests (uUnit framework)
hugin/          Logging library (submodule)
pugixml/        XML parser (submodule)
zita-resampler/ Audio resampler (submodule)
test/uunit/     Unit test framework (submodule)
```

## Build Commands

### CMake (preferred)
```sh
cmake -S . -B build
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
- LV2/VST builds require the `plugin/plugingizmo` submodule.
- GUI/plugin builds require `getoptpp` and `dggui/lodepng`; pugl backends also require the `pugl` submodule.

### CMake install
```sh
cmake --install build --prefix "$PWD/install"
```

### Initial Setup (from git)
```sh
git submodule init && git submodule update
./autogen.sh
./configure --prefix=$PWD/install --with-test --with-debug --enable-lv2
```

### Build
```sh
make            # Build everything
make -j$(nproc) # Parallel build
```

Autotools commands above are still supported during migration.

### Run All Tests
```sh
make check                  # Build and run all tests (requires --with-test)
```

### Run a Single Test
```sh
# From the test/ build directory:
make <testname> && ./<testname>
# Example:
make randomtest && ./randomtest
```

Available test targets: `resource`, `enginetest`, `paintertest`, `configfile`,
`audiocache`, `audiocachefile`, `audiocacheidmanager`, `audiocacheeventhandler`,
`randomtest`, `atomictest`, `syncedsettingstest`, `imagecachetest`,
`semaphoretest`, `drumkitcreatortest`, `bytesizeparsertest`, `notifiertest`,
`dgxmlparsertest`, `domloadertest`, `configparsertest`, `midimapparsertest`,
`eventsdstest`, `powermaptest`, `midimappertest`.

### Formatting
```sh
clang-format -i <file>      # Format a single file per .clang-format
```

There is no dedicated lint command. Debug builds use `-Wall -Werror -Wextra`.

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

## Unit Tests (uUnit Framework)

Tests use the custom `uUnit` micro-framework (in `test/uunit/`).

### Writing a Test

```cpp
#include <uunit.h>
#include "module_under_test.h"

class MyModuleTest : public uUnit
{
public:
    MyModuleTest()
    {
        uUNIT_TEST(MyModuleTest::basicOperation);
        uUNIT_TEST(MyModuleTest::edgeCase);
    }

    void setup() override { /* runs before each test */ }
    void teardown() override { /* runs after each test */ }

    void basicOperation()
    {
        MyModule m;
        uUNIT_ASSERT(m.isValid());
        uUNIT_ASSERT_EQUAL(42, m.getValue());
    }
};

static MyModuleTest test; // Auto-registers the suite
```

### Available Assertions

| Macro                            | Purpose                           |
|----------------------------------|-----------------------------------|
| `uUNIT_ASSERT(expr)`            | Assert truthiness                 |
| `uUNIT_ASSERT_EQUAL(exp, act)`  | Assert equality (with epsilon for doubles) |
| `uUNIT_ASSERT_THROWS(Exc, expr)`| Assert expression throws          |

## C++ Standard and Compiler Flags

- **C++11** (`-std=c++11`); debug builds: `-Wall -Werror -g`
- SSE auto-detected; logging disabled in release: `-DDISABLE_HUGIN`
