#!/usr/bin/env python3
"""Parse src/pico/pico_kernel_sources.mk vs glob of MINI_SHARED_ENGINE_SRCS."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MK_PATH = Path(__file__).resolve().with_name("pico_kernel_sources.mk")
EXCLUDED_ENGINE = {
    "src/main.c",
    "src/sm_cpu_infra.c",
    "src/sm_rtl.c",
    "src/predict_cli.c",
}
ENGINE_VARS = (
    "PICO_KERNEL_SHIP1_SRCS",
    "PICO_KERNEL_SHIP2_SRCS",
    "PICO_KERNEL_SHIP3_SRCS",
    "PICO_STUB_SRCS",
)
MINI_VARS = (
    "PICO_MINI_RUNTIME_KEEP_SRCS",
    "PICO_MINI_RUNTIME_OMIT_SRCS",
)


def parse_make_lists(text: str) -> dict[str, list[str]]:
    lists: dict[str, list[str]] = {}
    lines = text.splitlines()
    i = 0
    assign_re = re.compile(r"^([A-Z][A-Z0-9_]*)\s*:?=\s*(.*)$")
    while i < len(lines):
        raw = lines[i]
        if raw.lstrip().startswith("#") or not raw.strip():
            i += 1
            continue
        m = assign_re.match(raw)
        if not m:
            i += 1
            continue
        name, rest = m.group(1), m.group(2).rstrip()
        chunks = [rest]
        while chunks[-1].endswith("\\"):
            chunks[-1] = chunks[-1][:-1].rstrip()
            i += 1
            if i >= len(lines):
                break
            chunks.append(lines[i].rstrip())
        tokens: list[str] = []
        for chunk in chunks:
            stripped = chunk.strip()
            if not stripped or stripped.startswith("#"):
                continue
            tokens.extend(stripped.split())
        lists[name] = tokens
        i += 1
    return lists


def expand_list(lists: dict[str, list[str]], name: str, seen: set[str] | None = None) -> list[str]:
    if seen is None:
        seen = set()
    if name in seen:
        raise RuntimeError(f"cycle expanding {name}")
    seen.add(name)
    out: list[str] = []
    for token in lists.get(name, []):
        ref = re.fullmatch(r"\$\(([A-Z][A-Z0-9_]*)\)", token)
        if ref:
            out.extend(expand_list(lists, ref.group(1), seen))
        elif token.startswith("$("):
            continue
        else:
            out.append(token)
    return out


def object_size(rel: str) -> int:
    src = ROOT / rel
    mini = src.with_suffix(".mini.o")
    full = src.with_suffix(".o")
    sizes = []
    if mini.exists():
        sizes.append(mini.stat().st_size)
    if full.exists():
        sizes.append(full.stat().st_size)
    return max(sizes) if sizes else src.stat().st_size


def main() -> int:
    lists = parse_make_lists(MK_PATH.read_text())
    missing_vars = [name for name in ENGINE_VARS + MINI_VARS if name not in lists]
    if missing_vars:
        print("missing make vars:", ", ".join(missing_vars))
        return 1

    engine_glob = sorted(
        f"src/{p.name}" for p in (ROOT / "src").glob("*.c") if f"src/{p.name}" not in EXCLUDED_ENGINE
    )
    mini_glob = sorted(f"src/mini/{p.name}" for p in (ROOT / "src/mini").glob("*.c"))

    classified: dict[str, str] = {}
    errors: list[str] = []
    expanded = {var: expand_list(lists, var) for var in ENGINE_VARS + MINI_VARS}
    for var in ENGINE_VARS:
        for path in expanded[var]:
            if path in classified:
                errors.append(f"duplicate {path} in {classified[path]} and {var}")
            classified[path] = var

    glob_set = set(engine_glob)
    class_set = set(classified)
    extra = sorted(class_set - glob_set)
    missing = sorted(glob_set - class_set)
    if extra:
        errors.append("mk files not in MINI_SHARED_ENGINE_SRCS: " + ", ".join(extra))
    if missing:
        errors.append("MINI_SHARED_ENGINE_SRCS missing from mk: " + ", ".join(missing))

    mini_classified: dict[str, str] = {}
    for var in MINI_VARS:
        for path in expanded[var]:
            if path in mini_classified:
                errors.append(f"duplicate mini {path} in {mini_classified[path]} and {var}")
            mini_classified[path] = var
    mini_extra = sorted(set(mini_classified) - set(mini_glob))
    mini_missing = sorted(set(mini_glob) - set(mini_classified))
    if mini_extra:
        errors.append("mk mini files not in src/mini: " + ", ".join(mini_extra))
    if mini_missing:
        errors.append("src/mini/*.c missing from mk: " + ", ".join(mini_missing))

    counts = {var: len(expanded[var]) for var in ENGINE_VARS + MINI_VARS}
    stub = expanded["PICO_STUB_SRCS"]
    fattest = sorted(stub, key=object_size, reverse=True)[:10]

    print(f"src/*.c glob={len(list((ROOT / 'src').glob('*.c')))}")
    print(f"MINI_SHARED_ENGINE_SRCS glob={len(engine_glob)}")
    print(
        "KEEP_SHIP1={c[PICO_KERNEL_SHIP1_SRCS]} KEEP_SHIP2={c[PICO_KERNEL_SHIP2_SRCS]} "
        "KEEP_SHIP3={c[PICO_KERNEL_SHIP3_SRCS]} STUB={c[PICO_STUB_SRCS]} sum={s}".format(
            c=counts,
            s=sum(counts[v] for v in ENGINE_VARS),
        )
    )
    print(
        "MINI keep={c[PICO_MINI_RUNTIME_KEEP_SRCS]} omit={c[PICO_MINI_RUNTIME_OMIT_SRCS]} "
        "mini glob={n}".format(c=counts, n=len(mini_glob))
    )
    print("10 fattest STUB (by .mini.o/.o/src bytes):")
    for path in fattest:
        print(f"  {object_size(path):8}  {path}")

    if errors:
        print("FAIL")
        for err in errors:
            print(" ", err)
        return 1
    print("OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
