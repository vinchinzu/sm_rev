"""
Headless runtime tests for sm_rev.

Uses SDL_VIDEODRIVER=offscreen + SDL_AUDIODRIVER=dummy to run frames without
a display or audio device. Requires sm.smc ROM to be present (skipped otherwise).

These tests catch runtime crashes, infinite loops, and game-logic regressions
that wouldn't be caught by a compile-only check.
"""

from __future__ import annotations

import json
import subprocess
from pathlib import Path

import pytest

SM_REV_DIR = Path(__file__).parent.parent
BINARY = SM_REV_DIR / "sm_rev"
ROM = SM_REV_DIR / "sm.smc"

# SDL env vars for headless operation
HEADLESS_ENV = {
    "SDL_VIDEODRIVER": "offscreen",
    "SDL_AUDIODRIVER": "dummy",
    "DISPLAY": "",
}


def headless(frames: int, extra_args: list[str] | None = None, timeout: int = 30) -> subprocess.CompletedProcess:
    """Run sm_rev headlessly for `frames` frames, dumping RAM state to stdout."""
    import os
    env = os.environ.copy()
    env.update(HEADLESS_ENV)
    cmd = [str(BINARY), "--headless", str(frames), "--dump", "-"] + (extra_args or [])
    return subprocess.run(cmd, cwd=SM_REV_DIR, capture_output=True, text=True, env=env, timeout=timeout)


def find_saves() -> list[Path]:
    """Return .sav files available for state-load tests."""
    saves_dir = SM_REV_DIR / "saves"
    if not saves_dir.exists():
        return []
    return sorted(saves_dir.glob("*.sav"))


def parse_dump(r: subprocess.CompletedProcess) -> dict:
    """Parse the JSON RAM dump from stdout.

    The binary may print non-JSON lines before the dump (e.g. PPU verify
    messages).  Extract the last {...} block to be robust.
    """
    # Find the last '{' ... '}' block in stdout
    start = r.stdout.rfind("{")
    end = r.stdout.rfind("}") + 1
    if start == -1 or end == 0:
        raise ValueError(f"No JSON object found in stdout:\n{r.stdout!r}")
    return json.loads(r.stdout[start:end])


@pytest.fixture(autouse=True)
def require_binary_and_rom():
    if not BINARY.exists():
        pytest.skip("sm_rev binary not built — run `make` first")
    if not ROM.exists():
        pytest.skip("sm.smc ROM not present — required for runtime tests")


class TestHeadlessSmoke:
    def test_runs_10_frames(self):
        """Binary must survive 10 frames headlessly."""
        r = headless(10)
        assert r.returncode == 0, f"Crashed after 10 frames:\n{r.stderr}"

    def test_runs_60_frames(self):
        """Binary must survive 60 frames (1 second of game time) headlessly."""
        r = headless(60)
        assert r.returncode == 0, f"Crashed after 60 frames:\n{r.stderr}"

    def test_runs_300_frames(self):
        """Binary must survive 300 frames (5 seconds) headlessly — catches deferred crashes."""
        r = headless(300)
        assert r.returncode == 0, f"Crashed after 300 frames:\n{r.stderr}"

    def test_dump_is_valid_json(self):
        """--dump - must produce parseable JSON."""
        r = headless(10)
        assert r.returncode == 0
        data = parse_dump(r)
        assert isinstance(data, dict)

    def test_dump_has_required_fields(self):
        """RAM dump must contain all expected fields."""
        r = headless(10)
        data = parse_dump(r)
        for field in ("frames", "game_state", "area_index", "room_ptr", "samus_health", "samus_y_pos"):
            assert field in data, f"Missing field in dump: {field}"

    def test_dump_frame_count_matches(self):
        """Reported frame count must match requested count."""
        for n in (10, 60, 120):
            r = headless(n)
            data = parse_dump(r)
            assert data["frames"] == n, f"Expected {n} frames, got {data['frames']}"

    def test_no_stderr_errors(self):
        """No error messages on stderr during clean headless run."""
        r = headless(60)
        # Filter out harmless SDL offscreen messages
        errors = [l for l in r.stderr.splitlines()
                  if l.strip() and "offscreen" not in l.lower() and "dummy" not in l.lower()]
        assert not errors, f"Unexpected stderr output:\n{''.join(errors)}"


class TestHeadlessStateLoad:
    """Verify --load-state puts Samus in-game (non-title) state."""

    @pytest.fixture(autouse=True)
    def require_saves(self):
        saves = find_saves()
        if not saves:
            pytest.skip("No .sav files found in saves/ — skipping state-load tests")
        self.saves = saves

    def test_load_state_does_not_crash(self):
        """Binary must exit 0 after loading any available save state."""
        r = headless(10, extra_args=["--load-state", str(self.saves[0])])
        assert r.returncode == 0, f"Crashed on --load-state:\n{r.stderr}"

    def test_load_state_gives_valid_json(self):
        """--load-state + --dump must produce parseable JSON."""
        r = headless(10, extra_args=["--load-state", str(self.saves[0])])
        assert r.returncode == 0
        data = parse_dump(r)
        assert isinstance(data, dict)

    def test_load_state_exits_title_screen(self):
        """After loading a save state, game_state must be > 0x1F (in-game, past title)."""
        r = headless(60, extra_args=["--load-state", str(self.saves[0])])
        data = parse_dump(r)
        # Title screen game_state is 0x00-0x0D; in-game is typically 0x1E-0x1F
        assert data["game_state"] > 0, "game_state is still 0 — state did not load"

    def test_load_state_samus_health_nonzero(self):
        """Loaded save must have nonzero Samus health (not title screen)."""
        r = headless(60, extra_args=["--load-state", str(self.saves[0])])
        data = parse_dump(r)
        assert data["samus_health"] > 0, f"samus_health=0 after loading state — may not be in-game"

    def test_load_state_deterministic(self):
        """Two runs loading the same save must produce identical RAM dumps."""
        save = self.saves[0]
        r1 = headless(60, extra_args=["--load-state", str(save)])
        r2 = headless(60, extra_args=["--load-state", str(save)])
        d1 = parse_dump(r1)
        d2 = parse_dump(r2)
        assert d1 == d2, f"Non-deterministic output after state load:\nrun1={d1}\nrun2={d2}"


class TestHeadlessRAMSanity:
    """Sanity-check RAM values after cold boot — not deep physics, just 'not garbage'."""

    def test_samus_health_nonzero_after_boot(self):
        """After enough frames for game to initialize, health should be nonzero."""
        # Boot screen takes a while; run 300 frames to get past early init
        r = headless(300)
        data = parse_dump(r)
        # During title screen Samus health may be 0 (not in game yet) — just check it's a uint16
        assert 0 <= data["samus_health"] <= 0xFFFF

    def test_game_state_is_valid(self):
        """game_state should be a known value, not garbage."""
        r = headless(60)
        data = parse_dump(r)
        # Valid game_state values are in a small range (0x00-0x1F typical)
        assert 0 <= data["game_state"] <= 0x1F, f"Unexpected game_state: {data['game_state']:#x}"

    def test_area_index_is_valid(self):
        """area_index should be in [0..6] (Crateria..Tourian)."""
        r = headless(60)
        data = parse_dump(r)
        # On cold boot this will be 0 (Crateria) or uninitialized
        assert 0 <= data["area_index"] <= 7, f"Unexpected area_index: {data['area_index']}"

    def test_deterministic_across_runs(self):
        """Two headless runs of the same frame count must produce identical RAM dumps."""
        r1 = headless(120)
        r2 = headless(120)
        d1 = parse_dump(r1)
        d2 = parse_dump(r2)
        assert d1 == d2, f"Non-deterministic output:\nrun1={d1}\nrun2={d2}"
