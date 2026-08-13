#!/usr/bin/env python3
"""Record a deterministic, two-sided mini multiplayer showcase."""

from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path

import mini_browser_server as mini


REPO_ROOT = Path(__file__).resolve().parents[1]
VIDEO_WIDTH = mini.GAME_WIDTH
GAME_TOP = 16
VIDEO_HEIGHT = mini.GAME_HEIGHT + GAME_TOP
DEFAULT_FRAMES = 600

PANEL = (7, 11, 17, 255)
WHITE = (238, 245, 252, 255)
MUTED = (137, 157, 179, 255)
BLACK = (0, 0, 0, 255)
P1_COLOR = (57, 213, 255, 255)
P2_COLOR = (255, 82, 166, 255)
HIT_COLOR = (255, 239, 92, 255)

GLYPHS = {
    " ": (0, 0, 0, 0, 0),
    "-": (0, 0, 7, 0, 0),
    ":": (0, 2, 0, 2, 0),
    "0": (7, 5, 5, 5, 7),
    "1": (2, 6, 2, 2, 7),
    "2": (7, 1, 7, 4, 7),
    "3": (7, 1, 7, 1, 7),
    "4": (5, 5, 7, 1, 1),
    "5": (7, 4, 7, 1, 7),
    "6": (7, 4, 7, 5, 7),
    "7": (7, 1, 1, 1, 1),
    "8": (7, 5, 7, 5, 7),
    "9": (7, 5, 7, 1, 7),
    "A": (2, 5, 7, 5, 5),
    "B": (6, 5, 6, 5, 6),
    "C": (3, 4, 4, 4, 3),
    "D": (6, 5, 5, 5, 6),
    "E": (7, 4, 6, 4, 7),
    "F": (7, 4, 6, 4, 4),
    "G": (3, 4, 5, 5, 3),
    "H": (5, 5, 7, 5, 5),
    "I": (7, 2, 2, 2, 7),
    "J": (1, 1, 1, 5, 2),
    "K": (5, 5, 6, 5, 5),
    "L": (4, 4, 4, 4, 7),
    "M": (5, 7, 7, 5, 5),
    "N": (5, 7, 7, 7, 5),
    "O": (2, 5, 5, 5, 2),
    "P": (6, 5, 6, 4, 4),
    "Q": (2, 5, 5, 3, 1),
    "R": (6, 5, 6, 5, 5),
    "S": (3, 4, 2, 1, 6),
    "T": (7, 2, 2, 2, 2),
    "U": (5, 5, 5, 5, 7),
    "V": (5, 5, 5, 5, 2),
    "W": (5, 5, 7, 7, 5),
    "X": (5, 5, 2, 5, 5),
    "Y": (5, 5, 2, 2, 2),
    "Z": (7, 1, 2, 4, 7),
}


def fill_rect(
    pixels: bytearray,
    left: int,
    top: int,
    width: int,
    height: int,
    color: tuple[int, int, int, int],
) -> None:
    left = max(0, left)
    top = max(0, top)
    right = min(VIDEO_WIDTH, left + max(0, width))
    bottom = min(VIDEO_HEIGHT, top + max(0, height))
    row = bytes(color) * max(0, right - left)
    for y in range(top, bottom):
        start = (y * VIDEO_WIDTH + left) * 4
        pixels[start : start + len(row)] = row


def draw_text(
    pixels: bytearray,
    x: int,
    y: int,
    text: str,
    color: tuple[int, int, int, int],
    scale: int = 1,
) -> None:
    cursor = x
    for char in text.upper():
        rows = GLYPHS.get(char, GLYPHS[" "])
        for row_index, bits in enumerate(rows):
            for column in range(3):
                if bits & (1 << (2 - column)):
                    fill_rect(
                        pixels,
                        cursor + column * scale,
                        y + row_index * scale,
                        scale,
                        scale,
                        color,
                    )
        cursor += 4 * scale


def draw_outline(
    pixels: bytearray,
    left: int,
    top: int,
    width: int,
    height: int,
    color: tuple[int, int, int, int],
) -> None:
    fill_rect(pixels, left, top, width, 1, color)
    fill_rect(pixels, left, top + height - 1, width, 1, color)
    fill_rect(pixels, left, top, 1, height, color)
    fill_rect(pixels, left + width - 1, top, 1, height, color)


def demo_buttons(frame: int) -> tuple[int, int]:
    """Choreograph a compact beam-and-missile bout in one camera."""
    player1 = 0
    player2 = 0

    if 60 <= frame < 72:
        player1 |= mini.BUTTON_LEFT
        player2 |= mini.BUTTON_RIGHT
    elif 238 <= frame < 250 or 330 <= frame < 336:
        player1 |= mini.BUTTON_RIGHT
        player2 |= mini.BUTTON_LEFT

    if 205 <= frame < 213 or 410 <= frame < 418:
        player1 |= mini.BUTTON_A
        player2 |= mini.BUTTON_A

    if frame in (84, 104, 124, 276, 290, 304, 348, 362, 376, 466, 480, 494):
        player1 |= mini.BUTTON_X
        player2 |= mini.BUTTON_X

    if frame in (148, 520):
        player1 |= mini.BUTTON_SELECT
        player2 |= mini.BUTTON_SELECT
    if frame in (166, 538):
        player1 |= mini.BUTTON_X
        player2 |= mini.BUTTON_X
    if frame in (232, 570):
        player1 |= mini.BUTTON_SELECT
        player2 |= mini.BUTTON_SELECT

    return player1, player2


def copy_game_frame(canvas: bytearray, frame: bytes) -> None:
    row_bytes = mini.GAME_WIDTH * 4
    for y in range(mini.GAME_HEIGHT):
        source = y * row_bytes
        target = ((y + GAME_TOP) * VIDEO_WIDTH) * 4
        canvas[target : target + row_bytes] = frame[source : source + row_bytes]


def draw_player_marker(
    canvas: bytearray,
    player: mini.MiniNetPlayerSnapshot,
    label: str,
    color: tuple[int, int, int, int],
) -> None:
    center_x = int(player.screen_x) + int(player.x_radius)
    label_x = center_x - 4
    label_y = max(GAME_TOP + 1, GAME_TOP + int(player.screen_y) - 8)
    draw_text(canvas, label_x + 1, label_y + 1, label, BLACK)
    draw_text(canvas, label_x, label_y, label, color)
    fill_rect(canvas, center_x - 2, label_y + 6, 5, 1, color)


def draw_hud(
    canvas: bytearray,
    snapshot: mini.MiniNetSnapshot,
    damage: list[int],
) -> None:
    fill_rect(canvas, 0, 0, VIDEO_WIDTH, GAME_TOP, PANEL)
    draw_text(
        canvas,
        3,
        2,
        f"P1 DMG {damage[0]:03d} M{int(snapshot.players[0].missiles)}",
        P1_COLOR,
    )
    draw_text(canvas, 3, 9, f"HITS TAKEN {int(snapshot.players[0].hit_count):02d}", WHITE)
    draw_text(
        canvas,
        157,
        2,
        f"P2 DMG {damage[1]:03d} M{int(snapshot.players[1].missiles)}",
        P2_COLOR,
    )
    draw_text(canvas, 157, 9, f"HITS TAKEN {int(snapshot.players[1].hit_count):02d}", WHITE)
    draw_text(canvas, 118, 2, "VS", MUTED)
    draw_text(canvas, 109, 9, f"F{int(snapshot.frame):04d}", MUTED)

    draw_player_marker(canvas, snapshot.players[0], "P1", P1_COLOR)
    draw_player_marker(canvas, snapshot.players[1], "P2", P2_COLOR)

    for index, player in enumerate(snapshot.players):
        if int(player.hitstun_frames) == 0:
            continue
        left = int(player.screen_x) - 2
        top = GAME_TOP + int(player.screen_y) - 2
        width = int(player.x_radius) * 2 + 5
        height = int(player.y_radius) * 2 + 5
        draw_outline(canvas, left, top, width, height, HIT_COLOR)


def draw_slate(canvas: bytearray, frame: int, frame_count: int) -> None:
    if frame < 48:
        fill_rect(canvas, 28, 82, 200, 48, PANEL)
        draw_text(canvas, 48, 91, "MINI MULTIPLAYER", WHITE, scale=2)
        draw_text(canvas, 55, 113, "BEAMS MISSILES IMPACTS", MUTED)
    elif frame >= frame_count - 54:
        fill_rect(canvas, 32, 86, 192, 42, PANEL)
        draw_text(canvas, 42, 94, "BOTH PLAYERS ARMED", WHITE, scale=2)
        draw_text(canvas, 58, 116, "DEATH AND STOCKS ARE NEXT", MUTED)


def start_ffmpeg(output: Path) -> subprocess.Popen[bytes]:
    ffmpeg = shutil.which("ffmpeg")
    if ffmpeg is None:
        raise RuntimeError("ffmpeg is required to record the multiplayer demo")
    output.parent.mkdir(parents=True, exist_ok=True)
    command = [
        ffmpeg,
        "-loglevel",
        "error",
        "-y",
        "-f",
        "rawvideo",
        "-pixel_format",
        "rgba",
        "-video_size",
        f"{VIDEO_WIDTH}x{VIDEO_HEIGHT}",
        "-framerate",
        "60",
        "-i",
        "-",
        "-vf",
        "scale=768:720:flags=neighbor",
        "-an",
        "-c:v",
        "libx264",
        "-preset",
        "veryfast",
        "-crf",
        "22",
        "-pix_fmt",
        "yuv420p",
        "-movflags",
        "+faststart",
        str(output),
    ]
    return subprocess.Popen(command, stdin=subprocess.PIPE)


def record_demo(output: Path, frame_count: int, build: bool) -> dict[str, object]:
    if build:
        subprocess.run(["make", "mini-browser-lib"], cwd=REPO_ROOT, check=True)

    damage = [0, 0]
    hit_pairs: set[tuple[int, int]] = set()
    projectile_types: set[int] = set()
    missile_explosion_seen = False
    ffmpeg = start_ffmpeg(output)
    assert ffmpeg.stdin is not None
    final_snapshot: mini.MiniNetSnapshot | None = None
    try:
        with mini.MiniKernel.from_repo(REPO_ROOT, build=False) as kernel:
            for frame_index in range(frame_count):
                player1, player2 = demo_buttons(frame_index)
                kernel.step2(player1, player2)
                snapshot = kernel.snapshot(1)
                for projectile_index in range(int(snapshot.projectile_count)):
                    projectile_type = int(snapshot.projectiles[projectile_index].type)
                    projectile_types.add(projectile_type)
                    missile_explosion_seen |= (
                        projectile_type & 0x0F00
                    ) == 0x0800
                for event_index in range(int(snapshot.hit_event_count)):
                    event = snapshot.hit_events[event_index]
                    defender_index = int(event.defender) - 1
                    if 0 <= defender_index < len(damage):
                        damage[defender_index] += int(event.damage)
                    hit_pairs.add((int(event.attacker), int(event.defender)))

                canvas = bytearray(VIDEO_WIDTH * VIDEO_HEIGHT * 4)
                copy_game_frame(canvas, kernel.render_rgba(1))
                draw_hud(canvas, snapshot, damage)
                draw_slate(canvas, frame_index, frame_count)
                ffmpeg.stdin.write(canvas)
                final_snapshot = snapshot
    finally:
        ffmpeg.stdin.close()
        return_code = ffmpeg.wait()
    if return_code != 0:
        raise RuntimeError(f"ffmpeg exited with status {return_code}")
    if final_snapshot is None:
        raise RuntimeError("no demo frames were recorded")

    return {
        "output": str(output.resolve()),
        "frames": frame_count,
        "seconds": frame_count / 60.0,
        "state_hash": f"0x{int(final_snapshot.state_hash):016x}",
        "player1_hits_taken": int(final_snapshot.players[0].hit_count),
        "player2_hits_taken": int(final_snapshot.players[1].hit_count),
        "player1_damage_taken": damage[0],
        "player2_damage_taken": damage[1],
        "reciprocal_hits": hit_pairs == {(1, 2), (2, 1)},
        "missile_flight_seen": any(
            (projectile_type & 0x0F00) == 0x0100
            for projectile_type in projectile_types
        ),
        "missile_explosion_seen": missile_explosion_seen,
        "dropped_hit_events": int(final_snapshot.hit_event_dropped_count),
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output",
        type=Path,
        default=REPO_ROOT / "out" / "mini_multiplayer_demo.mp4",
        help="destination MP4 path",
    )
    parser.add_argument("--frames", type=int, default=DEFAULT_FRAMES)
    parser.add_argument("--no-build", action="store_true")
    args = parser.parse_args()
    if args.frames < 120:
        parser.error("--frames must be at least 120")
    return args


def main() -> int:
    args = parse_args()
    try:
        result = record_demo(args.output, args.frames, build=not args.no_build)
    except (OSError, RuntimeError, subprocess.CalledProcessError) as exc:
        print(f"record multiplayer demo: {exc}", file=sys.stderr)
        return 1
    print(json.dumps(result, sort_keys=True))
    return 0 if (
        result["reciprocal_hits"]
        and result["missile_flight_seen"]
        and result["missile_explosion_seen"]
        and result["dropped_hit_events"] == 0
    ) else 1


if __name__ == "__main__":
    raise SystemExit(main())
