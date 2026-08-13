from __future__ import annotations

import json
import struct
import subprocess
from pathlib import Path

import pytest


SM_REV_DIR = Path(__file__).parent.parent
MINI_BINARY = SM_REV_DIR / "sm_rev_mini"


def run(cmd: list[str], **kw) -> subprocess.CompletedProcess:
    cwd = kw.pop("cwd", SM_REV_DIR)
    return subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, **kw)


def parse_json_payload(stdout: str) -> dict:
    start = stdout.find("{")
    end = stdout.rfind("}") + 1
    assert start != -1 and end > start, f"No JSON object found in stdout:\n{stdout!r}"
    return json.loads(stdout[start:end])


def count_samus_debug_colors(path: Path) -> tuple[int, int]:
    data = path.read_bytes()
    pixel_offset = struct.unpack_from("<I", data, 10)[0]
    width, height = struct.unpack_from("<ii", data, 18)
    bits_per_pixel = struct.unpack_from("<H", data, 28)[0]
    assert bits_per_pixel == 32, f"expected 32bpp BMP, got {bits_per_pixel}"
    row_stride = abs(width) * 4
    yellow_pixels = 0
    green_pixels = 0
    for y in range(abs(height)):
        src_y = abs(height) - 1 - y if height > 0 else y
        row_start = pixel_offset + src_y * row_stride
        for x in range(abs(width)):
            pixel = struct.unpack_from("<I", data, row_start + x * 4)[0]
            red = (pixel >> 16) & 0xFF
            green = (pixel >> 8) & 0xFF
            blue = pixel & 0xFF
            if red >= 180 and green >= 120 and blue <= 96:
                yellow_pixels += 1
            if green >= 150 and red < 140 and blue < 140:
                green_pixels += 1
    return yellow_pixels, green_pixels


def require_mini_binary() -> None:
    if not MINI_BINARY.exists():
        pytest.skip("sm_rev_mini binary not built - run `make mini` first")


def test_two_player_defaults_to_rom_runtime_and_spawns_two_samuses():
    require_mini_binary()

    result = run([
        str(MINI_BINARY),
        "--headless",
        "--players",
        "2",
        "--frames",
        "1",
    ])
    assert result.returncode == 0, f"mini multiplayer spawn failed:\n{result.stderr}\n{result.stdout}"
    payload = parse_json_payload(result.stdout)

    assert payload["player_count"] == 2
    assert payload["rom_room"] is True
    assert payload["original_runtime"] is True
    assert payload["room_source"] == "rom_demo"
    assert payload["room_visuals"] == "rom"
    assert payload["player1_world_x"] != payload["player2_world_x"]
    assert payload["player2_world_x"] > payload["player1_world_x"]


def test_two_player_p2_input_moves_second_samus_in_rom_runtime(tmp_path: Path):
    require_mini_binary()
    script = tmp_path / "p2_right.script"
    script.write_text(("P2:RIGHT\n" * 60), encoding="utf-8")

    result = run([
        str(MINI_BINARY),
        "--headless",
        "--players",
        "2",
        "--frames",
        "60",
        "--input-script",
        str(script),
    ])
    assert result.returncode == 0, f"mini multiplayer P2 movement failed:\n{result.stderr}\n{result.stdout}"
    payload = parse_json_payload(result.stdout)

    assert payload["rom_room"] is True
    assert payload["original_runtime"] is True
    assert payload["player1_world_x"] == 1153
    assert payload["player2_world_x"] > 1201
    assert payload["player2_buttons"] != 0


def test_two_player_screenshot_does_not_tint_or_wrap_offscreen_p2(tmp_path: Path):
    require_mini_binary()
    script = tmp_path / "p2_right.script"
    script.write_text(("P2:RIGHT\n" * 60), encoding="utf-8")
    single_frame = tmp_path / "single.bmp"
    multiplayer_frame = tmp_path / "multi.bmp"

    single = run([
        str(MINI_BINARY),
        "--headless",
        "--frames",
        "60",
        "--screenshot",
        str(single_frame),
    ])
    assert single.returncode == 0, f"single-player screenshot failed:\n{single.stderr}\n{single.stdout}"

    multiplayer = run([
        str(MINI_BINARY),
        "--headless",
        "--players",
        "2",
        "--frames",
        "60",
        "--input-script",
        str(script),
        "--screenshot",
        str(multiplayer_frame),
    ])
    assert multiplayer.returncode == 0, (
        f"mini multiplayer screenshot failed:\n{multiplayer.stderr}\n{multiplayer.stdout}"
    )
    payload = parse_json_payload(multiplayer.stdout)
    assert payload["player2_screen_x"] >= 256

    single_yellow, single_green = count_samus_debug_colors(single_frame)
    multi_yellow, multi_green = count_samus_debug_colors(multiplayer_frame)
    assert multi_yellow <= single_yellow + 32
    assert multi_green <= single_green + 8


def test_two_player_multiplayer_demo_uses_rom_runtime():
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
    assert payload["rom_room"] is True
    assert payload["original_runtime"] is True
    assert payload["room_source"] == "rom_demo"
    assert payload["samus_suit"] == "power"
    assert payload["equipped_items"] == 0x1004
    assert payload["equipped_beams"] == 0
    assert payload["player1_world_x"] > 1153
    assert payload["player2_world_x"] < 1201
    assert payload["player1_hit_count"] >= 1
    assert payload["player2_hit_count"] >= 1
    assert payload["player1_pending_damage"] > 0
    assert payload["player2_pending_damage"] > 0
    assert payload["player1_last_hit_by_player"] == 2
    assert payload["player2_last_hit_by_player"] == 1
    assert payload["hit_event_count"] == len(payload["hit_events"])
    assert payload["hit_event_dropped_count"] == 0


def test_two_player_replay_roundtrip_preserves_player_inputs(tmp_path: Path):
    require_mini_binary()
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
        "--input-script",
        str(script),
        "--replay-out",
        str(replay),
    ])
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
    ])
    assert read_result.returncode == 0, (
        f"mini multiplayer replay read failed:\n{read_result.stderr}\n{read_result.stdout}"
    )
    read_payload = parse_json_payload(read_result.stdout)
    assert read_payload["player_count"] == 2
    assert read_payload["state_hash"] == write_payload["state_hash"]
    assert read_payload["replay_verified"] is True
