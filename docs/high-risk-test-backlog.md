# High-Risk Untested Paths Backlog

This document tracks the highest-risk test gaps in DrumGizmo's core runtime
paths and converts them into a prioritized implementation backlog.

## Scope Audited

- Audio cache (`src/audiocache*.cc`)
- Parsing (`src/configparser.cc`, `src/dgxmlparser.cc`, `src/midimapparser.cc`)
- MIDI mapping/input (`src/midimapper.cc`, `src/audioinputenginemidi.cc`)
- Engine lifecycle (`src/drumgizmo.cc`)

Current coverage references used in this audit:

- `test/audiocachetest.cc`
- `test/audiocachefiletest.cc`
- `test/audiocacheeventhandlertest.cc`
- `test/configparsertest.cc`
- `test/dgxmlparsertest.cc`
- `test/midimapparsertest.cc`
- `test/midimappertest.cc`
- `test/enginetest.cc`

## Prioritization Criteria

Each scenario is scored 1-5 on four dimensions and ranked by weighted total:

- `Impact` (weight 0.35): User-visible breakage risk (silence, wrong triggers,
  crashes, corrupted output).
- `Runtime criticality` (weight 0.30): How frequently the path executes during
  normal real-time playback.
- `Complexity` (weight 0.20): Concurrency/state-space complexity and branching.
- `Bug history signal` (weight 0.15): Proxy signal from legacy code paths,
  TODO/assert density, and known fragile behavior patterns in similar modules.

Formula:

`priority_score = impact*0.35 + runtime_criticality*0.30 + complexity*0.20 + bug_history_signal*0.15`

## Prioritized Gaps

| Rank | ID | Area | Untested scenario | Score | Status |
| ---- | -- | ---- | ----------------- | ----: | ------ |
| 1 | HR-01 | Audio cache | Event queue behavior under concurrent `setChunkSize()`, `close()`, and pending async reads | 4.75 | implemented (`test/audiocacheeventhandlertest.cc`) |
| 2 | HR-02 | Engine lifecycle | Resampling path behavior in `DrumGizmo::run()` (ratio != 1.0), including latency and buffer transitions | 4.55 | in progress |
| 3 | HR-03 | MIDI input/mapping | `AudioInputEngineMidi::processNote()` semantic edge cases (note-on velocity 0, aftertouch choke, short packets, multi-map fanout) | 4.45 | implemented (`test/audioinputenginemiditest.cc`) |
| 4 | HR-04 | Parsing | Robustness for malformed/partial XML in drumkit/instrument parsing beyond happy-path fixture shapes | 4.20 | in progress |
| 5 | HR-05 | Audio cache | Underrun and dummy-id behavior when preload/file validity/cache-pool limits fail under load | 4.05 | in progress |
| 6 | HR-06 | MIDI map parser | Repeat parse state handling and invalid-map entry filtering behavior in `MidiMapParser` | 3.85 | implemented (`test/midimapparsertest.cc`) |
| 7 | HR-07 | Engine lifecycle | Kit switching assertions in `test/enginetest.cc` are absent despite heavy lifecycle churn loops | 3.75 | in progress |
| 8 | HR-08 | Config parser | Version handling and missing-node edge behavior not fully asserted against persisted parser state | 3.55 | in progress |

## Recently Addressed

- `TST-AC-01` (`test/audiocacheeventhandlertest.cc`)
  - Adds threaded close-event release coverage (ID is reusable after queued close is processed).
  - Adds threaded/non-threaded parity coverage for identical `pushLoadNextEvent()` reads.
  - Complements existing queue-deduplication and `setChunkSize()` queue-clear assertions.

## Follow-Up Work Items

### HR-01: Audio cache concurrent queue/chunk transitions

- Why high risk: asynchronous queueing + manual memory ownership in
  `AudioCacheEventHandler` and `AudioCache` combines lock, semaphore, and
  deferred close behavior.
- Current gap signal: `test/audiocacheeventhandlertest.cc` only constructs the
  handler and has no event-processing assertions.
- Implementation backlog item: `TST-AC-01`
  - Add deterministic tests for:
    - deduplication of same `(file, pos)` load events across channels,
    - `setChunkSize()` while events are queued,
    - `close()` ordering vs queued load events,
    - non-threaded vs threaded parity.
  - Target files: `test/audiocacheeventhandlertest.cc`, `test/audiocachetest.cc`.

### HR-02: Engine resampling runtime path

- Why high risk: this path touches sample-rate conversion state, per-channel
  buffers, and mixed internal/external output buffers during every block.
- Current gap signal: `test/enginetest.cc` does not assert resampling branch
  behavior and mostly stress-runs kit switching loops.
- Implementation backlog item: `TST-ENG-01`
  - Add tests for:
    - `ratio != 1.0` path in `DrumGizmo::run()`,
    - `setSamplerate()` quality clamping and prefill behavior,
    - expected `getLatency()` changes with/without resampling.
  - Target files: `test/enginetest.cc` (+ focused engine fixture helpers).

### HR-03: MIDI event semantics and mapping fanout

- Why high risk: mapping and trigger semantics directly affect hit correctness
  and choke behavior at runtime.
- Current gap signal: mapping tables are tested, but raw MIDI packet processing
  in `AudioInputEngineMidi::processNote()` is untested.
- Implementation backlog item: `TST-MIDI-01`
  - Add tests for:
    - short packet ignore (`len < 3`),
    - note-on with velocity 0 (must not emit OnSet),
    - aftertouch > 0 emitting choke,
    - one note mapped to multiple instruments.
  - Target file: new `test/audioinputenginemiditest.cc`.

### HR-04: Parser robustness on malformed/partial XML

- Why high risk: malformed user kit XML should fail safely without producing
  partially inconsistent runtime data.
- Current gap signal: parser tests are primarily valid-shape fixtures plus one
  simple invalid case per parser.
- Implementation backlog item: `TST-PARSE-01`
  - Add negative/edge parser cases for:
    - missing required attributes,
    - unknown/unsupported versions,
    - repeated/conflicting nodes,
    - invalid numeric conversions and boundary values.
  - Target files: `test/dgxmlparsertest.cc`, `test/configparsertest.cc`,
    `test/midimapparsertest.cc`.

### HR-05: Audio cache underrun and pool exhaustion behavior

- Why high risk: fallback-to-silence and underrun counters protect real-time
  stability when resources are constrained.
- Current gap signal: happy-path reads are heavily tested, but forced-pool
  exhaustion and sustained underrun transitions are not asserted.
- Implementation backlog item: `TST-AC-02`
  - Add tests for:
    - `CACHE_DUMMYID` path when ID pool is exhausted,
    - invalid/unloaded file path through `open()` and `next()`,
    - underrun counter increments across repeated fallback blocks.
  - Target file: `test/audiocachetest.cc`.

### HR-06: MidiMapParser state and filtering edge cases

- Why medium risk: parser state is mutable and currently append-based.
- Current gap signal: tests do not call `parseFile()` multiple times on the
  same parser instance and do not assert all invalid map-entry variants.
- Implementation backlog item: `TST-MIDI-02`
  - Add tests for:
    - repeated parse on one parser object,
    - empty/missing `note` and `instr` attributes,
    - root-node absent behavior.
  - Target file: `test/midimapparsertest.cc`.

### HR-07: Engine kit-switch lifecycle assertions

- Why medium risk: rapid drumkit swapping is exercised but not verified, so
  regressions could pass silently.
- Current gap signal: `test/enginetest.cc` primarily checks survivability.
- Implementation backlog item: `TST-ENG-02`
  - Add assertions for:
    - expected event reset behavior on kit change,
    - no stale instrument mapping after switch,
    - deterministic state after repeated toggles.
  - Target file: `test/enginetest.cc`.

### HR-08: Config parser state/version behavior

- Why medium risk: config parsing is user-entry-point sensitive but low
  runtime-frequency.
- Current gap signal: no explicit assertions that parser state is safe after
  parse failures or unsupported version declarations.
- Implementation backlog item: `TST-CFG-01`
  - Add tests for:
    - unsupported `version` rejection,
    - missing `<config>` node behavior,
    - value lookup behavior after failed parse attempts.
  - Target file: `test/configparsertest.cc`.

## Execution Order Recommendation

Implement in this order for highest risk reduction per effort:

1. `TST-AC-01`
2. `TST-ENG-01`
3. `TST-MIDI-01`
4. `TST-PARSE-01`
5. `TST-AC-02`

Items 6-8 can proceed in parallel once top runtime-critical gaps are covered.
