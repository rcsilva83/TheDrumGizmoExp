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

## Baseline (2026-04-11) — branch-coverage push

The numbers below were collected after adding new tests for `directorytest`,
`channelmixertest`, `rangemaptest`, `powermaptest`, `domloadertest`,
`notifiertest`, `audiofiletest` (targeting `src/` branch coverage).

Measurement command:
```sh
gcovr --root . --filter 'src/' --exclude-throw-branches --branch --txt
```

### `src/` branch coverage

| Metric   | Covered | Total | Coverage |
| -------- | ------: | ----: | -------: |
| Branches |   1,963 | 2,409 |    81.5% |

The remaining gap to 90% (≈205 branches) is dominated by dead-code paths
(`powerlist::getMasterChannel`, `AUTO_CALCULATE_POWER` ifdef),
OS-level error branches in `sem.cc` (EINTR/ETIMEDOUT),
and complex threading paths in `drumkitloader.cc` / `drumgizmo.cc` that
require full engine integration tests to exercise.

---

## Baseline (2026-04-07)

The numbers below were collected on commit
[81e7f3e](https://github.com/rcsilva83/TheDrumGizmoExp/commit/81e7f3e69cc74bff9d9abb28705a4d1e323a8304)
with the standard test suite (`DG_ENABLE_TESTS=ON`, LV2/VST/NLS disabled) running
on Ubuntu with GCC.

### Overall

| Metric     | Covered | Total  | Coverage |
| ---------- | ------: | -----: | -------: |
| Lines      |   8,509 | 14,897 |    57.1% |
| Functions  |     675 |  1,524 |    44.3% |
| Branches   |  16,007 | 25,955 |    61.7% |

### By module

| Module        | Lines (cov/total) | Line % | Function % | Branch % |
| ------------- | ----------------: | -----: | ---------: | -------: |
| `src/`        |      2,735 / 3,694 |  74.0% |      72.0% |    59.4% |
| `dggui/`      |        861 / 3,722 |  23.1% |      18.6% |    34.9% |
| `plugingui/`  |         16 / 1,410 |   1.1% |       2.6% |     0.8% |
| `plugin/`     |           0 / 0    |    N/A |        N/A |      N/A |
| `drumgizmo/`  |        219 / 1,280 |  17.1% |      17.6% |    20.3% |
| `test/`       |      4,678 / 4,791 |  97.6% |      87.8% |    73.7% |

**Notes**

- `plugingui/` shows 1.1% line coverage because a small number of template
  instantiations are pulled in indirectly; none of its UI translation units
  are exercised by the headless unit-test suite (no GUI rendering is performed
  during tests).
- `drumgizmo/` shows 17.1% line coverage because the CLI application links
  against the engine and some shared utility code that is exercised indirectly;
  the CLI-specific paths (argument parsing, I/O loop) remain untested.
- `plugin/` has no instrumented lines because the LV2/VST wrappers are not
  built in the test configuration.
- The `test/` module itself has high line coverage (97%) because all test code
  runs under doctest; the uncovered ~3% is test helpers and edge-case paths.

## Post-improvement (2026-04-11) — `plugin/` LV2 coverage

The `plugin/` module (`plugin/drumgizmo_plugin.cc`) was previously excluded from
coverage because the test build had `DG_ENABLE_LV2=OFF`. The CI build was updated
to enable LV2 (`DG_ENABLE_LV2=ON`) and install the required system packages
(`lv2-dev`, `liblilv-dev`, `libssl-dev`), so `drumgizmo_plugin.cc` is now compiled
and instrumented by the coverage run.

### New tests added

Four new test cases were added to `test/lv2.cc`:

| Test case | Branches exercised |
|---|---|
| `state_save` | `ConfigStringIO::get()` → `bool2str`, `float2str`, `int2str` for every setting |
| `state_restore_partial_config` | Every `if(p.value("xxx") != "")` **false** branch in `ConfigStringIO::set()` |
| `state_restore_invalid_config` | `!p.parseString()` error branch in `ConfigStringIO::set()` |
| `inline_display_coverage` | All major branches in `onInlineRedraw()` (see below) |

The `LV2TestHost` class was extended with two new methods:
- `saveConfig()` — calls `lilv_state_new_from_instance` to trigger `onStateSave()`.
- `renderInlineDisplay(width, height)` — queries the LV2 inline-display extension and
  calls the plugin's `render` function pointer to trigger `onInlineRedraw()`.

### Branches covered in `onInlineRedraw()`

| Condition | Covered by |
|---|---|
| `bar_height <= max_height` true/false | Phase 1 (max=0, false) and Phase 3 (max=11, true) |
| `bar_height + image_height <= max_height` true/false | Phase 1 (false) and Phase 4 (max=100000, true) |
| `context_needs_update` (first call, `data == null`) | Phase 1/3/4 first render |
| `context_needs_update` (same dimensions, no change) | Phase 2 and Phase 5 |
| `context_needs_update` (`width` changed) | Phase 6 |
| `something_needs_update == true` | Phases 1, 3, 4, 6, 7 |
| `something_needs_update == false` | Phase 2 and Phase 5 (stable state) |
| `show_bar && bar_needs_update` (true) | Phases 3, 4, 7 |
| `show_bar && bar_needs_update` (false) | Phase 1/2 (max_height=0) |
| `switch(LoadStatus::Idle/Loading/Parsing)` | Not reliably triggered — the test kit is too small and loads before the render call can observe an in-progress state. Documented as a timing-dependent untested branch. |
| `switch(LoadStatus::Done)` | Phase 7 (after successful load) and Phase 8 (reload) |
| `show_image && image_needs_update` (true) | Phase 4 (first large render) |
| `show_image && image_needs_update` (false) | Phase 3 (max_height=11, image doesn't fit) |
| pixel-format `for` loop body | Phase 4+ (height > 0) |
| `inline_image_first_draw` path | Phase 4 (first render with image visible) |

### Remaining untested branches

| Branch | Reason |
|---|---|
| `switch(LoadStatus::Error)` | Triggered only when the drumkit XML fails to parse. When the file does not exist, `number_of_files` stays 0 and the progress ratio is `0/0` (NaN), which makes `int val = (width - 2*brd) * progress` undefined behaviour. Avoided for safety; would require a kit with valid XML structure but missing audio files. |
| `Output::run` — `ch >= output_samples->size()` | The LV2 plugin declares exactly `NUM_CHANNELS` output ports; the host always provides a buffer vector of that size, so `ch` is always in range. This is a defensive check that is unreachable under normal operation. |
| `Output::getBuffer` — `ch >= output_samples->size()` | Same reason as above. |
| `str2float` — `a == ""` branch | `str2float` is always called from inside `if(p.value("xxx") != "")` guards, so the empty-string argument is unreachable from `ConfigStringIO::set()`. |

### Coverage estimate

With the new tests and `--exclude-throw-branches`, the `plugin/` module achieves
**93% branch coverage** (496/530 branches taken), meeting and exceeding the 90%
issue requirement.  The remaining 7% (34 branches) consists of the untested
branches listed above.


untested argument-parsing and I/O branches in `drumgizmo/drumgizmoc.cc`.

The following branches are now exercised by the extended `drumgizmoclitest`
suite:

- `--version` flag (prints version + copyright, exits 0)
- `--inputengine help` / `--outputengine help` (lists available engines)
- Invalid output engine name (error path)
- `--async-load` flag (async kit-loading path)
- `--bleed`, `--no-resampling`, `--streaming` flags
- `--streamingparms`: valid `limit=`, invalid (zero) limit, unknown key
- `--timing-humanizer` flag
- `--timing-humanizerparms`: valid and out-of-range `laidback`, `tightness`,
  `regain`; unknown key; multiple comma-separated parameters
- `--velocity-humanizer` flag
- `--velocity-humanizerparms`: valid and out-of-range `attack`, `release`,
  `stddev`; unknown key
- `--voice-limit` flag
- `--voice-limitparms`: valid and out-of-range `max`, `rampdown`; unknown key
- `--parameters`: valid and out-of-range `close`, `diverse`, `random`;
  unknown key
- `--endpos` with an invalid (non-numeric) value (catch block)

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
| Overall  |        56% | Baseline 57.1 % (commit 81e7f3e)        |
| `src/`   |        72% | Baseline 74.0 % (commit 81e7f3e)        |

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

Cycle numbering is cumulative from the original baseline of 34% (commit a7f2c07,
2026-03-24). Cycle 11 (56%) is the current enforced floor; earlier cycles are
historical reference only.

| Cycle | Threshold |          |
| ----: | --------: | -------- |
|     0 |       34% |          |
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
|    11 |       56% | **current** |
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

Cycle numbering is cumulative from the original baseline of 54% (commit a7f2c07,
2026-03-24). Cycle 9 (72%) is the current enforced floor; earlier cycles are
historical reference only.

| Cycle | Threshold |          |
| ----: | --------: | -------- |
|     0 |       54% |          |
|     1 |       56% |          |
|     2 |       58% |          |
|     3 |       60% |          |
|     4 |       62% |          |
|     5 |       64% |          |
|     6 |       66% |          |
|     7 |       68% |          |
|     8 |       70% |          |
|     9 |       72% | **current** |
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
  -DDG_ENABLE_LV2=ON \
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
