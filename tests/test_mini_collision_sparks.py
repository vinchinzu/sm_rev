from __future__ import annotations

import importlib.util
import subprocess
from pathlib import Path

import pytest


REPO_ROOT = Path(__file__).parent.parent
SERVER_PATH = REPO_ROOT / "tools" / "mini_browser_server.py"
SPARK_OUTER = bytes((255, 90, 24, 255))
SPARK_INNER = bytes((255, 216, 61, 255))
SPARK_CORE = bytes((255, 255, 255, 255))


@pytest.fixture(scope="module")
def mini_browser_module():
    result = subprocess.run(
        ["make", "mini-browser-lib"],
        cwd=REPO_ROOT,
        capture_output=True,
        text=True,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    spec = importlib.util.spec_from_file_location("mini_browser_server", SERVER_PATH)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def pixel(frame: bytes, x: int, y: int) -> bytes:
    offset = (y * 256 + x) * 4
    return frame[offset:offset + 4]


def expected_impact(snapshot, event) -> tuple[int, int]:
    attacker_index = event.attacker - 1
    defender_index = event.defender - 1
    attacker = snapshot.players[attacker_index]
    defender = snapshot.players[defender_index]
    if attacker.world_x == defender.world_x:
        impact_side = -1 if attacker_index < defender_index else 1
    else:
        impact_side = -1 if attacker.world_x < defender.world_x else 1
    return (
        defender.world_x + impact_side * (defender.x_radius + 2) - snapshot.camera_x,
        defender.world_y - defender.y_radius // 4 - snapshot.camera_y,
    )


def assert_spark_at(frame: bytes, x: int, y: int) -> None:
    assert all(
        pixel(frame, px, py) == SPARK_CORE
        for py in range(y - 2, y + 3)
        for px in range(x - 2, x + 3)
    )
    assert pixel(frame, x - 10, y) == SPARK_OUTER
    assert pixel(frame, x + 10, y) == SPARK_OUTER
    assert pixel(frame, x, y - 10) == SPARK_OUTER
    assert pixel(frame, x, y + 10) == SPARK_OUTER
    assert pixel(frame, x - 4, y) == SPARK_INNER
    assert pixel(frame, x + 4, y) == SPARK_INNER


def test_hit_queue_renders_deterministic_camera_correct_sparks_for_both_players(
    mini_browser_module,
):
    seen_pairs: set[tuple[int, int]] = set()
    with mini_browser_module.MiniKernel.from_repo(REPO_ROOT, build=False) as kernel:
        for _ in range(20):
            kernel.step2(mini_browser_module.BUTTON_X, mini_browser_module.BUTTON_X)
            state = kernel.snapshot(1)
            if state.hit_event_count == 0:
                continue

            assert state.hit_event_count == 2

            for focus_player in (1, 2):
                focused_state = kernel.snapshot(focus_player)
                frame = kernel.render_rgba(focus_player)
                assert frame == kernel.render_rgba(focus_player)
                for event_index in range(focused_state.hit_event_count):
                    event = focused_state.hit_events[event_index]
                    seen_pairs.add((event.attacker, event.defender))
                    assert_spark_at(frame, *expected_impact(focused_state, event))

        assert seen_pairs == {(1, 2), (2, 1)}
