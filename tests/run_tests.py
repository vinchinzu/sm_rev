#!/usr/bin/env python3
"""
sm_rev test runner.

Usage:
    python tests/run_tests.py           # all tests
    python tests/run_tests.py --fast    # build-only (no ROM needed)
    python tests/run_tests.py --build   # alias for --fast
    python tests/run_tests.py -v        # verbose

Exit codes:
    0 = all ran tests passed
    1 = failures
    2 = usage error
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

TESTS_DIR = Path(__file__).parent
SM_REV_DIR = TESTS_DIR.parent


def main() -> int:
    args = sys.argv[1:]
    fast = "--fast" in args or "--build" in args
    verbose = "-v" in args or "--verbose" in args

    # Remove our custom flags before passing to pytest
    pytest_args = [a for a in args if a not in ("--fast", "--build")]

    cmd = [sys.executable, "-m", "pytest", str(TESTS_DIR)]

    if fast:
        # Only run build tests — no ROM, no SDL needed
        cmd += ["-k", "TestBuild"]
    else:
        cmd += ["-k", "TestBuild or TestHeadless"]

    if verbose:
        cmd += ["-v"]
    else:
        cmd += ["-q"]

    cmd += pytest_args

    print(f"Running: {' '.join(cmd)}")
    r = subprocess.run(cmd, cwd=SM_REV_DIR)
    return r.returncode


if __name__ == "__main__":
    sys.exit(main())
