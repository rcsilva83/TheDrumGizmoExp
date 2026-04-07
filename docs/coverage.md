# Test Coverage

This document records the test coverage baseline for DrumGizmo and defines
target thresholds to guide future coverage improvements.

For risk-driven gap prioritization and follow-up test implementation planning,
see `docs/high-risk-test-backlog.md`.

Coverage is measured with [gcovr](https://gcovr.com/) using GCC's `--coverage`
instrumentation. The CI workflow (`build.yml`) produces detailed HTML, Cobertura
XML and SonarQube XML reports on every push.

The workflow also emits a JSON summary (`coverage-summary.json`) and generates
`docs/coverage-top20-file-gaps.md`, a prioritized top-20 file list by uncovered
branches with explicit per-file branch/line targets.

---

## Baseline (2026-04-07)

The numbers below were collected on commit
[81e7f3e](https://github.com/rcsilva83/TheDrumGizmoExp/commit/81e7f3e)
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

## CI Enforcement

The CI coverage job (`build.yml`) enforces minimum line-coverage thresholds to
prevent regression. After generating the coverage reports the workflow runs:

```sh
python3 scripts/check-coverage.py
python3 scripts/generate-coverage-gap-report.py --summary coverage-summary.json --output docs/coverage-top20-file-gaps.md
```

The thresholds are stored in **`.coverage-thresholds.json`** at the repository
root. The script reads `coverage.xml` (Cobertura format), computes overall and
per-module line coverage, and exits with a non-zero status if any threshold is
not met — causing the CI job to fail.

On pull requests an additional check runs for each changed source file:

```sh
python3 scripts/check-pr-coverage.py --xml coverage.xml <changed files>
```

Any changed `.c`/`.cc`/`.cpp`/`.cxx` file in `src/`, `dggui/`, `plugingui/`,
`plugin/`, or `drumgizmo/` must meet the `changed_file_min` floor (currently
**30%**) or the CI job fails.

Branch-level coverage visibility is provided in three places:

- GitHub Actions step summary (overall branch coverage and top uncovered files)
- `docs/coverage-top20-file-gaps.md` in the repository
- `coverage-reports` artifact (`coverage.xml`, `coverage-summary.json`, HTML)

### Configured thresholds

The values below match the current baseline (see table above) with a small
buffer to tolerate natural measurement variation:

| Scope    | Min line % | Note                                    |
| -------- | ---------: | --------------------------------------- |
| Overall  |        34% | Baseline 34.6 % (commit a7f2c07)        |
| `src/`   |        54% | Baseline 54.3 % (commit a7f2c07)        |

---

## Ratchet Policy

### Goal and ownership

The project targets **90% line coverage** on all source modules. Coverage
thresholds are owned by the contributor who last advanced the ratchet; any PR
that would drop coverage below the current floor is blocked by CI.

### Ratchet mechanics

Coverage thresholds rise by **+2 percentage points** per ratchet cycle until
they reach the 90% target. After merging a batch of new tests, run:

```sh
python3 scripts/advance-ratchet.py
```

This increments every `line_min` value in `.coverage-thresholds.json` by the
configured increment and prints a summary. Commit the updated file together
with a note in the baseline table below.

### Incremental schedule

The table shows every ratchet step from the current baselines to the 90%
target. Steps marked **current** are the active thresholds.

#### Overall coverage (34% → 90%, +2%/cycle)

| Cycle | Threshold |          |
| ----: | --------: | -------- |
|     0 |       34% | **current** |
|     1 |       36% |          |
|     2 |       38% |          |
|     3 |       40% |          |
|     4 |       42% |          |
|     5 |       44% |          |
|     6 |       46% |          |
|     7 |       48% |          |
|     8 |       50% |          |
|     9 |       52% |          |
|    10 |       54% |          |
|    11 |       56% |          |
|    12 |       58% |          |
|    13 |       60% |          |
|    14 |       62% |          |
|    15 |       64% |          |
|    16 |       66% |          |
|    17 |       68% |          |
|    18 |       70% |          |
|    19 |       72% |          |
|    20 |       74% |          |
|    21 |       76% |          |
|    22 |       78% |          |
|    23 |       80% |          |
|    24 |       82% |          |
|    25 |       84% |          |
|    26 |       86% |          |
|    27 |       88% |          |
|    28 |       90% | **target** |

#### `src/` module (54% → 90%, +2%/cycle)

| Cycle | Threshold |          |
| ----: | --------: | -------- |
|     0 |       54% | **current** |
|     1 |       56% |          |
|     2 |       58% |          |
|     3 |       60% |          |
|     4 |       62% |          |
|     5 |       64% |          |
|     6 |       66% |          |
|     7 |       68% |          |
|     8 |       70% |          |
|     9 |       72% |          |
|    10 |       74% |          |
|    11 |       76% |          |
|    12 |       78% |          |
|    13 |       80% |          |
|    14 |       82% |          |
|    15 |       84% |          |
|    16 |       86% |          |
|    17 |       88% |          |
|    18 |       90% | **target** |

### Updating thresholds

When a batch of new tests is merged and the coverage numbers improve
meaningfully, raise the floor to lock in the improvement:

1. Run `python3 scripts/advance-ratchet.py` to bump every `line_min` by 2%.
2. Run the coverage build locally (see *Generating Coverage Locally* below) to
   confirm the new thresholds are met.
3. Update the baseline table in this file with the new numbers and commit SHA.
4. Commit `.coverage-thresholds.json` and `docs/coverage.md` together.

The active thresholds are always the `line_min` values in
`.coverage-thresholds.json`; the schedule tables above show the planned path to
the 90% target for reference.

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

# 4. Generate JSON summary + top-20 branch gap list
gcovr \
  --root . \
  --object-directory build-coverage \
  --gcov-ignore-parse-errors=all \
  --gcov-ignore-errors=source_not_found \
  --filter '^(src|dggui|plugingui|plugin|drumgizmo|test)/' \
  --json-summary-pretty \
  --json-summary coverage-summary.json

python3 scripts/generate-coverage-gap-report.py \
  --summary coverage-summary.json \
  --output docs/coverage-top20-file-gaps.md

# Open coverage/index.html in a browser
```

---

## Updating the Baseline

Whenever a significant batch of new tests is merged, update the baseline table
in this file with the new numbers and the commit SHA. This keeps the document
useful as a historical reference.
