#!/usr/bin/env python3
from __future__ import annotations

import argparse
import ctypes
import json
import subprocess
import sys
import threading
import time
import uuid
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Any
from urllib.parse import parse_qs, urlparse


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 8765
GAME_WIDTH = 256
GAME_HEIGHT = 224
MAX_PLAYERS = 2
MAX_PROJECTILES = 5

BUTTON_R = 0x10
BUTTON_L = 0x20
BUTTON_X = 0x40
BUTTON_A = 0x80
BUTTON_RIGHT = 0x100
BUTTON_LEFT = 0x200
BUTTON_DOWN = 0x400
BUTTON_UP = 0x800
BUTTON_START = 0x1000
BUTTON_SELECT = 0x2000
BUTTON_Y = 0x4000
BUTTON_B = 0x8000


class MiniNetPlayerSnapshot(ctypes.Structure):
    _fields_ = [
        ("world_x", ctypes.c_int32),
        ("world_y", ctypes.c_int32),
        ("screen_x", ctypes.c_int32),
        ("screen_y", ctypes.c_int32),
        ("x_velocity", ctypes.c_int32),
        ("y_velocity", ctypes.c_int32),
        ("x_radius", ctypes.c_uint16),
        ("y_radius", ctypes.c_uint16),
        ("pose", ctypes.c_uint16),
        ("movement_type", ctypes.c_uint16),
        ("buttons", ctypes.c_uint16),
        ("hit_count", ctypes.c_uint16),
        ("pending_damage", ctypes.c_uint16),
        ("hitstun_frames", ctypes.c_uint16),
        ("invulnerable_frames", ctypes.c_uint16),
        ("suit", ctypes.c_uint8),
        ("on_ground", ctypes.c_uint8),
        ("last_hit_by_player", ctypes.c_uint8),
        ("active", ctypes.c_uint8),
    ]


class MiniNetProjectileSnapshot(ctypes.Structure):
    _fields_ = [
        ("screen_x", ctypes.c_int32),
        ("screen_y", ctypes.c_int32),
        ("slot_index", ctypes.c_uint16),
        ("type", ctypes.c_uint16),
        ("direction", ctypes.c_uint16),
        ("x_pos", ctypes.c_uint16),
        ("y_pos", ctypes.c_uint16),
        ("x_radius", ctypes.c_uint16),
        ("y_radius", ctypes.c_uint16),
        ("damage", ctypes.c_uint16),
        ("active", ctypes.c_uint8),
        ("owner", ctypes.c_uint8),
    ]


class MiniNetSnapshot(ctypes.Structure):
    _fields_ = [
        ("frame", ctypes.c_uint32),
        ("player_count", ctypes.c_int32),
        ("focus_player", ctypes.c_int32),
        ("viewport_width", ctypes.c_int32),
        ("viewport_height", ctypes.c_int32),
        ("camera_x", ctypes.c_int32),
        ("camera_y", ctypes.c_int32),
        ("room_left", ctypes.c_int32),
        ("room_top", ctypes.c_int32),
        ("room_right", ctypes.c_int32),
        ("room_bottom", ctypes.c_int32),
        ("room_width", ctypes.c_int32),
        ("room_height", ctypes.c_int32),
        ("projectile_count", ctypes.c_int32),
        ("state_hash", ctypes.c_uint64),
        ("players", MiniNetPlayerSnapshot * MAX_PLAYERS),
        ("projectiles", MiniNetProjectileSnapshot * MAX_PROJECTILES),
    ]


def find_library(repo_root: Path) -> Path | None:
    for name in ("libsm_rev_mini_net.so", "libsm_rev_mini_net.dylib"):
        candidate = repo_root / name
        if candidate.exists():
            return candidate
    return None


def ensure_library(repo_root: Path, build: bool) -> Path:
    library = find_library(repo_root)
    if library is not None:
        return library
    if not build:
        raise FileNotFoundError(
            f"mini browser library is missing under {repo_root}; run `make mini-browser-lib`"
        )
    subprocess.run(["make", "mini-browser-lib"], cwd=repo_root, check=True)
    library = find_library(repo_root)
    if library is None:
        raise FileNotFoundError("make mini-browser-lib finished but no mini browser library was found")
    return library


class MiniKernel:
    def __init__(self, library_path: Path, player_count: int = MAX_PLAYERS):
        self._library_path = library_path
        self._lib = ctypes.CDLL(str(library_path))
        self._lib.MiniNetCreate.argtypes = [ctypes.c_int32, ctypes.c_int32, ctypes.c_int32]
        self._lib.MiniNetCreate.restype = ctypes.c_void_p
        self._lib.MiniNetDestroy.argtypes = [ctypes.c_void_p]
        self._lib.MiniNetDestroy.restype = None
        self._lib.MiniNetSetPlayerCount.argtypes = [ctypes.c_void_p, ctypes.c_int32]
        self._lib.MiniNetSetPlayerCount.restype = None
        self._lib.MiniNetStep2.argtypes = [
            ctypes.c_void_p,
            ctypes.c_uint16,
            ctypes.c_uint16,
            ctypes.c_uint8,
        ]
        self._lib.MiniNetStep2.restype = None
        self._lib.MiniNetReadSnapshot.argtypes = [
            ctypes.c_void_p,
            ctypes.c_int32,
            ctypes.POINTER(MiniNetSnapshot),
        ]
        self._lib.MiniNetReadSnapshot.restype = ctypes.c_uint8
        self._lib.MiniNetRenderFrame.argtypes = [
            ctypes.c_void_p,
            ctypes.c_int32,
            ctypes.POINTER(ctypes.c_uint32),
            ctypes.c_int32,
        ]
        self._lib.MiniNetRenderFrame.restype = ctypes.c_uint8
        self._lib.MiniNetRenderFrameRgba.argtypes = [
            ctypes.c_void_p,
            ctypes.c_int32,
            ctypes.POINTER(ctypes.c_uint8),
            ctypes.c_int32,
        ]
        self._lib.MiniNetRenderFrameRgba.restype = ctypes.c_uint8
        self._state = self._lib.MiniNetCreate(player_count, GAME_WIDTH, GAME_HEIGHT)
        if not self._state:
            raise RuntimeError("MiniNetCreate failed")
        self._closed = False

    @classmethod
    def from_repo(cls, repo_root: Path = REPO_ROOT, build: bool = True) -> "MiniKernel":
        return cls(ensure_library(repo_root, build))

    def close(self) -> None:
        if not self._closed:
            self._lib.MiniNetDestroy(self._state)
            self._closed = True

    def set_player_count(self, player_count: int) -> None:
        self._lib.MiniNetSetPlayerCount(self._state, player_count)

    def step2(self, player1_buttons: int, player2_buttons: int, quit_requested: bool = False) -> None:
        self._lib.MiniNetStep2(
            self._state,
            ctypes.c_uint16(player1_buttons & 0xFFFF),
            ctypes.c_uint16(player2_buttons & 0xFFFF),
            ctypes.c_uint8(1 if quit_requested else 0),
        )

    def snapshot(self, focus_player: int) -> MiniNetSnapshot:
        snapshot = MiniNetSnapshot()
        ok = self._lib.MiniNetReadSnapshot(self._state, focus_player, ctypes.byref(snapshot))
        if not ok:
            raise RuntimeError("MiniNetReadSnapshot failed")
        return snapshot

    def render_rgba(self, focus_player: int) -> bytes:
        byte_count = GAME_WIDTH * GAME_HEIGHT * 4
        rgba = (ctypes.c_uint8 * byte_count)()
        ok = self._lib.MiniNetRenderFrameRgba(self._state, focus_player, rgba, GAME_WIDTH * 4)
        if not ok:
            raise RuntimeError("MiniNetRenderFrameRgba failed")
        return bytes(rgba)

    def __enter__(self) -> "MiniKernel":
        return self

    def __exit__(self, exc_type: object, exc: object, tb: object) -> None:
        self.close()


class MiniBrowserSession:
    def __init__(self, kernel: MiniKernel):
        self._kernel = kernel
        self._lock = threading.RLock()
        self._buttons = [0, 0]
        self._tokens: dict[str, dict[str, Any]] = {}
        self._join_counts = {1: 0, 2: 0}

    def close(self) -> None:
        with self._lock:
            self._kernel.close()

    def join(self, player_hint: int | None, name: str = "") -> dict[str, Any]:
        with self._lock:
            player = self._normalize_player(player_hint) if player_hint else self._least_joined_player()
            token = uuid.uuid4().hex
            self._tokens[token] = {
                "player": player,
                "name": name[:32],
                "last_seen": time.time(),
            }
            self._join_counts[player] += 1
            return {"ok": True, "token": token, "player": player}

    def set_buttons(self, token: str, buttons: int) -> dict[str, Any]:
        with self._lock:
            session = self._tokens.get(token)
            if session is None:
                return {"ok": False, "error": "unknown token"}
            player = int(session["player"])
            session["last_seen"] = time.time()
            self._buttons[player - 1] = buttons & 0xFFFF
            return {"ok": True, "player": player, "buttons": self._buttons[player - 1]}

    def step_once(self) -> None:
        with self._lock:
            self._kernel.step2(self._buttons[0], self._buttons[1])

    def state_for_token(self, token: str | None, player_hint: int | None = None) -> tuple[int, dict[str, Any]]:
        with self._lock:
            player = self._player_for_request(token, player_hint)
            if player is None:
                return HTTPStatus.FORBIDDEN, {"ok": False, "error": "join first"}
            return HTTPStatus.OK, self._snapshot_dict(player)

    def frame_for_token(self, token: str | None, player_hint: int | None = None) -> tuple[int, bytes]:
        with self._lock:
            player = self._player_for_request(token, player_hint)
            if player is None:
                return HTTPStatus.FORBIDDEN, b"join first"
            return HTTPStatus.OK, self._kernel.render_rgba(player)

    def _snapshot_dict(self, focus_player: int) -> dict[str, Any]:
        snapshot = self._kernel.snapshot(focus_player)
        players = []
        for index, player in enumerate(snapshot.players):
            players.append(
                {
                    "id": index + 1,
                    "active": bool(player.active),
                    "world_x": int(player.world_x),
                    "world_y": int(player.world_y),
                    "screen_x": int(player.screen_x),
                    "screen_y": int(player.screen_y),
                    "x_velocity": int(player.x_velocity),
                    "y_velocity": int(player.y_velocity),
                    "x_radius": int(player.x_radius),
                    "y_radius": int(player.y_radius),
                    "pose": int(player.pose),
                    "movement_type": int(player.movement_type),
                    "buttons": int(player.buttons),
                    "hit_count": int(player.hit_count),
                    "pending_damage": int(player.pending_damage),
                    "hitstun_frames": int(player.hitstun_frames),
                    "invulnerable_frames": int(player.invulnerable_frames),
                    "last_hit_by_player": int(player.last_hit_by_player),
                    "on_ground": bool(player.on_ground),
                    "suit": int(player.suit),
                }
            )

        projectiles = []
        for index in range(int(snapshot.projectile_count)):
            projectile = snapshot.projectiles[index]
            projectiles.append(
                {
                    "slot_index": int(projectile.slot_index),
                    "active": bool(projectile.active),
                    "owner": int(projectile.owner),
                    "type": int(projectile.type),
                    "direction": int(projectile.direction),
                    "world_x": int(projectile.x_pos),
                    "world_y": int(projectile.y_pos),
                    "screen_x": int(projectile.screen_x),
                    "screen_y": int(projectile.screen_y),
                    "x_radius": int(projectile.x_radius),
                    "y_radius": int(projectile.y_radius),
                    "damage": int(projectile.damage),
                }
            )

        return {
            "ok": True,
            "frame": int(snapshot.frame),
            "player": int(snapshot.focus_player),
            "player_count": int(snapshot.player_count),
            "state_hash": f"0x{int(snapshot.state_hash):016x}",
            "view": {
                "width": int(snapshot.viewport_width),
                "height": int(snapshot.viewport_height),
                "camera_x": int(snapshot.camera_x),
                "camera_y": int(snapshot.camera_y),
            },
            "room": {
                "left": int(snapshot.room_left),
                "top": int(snapshot.room_top),
                "right": int(snapshot.room_right),
                "bottom": int(snapshot.room_bottom),
                "width": int(snapshot.room_width),
                "height": int(snapshot.room_height),
            },
            "players": players,
            "projectiles": projectiles,
        }

    def _least_joined_player(self) -> int:
        return 1 if self._join_counts[1] <= self._join_counts[2] else 2

    def _player_for_request(self, token: str | None, player_hint: int | None) -> int | None:
        if token and token in self._tokens:
            session = self._tokens[token]
            session["last_seen"] = time.time()
            return int(session["player"])
        if player_hint is not None:
            return self._normalize_player(player_hint)
        return None

    @staticmethod
    def _normalize_player(player: int | None) -> int:
        return 2 if player == 2 else 1


class MiniFrameRunner:
    def __init__(self, session: MiniBrowserSession, fps: int):
        self._session = session
        self._interval = 1.0 / fps
        self._stop = threading.Event()
        self._thread = threading.Thread(target=self._run, name="mini-frame-runner", daemon=True)

    def start(self) -> None:
        self._thread.start()

    def stop(self) -> None:
        self._stop.set()
        self._thread.join(timeout=2.0)

    def _run(self) -> None:
        while not self._stop.is_set():
            started = time.monotonic()
            self._session.step_once()
            elapsed = time.monotonic() - started
            self._stop.wait(max(0.0, self._interval - elapsed))


def parse_player_hint(query: dict[str, list[str]]) -> int | None:
    values = query.get("player")
    if not values:
        return None
    try:
        return 2 if int(values[0]) == 2 else 1
    except ValueError:
        return None


def render_index(player_hint: int | None) -> bytes:
    player_hint_json = "null" if player_hint is None else str(player_hint)
    html = f"""<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>sm_rev mini netplay</title>
<style>
:root {{
  color-scheme: dark;
  --panel: #141920;
  --line: #2c3642;
  --text: #e8eef5;
  --muted: #91a3b8;
  --screen-width: {GAME_WIDTH * 3}px;
}}
* {{ box-sizing: border-box; }}
body {{
  margin: 0;
  min-height: 100vh;
  display: grid;
  place-items: center;
  background: #080b10;
  color: var(--text);
  font: 14px/1.4 system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
}}
main {{
  width: 100vw;
  min-height: 100vh;
  padding: 12px;
  display: grid;
  place-items: center;
}}
.shell {{
  display: grid;
  grid-template-rows: auto auto;
  justify-items: center;
  gap: 8px;
  max-width: 100%;
}}
canvas {{
  box-sizing: content-box;
  display: block;
  width: {GAME_WIDTH * 3}px;
  height: {GAME_HEIGHT * 3}px;
  max-width: 100%;
  image-rendering: pixelated;
  background: #05070a;
  border: 1px solid var(--line);
}}
.side {{
  width: min(100%, var(--screen-width));
  display: flex;
  align-items: center;
  justify-content: center;
  flex-wrap: wrap;
  gap: 6px 14px;
  border: 1px solid var(--line);
  background: var(--panel);
  border-radius: 6px;
  padding: 6px 8px;
  opacity: 0.78;
  font-size: 12px;
}}
.row {{
  display: flex;
  justify-content: center;
  gap: 5px;
  padding: 0;
  border-bottom: 0;
}}
.label {{ color: var(--muted); }}
.badge {{
  display: inline-flex;
  align-items: center;
  min-width: 30px;
  justify-content: center;
  border-radius: 4px;
  padding: 2px 7px;
  background: #213247;
  color: #f3f8ff;
  font-weight: 700;
}}
@media (max-width: 820px) {{
  main {{ padding: 8px; }}
  .side {{ justify-content: flex-start; }}
}}
</style>
</head>
<body>
<main>
  <div class="shell">
    <canvas id="screen" width="256" height="224" aria-label="sm_rev mini browser viewport"></canvas>
    <aside class="side" aria-live="polite">
      <div class="row"><span class="label">Player</span><span id="player" class="badge">...</span></div>
      <div class="row"><span class="label">Frame</span><span id="frame">0</span></div>
      <div class="row"><span class="label">Camera</span><span id="camera">0,0</span></div>
      <div class="row"><span class="label">P1</span><span id="p1">0,0</span></div>
      <div class="row"><span class="label">P2</span><span id="p2">0,0</span></div>
      <div class="row"><span class="label">Hash</span><span id="hash">...</span></div>
    </aside>
  </div>
</main>
<script>
const playerHint = {player_hint_json};
const buttonMap = new Map([
  ["ArrowRight", {BUTTON_RIGHT}],
  ["ArrowLeft", {BUTTON_LEFT}],
  ["ArrowDown", {BUTTON_DOWN}],
  ["ArrowUp", {BUTTON_UP}],
  ["KeyS", {BUTTON_X}],
  ["KeyX", {BUTTON_A}],
  ["KeyZ", {BUTTON_B}],
  ["KeyA", {BUTTON_Y}],
  ["KeyC", {BUTTON_L}],
  ["KeyV", {BUTTON_R}],
  ["Enter", {BUTTON_START}],
  ["ShiftLeft", {BUTTON_SELECT}],
  ["ShiftRight", {BUTTON_SELECT}]
]);
const canvas = document.getElementById("screen");
const ctx = canvas.getContext("2d");
ctx.imageSmoothingEnabled = false;
const side = document.querySelector(".side");
const held = new Set();
let token = null;
let assignedPlayer = playerHint || 0;
let latestState = null;
let lastSentButtons = -1;
const frameImage = ctx.createImageData({GAME_WIDTH}, {GAME_HEIGHT});

function resizeCanvasForIntegerScale() {{
  const sideHeight = side ? side.getBoundingClientRect().height : 0;
  const maxScaleX = Math.floor((window.innerWidth - 32) / {GAME_WIDTH});
  const maxScaleY = Math.floor((window.innerHeight - sideHeight - 28) / {GAME_HEIGHT});
  const scale = Math.max(1, Math.min(4, maxScaleX, maxScaleY));
  const width = {GAME_WIDTH} * scale;
  const height = {GAME_HEIGHT} * scale;
  canvas.style.width = `${{width}}px`;
  canvas.style.height = `${{height}}px`;
  document.documentElement.style.setProperty("--screen-width", `${{width}}px`);
}}

function storageKey() {{
  return `sm_rev_mini_token_${{playerHint || "auto"}}`;
}}

function currentButtons() {{
  let buttons = 0;
  for (const code of held) {{
    buttons |= buttonMap.get(code) || 0;
  }}
  return buttons;
}}

async function join() {{
  const qs = playerHint ? `?player=${{playerHint}}` : "";
  const response = await fetch(`/join${{qs}}`);
  const payload = await response.json();
  if (!payload.ok) throw new Error(payload.error || "join failed");
  token = payload.token;
  assignedPlayer = payload.player;
  localStorage.setItem(storageKey(), token);
  document.getElementById("player").textContent = `P${{assignedPlayer}}`;
}}

async function sendInput(force = false) {{
  if (!token) return;
  const buttons = currentButtons();
  if (!force && buttons === lastSentButtons) return;
  lastSentButtons = buttons;
  await fetch("/input", {{
    method: "POST",
    headers: {{ "Content-Type": "application/json" }},
    body: JSON.stringify({{ token, buttons }})
  }});
}}

async function pollState() {{
  while (true) {{
    try {{
      if (!token) await join();
      const response = await fetch(`/state?token=${{encodeURIComponent(token)}}`);
      if (response.status === 403) {{
        localStorage.removeItem(storageKey());
        token = null;
        continue;
      }}
      latestState = await response.json();
      updateHud(latestState);
    }} catch (error) {{
      latestState = null;
    }}
    await new Promise(resolve => setTimeout(resolve, 16));
  }}
}}

function updateHud(state) {{
  document.getElementById("player").textContent = `P${{state.player}}`;
  document.getElementById("frame").textContent = state.frame;
  document.getElementById("camera").textContent = `${{state.view.camera_x}},${{state.view.camera_y}}`;
  document.getElementById("p1").textContent = `${{state.players[0].world_x}},${{state.players[0].world_y}}`;
  document.getElementById("p2").textContent = `${{state.players[1].world_x}},${{state.players[1].world_y}}`;
  document.getElementById("hash").textContent = state.state_hash.slice(0, 10);
}}

async function renderFrames() {{
  while (true) {{
    try {{
      if (!token) await join();
      const response = await fetch(`/frame?token=${{encodeURIComponent(token)}}`, {{ cache: "no-store" }});
      if (response.status === 403) {{
        localStorage.removeItem(storageKey());
        token = null;
        continue;
      }}
      const rgba = new Uint8ClampedArray(await response.arrayBuffer());
      if (rgba.byteLength === frameImage.data.byteLength) {{
        frameImage.data.set(rgba);
        ctx.putImageData(frameImage, 0, 0);
      }}
    }} catch (error) {{
      ctx.fillStyle = "#05070a";
      ctx.fillRect(0, 0, canvas.width, canvas.height);
    }}
    await new Promise(resolve => requestAnimationFrame(resolve));
  }}
}}

window.addEventListener("keydown", event => {{
  if (!buttonMap.has(event.code)) return;
  held.add(event.code);
  event.preventDefault();
  sendInput();
}});
window.addEventListener("keyup", event => {{
  if (!buttonMap.has(event.code)) return;
  held.delete(event.code);
  event.preventDefault();
  sendInput(true);
}});
window.addEventListener("blur", () => {{
  held.clear();
  sendInput(true);
}});
window.addEventListener("resize", resizeCanvasForIntegerScale);

token = localStorage.getItem(storageKey());
resizeCanvasForIntegerScale();
pollState();
setInterval(() => sendInput(), 33);
renderFrames();
</script>
</body>
</html>
"""
    return html.encode("utf-8")


def make_handler(session: MiniBrowserSession) -> type[BaseHTTPRequestHandler]:
    class MiniBrowserHandler(BaseHTTPRequestHandler):
        server_version = "sm_rev_mini_browser/0.1"

        def log_message(self, format: str, *args: object) -> None:
            sys.stderr.write("%s - - [%s] %s\n" % (self.address_string(), self.log_date_time_string(), format % args))

        def do_GET(self) -> None:
            parsed = urlparse(self.path)
            query = parse_qs(parsed.query)
            if parsed.path in ("/", "/p1", "/p2"):
                hint = 1 if parsed.path == "/p1" else 2 if parsed.path == "/p2" else None
                self._send_bytes(HTTPStatus.OK, render_index(hint), "text/html; charset=utf-8")
                return
            if parsed.path == "/join":
                payload = session.join(parse_player_hint(query), query.get("name", [""])[0])
                self._send_json(HTTPStatus.OK, payload)
                return
            if parsed.path == "/state":
                token = query.get("token", [None])[0]
                status, payload = session.state_for_token(token, parse_player_hint(query))
                self._send_json(status, payload)
                return
            if parsed.path == "/frame":
                token = query.get("token", [None])[0]
                status, body = session.frame_for_token(token, parse_player_hint(query))
                self._send_bytes(status, body, "application/octet-stream")
                return
            if parsed.path == "/health":
                self._send_json(HTTPStatus.OK, {"ok": True})
                return
            self._send_json(HTTPStatus.NOT_FOUND, {"ok": False, "error": "not found"})

        def do_POST(self) -> None:
            parsed = urlparse(self.path)
            if parsed.path == "/input":
                payload = self._read_json()
                token = str(payload.get("token", ""))
                try:
                    buttons = int(payload.get("buttons", 0))
                except (TypeError, ValueError):
                    buttons = 0
                result = session.set_buttons(token, buttons)
                self._send_json(HTTPStatus.OK if result.get("ok") else HTTPStatus.FORBIDDEN, result)
                return
            self._send_json(HTTPStatus.NOT_FOUND, {"ok": False, "error": "not found"})

        def _read_json(self) -> dict[str, Any]:
            length = int(self.headers.get("Content-Length", "0") or "0")
            raw = self.rfile.read(length) if length > 0 else b"{}"
            try:
                value = json.loads(raw.decode("utf-8"))
            except json.JSONDecodeError:
                return {}
            return value if isinstance(value, dict) else {}

        def _send_json(self, status: int, payload: dict[str, Any]) -> None:
            body = json.dumps(payload, separators=(",", ":")).encode("utf-8")
            self._send_bytes(status, body, "application/json")

        def _send_bytes(self, status: int, body: bytes, content_type: str) -> None:
            self.send_response(status)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            self.wfile.write(body)

    return MiniBrowserHandler


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Serve a two-browser sm_rev mini multiplayer MVP.")
    parser.add_argument("--host", default=DEFAULT_HOST)
    parser.add_argument("--port", type=int, default=DEFAULT_PORT)
    parser.add_argument("--fps", type=int, default=60)
    parser.add_argument("--lib", type=Path, default=None)
    parser.add_argument("--no-build", action="store_true")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    library = args.lib if args.lib is not None else ensure_library(REPO_ROOT, not args.no_build)
    kernel = MiniKernel(library)
    session = MiniBrowserSession(kernel)
    runner = MiniFrameRunner(session, max(1, args.fps))
    handler = make_handler(session)
    server = ThreadingHTTPServer((args.host, args.port), handler)
    runner.start()
    url = f"http://{args.host}:{server.server_port}"
    print(f"mini browser server listening on {url}")
    print(f"P1: {url}/p1")
    print(f"P2: {url}/p2")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.shutdown()
        runner.stop()
        session.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
