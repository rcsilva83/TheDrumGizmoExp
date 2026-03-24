#!/usr/bin/env python3
"""
Check that a Cobertura coverage.xml report meets the thresholds defined in
.coverage-thresholds.json.

Exits with status 1 if any threshold is not met.
When running inside GitHub Actions, appends a Markdown table to
GITHUB_STEP_SUMMARY for reviewer visibility.

Usage:
    python3 scripts/check-coverage.py [--config PATH] [--xml PATH]

Defaults:
    --config  .coverage-thresholds.json  (relative to CWD)
    --xml     coverage.xml               (relative to CWD)
"""

import argparse
import json
import os
import sys
import xml.etree.ElementTree as ET


def line_stats(root, filename_prefix=None):
    """Return (covered_lines, total_lines) from a Cobertura XML element tree.

    When *filename_prefix* is given, only <class> elements whose ``filename``
    attribute starts with that prefix are considered.
    """
    covered = 0
    total = 0
    for cls in root.findall(".//class"):
        fname = cls.attrib.get("filename", "")
        if filename_prefix and not fname.startswith(filename_prefix):
            continue
        for line in cls.findall("lines/line"):
            total += 1
            if int(line.attrib.get("hits", "0")) > 0:
                covered += 1
    return covered, total


def percent(covered, total):
    return covered / total * 100 if total > 0 else 0.0


def main(argv=None):
    parser = argparse.ArgumentParser(description="Check coverage thresholds")
    parser.add_argument(
        "--config",
        default=".coverage-thresholds.json",
        help="Path to the thresholds JSON file (default: .coverage-thresholds.json)",
    )
    parser.add_argument(
        "--xml",
        default="coverage.xml",
        help="Path to the Cobertura XML report (default: coverage.xml)",
    )
    args = parser.parse_args(argv)

    # Load threshold configuration
    with open(args.config, encoding="utf-8") as fh:
        config = json.load(fh)

    # Parse coverage XML
    tree = ET.parse(args.xml)
    root = tree.getroot()

    results = []
    failed = False

    # Overall coverage
    cov, tot = line_stats(root)
    pct = percent(cov, tot)
    min_pct = config["overall"]["line_min"]
    ok = pct >= min_pct
    results.append(("Overall", pct, min_pct, ok, cov, tot))
    if not ok:
        failed = True

    # Per-module coverage
    for mod, cfg in config.get("modules", {}).items():
        prefix = mod.rstrip("/") + "/"
        cov, tot = line_stats(root, filename_prefix=prefix)
        pct = percent(cov, tot)
        min_pct = cfg["line_min"]
        ok = pct >= min_pct
        results.append((f"`{mod}/`", pct, min_pct, ok, cov, tot))
        if not ok:
            failed = True

    # Write GitHub step summary when running in GitHub Actions
    summary_path = os.environ.get("GITHUB_STEP_SUMMARY", "")
    if summary_path:
        with open(summary_path, "a", encoding="utf-8") as fh:
            fh.write("## Coverage threshold checks\n\n")
            fh.write("| Scope | Lines covered | Line % | Min threshold | Status |\n")
            fh.write("| --- | ---: | ---: | ---: | :---: |\n")
            for name, pct, min_pct, ok, cov, tot in results:
                status = "✅ pass" if ok else "❌ fail"
                fh.write(
                    f"| {name} | {cov} / {tot} | {pct:.2f}% | {min_pct:.0f}% | {status} |\n"
                )
            if failed:
                fh.write(
                    "\n> ⛔ One or more coverage thresholds were not met. "
                    "See `docs/coverage.md` for the baseline and "
                    "`.coverage-thresholds.json` for the configured thresholds.\n"
                )
            else:
                fh.write("\n> ✅ All coverage thresholds met.\n")

    # Console output
    col_w = max(len(r[0]) for r in results) + 2
    header = f"{'Scope':<{col_w}} {'Coverage':>10}  {'Min':>6}  Status"
    print(header)
    print("-" * len(header))
    for name, pct, min_pct, ok, cov, tot in results:
        status = "PASS" if ok else "FAIL"
        print(f"{name:<{col_w}} {pct:>9.2f}%  {min_pct:>5.0f}%  {status}")

    if failed:
        print(
            "\n\u274c Coverage gate FAILED. "
            "Update tests or lower the threshold in .coverage-thresholds.json.",
            file=sys.stderr,
        )
        sys.exit(1)

    print("\n\u2705 Coverage gate PASSED.")


if __name__ == "__main__":
    main()
