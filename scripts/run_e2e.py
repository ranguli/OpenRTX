#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

"""
E2E test runner for OpenRTX.

Takes a test script file containing emulator shell commands (key, sleep,
screenshot, quit, etc.) and runs it against the linux emulator.  Any
"screenshot <name>.bmp" commands in the script automatically become
assertions: the captured image is compared pixel-for-pixel against a
golden reference in tests/e2e/golden/<base_test>/<variant>/<name>.bmp.

The base test name is derived from the script filename by stripping a
trailing _<variant> suffix when present (e.g. about_default.txt with
variant "default" resolves to golden dir golden/about/default/).

Run all tests (default):
    python scripts/run_e2e.py
    python scripts/run_e2e.py --build-dir build_linux_debug

Single test:
    python scripts/run_e2e.py tests/e2e/about_default.txt --variant default
    python scripts/run_e2e.py tests/e2e/main_menu.txt --binary ./build/bin

Environment:
    UPDATE_GOLDEN=1  -- overwrite golden images with current output
    TOLERANCE=0      -- max differing pixels before failure (default: 0)
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

from PIL import Image

PROJECT_ROOT = Path(__file__).resolve().parent.parent
E2E_DIR = PROJECT_ROOT / "tests" / "e2e"

# Variant name -> binary filename (relative to build dir)
VARIANTS = {
    "default": "openrtx_linux",
    "module17": "openrtx_linux_mod17",
}


def compare_images(actual_path, golden_path):
    """Compare two images pixel-by-pixel, return count of differing
    pixels."""
    actual = Image.open(actual_path)
    golden = Image.open(golden_path)

    if actual.size != golden.size:
        raise ValueError(
            f"Size mismatch: actual {actual.size} vs golden {golden.size}"
        )

    actual_data = actual.tobytes()
    golden_data = golden.tobytes()

    if actual_data == golden_data:
        return 0

    # Count differing pixels by comparing per-pixel chunks
    bpp = len(actual_data) // (actual.size[0] * actual.size[1])
    diff_count = 0
    for i in range(0, len(actual_data), bpp):
        if actual_data[i : i + bpp] != golden_data[i : i + bpp]:
            diff_count += 1

    return diff_count


def generate_diff_image(actual_path, golden_path, diff_path):
    """Generate a diff image highlighting pixel differences in red."""
    actual = Image.open(actual_path).convert("RGB")
    golden = Image.open(golden_path).convert("RGB")

    diff = Image.new("RGB", actual.size, (0, 0, 0))
    actual_px = actual.load()
    golden_px = golden.load()
    diff_px = diff.load()

    w, h = actual.size
    for y in range(h):
        for x in range(w):
            if actual_px[x, y] != golden_px[x, y]:
                diff_px[x, y] = (255, 0, 0)

    diff.save(diff_path)


def resolve_base_name(script_name, variant):
    """Strip trailing _<variant> suffix from the script name."""
    suffix = f"_{variant}"
    if script_name.endswith(suffix):
        return script_name[: -len(suffix)]
    return script_name


def run_test(script_path, binary, variant, tolerance, update_golden):
    """Run a single e2e test.  Returns True on success."""
    test_name = script_path.stem
    base_name = resolve_base_name(test_name, variant)
    golden_dir = E2E_DIR / "golden" / base_name / variant

    with tempfile.TemporaryDirectory() as tmpdir_str:
        tmpdir = Path(tmpdir_str)

        # Parse test script, rewrite screenshot paths into tmpdir
        screenshots = []
        rewritten_lines = []

        with open(script_path) as f:
            for line in f:
                line = line.rstrip("\n")
                m = re.match(r"^\s*screenshot\s+(.+)$", line)
                if m:
                    name = m.group(1)
                    screenshots.append(name)
                    rewritten_lines.append(
                        f"screenshot {tmpdir / name}"
                    )
                else:
                    rewritten_lines.append(line)

        rewritten_script = "\n".join(rewritten_lines) + "\n"

        # Give each test its own codeplug so parallel runs don't collide
        codeplug = PROJECT_ROOT / "default.rtxc"
        if codeplug.is_file():
            shutil.copy2(codeplug, tmpdir / "default.rtxc")

        print(f"Running E2E test: {base_name} ({variant})")

        env = {
            **os.environ,
            "TZ": "UTC",
            "XDG_STATE_HOME": str(tmpdir / "state"),
            "SDL_VIDEODRIVER": "offscreen",
            "SDL_AUDIODRIVER": "dummy",
        }

        cmd = [
            "faketime", "-f", "2000-01-01 00:00:00", str(binary),
        ]

        try:
            subprocess.run(
                cmd,
                input=rewritten_script,
                capture_output=True,
                text=True,
                timeout=30,
                cwd=str(tmpdir),
                env=env,
            )
        except subprocess.TimeoutExpired:
            print("FAIL: emulator timed out after 30s")
            return False

        # -- compare screenshots ------------------------------------------

        fail_dir = (
            PROJECT_ROOT
            / "build_linux"
            / "e2e_failures"
            / base_name
            / variant
        )
        failures = 0

        for name in screenshots:
            actual = tmpdir / name
            golden = golden_dir / name

            if not actual.is_file():
                print(f"  FAIL: screenshot not produced: {name}")
                failures += 1
                continue

            if update_golden:
                golden_dir.mkdir(parents=True, exist_ok=True)
                shutil.copy2(actual, golden)
                print(f"  updated golden: {name}")
                continue

            if not golden.is_file():
                print(f"  FAIL: golden image missing: {golden}")
                print("    (run with UPDATE_GOLDEN=1 to create)")
                failures += 1
                continue

            try:
                diff_pixels = compare_images(actual, golden)
            except Exception as e:
                print(f"  FAIL: {name} -- comparison error: {e}")
                failures += 1
                continue

            if diff_pixels <= tolerance:
                print(f"  PASS: {name} ({diff_pixels} pixels differ)")
            else:
                print(
                    f"  FAIL: {name} -- {diff_pixels} pixels differ"
                    f" (tolerance: {tolerance})"
                )
                fail_dir.mkdir(parents=True, exist_ok=True)
                shutil.copy2(actual, fail_dir / f"actual_{name}")
                shutil.copy2(golden, fail_dir / f"expected_{name}")
                try:
                    generate_diff_image(
                        actual, golden, fail_dir / f"diff_{name}"
                    )
                except Exception:
                    pass
                print(
                    f"    actual:   {fail_dir / f'actual_{name}'}"
                )
                print(
                    f"    expected: {fail_dir / f'expected_{name}'}"
                )
                print(f"    diff:     {fail_dir / f'diff_{name}'}")
                failures += 1

        if failures > 0:
            print(f"  Snapshot failures saved to: {fail_dir}")
            return False

        print("  All assertions passed.")
        return True


def discover_tests(build_dir):
    """Discover test scripts and yield (script_path, binary, variant)
    tuples.

    Convention: if a script name ends with _<variant>, it is
    variant-specific and only runs with that variant.  Otherwise it is
    shared and runs with every known variant.
    """
    scripts = sorted(E2E_DIR.glob("*.txt"))

    for script in scripts:
        name = script.stem
        matched_variant = None
        for v in VARIANTS:
            if name.endswith(f"_{v}"):
                matched_variant = v
                break

        if matched_variant:
            binary = build_dir / VARIANTS[matched_variant]
            yield script, binary, matched_variant
        else:
            for v, bin_name in VARIANTS.items():
                yield script, build_dir / bin_name, v


def check_prerequisites(binary=None):
    """Verify external tools are available."""
    if not shutil.which("faketime"):
        print("FAIL: 'faketime' not found on PATH", file=sys.stderr)
        sys.exit(1)

    if binary and not os.access(binary, os.X_OK):
        print(
            f"FAIL: binary not found or not executable: {binary}",
            file=sys.stderr,
        )
        sys.exit(1)


def main():
    parser = argparse.ArgumentParser(
        description="OpenRTX E2E test runner"
    )
    parser.add_argument(
        "test_script",
        nargs="?",
        help="Path to test script (.txt)",
    )
    parser.add_argument("--binary", help="Path to openrtx binary")
    parser.add_argument(
        "--variant",
        default="default",
        help="UI variant (default: default)",
    )
    parser.add_argument(
        "--all",
        action="store_true",
        help="Discover and run all e2e tests",
    )
    parser.add_argument(
        "--build-dir",
        default=str(PROJECT_ROOT / "build_linux"),
        help="Build directory containing emulator binaries"
             " (default: build_linux)",
    )
    args = parser.parse_args()

    tolerance = int(os.environ.get("TOLERANCE", "0"))
    update_golden = os.environ.get("UPDATE_GOLDEN") == "1"

    if args.all or not args.test_script:
        return run_all(args, tolerance, update_golden)

    script_path = Path(args.test_script).resolve()
    build_dir = Path(args.build_dir).resolve()

    if args.binary:
        binary = Path(args.binary).resolve()
    else:
        binary = build_dir / VARIANTS.get(args.variant, "openrtx_linux")

    check_prerequisites(binary)

    if not script_path.is_file():
        print(
            f"FAIL: test script not found: {script_path}",
            file=sys.stderr,
        )
        sys.exit(1)

    ok = run_test(
        script_path, binary, args.variant, tolerance, update_golden
    )
    sys.exit(0 if ok else 1)


def run_all(args, tolerance, update_golden):
    """Discover and run every e2e test."""
    build_dir = Path(args.build_dir).resolve()
    check_prerequisites()

    tests = list(discover_tests(build_dir))
    if not tests:
        print("No e2e test scripts found.", file=sys.stderr)
        sys.exit(1)

    passed = 0
    failed = 0
    failed_names = []

    for i, (script, binary, variant) in enumerate(tests, 1):
        base = resolve_base_name(script.stem, variant)
        label = f"[{i}/{len(tests)}] {base} ({variant})"

        if not os.access(binary, os.X_OK):
            print(f"{label} ... SKIP (binary not found: {binary})")
            continue

        ok = run_test(script, binary, variant, tolerance, update_golden)
        if ok:
            passed += 1
        else:
            failed += 1
            failed_names.append(f"{base} ({variant})")

    print()
    print(f"{passed} passed, {failed} failed, {len(tests)} total")

    if failed_names:
        print("\nFailed tests:")
        for name in failed_names:
            print(f"  {name}")

    sys.exit(0 if failed == 0 else 1)


if __name__ == "__main__":
    main()
