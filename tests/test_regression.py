"""
Regression tests for sm_rev RM_MINE mode.

All tests here run exclusively with --runmode mine so the SNES emulator fallback
never masks bugs in our C port. Tests are intentionally aggressive — many will
fail while regressions exist, which is the point.

Known open regressions (tracked here):
  - sprite tile corruption after room load
  - room navigation / door transition glitches
  - enemy state corruption
"""

from __future__ import annotations

import json
import subprocess
from pathlib import Path

import pytest

SM_REV_DIR = Path(__file__).parent.parent
BINARY = SM_REV_DIR / "sm_rev"
ROM = SM_REV_DIR / "sm.smc"

HEADLESS_ENV = {
    "SDL_VIDEODRIVER": "offscreen",
    "SDL_AUDIODRIVER": "dummy",
    "DISPLAY": "",
}


def headless_mine(
    frames: int,
    save: Path | None = None,
    extra_args: list[str] | None = None,
    timeout: int = 60,
    cwd: Path | None = None,
) -> subprocess.CompletedProcess:
    import os
    env = os.environ.copy()
    env.update(HEADLESS_ENV)
    cmd = [str(BINARY), "--headless", str(frames), "--runmode", "mine", "--dump", "-"]
    if save:
        cmd += ["--load-state", str(save)]
    cmd += extra_args or []
    return subprocess.run(cmd, cwd=cwd or SM_REV_DIR, capture_output=True, text=True, env=env, timeout=timeout)


def headless_theirs(
    frames: int,
    save: Path | None = None,
    extra_args: list[str] | None = None,
    timeout: int = 60,
    cwd: Path | None = None,
) -> subprocess.CompletedProcess:
    import os
    env = os.environ.copy()
    env.update(HEADLESS_ENV)
    cmd = [str(BINARY), "--headless", str(frames), "--runmode", "theirs", "--dump", "-"]
    if save:
        cmd += ["--load-state", str(save)]
    cmd += extra_args or []
    return subprocess.run(cmd, cwd=cwd or SM_REV_DIR, capture_output=True, text=True, env=env, timeout=timeout)


def parse_dump(r: subprocess.CompletedProcess) -> dict:
    start = r.stdout.rfind("{")
    end = r.stdout.rfind("}") + 1
    if start == -1 or end == 0:
        raise ValueError(f"No JSON in stdout:\n{r.stdout!r}")
    return json.loads(r.stdout[start:end])


def all_saves() -> list[Path]:
    saves_dir = SM_REV_DIR / "saves"
    return sorted(saves_dir.glob("*.sav")) if saves_dir.exists() else []


@pytest.fixture(autouse=True)
def require_binary_and_rom():
    if not BINARY.exists():
        pytest.skip("sm_rev binary not built")
    if not ROM.exists():
        pytest.skip("sm.smc ROM not present")


@pytest.fixture
def saves() -> list[Path]:
    s = all_saves()
    if not s:
        pytest.skip("No .sav files in saves/")
    return s


class TestMineSmokeAllSaves:
    """RM_MINE must not crash on any save state, even for long runs."""

    @pytest.mark.parametrize("frames", [60, 300, 600])
    def test_cold_boot_stability(self, frames: int):
        r = headless_mine(frames)
        assert r.returncode == 0, f"RM_MINE crashed at {frames} frames (cold boot):\n{r.stderr}"

    def test_all_saves_survive_600_frames(self, saves: list[Path]):
        failures = []
        for save in saves:
            r = headless_mine(600, save=save)
            if r.returncode != 0:
                failures.append(f"{save.name}: returncode={r.returncode}\n{r.stderr[:200]}")
        assert not failures, "RM_MINE crashed on saves:\n" + "\n".join(failures)

    def test_all_saves_produce_valid_json(self, saves: list[Path]):
        for save in saves:
            r = headless_mine(60, save=save)
            assert r.returncode == 0, f"{save.name} crashed"
            data = parse_dump(r)
            assert data["frame_source"] == "mine", f"{save.name}: frame_source={data['frame_source']}"
            assert data["runmode"] == "mine", f"{save.name}: runmode={data['runmode']}"


class TestMineVsTheirsFrameDump:
    """RM_MINE pixel output must match RM_THEIRS (emulator) for every save state.

    Failures here identify rendering regressions: sprite tile corruption, wrong
    palettes, missing/corrupted enemies, room layer glitches, etc.
    """

    @pytest.mark.parametrize("frames", [5, 30, 60, 120, 300])
    def test_cold_boot_frame_match(self, frames: int, tmp_path: Path):
        mine_frame = tmp_path / "mine.raw"
        theirs_frame = tmp_path / "theirs.raw"
        mine = headless_mine(frames, extra_args=["--dump-frame", str(mine_frame)])
        theirs = headless_theirs(frames, extra_args=["--dump-frame", str(theirs_frame)])
        assert mine.returncode == 0, f"mine crashed:\n{mine.stderr}"
        assert theirs.returncode == 0, f"theirs crashed:\n{theirs.stderr}"
        assert mine_frame.read_bytes() == theirs_frame.read_bytes(), (
            f"Frame pixel mismatch at {frames} frames (cold boot)\n"
            f"mine  RAM: {parse_dump(mine)}\n"
            f"theirs RAM: {parse_dump(theirs)}"
        )

    def test_all_saves_frame_match_at_60(self, saves: list[Path], tmp_path: Path):
        """After loading each save, mine and theirs must produce identical frames at 60 frames."""
        failures = []
        for i, save in enumerate(saves):
            mine_frame = tmp_path / f"mine_{i}.raw"
            theirs_frame = tmp_path / f"theirs_{i}.raw"
            mine = headless_mine(60, save=save.resolve(), extra_args=["--dump-frame", str(mine_frame)])
            theirs = headless_theirs(60, save=save.resolve(), extra_args=["--dump-frame", str(theirs_frame)])
            if mine.returncode != 0 or theirs.returncode != 0:
                failures.append(f"{save.name}: crash (mine={mine.returncode} theirs={theirs.returncode})")
                continue
            if mine_frame.read_bytes() != theirs_frame.read_bytes():
                failures.append(f"{save.name}: frame mismatch at 60 frames")
        assert not failures, "mine≠theirs on saves:\n" + "\n".join(failures)

    def test_all_saves_frame_match_at_300(self, saves: list[Path], tmp_path: Path):
        """After loading each save, mine and theirs must produce identical frames at 300 frames."""
        failures = []
        for i, save in enumerate(saves):
            mine_frame = tmp_path / f"mine_{i}.raw"
            theirs_frame = tmp_path / f"theirs_{i}.raw"
            mine = headless_mine(300, save=save.resolve(), extra_args=["--dump-frame", str(mine_frame)], timeout=90)
            theirs = headless_theirs(300, save=save.resolve(), extra_args=["--dump-frame", str(theirs_frame)], timeout=90)
            if mine.returncode != 0 or theirs.returncode != 0:
                failures.append(f"{save.name}: crash (mine={mine.returncode} theirs={theirs.returncode})")
                continue
            if mine_frame.read_bytes() != theirs_frame.read_bytes():
                failures.append(f"{save.name}: frame mismatch at 300 frames")
        assert not failures, "mine≠theirs on saves:\n" + "\n".join(failures)


class TestMineVsTheirsRAM:
    """Key RAM fields from RM_MINE must match RM_THEIRS at every save state.

    This is a cheaper check than pixel comparison — if RAM diverges, visuals will too.
    """

    FIELDS_TO_COMPARE = ("game_state", "area_index", "room_ptr", "samus_health", "samus_y_pos")

    def _compare_ram(self, save: Path, frames: int) -> list[str]:
        mine = headless_mine(frames, save=save)
        theirs = headless_theirs(frames, save=save)
        errors = []
        if mine.returncode != 0:
            errors.append(f"mine crashed: {mine.stderr[:200]}")
            return errors
        if theirs.returncode != 0:
            errors.append(f"theirs crashed: {theirs.stderr[:200]}")
            return errors
        dm = parse_dump(mine)
        dt = parse_dump(theirs)
        for field in self.FIELDS_TO_COMPARE:
            if dm.get(field) != dt.get(field):
                errors.append(f"  {field}: mine={dm.get(field):#x if isinstance(dm.get(field), int) else dm.get(field)} theirs={dt.get(field):#x if isinstance(dt.get(field), int) else dt.get(field)}")
        return errors

    def test_all_saves_ram_match_at_60(self, saves: list[Path]):
        failures = []
        for save in saves:
            errs = self._compare_ram(save, 60)
            if errs:
                failures.append(f"{save.name} @60f:\n" + "\n".join(errs))
        assert not failures, "RAM divergence (mine≠theirs):\n" + "\n".join(failures)

    def test_all_saves_ram_match_at_300(self, saves: list[Path]):
        failures = []
        for save in saves:
            errs = self._compare_ram(save, 300)
            if errs:
                failures.append(f"{save.name} @300f:\n" + "\n".join(errs))
        assert not failures, "RAM divergence (mine≠theirs):\n" + "\n".join(failures)


class TestDivergenceDetection:
    """RM_BOTH must never hit a verify failure (mine≠theirs divergence) on clean saves."""

    def _run_both(self, frames: int, save: Path | None = None, timeout: int = 120) -> subprocess.CompletedProcess:
        import os
        env = os.environ.copy()
        env.update(HEADLESS_ENV)
        cmd = [str(BINARY), "--headless", str(frames), "--runmode", "both", "--dump", "-"]
        if save:
            cmd += ["--load-state", str(save)]
        return subprocess.run(cmd, cwd=SM_REV_DIR, capture_output=True, text=True, env=env, timeout=timeout)

    def test_cold_boot_no_divergence_300f(self):
        r = self._run_both(300)
        assert r.returncode == 0, f"RM_BOTH crashed:\n{r.stderr}"
        assert "Verify failure" not in r.stdout, (
            f"RM_BOTH divergence detected at cold boot (300f)\n"
            f"stdout excerpt:\n{r.stdout[:500]}"
        )

    def test_all_saves_no_divergence_at_60(self, saves: list[Path]):
        failures = []
        for save in saves:
            r = self._run_both(60, save=save)
            if r.returncode != 0:
                failures.append(f"{save.name}: crash")
            elif "Verify failure" in r.stdout:
                failures.append(f"{save.name}: divergence detected")
        assert not failures, "RM_BOTH divergences:\n" + "\n".join(failures)
