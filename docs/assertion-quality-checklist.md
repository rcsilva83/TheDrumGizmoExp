# Assertion Quality Checklist

Use this checklist when adding or reviewing tests to keep assertions focused on
observable behavior instead of survivability.

For complete test-writing conventions and doctest examples, see
[`test-authoring-guidelines.md`](test-authoring-guidelines.md).

## Checklist

- Verify behavior, not just execution: avoid tests that only prove code does not
  crash.
- Assert outputs and state transitions: check return values, object state, and
  counters before and after the action under test.
- Cover both success and failure paths where the API supports both.
- Assert invariants that should always hold (sizes, ordering, identity,
  boundaries).
- Prefer specific expectations over broad truthy checks (for example
  `CHECK_EQ(size, 3u)` over `CHECK(!items.empty())` when exact size matters).
- Include at least one regression-facing assertion that would fail if a known
  behavior changes.

## Review Gate

For test-focused pull requests, reviewers should confirm that changed tests
pass this checklist before approval.
