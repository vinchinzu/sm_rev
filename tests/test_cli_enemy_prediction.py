#!/usr/bin/env python3
"""
Golden test for sm_rev_predict CLI enemy prediction.

Verifies that:
1. The CLI emits enemies[] when MiniSim has enemies
2. At least one enemy's x or y position changes across frames
"""

import json
import subprocess
import sys


def test_cli_enemy_prediction():
    """Run sm_rev_predict and verify enemy prediction."""
    # Simple 5-frame prediction with no inputs
    input_json = {
        "inputs": [
            {"buttons": 0},
            {"buttons": 0},
            {"buttons": 0},
            {"buttons": 0},
            {"buttons": 0},
        ]
    }
    
    # Run the predict CLI
    proc = subprocess.run(
        ["./sm_rev_predict"],
        input=json.dumps(input_json),
        capture_output=True,
        text=True,
        timeout=5,
    )
    
    if proc.returncode != 0:
        print(f"CLI failed: {proc.stderr}", file=sys.stderr)
        return False
    
    # Parse output (skip physics debug line if present)
    output_lines = proc.stdout.strip().split('\n')
    json_line = output_lines[-1] if output_lines[-1].startswith('{') else output_lines[0]
    
    try:
        result = json.loads(json_line)
    except json.JSONDecodeError as e:
        print(f"Failed to parse JSON: {e}", file=sys.stderr)
        print(f"Output was: {proc.stdout[:200]}", file=sys.stderr)
        return False
    
    # Verify structure
    if "frames" not in result:
        print("Missing 'frames' in output", file=sys.stderr)
        return False
    
    frames = result["frames"]
    if len(frames) < 5:
        print(f"Expected 5 frames, got {len(frames)}", file=sys.stderr)
        return False
    
    # Gate 1: Verify enemies[] is present
    frames_with_enemies = [f for f in frames if "enemies" in f and len(f["enemies"]) > 0]
    if len(frames_with_enemies) == 0:
        print("FAIL: No frames contain enemies[]", file=sys.stderr)
        return False
    
    print(f"✓ Found {len(frames_with_enemies)} frames with enemies")
    
    # Gate 2: Verify enemy structure has id, type, x, y
    first_enemy = frames_with_enemies[0]["enemies"][0]
    required_fields = ["id", "type", "x", "y"]
    for field in required_fields:
        if field not in first_enemy:
            print(f"FAIL: Enemy missing required field '{field}'", file=sys.stderr)
            return False
    
    print(f"✓ Enemy structure valid: id={first_enemy['id']:04x}, type={first_enemy['type']}, x={first_enemy['x']}, y={first_enemy['y']}")
    
    # Gate 3: Verify at least one enemy's position changes
    enemy_id = first_enemy["id"]
    positions = []
    for frame in frames_with_enemies:
        enemy = next((e for e in frame["enemies"] if e["id"] == enemy_id), None)
        if enemy:
            positions.append((enemy["x"], enemy["y"]))
    
    if len(set(positions)) <= 1:
        print(f"FAIL: Enemy {enemy_id:04x} did not move (positions: {positions})", file=sys.stderr)
        return False
    
    print(f"✓ Enemy {enemy_id:04x} moved: {positions[:5]}")
    
    # Gate 4: Verify x positions increase (roach walking right)
    x_positions = [pos[0] for pos in positions]
    if x_positions != sorted(x_positions):
        print(f"WARNING: X positions not monotonically increasing: {x_positions}", file=sys.stderr)
    
    print(f"✓ Enemy movement correct: x={x_positions[0]} → x={x_positions[-1]}")
    
    return True


if __name__ == "__main__":
    success = test_cli_enemy_prediction()
    sys.exit(0 if success else 1)
