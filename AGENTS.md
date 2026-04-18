# Refactoring Guide: From ASM-like to C-like

This project is a reverse engineering of Super Metroid into C. The initial porting process often results in "ASM-like" C code that mirrors the original machine-generated logic. Our goal is to refactor this into modern, readable, and maintainable C while preserving the full game and building a subtractive `mini` target for Samus-and-physics work.

## 0. Current Mission

There are now two compatible tracks:

- **Full build parity**: `make` must keep original full-game behavior.
- **Mini build growth**: `make mini` should evolve into a small Samus + physics sandbox by excluding unrelated systems first.

The mini path is intentionally **negative-first**:
- Keep core Samus / physics files as the long-term mini target.
- Exclude enemies, bosses, room systems, demos, save flow, pause/map/equipment screens, and audio until needed.
- Prefer build-level subtraction and narrow stubs over spraying `#ifdef` logic through unrelated code.

When doing refactor work, ask:
1. Does this improve the full build without changing behavior?
2. Does this help extract a clean Samus / physics slice for mini?
3. Can this dependency be moved, stubbed, or isolated instead of globally wrapped?

## 1. Modularization (Anti-Monolithic)

Avoid monolithic files based on ROM banks (e.g., `sm_91.c`). Instead, group code by **logical topic**.

- **Filename Convention**: Use `samus_*.c` for Samus subsystems, or descriptive names like `physics.c`, `enemy_ai.c`, etc.
- **Topic Clusters**: Group related functions together (e.g., all palette effects in `samus_palette.c`, all resource handling in `samus_resource.c`).
- **Encapsulation**: Keep helper functions `static` within their module if they aren't needed elsewhere.

## 2. Transitioning from ASM-like to C-like

### Remove Raw Memory Accesses
- **ASM-like**: `*(uint16*)RomFixedPtr(0x9383c1)`
- **C-like**: Use named constants or arrays defined in headers, or descriptive macros if the data is still in ROM.
- **Better**: Create a struct or array that maps to the data and use that.

### Structs over Offsets
- **ASM-like**: Accessing `projectile_variables[k >> 1]` and then `*((uint8 *)projectile_variables + k + 1)`.
- **C-like**: Use proper structs for complex data types. (e.g., `ProjectileDataTable`, `EnemyData`).

### Control Flow
- Replace `goto` chains and "fall-through" logic with `if/else`, `switch/case`, and `for/while` loops.
- Use `bool` or `sign16()` instead of bitwise checks like `(val & 0x8000)`.

## 3. Eliminating Magic Numbers

Magic numbers make code hard to read and sensitive to changes.

### Bitwise Flags
Replace bitmask checks with named constants.
- **Bad**: `if (projectile_type[v1] & 0x200)`
- **Good**: `if (projectile_type[v1] & kProjectileType_SuperMissile)`

### SFX and IDs
Use enums or defines for sound effects and internal IDs.
- **Bad**: `QueueSfx2_Max6(7);`
- **Good**: `QueueSfx2_Max6(kSfx_MissileExplosion);` (if defined)

### Samus Poses and Movements
Use the constants in `ida_types.h`.
- **Bad**: `if (samus_pose == 0x9B)`
- **Good**: `if (samus_pose == kPose_9B_FaceF_VariaGravitySuit)`

## 4. Refactoring Workflow

1.  **Plan**: Identify a "cluster" in a monolithic file (see `docs/sm91_split_plan.md` for inspiration).
2.  **Extract**: Move the functions to a new or existing topical file.
3.  **Refactor**: Clean up the logic, remove magic numbers, and use structs.
4.  **Validate**: Run the smallest relevant checks:
    - `python3 -m pytest tests/test_build.py -q` for build integrity
    - `make mini-test` for mini shell smoke
    - `tests/test_headless.py` or `python3 tests/run_tests.py -v` when full runtime behavior may have changed

## 4A. Mini Build Plan

The current repo plan for `make mini` is:

1. **Shell first**: keep `sm_rev_mini` building and smoke-tested even before Samus runtime is linked.
2. **Extract startup code**: split reusable SDL/input/render/timing code out of `src/main.c`.
3. **Stub cross-system calls**: add a dedicated `stubs_mini.c` instead of ad hoc temporary hacks.
4. **Link Samus/physics in slices**:
   - first slice: `physics.c`, `physics_config.c`, `samus_input.c`, `samus_motion.c`, `samus_jump.c`, `samus_collision.c`
   - second slice: `samus_pose.c`, `samus_runtime.c`, `samus_draw.c`, `samus_speed.c`, `samus_transition.c`
5. **Keep heavy systems out**: `sm_*.c`, room code, enemy AI, bosses, demos, and audio stay excluded from mini until cleanly split.
6. **Promote tests with runtime growth**: once mini has real state, add deterministic headless assertions beyond shell smoke.

If a file mixes Samus and heavy full-game logic, the preferred move is to split the Samus cluster into a topical file before trying to link it into mini.

## 5. Naming and Topic Selection

When choosing a new file for a code cluster, follow these steps:

1.  **Identify the Core Object**: Is it Samus, an Enemy, a PLM, or a global subsystem (like Palette or Sound)?
2.  **Identify the Action**: Is it Animation, Physics, Collision, Input, or AI?
3.  **Combine following the pattern**:
    -   `samus_pose.c` (Samus + Animation/Pose logic)
    -   `samus_collision.c` (Samus + Collision logic)
    -   `samus_resource.c` (Samus + Health/Ammo resources)
    -   `physics.c` (Generic/Global physics dispatch)
4.  **Avoid generic names**: Don't use `util.c` or `helpers.c` for everything. Be as specific as possible.
5.  **Check for existing files**: Always check `src/` first to see if a logical home already exists for the code.

> [!TIP]
> Always aim for "Behavioral Parity" first. Refactor second. If you can do both in one commit while keeping tests passing, even better.
