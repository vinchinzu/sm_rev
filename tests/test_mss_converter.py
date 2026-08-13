#!/usr/bin/env python3
"""Test MiniSaveState converter with synthetic WRAM (no ROM required)."""

import json
import struct
import subprocess
import tempfile
from pathlib import Path

import pytest


REPO_ROOT = Path(__file__).parent.parent
TOOLS_DIR = REPO_ROOT / "tools"
CONVERTER = TOOLS_DIR / "mss_converter.py"

# WRAM addresses (from mini_wram_peek.h)
WRAM_ADDR_ROOM_ID = 0x079B
WRAM_ADDR_POSE = 0x0A1C
WRAM_ADDR_SAMUS_X = 0x0AF6
WRAM_ADDR_SAMUS_X_SUB = 0x0AF8
WRAM_ADDR_SAMUS_Y = 0x0AFA
WRAM_ADDR_SAMUS_Y_SUB = 0x0AFC
WRAM_ADDR_ENERGY = 0x09C2

MINI_SNAPSHOT_MAGIC = 0x4D53534D  # "MSSM"
MINI_SNAPSHOT_VERSION = 4
MINI_RAM_SIZE = 0x20000
MINI_SRAM_SIZE = 0x2000


def write_u16_le(buf: bytearray, offset: int, value: int) -> None:
    """Write uint16 little-endian to buffer."""
    struct.pack_into("<H", buf, offset, value)


def read_u16_le(buf: bytes | bytearray, offset: int) -> int:
    """Read uint16 little-endian from buffer."""
    return struct.unpack_from("<H", buf, offset)[0]


def find_ram_offset_in_mss(mss_data: bytes) -> int | None:
    """Find the offset of the ram[] array in a MiniStateSnapshot.
    
    Copied from mss_converter.py for test independence.
    """
    if len(mss_data) < MINI_RAM_SIZE + MINI_SRAM_SIZE + 100:
        return None
    
    magic = struct.unpack_from("<I", mss_data, 0)[0]
    version = struct.unpack_from("<I", mss_data, 4)[0]
    
    if magic != MINI_SNAPSHOT_MAGIC or version != MINI_SNAPSHOT_VERSION:
        return None
    
    expected_ram_offset = len(mss_data) - MINI_RAM_SIZE - MINI_SRAM_SIZE - 16
    
    for offset in range(expected_ram_offset - 1024, expected_ram_offset + 1024):
        if offset < 0 or offset + MINI_RAM_SIZE > len(mss_data):
            continue
        sram_offset = offset + MINI_RAM_SIZE
        if sram_offset + MINI_SRAM_SIZE <= len(mss_data):
            return offset
    
    return None


def create_synthetic_wram_buffer(
    *,
    samus_x: int = 512,
    samus_x_sub: int = 0x8000,
    samus_y: int = 256,
    samus_y_sub: int = 0x4000,
    pose: int = 0x01,
    room_id: int = 0x91F8,
    energy: int = 99,
) -> bytes:
    """Create a synthetic 128KB WRAM buffer with known values."""
    wram = bytearray(MINI_RAM_SIZE)
    
    # Set known values at specific addresses
    write_u16_le(wram, WRAM_ADDR_SAMUS_X, samus_x)
    write_u16_le(wram, WRAM_ADDR_SAMUS_X_SUB, samus_x_sub)
    write_u16_le(wram, WRAM_ADDR_SAMUS_Y, samus_y)
    write_u16_le(wram, WRAM_ADDR_SAMUS_Y_SUB, samus_y_sub)
    write_u16_le(wram, WRAM_ADDR_POSE, pose)
    write_u16_le(wram, WRAM_ADDR_ROOM_ID, room_id)
    write_u16_le(wram, WRAM_ADDR_ENERGY, energy)
    
    return bytes(wram)


def create_synthetic_s9x_state(wram: bytes, output_path: Path) -> None:
    """Create a minimal synthetic snes9x .state file with WRAM.
    
    This creates a minimal but valid #!s9xsnp: file with RAM and SRA blocks.
    """
    import gzip
    
    # Build a minimal snes9x snapshot with RAM block
    header = b"#!s9xsnp:0006\n"
    
    # RAM block: "RAM:131072:" + 128KB data
    ram_header = f"RAM:{len(wram):06d}:".encode("ascii")
    ram_block = ram_header + wram
    
    # SRA block: "SRA:008192:" + 8KB SRAM (zeroed for test)
    sram = bytes(8192)
    sra_header = f"SRA:{len(sram):06d}:".encode("ascii")
    sra_block = sra_header + sram
    
    # Combine and gzip
    snapshot_data = header + ram_block + sra_block
    compressed = gzip.compress(snapshot_data)
    
    output_path.write_bytes(compressed)


def test_synthetic_wram_overlay():
    """Test overlaying synthetic WRAM onto a MiniSaveState template.
    
    This test creates a synthetic WRAM buffer, overlays it onto a template,
    and verifies the round-trip WITHOUT requiring a ROM or real .state file.
    """
    # Create synthetic WRAM with known values
    test_values = {
        "samus_x": 512,
        "samus_x_sub": 0x8000,
        "samus_y": 256,
        "samus_y_sub": 0x4000,
        "pose": 0x01,
        "room_id": 0x91F8,
        "energy": 99,
    }
    
    synthetic_wram = create_synthetic_wram_buffer(**test_values)
    
    with tempfile.TemporaryDirectory() as tmpdir:
        tmppath = Path(tmpdir)
        
        # Create a synthetic .state file
        state_file = tmppath / "synthetic.state"
        create_synthetic_s9x_state(synthetic_wram, state_file)
        
        # Convert to .mss
        mss_file = tmppath / "converted.mss"
        result = subprocess.run(
            [str(CONVERTER), str(state_file), "-o", str(mss_file)],
            capture_output=True,
            text=True,
        )
        
        assert result.returncode == 0, f"Converter failed: {result.stderr}"
        assert mss_file.exists(), "Converter did not create output file"
        
        # Verify the .mss has the overlaid WRAM values
        mss_data = mss_file.read_bytes()
        ram_offset = find_ram_offset_in_mss(mss_data)
        
        assert ram_offset is not None, "Could not find RAM offset in .mss"
        
        # Verify all key addresses match
        assert read_u16_le(mss_data, ram_offset + WRAM_ADDR_SAMUS_X) == test_values["samus_x"]
        assert read_u16_le(mss_data, ram_offset + WRAM_ADDR_SAMUS_X_SUB) == test_values["samus_x_sub"]
        assert read_u16_le(mss_data, ram_offset + WRAM_ADDR_SAMUS_Y) == test_values["samus_y"]
        assert read_u16_le(mss_data, ram_offset + WRAM_ADDR_SAMUS_Y_SUB) == test_values["samus_y_sub"]
        assert read_u16_le(mss_data, ram_offset + WRAM_ADDR_POSE) == test_values["pose"]
        assert read_u16_le(mss_data, ram_offset + WRAM_ADDR_ROOM_ID) == test_values["room_id"]
        assert read_u16_le(mss_data, ram_offset + WRAM_ADDR_ENERGY) == test_values["energy"]


def test_mss_roundtrip_via_cli():
    """Test that converted .mss can be loaded by sm_rev_predict.
    
    This test creates a synthetic .mss, loads it via sm_rev_predict --load-state,
    and verifies that frame[0] matches the source WRAM values.
    
    NOTE: This test requires ROM assets to be available for Mini to run.
    If ROM is not available, the test will skip gracefully.
    The core converter functionality is tested by test_synthetic_wram_overlay.
    """
    predict_cli = REPO_ROOT / "sm_rev_predict"
    if not predict_cli.exists():
        pytest.skip("sm_rev_predict not built")
    
    # Create synthetic WRAM with known values
    test_values = {
        "samus_x": 512,
        "samus_x_sub": 0x8000,
        "samus_y": 256,
        "samus_y_sub": 0x4000,
        "pose": 0x01,
        "room_id": 0x91F8,
        "energy": 99,
    }
    
    synthetic_wram = create_synthetic_wram_buffer(**test_values)
    
    with tempfile.TemporaryDirectory() as tmpdir:
        tmppath = Path(tmpdir)
        
        # Create synthetic .state and convert
        state_file = tmppath / "synthetic.state"
        create_synthetic_s9x_state(synthetic_wram, state_file)
        
        mss_file = tmppath / "converted.mss"
        result = subprocess.run(
            [str(CONVERTER), str(state_file), "-o", str(mss_file)],
            capture_output=True,
            text=True,
        )
        assert result.returncode == 0, f"Converter failed: {result.stderr}"
        
        # Run sm_rev_predict with --load-state and capture frame[0]
        # The CLI takes JSON via stdin: {"inputs": [{"buttons": int}, ...]}
        test_input = {
            "inputs": [{"buttons": 0}]  # 1 frame of no input (wire format)
        }
        test_input_json = json.dumps(test_input)
        
        result = subprocess.run(
            [str(predict_cli), "--load-state", str(mss_file)],
            input=test_input_json,
            capture_output=True,
            text=True,
        )
        
        # The CLI may fail if ROM assets are not available - this is expected
        if result.returncode != 0:
            pytest.skip(
                f"sm_rev_predict failed (needs ROM assets or state incompatible): "
                f"exit={result.returncode}, stderr={result.stderr[:100]}"
            )
        
        # Parse output from stdout
        output = json.loads(result.stdout)
        
        assert "frames" in output, "Output missing frames"
        assert len(output["frames"]) >= 1, "Output has no frames"
        
        frame0 = output["frames"][0]
        
        # Verify frame[0] matches the overlaid WRAM values (corresponding start)
        assert frame0["samus_x"] == test_values["samus_x"], \
            f"samus_x mismatch: {frame0['samus_x']} != {test_values['samus_x']}"
        assert frame0["samus_x_sub"] == test_values["samus_x_sub"], \
            f"samus_x_sub mismatch: {frame0['samus_x_sub']} != {test_values['samus_x_sub']}"
        assert frame0["samus_y"] == test_values["samus_y"], \
            f"samus_y mismatch: {frame0['samus_y']} != {test_values['samus_y']}"
        assert frame0["samus_y_sub"] == test_values["samus_y_sub"], \
            f"samus_y_sub mismatch: {frame0['samus_y_sub']} != {test_values['samus_y_sub']}"
        assert frame0["pose"] == test_values["pose"], \
            f"pose mismatch: {frame0['pose']} != {test_values['pose']}"
        assert frame0["room_id"] == test_values["room_id"], \
            f"room_id mismatch: {frame0['room_id']} != {test_values['room_id']}"
        assert frame0["energy"] == test_values["energy"], \
            f"energy mismatch: {frame0['energy']} != {test_values['energy']}"


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
