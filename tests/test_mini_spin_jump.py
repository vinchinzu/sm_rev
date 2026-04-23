from __future__ import annotations

import json
import subprocess
from pathlib import Path

import pytest

SM_REV_DIR = Path(__file__).parent.parent
MINI_BINARY = SM_REV_DIR / "sm_rev_mini"
LANDING_SITE_EXPORT = SM_REV_DIR.parent / "super_metroid_editor" / "export" / "sm_nav" / "rooms" / "room_91F8.json"

K_MOVEMENT_TYPE_SPIN_JUMP = 3
K_POSE_FACE_RIGHT_SPIN_JUMP = 0x19
K_POSE_FACE_LEFT_SPIN_JUMP = 0x1A


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
def mini_room_and_script(tmp_path: Path) -> tuple[Path, Path]:
    if not LANDING_SITE_EXPORT.exists():
        pytest.skip("Landing Site editor export not present — mini spin-jump replay test requires editor export data")

    room_path = tmp_path / "spin_jump_room.json"
    room_path.write_text(LANDING_SITE_EXPORT.read_text())

    script_path = tmp_path / "spin_jump.script"
    script_path.write_text("R\nR\nR J\nR\nR\n")
    return room_path, script_path


class TestMiniSpinJumpReplay:
    @pytest.fixture(autouse=True)
    def require_mini_binary(self):
        if not MINI_BINARY.exists():
            pytest.skip("sm_rev_mini binary not built — run `make mini` first")

    def test_editor_export_input_script_spin_jump_regression(self, mini_room_and_script: tuple[Path, Path]):
        room_path, script_path = mini_room_and_script
        cmd = [
            str(MINI_BINARY),
            "--headless",
            "--frames",
            "5",
            "--room-export",
            str(room_path),
            "--input-script",
            str(script_path),
        ]

        r1 = run(cmd)
        r2 = run(cmd)

        assert r1.returncode == 0, f"first mini spin-jump replay run failed:\n{r1.stderr}\n{r1.stdout}"
        assert r2.returncode == 0, f"second mini spin-jump replay run failed:\n{r2.stderr}\n{r2.stdout}"

        state1 = parse_json_payload(r1.stdout)
        state2 = parse_json_payload(r2.stdout)
        assert state1 == state2, f"mini replay output was not deterministic:\nrun1={state1}\nrun2={state2}"

        assert state1["room_source"] == "editor_export"
        assert state1["rom_room"] is False
        assert state1["room_handle"] == "landingSite"
        assert state1["frames"] == 5
        assert state1["samus_movement_type"] == K_MOVEMENT_TYPE_SPIN_JUMP
        assert state1["samus_pose"] in (K_POSE_FACE_RIGHT_SPIN_JUMP, K_POSE_FACE_LEFT_SPIN_JUMP)
