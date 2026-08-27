# Per-file C-ify bar

Use this when cleaning an already-split `enemy_*.c` or `eproj_*.c` family
file. Split first; C-ify second. One family per patch. Work on `main`.

`enemy_elevator.c` is the current example of a file that meets the bar.

## Bar

A family file is done when all of these hold, or each leftover is listed in
the bead notes with a reason:

1. **Named tables.** ROM data lives as `static const` arrays or a small
   struct. No `#define g_word_* ((uint16*)RomFixedPtr(...))` and no new
   `RomFixedPtr` macros. Copy the table when its size is known (PJ bank log
   or a bounded loop). Keep a `RomFixedPtr` only when the size is unknown;
   note that leftover.
2. **Named identifiers.** `g_word_A*` / `g_off_A*` / `g_byte_A*` become
   `kFamilyThing`. Scalars become enum constants. Instruction-list pointers
   belong in `ida_types.h` as `addr_kFamily_Ilist_*`.
3. **Control flow.** No `uint16 v0` leftovers, no `LABEL_*`, no `goto` for
   ordinary branching. `if` / `else` / `switch` / `for`. `sign16()` or a
   local helper instead of `(val & 0x8000)`.
4. **Named magics in hot paths.** `kSfx1_*` / `kSfx2_*` / `kSfx3_*` (file
   enum is fine), `kEnemyProps_*`, `kProjectileType_*`, `kPose_*`. Music
   track IDs get a local `kMusic_*` / `kFamilyMusic_*` name.
5. **Structs over overlays.** Indexed arrays and typed structs instead of
   `r18_r20` and `(uint8*)` overlays on word arrays.
6. **Size.** The pass must not grow the file past ~1k to "finish" it. A
   2k boss stays one file; C-ify in place. Do not split for line count.

## Steps

1. Count `RomFixedPtr`, `g_word_`, `v0`, `LABEL_`, `goto`, and raw
   `QueueSfx*` / `QueueMusic*` in the file. That list is the work.
2. Identify each table from the sibling `../sm/src/sm_*.c` function and the
   PJ bank log (`https://patrickjohnston.org/ASM/ROM data/Super Metroid/`).
3. Copy tables, rename, rewrite `goto`/`LABEL_`/`v0`, name SFX/music/props.
   Leave unknown EnemyDef words and unique nullsubs alone.
4. Rebuild (`make -j`) and run
   `uvx pytest tests/test_build.py tests/test_enemy_ai_canon.py -q`.
   If a runtime path moved, also run the headless RAM/300/smoke subset.
5. If something still fails the bar, write the leftover list in the bead
   notes (symbol + why) instead of guessing.

## Guardrails

- Preserve behavior. Compare surprising changes against `../sm/src/sm_*.c`.
- Do not C-ify inside a leftover mixed dump; split that family first.
- Do not resurrect `enemy_dispatch.c` or a 24-bit AI switch.
- Do not copy deleted `Enemy_Normal*AI_A2..B3` wrappers into the file.
- `pytest` is `uvx pytest`, not system Python.
