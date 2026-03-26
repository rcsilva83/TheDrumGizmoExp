#!/usr/bin/env python3
"""
Advance coverage ratchet thresholds toward the configured target.

Reads .coverage-thresholds.json, increments every ``line_min`` value by
``ratchet.increment`` percentage points (capped at ``ratchet.target``), and
writes the file back in place.

Usage:
    python3 scripts/advance-ratchet.py [--config PATH] [--dry-run]

Defaults:
    --config  .coverage-thresholds.json  (relative to CWD)
"""

import argparse
import json
import sys


def advance(current, increment, target):
    """Return the next ratchet position, never exceeding *target*."""
    return min(current + increment, target)


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Advance coverage ratchet thresholds toward target"
    )
    parser.add_argument(
        "--config",
        default=".coverage-thresholds.json",
        help="Path to the thresholds JSON file (default: .coverage-thresholds.json)",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print proposed changes without writing to disk",
    )
    args = parser.parse_args(argv)

    with open(args.config, encoding="utf-8") as fh:
        config = json.load(fh)

    ratchet = config.get("ratchet", {})
    target = int(ratchet.get("target", 90))
    increment = int(ratchet.get("increment", 2))

    changes = []
    at_target = []

    # Overall threshold
    current_threshold = config["overall"]["line_min"]
    next_threshold = advance(current_threshold, increment, target)
    if next_threshold != current_threshold:
        changes.append(("overall", current_threshold, next_threshold))
        config["overall"]["line_min"] = next_threshold
    else:
        at_target.append("overall")

    # Per-module thresholds
    for mod, cfg in config.get("modules", {}).items():
        current_threshold = cfg["line_min"]
        next_threshold = advance(current_threshold, increment, target)
        if next_threshold != current_threshold:
            changes.append((f"modules.{mod}", current_threshold, next_threshold))
            cfg["line_min"] = next_threshold
        else:
            at_target.append(f"modules.{mod}")

    if not changes and not at_target:
        print("No thresholds found to advance.")
        return

    for name, old_val, new_val in changes:
        print(f"  {name}: {old_val}% → {new_val}%")

    for name in at_target:
        print(f"  {name}: already at target ({target}%)")

    if not changes:
        print("All thresholds already at target. Nothing to update.")
        return

    if args.dry_run:
        print("\n[dry-run] No changes written.")
        return

    with open(args.config, "w", encoding="utf-8") as fh:
        json.dump(config, fh, indent=2, ensure_ascii=False)
        fh.write("\n")

    print(f"\nUpdated {args.config}.")
    print(
        "Commit the updated .coverage-thresholds.json and update docs/coverage.md "
        "with the new baseline table."
    )


if __name__ == "__main__":
    main()
