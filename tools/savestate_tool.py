#!/usr/bin/env python3
"""Utilities for generating and inspecting sm_rev save states.

The sm_rev .sav format wraps an input log plus one native SNES snapshot.
This tool intentionally edits only the current snapshot, which is what
--load-state uses for normal gameplay resumes.
"""

from __future__ import annotations

import argparse
import gzip
import json
import os
import struct
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path


WRAM_SIZE = 0x20000
SRAM_SIZE = 0x2000
SNES_FIELD_SPAN_BEFORE_WRAM = 54
SNAPSHOT_TRAILING_RAMADR_SIZE = 4

FIELD_MAP = {
    "bug_fix_counter": 0x00C2,
    "game_state": 0x0998,
    "room_ptr": 0x079B,
    "area_index": 0x079F,
    "equipped_items": 0x09A2,
    "collected_items": 0x09A4,
    "equipped_beams": 0x09A6,
    "collected_beams": 0x09A8,
    "health": 0x09C2,
    "max_health": 0x09C4,
    "missiles": 0x09C6,
    "max_missiles": 0x09C8,
    "super_missiles": 0x09CA,
    "max_super_missiles": 0x09CC,
    "power_bombs": 0x09CE,
    "max_power_bombs": 0x09D0,
    "samus_x": 0x0AF6,
    "samus_y": 0x0AFA,
    "samus_pose": 0x0A1C,
    "samus_x_speed": 0x0DBC,
    "samus_y_speed": 0x0B2E,
    "boss_bits_area0": 0xD828,
    "event_byte_0": 0xD820,
}

BUTTON_MAP = {
    "r": 0x0010,
    "l": 0x0020,
    "x": 0x0040,
    "a": 0x0080,
    "right": 0x0100,
    "left": 0x0200,
    "down": 0x0400,
    "up": 0x0800,
    "start": 0x1000,
    "select": 0x2000,
    "y": 0x4000,
    "b": 0x8000,
}

DEFAULT_TORIZO_SOURCE = Path("../custom_integrations/SuperMetroid-Snes/Flyway [from Parlor and Alcatraz].state")
DEFAULT_TEMPLATE = Path("saves/save0.sav")


def parse_int(value: str) -> int:
    return int(value, 0)


def read_u16(buf: bytes | bytearray, addr: int) -> int:
    return struct.unpack_from("<H", buf, addr)[0]


def write_u16(buf: bytearray, addr: int, value: int) -> None:
    if not 0 <= value <= 0xFFFF:
        raise ValueError(f"value for 0x{addr:04x} is outside uint16 range: {value}")
    struct.pack_into("<H", buf, addr, value)


def parse_setters(raw_setters: list[str]) -> list[tuple[int, int]]:
    setters: list[tuple[int, int]] = []
    for raw in raw_setters:
        if "=" not in raw:
            raise ValueError(f"--set expects FIELD=VALUE or 0xADDR=VALUE, got {raw!r}")
        key, value_text = raw.split("=", 1)
        if key in FIELD_MAP:
            addr = FIELD_MAP[key]
        else:
            addr = parse_int(key)
        if not 0 <= addr < WRAM_SIZE:
            raise ValueError(f"WRAM address is outside 0x0000..0x1ffff: {key}")
        setters.append((addr, parse_int(value_text)))
    return setters


def apply_setters(wram: bytearray, raw_setters: list[str]) -> None:
    for addr, value in parse_setters(raw_setters):
        write_u16(wram, addr, value)


def parse_s9x_blocks(path: Path) -> dict[str, bytes]:
    data = path.read_bytes()
    if data.startswith(b"\x1f\x8b"):
        data = gzip.decompress(data)
    if not data.startswith(b"#!s9xsnp:"):
        raise ValueError(f"{path} is not an Snes9x snapshot")

    pos = data.find(b"\n") + 1
    blocks: dict[str, bytes] = {}
    while pos < len(data):
        if pos + 11 > len(data) or data[pos + 3:pos + 4] != b":" or data[pos + 10:pos + 11] != b":":
            raise ValueError(f"bad Snes9x block header at offset {pos}")
        tag = data[pos:pos + 3].decode("ascii")
        size = int(data[pos + 4:pos + 10])
        start = pos + 11
        end = start + size
        blocks[tag] = data[start:end]
        pos = end
    return blocks


@dataclass
class SmRevState:
    header: list[int]
    log: bytes
    base_snapshot: bytes
    snapshot: bytearray

    @classmethod
    def load(cls, path: Path) -> "SmRevState":
        data = path.read_bytes()
        if len(data) < 64:
            raise ValueError(f"{path} is too small to be an sm_rev .sav")
        header = list(struct.unpack_from("<16I", data, 0))
        if header[0] != 2:
            raise ValueError(f"{path} has unsupported sm_rev save version {header[0]}")
        log_size = header[2]
        base_size = header[5]
        snapshot_size = header[6]
        log_start = 64
        base_start = log_start + log_size
        snapshot_start = base_start + base_size
        snapshot_end = snapshot_start + snapshot_size
        if snapshot_end != len(data):
            raise ValueError(
                f"{path} size mismatch: header ends at {snapshot_end}, file is {len(data)} bytes"
            )
        return cls(
            header=header,
            log=data[log_start:base_start],
            base_snapshot=data[base_start:snapshot_start],
            snapshot=bytearray(data[snapshot_start:snapshot_end]),
        )

    def wram_offset(self) -> int:
        offset = len(self.snapshot) - WRAM_SIZE - SNAPSHOT_TRAILING_RAMADR_SIZE
        if offset < 0:
            raise ValueError("snapshot is too small to contain WRAM")
        return offset

    def sram_offset(self) -> int:
        offset = self.wram_offset() - SNES_FIELD_SPAN_BEFORE_WRAM - SRAM_SIZE
        if offset < 0:
            raise ValueError("snapshot is too small to contain SRAM")
        return offset

    def wram(self) -> bytearray:
        start = self.wram_offset()
        return self.snapshot[start:start + WRAM_SIZE]

    def replace_wram(self, wram: bytes | bytearray) -> None:
        if len(wram) != WRAM_SIZE:
            raise ValueError(f"WRAM must be {WRAM_SIZE} bytes, got {len(wram)}")
        start = self.wram_offset()
        self.snapshot[start:start + WRAM_SIZE] = wram

    def replace_sram(self, sram: bytes | bytearray) -> None:
        if len(sram) < SRAM_SIZE:
            raise ValueError(f"SRAM must be at least {SRAM_SIZE} bytes, got {len(sram)}")
        start = self.sram_offset()
        self.snapshot[start:start + SRAM_SIZE] = sram[:SRAM_SIZE]

    def write(self, path: Path, *, current_snapshot_only: bool = False) -> None:
        header = self.header[:]
        log = b"" if current_snapshot_only else self.log
        base = b"" if current_snapshot_only else self.base_snapshot
        header[2] = len(log)
        header[5] = len(base)
        header[6] = len(self.snapshot)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(struct.pack("<16I", *header) + log + base + self.snapshot)


def summarize_wram(wram: bytes | bytearray) -> dict[str, int]:
    return {name: read_u16(wram, addr) for name, addr in FIELD_MAP.items()}


def command_inspect(args: argparse.Namespace) -> int:
    path = Path(args.path)
    try:
        blocks = parse_s9x_blocks(path)
        wram = blocks["RAM"]
        summary = summarize_wram(wram)
        summary["format"] = "s9x"
        summary["size"] = path.stat().st_size
    except Exception:
        state = SmRevState.load(path)
        wram = state.wram()
        summary = summarize_wram(wram)
        summary["format"] = "sm_rev"
        summary["size"] = path.stat().st_size
        summary["snapshot_size"] = len(state.snapshot)
        summary["log_size"] = len(state.log)
        summary["base_snapshot_size"] = len(state.base_snapshot)

    if args.json:
        print(json.dumps(summary, indent=2, sort_keys=True))
    else:
        print(
            f"{path}: {summary['format']} room=0x{summary['room_ptr']:04x} "
            f"state={summary['game_state']} x={summary['samus_x']} y={summary['samus_y']} "
            f"hp={summary['health']}/{summary['max_health']} "
            f"missiles={summary['missiles']}/{summary['max_missiles']} "
            f"items=0x{summary['equipped_items']:04x} event0=0x{summary['event_byte_0']:04x}"
        )
    return 0


def import_s9x_to_sm_rev(source: Path, template: Path, output: Path, setters: list[str]) -> None:
    blocks = parse_s9x_blocks(source)
    if "RAM" not in blocks:
        raise ValueError(f"{source} has no RAM block")
    if "SRA" not in blocks:
        raise ValueError(f"{source} has no SRA block")

    state = SmRevState.load(template)
    wram = bytearray(blocks["RAM"])
    write_u16(wram, FIELD_MAP["bug_fix_counter"], 1)
    apply_setters(wram, setters)
    state.replace_wram(wram)
    state.replace_sram(blocks["SRA"])
    state.header = [0] * 16
    state.header[0] = 2
    state.write(output, current_snapshot_only=True)


def command_import_s9x(args: argparse.Namespace) -> int:
    import_s9x_to_sm_rev(Path(args.source), Path(args.template), Path(args.output), args.set)
    return command_inspect(argparse.Namespace(path=args.output, json=args.json))


def command_patch(args: argparse.Namespace) -> int:
    state = SmRevState.load(Path(args.input))
    wram = state.wram()
    apply_setters(wram, args.set)
    state.replace_wram(wram)
    state.write(Path(args.output))
    return command_inspect(argparse.Namespace(path=args.output, json=args.json))


def swapped_input(*buttons: int) -> int:
    value = 0
    for button in buttons:
        value |= button
    result = 0
    x = value
    for _ in range(16):
        result = result * 2 + (x & 1)
        x >>= 1
    return result


def parse_buttons(raw: str) -> int:
    if raw.startswith("0x"):
        return parse_int(raw)
    value = 0
    for name in raw.replace(",", "+").split("+"):
        name = name.strip().lower()
        if not name or name == "none":
            continue
        try:
            value |= BUTTON_MAP[name]
        except KeyError as exc:
            raise ValueError(f"unknown button {name!r}; known buttons: {', '.join(BUTTON_MAP)}") from exc
    return value


def parse_hold(raw: str) -> tuple[int, int]:
    if ":" not in raw:
        raise ValueError(f"--hold expects BUTTONS:FRAMES, got {raw!r}")
    buttons, frames_text = raw.rsplit(":", 1)
    frames = parse_int(frames_text)
    if frames < 0:
        raise ValueError(f"--hold frame count must be non-negative, got {frames}")
    return swapped_input(parse_buttons(buttons)), frames


def build_input_values(holds: list[str], idle_frames: int, raw_input_list: str | None) -> list[int]:
    if holds and raw_input_list:
        raise ValueError("--hold and --input-list are mutually exclusive")
    if raw_input_list:
        values = [int(token.strip(), 16) for token in raw_input_list.split(",") if token.strip()]
    else:
        values = []
        for raw_hold in holds:
            encoded, frames = parse_hold(raw_hold)
            values.extend([encoded] * frames)
        values.extend([0] * idle_frames)
    for value in values:
        if not 0 <= value <= 0xFFFF:
            raise ValueError(f"input value is outside uint16 range: {value}")
    return values


def write_headless_config(path: Path, *, fullspec: bool) -> None:
    path.write_text(
        "[General]\n"
        "SkipIntro = 0\n"
        "DisableFrameDelay = 1\n"
        "[Features]\n"
        f"FullSpec = {1 if fullspec else 0}\n"
        "[Graphics]\n"
        "OutputMethod = SDL\n"
        "[Sound]\n"
        "EnableAudio = 0\n",
        encoding="utf-8",
    )


def run_sm_rev_save(
    *,
    repo: Path,
    binary: Path,
    config: Path,
    input_state: Path,
    output: Path,
    frames: int,
    input_values: list[int],
) -> None:
    if frames < 0:
        raise ValueError(f"frame count must be non-negative, got {frames}")
    if not input_values:
        input_values = [0]
    input_list = ",".join(f"{value:04x}" for value in input_values)
    env = os.environ.copy()
    env.update({"SDL_VIDEODRIVER": "offscreen", "SDL_AUDIODRIVER": "dummy", "DISPLAY": ""})
    cmd = [
        str(binary),
        "--config",
        str(config),
        "--headless",
        str(frames),
        "--runmode",
        "mine",
        "--load-state",
        str(input_state),
        "--input-list",
        input_list,
        "--save-state",
        str(output),
    ]
    subprocess.run(cmd, cwd=repo, env=env, check=True)


def command_advance(args: argparse.Namespace) -> int:
    repo = Path(__file__).resolve().parents[1]
    input_state = (repo / args.input).resolve() if not Path(args.input).is_absolute() else Path(args.input)
    output = (repo / args.output).resolve() if not Path(args.output).is_absolute() else Path(args.output)
    binary = (repo / args.binary).resolve() if not Path(args.binary).is_absolute() else Path(args.binary)

    input_values = build_input_values(args.hold, args.idle_frames, args.input_list)
    frames = args.frames if args.frames is not None else len(input_values)
    if frames == 0 and not input_values:
        frames = args.idle_frames
    with tempfile.TemporaryDirectory(prefix="sm_rev_state_") as tmp:
        config = Path(tmp) / "headless.ini"
        write_headless_config(config, fullspec=args.fullspec)
        run_sm_rev_save(
            repo=repo,
            binary=binary,
            config=config,
            input_state=input_state,
            output=output,
            frames=frames,
            input_values=input_values,
        )
    return command_inspect(argparse.Namespace(path=output, json=args.json))


def command_flyway_pre_torizo(args: argparse.Namespace) -> int:
    repo = Path(__file__).resolve().parents[1]
    source = (repo / args.source).resolve() if not Path(args.source).is_absolute() else Path(args.source)
    template = (repo / args.template).resolve() if not Path(args.template).is_absolute() else Path(args.template)
    output = (repo / args.output).resolve() if not Path(args.output).is_absolute() else Path(args.output)
    binary = (repo / args.binary).resolve() if not Path(args.binary).is_absolute() else Path(args.binary)

    right_b = swapped_input(0x0100, 0x8000)
    input_list = ",".join([f"{right_b:04x}"] * args.frames_to_door + ["0000"] * args.idle_frames)
    with tempfile.TemporaryDirectory(prefix="sm_rev_state_") as tmp:
        tmpdir = Path(tmp)
        seed = tmpdir / "flyway_seed.sav"
        config = tmpdir / "no_fullspec.ini"
        import_s9x_to_sm_rev(
            source,
            template,
            seed,
            ["missiles=10", "max_missiles=10", "health=90", "max_health=99"],
        )
        write_headless_config(config, fullspec=False)
        run_sm_rev_save(
            repo=repo,
            binary=binary,
            config=config,
            input_state=seed,
            output=output,
            frames=args.frames_to_door + args.idle_frames,
            input_values=[int(token, 16) for token in input_list.split(",")],
        )
    return command_inspect(argparse.Namespace(path=output, json=args.json))


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)

    inspect_p = sub.add_parser("inspect", help="print core RAM fields from an sm_rev .sav or Snes9x .state")
    inspect_p.add_argument("path")
    inspect_p.add_argument("--json", action="store_true")
    inspect_p.set_defaults(func=command_inspect)

    import_p = sub.add_parser("import-s9x", help="graft Snes9x RAM/SRAM into an sm_rev snapshot template")
    import_p.add_argument("source")
    import_p.add_argument("--template", default=str(DEFAULT_TEMPLATE))
    import_p.add_argument("--output", required=True)
    import_p.add_argument("--set", action="append", default=[], metavar="FIELD=VALUE")
    import_p.add_argument("--json", action="store_true")
    import_p.set_defaults(func=command_import_s9x)

    patch_p = sub.add_parser("patch", help="patch uint16 WRAM fields in an existing sm_rev .sav")
    patch_p.add_argument("input")
    patch_p.add_argument("--output", required=True)
    patch_p.add_argument("--set", action="append", default=[], metavar="FIELD=VALUE")
    patch_p.add_argument("--json", action="store_true")
    patch_p.set_defaults(func=command_patch)

    advance_p = sub.add_parser("advance", help="load an sm_rev .sav, run headless input, and save the result")
    advance_p.add_argument("input")
    advance_p.add_argument("--output", required=True)
    advance_p.add_argument("--binary", default="./sm_rev")
    advance_p.add_argument("--frames", type=int)
    advance_p.add_argument("--hold", action="append", default=[], metavar="BUTTONS:FRAMES")
    advance_p.add_argument("--idle-frames", type=int, default=0)
    advance_p.add_argument("--input-list", help="raw comma-separated sm_rev input values, already bit-swapped")
    advance_p.add_argument("--fullspec", action="store_true", help="run with FullSpec enabled")
    advance_p.add_argument("--json", action="store_true")
    advance_p.set_defaults(func=command_advance)

    torizo_p = sub.add_parser("flyway-pre-torizo", help="generate a stable Flyway state at the Bomb Torizo door")
    torizo_p.add_argument("--source", default=str(DEFAULT_TORIZO_SOURCE))
    torizo_p.add_argument("--template", default=str(DEFAULT_TEMPLATE))
    torizo_p.add_argument("--binary", default="./sm_rev")
    torizo_p.add_argument("--output", default="saves/flyway_pre_torizo.sav")
    torizo_p.add_argument("--frames-to-door", type=int, default=150)
    torizo_p.add_argument("--idle-frames", type=int, default=4)
    torizo_p.add_argument("--json", action="store_true")
    torizo_p.set_defaults(func=command_flyway_pre_torizo)

    return parser


def main(argv: list[str]) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        return args.func(args)
    except Exception as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
