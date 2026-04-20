import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
FUNCS_H = ROOT / "src" / "funcs.h"
PLM_DISPATCH_C = ROOT / "src" / "plm_dispatch.c"


def _dispatch_cases(prefix: str) -> set[str]:
    text = PLM_DISPATCH_C.read_text()
    names = set(re.findall(rf"case\s+(fn{prefix}\w+):", text))
    return {name[2:] for name in names}


def _declared_functions(pattern: str) -> set[str]:
    text = FUNCS_H.read_text()
    return set(re.findall(pattern, text, re.MULTILINE))


def test_plm_instr_dispatch_covers_declared_handlers():
    declared = _declared_functions(r"^const uint8 \*(PlmInstr_\w+)\(")
    dispatch_cases = _dispatch_cases("PlmInstr_")
    missing = sorted(declared - dispatch_cases)
    assert not missing, f"Missing PLM instruction dispatch cases: {missing}"


def test_plm_header_dispatch_covers_declared_setups():
    declared = _declared_functions(r"^uint8\s+(PlmSetup_\w+)\(")
    dispatch_cases = _dispatch_cases("PlmSetup_")
    missing = sorted(declared - dispatch_cases)
    assert not missing, f"Missing PLM setup dispatch cases: {missing}"


def test_plm_preinstr_dispatch_covers_declared_handlers():
    declared = _declared_functions(r"^void\s+(PlmPreInstr_\w+)\(")
    dispatch_cases = _dispatch_cases("PlmPreInstr_")
    missing = sorted(declared - dispatch_cases)
    assert not missing, f"Missing PLM pre-instruction dispatch cases: {missing}"
