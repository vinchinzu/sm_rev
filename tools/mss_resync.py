#!/usr/bin/env python3
"""Create MiniSaveState .mss files from WRAM by loading and re-saving through Mini.

This ensures MiniGameState is properly synchronized with the overlaid WRAM.
"""

import struct
import subprocess
import sys
import tempfile
from pathlib import Path

# Add tools to path for converter
sys.path.insert(0, str(Path(__file__).parent.parent / "tools"))
from mss_converter import convert_state_to_mss, compute_ram_offset_in_mss

# Add tests to path for WRAM helpers
sys.path.insert(0, str(Path(__file__).parent.parent / "tests"))
from test_mss_converter import create_synthetic_wram_buffer, create_synthetic_s9x_state, read_u16_le


def reload_mss_through_mini(mss_path: Path, output_path: Path, repo_root: Path) -> None:
    """Load an .mss file through Mini and re-save it to synchronize MiniGameState.
    
    This uses a C helper program that:
    1. Loads the .mss with MiniLoadState
    2. Immediately saves it with MiniSaveState
    
    This ensures MiniGameState is synchronized with g_ram.
    """
    # We need a C helper for this - for now, just copy
    # TODO: Create tests/mss_resync.c to do: MiniLoadState + MiniSaveState
    import shutil
    shutil.copy(mss_path, output_path)


if __name__ == "__main__":
    repo_root = Path(__file__).parent.parent
    
    # Test: create synthetic WRAM, convert, and verify
    test_values = {
        "samus_x": 512,
        "samus_x_sub": 0x8000,
        "samus_y": 256,
        "samus_y_sub": 0x4000,
        "pose": 0x01,
        "room_id": 0x91F8,
        "energy": 99,
    }
    
    with tempfile.TemporaryDirectory() as tmpdir:
        tmppath = Path(tmpdir)
        
        # Create synthetic WRAM and convert
        wram = create_synthetic_wram_buffer(**test_values)
        state_file = tmppath / "test.state"
        create_synthetic_s9x_state(wram, state_file)
        
        mss_file = tmppath / "test.mss"
        convert_state_to_mss(state_file, mss_file, repo_root=repo_root)
        
        # Verify overlay
        mss_data = mss_file.read_bytes()
        ram_offset = compute_ram_offset_in_mss(len(mss_data))
        
        WRAM_ADDR_SAMUS_X = 0x0AF6
        WRAM_ADDR_ROOM_ID = 0x079B
        
        x = read_u16_le(mss_data, ram_offset + WRAM_ADDR_SAMUS_X)
        room = read_u16_le(mss_data, ram_offset + WRAM_ADDR_ROOM_ID)
        
        print(f"Overlaid WRAM: x={x}, room=0x{room:04X}")
        print(f"Note: This .mss may crash in Mini because MiniGameState is not synchronized.")
        print(f"      A proper resync tool (MiniLoadState + MiniSaveState) is needed.")
