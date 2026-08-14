from __future__ import annotations

import json
import subprocess
from pathlib import Path

import pytest

SM_REV_DIR = Path(__file__).parent.parent
MINI_BINARY = SM_REV_DIR / "sm_rev_mini"
LANDING_SITE_EXPORT = SM_REV_DIR.parent / "super_metroid_editor" / "export" / "sm_nav" / "rooms" / "room_91F8.json"
ROM_CANDIDATES = [
    SM_REV_DIR / "sm.smc",
    SM_REV_DIR.parent / "sm" / "sm.smc",
    SM_REV_DIR.parent / "roms" / "rom.sfc",
]


def run(cmd: list[str], **kw) -> subprocess.CompletedProcess:
    cwd = kw.pop("cwd", SM_REV_DIR)
    return subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, **kw)


def parse_json_payload(stdout: str) -> dict:
    start = stdout.rfind("{")
    end = stdout.rfind("}") + 1
    if start == -1 or end == 0:
        raise ValueError(f"No JSON object found in stdout:\n{stdout!r}")
    return json.loads(stdout[start:end])


@pytest.fixture
def landing_site_room(tmp_path: Path) -> Path:
    if not LANDING_SITE_EXPORT.exists():
        pytest.skip("Landing Site editor export not present — mini state hash test requires editor export data")
    room_path = tmp_path / "landing_site.json"
    room_path.write_text(LANDING_SITE_EXPORT.read_text())
    return room_path


def write_script(path: Path, lines: list[str]) -> Path:
    path.write_text("\n".join(lines) + "\n")
    return path


def expand_nav_string(nav: str, prefix_wait: int = 0) -> list[str]:
    """Expand a retro_rl `BUTTONS:hold:wait` nav string into mini input-script lines."""
    mapping = {
        "WAIT": ".",
        "RIGHT": "RIGHT",
        "LEFT": "LEFT",
        "RIGHT+A": "RIGHT JUMP",
        "LEFT+A": "LEFT JUMP",
        "RIGHT+B": "RIGHT RUN",
        "LEFT+B": "LEFT RUN",
        "RIGHT+B+A": "RIGHT RUN JUMP",
        "LEFT+B+A": "LEFT RUN JUMP",
    }
    lines = ["."] * prefix_wait
    for token in nav.split():
        buttons, hold_text, wait_text = token.split(":")
        hold = int(hold_text)
        wait = int(wait_text)
        if buttons == "WAIT":
            lines.extend(["."] * wait)
            continue
        lines.extend([mapping[buttons]] * hold)
        lines.extend(["."] * wait)
    return lines


# Vanilla Ceres pad eproj waits 60 frames, then lowers Samus to y=72 and unlocks
# input via SamusCode_0E. Match that before replaying retro_rl shaft inputs.
CERES_PAD_UNLOCK_FRAMES = 150
CERES_ELEVATOR_TO_FALLING_TILE_NAV = "RIGHT+A:24:0 RIGHT:120:0 LEFT:120:0 RIGHT+B:240:60"
CERES_START_TO_RIDLEY_NAV = (
    "RIGHT+A:24:0 RIGHT:120:0 LEFT:120:0 RIGHT+B:240:60 "
    "RIGHT:24:0 RIGHT+B:24:0 RIGHT+B+A:24:0 RIGHT+A:24:0 "
    "RIGHT:24:0 RIGHT:24:0 RIGHT:24:0 RIGHT:24:0 RIGHT+B:24:12 RIGHT:24:0 "
    "WAIT:0:140 RIGHT:160:0 LEFT:120:0 RIGHT+B:96:0 "
    "WAIT:0:120 RIGHT+B:216:0 WAIT:0:150 RIGHT+B:240:0 WAIT:0:200"
)


class TestMiniStateHash:
    @pytest.fixture(autouse=True)
    def require_mini_binary(self):
        if not MINI_BINARY.exists():
            pytest.skip("sm_rev_mini binary not built — run `make mini` first")

    def test_headless_state_hash_is_deterministic_and_input_sensitive(self, tmp_path: Path, landing_site_room: Path):
        idle_script = write_script(tmp_path / "idle.script", [".", ".", ".", ".", "."])
        spin_script = write_script(tmp_path / "spin.script", ["R", "R", "R J", "R", "R"])

        idle_cmd = [
            str(MINI_BINARY),
            "--headless",
            "--frames",
            "5",
            "--room-export",
            str(landing_site_room),
            "--input-script",
            str(idle_script),
        ]
        spin_cmd = [
            str(MINI_BINARY),
            "--headless",
            "--frames",
            "5",
            "--room-export",
            str(landing_site_room),
            "--input-script",
            str(spin_script),
        ]

        idle_run_a = run(idle_cmd)
        idle_run_b = run(idle_cmd)
        spin_run = run(spin_cmd)

        assert idle_run_a.returncode == 0, f"idle mini hash run A failed:\n{idle_run_a.stderr}\n{idle_run_a.stdout}"
        assert idle_run_b.returncode == 0, f"idle mini hash run B failed:\n{idle_run_b.stderr}\n{idle_run_b.stdout}"
        assert spin_run.returncode == 0, f"spin mini hash run failed:\n{spin_run.stderr}\n{spin_run.stdout}"

        idle_state_a = parse_json_payload(idle_run_a.stdout)
        idle_state_b = parse_json_payload(idle_run_b.stdout)
        spin_state = parse_json_payload(spin_run.stdout)

        assert idle_state_a["state_hash"] == idle_state_b["state_hash"]
        assert idle_state_a["state_hash"] != spin_state["state_hash"]

    def test_rom_landing_site_state_hash_is_deterministic_across_frame_contracts(self):
        if not any(path.exists() for path in ROM_CANDIDATES):
            pytest.skip("ROM-backed Landing Site state hash test requires a local ROM")

        def run_rom_state(frames: int) -> dict:
            result = run([
                str(MINI_BINARY),
                "--headless",
                "--frames",
                str(frames),
            ])
            assert result.returncode == 0, (
                f"ROM mini hash run for {frames} frames failed:\n{result.stderr}\n{result.stdout}"
            )
            return parse_json_payload(result.stdout)

        state_1_a = run_rom_state(1)
        state_1_b = run_rom_state(1)
        state_8_a = run_rom_state(8)
        state_8_b = run_rom_state(8)

        assert state_1_a == state_1_b
        assert state_8_a == state_8_b
        assert state_1_a["frames"] == 1
        assert state_8_a["frames"] == 8
        assert state_1_a["state_hash"] != state_8_a["state_hash"]
        for state in (state_1_a, state_8_a):
            assert state["rom_room"] is True
            assert state["original_runtime"] is True
            assert state["room_source"] in ("rom_demo", "rom_save")
            assert state["room_handle"] == "landingSite"
            assert state["room_width"] == 2304
            assert state["room_height"] == 1280
            assert 0 <= state["camera_x"] <= state["room_width"] - 256
            assert 0 <= state["camera_y"] <= state["room_height"] - 224
            assert 0 <= state["samus_world_x"] <= state["room_width"]
            assert 0 <= state["samus_world_y"] <= state["room_height"]

    def test_rom_ceres_start_boots_elevator_and_is_deterministic(self, tmp_path: Path):
        if not any(path.exists() for path in ROM_CANDIDATES):
            pytest.skip("ROM-backed Ceres start requires a local ROM")

        idle = tmp_path / "idle.script"
        idle.write_text(".\n" * 8, encoding="utf-8")
        walk = tmp_path / "walk.script"
        walk.write_text("RIGHT\n" * 240, encoding="utf-8")

        def run_ceres(frames: int, script: Path) -> dict:
            result = run([
                str(MINI_BINARY),
                "--headless",
                "--start",
                "ceres",
                "--input-script",
                str(script),
                "--frames",
                str(frames),
            ])
            assert result.returncode == 0, (
                f"Ceres mini run failed:\n{result.stderr}\n{result.stdout}"
            )
            return parse_json_payload(result.stdout)

        idle_a = run_ceres(8, idle)
        idle_b = run_ceres(8, idle)
        walk_state = run_ceres(240, walk)

        assert idle_a == idle_b
        assert idle_a["content_scope"] == "ceres"
        assert idle_a["room_source"] == "rom_ceres"
        assert idle_a["room_handle"] == "ceresElevator"
        assert idle_a["room_ptr"] == 0xDF45
        assert idle_a["original_runtime"] is True
        assert idle_a["game_state"] == 8
        assert idle_a["ceres_status"] == 0
        assert idle_a["timer_status"] == 0
        assert idle_a["no_bosses"] is False
        assert idle_a["original_enemies"] is True
        assert walk_state["state_hash"] != idle_a["state_hash"]
        assert walk_state["room_ptr"] in (0xDF45, 0xDF8D)
        if walk_state["room_ptr"] == 0xDF45:
            assert walk_state["samus_world_x"] > idle_a["samus_world_x"]

    def _run_ceres_nav(self, tmp_path: Path, nav: str, name: str) -> dict:
        script = write_script(
            tmp_path / f"{name}.script",
            expand_nav_string(nav, prefix_wait=CERES_PAD_UNLOCK_FRAMES),
        )
        frames = sum(1 for _ in script.read_text().splitlines() if _)
        result = run([
            str(MINI_BINARY),
            "--headless",
            "--start",
            "ceres",
            "--input-script",
            str(script),
            "--frames",
            str(frames),
        ])
        assert result.returncode == 0, (
            f"Ceres {name} run failed:\n{result.stderr}\n{result.stdout}"
        )
        return parse_json_payload(result.stdout)

    def test_rom_ceres_elevator_reaches_falling_tile_door(self, tmp_path: Path):
        if not any(path.exists() for path in ROM_CANDIDATES):
            pytest.skip("ROM-backed Ceres start requires a local ROM")

        # Door block (15, 39) and shaft collision come from
        # super_metroid_editor/export/sm_nav/rooms/room_DF45.json.
        # Inputs are the published retro_rl Start -> Falling Tile prefix.
        state = self._run_ceres_nav(
            tmp_path, CERES_ELEVATOR_TO_FALLING_TILE_NAV, "falling_tile"
        )
        assert state["room_source"] == "rom_ceres"
        assert state["original_runtime"] is True
        assert state["ceres_status"] == 0
        assert state["timer_status"] == 0
        assert state["room_ptr"] == 0xDF8D
        assert state["room_handle"] == "ceresFallingTile"
        assert state["game_state"] == 8

    def test_rom_ceres_elevator_reaches_ridley_room(self, tmp_path: Path):
        if not any(path.exists() for path in ROM_CANDIDATES):
            pytest.skip("ROM-backed Ceres start requires a local ROM")

        state = self._run_ceres_nav(
            tmp_path, CERES_START_TO_RIDLEY_NAV, "ridley"
        )
        assert state["room_source"] == "rom_ceres"
        assert state["original_runtime"] is True
        assert state["original_enemies"] is True
        assert state["no_bosses"] is False
        assert state["room_ptr"] == 0xE0B5
        assert state["room_handle"] == "ceresRidley"
        assert state["game_state"] == 8
        assert state["ceres_status"] == 0
        assert state["timer_status"] == 0
