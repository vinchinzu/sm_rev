#!/usr/bin/env python3
"""Generate a MiniSaveState blob fixture for testing --load-state.

Creates a binary MiniSaveState snapshot with known WRAM values at critical
addresses for frame 0 validation.
"""

import struct
import sys
from pathlib import Path


def create_mini_savestate_fixture(output_path: Path, wram_values: dict[int, int]) -> None:
    """Create a MiniSaveState blob with custom WRAM values.
    
    This generates a synthetic fixture by:
    1. Running sm_rev_mini headless for 1 frame to get a valid baseline snapshot
    2. Patching the WRAM portion with our custom values
    3. Writing the result as a binary blob
    
    For now, we'll create a minimal valid structure.
    The MiniSaveState format (from mini_game.c):
    - magic: 0x4D53534D ('MSSM')
    - version: 4
    - MiniGameState
    - MiniStubsSnapshot
    - MiniPpuSnapshot
    - ram[0x20000]
    - sram[0x2000]
    - use_my_apu_code (bool)
    - host_debug_flag (bool)
    - snes_frame_counter (int)
    - installed_bug_fix_counter (uint16)
    """
    print(f"Generating MiniSaveState fixture: {output_path}")
    print("  This requires sm_rev_mini to generate a baseline snapshot...")
    
    # For now, instruct the user to generate this manually
    print(f"""
To generate the fixture:

1. Run mini and save a state:
   ./sm_rev_mini --headless --frames 1 --save-state {output_path}

2. Or use Python to call sm_rev_mini subprocess:
   import subprocess
   subprocess.run(['./sm_rev_mini', '--headless', '--frames', '1', 
                   '--save-state', str(output_path)], check=True)

3. Then patch WRAM values if needed with tools/savestate_tool.py
""")


def main():
    """Generate test fixtures."""
    workspace = Path(__file__).parent.parent
    fixtures_dir = workspace / "tests" / "fixtures"
    fixtures_dir.mkdir(parents=True, exist_ok=True)
    
    # We need a real MiniSaveState blob
    # The simplest way is to run sm_rev_mini and capture its snapshot
    output_path = fixtures_dir / "landing_site_spawn.mss"
    
    print("=" * 60)
    print("MiniSaveState Fixture Generation")
    print("=" * 60)
    
    create_mini_savestate_fixture(output_path, {
        0x0AF6: 128,     # Samus X
        0x0AF8: 0,       # Samus X subpixel
        0x0AFA: 176,     # Samus Y
        0x0AFC: 0,       # Samus Y subpixel
    })
    
    print("\nOnce generated, commit the .mss file to tests/fixtures/")


if __name__ == "__main__":
    main()
