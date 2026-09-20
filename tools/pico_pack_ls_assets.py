#!/usr/bin/env python3
"""Pack Landing Site editor bins into src/pico/assets/pico_ls_extracted.inc.

Reads assets/local_mini tileset 00 + room_91F8.json + default BG2 + packed
Samus animation frames (bank92 / samus_data / power palette) + the Landing
Site gunship roomSprites. Does not open sm.smc.
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
DEFAULT_SAMUS_OUT = ROOT / "src" / "pico" / "assets" / "pico_samus_anim.inc"
DEFAULT_GUNSHIP_OUT = ROOT / "src" / "pico" / "assets" / "pico_ls_gunship.inc"
DEFAULT_ROOM_OUT = ROOT / "src" / "pico" / "assets" / "pico_ls_room.inc"

TILES_PATH = ASSETS / "tilesets" / "tileset_00_tiles4bpp.bin"
META_PATH = ASSETS / "tilesets" / "tileset_00_metatile_words.bin"
PAL_PATH = ASSETS / "tilesets" / "tileset_00_palette.bin"
ROOM_PATH = ASSETS / "room_91F8.json"

K_TILES_SIZE = 1024 * 32
K_META_WORDS = 1024 * 4
K_PAL_COUNT = 8 * 16
K_BG2_WORDS = 64 * 32
# The BG2 .bin is NOT a 64x32 linear tilemap. It is a raw 4096-byte dump of
# BG2 VRAM 0x4800..0x4FFF, i.e. two SNES 32x32 screens. Vanilla DMAs exactly
# one screen per door transition; the other is left as the stale VRAM fill
# 0x2C0F (tile 0x0F, palette 3, and priority 1, so it paints over BG1).
K_BG2_SCREEN_WORDS = 32 * 32
K_BG2_STALE_FILL = 0x2C0F
K_BG2_PRIORITY_BIT = 0x2000
# room_91F8.json scroll.bgScrolling = 0x0181 -> layer2_scroll_x = 0x81,
# layer2_scroll_y = 0x01 (see src/mini/mini_room_adapter.c:143).
#   X: CalculateLayer2Xpos() (src/room_scrolling.c:105) takes t = 0x81 & 0xFE
#      = 0x80 and returns 0x80*HIBYTE(x) + (0x80*LOBYTE(x) >> 8), which is
#      exactly layer1_x_pos / 2. BG2 parallaxes at HALF the BG1 rate.
#   Y: CalculateLayer2Ypos() returns 1 when layer2_scroll_y == 1, and the
#      caller then leaves reg_BG2VOFS alone (src/room_scrolling.c:156). BG2
#      never scrolls vertically; it holds its room-load value, which is 0
#      (mini_room_adapter.c zeroes bg2_y_scroll, hdma_core.c layer2_y_pos).
K_BG2_SCROLL_X = 0x81
K_BG2_SCROLL_Y = 0x01
K_BG2_VOFS = 0
K_METATILE_MASK = 0x03FF
K_HFLIP = 0x0400
K_VFLIP = 0x0800
K_MAP_WORDS = 64 * 32
K_ENEMY_TILE_BASE = 256
K_GUNSHIP_TILES = 144
K_GUNSHIP_TILE_BYTES = K_GUNSHIP_TILES * 32
# Priority 2 == the Samus spritemap attr, so OAM index decides overlap and the
# ship (higher slots) stays behind her instead of drawing over her.
K_SPRITE_PRI = 2
K_GUNSHIP_PAL_TOP = 1  # CGRAM 144, not blob pal 0
K_GUNSHIP_PAL_BOTTOM = 2  # CGRAM 160
K_GUNSHIP_KEYS = (
    "landing_ship_top",
    "landing_ship_bottom_front",
    "landing_ship_bottom_rear",
)
# room_91F8.json gives all three ship roomSprites pixelY 1144, but the D07F
# "top" piece really sits 40 px higher. Measured against out/rom_f120.png (a
# ROM capture of this room): its gold row profile matches world rows 1127.. at
# screen 126.. (world = screen + 1001) while the D0BF body matches world
# 1126.. at screen 165.. (world = screen + 961). 1144 - 40 = 1104 = block row
# 69 * 16, the top of the ship deck collision. With the correction the pieces
# are contiguous (top 1086..1117, body 1118..1165) and stop overlapping.
K_GUNSHIP_Y_FIX = {"landing_ship_top": -40}
# Measured room facts, asserted before anything is written. The firmware room
# is worthless if these move, because the physics fix depends on them.
K_ROOM_BLOCK_PX = 16
K_ROOM_SPAWN_BLOCK = (72, 68)
K_ROOM_DECK_ROWS = range(69, 74)     # gunship deck collision, blockWord 0x80ff
K_ROOM_DECK_SOLID_COLS = range(69, 75)
K_ROOM_FLOOR_ROWS = range(77, 80)    # real terrain, world y 1232+
K_BLOCK_TYPE_SHIFT = 12
K_BLOCK_TYPE_AIR = 0x0
K_BLOCK_TYPE_SLOPE = 0x1
K_BLOCK_TYPE_SOLID = 0x8
K_SAMUS_POSE = 1  # kPose_01_FaceR_Normal (the stand-still reference pose)
K_SAMUS_ANIM_FRAME = 0
K_SAMUS_CHR_SIZE = 32 * 32  # OBJ tiles 0-31 at name base (0x6000..0x61FF words)
# kPoseParams[K_SAMUS_POSE].y_offset_to_gfx. Not a choice: it is asserted
# against the packed bank 0x91 table below, and only survives as the reference
# value the display side reports.
K_SAMUS_Y_OFFSET_TO_GFX = 6
K_SAMUS_Y_RADIUS = 21  # kPoseParams[K_SAMUS_POSE].y_radius, same assertion
K_SAMUS_POSE_TABLE_LEN = 0x100
# Airborne poses: kept only to label the per-pose report, not to derive
# anything. sm_rev-k5q.6 replaced the foot-alignment stand-in with the real
# kPoseParams table, which covers air and ground alike.
K_SAMUS_AIR_POSES = frozenset((0x13, 0x14, 0x19, 0x1A, 0x2F, 0x30))
# Stand, run, turn (the walking set), plus jump/spin heads so the sim cannot
# index an unpacked pose during a hop. Each entry: (pose, short name).
K_SAMUS_POSES = (
    (0x00, "FaceF"),
    (0x01, "StandR"),
    (0x02, "StandL"),
    (0x09, "RunR"),
    (0x0A, "RunL"),
    (0x0B, "RunR_Gun"),
    (0x0C, "RunL_Gun"),
    (0x13, "JumpR"),
    (0x14, "JumpL"),
    (0x19, "SpinR"),
    (0x1A, "SpinL"),
    (0x25, "TurnR"),
    (0x26, "TurnL"),
    (0x2F, "TurnJumpR"),
    (0x30, "TurnJumpL"),
    # B maps to Down on the Explorer pad, so crouch and morphball are one
    # button press away and must not fall back to standing.
    (0x1D, "MorphR"),
    (0x1E, "MorphMoveR"),
    (0x1F, "MorphMoveL"),
    # sm_rev-17t: 0x41 is 0x1D's LEFT-facing partner, and it is the pose mini
    # actually uses to roll left on the ground (0x1F is never reached -- see
    # tests/test_pico_samus_anim_lr.c). Leaving it out made every left-facing
    # morphball frame resolve to kPicoOamSamusFallbackPose, i.e. a standing
    # Samus that slides without animating. That was the reported on-glass bug.
    (0x41, "MorphL"),
    # The rest of the morphball family, so a hop or a (un)morph transition
    # cannot land on an unpacked pose either.
    (0x31, "MorphAirR"),
    (0x32, "MorphAirL"),
    (0x37, "MorphTransR"),
    (0x38, "MorphTransL"),
    (0x3D, "UnmorphTransR"),
    (0x3E, "UnmorphTransL"),
    (0x27, "CrouchR"),
    (0x28, "CrouchL"),
    (0x35, "CrouchTransR"),
    (0x36, "CrouchTransL"),
    (0x3B, "StandTransR"),
    (0x3C, "StandTransL"),
    (0x43, "TurnCrouchR"),
    (0x44, "TurnCrouchL"),
)
K_SAMUS_BANK92_OFF = 0x8000
# ---- ROM bank 0x91: Samus pose parameters + animation delay data ------------
# assets/local_mini carries no bank 0x91, so two runtime numbers were invented
# (sm_rev-k5q.6). samus_bank91.bin is the same LoROM slice bank 0x92 already
# is: sm.smc[(bank - 0x80) * 0x8000 ...], SNES $918000..$91FFFF.
BANK91_PATH = ASSETS / "samus" / "samus_bank91.bin"
K_SAMUS_BANK91_OFF = 0x8000
K_SAMUS_BANK91_SIZE = 0x8000
# kPoseParams (0x91B629, 8 bytes x 256 poses, ends 0x91BE29), the animation
# delay pointer table (0x91B010) and every delay byte stream it points at, the
# default/speed-boost delay pointers (0x91B5D1 / 0x91B5DE) and kSpeedBoostToCtr
# (0x91B61F) all live inside this one 4KB window. Packing the window instead of
# individual tables means the vanilla pointer-chasing in Samus_HandleAnimDelay()
# runs unmodified; every address it can reach is validated to be inside it.
K_SAMUS_BANK91_WINDOW = 0xB000
K_SAMUS_BANK91_WINDOW_LEN = 0x1000
K_SAMUS_POSE_PARAMS = 0xB629
K_SAMUS_POSE_PARAM_SIZE = 8
K_SAMUS_POSE_PARAM_Y_OFF = 4   # SamusPoseParams.y_offset_to_gfx
K_SAMUS_POSE_PARAM_Y_RADIUS = 6  # SamusPoseParams.y_radius
K_SAMUS_ANIM_DELAY_TABLE = 0xB010
K_SAMUS_DEFAULT_ANIM_FRAME_PTR = 0xB5D1
K_SAMUS_SPEEDBOOST_ANIM_FRAME_PTR = 0xB5DE
K_SAMUS_SPEEDBOOST_CTR = 0xB61F
K_SAMUS_ANIM_DEF_TABLE = 0xD94E
K_SAMUS_TOP_TILE_TABLE = 0xD91E
K_SAMUS_BOTTOM_TILE_TABLE = 0xD938
K_SAMUS_TOP_SPRITEMAP_TABLE = 0x9263
K_SAMUS_BOTTOM_SPRITEMAP_TABLE = 0x945D
K_SAMUS_SPRITEMAP_PTRS = 0x808D


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


def bg2_screen(words: list[int], path: Path) -> list[int]:
    """Pick the one live 32x32 screen out of a two-screen BG2 VRAM dump.

    Reading the dump as a single 64x32 linear map interleaves the live screen
    with the stale fill (linear row r is live rows 2r and 2r+1 side by side for
    r < 16, and pure filler above), which is what put a uniform priority-1
    wallpaper over the bottom half of the frame.
    """
    count = len(words) // K_BG2_SCREEN_WORDS
    screens = [
        words[i * K_BG2_SCREEN_WORDS : (i + 1) * K_BG2_SCREEN_WORDS]
        for i in range(count)
    ]
    live = [
        s
        for s in screens
        if len(set(s)) > 1 and not any(w == K_BG2_STALE_FILL for w in s)
    ]
    if len(live) != 1:
        uniques = [len(set(s)) for s in screens]
        raise SystemExit(
            f"{path}: expected exactly 1 live BG2 screen of {count}, "
            f"got {len(live)} (unique words per screen: {uniques})"
        )
    return live[0]


def expand_bg2(screen: list[int], origin_tx: int, origin_ty: int) -> list[int]:
    """Tile one 32x32 BG2 screen across the 64x32 packed map.

    Modulo 32 in BOTH axes: the source is 256x256 px of backdrop that vanilla
    repeats, and there is no 64th source column to walk. The packed map stays
    64 tiles wide so the renderer's 512 px horizontal wrap is a whole number
    of copies of the 256 px pattern.
    """
    out = [0] * K_MAP_WORDS
    for ty in range(32):
        src_ty = (origin_ty + ty) % 32
        for tx in range(64):
            src_tx = (origin_tx + tx) % 32
            out[snes_map_index(tx, ty)] = screen[src_ty * 32 + src_tx] & 0xFFFF
    return out


def emit_u8(fp, name: str, data: bytes, static: bool = True) -> None:
    prefix = "static " if static else ""
    fp.write(f"{prefix}const uint8_t {name}[{len(data)}] = {{\n")
    for i in range(0, len(data), 16):
        chunk = data[i : i + 16]
        fp.write("  " + ", ".join(f"0x{b:02X}" for b in chunk))
        fp.write(",\n" if i + 16 < len(data) or chunk else "\n")
    fp.write("};\n\n")


def pack_gunship(room: dict, camera_x: int, camera_y: int) -> tuple[bytes, list[int], bytes, bytes]:
    """OAM in packed extract space (scroll 0). JSON cameraY 976 is not used."""
    sprites = {s.get("key"): s for s in (room.get("roomSprites") or [])}
    missing = [k for k in K_GUNSHIP_KEYS if k not in sprites]
    if missing:
        raise SystemExit(f"{ROOM_PATH}: missing roomSprites {missing}")

    chr_rel = sprites[K_GUNSHIP_KEYS[0]].get("tileDataPath")
    if not chr_rel:
        raise SystemExit(f"{ROOM_PATH}: gunship missing tileDataPath")
    chr_path = (ROOM_PATH.parent / chr_rel).resolve()
    chr_bytes = chr_path.read_bytes()
    if len(chr_bytes) != K_GUNSHIP_TILE_BYTES:
        raise SystemExit(
            f"{chr_path}: expected {K_GUNSHIP_TILE_BYTES} bytes "
            f"({K_GUNSHIP_TILES} tiles), got {len(chr_bytes)}"
        )

    pal_words: list[int] = []
    pal_seen: dict[str, int] = {}
    oam = bytearray()
    oam_hi = bytearray()
    tile_min = None
    tile_max = None

    for key in K_GUNSHIP_KEYS:
        spr = sprites[key]
        pal_rel = spr.get("palettePath")
        if not pal_rel:
            raise SystemExit(f"{ROOM_PATH}: {key} missing palettePath")
        pal_path = str((ROOM_PATH.parent / pal_rel).resolve())
        if pal_path not in pal_seen:
            if len(pal_seen) >= 2:
                raise SystemExit(f"{ROOM_PATH}: more than two gunship palettes")
            row = K_GUNSHIP_PAL_TOP if len(pal_seen) == 0 else K_GUNSHIP_PAL_BOTTOM
            pal_seen[pal_path] = row
            pal_words.extend(load_u16(Path(pal_path), 16))
        pal_row = pal_seen[pal_path]
        world_x = int(spr["pixelX"])
        world_y = int(spr["pixelY"]) + K_GUNSHIP_Y_FIX.get(key, 0)
        for entry in spr.get("entries") or []:
            tile = int(entry["tileNum"])
            local = tile - K_ENEMY_TILE_BASE
            if local < 0 or local >= K_GUNSHIP_TILES:
                raise SystemExit(
                    f"{key}: tileNum {tile} out of {K_ENEMY_TILE_BASE}.."
                    f"{K_ENEMY_TILE_BASE + K_GUNSHIP_TILES - 1}"
                )
            if tile_min is None or tile < tile_min:
                tile_min = tile
            if tile_max is None or tile > tile_max:
                tile_max = tile
            sx = world_x - camera_x + int(entry["xOffset"])
            sy = world_y - camera_y + int(entry["yOffset"])
            hi = 0
            if sx < 0:
                sx += 512
                hi |= 1
            if bool(entry.get("is16x16")):
                hi |= 2
            name = (tile >> 8) & 1
            attr = (
                name
                | ((pal_row & 7) << 1)
                | ((K_SPRITE_PRI & 3) << 4)
                | (0x40 if entry.get("hFlip") else 0)
                | (0x80 if entry.get("vFlip") else 0)
            )
            oam.extend((sx & 0xFF, sy & 0xFF, tile & 0xFF, attr & 0xFF))
            oam_hi.append(hi & 3)

    if len(oam) != 56 * 4 or len(oam_hi) != 56:
        raise SystemExit(f"gunship OAM entries {len(oam_hi)}, expected 56")
    if tile_min != K_ENEMY_TILE_BASE or tile_max != 357:
        print(f"gunship tileNum {tile_min}-{tile_max} (local {tile_min - K_ENEMY_TILE_BASE}-{tile_max - K_ENEMY_TILE_BASE})")
    return chr_bytes, pal_words, bytes(oam), bytes(oam_hi)


def bank92_word(bank92: bytes, addr: int) -> int:
    off = addr - K_SAMUS_BANK92_OFF
    if off < 0 or off + 2 > len(bank92):
        raise SystemExit(f"bank92 word {addr:#x} out of range")
    return struct.unpack_from("<H", bank92, off)[0]


def bank92_bytes(bank92: bytes, addr: int, n: int) -> bytes:
    off = addr - K_SAMUS_BANK92_OFF
    if off < 0 or off + n > len(bank92):
        raise SystemExit(f"bank92 bytes {addr:#x}+{n} out of range")
    return bank92[off : off + n]


def bank91_word(bank91: bytes, addr: int) -> int:
    off = addr - K_SAMUS_BANK91_OFF
    if off < 0 or off + 2 > len(bank91):
        raise SystemExit(f"bank91 word {addr:#x} out of range")
    return struct.unpack_from("<H", bank91, off)[0]


def bank91_bytes(bank91: bytes, addr: int, n: int) -> bytes:
    off = addr - K_SAMUS_BANK91_OFF
    if off < 0 or off + n > len(bank91):
        raise SystemExit(f"bank91 bytes {addr:#x}+{n} out of range")
    return bank91[off : off + n]


def pose_param(bank91: bytes, pose: int, field: int) -> int:
    """One SamusPoseParams byte (src/ida_types.h) out of kPoseParams."""
    return bank91_bytes(bank91, K_SAMUS_POSE_PARAMS + K_SAMUS_POSE_PARAM_SIZE * pose
                        + field, 1)[0]


def samus_data_slice(data: bytes, ranges: list[dict], snes: int, size: int) -> bytes:
    for rng in ranges:
        base = int(rng["snesAddress"])
        rng_size = int(rng["size"])
        if snes < base or snes + size > base + rng_size:
            continue
        off = int(rng["dataOffset"]) + (snes - base)
        if off < 0 or off + size > len(data):
            raise SystemExit(f"samus_data overflow {snes:#x} size {size}")
        return data[off : off + size]
    raise SystemExit(f"samus_data missing {snes:#x} size {size}")


def load_samus_half(bank92: bytes, data: bytes, ranges: list[dict], idx: int, pos: int, table: int) -> tuple[bytes, int, int]:
    tile_src = 7 * pos + bank92_word(bank92, table + idx * 2)
    raw = bank92_bytes(bank92, tile_src, 7)
    src_addr, src_bank, part1, part2 = struct.unpack_from("<HBHH", raw, 0)
    snes = src_addr | (src_bank << 16)
    blob = samus_data_slice(data, ranges, snes, part1 + part2)
    return blob, part1, part2


def load_spritemap(bank92: bytes, index: int) -> bytes:
    ptr = bank92_word(bank92, K_SAMUS_SPRITEMAP_PTRS + index * 2)
    if ptr == 0:
        raise SystemExit(f"spritemap index {index} is null")
    count = bank92_word(bank92, ptr)
    if count == 0 or count > 32:
        raise SystemExit(f"spritemap index {index} count {count}")
    return bank92_bytes(bank92, ptr, 2 + count * 5)


def samus_anim_bases(bank92: bytes) -> dict[int, int]:
    """pose -> animation-definition pointer, for every pose in the table."""
    out = {}
    for pose in range(K_SAMUS_POSE_TABLE_LEN):
        try:
            out[pose] = bank92_word(bank92, K_SAMUS_ANIM_DEF_TABLE + pose * 2)
        except SystemExit:
            break
    return out


def samus_frame_bound(bases: dict[int, int], pose: int) -> int:
    """Frames available to a pose, bounded by the next pose pointer in the bank.

    The frame lists are packed back to back, so the next distinct pointer above
    this pose's is where its list ends. That is a data-derived bound, not a
    guess, and it stops a short animation from running into its neighbour.
    """
    base = bases[pose]
    higher = [v for v in set(bases.values()) if v > base]
    if not higher:
        return 0
    return (min(higher) - base) // 4


def pack_samus_frame(bank92: bytes, data: bytes, ranges: list[dict],
                     pose: int, frame: int) -> tuple[bytes, bytes]:
    """One animation frame: 1KB CHR at the OBJ name base + joined spritemap."""
    anim_base = bank92_word(bank92, K_SAMUS_ANIM_DEF_TABLE + pose * 2)
    top_idx, top_pos, bot_idx, bot_pos = bank92_bytes(bank92, anim_base + 4 * frame, 4)
    if bot_idx == 255:
        raise SystemExit(f"pose {pose:#04x} frame {frame} has no bottom half")

    top, t1, t2 = load_samus_half(bank92, data, ranges, top_idx, top_pos, K_SAMUS_TOP_TILE_TABLE)
    bot, b1, b2 = load_samus_half(bank92, data, ranges, bot_idx, bot_pos, K_SAMUS_BOTTOM_TILE_TABLE)
    if t1 + t2 > len(top) or b1 + b2 > len(bot):
        raise SystemExit(f"pose {pose:#04x} frame {frame}: half size mismatch")
    if t1 > 256 or t2 > 256 or b1 > 256 or b2 > 256:
        raise SystemExit(f"pose {pose:#04x} frame {frame}: half exceeds 8-tile row")

    # Quadrant layout matches NmiTransferSamusToVram: top rows at 0x000/0x200,
    # bottom rows at 0x100/0x300 (VRAM words $6000/$6100/$6080/$6180).
    chr_buf = bytearray(K_SAMUS_CHR_SIZE)
    chr_buf[0:t1] = top[:t1]
    if t2:
        chr_buf[0x200 : 0x200 + t2] = top[t1 : t1 + t2]
    chr_buf[0x100 : 0x100 + b1] = bot[:b1]
    if b2:
        chr_buf[0x300 : 0x300 + b2] = bot[b1 : b1 + b2]

    top_sm_idx = frame + bank92_word(bank92, K_SAMUS_TOP_SPRITEMAP_TABLE + pose * 2)
    bot_sm_idx = frame + bank92_word(bank92, K_SAMUS_BOTTOM_SPRITEMAP_TABLE + pose * 2)
    top_sm = load_spritemap(bank92, top_sm_idx)
    bot_sm = load_spritemap(bank92, bot_sm_idx)
    top_n = struct.unpack_from("<H", top_sm, 0)[0]
    bot_n = struct.unpack_from("<H", bot_sm, 0)[0]
    if top_n + bot_n < 2:
        raise SystemExit(f"pose {pose:#04x} frame {frame}: spritemap too small")
    spritemap = struct.pack("<H", top_n + bot_n) + top_sm[2:] + bot_sm[2:]
    return bytes(chr_buf), spritemap


def spritemap_y_extent(sm: bytes) -> tuple[int, int]:
    """(min_y, max_y) of a spritemap in origin-relative pixels, for diagnostics."""
    n = struct.unpack_from("<H", sm, 0)[0]
    lo, hi = 127, -128
    for i in range(n):
        e = sm[2 + i * 5 : 7 + i * 5]
        size = 16 if (e[1] & 0x80) else 8
        y = e[2] if e[2] < 128 else e[2] - 256
        lo = min(lo, y)
        hi = max(hi, y + size - 1)
    return lo, hi


def pack_samus_anim(room: dict) -> dict:
    """All poses in K_SAMUS_POSES, every frame their table bound allows."""
    sa = room.get("samusAssets") or {}
    bank_rel = sa.get("bank92Path")
    data_rel = sa.get("dataPath")
    pal_rel = (sa.get("paletteAssets") or {}).get("powerPath")
    ranges = sa.get("ranges") or []
    if not bank_rel or not data_rel or not pal_rel or not ranges:
        raise SystemExit(f"{ROOM_PATH}: missing samusAssets bank92/data/power palette/ranges")
    if "armadillo" in str(bank_rel):
        raise SystemExit("refusing samus_armadillo assets")

    bank92 = (ROOM_PATH.parent / bank_rel).resolve().read_bytes()
    data = (ROOM_PATH.parent / data_rel).resolve().read_bytes()
    pal = load_u16((ROOM_PATH.parent / pal_rel).resolve(), 16)
    if len(bank92) != 32768:
        raise SystemExit(f"{bank_rel}: expected 32768 bytes, got {len(bank92)}")

    bank91_rel = sa.get("bank91Path")
    bank91_path = ((ROOM_PATH.parent / bank91_rel).resolve() if bank91_rel
                   else BANK91_PATH)
    if not bank91_path.exists():
        raise SystemExit(f"{bank91_path}: missing ROM bank 0x91 "
                         f"(SNES $918000..$91FFFF); kPoseParams and the "
                         f"animation delay data cannot be packed without it")
    bank91 = bank91_path.read_bytes()
    if len(bank91) != K_SAMUS_BANK91_SIZE:
        raise SystemExit(f"{bank91_path}: expected {K_SAMUS_BANK91_SIZE} bytes, "
                         f"got {len(bank91)}")

    bases = samus_anim_bases(bank92)
    chr_blob = bytearray()
    sm_blob = bytearray()
    frame_sm_off: list[int] = []
    pose_first = [0] * K_SAMUS_POSE_TABLE_LEN
    pose_count = [0] * K_SAMUS_POSE_TABLE_LEN
    # The real thing, for every pose in the table: kPoseParams[pose]
    # .y_offset_to_gfx / .y_radius straight out of bank 0x91. Vanilla draws at
    # samus_y_pos - y_offset_to_gfx (Samus_CalcSpritemapPos_Default,
    # src/samus_draw.c) and collides at samus_y_pos +/- y_radius.
    pose_yoff = [pose_param(bank91, p, K_SAMUS_POSE_PARAM_Y_OFF)
                 for p in range(K_SAMUS_POSE_TABLE_LEN)]
    pose_yradius = [pose_param(bank91, p, K_SAMUS_POSE_PARAM_Y_RADIUS)
                    for p in range(K_SAMUS_POSE_TABLE_LEN)]
    feet_by_pose: dict[int, int] = {}
    report: list[tuple] = []

    for pose, name in K_SAMUS_POSES:
        if pose not in bases:
            raise SystemExit(f"pose {pose:#04x} outside the bank92 anim table")
        bound = samus_frame_bound(bases, pose)
        first = len(frame_sm_off)
        extents = []
        for frame in range(bound):
            try:
                cb, sm = pack_samus_frame(bank92, data, ranges, pose, frame)
            except SystemExit:
                # The bound is an upper limit; short animations stop early.
                break
            frame_sm_off.append(len(sm_blob))
            chr_blob.extend(cb)
            sm_blob.extend(sm)
            extents.append(spritemap_y_extent(sm))
        n = len(frame_sm_off) - first
        if n == 0:
            raise SystemExit(f"pose {pose:#04x} ({name}) packed zero frames")
        pose_first[pose] = first
        pose_count[pose] = n
        feet = [e[1] for e in extents]
        feet_by_pose[pose] = max(feet)
        report.append((pose, name, n, min(feet), max(feet)))

    if K_SAMUS_POSE not in feet_by_pose:
        raise SystemExit(f"reference pose {K_SAMUS_POSE:#04x} was not packed")

    if len(frame_sm_off) > 0xFFFF or len(sm_blob) > 0xFFFF:
        raise SystemExit("samus animation blob exceeds the u16 offset tables")

    return {
        "chr": bytes(chr_blob),
        "pal": pal,
        "spritemaps": bytes(sm_blob),
        "frame_sm_off": frame_sm_off,
        "pose_first": pose_first,
        "pose_count": pose_count,
        "pose_yoff": pose_yoff,
        "pose_yradius": pose_yradius,
        "bank91": bank91_bytes(bank91, K_SAMUS_BANK91_WINDOW,
                               K_SAMUS_BANK91_WINDOW_LEN),
        "bank91_source": str(bank91_path.relative_to(ROOT)),
        "feet": feet_by_pose,
        "report": report,
    }


def emit_u16(fp, name: str, data: list[int], static: bool = True) -> None:
    prefix = "static " if static else ""
    fp.write(f"{prefix}const uint16_t {name}[{len(data)}] = {{\n")
    for i in range(0, len(data), 8):
        chunk = data[i : i + 8]
        fp.write("  " + ", ".join(f"0x{w:04X}" for w in chunk))
        fp.write(",\n" if i + 8 < len(data) or chunk else "\n")
    fp.write("};\n\n")


def pack_room_collision(room: dict) -> dict:
    """Full-room level words + BTS, flat and row-major, for mini's level_data.

    This is the gameplay half of the room: without it mini has no filesystem
    on-device, falls back to its built-in room, and Samus walks off the gunship
    on a floor that is not there. blockWords carry the collision type in the
    top nibble, so packing them covers collision; BTS carries slope shape.
    """
    width = int(room["widthBlocks"])
    height = int(room["heightBlocks"])
    block_words = room["blockWords"]
    bts_rows = room["bts"]
    collision_rows = room["collision"]
    for name, rows in (("blockWords", block_words), ("bts", bts_rows),
                       ("collision", collision_rows)):
        if len(rows) != height or any(len(r) != width for r in rows):
            raise SystemExit(f"{ROOM_PATH}: {name} is not {height}x{width}")

    level: list[int] = []
    bts: list[int] = []
    for by in range(height):
        for bx in range(width):
            word = int(block_words[by][bx]) & 0xFFFF
            declared = int(collision_rows[by][bx]) & 0xF
            if (word >> K_BLOCK_TYPE_SHIFT) != declared:
                raise SystemExit(
                    f"{ROOM_PATH}: block ({bx},{by}) word {word:#06x} type "
                    f"{word >> K_BLOCK_TYPE_SHIFT:#x} != collision {declared:#x}")
            level.append(word)
            bts.append(int(bts_rows[by][bx]) & 0xFF)

    scroll = room.get("scroll") or {}
    screens = scroll.get("screens") or []
    width_screens = int(room["widthScreens"])
    height_screens = int(room["heightScreens"])
    scrolls: list[int] = []
    for row in screens:
        scrolls.extend(int(v) & 0xFF for v in row)
    if len(scrolls) != width_screens * height_screens:
        raise SystemExit(
            f"{ROOM_PATH}: scroll.screens is {len(scrolls)} entries, expected "
            f"{width_screens * height_screens}")

    return {
        "width": width,
        "height": height,
        "width_screens": width_screens,
        "height_screens": height_screens,
        "level": level,
        "bts": bts,
        "scrolls": scrolls,
        "up_scroller": int(scroll.get("upScroller", 112)),
        "down_scroller": int(scroll.get("downScroller", 160)),
        "bg_scrolling": int(scroll.get("bgScrolling", 0)),
    }


def check_room_collision(rc: dict, spawn_x: int, spawn_y: int) -> list[str]:
    """Re-prove the measured facts the physics fix rests on. Empty == good."""
    width = rc["width"]
    level = rc["level"]

    def block_type(bx: int, by: int) -> int:
        return (level[by * width + bx] >> K_BLOCK_TYPE_SHIFT) & 0xF

    fails: list[str] = []
    spawn_bx = spawn_x // K_ROOM_BLOCK_PX
    spawn_by = spawn_y // K_ROOM_BLOCK_PX
    if (spawn_bx, spawn_by) != K_ROOM_SPAWN_BLOCK:
        fails.append(f"spawn block ({spawn_bx},{spawn_by}) != {K_ROOM_SPAWN_BLOCK}")
    if block_type(spawn_bx, spawn_by) != K_BLOCK_TYPE_AIR:
        fails.append(f"spawn block ({spawn_bx},{spawn_by}) is not air")

    for by in K_ROOM_DECK_ROWS:
        row_types = {block_type(bx, by) for bx in K_ROOM_DECK_SOLID_COLS}
        if row_types <= {K_BLOCK_TYPE_AIR}:
            fails.append(f"ship deck row {by} has no collision")
    for bx in K_ROOM_DECK_SOLID_COLS:
        if block_type(bx, 71) != K_BLOCK_TYPE_SOLID:
            fails.append(f"ship deck block ({bx},71) is not solid")
    if block_type(spawn_bx, 74) != K_BLOCK_TYPE_AIR:
        fails.append("block under the deck at the spawn column is not air")
    for by in K_ROOM_FLOOR_ROWS:
        if block_type(spawn_bx, by) != K_BLOCK_TYPE_SOLID:
            fails.append(f"terrain floor block ({spawn_bx},{by}) is not solid")
    if block_type(spawn_bx, 76) == K_BLOCK_TYPE_SOLID:
        fails.append("block above the terrain floor at the spawn column is solid")
    return fails


# kPoseParams[0x01] (kPose_01_FaceR_Normal) byte for byte: pose_x_dir,
# movement_type, new_pose_unless_buttons, direction_shots_fired,
# y_offset_to_gfx, field_5, y_radius, field_7 (src/ida_types.h). A 1-byte slip
# in the bank 0x91 extraction moves every one of these, so it is the fingerprint
# that proves the window is aligned before anything is written.
K_SAMUS_POSE1_PARAMS = (8, 0, 255, 2, 6, 0, 21, 0)


def check_samus_bank91(anim: dict) -> list[str]:
    """Prove the packed bank 0x91 window before it overwrites the .inc."""
    fails: list[str] = []
    window = anim["bank91"]
    base = K_SAMUS_BANK91_WINDOW
    end = base + K_SAMUS_BANK91_WINDOW_LEN

    if len(window) != K_SAMUS_BANK91_WINDOW_LEN:
        fails.append(f"window is {len(window)} B, want {K_SAMUS_BANK91_WINDOW_LEN}")
        return fails
    if not any(window):
        fails.append("window is all zero")

    def inside(addr: int, size: int, what: str) -> bool:
        if addr < base or addr + size > end:
            fails.append(f"{what} {addr:#06x}+{size} is outside the packed "
                         f"window {base:#06x}..{end:#06x}")
            return False
        return True

    def win_byte(addr: int) -> int:
        return window[addr - base]

    def win_word(addr: int) -> int:
        return window[addr - base] | (window[addr - base + 1] << 8)

    inside(K_SAMUS_POSE_PARAMS,
           K_SAMUS_POSE_PARAM_SIZE * K_SAMUS_POSE_TABLE_LEN, "kPoseParams")
    inside(K_SAMUS_ANIM_DELAY_TABLE, 2 * K_SAMUS_POSE_TABLE_LEN,
           "kSamusAnimationDelayData")
    inside(K_SAMUS_DEFAULT_ANIM_FRAME_PTR, 2, "kDefaultAnimFramePtr")
    inside(K_SAMUS_SPEEDBOOST_ANIM_FRAME_PTR, 2 * 5, "kSpeedBoostToAnimFramePtr")
    inside(K_SAMUS_SPEEDBOOST_CTR, 2 * 5, "kSpeedBoostToCtr")
    if fails:
        return fails

    got = tuple(window[K_SAMUS_POSE_PARAMS - base
                       + K_SAMUS_POSE_PARAM_SIZE * K_SAMUS_POSE
                       : K_SAMUS_POSE_PARAMS - base
                       + K_SAMUS_POSE_PARAM_SIZE * (K_SAMUS_POSE + 1)])
    if got != K_SAMUS_POSE1_PARAMS:
        fails.append(f"kPoseParams[{K_SAMUS_POSE:#04x}] is {got}, "
                     f"want {K_SAMUS_POSE1_PARAMS}: the bank 0x91 window is "
                     f"misaligned or from the wrong ROM")
        return fails

    yoff = anim["pose_yoff"]
    yrad = anim["pose_yradius"]
    if yoff[K_SAMUS_POSE] != K_SAMUS_Y_OFFSET_TO_GFX:
        fails.append(f"reference y_offset_to_gfx {yoff[K_SAMUS_POSE]} != "
                     f"{K_SAMUS_Y_OFFSET_TO_GFX}")
    if yrad[K_SAMUS_POSE] != K_SAMUS_Y_RADIUS:
        fails.append(f"reference y_radius {yrad[K_SAMUS_POSE]} != "
                     f"{K_SAMUS_Y_RADIUS}")

    # Standing is taller than crouching is taller than a morph ball. If the
    # slice were off these would not order.
    stand, crouch, morph = yrad[0x01], yrad[0x27], yrad[0x1D]
    if not stand > crouch > morph > 0:
        fails.append(f"y_radius stand/crouch/morph {stand}/{crouch}/{morph} "
                     f"is not a decreasing height order")

    # Every pose we animate must have a delay stream inside the window, long
    # enough for the frames we packed: Samus_HandleAnimDelay() indexes it by
    # samus_anim_frame, so a short stream reads someone else's data.
    for pose, name in K_SAMUS_POSES:
        ptr = win_word(K_SAMUS_ANIM_DELAY_TABLE + pose * 2)
        frames = anim["pose_count"][pose]
        if ptr < 0x8000:
            fails.append(f"pose {pose:#04x} ({name}) delay pointer {ptr:#06x} "
                         f"is not a bank 0x91 ROM address")
            continue
        if not inside(ptr, frames, f"pose {pose:#04x} ({name}) delay stream"):
            continue
        stream = window[ptr - base : ptr - base + frames]
        if not any(stream):
            fails.append(f"pose {pose:#04x} ({name}) delay stream is all zero: "
                         f"the animation clock would never advance")
        if yrad[pose] == 0:
            fails.append(f"pose {pose:#04x} ({name}) y_radius is 0")
        if not -128 <= (yoff[pose] - 256 if yoff[pose] > 127 else yoff[pose]) <= 127:
            fails.append(f"pose {pose:#04x} ({name}) y_offset does not fit int8")
    return fails


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-o", "--output", type=Path, default=DEFAULT_OUT)
    parser.add_argument("--samus-output", type=Path, default=DEFAULT_SAMUS_OUT)
    parser.add_argument("--gunship-output", type=Path, default=DEFAULT_GUNSHIP_OUT)
    parser.add_argument("--room-output", type=Path, default=DEFAULT_ROOM_OUT)
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
    json_camera_x = int(camera.get("cameraX", 0))
    json_camera_y = int(camera.get("cameraY", 0))
    # JSON cameraY/16=61 packs pal4 platforms (world by 75-76) at ty 28-31,
    # below the 224-line viewport. Origin += 2 blocks so they land at ty 24-27.
    #
    # The +2 is a FRAMING CHOICE, not vanilla (sm_rev-k5q.5). The principled
    # origin_by from the room geometry is 64, not the 63 this produces, but
    # origin_by moves Samus, the gunship and BG1 together and overlaps
    # sm_rev-k5q.3's work, so the framing is deliberately left as-is and
    # pinned by the origin_by guard below rather than quietly corrected.
    origin_bx = json_camera_x // 16
    origin_by = json_camera_y // 16 + 2
    camera_x = origin_bx * 16
    camera_y = origin_by * 16
    block_words = room["blockWords"]
    bg1 = expand_bg1(block_words, meta, origin_bx, origin_by)
    # BG2 runs at half the BG1 rate horizontally and does not scroll at all
    # vertically, so its origin is (camera_x / 2, K_BG2_VOFS), not the camera.
    bg2_words = bg2_screen(bg2_linear, bg2_path)
    bg2_origin_tx = (camera_x // 2 // 8) % 32
    bg2_origin_ty = (K_BG2_VOFS // 8) % 32
    bg2 = expand_bg2(bg2_words, bg2_origin_tx, bg2_origin_ty)
    anim = pack_samus_anim(room)
    gun_chr, gun_pal, gun_oam, gun_oam_hi = pack_gunship(room, camera_x, camera_y)
    spawn_x = int(camera.get("spawnX", camera_x))
    spawn_y = int(camera.get("spawnY", camera_y))
    room_collision = pack_room_collision(room)

    packed = (
        len(tiles)
        + len(pal) * 2
        + len(bg1) * 2
        + len(bg2) * 2
        + len(anim["chr"])
        + len(anim["pal"]) * 2
        + len(anim["spritemaps"])
        + len(gun_chr)
        + len(gun_pal) * 2
        + len(gun_oam)
        + len(gun_oam_hi)
    )
    unpacked = (
        TILES_PATH.stat().st_size
        + PAL_PATH.stat().st_size
        + META_PATH.stat().st_size
        + bg2_path.stat().st_size
        + room["widthBlocks"] * room["heightBlocks"] * 2
        + len(anim["chr"])
        + len(anim["pal"]) * 2
        + len(anim["spritemaps"])
        + len(gun_chr)
        + len(gun_pal) * 2
    )

    bg1_nonzero = sum(1 for w in bg1 if w)
    bg2_nonzero = sum(1 for w in bg2 if w)
    pal_nonzero = sum(1 for w in pal if w)

    # Validate before writing: these outputs are checked in, and a failed run
    # must not leave them overwritten with the wrong window.
    if (pal_nonzero == 0 or bg2_nonzero == 0 or not any(tiles)
            or not any(anim["chr"]) or not any(gun_chr)):
        print("FAIL: packed arrays look empty", file=sys.stderr)
        return 1
    # Pins the framing choice above: this guard does not validate the camera,
    # it locks the +2 nudge so it cannot drift without someone noticing.
    if origin_by != 63 or camera_y != 1008:
        print(f"FAIL: origin_by {origin_by} camera_y {camera_y} (want 63 / 1008)", file=sys.stderr)
        return 1
    bg_scrolling = int((room.get("scroll") or {}).get("bgScrolling", -1))
    want_scrolling = K_BG2_SCROLL_X | (K_BG2_SCROLL_Y << 8)
    if bg_scrolling != want_scrolling:
        print(
            f"FAIL: scroll.bgScrolling 0x{bg_scrolling:04X}, "
            f"want 0x{want_scrolling:04X}: the half-rate/locked BG2 scroll "
            f"rule no longer matches the room",
            file=sys.stderr,
        )
        return 1
    bg2_stale = sum(1 for w in bg2 if w == K_BG2_STALE_FILL)
    if bg2_stale:
        print(f"FAIL: BG2 carries {bg2_stale} stale VRAM fill words "
              f"0x{K_BG2_STALE_FILL:04X}", file=sys.stderr)
        return 1
    bg2_priority = sum(1 for w in bg2 if w & K_BG2_PRIORITY_BIT)
    if bg2_priority:
        print(f"FAIL: BG2 carries {bg2_priority} words with the "
              f"0x{K_BG2_PRIORITY_BIT:04X} priority bit, which paints over BG1",
              file=sys.stderr)
        return 1
    room_fails = check_room_collision(room_collision, spawn_x, spawn_y)
    if room_fails:
        for message in room_fails:
            print(f"FAIL: room collision: {message}", file=sys.stderr)
        return 1
    bank91_fails = check_samus_bank91(anim)
    if bank91_fails:
        for message in bank91_fails:
            print(f"FAIL: samus bank 0x91: {message}", file=sys.stderr)
        return 1

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", encoding="utf-8") as fp:
        fp.write("/* Generated by tools/pico_pack_ls_assets.py from assets/local_mini.\n")
        fp.write(" * Landing Site (0x91F8) tileset 00 + spawn-window BG1/BG2.\n")
        fp.write(" * Do not embed sm.smc. ROM was not used. Gunship OAM is not packed.\n")
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
        fp.write(f"  kPicoLsOriginBlockY = {origin_by},\n")
        fp.write(f"  kPicoLsBg2ScrollX = 0x{K_BG2_SCROLL_X:02X},\n")
        fp.write(f"  kPicoLsBg2ScrollY = 0x{K_BG2_SCROLL_Y:02X},\n")
        fp.write(f"  kPicoLsBg2Vofs = {K_BG2_VOFS},\n")
        fp.write(f"  kPicoLsSpawnX = {spawn_x},\n")
        fp.write(f"  kPicoLsSpawnY = {spawn_y}\n")
        fp.write("};\n\n")
        emit_u8(fp, "kPicoLsTiles4bpp", tiles)
        emit_u16(fp, "kPicoLsPalette", pal)
        emit_u16(fp, "kPicoLsBg1Tilemap", bg1)
        emit_u16(fp, "kPicoLsBg2Tilemap", bg2)

    args.samus_output.parent.mkdir(parents=True, exist_ok=True)
    with args.samus_output.open("w", encoding="utf-8") as fp:
        fp.write("/* Generated by tools/pico_pack_ls_assets.py from assets/local_mini.\n")
        fp.write(" * Samus stand/run/turn animation frames: CHR + spritemaps + power pal.\n")
        fp.write(" * CHR is one fixed-size block per frame, DMAd to the OBJ name base.\n")
        fp.write(" * kPicoLsSamusPoseYOffset/YRadius are kPoseParams (ROM 0x91B629)\n")
        fp.write(" * verbatim, and kPicoLsSamusBank91 is the $91B000..$91BFFF window\n")
        fp.write(" * that carries kPoseParams + the animation delay data the vanilla\n")
        fp.write(f" * clock chases. Source: {anim['bank91_source']}.\n")
        fp.write(" */\n\n")
        fp.write("enum {\n")
        fp.write(f"  kPicoLsSamusChrFrameSize = {K_SAMUS_CHR_SIZE},\n")
        fp.write(f"  kPicoLsSamusFrameCount = {len(anim['frame_sm_off'])},\n")
        fp.write(f"  kPicoLsSamusPoseCount = {K_SAMUS_POSE_TABLE_LEN},\n")
        fp.write(f"  kPicoLsSamusPalCount = {len(anim['pal'])},\n")
        fp.write(f"  kPicoLsSamusSpritemapSize = {len(anim['spritemaps'])},\n")
        fp.write(f"  kPicoLsSamusYOffsetToGfx = {K_SAMUS_Y_OFFSET_TO_GFX},\n")
        fp.write(f"  kPicoLsSamusYRadius = {K_SAMUS_Y_RADIUS},\n")
        fp.write(f"  kPicoLsSamusBank91Base = 0x{K_SAMUS_BANK91_WINDOW:04X},\n")
        fp.write(f"  kPicoLsSamusBank91Size = {K_SAMUS_BANK91_WINDOW_LEN},\n")
        fp.write("  kPicoLsSamusCgramIndex = 192\n")
        fp.write("};\n\n")
        emit_u8(fp, "kPicoLsSamusChr", anim["chr"])
        emit_u16(fp, "kPicoLsSamusPal", anim["pal"])
        emit_u8(fp, "kPicoLsSamusSpritemaps", anim["spritemaps"])
        emit_u16(fp, "kPicoLsSamusFrameSmOffset", anim["frame_sm_off"])
        emit_u16(fp, "kPicoLsSamusPoseFirstFrame", anim["pose_first"])
        emit_u8(fp, "kPicoLsSamusPoseFrameCount", bytes(anim["pose_count"]))
        emit_u8(fp, "kPicoLsSamusPoseYOffset", bytes(anim["pose_yoff"]))
        emit_u8(fp, "kPicoLsSamusPoseYRadius", bytes(anim["pose_yradius"]))
        emit_u8(fp, "kPicoLsSamusBank91", anim["bank91"], static=False)

    args.gunship_output.parent.mkdir(parents=True, exist_ok=True)
    with args.gunship_output.open("w", encoding="utf-8") as fp:
        fp.write("/* Generated by tools/pico_pack_ls_assets.py from assets/local_mini.\n")
        fp.write(" * Landing Site gunship roomSprites, OAM in packed extract space.\n")
        fp.write(" * Samus stands on this: its collision blocks carry metatile 0xFF\n")
        fp.write(" * (air graphics), so without it the spawn floor is invisible.\n")
        fp.write(" * Do not embed sm.smc. ROM was not used.\n")
        fp.write(" */\n\n")
        emit_u8(fp, "kPicoLsGunshipChr", gun_chr, static=False)
        emit_u16(fp, "kPicoLsGunshipPal", gun_pal, static=False)
        emit_u8(fp, "kPicoLsGunshipOam", gun_oam, static=False)
        emit_u8(fp, "kPicoLsGunshipOamHi", gun_oam_hi, static=False)

    args.room_output.parent.mkdir(parents=True, exist_ok=True)
    with args.room_output.open("w", encoding="utf-8") as fp:
        fp.write("/* Generated by tools/pico_pack_ls_assets.py from assets/local_mini.\n")
        fp.write(" * Landing Site (0x91F8) GAMEPLAY room: level words + BTS + scrolls.\n")
        fp.write(" * Without this mini has no filesystem on-device, falls back to its\n")
        fp.write(" * built-in room, and the gunship deck and terrain have no collision.\n")
        fp.write(" * Do not embed sm.smc. ROM was not used.\n")
        fp.write(" */\n\n")
        fp.write("enum {\n")
        fp.write(f"  kPicoLsRoomIdPacked = {int(room['roomId'])},\n")
        fp.write(f"  kPicoLsRoomWidthBlocks = {room_collision['width']},\n")
        fp.write(f"  kPicoLsRoomHeightBlocks = {room_collision['height']},\n")
        fp.write(f"  kPicoLsRoomWidthScreens = {room_collision['width_screens']},\n")
        fp.write(f"  kPicoLsRoomHeightScreens = {room_collision['height_screens']},\n")
        fp.write(f"  kPicoLsRoomUpScroller = {room_collision['up_scroller']},\n")
        fp.write(f"  kPicoLsRoomDownScroller = {room_collision['down_scroller']},\n")
        fp.write(f"  kPicoLsRoomBgScrolling = {room_collision['bg_scrolling']},\n")
        fp.write(f"  kPicoLsRoomCameraXPacked = {json_camera_x},\n")
        fp.write(f"  kPicoLsRoomCameraYPacked = {json_camera_y},\n")
        fp.write(f"  kPicoLsRoomSpawnXPacked = {spawn_x},\n")
        fp.write(f"  kPicoLsRoomSpawnYPacked = {spawn_y}\n")
        fp.write("};\n\n")
        emit_u16(fp, "kPicoLsRoomLevelData", room_collision["level"])
        emit_u8(fp, "kPicoLsRoomBts", bytes(room_collision["bts"]))
        emit_u8(fp, "kPicoLsRoomScrolls", bytes(room_collision["scrolls"]))

    print(f"wrote {args.output}")
    print(f"wrote {args.samus_output}")
    print(f"wrote {args.gunship_output}")
    print(f"wrote {args.room_output}")
    print(f"bg2 variant {selected.get('key')} from {bg2_path.relative_to(ROOT)}")
    print(
        f"camera {camera_x},{camera_y} origin block {origin_bx},{origin_by} "
        f"bg2 tile {bg2_origin_tx},{bg2_origin_ty} "
        f"(hofs {camera_x // 2}, vofs {K_BG2_VOFS})"
    )
    print(f"packed {packed} unpacked_source {unpacked}")
    print(f"nonzero pal={pal_nonzero} bg1_words={bg1_nonzero} bg2_words={bg2_nonzero}")
    print(
        f"samus frames={len(anim['frame_sm_off'])} chr={len(anim['chr'])} "
        f"spritemaps={len(anim['spritemaps'])} bank91={len(anim['bank91'])} B "
        f"from {anim['bank91_source']} (kPoseParams + animation delay data)"
    )
    for pose, name, n, feet_lo, feet_hi in anim["report"]:
        yo = anim["pose_yoff"][pose]
        yo = yo - 256 if yo > 127 else yo
        kind = "air " if pose in K_SAMUS_AIR_POSES else "grnd"
        print(f"  pose {pose:#04x} {name:<12} {kind} frames={n:2d} "
              f"feet {feet_lo:+d}..{feet_hi:+d} y_off={yo:+d} "
              f"y_radius={anim['pose_yradius'][pose]:2d}")
    print(f"gunship oam={len(gun_oam_hi)} chr={len(gun_chr)} pal={len(gun_pal)} spawn={spawn_x},{spawn_y}")
    print(
        f"room collision {room_collision['width']}x{room_collision['height']} "
        f"level={len(room_collision['level']) * 2} B bts={len(room_collision['bts'])} B "
        f"scrolls={len(room_collision['scrolls'])} B "
        f"spawn block {spawn_x // K_ROOM_BLOCK_PX},{spawn_y // K_ROOM_BLOCK_PX}")
    print("rom used: no")
    return 0


if __name__ == "__main__":
    sys.exit(main())
