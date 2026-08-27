"""Pure CanonicalizeEnemyHandler alias-table tests. No ROM required."""

from __future__ import annotations

import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BIN = ROOT / "build" / "test_enemy_ai_canon"


def test_canonicalize_enemy_handler_table():
    (ROOT / "build").mkdir(exist_ok=True)
    compile_cmd = [
        "cc",
        "-O0",
        "-fno-strict-aliasing",
        "-Werror",
        "-I.",
        "-iquote",
        "src",
        "tests/test_enemy_ai_canon.c",
        "src/enemy_ai_canon.c",
        "-o",
        str(BIN),
    ]
    compiled = subprocess.run(compile_cmd, cwd=ROOT, capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr + compiled.stdout
    ran = subprocess.run([str(BIN)], cwd=ROOT, capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr
