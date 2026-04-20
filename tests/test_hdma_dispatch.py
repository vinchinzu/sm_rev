import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
FUNCS_H = ROOT / "src" / "funcs.h"
HDMA_DISPATCH_C = ROOT / "src" / "hdma_core.c"


def _dispatch_cases(prefix: str) -> set[str]:
    text = HDMA_DISPATCH_C.read_text()
    names = set(re.findall(rf"case\s+(fn{prefix}\w+):", text))
    return {name[2:] for name in names}


def _declared_functions(pattern: str) -> set[str]:
    text = FUNCS_H.read_text()
    return set(re.findall(pattern, text, re.MULTILINE))


def test_hdma_instr_dispatch_covers_declared_handlers():
    declared = _declared_functions(r"^const uint8 \*(HdmaobjInstr_\w+)\(")
    dispatch_cases = _dispatch_cases("HdmaobjInstr_")
    missing = sorted(declared - dispatch_cases)
    assert not missing, f"Missing HDMA instruction dispatch cases: {missing}"


def test_hdma_preinstr_dispatch_covers_declared_handlers():
    declared = _declared_functions(r"^void\s+(HdmaobjPreInstr_\w+)\(")
    dispatch_cases = _dispatch_cases("HdmaobjPreInstr_")
    missing = sorted(declared - dispatch_cases)
    assert not missing, f"Missing HDMA pre-instruction dispatch cases: {missing}"
