#!/usr/bin/env python3
"""Bundle mini editor room assets from a Super Metroid ROM.

Writes tileset bins, optional BG2 tilemap, and merges a full mini room JSON
with blockWords + asset metadata (matching the Landing Site export shape).
"""

from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path

TILESET_TABLE_SNES = 0x8FE6A2
CRE_TILE_TABLE_SNES = 0xB9A09D
CRE_GFX_SNES = 0xB98000
METATILE_COUNT = 1024
BYTES_PER_TILE = 32
TOTAL_TILES = 1024
CRE_GFX_OFFSET = 0x5000
TILES4BPP_SIZE = 1024 * 32
METATILE_WORD_COUNT = METATILE_COUNT * 4
PALETTE_WORD_COUNT = 8 * 16
BG2_WORD_COUNT = 64 * 32
BANK_ROOM_DATA = 0x8F0000
BANK_ENEMY_POPULATION = 0xA10000
WRAM_SIZE = 0x20000
BG2_VRAM_WORD_BASE = 0x4800
BG2_CLEAR_WORD = 0x2C0F
ROOM_STATE_SELECT_FINISH = 0xE5E6
ROOM_STATE_SELECT_EVENT = 0xE612
ROOM_STATE_SELECT_BOSS = 0xE629
ROOM_STATE_SELECT_TOURIAN_BOSS = 0xE5FF
ROOM_STATE_SELECT_MORPH_MISSILES = 0xE652
ROOM_STATE_SELECT_POWER_BOMBS = 0xE669
CLIMB_ROOM_ID = 0x96BA
CLIMB_AWAKE_EVENT = 0
ENEMY_ENTRY_WORD_COUNT = 8
ENEMY_TERMINATOR = 0xFFFF
ENEMY_SPRITE_TILE_BASE = 0x100
ENEMY_PALETTE_WORD_COUNT = 16


def snes_to_pc(rom: bytes, snes_address: int) -> int:
    rom_start = 0x200 if len(rom) % 0x8000 == 0x200 else 0
    bank = (snes_address >> 16) & 0xFF
    addr = snes_address & 0xFFFF
    return rom_start + ((bank & 0x7F) * 0x8000) + (addr & 0x7FFF)


def read_u16(rom: bytes, pc: int) -> int:
    return rom[pc] | (rom[pc + 1] << 8)


def read_u24(rom: bytes, pc: int) -> int:
    return rom[pc] | (rom[pc + 1] << 8) | (rom[pc + 2] << 16)


def decompress_lz5(rom: bytes, start_pc: int) -> bytes:
    dst = bytearray()
    pos = start_pc
    while pos < len(rom):
        cmd = rom[pos]
        pos += 1
        if cmd == 0xFF:
            break
        top_bits = (cmd >> 5) & 7
        if top_bits == 7:
            cmd_code = (cmd >> 2) & 7
            length = ((cmd & 0x03) << 8 | rom[pos]) + 1
            pos += 1
        else:
            cmd_code = top_bits
            length = (cmd & 0x1F) + 1
        if cmd_code == 0:
            dst.extend(rom[pos : pos + length])
            pos += length
        elif cmd_code == 1:
            fill = rom[pos]
            pos += 1
            dst.extend(bytes([fill]) * length)
        elif cmd_code == 2:
            b1, b2 = rom[pos], rom[pos + 1]
            pos += 2
            for i in range(length):
                dst.append(b1 if i % 2 == 0 else b2)
        elif cmd_code == 3:
            base = rom[pos]
            pos += 1
            for i in range(length):
                dst.append((base + i) & 0xFF)
        elif cmd_code in (4, 5):
            addr = rom[pos] | (rom[pos + 1] << 8)
            pos += 2
            xor = 0xFF if cmd_code == 5 else 0
            for i in range(length):
                src_index = addr + i
                value = dst[src_index] if src_index < len(dst) else 0
                dst.append(value ^ xor)
        elif cmd_code in (6, 7):
            rel = rom[pos]
            pos += 1
            xor = 0xFF if cmd_code == 7 else 0
            src_anchor = len(dst) - rel
            for i in range(length):
                if rel > 0:
                    src_index = src_anchor + (i % rel)
                else:
                    src_index = src_anchor
                value = dst[src_index] if 0 <= src_index < len(dst) else 0
                dst.append(value ^ xor)
    return bytes(dst)


def parse_tile_table_raw(data: bytes) -> list[list[int]]:
    metatiles: list[list[int]] = [[0, 0, 0, 0] for _ in range(METATILE_COUNT)]
    entry_count = min(len(data) // 8, METATILE_COUNT)
    for i in range(entry_count):
        off = i * 8
        words = [
            data[off] | (data[off + 1] << 8),
            data[off + 2] | (data[off + 3] << 8),
            data[off + 4] | (data[off + 5] << 8),
            data[off + 6] | (data[off + 7] << 8),
        ]
        metatiles[i] = words
    return metatiles


def parse_tile_table(var_table: bytes, cre_table: bytes) -> list[list[int]]:
    combined = cre_table + var_table
    return parse_tile_table_raw(combined)


def load_tileset_bundle(rom: bytes, tileset_id: int) -> tuple[bytes, list[int], bytes]:
    entry_pc = snes_to_pc(rom, TILESET_TABLE_SNES) + tileset_id * 9
    tile_table_ptr = read_u24(rom, entry_pc)
    gfx_ptr = read_u24(rom, entry_pc + 3)
    palette_ptr = read_u24(rom, entry_pc + 6)

    var_tile_table = decompress_lz5(rom, snes_to_pc(rom, tile_table_ptr))
    var_gfx = decompress_lz5(rom, snes_to_pc(rom, gfx_ptr))
    palette = decompress_lz5(rom, snes_to_pc(rom, palette_ptr))

    if len(var_tile_table) >= METATILE_COUNT * 8:
        metatiles = parse_tile_table_raw(var_tile_table)
    else:
        cre_tile_table = decompress_lz5(rom, snes_to_pc(rom, CRE_TILE_TABLE_SNES))
        metatiles = parse_tile_table(var_tile_table, cre_tile_table)

    combined_gfx = bytearray(TOTAL_TILES * BYTES_PER_TILE)
    combined_gfx[: min(len(var_gfx), len(combined_gfx))] = var_gfx[
        : min(len(var_gfx), len(combined_gfx))
    ]
    if tileset_id != 27:
        cre_gfx = decompress_lz5(rom, snes_to_pc(rom, CRE_GFX_SNES))
        copy_len = min(len(cre_gfx), len(combined_gfx) - CRE_GFX_OFFSET)
        if copy_len > 0:
            combined_gfx[CRE_GFX_OFFSET : CRE_GFX_OFFSET + copy_len] = cre_gfx[:copy_len]

    flat_metatile_words: list[int] = []
    for meta in metatiles:
        flat_metatile_words.extend(meta)

    palette_words = [
        palette[i] | (palette[i + 1] << 8) for i in range(0, min(len(palette), 256), 2)
    ]
    while len(palette_words) < PALETTE_WORD_COUNT:
        palette_words.append(0)

    tiles4bpp = bytes(combined_gfx[:TILES4BPP_SIZE]).ljust(TILES4BPP_SIZE, b"\x00")
    return tiles4bpp, flat_metatile_words[:METATILE_WORD_COUNT], bytes(
        struct.pack("<" + "H" * PALETTE_WORD_COUNT, *palette_words[:PALETTE_WORD_COUNT])
    )


def select_room_state_ptr(
    rom: bytes,
    room_id: int,
    event_ids: set[int] | None = None,
) -> int:
    events = event_ids or set()
    pos = snes_to_pc(rom, BANK_ROOM_DATA | room_id) + 11
    end = min(pos + 200, len(rom) - 1)
    while pos < end:
        code = read_u16(rom, pos)
        pos += 2
        if code == ROOM_STATE_SELECT_FINISH:
            return read_u16(rom, pos)
        if code == ROOM_STATE_SELECT_EVENT:
            event_id = rom[pos]
            state_ptr = read_u16(rom, pos + 1)
            if event_id in events:
                return state_ptr
            pos += 3
            continue
        if code == ROOM_STATE_SELECT_BOSS:
            pos += 3
            continue
        if code in (
            ROOM_STATE_SELECT_TOURIAN_BOSS,
            ROOM_STATE_SELECT_MORPH_MISSILES,
            ROOM_STATE_SELECT_POWER_BOMBS,
        ):
            pos += 2
            continue
        raise RuntimeError(
            f"unsupported room state selector 0x{code:04X} for room 0x{room_id:04X}"
        )
    raise RuntimeError(f"room state not found for 0x{room_id:04X}")


def read_room_state(
    rom: bytes,
    room_id: int,
    event_ids: set[int] | None = None,
) -> dict[str, int]:
    state_ptr = select_room_state_ptr(rom, room_id, event_ids)
    state_pc = snes_to_pc(rom, BANK_ROOM_DATA | state_ptr)
    return {
        "state_ptr": state_ptr,
        "level_ptr": read_u24(rom, state_pc),
        "tileset": rom[state_pc + 3],
        "bg_scrolling": read_u16(rom, state_pc + 12),
        "scroll_ptr": read_u16(rom, state_pc + 14),
        "bg_data_ptr": read_u16(rom, state_pc + 22),
        "enemy_population_ptr": read_u16(rom, state_pc + 8),
    }


def read_room_header(rom: bytes, room_id: int) -> dict[str, int]:
    pc = snes_to_pc(rom, BANK_ROOM_DATA | room_id)
    return {
        "up_scroller": rom[pc + 6],
        "down_scroller": rom[pc + 7],
        "width_screens": rom[pc + 4],
        "height_screens": rom[pc + 5],
        "cre_bitflag": rom[pc + 8],
    }


def parse_block_words(
    rom: bytes, level_ptr: int, width_blocks: int, height_blocks: int
) -> list[list[int]]:
    level = decompress_lz5(rom, snes_to_pc(rom, level_ptr))
    layer1_size = level[0] | (level[1] << 8)
    tile_start = 2
    total = width_blocks * height_blocks
    expected = total * 2
    if layer1_size < expected:
        raise RuntimeError(
            f"layer1 size {layer1_size} smaller than expected {expected}"
        )
    words: list[int] = []
    for i in range(total):
        off = tile_start + i * 2
        words.append(level[off] | (level[off + 1] << 8))
    rows: list[list[int]] = []
    for y in range(height_blocks):
        row: list[int] = []
        for x in range(width_blocks):
            row.append(words[y * width_blocks + x])
        rows.append(row)
    return rows


def read_scroll_grid(
    rom: bytes, scroll_ptr: int, width_screens: int, height_screens: int
) -> list[list[int]]:
    pc = snes_to_pc(rom, BANK_ROOM_DATA | scroll_ptr)
    rows: list[list[int]] = []
    pos = pc
    for _y in range(height_screens):
        row: list[int] = []
        for _x in range(width_screens):
            row.append(rom[pos])
            pos += 1
        rows.append(row)
    return rows


def read_dma_source(rom: bytes, wram: bytearray, src: int, size: int) -> bytes:
    bank = (src >> 16) & 0xFF
    addr = src & 0xFFFF
    if bank in (0x7E, 0x7F):
        offset = addr + (0x10000 if bank == 0x7F else 0)
        if offset >= len(wram):
            return bytes(size)
        return bytes(wram[offset : offset + size]).ljust(size, b"\x00")
    pc = snes_to_pc(rom, src)
    return rom[pc : pc + size].ljust(size, b"\x00")


def write_bg2_vram(bg2_bytes: bytearray, vram_word_dst: int, payload: bytes) -> bool:
    word_offset = vram_word_dst - BG2_VRAM_WORD_BASE
    if word_offset < 0 or word_offset >= BG2_WORD_COUNT:
        return False
    byte_offset = word_offset * 2
    copy_len = min(len(payload), len(bg2_bytes) - byte_offset)
    if copy_len > 0:
        bg2_bytes[byte_offset : byte_offset + copy_len] = payload[:copy_len]
    return copy_len > 0


def load_bg2_tilemap(rom: bytes, bg_data_ptr: int) -> list[int] | None:
    if not (bg_data_ptr & 0x8000):
        return None
    pc = snes_to_pc(rom, BANK_ROOM_DATA | bg_data_ptr)
    wram = bytearray(WRAM_SIZE)
    bg2_bytes = bytearray(struct.pack("<" + "H" * BG2_WORD_COUNT, *([BG2_CLEAR_WORD] * BG2_WORD_COUNT)))
    touched_bg2 = False
    while pc:
        cmd = read_u16(rom, pc)
        if cmd == 0:
            break
        if cmd == 2 or cmd == 8:
            payload = pc + 2
            src = read_u16(rom, payload) | (rom[payload + 2] << 16)
            vram_dst = read_u16(rom, payload + 3)
            size = read_u16(rom, payload + 5)
            dma_payload = read_dma_source(rom, wram, src, size)
            touched_bg2 = write_bg2_vram(bg2_bytes, vram_dst, dma_payload) or touched_bg2
            pc = payload + 7
            continue
        if cmd == 4:
            payload = pc + 2
            src = read_u24(rom, payload)
            dst = read_u16(rom, payload + 3)
            decomp = decompress_lz5(rom, snes_to_pc(rom, src))
            if dst < len(wram):
                copy_len = min(len(decomp), len(wram) - dst)
                wram[dst : dst + copy_len] = decomp[:copy_len]
            pc = payload + 5
            continue
        if cmd == 10 or cmd == 12:
            bg2_bytes[:] = struct.pack("<" + "H" * BG2_WORD_COUNT, *([BG2_CLEAR_WORD] * BG2_WORD_COUNT))
            touched_bg2 = True
            pc += 2
            continue
        if cmd == 6:
            pc += 2
            continue
        if cmd == 14:
            pc += 2 + 9
            continue
        pc += 2
    if not touched_bg2:
        return None
    words = [
        bg2_bytes[i] | (bg2_bytes[i + 1] << 8)
        for i in range(0, len(bg2_bytes), 2)
    ]
    return words[:BG2_WORD_COUNT]


def write_word_bin(path: Path, words: list[int]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(struct.pack("<" + "H" * len(words), *words))


def load_landing_samus_template(local_mini: Path) -> dict:
    landing = json.loads((local_mini / "room_91F8.json").read_text())
    return landing["samusAssets"]


def enemy_name(species_id: int) -> str:
    return {
        0xD87F: "Roach",
        0xF353: "Space Pirate",
    }.get(species_id, f"Enemy 0x{species_id:04X}")


def parse_enemy_header(rom: bytes, species_id: int) -> dict[str, int]:
    pc = snes_to_pc(rom, 0xA00000 | species_id)
    if pc + 64 > len(rom):
        raise RuntimeError(f"enemy definition 0x{species_id:04X} out of ROM range")
    return {
        "tile_data_size": read_u16(rom, pc) & 0x7FFF,
        "palette_ptr": read_u16(rom, pc + 2),
        "ai_bank": rom[pc + 0x0C],
        "gfx_offset": read_u16(rom, pc + 0x36),
        "gfx_bank": rom[pc + 0x38],
    }


def read_enemy_palette_words(rom: bytes, species_id: int) -> list[int]:
    header = parse_enemy_header(rom, species_id)
    pc = snes_to_pc(rom, (header["ai_bank"] << 16) | header["palette_ptr"])
    words = [0]
    for i in range(1, ENEMY_PALETTE_WORD_COUNT):
        words.append(read_u16(rom, pc + i * 2))
    return words


def read_enemy_tile_data(rom: bytes, species_id: int) -> bytes:
    header = parse_enemy_header(rom, species_id)
    pc = snes_to_pc(rom, (header["gfx_bank"] << 16) | header["gfx_offset"])
    size = header["tile_data_size"]
    if size <= 0 or pc + size > len(rom):
        raise RuntimeError(f"enemy definition 0x{species_id:04X} has invalid tile data")
    return rom[pc : pc + size]


def grid_oam_entries(tile_count: int, cols: int = 4) -> list[dict]:
    entries: list[dict] = []
    for tile_index in range(tile_count):
        entries.append(
            {
                "xOffset": (tile_index % cols) * 8,
                "yOffset": (tile_index // cols) * 8,
                "tileNum": ENEMY_SPRITE_TILE_BASE + tile_index,
                "paletteRow": 7,
                "hFlip": False,
                "vFlip": False,
                "is16x16": False,
            }
        )
    return entries


def compact_space_pirate_oam_entries() -> list[dict]:
    offsets = [
        (0, 0), (8, 0), (16, 0), (24, 0),
        (0, 8), (8, 8), (16, 8), (24, 8),
        (0, 16), (8, 16), (16, 16), (24, 16),
        (0, 24), (8, 24), (16, 24), (24, 24), (32, 24),
        (0, 32), (8, 32), (16, 32), (24, 32), (32, 32),
        (8, 40), (24, 40), (32, 40),
        (0, 48), (8, 48),
        (0, 56), (8, 56),
    ]
    return [
        {
            "xOffset": x,
            "yOffset": y,
            "tileNum": ENEMY_SPRITE_TILE_BASE + i,
            "paletteRow": 7,
            "hFlip": False,
            "vFlip": False,
            "is16x16": False,
        }
        for i, (x, y) in enumerate(offsets)
    ]


def sprite_origin_delta(entries: list[dict]) -> tuple[int, int]:
    if not entries:
        return 0, 0
    min_x = max_x = int(entries[0]["xOffset"])
    min_y = max_y = int(entries[0]["yOffset"])
    for entry in entries:
        x = int(entry["xOffset"])
        y = int(entry["yOffset"])
        size = 16 if entry["is16x16"] else 8
        min_x = min(min_x, x)
        min_y = min(min_y, y)
        max_x = max(max_x, x + size)
        max_y = max(max_y, y + size)
    return (min_x + max_x) // 2, (min_y + max_y) // 2


def export_enemy_species_assets(
    rom: bytes,
    species_id: int,
    output_dir: Path,
) -> dict:
    species_hex = f"{species_id:04X}"
    room_sprite_dir = output_dir / "room_sprites"
    room_sprite_dir.mkdir(parents=True, exist_ok=True)
    tiles_name = f"species_{species_hex}_tiles4bpp.bin"
    palette_name = f"species_{species_hex}_palette.bin"
    tile_data = read_enemy_tile_data(rom, species_id)
    palette_words = read_enemy_palette_words(rom, species_id)
    (room_sprite_dir / tiles_name).write_bytes(tile_data)
    write_word_bin(room_sprite_dir / palette_name, palette_words)
    entries = (
        compact_space_pirate_oam_entries()
        if species_id == 0xF353
        else grid_oam_entries(len(tile_data) // BYTES_PER_TILE)
    )
    origin_dx, origin_dy = sprite_origin_delta(entries)
    return {
        "tileDataPath": f"room_sprites/{tiles_name}",
        "palettePath": f"room_sprites/{palette_name}",
        "entries": entries,
        "originDx": origin_dx,
        "originDy": origin_dy,
    }


def read_enemy_population(rom: bytes, population_ptr: int) -> list[dict]:
    enemies: list[dict] = []
    pc = snes_to_pc(rom, BANK_ENEMY_POPULATION | population_ptr)
    for _ in range(64):
        words = [read_u16(rom, pc + i * 2) for i in range(ENEMY_ENTRY_WORD_COUNT)]
        species_id = words[0]
        if species_id == ENEMY_TERMINATOR:
            return enemies
        pixel_x = words[1]
        pixel_y = words[2]
        enemies.append(
            {
                "id": species_id,
                "idHex": f"0x{species_id:04X}",
                "name": enemy_name(species_id),
                "pixelX": pixel_x,
                "pixelY": pixel_y,
                "blockX": pixel_x // 16,
                "blockY": pixel_y // 16,
                "initParam": words[3],
                "properties1": words[4],
                "properties2": words[5],
                "extraParam1": words[6],
                "extraParam2": words[7],
            }
        )
        pc += ENEMY_ENTRY_WORD_COUNT * 2
    raise RuntimeError(f"enemy population 0x{population_ptr:04X} did not terminate")


def room_sprites_for_enemy_population(
    rom: bytes,
    room_id: int,
    enemies: list[dict],
    output_dir: Path,
) -> list[dict]:
    species_assets: dict[int, dict] = {}
    sprites: list[dict] = []
    for i, enemy in enumerate(enemies):
        species_id = int(enemy["id"])
        if species_id not in species_assets:
            species_assets[species_id] = export_enemy_species_assets(rom, species_id, output_dir)
        assets = species_assets[species_id]
        origin_dx = int(assets["originDx"])
        origin_dy = int(assets["originDy"])
        species_hex = f"{species_id:04X}"
        sprites.append(
            {
                "key": f"room_{room_id:04X}_enemy_{i:02d}_{species_hex}",
                "label": enemy["name"],
                "speciesId": species_id,
                "speciesIdHex": f"0x{species_hex}",
                "pixelX": int(enemy["pixelX"]) - origin_dx,
                "pixelY": int(enemy["pixelY"]) - origin_dy,
                "tileDataPath": assets["tileDataPath"],
                "palettePath": assets["palettePath"],
                "entries": assets["entries"],
            }
        )
    return sprites


def bundle_room(
    rom_path: Path,
    room_id: int,
    collision_json: Path,
    output_dir: Path,
    event_ids: set[int] | None = None,
) -> Path:
    rom = rom_path.read_bytes()
    base = json.loads(collision_json.read_text())
    header = read_room_header(rom, room_id)
    state = read_room_state(rom, room_id, event_ids)
    width_blocks = int(base["widthBlocks"])
    height_blocks = int(base["heightBlocks"])
    width_screens = int(base["widthScreens"])
    height_screens = int(base["heightScreens"])
    tileset_id = int(state["tileset"])

    block_words = parse_block_words(rom, state["level_ptr"], width_blocks, height_blocks)
    tiles4bpp, metatile_words, palette_bin = load_tileset_bundle(rom, tileset_id)
    enemy_spawns = read_enemy_population(rom, state["enemy_population_ptr"])
    room_sprites = room_sprites_for_enemy_population(rom, room_id, enemy_spawns, output_dir)

    tileset_tag = f"{tileset_id:02d}"
    tileset_dir = output_dir / "tilesets"
    tileset_dir.mkdir(parents=True, exist_ok=True)
    (tileset_dir / f"tileset_{tileset_tag}_tiles4bpp.bin").write_bytes(tiles4bpp)
    write_word_bin(tileset_dir / f"tileset_{tileset_tag}_metatile_words.bin", metatile_words)
    (tileset_dir / f"tileset_{tileset_tag}_palette.bin").write_bytes(palette_bin)

    bg_variant_key = "default"
    bg_rel = f"backgrounds/bg2_{room_id:04X}_{bg_variant_key}.bin"
    bg_words = load_bg2_tilemap(rom, state["bg_data_ptr"])
    if bg_words is not None:
        write_word_bin(output_dir / bg_rel, bg_words)

    handle = base.get("handle", f"room_{room_id:04X}")
    room_name = base.get("name", f"Room 0x{room_id:04X}")
    merged = dict(base)
    merged.update(
        {
            "roomId": room_id,
            "roomIdHex": f"0x{room_id:04X}",
            "handle": handle,
            "name": room_name,
            "tileset": tileset_id,
            "blockWords": block_words,
            "roomState": {
                "statePtr": state["state_ptr"],
                "statePtrHex": f"0x{state['state_ptr']:04X}",
                "eventIds": sorted(event_ids or []),
                "enemyPopulationPtr": state["enemy_population_ptr"],
                "enemyPopulationPtrHex": f"0x{state['enemy_population_ptr']:04X}",
            },
            "tilesetMeta": {
                "id": tileset_id,
                "creBitflag": header["cre_bitflag"],
                "assets": {
                    "tiles4bppPath": f"tilesets/tileset_{tileset_tag}_tiles4bpp.bin",
                    "metatileWordsPath": (
                        f"tilesets/tileset_{tileset_tag}_metatile_words.bin"
                    ),
                    "palettePath": f"tilesets/tileset_{tileset_tag}_palette.bin",
                },
            },
            "scroll": {
                "roomScrollsPtr": state["scroll_ptr"],
                "upScroller": header["up_scroller"],
                "downScroller": header["down_scroller"],
                "bgScrolling": state["bg_scrolling"],
                "screens": read_scroll_grid(
                    rom, state["scroll_ptr"], width_screens, height_screens
                ),
            },
            "camera": {
                "spawnX": 384,
                "spawnY": 2192,
                "cameraX": 256,
                "cameraY": 2032,
                "source": "mini_climb_endless_default",
            },
            "samusAssets": load_landing_samus_template(output_dir),
            "enemies": enemy_spawns,
            "roomSprites": room_sprites,
        }
    )
    if bg_words is not None:
        merged["backgroundAssets"] = {
            "defaultVariantKey": bg_variant_key,
            "variants": [
                {
                    "key": bg_variant_key,
                    "tilemapWordsPath": bg_rel,
                }
            ],
        }

    out_json = output_dir / f"room_{room_id:04X}.json"
    out_json.write_text(json.dumps(merged, indent=2))
    return out_json


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", type=Path, default=Path("sm.smc"))
    parser.add_argument("--room", type=lambda x: int(x, 0), default=0x96BA)
    parser.add_argument(
        "--collision-json",
        type=Path,
        default=Path("assets/local_mini/room_96BA.json"),
    )
    parser.add_argument("--output", type=Path, default=Path("assets/local_mini"))
    parser.add_argument(
        "--event",
        type=lambda x: int(x, 0),
        action="append",
        dest="events",
        help="Room event id to treat as set while selecting the room state.",
    )
    args = parser.parse_args()
    event_ids = set(args.events or [])
    if args.events is None and args.room == CLIMB_ROOM_ID:
        event_ids.add(CLIMB_AWAKE_EVENT)
    out = bundle_room(args.rom, args.room, args.collision_json, args.output, event_ids)
    print(out)


if __name__ == "__main__":
    main()
