#!/usr/bin/env python3
"""Test sm_rev_predict --load-state CLI functionality with MiniSaveState blobs."""

import json
import struct
import subprocess
import sys
from pathlib import Path


def read_u16_le(data: bytes, offset: int) -> int:
    """Read uint16 little-endian from bytes."""
    return struct.unpack_from("<H", data, offset)[0]


def extract_wram_from_mss(blob_path: Path) -> bytes:
    """Extract WRAM (g_ram) from MiniSaveState blob.
    
    MiniSaveState format:
    - magic: 4 bytes (0x4D53534D 'MSSM')
    - version: 4 bytes
    - MiniGameState: variable
    - MiniStubsSnapshot: variable
    - MiniPpuSnapshot: variable
    - ram[0x20000]: 128KB WRAM
    - sram[0x2000]: 8KB
    - use_my_apu_code: 1 byte
    - host_debug_flag: 1 byte
    - (padding: 2 bytes for int alignment)
    - snes_frame_counter: 4 bytes (int)
    - installed_bug_fix_counter: 2 bytes
    - (padding: 2 bytes for struct alignment)
    
    Trailing after WRAM: 0x200C bytes (SRAM + flags + padding)
    """
    with open(blob_path, "rb") as f:
        blob = f.read()
    
    # WRAM is 0x20000 bytes and appears near the end
    # Trailing after WRAM: SRAM (0x2000) + flags + padding (0xC bytes total)
    # So WRAM ends at: len - 0x200C
    wram_end = len(blob) - 0x200C
    wram_start = wram_end - 0x20000
    
    if wram_start < 0:
        raise ValueError(f"Blob too small: {len(blob)} bytes")
    
    return blob[wram_start:wram_end]


def verify_frame0_from_blob(output: dict, expected_x: int, expected_x_sub: int, 
                            expected_y: int, expected_y_sub: int) -> tuple[bool, str]:
    """Verify frame 0 matches expected values from blob.
    
    Returns (success, error_message).
    """
    if len(output.get("frames", [])) == 0:
        return False, "No frames in output"
    
    frame0 = output["frames"][0]
    
    # Frame 0 is the pre-step state from the loaded blob
    if frame0["samus_x"] != expected_x:
        return False, f"Frame 0 samus_x={frame0['samus_x']}, expected {expected_x} (from blob)"
    
    if frame0["samus_y"] != expected_y:
        return False, f"Frame 0 samus_y={frame0['samus_y']}, expected {expected_y} (from blob)"
    
    # Verify subpixels are also loaded correctly
    if frame0["samus_x_sub"] != expected_x_sub:
        return False, f"Frame 0 samus_x_sub={frame0['samus_x_sub']}, expected {expected_x_sub} (from blob)"
    
    if frame0["samus_y_sub"] != expected_y_sub:
        return False, f"Frame 0 samus_y_sub={frame0['samus_y_sub']}, expected {expected_y_sub} (from blob)"
    
    return True, ""


def test_load_state_cli():
    """Test that --load-state flag works with MiniSaveState blobs."""
    workspace = Path(__file__).parent.parent
    predict_cli = workspace / "sm_rev_predict"
    state_file = workspace / "tests" / "fixtures" / "landing_site_spawn.mss"
    
    if not predict_cli.exists():
        print(f"ERROR: {predict_cli} not found. Run 'make mini-predict-cli' first.")
        return 1
    
    if not state_file.exists():
        print(f"ERROR: {state_file} not found. Run 'make generate_mss_fixture' first.")
        return 1
    
    # Extract expected values from the .mss blob (not hardcoded!)
    try:
        wram = extract_wram_from_mss(state_file)
    except Exception as e:
        print(f"ERROR: Failed to extract WRAM from blob: {e}")
        return 1
    
    # Read the four critical fields from g_ram
    expected_x = read_u16_le(wram, 0x0AF6)
    expected_x_sub = read_u16_le(wram, 0x0AF8)
    expected_y = read_u16_le(wram, 0x0AFA)
    expected_y_sub = read_u16_le(wram, 0x0AFC)
    
    print(f"Expected from blob: x={expected_x}.{expected_x_sub}, y={expected_y}.{expected_y_sub}")
    
    # Test input: 10 frames of Right button (128 in wire format)
    test_input = {
        "inputs": [{"buttons": 128}] * 10
    }
    test_input_json = json.dumps(test_input)
    
    # Test all three argument orderings
    test_cases = [
        ("--load-state FILE", [str(predict_cli), "--load-state", str(state_file)]),
        ("predict --load-state FILE", [str(predict_cli), "predict", "--load-state", str(state_file)]),
        ("--load-state FILE predict", [str(predict_cli), "--load-state", str(state_file), "predict"]),
    ]
    
    for test_name, args in test_cases:
        print(f"\nTesting: {test_name}")
        
        result = subprocess.run(
            args,
            input=test_input_json,
            capture_output=True,
            text=True
        )
        
        if result.returncode != 0:
            print(f"  ERROR: CLI returned non-zero exit code: {result.returncode}")
            print(f"  stderr: {result.stderr}")
            return 1
        
        # Parse output
        try:
            output = json.loads(result.stdout)
        except json.JSONDecodeError as e:
            print(f"  ERROR: Failed to parse JSON output: {e}")
            print(f"  stdout: {result.stdout}")
            return 1
        
        # Verify frame 0 matches the loaded state (pre-step)
        success, error_msg = verify_frame0_from_blob(
            output, expected_x, expected_x_sub, expected_y, expected_y_sub
        )
        
        if not success:
            print(f"  ERROR: {error_msg}")
            return 1
        
        frame0 = output["frames"][0]
        print(f"  ✓ frames[0] (pre-step): x={frame0['samus_x']}.{frame0['samus_x_sub']}, " + 
              f"y={frame0['samus_y']}.{frame0['samus_y_sub']}")
        print(f"  ✓ Total frames: {len(output['frames'])} (frame 0 pre-step + {len(output['frames'])-1} post-step)")
    
    print("\n✓ All CLI argument orderings work correctly")
    return 0


if __name__ == "__main__":
    sys.exit(test_load_state_cli())
