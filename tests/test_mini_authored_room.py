from __future__ import annotations

import json
import subprocess
from pathlib import Path

import pytest

SM_REV_DIR = Path(__file__).parent.parent
MINI_BINARY = SM_REV_DIR / "sm_rev_mini"
LANDING_SITE_ROOM_ID = 0x91F8
K_MOVEMENT_TYPE_SPIN_JUMP = 3
K_MOVEMENT_TYPE_MORPH_BALL_ON_GROUND = 4
K_PROJECTILE_TYPE_BOMB = 0x500


def run(cmd: list[str], **kw) -> subprocess.CompletedProcess:
    cwd = kw.pop("cwd", SM_REV_DIR)
    return subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, **kw)


def parse_json_payload(stdout: str) -> dict:
    start = stdout.rfind("{")
    end = stdout.rfind("}") + 1
    if start == -1 or end == 0:
        raise ValueError(f"No JSON object found in stdout:\n{stdout!r}")
    return json.loads(stdout[start:end])


def make_room_materials(width: int = 32, height: int = 16) -> list[list[str]]:
    materials = []
    for y in range(height):
        row = []
        for x in range(width):
            if y >= 13 or x in (0, width - 1):
                row.append("solid")
            else:
                row.append("air")
        materials.append(row)
    materials[12][width - 2] = "door"
    return materials


def write_room(
    path: Path,
    materials: list[list[str]],
    *,
    bts: list[list[int]] | None = None,
    doorways: list[dict[str, int]] | None = None,
    camera_follow: dict[str, int] | None = None,
    spawn_x: int = 64,
    spawn_y: int = 192,
    camera_x: int = 0,
    camera_y: int = 32,
    room_id: int = LANDING_SITE_ROOM_ID,
    handle: str = "landingSite",
    name: str = "Tiny Authored Landing Site",
) -> Path:
    height = len(materials)
    width = len(materials[0])

    room = {
        "roomId": room_id,
        "handle": handle,
        "name": name,
        "widthScreens": width // 16,
        "heightScreens": height // 16,
        "widthBlocks": width,
        "heightBlocks": height,
        "materials": materials,
        "camera": {
            "spawnX": spawn_x,
            "spawnY": spawn_y,
            "cameraX": camera_x,
            "cameraY": camera_y,
        },
    }
    if bts is not None:
        room["bts"] = bts
    if doorways is not None:
        room["doorways"] = doorways
    if camera_follow is not None:
        room["cameraFollow"] = camera_follow
    path.write_text(json.dumps(room), encoding="utf-8")
    return path


def write_authored_room(path: Path) -> Path:
    return write_room(path, make_room_materials())


def write_script(path: Path, lines: list[str]) -> Path:
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return path


class TestMiniAuthoredRoom:
    @pytest.fixture(autouse=True)
    def require_mini_binary(self):
        if not MINI_BINARY.exists():
            pytest.skip("sm_rev_mini binary not built - run `make mini` first")

    def test_named_material_room_runs_without_rom_shaped_collision_grid(self, tmp_path: Path):
        room_path = write_authored_room(tmp_path / "authored_room.json")

        r = run([
            str(MINI_BINARY),
            "--headless",
            "--frames",
            "1",
            "--room-export",
            str(room_path),
        ], cwd=tmp_path)

        assert r.returncode == 0, f"mini authored room failed:\n{r.stderr}\n{r.stdout}"
        payload = parse_json_payload(r.stdout)
        assert payload["room_source"] == "editor_export"
        assert payload["room_handle"] == "landingSite"
        assert payload["room_width"] == 512
        assert payload["room_height"] == 256
        assert payload["rom_room"] is False
        assert payload["original_runtime"] is False

    def test_non_landing_material_room_is_blocked_by_content_scope(self, tmp_path: Path):
        room_path = write_room(
            tmp_path / "non_landing_room.json",
            make_room_materials(),
            room_id=0x92B3,
            handle="nonLanding",
            name="Non-Landing Authored Room",
        )

        r = run([
            str(MINI_BINARY),
            "--headless",
            "--frames",
            "1",
            "--room-export",
            str(room_path),
        ], cwd=tmp_path)

        assert r.returncode == 0, f"mini non-Landing scope run failed:\n{r.stderr}\n{r.stdout}"
        payload = parse_json_payload(r.stdout)
        assert payload["content_scope"] == "landing_site_only"
        assert payload["room_source"] != "editor_export"
        assert payload["room_handle"] != "nonLanding"
        assert payload["room_ptr"] != 0x92B3

    def test_named_material_room_traversal_is_deterministic(self, tmp_path: Path):
        room_path = write_authored_room(tmp_path / "authored_room.json")
        idle_script = write_script(tmp_path / "idle.script", ["."] * 30)
        spin_script = write_script(tmp_path / "spin.script", ["RIGHT", "RIGHT", "RIGHT JUMP"] + ["RIGHT"] * 9)

        base_cmd = [
            str(MINI_BINARY),
            "--headless",
            "--room-export",
            str(room_path),
            "--input-script",
        ]
        idle_a = run(base_cmd + [str(idle_script), "--frames", "30"], cwd=tmp_path)
        idle_b = run(base_cmd + [str(idle_script), "--frames", "30"], cwd=tmp_path)
        spin_a = run(base_cmd + [str(spin_script), "--frames", "12"], cwd=tmp_path)
        spin_b = run(base_cmd + [str(spin_script), "--frames", "12"], cwd=tmp_path)

        assert idle_a.returncode == 0, f"idle authored run failed:\n{idle_a.stderr}\n{idle_a.stdout}"
        assert idle_b.returncode == 0, f"second idle authored run failed:\n{idle_b.stderr}\n{idle_b.stdout}"
        assert spin_a.returncode == 0, f"spin authored run failed:\n{spin_a.stderr}\n{spin_a.stdout}"
        assert spin_b.returncode == 0, f"second spin authored run failed:\n{spin_b.stderr}\n{spin_b.stdout}"

        idle_a_state = parse_json_payload(idle_a.stdout)
        idle_b_state = parse_json_payload(idle_b.stdout)
        spin_a_state = parse_json_payload(spin_a.stdout)
        spin_b_state = parse_json_payload(spin_b.stdout)

        assert idle_a_state["state_hash"] == idle_b_state["state_hash"]
        assert spin_a_state["state_hash"] == spin_b_state["state_hash"]
        assert spin_a_state["state_hash"] != idle_a_state["state_hash"]
        assert spin_a_state["samus_x"] > idle_a_state["samus_x"]
        assert spin_a_state["samus_movement_type"] == K_MOVEMENT_TYPE_SPIN_JUMP

    def test_named_material_room_uses_physics_mod_config(self, tmp_path: Path):
        room_path = write_authored_room(tmp_path / "authored_room.json")
        run_script = write_script(tmp_path / "run.script", ["RIGHT"] * 10)
        cmd = [
            str(MINI_BINARY),
            "--headless",
            "--room-export",
            str(room_path),
            "--input-script",
            str(run_script),
            "--frames",
            "10",
        ]

        vanilla = run(cmd, cwd=tmp_path)
        assert vanilla.returncode == 0, f"vanilla authored run failed:\n{vanilla.stderr}\n{vanilla.stdout}"
        (tmp_path / "sm_mods.json").write_text(json.dumps({"run_speed_scale_percent": 200}), encoding="utf-8")
        boosted = run(cmd, cwd=tmp_path)
        assert boosted.returncode == 0, f"boosted authored run failed:\n{boosted.stderr}\n{boosted.stdout}"

        vanilla_state = parse_json_payload(vanilla.stdout)
        boosted_state = parse_json_payload(boosted.stdout)
        assert boosted_state["samus_x"] > vanilla_state["samus_x"]

    def test_named_material_room_camera_follows_traversal(self, tmp_path: Path):
        room_path = write_authored_room(tmp_path / "authored_room.json")
        run_script = write_script(tmp_path / "run.script", ["RIGHT"] * 80)

        r = run([
            str(MINI_BINARY),
            "--headless",
            "--room-export",
            str(room_path),
            "--input-script",
            str(run_script),
            "--frames",
            "80",
        ], cwd=tmp_path)

        assert r.returncode == 0, f"authored camera run failed:\n{r.stderr}\n{r.stdout}"
        state = parse_json_payload(r.stdout)
        assert state["camera_x"] > 0
        assert state["camera_x"] <= state["room_width"] - 256
        assert state["camera_target_x_percent"] == 50
        assert state["camera_target_y_percent"] == 50

    def test_named_material_room_uses_authored_camera_follow_target(self, tmp_path: Path):
        default_room_path = write_authored_room(tmp_path / "default_camera_room.json")
        lookahead_room_path = write_room(
            tmp_path / "lookahead_camera_room.json",
            make_room_materials(),
            camera_follow={
                "targetXPercent": 25,
                "targetYPercent": 50,
            },
            name="Lookahead Camera Authored Landing Site",
        )
        run_script = write_script(tmp_path / "run.script", ["RIGHT"] * 80)

        base_cmd = [
            str(MINI_BINARY),
            "--headless",
            "--input-script",
            str(run_script),
            "--frames",
            "80",
            "--room-export",
        ]
        default_run = run(base_cmd + [str(default_room_path)], cwd=tmp_path)
        lookahead_run = run(base_cmd + [str(lookahead_room_path)], cwd=tmp_path)

        assert default_run.returncode == 0, (
            f"default authored camera run failed:\n{default_run.stderr}\n{default_run.stdout}"
        )
        assert lookahead_run.returncode == 0, (
            f"lookahead authored camera run failed:\n{lookahead_run.stderr}\n{lookahead_run.stdout}"
        )
        default_state = parse_json_payload(default_run.stdout)
        lookahead_state = parse_json_payload(lookahead_run.stdout)

        assert lookahead_state["samus_world_x"] == default_state["samus_world_x"]
        assert lookahead_state["camera_target_x_percent"] == 25
        assert lookahead_state["camera_target_y_percent"] == 50
        assert lookahead_state["camera_x"] > default_state["camera_x"]
        assert lookahead_state["state_hash"] != default_state["state_hash"]

    def test_named_material_room_traverses_authored_slope(self, tmp_path: Path):
        materials = make_room_materials()
        bts = [[0 for _ in row] for row in materials]
        for x in range(6, 14):
            materials[12][x] = "slope"
        room_path = write_room(
            tmp_path / "slope_room.json",
            materials,
            bts=bts,
            name="Slope Authored Landing Site",
        )
        run_script = write_script(tmp_path / "run.script", ["RIGHT"] * 28)

        r = run([
            str(MINI_BINARY),
            "--headless",
            "--room-export",
            str(room_path),
            "--input-script",
            str(run_script),
            "--frames",
            "28",
        ], cwd=tmp_path)

        assert r.returncode == 0, f"authored slope run failed:\n{r.stderr}\n{r.stdout}"
        state = parse_json_payload(r.stdout)
        assert state["samus_world_x"] > 120
        assert state["samus_world_y"] < 192
        assert state["samus_on_ground"] is True

    def test_named_material_room_treats_door_as_passable_marker(self, tmp_path: Path):
        room_path = write_authored_room(tmp_path / "door_room.json")
        run_script = write_script(tmp_path / "run.script", ["RIGHT"] * 160)

        r = run([
            str(MINI_BINARY),
            "--headless",
            "--room-export",
            str(room_path),
            "--input-script",
            str(run_script),
            "--frames",
            "160",
        ], cwd=tmp_path)

        assert r.returncode == 0, f"authored door-marker run failed:\n{r.stderr}\n{r.stdout}"
        state = parse_json_payload(r.stdout)
        assert state["samus_world_x"] > 30 * 16
        assert state["camera_x"] == state["room_width"] - 256

    def test_named_material_room_supports_morph_ball_tunnel_traversal(self, tmp_path: Path):
        materials = make_room_materials(width=48)
        for x in range(8, 22):
            materials[11][x] = "solid"
        room_path = write_room(
            tmp_path / "morph_tunnel_room.json",
            materials,
            name="Morph Tunnel Authored Landing Site",
        )
        frame_count = 110
        stand_script = write_script(tmp_path / "stand_run.script", ["RIGHT"] * frame_count)
        morph_script = write_script(tmp_path / "morph_run.script",
                                    ["DOWN"] + ["RIGHT"] * (frame_count - 1))
        base_cmd = [
            str(MINI_BINARY),
            "--headless",
            "--room-export",
            str(room_path),
            "--input-script",
        ]

        standing_run = run(
            base_cmd + [str(stand_script), "--frames", str(frame_count)],
            cwd=tmp_path,
        )
        morph_run = run(
            base_cmd + [str(morph_script), "--frames", str(frame_count)],
            cwd=tmp_path,
        )

        assert standing_run.returncode == 0, (
            f"standing tunnel run failed:\n{standing_run.stderr}\n{standing_run.stdout}"
        )
        assert morph_run.returncode == 0, (
            f"morph tunnel run failed:\n{morph_run.stderr}\n{morph_run.stdout}"
        )
        standing_state = parse_json_payload(standing_run.stdout)
        morph_state = parse_json_payload(morph_run.stdout)

        assert standing_state["samus_world_x"] < 8 * 16
        assert morph_state["samus_world_x"] > 22 * 16
        assert morph_state["samus_movement_type"] == K_MOVEMENT_TYPE_MORPH_BALL_ON_GROUND
        assert morph_state["samus_on_ground"] is True
        assert morph_state["state_hash"] != standing_state["state_hash"]

    def test_named_material_room_applies_authored_doorway_transition(self, tmp_path: Path):
        room_path = write_room(
            tmp_path / "door_transition_room.json",
            make_room_materials(),
            doorways=[
                {
                    "blockX": 30,
                    "blockY": 12,
                    "targetX": 64,
                    "targetY": 192,
                    "cameraX": 0,
                    "cameraY": 32,
                }
            ],
            name="Door Transition Authored Landing Site",
        )
        run_script = write_script(tmp_path / "run.script", ["RIGHT"] * 160)

        transition_a = run([
            str(MINI_BINARY),
            "--headless",
            "--room-export",
            str(room_path),
            "--input-script",
            str(run_script),
            "--frames",
            "160",
        ], cwd=tmp_path)
        transition_b = run([
            str(MINI_BINARY),
            "--headless",
            "--room-export",
            str(room_path),
            "--input-script",
            str(run_script),
            "--frames",
            "160",
        ], cwd=tmp_path)

        assert transition_a.returncode == 0, (
            f"authored doorway transition run failed:\n{transition_a.stderr}\n{transition_a.stdout}"
        )
        assert transition_b.returncode == 0, (
            f"second authored doorway transition run failed:\n{transition_b.stderr}\n{transition_b.stdout}"
        )
        state_a = parse_json_payload(transition_a.stdout)
        state_b = parse_json_payload(transition_b.stdout)
        assert state_a["state_hash"] == state_b["state_hash"]
        assert state_a["samus_world_x"] < 180
        assert state_a["samus_world_y"] == 192
        assert state_a["camera_x"] == 0

    def test_named_material_room_supports_airborne_wall_jump(self, tmp_path: Path):
        materials = make_room_materials()
        for y in range(8, 13):
            materials[y][8] = "wall"
        room_path = write_room(
            tmp_path / "wall_jump_room.json",
            materials,
            spawn_x=121,
            spawn_y=176,
            name="Wall Jump Authored Landing Site",
        )
        wall_jump_script = write_script(tmp_path / "wall_jump.script", ["JUMP"] + ["LEFT"] * 8)

        r = run([
            str(MINI_BINARY),
            "--headless",
            "--room-export",
            str(room_path),
            "--input-script",
            str(wall_jump_script),
            "--frames",
            "9",
        ], cwd=tmp_path)

        assert r.returncode == 0, f"authored wall jump failed:\n{r.stderr}\n{r.stdout}"
        state = parse_json_payload(r.stdout)
        assert state["samus_world_x"] < 121
        assert state["samus_world_y"] < 176
        assert state["samus_movement_type"] == K_MOVEMENT_TYPE_SPIN_JUMP

    def test_named_material_room_supports_bomb_jump(self, tmp_path: Path):
        room_path = write_authored_room(tmp_path / "bomb_jump_room.json")
        bomb_script = write_script(tmp_path / "bomb_jump.script", ["DOWN SHOOT"] + ["."] * 8)
        idle_script = write_script(tmp_path / "idle.script", ["."] * 9)
        base_cmd = [
            str(MINI_BINARY),
            "--headless",
            "--room-export",
            str(room_path),
            "--input-script",
        ]

        bomb_a = run(base_cmd + [str(bomb_script), "--frames", "9"], cwd=tmp_path)
        bomb_b = run(base_cmd + [str(bomb_script), "--frames", "9"], cwd=tmp_path)
        idle = run(base_cmd + [str(idle_script), "--frames", "9"], cwd=tmp_path)

        assert bomb_a.returncode == 0, f"authored bomb-jump run failed:\n{bomb_a.stderr}\n{bomb_a.stdout}"
        assert bomb_b.returncode == 0, f"second authored bomb-jump run failed:\n{bomb_b.stderr}\n{bomb_b.stdout}"
        assert idle.returncode == 0, f"authored bomb-jump idle run failed:\n{idle.stderr}\n{idle.stdout}"

        bomb_a_state = parse_json_payload(bomb_a.stdout)
        bomb_b_state = parse_json_payload(bomb_b.stdout)
        idle_state = parse_json_payload(idle.stdout)

        assert bomb_a_state["state_hash"] == bomb_b_state["state_hash"]
        assert bomb_a_state["state_hash"] != idle_state["state_hash"]
        assert bomb_a_state["samus_world_y"] < idle_state["samus_world_y"]
        assert bomb_a_state["samus_on_ground"] is False
        assert bomb_a_state["samus_movement_type"] == K_MOVEMENT_TYPE_SPIN_JUMP
        assert bomb_a_state["projectile_count"] >= 1
        assert bomb_a_state["first_projectile_type"] == K_PROJECTILE_TYPE_BOMB
