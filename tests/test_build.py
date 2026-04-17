"""
Build integrity tests for sm_rev.

These run `make` and verify the binary comes out clean. No ROM needed.
All tests here must pass after EVERY code change — they are the first gate.
"""

from __future__ import annotations

import os
import subprocess
from pathlib import Path

SM_REV_DIR = Path(__file__).parent.parent
BINARY = SM_REV_DIR / "sm_rev"


def run(cmd: list[str], **kw) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, cwd=SM_REV_DIR, capture_output=True, text=True, **kw)


class TestBuild:
    def test_make_clean_succeeds(self):
        """make clean should always exit 0."""
        r = run(["make", "clean"])
        assert r.returncode == 0, f"make clean failed:\n{r.stderr}"

    def test_make_succeeds(self):
        """Full build must exit 0 with -Werror — no warnings allowed."""
        r = run(["make"])
        assert r.returncode == 0, f"make failed:\n{r.stderr}\n{r.stdout}"

    def test_binary_exists(self):
        """Binary must exist after build."""
        assert BINARY.exists(), f"Binary not found at {BINARY}"

    def test_binary_is_executable(self):
        """Binary must be executable."""
        assert os.access(BINARY, os.X_OK), f"Binary not executable: {BINARY}"

    def test_no_compilation_warnings(self):
        """make output must be warning-free (enforced by -Werror, but double-check stderr)."""
        r = run(["make", "clean"])
        r2 = run(["make"])
        assert r2.returncode == 0
        # -Werror makes warnings into errors, so a clean build has no warning lines
        for line in r2.stderr.splitlines():
            assert "warning:" not in line.lower(), f"Unexpected warning in build:\n{line}"
