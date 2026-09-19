#!/usr/bin/env python3
"""Pack Landing Site editor bins into src/pico/assets/pico_ls_extracted.inc.

Reads assets/local_mini tileset 00 + room_91F8.json + default BG2. Does not
open sm.smc. Regenerates the extracted C arrays used by pico_ls_assets.c.
"""

from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "assets" / "local_mini"
DEFAULT_OUT = ROOT / "src" / "pico" / "assets" / "pico_ls_extracted.inc"

TILES_PATH = ASSETS / "tilesets" / "tileset_00_tiles4bpp.bin"
META_PATH = ASSETS / "tilesets" / "tileset_00_metatile_words.bin"
PAL_PATH = ASSETS / "tilesets" / "tileset_00_palette.bin"
ROOM_PATH = ASSETS / "room_91F8.json"

K_TILES_SIZE = 1024 * 32
K_META_WORDS = 1024 * 4
K_PAL_COUNT = 8 * 16
K_BG2_WORDS = 64 * 32
K_METATILE_MASK = 0x03FF
K_HFLIP = 0x0400
K_VFLIP = 0x0800
K_MAP_WORDS = 64 * 32


def load_u16(path: Path, count: int) -> list[int]:
    data = path.read_bytes()
    if len(data) != count * 2:
        raise SystemExit(f"{path}: expected {count * 2} bytes, got {len(data)}")
    return list(struct.unpack("<" + "H" * count, data))


def snes_map_index(tx: int, ty: int) -> int:
    addr = (ty & 31) * 32 + (tx & 31)
    if tx & 32:
        addr += 0x400
    if ty & 32:
        addr += 0x800
    return addr


def expand_bg1(block_words: list[list[int]], meta: list[int], origin_bx: int, origin_by: int) -> list[int]:
    height = len(block_words)
    width = len(block_words[0]) if height else 0
    out = [0] * K_MAP_WORDS
    for by in range(16):
        room_y = origin_by + by
        for bx in range(32):
            room_x = origin_bx + bx
            if room_y < 0 or room_y >= height or room_x < 0 or room_x >= width:
                continue
            level = block_words[room_y][room_x] & 0xFFFF
            mt = level & K_METATILE_MASK
            # Air (0xFF) is a color-0 tile; keep it so word 0 is not tileset tile 0.
            if mt >= 1024:
                continue
            hflip = (level & K_HFLIP) != 0
            vflip = (level & K_VFLIP) != 0
            base = mt * 4
            for quadrant in range(4):
                src = quadrant
                if hflip:
                    src ^= 1
                if vflip:
                    src ^= 2
                attr = meta[base + src]
                if hflip:
                    attr ^= 0x4000
                if vflip:
                    attr ^= 0x8000
                tx = bx * 2 + (quadrant & 1)
                ty = by * 2 + ((quadrant & 2) >> 1)
                out[snes_map_index(tx, ty)] = attr & 0xFFFF
    return out


def expand_bg2(linear: list[int]) -> list[int]:
    out = [0] * K_MAP_WORDS
    for ty in range(32):
        for tx in range(64):
            out[snes_map_index(tx, ty)] = linear[ty * 64 + tx] & 0xFFFF
    return out


def emit_u8(fp, name: str, data: bytes) -> None:
    fp.write(f"static const uint8_t {name}[{len(data)}] = {{\n")
    for i in range(0, len(data), 16):
        chunk = data[i : i + 16]
        fp.write("  " + ", ".join(f"0x{b:02X}" for b in chunk))
        fp.write(",\n" if i + 16 < len(data) or chunk else "\n")
    fp.write("};\n\n")


def emit_u16(fp, name: str, data: list[int]) -> None:
    fp.write(f"static const uint16_t {name}[{len(data)}] = {{\n")
    for i in range(0, len(data), 8):
        chunk = data[i : i + 8]
        fp.write("  " + ", ".join(f"0x{w:04X}" for w in chunk))
        fp.write(",\n" if i + 8 < len(data) or chunk else "\n")
    fp.write("};\n\n")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-o", "--output", type=Path, default=DEFAULT_OUT)
    args = parser.parse_args()

    tiles = TILES_PATH.read_bytes()
    if len(tiles) != K_TILES_SIZE:
        raise SystemExit(f"{TILES_PATH}: expected {K_TILES_SIZE} bytes, got {len(tiles)}")
    meta = load_u16(META_PATH, K_META_WORDS)
    pal = load_u16(PAL_PATH, K_PAL_COUNT)
    room = json.loads(ROOM_PATH.read_text())
    if int(room.get("tileset", -1)) != 0:
        raise SystemExit(f"{ROOM_PATH}: expected tileset 0, got {room.get('tileset')}")

    bg_assets = room.get("backgroundAssets") or {}
    preferred = bg_assets.get("defaultVariantKey")
    variants = bg_assets.get("variants") or []
    selected = None
    for variant in variants:
        if preferred and variant.get("key") == preferred:
            selected = variant
            break
        if selected is None:
            selected = variant
    if not selected or not selected.get("tilemapWordsPath"):
        raise SystemExit(f"{ROOM_PATH}: missing BG2 variant")
    bg2_path = (ROOM_PATH.parent / selected["tilemapWordsPath"]).resolve()
    bg2_linear = load_u16(bg2_path, K_BG2_WORDS)

    camera = room.get("camera") or {}
    camera_x = int(camera.get("cameraX", 0))
    camera_y = int(camera.get("cameraY", 0))
    origin_bx = camera_x // 16
    origin_by = camera_y // 16
    block_words = room["blockWords"]
    bg1 = expand_bg1(block_words, meta, origin_bx, origin_by)
    bg2 = expand_bg2(bg2_linear)

    packed = len(tiles) + len(pal) * 2 + len(bg1) * 2 + len(bg2) * 2
    unpacked = (
        TILES_PATH.stat().st_size
        + PAL_PATH.stat().st_size
        + META_PATH.stat().st_size
        + bg2_path.stat().st_size
        + room["widthBlocks"] * room["heightBlocks"] * 2
    )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", encoding="utf-8") as fp:
        fp.write("/* Generated by tools/pico_pack_ls_assets.py from assets/local_mini.\n")
        fp.write(" * Landing Site (0x91F8) tileset 00 + default BG2 + spawn-window BG1.\n")
        fp.write(" * Do not embed sm.smc. ROM was not used.\n")
        fp.write(" */\n\n")
        fp.write("enum {\n")
        fp.write(f"  kPicoLsPackedTilesSize = {len(tiles)},\n")
        fp.write(f"  kPicoLsPackedPaletteCount = {len(pal)},\n")
        fp.write(f"  kPicoLsPackedTilemapWords = {K_MAP_WORDS},\n")
        fp.write(f"  kPicoLsPackedSize = {packed},\n")
        fp.write(f"  kPicoLsUnpackedSourceSize = {unpacked},\n")
        fp.write(f"  kPicoLsCameraX = {camera_x},\n")
        fp.write(f"  kPicoLsCameraY = {camera_y},\n")
        fp.write(f"  kPicoLsOriginBlockX = {origin_bx},\n")
        fp.write(f"  kPicoLsOriginBlockY = {origin_by}\n")
        fp.write("};\n\n")
        emit_u8(fp, "kPicoLsTiles4bpp", tiles)
        emit_u16(fp, "kPicoLsPalette", pal)
        emit_u16(fp, "kPicoLsBg1Tilemap", bg1)
        emit_u16(fp, "kPicoLsBg2Tilemap", bg2)

    bg1_nonzero = sum(1 for w in bg1 if w)
    bg2_nonzero = sum(1 for w in bg2 if w)
    pal_nonzero = sum(1 for w in pal if w)
    print(f"wrote {args.output}")
    print(f"bg2 variant {selected.get('key')} from {bg2_path.relative_to(ROOT)}")
    print(f"camera {camera_x},{camera_y} origin block {origin_bx},{origin_by}")
    print(f"packed {packed} unpacked_source {unpacked}")
    print(f"nonzero pal={pal_nonzero} bg1_words={bg1_nonzero} bg2_words={bg2_nonzero}")
    print("rom used: no")
    if pal_nonzero == 0 or bg2_nonzero == 0 or not any(tiles):
        print("FAIL: packed arrays look empty", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
