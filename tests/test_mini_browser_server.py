from __future__ import annotations

import importlib.util
import json
import subprocess
import threading
from http.server import ThreadingHTTPServer
from pathlib import Path
from urllib.request import Request, urlopen

import pytest


SM_REV_DIR = Path(__file__).parent.parent
SERVER_PATH = SM_REV_DIR / "tools" / "mini_browser_server.py"


def run(cmd: list[str], **kw) -> subprocess.CompletedProcess:
    cwd = kw.pop("cwd", SM_REV_DIR)
    return subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, **kw)


def count_bright_samus_pixels_near(frame: bytes, center_x: int, half_width: int = 16) -> int:
    count = 0
    left = max(0, center_x - half_width)
    right = min(255, center_x + half_width)
    for y in range(224):
        row = y * 256 * 4
        for x in range(left, right + 1):
            offset = row + x * 4
            red, green, blue, alpha = frame[offset:offset + 4]
            if alpha == 255 and red > 150 and green > 110 and blue < 90:
                count += 1
    return count


def count_non_samus_pixel_diffs(left_frame: bytes, right_frame: bytes) -> int:
    def is_bright_samus_pixel(offset: int, frame: bytes) -> bool:
        red, green, blue, alpha = frame[offset:offset + 4]
        return alpha == 255 and red > 120 and green > 80 and blue < 110

    count = 0
    for offset in range(0, len(left_frame), 4):
        if left_frame[offset:offset + 4] == right_frame[offset:offset + 4]:
            continue
        if is_bright_samus_pixel(offset, left_frame) or is_bright_samus_pixel(offset, right_frame):
            continue
        count += 1
    return count


@pytest.fixture(scope="module")
def mini_browser_module():
    result = run(["make", "mini-browser-lib"])
    assert result.returncode == 0, f"make mini-browser-lib failed:\n{result.stderr}\n{result.stdout}"
    spec = importlib.util.spec_from_file_location("mini_browser_server", SERVER_PATH)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def test_kernel_snapshots_have_independent_player_follow_cameras(mini_browser_module):
    with mini_browser_module.MiniKernel.from_repo(SM_REV_DIR, build=False) as kernel:
        session = mini_browser_module.MiniBrowserSession(kernel)
        p1 = session.join(1)
        p2 = session.join(2)

        initial_p1 = session.state_for_token(p1["token"])[1]
        initial_p2 = session.state_for_token(p2["token"])[1]
        assert initial_p1["player"] == 1
        assert initial_p2["player"] == 2
        assert initial_p1["view"]["camera_x"] != initial_p2["view"]["camera_x"]
        assert abs(initial_p1["players"][0]["screen_x"] - initial_p2["players"][1]["screen_x"]) <= 2

        p1_start = initial_p1["players"][0]["world_x"]
        p2_start = initial_p2["players"][1]["world_x"]
        session.set_buttons(p1["token"], mini_browser_module.BUTTON_RIGHT)
        session.set_buttons(p2["token"], mini_browser_module.BUTTON_LEFT)
        for _ in range(30):
            session.step_once()

        moved_p1 = session.state_for_token(p1["token"])[1]
        moved_p2 = session.state_for_token(p2["token"])[1]
        assert moved_p1["players"][0]["world_x"] > p1_start
        assert moved_p2["players"][1]["world_x"] < p2_start
        assert moved_p1["view"]["camera_x"] != moved_p2["view"]["camera_x"]
        assert 90 <= moved_p1["players"][0]["screen_x"] <= 140
        assert 90 <= moved_p2["players"][1]["screen_x"] <= 140

        p1_status, p1_frame = session.frame_for_token(p1["token"])
        p2_status, p2_frame = session.frame_for_token(p2["token"])
        assert p1_status == 200
        assert p2_status == 200
        assert len(p1_frame) == mini_browser_module.GAME_WIDTH * mini_browser_module.GAME_HEIGHT * 4
        assert len(p2_frame) == mini_browser_module.GAME_WIDTH * mini_browser_module.GAME_HEIGHT * 4
        assert p1_frame != p2_frame
        unique_rgba = {p1_frame[i:i + 4] for i in range(0, len(p1_frame), 4)}
        assert len(unique_rgba) > 16
        assert all(p1_frame[i + 3] == 255 for i in range(0, len(p1_frame), 4096))

        fresh_kernel = mini_browser_module.MiniKernel.from_repo(SM_REV_DIR, build=False)
        try:
            fresh = mini_browser_module.MiniBrowserSession(fresh_kernel)
            fresh_p1 = fresh.join(1)
            fresh_p2 = fresh.join(2)
            fresh.step_once()
            fresh_p1_state = fresh.state_for_token(fresh_p1["token"])[1]
            fresh_p2_state = fresh.state_for_token(fresh_p2["token"])[1]
            fresh_p1_frame = fresh.frame_for_token(fresh_p1["token"])[1]
            fresh_p2_frame = fresh.frame_for_token(fresh_p2["token"])[1]
            assert count_non_samus_pixel_diffs(fresh_p1_frame, fresh_p2_frame) > 4000
            assert count_bright_samus_pixels_near(
                fresh_p1_frame, fresh_p1_state["players"][0]["screen_x"]
            ) > 20
            assert count_bright_samus_pixels_near(
                fresh_p1_frame, fresh_p1_state["players"][1]["screen_x"]
            ) > 20
            assert count_bright_samus_pixels_near(
                fresh_p2_frame, fresh_p2_state["players"][0]["screen_x"]
            ) > 20
            assert count_bright_samus_pixels_near(
                fresh_p2_frame, fresh_p2_state["players"][1]["screen_x"]
            ) > 20
        finally:
            fresh_kernel.close()


def test_http_join_input_and_state_routes_assign_p1_p2(mini_browser_module):
    with mini_browser_module.MiniKernel.from_repo(SM_REV_DIR, build=False) as kernel:
        session = mini_browser_module.MiniBrowserSession(kernel)
        server = ThreadingHTTPServer(
            ("127.0.0.1", 0),
            mini_browser_module.make_handler(session),
        )
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        base_url = f"http://127.0.0.1:{server.server_port}"
        try:
            page = urlopen(f"{base_url}/p1", timeout=5).read().decode("utf-8")
            assert "const playerHint = 1;" in page
            assert "/frame?token=" in page
            assert "resizeCanvasForIntegerScale" in page
            assert "lastInputSentAt" in page
            assert "grid-template-rows: auto auto" in page
            assert "drawRoom(" not in page

            p1 = json.loads(urlopen(f"{base_url}/join?player=1", timeout=5).read().decode("utf-8"))
            p2 = json.loads(urlopen(f"{base_url}/join?player=2", timeout=5).read().decode("utf-8"))
            assert p1["player"] == 1
            assert p2["player"] == 2
            assert p1["token"] != p2["token"]

            request = Request(
                f"{base_url}/input",
                data=json.dumps({
                    "token": p1["token"],
                    "buttons": mini_browser_module.BUTTON_RIGHT,
                }).encode("utf-8"),
                headers={"Content-Type": "application/json"},
                method="POST",
            )
            response = json.loads(urlopen(request, timeout=5).read().decode("utf-8"))
            assert response["ok"] is True
            for _ in range(8):
                session.step_once()

            p1_state = json.loads(
                urlopen(f"{base_url}/state?token={p1['token']}", timeout=5).read().decode("utf-8")
            )
            p2_state = json.loads(
                urlopen(f"{base_url}/state?token={p2['token']}", timeout=5).read().decode("utf-8")
            )
            assert p1_state["player"] == 1
            assert p2_state["player"] == 2
            assert p1_state["players"][0]["buttons"] == mini_browser_module.BUTTON_RIGHT
            assert p1_state["view"]["camera_x"] != p2_state["view"]["camera_x"]

            frame_response = urlopen(f"{base_url}/frame?token={p1['token']}", timeout=5)
            frame_body = frame_response.read()
            assert frame_response.headers["Content-Type"] == "application/octet-stream"
            assert len(frame_body) == mini_browser_module.GAME_WIDTH * mini_browser_module.GAME_HEIGHT * 4
        finally:
            server.shutdown()
            server.server_close()
            thread.join(timeout=2)


def test_browser_two_player_shooting_from_spawn_registers_hits(mini_browser_module):
    with mini_browser_module.MiniKernel.from_repo(SM_REV_DIR, build=False) as kernel:
        session = mini_browser_module.MiniBrowserSession(kernel)
        p1 = session.join(1)
        p2 = session.join(2)

        initial = session.state_for_token(p1["token"])[1]
        assert initial["players"][0]["pose"] != 0
        assert initial["players"][1]["pose"] != 0

        session.set_buttons(p1["token"], mini_browser_module.BUTTON_X)
        session.set_buttons(p2["token"], mini_browser_module.BUTTON_X)
        hit_event_queues = []
        projectile_owners = set()
        for _ in range(12):
            session.step_once()
            frame_state = session.state_for_token(p1["token"])[1]
            projectile_owners.update(
                projectile["owner"] for projectile in frame_state["projectiles"]
            )
            assert frame_state["hit_event_count"] == len(frame_state["hit_events"])
            if frame_state["hit_events"]:
                hit_event_queues.append(frame_state["hit_events"])

        state = session.state_for_token(p1["token"])[1]
        assert state["players"][0]["hit_count"] >= 1
        assert state["players"][1]["hit_count"] >= 1
        assert state["players"][0]["last_hit_by_player"] == 2
        assert state["players"][1]["last_hit_by_player"] == 1
        assert projectile_owners == {1, 2}
        assert hit_event_queues
        hit_events = [event for queue in hit_event_queues for event in queue]
        assert {(event["attacker"], event["defender"]) for event in hit_events} == {
            (1, 2),
            (2, 1),
        }
        assert all(event["attacker"] in (1, 2) for event in hit_events)
        assert all(event["defender"] in (1, 2) for event in hit_events)
        assert all(event["attacker"] != event["defender"] for event in hit_events)
        assert all(event["damage"] > 0 for event in hit_events)
        assert state["hit_event_dropped_count"] == 0


def test_projectile_tick_and_owner_counts_are_symmetric_between_seats(
    mini_browser_module,
):
    def hits_after_repeated_fire(shooter: int) -> tuple[int, int]:
        with mini_browser_module.MiniKernel.from_repo(
            SM_REV_DIR, build=False
        ) as kernel:
            for frame_index in range(360):
                shot = mini_browser_module.BUTTON_X if frame_index % 24 == 0 else 0
                kernel.step2(shot if shooter == 1 else 0, shot if shooter == 2 else 0)
            state = kernel.snapshot(1)
            return (state.players[0].hit_count, state.players[1].hit_count)

    assert hits_after_repeated_fire(1) == (0, 15)
    assert hits_after_repeated_fire(2) == (15, 0)


def test_players_fire_independent_vanilla_missiles_and_keep_impact_explosions(
    mini_browser_module,
):
    missile_flight = 0x8100
    missile_explosion = 0x8800

    with mini_browser_module.MiniKernel.from_repo(SM_REV_DIR, build=False) as kernel:
        for _ in range(30):
            kernel.step2(0, 0)
        kernel.step2(
            mini_browser_module.BUTTON_SELECT,
            mini_browser_module.BUTTON_SELECT,
        )
        selected = kernel.snapshot(1)
        assert [player.selected_weapon for player in selected.players] == [1, 1]
        assert [player.missiles for player in selected.players] == [3, 3]

        for _ in range(9):
            kernel.step2(0, 0)
        kernel.step2(mini_browser_module.BUTTON_X, mini_browser_module.BUTTON_X)
        fired = kernel.snapshot(1)
        assert [player.missiles for player in fired.players] == [2, 2]
        assert {
            (fired.projectiles[index].owner, fired.projectiles[index].type)
            for index in range(fired.projectile_count)
        } == {(1, missile_flight), (2, missile_flight)}

        hit_pairs = set()
        saw_both_explosions = False
        for _ in range(30):
            kernel.step2(0, 0)
            state = kernel.snapshot(1)
            hit_pairs.update(
                (state.hit_events[index].attacker, state.hit_events[index].defender)
                for index in range(state.hit_event_count)
            )
            explosion_owners = {
                state.projectiles[index].owner
                for index in range(state.projectile_count)
                if state.projectiles[index].type == missile_explosion
            }
            saw_both_explosions |= explosion_owners == {1, 2}

        assert hit_pairs == {(1, 2), (2, 1)}
        assert saw_both_explosions
        finished = kernel.snapshot(1)
        assert all(
            finished.projectiles[index].type != missile_explosion
            for index in range(finished.projectile_count)
        )


def test_multiplayer_jump_does_not_reveal_landing_site_placeholder_bands(
    mini_browser_module,
):
    placeholder_colors = {
        bytes((172, 123, 189, 255)),
        bytes((115, 65, 123, 255)),
        bytes((82, 41, 82, 255)),
    }

    with mini_browser_module.MiniKernel.from_repo(SM_REV_DIR, build=False) as kernel:
        for frame_index in range(100):
            buttons = mini_browser_module.BUTTON_A if 60 <= frame_index < 68 else 0
            kernel.step2(buttons, buttons)
            frame = kernel.render_rgba(1)
            assert not any(
                frame[offset:offset + 4] in placeholder_colors
                for offset in range(0, len(frame), 4)
            ), f"Landing Site placeholder band visible on jump frame {frame_index}"


def test_browser_input_sequence_and_timeout_prevent_stale_controls(mini_browser_module):
    now = [100.0]

    with mini_browser_module.MiniKernel.from_repo(SM_REV_DIR, build=False) as kernel:
        session = mini_browser_module.MiniBrowserSession(
            kernel,
            clock=lambda: now[0],
            input_timeout_seconds=0.25,
        )
        p1 = session.join(1)

        accepted = session.set_buttons(
            p1["token"], mini_browser_module.BUTTON_RIGHT, sequence=10
        )
        stale = session.set_buttons(
            p1["token"], mini_browser_module.BUTTON_LEFT, sequence=9
        )
        assert accepted["accepted"] is True
        assert stale["accepted"] is False
        assert stale["buttons"] == mini_browser_module.BUTTON_RIGHT

        session.step_once()
        moving = session.state_for_token(p1["token"])[1]
        assert moving["players"][0]["buttons"] == mini_browser_module.BUTTON_RIGHT

        now[0] += 0.30
        session.step_once()
        released = session.state_for_token(p1["token"])[1]
        assert released["players"][0]["buttons"] == 0
