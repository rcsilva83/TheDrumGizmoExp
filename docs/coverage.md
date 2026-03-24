# Test Coverage

This document records the test coverage baseline for DrumGizmo and defines
target thresholds to guide future coverage improvements.

Coverage is measured with [gcovr](https://gcovr.com/) using GCC's `--coverage`
instrumentation. The CI workflow (`build.yml`) produces detailed HTML, Cobertura
XML and SonarQube XML reports on every push.

---

## Baseline (2026-03-24)

The numbers below were collected on commit
[a7f2c07](https://github.com/rcsilva83/TheDrumGizmoExp/commit/a7f2c077990acbe2a357c522548a20ee4c654f3e)
with the standard test suite (`DG_ENABLE_TESTS=ON`, LV2/VST/NLS disabled) running
on Ubuntu with GCC.

### Overall

| Metric     | Covered | Total  | Coverage |
| ---------- | ------: | -----: | -------: |
| Lines      |   4,131 | 11,935 |    34.6% |
| Functions  |     458 |  1,444 |    31.7% |
| Branches   |   6,758 | 25,476 |    26.5% |

### By module

| Module        | Lines (cov/total) | Line % | Function % | Branch % |
| ------------- | ----------------: | -----: | ---------: | -------: |
| `src/`        |      1,994 / 3,674 |  54.3% |      57.9% |    29.3% |
| `dggui/`      |        382 / 3,705 |  10.3% |      11.6% |     5.0% |
| `plugingui/`  |          0 / 1,410 |   0.0% |       0.0% |     0.0% |
| `plugin/`     |           0 / 0    |    N/A |        N/A |      N/A |
| `drumgizmo/`  |          0 / 1,280 |   0.0% |       0.0% |     0.0% |
| `test/`       |      1,755 / 1,866 |  94.1% |      82.5% |    41.9% |

**Notes**

- `plugingui/` and `drumgizmo/` show 0% because none of their translation units
  are exercised by the current headless unit-test suite (no audio I/O or GUI
  rendering is performed during tests).
- `plugin/` has no instrumented lines because the LV2/VST wrappers are not
  built in the test configuration.
- The `test/` module itself has high line coverage (94%) because all test code
  runs under doctest; the uncovered ~6% is test helpers and edge-case paths.

---

## Coverage Targets

The project targets **90% line coverage** across all modules.

| Module       | Line target | Notes                                       |
| ------------ | ----------: | ------------------------------------------- |
| Overall      |        90%  |                                             |
| `src/`       |        90%  | Core engine; highest priority               |
| `dggui/`     |        90%  | GUI framework                               |
| `plugingui/` |        90%  | Plugin GUI                                  |
| `drumgizmo/` |        90%  | CLI application                             |

---

## Generating Coverage Locally

```sh
# 1. Configure with coverage instrumentation
cmake -S . -B build-coverage \
  -DDG_ENABLE_TESTS=ON \
  -DDG_WITH_NLS=OFF \
  -DDG_ENABLE_LV2=OFF \
  -DDG_ENABLE_VST=OFF \
  -DDG_GUI_BACKEND=x11 \
  -DCMAKE_BUILD_TYPE=Debug \
  "-DCMAKE_C_FLAGS=--coverage -O0 -g" \
  "-DCMAKE_CXX_FLAGS=--coverage -O0 -g" \
  "-DCMAKE_EXE_LINKER_FLAGS=--coverage" \
  "-DCMAKE_SHARED_LINKER_FLAGS=--coverage"

# 2. Build and run tests
cmake --build build-coverage -j$(nproc)
ctest --test-dir build-coverage --output-on-failure

# 3. Generate HTML report
pip install gcovr==8.6
mkdir -p coverage
gcovr \
  --root . \
  --object-directory build-coverage \
  --gcov-ignore-parse-errors=all \
  --gcov-ignore-errors=source_not_found \
  --filter '^(src|dggui|plugingui|plugin|drumgizmo|test)/' \
  --html-details --output coverage/index.html

# Open coverage/index.html in a browser
```

---

## Updating the Baseline

Whenever a significant batch of new tests is merged, update the baseline table
in this file with the new numbers and the commit SHA. This keeps the document
useful as a historical reference.
