#!/usr/bin/env python3
"""Generate Die() stubs for pico-kernel TUs that are not compiled.

Rerun:
  python3 src/pico/generate_pico_stubs.py
  python3 src/pico/generate_pico_stubs.py --from-linker /tmp/pico-ld.log

Does not compile STUB TUs. Signatures come from src/funcs.h plus non-static
definitions in SHIP2 / SHIP3 / STUB sources. Handwritten no-ops live in
pico_stubs.c and are omitted here.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MK_PATH = Path(__file__).resolve().with_name("pico_kernel_sources.mk")
FUNCS_H = ROOT / "src" / "funcs.h"
OUT_PATH = Path(__file__).resolve().with_name("pico_stubs_generated.inc")

# Strong no-ops / special stubs authored in pico_stubs.c. Do not emit these.
HANDWRITTEN = {
    "RomPtr",
    "PicoStubDie",
    "QueueSfx1_Internal",
    "QueueSfx1_Max1",
    "QueueSfx1_Max15",
    "QueueSfx1_Max3",
    "QueueSfx1_Max6",
    "QueueSfx1_Max9",
    "QueueSfx2_Internal",
    "QueueSfx2_Max1",
    "QueueSfx2_Max15",
    "QueueSfx2_Max3",
    "QueueSfx2_Max6",
    "QueueSfx2_Max9",
    "QueueSfx3_Internal",
    "QueueSfx3_Max1",
    "QueueSfx3_Max15",
    "QueueSfx3_Max3",
    "QueueSfx3_Max6",
    "QueueSfx3_Max9",
    "QueueMusic_Delayed8",
    "QueueMusic_DelayedY",
    "HandleSoundEffects",
    "HandleMusicQueue",
    "ResetSoundQueues",
    "HasQueuedMusic",
    "EnablePaletteFx",
    "EnableHdmaObjects",
    "EnableAnimtiles",
    "ClearPaletteFXObjects",
    "PaletteFxHandler",
    "HdmaObjectHandler",
    "AnimtilesHandler",
    "Vector_NMI",
    "NMI_ProcessVramWriteQueue",
    "NMI_ProcessVramReadQueue",
    "NmiProcessAnimtilesVramTransfers",
    "ClearOamExt",
    "ClearUnusedOam",
    "CopyToVramNow",
    "EnableEprojs",
    "ClearEprojs",
    "EprojRunAll",
    "EprojSamusCollDetect",
    "EprojProjCollDet",
    "ProcessEnemyPowerBombInteraction",
    "SamusProjectileInteractionHandler",
    "DetermineWhichEnemiesToProcess",
    "EnemyMain",
    "LoadEnemies",
    "DrawSamusEnemiesAndProjectiles",
    "QueueEnemyBG2TilemapTransfers",
    "HandleRoomShaking",
    "HandleHudTilemap",
    "RunRoomMainCode",
    "HandleSamusOutOfHealthAndGameTile",
    "DecrementSamusTimers",
    "TransferEnemyTilesToVramAndInit",
    "RefreshFxVisualsAfterLoad",
    "InitializeSpecialEffectsForNewRoom",
    "LoadFXHeader",
    "DeletePlm",
    "LoadFromSram",
    "SaveToSram",
}

DECL_RE = re.compile(
    r"^((?:const\s+)?[A-Za-z_][\w\s\*]*)\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(([^;]*)\)\s*;\s*$"
)
DEF_RE = re.compile(
    r"^(?P<static>static\s+)?(?:(?:inline|NOINLINE|NORETURN|UNUSED)\s+)*"
    r"(?P<ret>(?:const\s+)?[A-Za-z_][\w\s\*]*)\s+"
    r"(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*\((?P<args>[^;{]*)\)\s*"
    r"(?:\{|//)",
    re.M,
)
UNDEF_RE = re.compile(r"undefined reference to [`']([^'`]+)[`']")


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


def parse_funcs_h(text: str) -> dict[str, tuple[str, str]]:
    decls: dict[str, tuple[str, str]] = {}
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped.endswith(");") or stripped.startswith("#"):
            continue
        m = DECL_RE.match(stripped)
        if not m:
            continue
        ret, name, args = m.group(1).strip(), m.group(2), m.group(3).strip()
        if name in ("if", "for", "while", "switch"):
            continue
        decls[name] = (ret, args)
    return decls


def parse_c_defs(text: str) -> dict[str, tuple[str, str]]:
    defs: dict[str, tuple[str, str]] = {}
    for m in DEF_RE.finditer(text):
        if m.group("static"):
            continue
        ret = re.sub(r"\s+", " ", m.group("ret")).strip()
        name = m.group("name")
        args = re.sub(r"\s+", " ", m.group("args")).strip()
        if name in ("if", "for", "while", "switch", "return"):
            continue
        if "typedef" in ret or ret.startswith("struct ") and "{" in m.group(0):
            continue
        defs[name] = (ret, args)
    return defs


NOOP_PREFIX_VOID = (
    "FxTypeFunc_",
)


def dummy_return(ret: str) -> str:
    compact = re.sub(r"\s+", " ", ret).strip()
    if compact == "void":
        return ""
    if compact in ("CoroutineRet",):
        return "  return kCoroutineNone;\n"
    if compact.endswith("*"):
        return "  return NULL;\n"
    if compact in ("bool",):
        return "  return false;\n"
    if compact in ("PairU16", "Point16U", "Pair_Bool_Amt", "CheckEnemyColl_Result", "Point32"):
        return f"  return ({compact}){{0}};\n"
    return "  return 0;\n"


def arg_voids(args: str) -> str:
    args = args.strip()
    if not args or args == "void":
        return ""
    names: list[str] = []
    for part in args.split(","):
        part = part.strip()
        if part == "..." or not part:
            continue
        part = part.replace("*", " ").replace("const", " ")
        tokens = part.split()
        if not tokens:
            continue
        name = tokens[-1]
        if name in ("void", "uint8", "uint16", "uint32", "int", "bool"):
            continue
        names.append(name)
    return "".join(f"  (void){n};\n" for n in names)


def emit_stub(name: str, ret: str, args: str, weak: bool, noop: bool) -> str:
    attr = "__attribute__((weak)) " if weak else ""
    body = arg_voids(args)
    if noop:
        extra = dummy_return(ret)
        if extra:
            body += extra
        elif not body:
            body = "  /* ship1: KEEP room load; FX/HDMA TUs are SHIP2. */\n"
    else:
        body += f'  PicoStubDie("{name}");\n'
        body += dummy_return(ret)
    return f"{attr}{ret} {name}({args}) {{\n{body}}}\n"


def parse_linker_undefs(text: str) -> list[str]:
    seen: set[str] = set()
    out: list[str] = []
    for m in UNDEF_RE.finditer(text):
        name = m.group(1)
        if name not in seen:
            seen.add(name)
            out.append(name)
    return out


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--from-linker", type=Path, default=None)
    parser.add_argument("-o", type=Path, default=OUT_PATH)
    args = parser.parse_args()

    lists = parse_make_lists(MK_PATH.read_text())
    stub_srcs = expand_list(lists, "PICO_STUB_SRCS")
    ship2_srcs = expand_list(lists, "PICO_KERNEL_SHIP2_SRCS")
    ship3_srcs = expand_list(lists, "PICO_KERNEL_SHIP3_SRCS")
    decls = parse_funcs_h(FUNCS_H.read_text())

    file_kind: dict[str, str] = {}
    file_defs: dict[str, dict[str, tuple[str, str]]] = {}
    for kind, srcs in (("stub", stub_srcs), ("ship2", ship2_srcs), ("ship3", ship3_srcs)):
        for rel in srcs:
            path = ROOT / rel
            if not path.exists():
                print(f"missing {rel}", file=sys.stderr)
                return 1
            file_kind[rel] = kind
            file_defs[rel] = parse_c_defs(path.read_text(errors="replace"))

    extra_undefs: list[str] = []
    if args.from_linker:
        extra_undefs = parse_linker_undefs(args.from_linker.read_text(errors="replace"))

    emitted: dict[str, str] = {}
    lines = [
        "/* Generated by src/pico/generate_pico_stubs.py — do not edit by hand. */",
        "/* Weak: SHIP2/SHIP3 (overridden when those TUs join the link). Strong: STUB TUs. */",
        "",
    ]
    counts = {"stub": 0, "ship2": 0, "ship3": 0, "linker": 0}

    for rel in stub_srcs + ship2_srcs + ship3_srcs:
        kind = file_kind[rel]
        weak = kind != "stub"
        for name, (cret, cargs) in sorted(file_defs[rel].items()):
            if name in HANDWRITTEN or name in emitted:
                continue
            if name not in decls:
                continue
            ret, argstr = decls[name]
            noop = name.startswith(NOOP_PREFIX_VOID)
            lines.append(emit_stub(name, ret, argstr, weak=weak, noop=noop))
            emitted[name] = rel
            counts[kind] += 1

    for name in extra_undefs:
        if name in HANDWRITTEN or name in emitted:
            continue
        if name not in decls:
            print(f"linker undef without signature: {name}", file=sys.stderr)
            continue
        ret, argstr = decls[name]
        noop = name.startswith(NOOP_PREFIX_VOID)
        lines.append(emit_stub(name, ret, argstr, weak=True, noop=noop))
        emitted[name] = "<linker>"
        counts["linker"] += 1

    args.o.write_text("\n".join(lines) + "\n")
    print(
        f"wrote {args.o.relative_to(ROOT)} stubs={len(emitted)} "
        f"stub={counts['stub']} ship2={counts['ship2']} ship3={counts['ship3']} "
        f"linker={counts['linker']}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
