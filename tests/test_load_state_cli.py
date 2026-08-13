#!/usr/bin/env python3
"""Test sm_rev_predict --load-state CLI functionality with MiniSaveState blobs."""

import json
import subprocess
import sys
from pathlib import Path


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
    
    # Test input: 10 frames of Right button (128 in wire format)
    test_input = {
        "inputs": [{"buttons": 128}] * 10
    }
    
    # Run CLI with --load-state
    result = subprocess.run(
        [str(predict_cli), "--load-state", str(state_file)],
        input=json.dumps(test_input),
        capture_output=True,
        text=True
    )
    
    if result.returncode != 0:
        print(f"ERROR: CLI returned non-zero exit code: {result.returncode}")
        print(f"stderr: {result.stderr}")
        return 1
    
    # Parse output
    try:
        output = json.loads(result.stdout)
    except json.JSONDecodeError as e:
        print(f"ERROR: Failed to parse JSON output: {e}")
        print(f"stdout: {result.stdout}")
        return 1
    
    # Verify frame 0 matches the loaded state
    # From landing_site_spawn.mss: samus_x=128, samus_y=176
    if len(output["frames"]) == 0:
        print("ERROR: No frames in output")
        return 1
    
    frame0 = output["frames"][0]
    if frame0["samus_x"] != 128:
        print(f"ERROR: Frame 0 samus_x={frame0['samus_x']}, expected 128")
        return 1
    
    if frame0["samus_y"] != 176:
        print(f"ERROR: Frame 0 samus_y={frame0['samus_y']}, expected 176")
        return 1
    
    # Verify subpixels are also loaded (blob has x_sub=0, y_sub=0)
    if frame0["samus_x_sub"] != 0:
        print(f"ERROR: Frame 0 samus_x_sub={frame0['samus_x_sub']}, expected 0")
        return 1
    
    print("✓ CLI --load-state works correctly")
    print(f"  Frame 0: x={frame0['samus_x']}.{frame0['samus_x_sub']}, " + 
          f"y={frame0['samus_y']}.{frame0['samus_y_sub']}")
    print(f"  Total frames: {len(output['frames'])}")
    print(f"  Predictor: {output['predictor']}")
    
    return 0


if __name__ == "__main__":
    sys.exit(test_load_state_cli())
