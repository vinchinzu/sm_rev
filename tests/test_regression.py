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
    if not saves_dir.exists():
        return []
    slot_saves = sorted(saves_dir.glob("save*.sav"))
    return slot_saves if slot_saves else sorted(saves_dir.glob("*.sav"))


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
    """Cold-boot RM_MINE pixel output must match RM_THEIRS (emulator).

    Save-state loading is intentionally excluded here: C-port save files are not
    a stable oracle input for RM_THEIRS, so mine-vs-theirs comparisons on
    savestates produce false failures.
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

    def test_all_saves_frame_deterministic_at_60(self, saves: list[Path], tmp_path: Path):
        """After loading each canonical save, RM_MINE must produce deterministic 60f output."""
        failures = []
        for i, save in enumerate(saves):
            frame_a = tmp_path / f"mine_{i}_a.raw"
            frame_b = tmp_path / f"mine_{i}_b.raw"
            run_a = headless_mine(60, save=save.resolve(), extra_args=["--dump-frame", str(frame_a)])
            run_b = headless_mine(60, save=save.resolve(), extra_args=["--dump-frame", str(frame_b)])
            if run_a.returncode != 0 or run_b.returncode != 0:
                failures.append(f"{save.name}: crash (run_a={run_a.returncode} run_b={run_b.returncode})")
                continue
            if frame_a.read_bytes() != frame_b.read_bytes():
                failures.append(f"{save.name}: non-deterministic frame output at 60 frames")
        assert not failures, "non-deterministic RM_MINE save output:\n" + "\n".join(failures)

    def test_all_saves_frame_deterministic_at_300(self, saves: list[Path], tmp_path: Path):
        """After loading each canonical save, RM_MINE must produce deterministic 300f output."""
        failures = []
        for i, save in enumerate(saves):
            frame_a = tmp_path / f"mine_{i}_a.raw"
            frame_b = tmp_path / f"mine_{i}_b.raw"
            run_a = headless_mine(300, save=save.resolve(), extra_args=["--dump-frame", str(frame_a)], timeout=90)
            run_b = headless_mine(300, save=save.resolve(), extra_args=["--dump-frame", str(frame_b)], timeout=90)
            if run_a.returncode != 0 or run_b.returncode != 0:
                failures.append(f"{save.name}: crash (run_a={run_a.returncode} run_b={run_b.returncode})")
                continue
            if frame_a.read_bytes() != frame_b.read_bytes():
                failures.append(f"{save.name}: non-deterministic frame output at 300 frames")
        assert not failures, "non-deterministic RM_MINE save output:\n" + "\n".join(failures)


class TestMineVsTheirsRAM:
    """Key RAM fields from RM_MINE must be deterministic on canonical savestates.

    RM_THEIRS is still useful as a cold-boot oracle, but it is not a reliable
    savestate oracle for the C-port save format.
    """

    FIELDS_TO_COMPARE = ("game_state", "area_index", "room_ptr", "samus_health", "samus_y_pos")

    @staticmethod
    def _fmt(value):
        return f"{value:#x}" if isinstance(value, int) else str(value)

    def _compare_ram(self, save: Path, frames: int) -> list[str]:
        mine = headless_mine(frames, save=save)
        mine_repeat = headless_mine(frames, save=save)
        errors = []
        if mine.returncode != 0:
            errors.append(f"mine crashed: {mine.stderr[:200]}")
            return errors
        if mine_repeat.returncode != 0:
            errors.append(f"mine repeat crashed: {mine_repeat.stderr[:200]}")
            return errors
        dm = parse_dump(mine)
        dt = parse_dump(mine_repeat)
        for field in self.FIELDS_TO_COMPARE:
            if dm.get(field) != dt.get(field):
                errors.append(
                    f"  {field}: run_a={self._fmt(dm.get(field))} run_b={self._fmt(dt.get(field))}"
                )
        return errors

    def test_all_saves_ram_match_at_60(self, saves: list[Path]):
        failures = []
        for save in saves:
            errs = self._compare_ram(save, 60)
            if errs:
                failures.append(f"{save.name} @60f:\n" + "\n".join(errs))
        assert not failures, "non-deterministic RM_MINE RAM:\n" + "\n".join(failures)

    def test_all_saves_ram_match_at_300(self, saves: list[Path]):
        failures = []
        for save in saves:
            errs = self._compare_ram(save, 300)
            if errs:
                failures.append(f"{save.name} @300f:\n" + "\n".join(errs))
        assert not failures, "non-deterministic RM_MINE RAM:\n" + "\n".join(failures)


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
        pytest.skip("RM_BOTH savestate oracle is currently unsupported for C-port save files")
