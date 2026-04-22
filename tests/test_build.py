"""
Build integrity tests for sm_rev.

These run `make` and verify the binary comes out clean. No ROM needed.
All tests here must pass after EVERY code change — they are the first gate.
"""

from __future__ import annotations

import json
import os
import shutil
import subprocess
from pathlib import Path

SM_REV_DIR = Path(__file__).parent.parent
BINARY = SM_REV_DIR / "sm_rev"
MINI_BINARY = SM_REV_DIR / "sm_rev_mini"
EDITOR_LANDING_SITE_EXPORT = SM_REV_DIR.parent / "super_metroid_editor" / "export" / "sm_nav" / "rooms" / "room_91F8.json"


def run(cmd: list[str], **kw) -> subprocess.CompletedProcess:
    cwd = kw.pop("cwd", SM_REV_DIR)
    return subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, **kw)


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


class TestBuildMini:
    def test_make_mini_succeeds(self):
        """Mini shell build must compile cleanly."""
        r = run(["make", "mini"])
        assert r.returncode == 0, f"make mini failed:\n{r.stderr}\n{r.stdout}"

    def test_mini_binary_exists(self):
        """Mini shell binary must exist after build."""
        assert MINI_BINARY.exists(), f"Mini binary not found at {MINI_BINARY}"

    def test_mini_binary_is_executable(self):
        """Mini shell binary must be executable."""
        assert os.access(MINI_BINARY, os.X_OK), f"Mini binary not executable: {MINI_BINARY}"

    def test_mini_headless_smoke(self):
        """Mini shell headless mode must exit 0 and report mini build metadata."""
        r = run([str(MINI_BINARY), "--headless", "--frames", "3"])
        assert r.returncode == 0, f"mini headless smoke failed:\n{r.stderr}\n{r.stdout}"
        assert '"build":"mini"' in r.stdout
        assert '"frames":3' in r.stdout
        assert '"no_rooms":false' in r.stdout

    def test_mini_headless_smoke_outside_repo_cwd(self, tmp_path: Path):
        """Mini should still find its default room export when launched outside the repo cwd."""
        r = run([str(MINI_BINARY), "--headless", "--frames", "1"], cwd=tmp_path)
        assert r.returncode == 0, f"mini external-cwd smoke failed:\n{r.stderr}\n{r.stdout}"
        assert '"room_source":"editor_export"' in r.stdout
        assert '"room_handle":"landingSite"' in r.stdout
        assert '"room_visuals":"editor_tileset"' in r.stdout

    def test_mini_editor_export_bridge_smoke(self):
        """Mini should accept an explicit editor-exported Landing Site room file."""
        if not EDITOR_LANDING_SITE_EXPORT.exists():
            return
        r = run([
            str(MINI_BINARY),
            "--headless",
            "--frames",
            "1",
            "--room-export",
            str(EDITOR_LANDING_SITE_EXPORT),
        ])
        assert r.returncode == 0, f"mini editor-export smoke failed:\n{r.stderr}\n{r.stdout}"
        assert '"room_source":"editor_export"' in r.stdout
        assert '"room_handle":"landingSite"' in r.stdout
        assert '"rom_room":false' in r.stdout

    def test_mini_record_smoke(self, tmp_path: Path):
        """Mini should emit a capped low-resolution quick clip when recording is requested."""
        if shutil.which("ffmpeg") is None:
            return
        r = run([
            str(MINI_BINARY),
            "--headless",
            "--frames",
            "3",
            "--record",
        ], cwd=tmp_path)
        assert r.returncode == 0, f"mini record smoke failed:\n{r.stderr}\n{r.stdout}"
        payload = json.loads(r.stdout)
        output = Path(payload["record_path"])
        assert output.exists(), f"expected recording at {output}\nstdout={r.stdout}\nstderr={r.stderr}"
        assert "out" in output.parts
        assert output.stat().st_size > 0
        assert output.stat().st_size <= 10 * 1024 * 1024
        assert '"recording":true' in r.stdout
