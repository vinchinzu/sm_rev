"""Deterministic save-state checks for Torizo mod presets."""

from __future__ import annotations

import json
import os
import subprocess
from pathlib import Path

import pytest

SM_REV_DIR = Path(__file__).parent.parent
BINARY = SM_REV_DIR / "sm_rev"
ROM = SM_REV_DIR / "sm.smc"
TORIZO_PREFIGHT_SAVE = SM_REV_DIR / "saves" / "boss" / "torizo_pre_fight.sav"

HEADLESS_ENV = {
    "SDL_VIDEODRIVER": "offscreen",
    "SDL_AUDIODRIVER": "dummy",
    "DISPLAY": "",
}

BLUE_ORB_OPENING_PRESET = {
    "chozo_orbs": {
        "spawn_multiplier": 10,
        "contact_damage_multiplier": 2,
        "palette": "blue",
        "freeze_samus_frames": 0,
        "spawn_jitter_pixels": 14,
        "bomb_burst_count": -1,
        "golden_burst_count": -1,
        "max_per_burst": 18,
    },
    "attack_order": {
        "bomb_opening_attack": "chozo_orbs",
        "bomb_opening_chozo_orb_burst_count": 18,
        "bomb_opening_chozo_orb_wave_count": 5,
        "bomb_opening_chozo_orb_wave_interval_frames": 10,
    },
    "finish_explosion": {
        "body_burst_multiplier": 3,
        "body_burst_count": -1,
        "final_burst_multiplier": 3,
        "final_burst_count": -1,
        "flash_frames": 40,
        "position_jitter_pixels": 6,
        "max_per_burst": 18,
    },
}

DETERMINISTIC_FIELDS = (
    "game_state",
    "area_index",
    "room_ptr",
    "samus_health",
    "samus_x_pos",
    "samus_y_pos",
    "torizo_enemy_count",
    "torizo_opening_flags",
    "torizo_opening_waves_remaining",
    "torizo_opening_wave_timer",
    "torizo_chozo_orb_count",
    "active_eproj_count",
)


def parse_dump(r: subprocess.CompletedProcess) -> dict:
    start = r.stdout.rfind("{")
    end = r.stdout.rfind("}") + 1
    if start == -1 or end == 0:
        raise ValueError(f"No JSON object found in stdout:\n{r.stdout!r}")
    return json.loads(r.stdout[start:end])


def run_torizo_preset(tmp_path: Path, frames: int = 600) -> subprocess.CompletedProcess:
    tmp_path.mkdir(parents=True, exist_ok=True)
    config = tmp_path / "sm.ini"
    config.write_text("[General]\nSkipIntro=1\n[Features]\nFullSpec=0\n", encoding="utf-8")
    (tmp_path / "sm_torizo.json").write_text(
        json.dumps(BLUE_ORB_OPENING_PRESET, sort_keys=True) + "\n",
        encoding="utf-8",
    )

    env = os.environ.copy()
    env.update(HEADLESS_ENV)
    return subprocess.run(
        [
            str(BINARY),
            "--config",
            str(config),
            "--headless",
            str(frames),
            "--runmode",
            "mine",
            "--dump",
            "-",
            "--load-state",
            str(TORIZO_PREFIGHT_SAVE.resolve()),
            str(ROM.resolve()),
        ],
        cwd=tmp_path,
        capture_output=True,
        text=True,
        env=env,
        timeout=60,
    )


@pytest.fixture(autouse=True)
def require_runtime_assets():
    if not BINARY.exists():
        pytest.skip("sm_rev binary not built")
    if not ROM.exists():
        pytest.skip("sm.smc ROM not present")
    if not TORIZO_PREFIGHT_SAVE.exists():
        pytest.skip("Torizo pre-fight fixture missing")


def test_blue_orb_opening_is_deterministic(tmp_path: Path):
    first = run_torizo_preset(tmp_path / "first")
    second = run_torizo_preset(tmp_path / "second")
    assert first.returncode == 0, f"first Torizo run failed:\n{first.stderr}\n{first.stdout}"
    assert second.returncode == 0, f"second Torizo run failed:\n{second.stderr}\n{second.stdout}"

    first_dump = parse_dump(first)
    second_dump = parse_dump(second)
    assert {field: first_dump[field] for field in DETERMINISTIC_FIELDS} == {
        field: second_dump[field] for field in DETERMINISTIC_FIELDS
    }

    assert first_dump["game_state"] == 8
    assert first_dump["room_ptr"] == 38916
    assert first_dump["torizo_enemy_count"] == 1
    assert first_dump["torizo_opening_flags"] == 3
    assert first_dump["torizo_opening_waves_remaining"] == 4
    assert first_dump["torizo_opening_wave_timer"] == 10
    assert first_dump["torizo_chozo_orb_count"] == 18
    assert first_dump["active_eproj_count"] == 18
