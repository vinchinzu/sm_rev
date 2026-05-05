from __future__ import annotations

import json
import subprocess
from pathlib import Path

import pytest


SM_REV_DIR = Path(__file__).parent.parent
MINI_BINARY = SM_REV_DIR / "sm_rev_mini"
LANDING_SITE_ROOM_ID = 0x91F8


def run(cmd: list[str], **kw) -> subprocess.CompletedProcess:
    cwd = kw.pop("cwd", SM_REV_DIR)
    return subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, **kw)


def parse_json_payload(stdout: str) -> dict:
    start = stdout.find("{")
    end = stdout.rfind("}") + 1
    assert start != -1 and end > start, f"No JSON object found in stdout:\n{stdout!r}"
    return json.loads(stdout[start:end])


def write_multiplayer_room(path: Path) -> Path:
    width = 32
    height = 16
    materials = [
        ["solid" if y >= 13 or x in (0, width - 1) else "air" for x in range(width)]
        for y in range(height)
    ]
    room = {
        "roomId": LANDING_SITE_ROOM_ID,
        "handle": "landingSite",
        "name": "Mini Multiplayer Test Landing Site",
        "widthScreens": 2,
        "heightScreens": 1,
        "widthBlocks": width,
        "heightBlocks": height,
        "materials": materials,
        "camera": {
            "spawnX": 64,
            "spawnY": 192,
            "cameraX": 0,
            "cameraY": 32,
        },
    }
    path.write_text(json.dumps(room), encoding="utf-8")
    return path


def require_mini_binary() -> None:
    if not MINI_BINARY.exists():
        pytest.skip("sm_rev_mini binary not built - run `make mini` first")


def test_two_player_spawn_and_p2_movement_are_independent(tmp_path: Path):
    require_mini_binary()
    room = write_multiplayer_room(tmp_path / "room.json")
    script = tmp_path / "p2_right.script"
    script.write_text(("P2:RIGHT\n" * 12), encoding="utf-8")

    result = run([
        str(MINI_BINARY),
        "--headless",
        "--players",
        "2",
        "--frames",
        "12",
        "--room-export",
        str(room),
        "--input-script",
        str(script),
    ], cwd=tmp_path)
    assert result.returncode == 0, f"mini multiplayer movement failed:\n{result.stderr}\n{result.stdout}"
    payload = parse_json_payload(result.stdout)

    assert payload["player_count"] == 2
    assert payload["rom_room"] is False
    assert payload["player1_world_x"] == 64
    assert payload["player2_world_x"] > 64 + 48
    assert payload["player1_world_x"] != payload["player2_world_x"]
    assert payload["player2_buttons"] != 0


def test_two_player_basic_beam_records_hit_without_hud_damage(tmp_path: Path):
    require_mini_binary()
    room = write_multiplayer_room(tmp_path / "room.json")
    script = tmp_path / "p1_shoot.script"
    script.write_text("P1:SHOOT\n" + (".\n" * 12), encoding="utf-8")

    result = run([
        str(MINI_BINARY),
        "--headless",
        "--players",
        "2",
        "--frames",
        "13",
        "--room-export",
        str(room),
        "--input-script",
        str(script),
    ], cwd=tmp_path)
    assert result.returncode == 0, f"mini multiplayer hit test failed:\n{result.stderr}\n{result.stdout}"
    payload = parse_json_payload(result.stdout)

    assert payload["player1_hit_count"] == 0
    assert payload["player2_hit_count"] == 1
    assert payload["player2_pending_damage"] == 20
    assert payload["player2_last_hit_by_player"] == 0
    assert payload["projectile_count"] == 0


def test_two_player_multiplayer_demo_defaults_to_non_rom_exchange():
    require_mini_binary()

    result = run([
        str(MINI_BINARY),
        "--headless",
        "--multiplayer-demo",
        "--frames",
        "21",
    ])
    assert result.returncode == 0, f"mini multiplayer demo failed:\n{result.stderr}\n{result.stdout}"
    payload = parse_json_payload(result.stdout)

    assert payload["player_count"] == 2
    assert payload["rom_room"] is False
    assert payload["original_runtime"] is False
    assert payload["player1_hit_count"] == 1
    assert payload["player2_hit_count"] == 1
    assert payload["player1_pending_damage"] == 20
    assert payload["player2_pending_damage"] == 20
    assert payload["player1_last_hit_by_player"] == 1
    assert payload["player2_last_hit_by_player"] == 0
    assert payload["projectile_count"] == 0


def test_two_player_replay_roundtrip_preserves_player_inputs(tmp_path: Path):
    require_mini_binary()
    room = write_multiplayer_room(tmp_path / "room.json")
    script = tmp_path / "mixed.script"
    script.write_text("P1:SHOOT P2:LEFT\nP2:RIGHT\nP1:. P2:JUMP\n", encoding="utf-8")
    replay = tmp_path / "mini_multiplayer_replay.json"

    write_result = run([
        str(MINI_BINARY),
        "--headless",
        "--players",
        "2",
        "--frames",
        "3",
        "--room-export",
        str(room),
        "--input-script",
        str(script),
        "--replay-out",
        str(replay),
    ], cwd=tmp_path)
    assert write_result.returncode == 0, (
        f"mini multiplayer replay write failed:\n{write_result.stderr}\n{write_result.stdout}"
    )
    write_payload = parse_json_payload(write_result.stdout)
    artifact = json.loads(replay.read_text(encoding="utf-8"))
    assert artifact["player_count"] == 2
    assert artifact["inputs"][0]["buttons"] != 0
    assert artifact["inputs"][0]["player_buttons"][0] != 0
    assert artifact["inputs"][0]["player_buttons"][1] != 0

    read_result = run([
        str(MINI_BINARY),
        "--headless",
        "--replay-in",
        str(replay),
    ], cwd=tmp_path)
    assert read_result.returncode == 0, (
        f"mini multiplayer replay read failed:\n{read_result.stderr}\n{read_result.stdout}"
    )
    read_payload = parse_json_payload(read_result.stdout)
    assert read_payload["player_count"] == 2
    assert read_payload["state_hash"] == write_payload["state_hash"]
    assert read_payload["replay_verified"] is True
