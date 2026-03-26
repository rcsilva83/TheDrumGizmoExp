#!/usr/bin/env python3
"""
Check that changed source files on a pull request meet the configured minimum
line-coverage floor.

Reads coverage data from a Cobertura XML report and inspects only the files
listed on the command line (typically the output of ``git diff --name-only``).
Files that are not present in the coverage report (e.g. header-only files or
files that were not instrumented) are silently skipped.

Exits with status 1 if any changed file is below the configured floor.
When running inside GitHub Actions, appends a Markdown table to
GITHUB_STEP_SUMMARY for reviewer visibility.

Usage:
    python3 scripts/check-pr-coverage.py \\
        [--config .coverage-thresholds.json] \\
        [--xml coverage.xml] \\
        file1 file2 ...

Defaults:
    --config  .coverage-thresholds.json  (relative to CWD)
    --xml     coverage.xml               (relative to CWD)
"""

import argparse
import json
import os
import sys
import xml.etree.ElementTree as ET


def file_line_stats(root, filename):
    """Return (found, covered_lines, total_lines) for a single file in the Cobertura XML.

    *found* is True when the file has at least one ``<class>`` element in the
    report, regardless of whether it has any executable lines.  When *found* is
    False the file was not instrumented and should be skipped by the caller.
    """
    found = False
    covered = 0
    total = 0
    for cls in root.findall(".//class"):
        if cls.attrib.get("filename", "") == filename:
            found = True
            for line in cls.findall("lines/line"):
                total += 1
                if int(line.attrib.get("hits", "0")) > 0:
                    covered += 1
    return found, covered, total


def percent(covered, total):
    return covered / total * 100 if total > 0 else 0.0


# Source-file extensions that are expected to appear in the coverage report
_SOURCE_EXTS = {".c", ".cc", ".cpp", ".cxx"}

# Prefixes that identify non-test source modules
_SOURCE_PREFIXES = ("src/", "dggui/", "plugingui/", "plugin/", "drumgizmo/")


def is_source_file(path):
    """Return True if *path* is an instrumented source file (not a test)."""
    _, ext = os.path.splitext(path)
    if ext not in _SOURCE_EXTS:
        return False
    return any(path.startswith(prefix) for prefix in _SOURCE_PREFIXES)


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Check changed-file coverage floor for pull requests"
    )
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
    parser.add_argument(
        "files",
        nargs="*",
        metavar="FILE",
        help="Changed files to check (skip test/ files and headers automatically)",
    )
    args = parser.parse_args(argv)

    with open(args.config, encoding="utf-8") as fh:
        config = json.load(fh)

    floor = float(config.get("changed_file_min", 30))

    tree = ET.parse(args.xml)
    root = tree.getroot()

    # Filter to source files only
    source_files = [f for f in args.files if is_source_file(f)]

    if not source_files:
        print("No instrumented source files in the changed-file list. Nothing to check.")
        return

    results = []
    failed = False

    for path in sorted(source_files):
        found, cov, tot = file_line_stats(root, path)
        if not found:
            # File not in coverage report (not instrumented or header-only) — skip
            continue
        pct = percent(cov, tot)
        ok = pct >= floor
        results.append((path, pct, floor, ok, cov, tot))
        if not ok:
            failed = True

    if not results:
        print(
            "Changed source files were not found in the coverage report "
            "(possibly not instrumented). Nothing to check."
        )
        return

    # Write GitHub step summary when running in GitHub Actions
    summary_path = os.environ.get("GITHUB_STEP_SUMMARY", "")
    if summary_path:
        with open(summary_path, "a", encoding="utf-8") as fh:
            fh.write("## Changed-file coverage check\n\n")
            fh.write(
                f"Each changed source file must meet the {floor:.0f}% line-coverage floor "
                f"(configured in `.coverage-thresholds.json` → `changed_file_min`).\n\n"
            )
            fh.write("| File | Lines covered | Line % | Floor | Status |\n")
            fh.write("| ---- | ---: | ---: | ---: | :---: |\n")
            for path, pct, fl, ok, cov, tot in results:
                status = "✅ pass" if ok else "❌ fail"
                fh.write(
                    f"| `{path}` | {cov} / {tot} | {pct:.2f}% | {fl:.0f}% | {status} |\n"
                )
            if failed:
                fh.write(
                    "\n> ⛔ One or more changed files are below the coverage floor. "
                    "Add tests for the changed code or raise coverage before merging.\n"
                )
            else:
                fh.write(
                    f"\n> ✅ All changed source files meet the {floor:.0f}% coverage floor.\n"
                )

    # Console output
    col_w = max(len(r[0]) for r in results) + 2
    header = f"{'File':<{col_w}} {'Coverage':>10}  {'Floor':>6}  Status"
    print(header)
    print("-" * len(header))
    for path, pct, fl, ok, cov, tot in results:
        status = "PASS" if ok else "FAIL"
        print(f"{path:<{col_w}} {pct:>9.2f}%  {fl:>5.0f}%  {status}")

    if failed:
        print(
            "\n\u274c Changed-file coverage check FAILED. "
            "Add tests for changed source files or update the floor in "
            ".coverage-thresholds.json.",
            file=sys.stderr,
        )
        sys.exit(1)

    print(f"\n\u2705 Changed-file coverage check PASSED ({floor:.0f}% floor).")


if __name__ == "__main__":
    main()
