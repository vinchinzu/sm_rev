#!/usr/bin/env python3
"""Measure Mini / C-port residual against the in-repo emulator.

Compares WRAM $0AF6/$0AFA (pixels) and $0AF8/$0AFC (subpixels).
If Mini ≠ emu, emu wins. This script writes docs/mini_emu_delta.md.

E for this repo-local profile is sm_rev --runmode theirs (built-in snes9x).
That is not RetroRL SuperMetroidEnv. Tag the table accordingly.
"""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PIXEL_FIELDS = ("samus_x", "samus_y")
SUB_FIELDS = ("samus_x_sub", "samus_y_sub")
POSITION_FIELDS = PIXEL_FIELDS + SUB_FIELDS
RIGHT = "100"  # kButton_Right


def load_jsonl(path: Path) -> list[dict]:
    rows = []
    for line in path.read_text().splitlines():
        line = line.strip()
        if line:
            rows.append(json.loads(line))
    return rows


def first_diff(left: list[dict], right: list[dict], fields: tuple[str, ...]) -> int | None:
    n = min(len(left), len(right))
    for i in range(n):
        for field in fields:
            if int(left[i].get(field, 0)) != int(right[i].get(field, 0)):
                return i
    if len(left) != len(right):
        return n
    return None


def run(cmd: list[str], env: dict[str, str] | None = None) -> subprocess.CompletedProcess[str]:
    merged = os.environ.copy()
    merged.setdefault("SDL_VIDEODRIVER", "offscreen")
    merged.setdefault("SDL_AUDIODRIVER", "dummy")
    if env:
        merged.update(env)
    return subprocess.run(cmd, cwd=ROOT, env=merged, text=True, capture_output=True)


def run_sm_rev(trace: Path, runmode: str, frames: int, inputs: str | None, save: Path) -> None:
    cmd = [
        str(ROOT / "sm_rev"),
        "--headless",
        str(frames),
        "--runmode",
        runmode,
        "--load-state",
        str(save),
        "--trace-wram",
        str(trace),
        "--dump",
        "-",
    ]
    if inputs is not None:
        cmd.extend(["--inputs", inputs])
    result = run(cmd)
    if result.returncode != 0:
        sys.stderr.write(result.stderr)
        raise SystemExit(f"sm_rev --runmode {runmode} failed ({result.returncode})")


def run_mini(trace: Path, frames: int, script: Path | None) -> dict:
    cmd = [
        str(ROOT / "sm_rev_mini"),
        "--headless",
        "--frames",
        str(frames),
        "--trace-wram",
        str(trace),
    ]
    if script is not None:
        cmd.extend(["--input-script", str(script)])
    result = run(cmd)
    if result.returncode != 0:
        sys.stderr.write(result.stderr)
        raise SystemExit(f"sm_rev_mini failed ({result.returncode})")
    last = result.stdout.strip().splitlines()
    return json.loads(last[-1]) if last else {}


def write_script(path: Path, token: str, frames: int) -> None:
    path.write_text("\n".join([token] * frames) + "\n")


def fmt_fd(value: int | None) -> str:
    return "∞" if value is None else str(value)


def summarize(name: str, left: list[dict], right: list[dict]) -> dict:
    fd_pixel = first_diff(left, right, PIXEL_FIELDS)
    fd_sub = first_diff(left, right, POSITION_FIELDS)
    start_match = bool(left) and bool(right) and all(
        int(left[0].get(field, 0)) == int(right[0].get(field, 0))
        for field in POSITION_FIELDS
    )
    first_field = None
    first_values = None
    fd = fd_sub if fd_sub is not None else fd_pixel
    if fd is not None and fd < min(len(left), len(right)):
        for field in POSITION_FIELDS:
            if int(left[fd].get(field, 0)) != int(right[fd].get(field, 0)):
                first_field = field
                first_values = (left[fd].get(field), right[fd].get(field))
                break
    return {
        "name": name,
        "frames": min(len(left), len(right)),
        "start_match": start_match,
        "fd_pixel": fd_pixel,
        "fd_sub": fd_sub,
        "first_field": first_field,
        "first_values": first_values,
        "left0": left[0] if left else {},
        "right0": right[0] if right else {},
        "left_end": left[-1] if left else {},
        "right_end": right[-1] if right else {},
    }


def render_delta(rows: list[dict], frames: int, save: str) -> str:
    lines = [
        "# Mini–emulator residual delta",
        "",
        "Measured locally. **If Mini ≠ emu, emu wins.**",
        "",
        f"- E: `sm_rev --runmode theirs` (in-repo snes9x). Not RetroRL SuperMetroidEnv.",
        f"- M for C-port rows: `sm_rev --runmode mine` from `{save}` (same start as E).",
        "- Mini row: `sm_rev_mini` ROM-backed Landing Site default. Starts are **not**",
        "  corresponding unless Mini also loaded that `.sav`.",
        f"- Horizon: {frames} frames. Residual words: `$0AF6/$0AFA` + `$0AF8/$0AFC`.",
        "",
        "| Tape | Pair | Start match | fd pixels | fd sub+pixel | First field | Notes |",
        "| --- | --- | --- | --- | --- | --- | --- |",
    ]
    for row in rows:
        notes = []
        if not row["start_match"]:
            notes.append("starts do not correspond")
        if row["first_values"] is not None:
            notes.append(f"{row['first_field']} {row['first_values'][0]} vs {row['first_values'][1]}")
        if row["fd_pixel"] is None and row["fd_sub"] is None:
            notes.append("no disagreement on residual words")
            left0 = row.get("left0") or {}
            left_end = row.get("left_end") or {}
            if left0 and left_end and all(
                int(left0.get(field, 0)) == int(left_end.get(field, 0))
                for field in POSITION_FIELDS
            ):
                notes.append("neither side moved")
        elif row["fd_sub"] is not None and row["fd_pixel"] is None:
            notes.append("subpixel only")
        lines.append(
            f"| {row['name']} | {row['pair']} | "
            f"{'yes' if row['start_match'] else 'no'} | "
            f"{fmt_fd(row['fd_pixel'])} | {fmt_fd(row['fd_sub'])} | "
            f"{row['first_field'] or '—'} | {'; '.join(notes) or '—'} |"
        )
    lines.extend(
        [
            "",
            "## What this tells planning",
            "",
            "A corresponding-start C-port vs emu row is the useful residual.",
            "If those WRAM words already disagree, Mini original-runtime cannot be",
            "a TAS planning oracle until that Mini/C delta is filed and fixed.",
            "A Mini CLI vs emu row with mismatched start is **not** a physics residual.",
            "",
            "Haskell is not part of this measurement.",
            "",
        ]
    )
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--frames", type=int, default=60)
    parser.add_argument("--save", default="saves/save0.sav")
    parser.add_argument("--out", default="docs/mini_emu_delta.md")
    args = parser.parse_args()

    save = Path(args.save)
    if not save.is_absolute():
        save = ROOT / save
    if not save.exists():
        raise SystemExit(f"missing save: {save}")
    if not (ROOT / "sm_rev").exists() or not (ROOT / "sm_rev_mini").exists():
        raise SystemExit("build sm_rev and sm_rev_mini first")

    out_dir = ROOT / "out" / "residual"
    out_dir.mkdir(parents=True, exist_ok=True)
    script_right = out_dir / "right.txt"
    write_script(script_right, "RIGHT", args.frames)

    rows = []
    for name, inputs, mini_script in (
        ("idle", None, None),
        ("walk-right", RIGHT, script_right),
    ):
        mine_path = out_dir / f"{name}_mine.jsonl"
        theirs_path = out_dir / f"{name}_theirs.jsonl"
        mini_path = out_dir / f"{name}_mini.jsonl"
        run_sm_rev(mine_path, "mine", args.frames, inputs, save)
        run_sm_rev(theirs_path, "theirs", args.frames, inputs, save)
        run_mini(mini_path, args.frames, mini_script)
        mine = load_jsonl(mine_path)
        theirs = load_jsonl(theirs_path)
        mini = load_jsonl(mini_path)
        mine_row = summarize(name, mine, theirs)
        mine_row["pair"] = "mine vs theirs"
        mini_row = summarize(name, mini, theirs)
        mini_row["pair"] = "mini vs theirs"
        rows.append(mine_row)
        rows.append(mini_row)

    text = render_delta(rows, args.frames, str(save.relative_to(ROOT)))
    out_path = ROOT / args.out if not Path(args.out).is_absolute() else Path(args.out)
    out_path.write_text(text)
    print(text)
    print(f"wrote {out_path}")

    corresponding = [row for row in rows if row["pair"] == "mine vs theirs"]
    disagreed = [row for row in corresponding if row["fd_sub"] is not None]
    return 1 if disagreed else 0


if __name__ == "__main__":
    raise SystemExit(main())
