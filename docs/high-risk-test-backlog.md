# High-Risk Untested Paths Backlog

This document tracks the highest-risk test gaps in DrumGizmo's core runtime
paths and converts them into a prioritized implementation backlog.

## Scope Audited

### Audit round 1 (2026-03-24)

- Audio cache (`src/audiocache*.cc`)
- Parsing (`src/configparser.cc`, `src/dgxmlparser.cc`, `src/midimapparser.cc`)
- MIDI mapping/input (`src/midimapper.cc`, `src/audioinputenginemidi.cc`)
- Engine lifecycle (`src/drumgizmo.cc`)

Coverage references used in audit round 1:

- `test/audiocachetest.cc`
- `test/audiocachefiletest.cc`
- `test/audiocacheeventhandlertest.cc`
- `test/configparsertest.cc`
- `test/dgxmlparsertest.cc`
- `test/midimapparsertest.cc`
- `test/midimappertest.cc`
- `test/enginetest.cc`

### Audit round 2 (2026-04-03)

HR-01 through HR-08 were all implemented between 2026-03-24 and 2026-04-03
(see PRs #112, #111 and related). This round re-audits the same modules plus
`src/inputprocessor.cc` now that the first-wave gaps are closed.

Additional coverage references added in round 2:

- `test/audioinputenginemiditest.cc` (new file, covers TST-MIDI-01)
- Updated coverage gaps from `docs/coverage-top20-file-gaps.md`

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

### Round 1 items (all completed)

| Rank | ID | Area | Untested scenario | Score | Status |
| ---- | -- | ---- | ----------------- | ----: | ------ |
| 1 | HR-01 | Audio cache | Event queue behavior under concurrent `setChunkSize()`, `close()`, and pending async reads | 4.75 | ✅ Done (TST-AC-01) |
| 2 | HR-02 | Engine lifecycle | Resampling path behavior in `DrumGizmo::run()` (ratio != 1.0), including latency and buffer transitions | 4.55 | ✅ Done (TST-ENG-01) |
| 3 | HR-03 | MIDI input/mapping | `AudioInputEngineMidi::processNote()` semantic edge cases (note-on velocity 0, aftertouch choke, short packets, multi-map fanout) | 4.45 | ✅ Done (TST-MIDI-01) |
| 4 | HR-04 | Parsing | Robustness for malformed/partial XML in drumkit/instrument parsing beyond happy-path fixture shapes | 4.20 | ✅ Done (TST-PARSE-01) |
| 5 | HR-05 | Audio cache | Underrun and dummy-id behavior when preload/file validity/cache-pool limits fail under load | 4.05 | ✅ Done (TST-AC-02) |
| 6 | HR-06 | MIDI map parser | Repeat parse state handling and invalid-map entry filtering behavior in `MidiMapParser` | 3.85 | ✅ Done (TST-MIDI-02) |
| 7 | HR-07 | Engine lifecycle | Kit switching assertions in `test/enginetest.cc` are absent despite heavy lifecycle churn loops | 3.75 | ✅ Done (TST-ENG-02) |
| 8 | HR-08 | Config parser | Version handling and missing-node edge behavior not fully asserted against persisted parser state | 3.55 | ✅ Done (TST-CFG-01) |

### Round 2 items (new findings)

| Rank | ID | Area | Untested scenario | Score | Status |
| ---- | -- | ---- | ----------------- | ----: | ------ |
| 9  | HR-09 | Input processor | Voice-limit enforcement path in `InputProcessor::limitVoices()` when `enable_voice_limit` is true | 4.10 | ✅ Done (TST-INPUT-01) |
| 10 | HR-10 | Input processor | `processOnset()` with out-of-bounds instrument ID logs an error and drops the event; not directly asserted | 3.80 | ✅ Done (TST-INPUT-02) |
| 11 | HR-11 | Input processor | Choke-group rampdown via `applyChokeGroup()` when two instruments share a group name | 3.60 | ✅ Done (TST-INPUT-03) |

## Follow-Up Work Items

### ✅ HR-01: Audio cache concurrent queue/chunk transitions

- Implementation backlog item: `TST-AC-01` — **Completed**
- Implemented in: `test/audiocacheeventhandlertest.cc`, `test/audiocachetest.cc`
- Key subcases added:
  - `setChunkSize_with_queued_close_event_clears_queue`
  - `setChunkSize_different_value_disables_active_ids`
  - `deduplicate_load_events_threaded_both_channels_ready`
  - `updateChunkSizeWhileEventsQueued`, `closeWhileLoadQueued`

### ✅ HR-02: Engine resampling runtime path

- Implementation backlog item: `TST-ENG-01` — **Completed**
- Implemented in: `test/enginetest.cc`
- Key subcases added:
  - `setSamplerateQualityClamping`
  - `getLatencyIncludesResamplerWhenEnabled`
  - `getLatencyExcludesResamplerWhenDisabled`
  - `resamplingRecommendedSetWhenRatioNotOne`
  - `runWithRatioNotOne`, `runWithRatioOne`
  - `runtimeEnableResamplingToggleAffectsLatency`
  - `runtimeResamplingQualityChangeAffectsLatency`

### ✅ HR-03: MIDI event semantics and mapping fanout

- Implementation backlog item: `TST-MIDI-01` — **Completed**
- Implemented in: `test/audioinputenginemiditest.cc` (new file)
- Key subcases added:
  - `regression_0_9_19_note_on_velocity_zero_generates_no_event`
  - `regression_0_9_20_aftertouch_velocity_gt_zero_generates_choke`
  - `short_buffer_generates_no_event`
  - `one_note_mapped_to_multiple_instruments_generates_multiple_events`
  - `loadMidiMap_empty_filename_returns_false`

### ✅ HR-04: Parser robustness on malformed/partial XML

- Implementation backlog item: `TST-PARSE-01` — **Completed**
- Implemented in: `test/dgxmlparsertest.cc`, `test/configparsertest.cc`,
  `test/midimapparsertest.cc`
- Key subcases added:
  - `drumkitMalformedXml`, `drumkitMissingRequiredAttributes`
  - `instrumentMissingRequiredAttributes`
  - `instrumentVersionAndFailureMatrix`, `normalizedAttributeMatrix`
  - `unsupportedVersionRejection`, `missingConfigNode`
  - `edgeCaseMatrix` (all parsers)

### ✅ HR-05: Audio cache underrun and pool exhaustion behavior

- Implementation backlog item: `TST-AC-02` — **Completed**
- Implemented in: `test/audiocachetest.cc`
- Key subcases added:
  - `nullFrontBufferUnderrun`
  - `poolExhaustionUnderrunFromOpen`
  - `unloadedFileNextUnderrun`
  - `repeatedDummyIdUnderrunIncrements`
  - `dummyIdOperations`

### ✅ HR-06: MidiMapParser state and filtering edge cases

- Implementation backlog item: `TST-MIDI-02` — **Completed**
- Implemented in: `test/midimapparsertest.cc`
- Key subcases added:
  - `repeatValidParseFileAccumulatesEntries`
  - `wrongRootProducesNoMappings`
  - `emptyMidimapRootProducesNoMappings`
  - `mapEntryWithBothAttributesAbsentIsSkipped`

### ✅ HR-07: Engine kit-switch lifecycle assertions

- Implementation backlog item: `TST-ENG-02` — **Completed**
- Implemented in: `test/enginetest.cc`
- Key subcases added:
  - `kitSwitchResetsActiveEvents`
  - `kitSwitchNoStaleInstrumentMappingAfterSwitch`
  - `kitSwitchDeterministicStateAfterRepeatedToggles`

### ✅ HR-08: Config parser state/version behavior

- Implementation backlog item: `TST-CFG-01` — **Completed**
- Implemented in: `test/configparsertest.cc`
- Key subcases added:
  - `unsupportedVersionRejection`
  - `missingConfigNode`
  - `recoveryAfterVersionFailure`
  - `recoveryAfterParseFailure`

### ✅ HR-09: Voice-limit enforcement in InputProcessor

- Why high risk: `InputProcessor::limitVoices()` silently ramps down the
  oldest drum-hit voice when the per-instrument cap is exceeded. A bug here
  produces incorrect or stale audio output with no error signal. The entire
  `settings.enable_voice_limit` true-branch in `processOnset()` (line 265)
  and the full `limitVoices()` function body (~60 lines) are currently
  uncovered.
- Current gap signal: `src/inputprocessor.cc` has 5.7% branch coverage
  (rank 13 in coverage-top20-file-gaps); none of the current `enginetest.cc`
  subcases set `settings.enable_voice_limit = true`.
- Implementation backlog item: `TST-INPUT-01` — **Completed**
- Implemented in: `test/enginetest.cc`
- Key subcase added:
  - `voiceLimitCapsActiveVoicesDoesNotCrash`

### ✅ HR-10: processOnset with out-of-bounds instrument ID

- Why high risk: if the input engine or MIDI mapper supplies an instrument
  index beyond `kit.instruments.size()`, `processOnset()` sets `instr =
  nullptr`, logs an error, and returns `false`, dropping the event. This path is
  not directly asserted; a regression that accidentally crashes or corrupts
  state would be invisible.
- Current gap signal: the `instrument_id >= kit.instruments.size()` false-
  branch in `processOnset()` (line 224) and `processChoke()` (line 323) are
  not covered by any test.
- Implementation backlog item: `TST-INPUT-02` — **Completed**
- Implemented in: `test/enginetest.cc`
- Key subcase added:
  - `processOnsetOutOfBoundsInstrumentIdIsIgnored`

### ✅ HR-11: Choke-group rampdown via applyChokeGroup

- Why high risk: choke-group behavior is central to realistic cymbal/hi-hat
  simulation. If `applyChokeGroup()` fails to find matching group names, or
  the rampdown is never applied, the wrong instruments continue playing.
- Implementation backlog item: `TST-INPUT-03` — **Completed**
- Implemented in: `test/enginetest.cc`
- Key subcases added:
  - `chokeGroupMutesOtherInstrumentSameGroup`
  - `applyDirectedChokeRampsDownVictimEvents`
  - `applyChokeGroupNoEventsForSameGroup`
  - `applyChokeGroupNoEventOnFirstOnset`
  - `applyChokeGroupInstrumentWithNoGroup`
  - `applyChokeGroupTwoInstrumentsSameGroup`
  - `applyChokeGroupChannelOutOfBounds`

## Execution Order Recommendation

### Round 1 (all completed)

1. `TST-AC-01` ✅
2. `TST-ENG-01` ✅
3. `TST-MIDI-01` ✅
4. `TST-PARSE-01` ✅
5. `TST-AC-02` ✅
6. `TST-MIDI-02` ✅
7. `TST-ENG-02` ✅
8. `TST-CFG-01` ✅

### Round 2

All round 2 items were implemented. The order below reflects the actual
implementation sequence:

1. `TST-INPUT-01` ✅ (voice limit — highest score, pure settings toggle)
2. `TST-INPUT-02` ✅ (out-of-bounds instrument ID — simple custom stub)
3. `TST-INPUT-03` ✅ (choke group — required kit XML extension)
