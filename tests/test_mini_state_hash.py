from __future__ import annotations

import json
import subprocess
from pathlib import Path

import pytest

SM_REV_DIR = Path(__file__).parent.parent
MINI_BINARY = SM_REV_DIR / "sm_rev_mini"
LANDING_SITE_EXPORT = SM_REV_DIR.parent / "super_metroid_editor" / "export" / "sm_nav" / "rooms" / "room_91F8.json"


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
