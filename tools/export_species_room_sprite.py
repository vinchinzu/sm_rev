#!/usr/bin/env python3
"""Export mini roomSprites tile bins and OAM entries from a Super Metroid ROM."""

from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path

COLORS_PER_PALETTE = 16
TILE_BYTES = 32
MINI_TILE_BASE = 0x100


def snes_to_pc(snes_address: int) -> int:
    bank = (snes_address >> 16) & 0xFF
    addr = snes_address & 0xFFFF
    return ((bank & 0x7F) * 0x8000) + (addr & 0x7FFF)


def read_u16(rom: bytes, pc: int) -> int:
    return rom[pc] | (rom[pc + 1] << 8)


def read_u8(rom: bytes, pc: int) -> int:
    return rom[pc]


def parse_species_header(rom: bytes, species_id: int) -> dict[str, int] | None:
    pc = snes_to_pc(0xA00000 | species_id)
    if pc + 64 > len(rom):
        return None
    tile_data_size_raw = read_u16(rom, pc)
    pal_ptr = read_u16(rom, pc + 2)
    ai_bank = read_u8(rom, pc + 0x0C)
    gfx_offset = read_u16(rom, pc + 0x36)
    gfx_bank = read_u8(rom, pc + 0x38)
    return {
        "tile_data_size": tile_data_size_raw & 0x7FFF,
        "pal_ptr": pal_ptr,
        "ai_bank": ai_bank,
        "gfx_offset": gfx_offset,
        "gfx_bank": gfx_bank,
    }


def read_enemy_palette_words(rom: bytes, species_id: int) -> list[int]:
    header = parse_species_header(rom, species_id)
    if header is None:
        raise ValueError(f"species ${species_id:04X} header missing")
    pal_snes = (header["ai_bank"] << 16) | header["pal_ptr"]
    pal_pc = snes_to_pc(pal_snes)
    words = [0]
    for index in range(1, COLORS_PER_PALETTE):
        words.append(read_u16(rom, pal_pc + index * 2))
    return words


def load_enemy_tile_data(rom: bytes, species_id: int) -> bytes:
    header = parse_species_header(rom, species_id)
    if header is None or header["tile_data_size"] <= 0:
        raise ValueError(f"species ${species_id:04X} has no tile data")
    gfx_snes = (header["gfx_bank"] << 16) | header["gfx_offset"]
    gfx_pc = snes_to_pc(gfx_snes)
    size = header["tile_data_size"]
    if gfx_pc + size > len(rom):
        raise ValueError(f"species ${species_id:04X} tile data out of range")
    return rom[gfx_pc : gfx_pc + size]


def grid_oam_entries(tile_count: int, cols: int, palette_row: int) -> list[dict]:
    rows = (tile_count + cols - 1) // cols
    entries: list[dict] = []
    tile_index = 0
    for row in range(rows):
        for col in range(cols):
            if tile_index >= tile_count:
                break
            entries.append(
                {
                    "xOffset": col * 8,
                    "yOffset": row * 8,
                    "tileNum": MINI_TILE_BASE + tile_index,
                    "paletteRow": palette_row,
                    "hFlip": False,
                    "vFlip": False,
                    "is16x16": False,
                }
            )
            tile_index += 1
    return entries


def export_species(
    rom: bytes,
    species_id: int,
    output_dir: Path,
    *,
    palette_row: int = 5,
    cols: int = 4,
) -> dict:
    tile_data = load_enemy_tile_data(rom, species_id)
    palette_words = read_enemy_palette_words(rom, species_id)
    tile_count = len(tile_data) // TILE_BYTES
    species_hex = f"{species_id:04X}"
    tiles_name = f"species_{species_hex}_tiles4bpp.bin"
    palette_name = f"species_{species_hex}_palette.bin"
    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / tiles_name).write_bytes(tile_data)
    (output_dir / palette_name).write_bytes(
        struct.pack("<" + "H" * len(palette_words), *palette_words)
    )
    return {
        "speciesId": species_id,
        "speciesIdHex": f"0x{species_hex}",
        "tileDataPath": f"room_sprites/{tiles_name}",
        "palettePath": f"room_sprites/{palette_name}",
        "entries": grid_oam_entries(tile_count, cols, palette_row),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("species_id", type=lambda value: int(value, 0))
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("assets/local_mini/room_sprites"),
    )
    parser.add_argument("--palette-row", type=int, default=5)
    parser.add_argument("--cols", type=int, default=4)
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    payload = export_species(
        rom,
        args.species_id,
        args.output_dir,
        palette_row=args.palette_row,
        cols=args.cols,
    )
    json.dump(payload, sys.stdout, indent=2)
    sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
