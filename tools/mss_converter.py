#!/usr/bin/env python3
"""Convert stable-retro/snes9x .state files to MiniSaveState .mss format.

This converter creates "corresponding start" .mss files by overlaying WRAM data
from emulator savestates onto a MiniSaveState template. The resulting .mss file
allows Mini to start from the same WRAM state as the emulator.

IMPORTANT: This creates corresponding-start states (same WRAM bytes), NOT
TAS-correct Mini states. Mini physics may diverge from emulator physics.
"""

from __future__ import annotations

import argparse
import struct
import subprocess
import sys
from pathlib import Path
from typing import Any

# Import the existing .state parser
import sys
sys.path.insert(0, str(Path(__file__).parent))
from savestate_tool import parse_s9x_blocks, WRAM_SIZE


# MiniStateSnapshot binary layout (from src/mini/mini_game.c)
# typedef struct MiniStateSnapshot {
#   uint32 magic;                              // offset 0, 4 bytes
#   uint32 version;                            // offset 4, 4 bytes
#   MiniGameState game;                        // offset 8, variable size
#   MiniStubsSnapshot stubs;                   // variable offset
#   MiniPpuSnapshot ppu;                       // variable offset
#   uint8 ram[kMiniRamSnapshotSize];          // variable offset, 0x20000 bytes
#   uint8 sram[kMiniSramSnapshotSize];        // variable offset, 0x2000 bytes
#   bool use_my_apu_code;                      // variable offset
#   bool host_debug_flag;                      // variable offset
#   int snes_frame_counter;                    // variable offset
#   uint16 installed_bug_fix_counter;         // variable offset
# } MiniStateSnapshot;

MINI_SNAPSHOT_MAGIC = 0x4D53534D  # "MSSM"
MINI_SNAPSHOT_VERSION = 4
MINI_RAM_SIZE = 0x20000
MINI_SRAM_SIZE = 0x2000

# Key WRAM addresses that must match between emulator and Mini for "corresponding start"
CORRESPONDING_WRAM_ADDRS = {
    "samus_x": 0x0AF6,
    "samus_x_sub": 0x0AF8,
    "samus_y": 0x0AFA,
    "samus_y_sub": 0x0AFC,
    "pose": 0x0A1C,
    "room_id": 0x079B,
    "energy": 0x09C2,
}


def read_u16_le(data: bytes | bytearray, offset: int) -> int:
    """Read uint16 little-endian from buffer."""
    return struct.unpack_from("<H", data, offset)[0]


def read_u32_le(data: bytes | bytearray, offset: int) -> int:
    """Read uint32 little-endian from buffer."""
    return struct.unpack_from("<I", data, offset)[0]


def find_ram_offset_in_mss(mss_data: bytes) -> int | None:
    """Find the offset of the ram[] array in a MiniStateSnapshot binary.
    
    The ram[] array is 0x20000 bytes and comes after the variable-size structs.
    We search for it by looking for patterns or by calculating the offset from
    known structure sizes.
    
    For now, we use a heuristic: scan through the file looking for a plausible
    RAM section. The RAM section should be 0x20000 bytes followed by SRAM.
    """
    if len(mss_data) < MINI_RAM_SIZE + MINI_SRAM_SIZE + 100:
        return None
    
    # Verify magic and version
    if read_u32_le(mss_data, 0) != MINI_SNAPSHOT_MAGIC:
        return None
    if read_u32_le(mss_data, 4) != MINI_SNAPSHOT_VERSION:
        return None
    
    # The ram[] array is near the end: it's followed by sram[], then a few trailing fields.
    # Total trailing size after ram[]: sram[0x2000] + use_my_apu_code(1) + host_debug_flag(1)
    #                                   + snes_frame_counter(4) + installed_bug_fix_counter(2)
    #                                   (plus padding for alignment)
    # Let's estimate: 0x2000 + 8 bytes padding = 0x2008
    expected_ram_offset = len(mss_data) - MINI_RAM_SIZE - MINI_SRAM_SIZE - 16
    
    # Scan backwards from expected position (allow ±1KB tolerance for struct padding)
    for offset in range(expected_ram_offset - 1024, expected_ram_offset + 1024):
        if offset < 0 or offset + MINI_RAM_SIZE > len(mss_data):
            continue
        
        # Check if this looks like a valid RAM offset by verifying trailing SRAM exists
        sram_offset = offset + MINI_RAM_SIZE
        if sram_offset + MINI_SRAM_SIZE <= len(mss_data):
            return offset
    
    return None


def overlay_wram_onto_mss(mss_data: bytearray, wram: bytes) -> None:
    """Overlay WRAM bytes onto a MiniSaveState snapshot.
    
    Args:
        mss_data: MiniStateSnapshot binary (modified in-place)
        wram: Source WRAM bytes (0x20000 bytes)
    """
    if len(wram) != MINI_RAM_SIZE:
        raise ValueError(f"WRAM must be {MINI_RAM_SIZE} bytes, got {len(wram)}")
    
    ram_offset = find_ram_offset_in_mss(mss_data)
    if ram_offset is None:
        raise ValueError("Could not find ram[] offset in MiniStateSnapshot")
    
    # Overlay the entire WRAM
    mss_data[ram_offset:ram_offset + MINI_RAM_SIZE] = wram


def create_mss_template(repo_root: Path) -> bytes:
    """Create a blank MiniSaveState template by running generate_mss_fixture.
    
    Returns the raw .mss bytes.
    """
    import tempfile
    
    # Build the fixture generator if needed
    fixture_bin = repo_root / "generate_mss_fixture"
    if not fixture_bin.exists():
        print("Building generate_mss_fixture...", file=sys.stderr)
        subprocess.run(["make", "generate_mss_fixture"], cwd=repo_root, check=True)
    
    with tempfile.NamedTemporaryFile(suffix=".mss", delete=False) as tmp:
        tmp_path = Path(tmp.name)
    
    try:
        subprocess.run([str(fixture_bin), str(tmp_path)], cwd=repo_root, check=True)
        return tmp_path.read_bytes()
    finally:
        tmp_path.unlink(missing_ok=True)


def convert_state_to_mss(
    state_path: Path,
    output_path: Path,
    *,
    repo_root: Path | None = None,
    template_path: Path | None = None,
) -> None:
    """Convert a .state file to .mss format.
    
    Args:
        state_path: Path to .state file (stable-retro or snes9x format)
        output_path: Path to write .mss file
        repo_root: Repository root (for building template if needed)
        template_path: Optional pre-built .mss template to use
    """
    # Parse the .state file to extract WRAM
    blocks = parse_s9x_blocks(state_path)
    if "RAM" not in blocks:
        raise ValueError(f"{state_path} has no RAM block")
    
    wram = blocks["RAM"]
    if len(wram) != WRAM_SIZE:
        raise ValueError(f"Expected {WRAM_SIZE} bytes of WRAM, got {len(wram)}")
    
    # Get or create a MiniSaveState template
    if template_path:
        mss_template = template_path.read_bytes()
    elif repo_root:
        mss_template = create_mss_template(repo_root)
    else:
        raise ValueError("Either repo_root or template_path must be provided")
    
    # Overlay WRAM onto the template
    mss_data = bytearray(mss_template)
    overlay_wram_onto_mss(mss_data, wram)
    
    # Write output
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_bytes(mss_data)
    
    # Verify key addresses
    ram_offset = find_ram_offset_in_mss(mss_data)
    if ram_offset is not None:
        print(f"Converted {state_path} -> {output_path}", file=sys.stderr)
        print(f"Corresponding WRAM addresses:", file=sys.stderr)
        for name, addr in CORRESPONDING_WRAM_ADDRS.items():
            value = read_u16_le(mss_data, ram_offset + addr)
            print(f"  {name:12} @ 0x{addr:04X} = 0x{value:04X} ({value})", file=sys.stderr)


def command_convert(args: argparse.Namespace) -> int:
    """Convert .state to .mss command."""
    repo_root = Path(__file__).resolve().parents[1]
    state_path = Path(args.input)
    output_path = Path(args.output)
    template_path = Path(args.template) if args.template else None
    
    try:
        convert_state_to_mss(
            state_path,
            output_path,
            repo_root=repo_root,
            template_path=template_path,
        )
        return 0
    except Exception as exc:
        print(f"error: {exc}", file=sys.stderr)
        if args.verbose:
            import traceback
            traceback.print_exc()
        return 1


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", help="Input .state file (stable-retro or snes9x)")
    parser.add_argument("-o", "--output", required=True, help="Output .mss file")
    parser.add_argument(
        "-t",
        "--template",
        help="Use existing .mss template (default: generate from generate_mss_fixture)",
    )
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose output")
    return parser


def main(argv: list[str]) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    return command_convert(args)


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
