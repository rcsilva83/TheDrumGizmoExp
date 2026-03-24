# Test Authoring Guidelines

This guide standardizes how DrumGizmo tests are written so that failures are
high-signal, deterministic, and easy to maintain.

Tests use [doctest](https://github.com/doctest/doctest) and should follow the
project conventions in `CONTRIBUTING.md` and `AGENTS.md`.

## Core Patterns

### 1. Arrange-Act-Assert (AAA)

Keep tests readable by separating setup, behavior, and expectations.

```cpp
#include <doctest/doctest.h>

TEST_CASE("midimap resolves mapped note")
{
    // Arrange
    MidiMapper mapper;
    mapper.setMap(36, 38);

    // Act
    const int mapped = mapper.lookup(36);

    // Assert
    CHECK_EQ(38, mapped);
}
```

### 2. Deterministic setup

Do not depend on wall clock time, random seeds, or machine-specific state.
Inputs should be explicit and repeatable.

- Use fixed values instead of generated values unless the seed is fixed.
- Build fixtures with all required state in the test itself.
- Keep tests independent; each test should pass when run alone.

### 3. One behavior per test

Each `TEST_CASE` should target a single behavior. Use separate tests for
distinct outcomes (for example success and failure).

```cpp
TEST_CASE("bytesize parser accepts kib suffix")
{
    ByteSizeParser parser;
    CHECK_EQ(1024u, parser.parse("1KiB"));
}

TEST_CASE("bytesize parser rejects unknown suffix")
{
    ByteSizeParser parser;
    CHECK_EQ(0u, parser.parse("1XYZ"));
}
```

### 4. Assert concrete outcomes and invariants

Use specific assertions that document expected behavior.

- Prefer exact checks (`CHECK_EQ`, `REQUIRE_EQ`) when exact values matter.
- Check state transitions before and after the action under test.
- Assert invariants (size, ordering, identity, bounds), not only one output.

```cpp
TEST_CASE("cache insert updates size and lookup")
{
    AudioCache cache;
    CHECK_EQ(0u, cache.size());

    cache.insert(17, 1234);

    CHECK_EQ(1u, cache.size());
    CHECK_EQ(1234, cache.lookup(17));
}
```

### 5. Cover failure behavior deliberately

If an API has failure paths, test them explicitly and verify the expected
fallback behavior.

```cpp
TEST_CASE("config parser returns false on malformed input")
{
    ConfigParser parser;
    CHECK_EQ(false, parser.parse("<config><broken></config>"));
    CHECK_EQ(0u, parser.entryCount());
}
```

## Anti-Patterns To Avoid

### Trivial assertions

Bad:

```cpp
TEST_CASE("parse does not crash")
{
    ByteSizeParser parser;
    parser.parse("1KiB");
    CHECK(true);
}
```

Better:

```cpp
TEST_CASE("parse returns expected bytes for kib")
{
    ByteSizeParser parser;
    CHECK_EQ(1024u, parser.parse("1KiB"));
}
```

### Brittle implementation coupling

Avoid asserting on internal/private details that are not part of the behavior
contract.

- Do not assert call order across private helpers.
- Do not lock tests to incidental internal counters.
- Prefer public API expectations and externally visible state.

### Over-mocking

Mock only at clear boundaries (filesystem, network, audio device backends).
Over-mocking internal collaborators makes refactors expensive and tests noisy.

- Prefer real value objects and lightweight fixtures.
- Mock only what is needed to isolate the behavior under test.

## Review Checklist For Authors

Before opening a PR, confirm each new or changed test:

- Follows AAA structure.
- Is deterministic and independent.
- Targets one behavior per `TEST_CASE`.
- Uses strong assertions with concrete expected values.
- Includes relevant failure-path or regression-facing assertions when needed.

For a compact review gate, also apply
[`assertion-quality-checklist.md`](assertion-quality-checklist.md).
