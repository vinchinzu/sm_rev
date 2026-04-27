"""
Build integrity tests for sm_rev.

These run `make` and verify the binary comes out clean. No ROM needed.
All tests here must pass after EVERY code change — they are the first gate.
"""

from __future__ import annotations

import json
import os
import shutil
import struct
import subprocess
from pathlib import Path

SM_REV_DIR = Path(__file__).parent.parent
BINARY = SM_REV_DIR / "sm_rev"
MINI_BINARY = SM_REV_DIR / "sm_rev_mini"
EDITOR_LANDING_SITE_EXPORT = SM_REV_DIR.parent / "super_metroid_editor" / "export" / "sm_nav" / "rooms" / "room_91F8.json"


def run(cmd: list[str], **kw) -> subprocess.CompletedProcess:
    cwd = kw.pop("cwd", SM_REV_DIR)
    return subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, **kw)


def parse_json_payload(stdout: str) -> dict:
    start = stdout.find("{")
    end = stdout.rfind("}")
    assert start >= 0 and end > start, stdout
    return json.loads(stdout[start:end + 1])


def read_bmp_rows(path: Path) -> tuple[int, int, list[bytes]]:
    data = path.read_bytes()
    pixel_offset = struct.unpack_from("<I", data, 10)[0]
    dib_size = struct.unpack_from("<I", data, 14)[0]
    assert dib_size >= 40, f"unexpected BMP DIB header size: {dib_size}"
    width, height = struct.unpack_from("<ii", data, 18)
    bits_per_pixel = struct.unpack_from("<H", data, 28)[0]
    compression = struct.unpack_from("<I", data, 30)[0]
    assert bits_per_pixel == 32, f"expected 32bpp BMP, got {bits_per_pixel}"
    assert compression == 3, f"expected BI_BITFIELDS BMP, got compression={compression}"
    row_stride = width * 4
    rows = []
    for y in range(abs(height)):
        src_y = abs(height) - 1 - y if height > 0 else y
        start = pixel_offset + src_y * row_stride
        rows.append(data[start:start + row_stride])
    return width, abs(height), rows


def read_bmp_argb_pixels(path: Path) -> tuple[int, int, list[list[int]]]:
    width, height, rows = read_bmp_rows(path)
    pixels = [
        [struct.unpack_from("<I", row, x * 4)[0] for x in range(width)]
        for row in rows
    ]
    return width, height, pixels


class TestBuild:
    def test_make_clean_succeeds(self):
        """make clean should always exit 0."""
        r = run(["make", "clean"])
        assert r.returncode == 0, f"make clean failed:\n{r.stderr}"

    def test_make_succeeds(self):
        """Full build must exit 0 with -Werror — no warnings allowed."""
        r = run(["make"])
        assert r.returncode == 0, f"make failed:\n{r.stderr}\n{r.stdout}"

    def test_binary_exists(self):
        """Binary must exist after build."""
        assert BINARY.exists(), f"Binary not found at {BINARY}"

    def test_binary_is_executable(self):
        """Binary must be executable."""
        assert os.access(BINARY, os.X_OK), f"Binary not executable: {BINARY}"

    def test_no_compilation_warnings(self):
        """make output must be warning-free (enforced by -Werror, but double-check stderr)."""
        r = run(["make", "clean"])
        r2 = run(["make"])
        assert r2.returncode == 0
        # -Werror makes warnings into errors, so a clean build has no warning lines
        for line in r2.stderr.splitlines():
            assert "warning:" not in line.lower(), f"Unexpected warning in build:\n{line}"


class TestBuildMini:
    def test_make_mini_succeeds(self):
        """Mini shell build must compile cleanly."""
        r = run(["make", "mini"])
        assert r.returncode == 0, f"make mini failed:\n{r.stderr}\n{r.stdout}"

    def test_mini_binary_exists(self):
        """Mini shell binary must exist after build."""
        assert MINI_BINARY.exists(), f"Mini binary not found at {MINI_BINARY}"

    def test_mini_binary_is_executable(self):
        """Mini shell binary must be executable."""
        assert os.access(MINI_BINARY, os.X_OK), f"Mini binary not executable: {MINI_BINARY}"

    def test_mini_headless_smoke(self):
        """Mini shell headless mode must exit 0 and report mini build metadata."""
        r = run([str(MINI_BINARY), "--headless", "--frames", "3"])
        assert r.returncode == 0, f"mini headless smoke failed:\n{r.stderr}\n{r.stdout}"
        assert '"build":"mini"' in r.stdout
        assert '"frames":3' in r.stdout
        assert '"no_rooms":false' in r.stdout

    def test_mini_headless_smoke_outside_repo_cwd(self, tmp_path: Path):
        """Mini should still find its default room export when launched outside the repo cwd."""
        r = run([str(MINI_BINARY), "--headless", "--frames", "1"], cwd=tmp_path)
        assert r.returncode == 0, f"mini external-cwd smoke failed:\n{r.stderr}\n{r.stdout}"
        assert '"room_source":"editor_export"' in r.stdout
        assert '"room_handle":"landingSite"' in r.stdout
        assert '"room_visuals":"editor_tileset"' in r.stdout

    def test_mini_default_export_from_worktree_without_local_assets(self, tmp_path: Path):
        """A worktree-style checkout without ignored assets should still use the common checkout export."""
        local_export = SM_REV_DIR / "assets" / "local_mini" / "room_91F8.json"
        if not local_export.exists():
            return

        fake_checkout = tmp_path / "fake_worktree"
        fake_gitdir = tmp_path / "git" / "worktrees" / "fake_worktree"
        outside_cwd = tmp_path / "outside"
        fake_checkout.mkdir()
        fake_gitdir.mkdir(parents=True)
        outside_cwd.mkdir()

        common_git_dir = subprocess.run(
            ["git", "rev-parse", "--path-format=absolute", "--git-common-dir"],
            cwd=SM_REV_DIR,
            capture_output=True,
            text=True,
            check=True,
        ).stdout.strip()
        (fake_checkout / ".git").write_text(f"gitdir: {fake_gitdir}\n")
        (fake_gitdir / "commondir").write_text(f"{common_git_dir}\n")
        shutil.copy2(MINI_BINARY, fake_checkout / MINI_BINARY.name)

        r = run([str(fake_checkout / MINI_BINARY.name), "--headless", "--frames", "1"], cwd=outside_cwd)
        assert r.returncode == 0, f"mini fake-worktree smoke failed:\n{r.stderr}\n{r.stdout}"
        assert '"room_source":"editor_export"' in r.stdout
        assert '"room_handle":"landingSite"' in r.stdout
        assert '"room_visuals":"editor_tileset"' in r.stdout

    def test_mini_editor_export_bridge_smoke(self):
        """Mini should accept an explicit editor-exported Landing Site room file."""
        if not EDITOR_LANDING_SITE_EXPORT.exists():
            return
        r = run([
            str(MINI_BINARY),
            "--headless",
            "--frames",
            "1",
            "--room-export",
            str(EDITOR_LANDING_SITE_EXPORT),
        ])
        assert r.returncode == 0, f"mini editor-export smoke failed:\n{r.stderr}\n{r.stdout}"
        assert '"room_source":"editor_export"' in r.stdout
        assert '"room_handle":"landingSite"' in r.stdout
        assert '"rom_room":false' in r.stdout

    def test_mini_editor_export_uses_rom_visual_fallback_when_json_has_no_assets(self):
        """Plain editor JSON without embedded asset paths should still render with ROM-backed visuals when a ROM is available."""
        if not EDITOR_LANDING_SITE_EXPORT.exists():
            return
        rom_candidates = [
            SM_REV_DIR / "sm.smc",
            SM_REV_DIR.parent / "sm" / "sm.smc",
        ]
        if not any(path.exists() for path in rom_candidates):
            return
        r = run([
            str(MINI_BINARY),
            "--headless",
            "--frames",
            "1",
            "--room-export",
            str(EDITOR_LANDING_SITE_EXPORT),
        ])
        assert r.returncode == 0, f"mini editor-export ROM visual fallback failed:\n{r.stderr}\n{r.stdout}"
        assert '"room_visuals":"editor_tileset"' in r.stdout, r.stdout

    def test_mini_landing_site_background_effects_animate(self, tmp_path: Path):
        """Landing Site background effects should change across time instead of staying frozen."""
        room_path = SM_REV_DIR / "assets" / "local_mini" / "room_91F8.json"
        if not room_path.exists():
            return
        frame1 = tmp_path / "frame1.bmp"
        frame120 = tmp_path / "frame120.bmp"
        cmd_base = [
            str(MINI_BINARY),
            "--headless",
            "--room-export",
            str(room_path),
            "--screenshot",
        ]
        r1 = run(cmd_base + [str(frame1), "--frames", "1"])
        r2 = run(cmd_base + [str(frame120), "--frames", "120"])
        assert r1.returncode == 0, f"mini animated background frame 1 failed:\n{r1.stderr}\n{r1.stdout}"
        assert r2.returncode == 0, f"mini animated background frame 120 failed:\n{r2.stderr}\n{r2.stdout}"
        assert frame1.read_bytes() != frame120.read_bytes(), "Landing Site background stayed byte-identical from frame 1 to frame 120"

    def test_mini_landing_site_scanline_sky_rows_are_not_flat_bands(self, tmp_path: Path):
        """Landing Site should render scanline-varied sky/cloud rows rather than mostly flat solid-color bands."""
        room_path = SM_REV_DIR / "assets" / "local_mini" / "room_91F8.json"
        if not room_path.exists():
            return
        frame = tmp_path / "scanline.bmp"
        r = run([
            str(MINI_BINARY),
            "--headless",
            "--frames",
            "1",
            "--room-export",
            str(room_path),
            "--screenshot",
            str(frame),
        ])
        assert r.returncode == 0, f"mini scanline screenshot failed:\n{r.stderr}\n{r.stdout}"
        _, _, rows = read_bmp_rows(frame)
        top_rows = rows[:80]
        nonuniform_rows = sum(1 for row in top_rows if len(set(row[i:i + 4] for i in range(0, len(row), 4))) > 1)
        assert nonuniform_rows >= 16, f"expected at least 16 non-uniform sky rows in top 80 scanlines, got {nonuniform_rows}"

    def test_mini_generated_background_flag_changes_screenshot(self, tmp_path: Path):
        """Mini should support an opt-in generated background without changing the default path."""
        room_path = SM_REV_DIR / "assets" / "local_mini" / "room_91F8.json"
        if not room_path.exists():
            return
        default_frame = tmp_path / "default.bmp"
        generated_frame = tmp_path / "generated.bmp"
        cmd_base = [
            str(MINI_BINARY),
            "--headless",
            "--frames",
            "1",
            "--room-export",
            str(room_path),
            "--screenshot",
        ]
        r_default = run(cmd_base + [str(default_frame)])
        r_generated = run(cmd_base + [str(generated_frame), "--background", "generated"])
        assert r_default.returncode == 0, f"mini default screenshot failed:\n{r_default.stderr}\n{r_default.stdout}"
        assert r_generated.returncode == 0, f"mini generated background screenshot failed:\n{r_generated.stderr}\n{r_generated.stdout}"
        assert '"background":"game"' in r_default.stdout
        assert '"background":"generated"' in r_generated.stdout
        assert default_frame.read_bytes() != generated_frame.read_bytes()

    def test_mini_rom_runtime_screenshot_draws_samus_oam(self, tmp_path: Path):
        """ROM-backed original-runtime mini frames should preserve and render Samus OAM."""
        rom_candidates = [
            SM_REV_DIR / "sm.smc",
            SM_REV_DIR.parent / "sm" / "sm.smc",
        ]
        if not any(path.exists() for path in rom_candidates):
            return
        frame = tmp_path / "rom_oam.bmp"
        r = run([
            str(MINI_BINARY),
            "--headless",
            "--frames",
            "1",
            "--screenshot",
            str(frame),
        ])
        assert r.returncode == 0, f"mini ROM screenshot failed:\n{r.stderr}\n{r.stdout}"
        state = parse_json_payload(r.stdout)
        assert state["rom_room"] is True
        assert state["original_runtime"] is True

        width, height, pixels = read_bmp_argb_pixels(frame)
        left = max(0, int(state["samus_x"]) - 8)
        right = min(width, int(state["samus_x"]) + 32)
        top = max(0, int(state["samus_y"]) - 8)
        bottom = min(height, int(state["samus_y"]) + 48)
        samus_yellow_pixels = 0
        for y in range(top, bottom):
            for x in range(left, right):
                pixel = pixels[y][x]
                red = (pixel >> 16) & 0xFF
                green = (pixel >> 8) & 0xFF
                blue = pixel & 0xFF
                if red >= 180 and green >= 120 and blue <= 96:
                    samus_yellow_pixels += 1
        assert samus_yellow_pixels >= 8, (
            f"expected visible Samus OAM near ({state['samus_x']}, {state['samus_y']}), "
            f"found {samus_yellow_pixels} yellow pixels"
        )

    def test_mini_basic_shooting_spawns_power_beam(self, tmp_path: Path):
        """Mini should import the basic Samus beam path and expose active projectile state."""
        script = tmp_path / "shoot.txt"
        script.write_text(("RIGHT SHOOT\n" * 20), encoding="utf-8")
        r = run([
            str(MINI_BINARY),
            "--headless",
            "--frames",
            "20",
            "--room-export",
            str(tmp_path / "missing_room.json"),
            "--input-script",
            str(script),
        ], cwd=tmp_path)
        assert r.returncode == 0, f"mini shooting smoke failed:\n{r.stderr}\n{r.stdout}"
        payload = parse_json_payload(r.stdout)
        assert payload["projectile_count"] >= 1, r.stdout
        assert payload["first_projectile_type"] & 0xF00 == 0, r.stdout
        assert payload["first_projectile_type"] & 0x3F == 0, r.stdout
        assert payload["first_projectile_x"] > payload["samus_x"], r.stdout

    def test_mini_record_smoke(self, tmp_path: Path):
        """Mini should emit a capped low-resolution quick clip when recording is requested."""
        if shutil.which("ffmpeg") is None:
            return
        r = run([
            str(MINI_BINARY),
            "--headless",
            "--frames",
            "3",
            "--record",
        ], cwd=tmp_path)
        assert r.returncode == 0, f"mini record smoke failed:\n{r.stderr}\n{r.stdout}"
        payload = parse_json_payload(r.stdout)
        output = Path(payload["record_path"])
        assert output.exists(), f"expected recording at {output}\nstdout={r.stdout}\nstderr={r.stderr}"
        assert "out" in output.parts
        assert output.stat().st_size > 0
        assert output.stat().st_size <= 10 * 1024 * 1024
        assert '"recording":true' in r.stdout

    def test_mini_replay_artifact_roundtrip_verifies_hash(self, tmp_path: Path):
        """Mini should write a self-describing replay and reject final hash drift when reading it."""
        script = tmp_path / "script.txt"
        script.write_text("RIGHT\nRIGHT JUMP\n.\nSHOOT\n", encoding="utf-8")
        replay = tmp_path / "mini_replay.json"
        missing_room = tmp_path / "missing_room.json"

        r_write = run([
            str(MINI_BINARY),
            "--headless",
            "--frames",
            "4",
            "--room-export",
            str(missing_room),
            "--input-script",
            str(script),
            "--replay-out",
            str(replay),
        ], cwd=tmp_path)
        assert r_write.returncode == 0, f"mini replay write failed:\n{r_write.stderr}\n{r_write.stdout}"
        payload = parse_json_payload(r_write.stdout)
        artifact = json.loads(replay.read_text(encoding="utf-8"))

        assert artifact["format"] == "sm_rev_mini_replay"
        assert artifact["version"] == 1
        assert artifact["frames"] == 4
        assert artifact["viewport"] == {"width": 256, "height": 224}
        assert artifact["room"]["source"] == "fallback"
        assert artifact["room"]["export_path"] == str(missing_room)
        assert artifact["final_hash"] == payload["state_hash"]
        assert len(artifact["inputs"]) == 4
        assert artifact["inputs"][0]["buttons"] != 0

        r_read = run([
            str(MINI_BINARY),
            "--headless",
            "--replay-in",
            str(replay),
        ], cwd=tmp_path)
        assert r_read.returncode == 0, f"mini replay read failed:\n{r_read.stderr}\n{r_read.stdout}"
        replay_payload = parse_json_payload(r_read.stdout)
        assert replay_payload["state_hash"] == artifact["final_hash"]
        assert replay_payload["replay_verified"] is True

        artifact["final_hash"] = "0x0000000000000000"
        bad_replay = tmp_path / "mini_replay_bad_hash.json"
        bad_replay.write_text(json.dumps(artifact), encoding="utf-8")
        r_bad = run([
            str(MINI_BINARY),
            "--headless",
            "--replay-in",
            str(bad_replay),
        ], cwd=tmp_path)
        assert r_bad.returncode != 0, "mini replay accepted a mismatched final hash"
        assert "replay final hash mismatch" in r_bad.stderr
