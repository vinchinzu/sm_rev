# Bank-Origin Map

Use this when a refactored module regresses and you need to jump back to the
original bank-shaped source in the sibling `../sm/` tree.

The goal is not to keep the old structure alive. The goal is to make it fast to
answer: "where did this logic live before we split it?"

## Core rule

- Current topical files are the code we maintain.
- `../sm/src/sm_*.c` is the baseline for pre-refactor behavior and original
  function grouping.
- When you move more functions out of a monolithic bank file, extend this map in
  the same change.

## Room / Camera / Load Path

| Current file | Responsibility now | Original bank file in `../sm/` | Key entry points / anchors |
| --- | --- | --- | --- |
| `src/room_scrolling.c` | Room scrolling, background streaming, door-transition scrolling | `../sm/src/sm_80.c` | `DisplayViewablePartOfRoom` `0x80A176`, `HandleAutoscrolling_X` `0x80A528`, `HandleAutoscrolling_Y` `0x80A731`, `HandleScrollingWhenTriggeringScrollDown` `0x80A893`, `DoorTransitionScrollingSetup` `0x80AD36` |
| `src/room_transition.c` | Door transition pipeline, room header/state load, background load, room asset load | `../sm/src/sm_82.c` | `GameState_9_HitDoorBlock` `0x82E169`, `DoorTransitionFunction_SetupScrolling` `0x82E38E`, `LoadRoomHeader` `0x82DE6F`, `LoadStateHeader` `0x82DEF2`, `LoadLevelScrollAndCre` `0x82EA73`, `LoadLibraryBackground` |
| `src/room_setup.c` | Room setup callbacks and load-station setup | Mostly `../sm/src/sm_8f.c`; `LoadFromLoadStation` comes from Bank `$80` | `RunRoomSetupCode` `0x8FE88F`, `LoadFromLoadStation` `0x80C437` |
| `src/room_fx.c` | FX header load helpers and FX visual rebuild | `../sm/src/sm_89.c` | `LoadFXHeader`, `FxTypeFunc_*`, `RefreshFxVisualsAfterLoad` |
| `src/samus_camera_map.c` | Main gameplay camera follow, scrolling handoff, minimap updates | `../sm/src/sm_90.c` | `MainScrollingRoutine` `0x9094EC`, `Samus_HandleScroll_X` `0x9095A0`, `Samus_HandleScroll_Y` `0x90964F`, `UpdateMinimap` `0x90A91B` |
| `src/game_init.c` | Game bootstrap, demo-room setup, main gameplay load path | `../sm/src/sm_82.c` | `InitAndLoadGameData_Async` `0x828000`, `LoadGameData_Async`, `LoadDemoRoomData` |
| `src/demo_manager.c` | Demo-room positioning and scroll init | `../sm/src/sm_82.c` | `LoadDemoRoomData` helpers and demo-room scroll setup |

## Samus Runtime Clusters

| Current file | Original bank file in `../sm/` | Notes |
| --- | --- | --- |
| `src/samus_runtime.c` | `../sm/src/sm_90.c` | Main Samus frame handler and state dispatcher |
| `src/samus_draw.c` | `../sm/src/sm_90.c` | Samus draw/spritemap composition |
| `src/samus_speed.c` | `../sm/src/sm_90.c` | Horizontal speed tables and speed booster helpers |
| `src/samus_jump.c` | `../sm/src/sm_90.c` | Jump/gravity selection |
| `src/samus_collision.c` | `../sm/src/sm_90.c`, `../sm/src/sm_91.c`, Bank `$94` code paths | Mixed-origin collision work; confirm the function comment/header before editing |
| `src/samus_grapple.c` | `../sm/src/sm_9b.c` and `../sm/src/sm_90.c` | Grapple logic was split across Samus banks before extraction |

## Enemy / Combat Helpers

| Current file | Original bank file in `../sm/` | Notes |
| --- | --- | --- |
| `src/eproj_core.c` | `../sm/src/sm_86.c` | Enemy-projectile lifecycle, generic instruction handlers, shared block-collision/movement helpers, draw path, and screen-shake helpers |
| `src/eproj_environment.c` | `../sm/src/sm_86.c` | Environment/facility enemy-projectile families; currently owns the fake-wall / dust-cloud / shot-gate cluster (`EprojInit_TourianEscapeShaftFakeWallExplode`, `EprojInit_DustCloudOrExplosion`, `EprojPreInstr_DustCloudOrExplosion`, `EprojInit_SpawnedShotGate`, `EprojInit_ClosedDownwardsShotGate`, `EprojInit_ClosedUpwardsShotGate`, `EprojPreInstr_E605`, `CheckIfEprojIsOffScreen`), the lava / fireball cluster (`EprojInit_LavaSeahorseFireball`, `sub_86B535`, `EprojInit_NamiFuneFireball`, `EprojPreInstr_NamiFuneFireball`, `EprojInit_LavaThrownByLavaman`, `sub_86E049`), the eye-door/save-station cluster (`EprojInit_EyeDoorProjectile`, `EprojInit_EyeDoorSweat`, `EprojPreInstr_EyeDoorProjectile`, `EprojPreInstr_EyeDoorSweat`, `EprojInit_EyeDoorSmoke`, `EprojInit_SaveStationElectricity`), `EprojInit_NuclearWaffleBody`, and the Norfair lavaquake rocks cluster (`EprojInit_NorfairLavaquakeRocks` through `EprojPreInstr_NorfairLavaquakeRocks_Inner2`) |
| `src/eproj_tourian.c` | `../sm/src/sm_86.c` | Tourian and Mother Brain enemy-projectile families; currently owns the Tourian statue entrance/effects cluster (`EprojInstr_Earthquake`, `EprojInstr_SpawnTourianStatueUnlockingParticleTail`, `EprojInit_TourianStatueUnlockingParticleWaterSplash` through `EprojPreInstr_TourianStatueStuff`) and the Mother Brain room/projectile cluster (`EprojInit_MotherBrainRoomTurrets`, `Eproj_MotherBrainsBlueRingLasers`, `EprojInit_MotherBrainBomb`, `sub_86C605`, `EprojInit_MotherBrainDeathBeemFired`, `EprojInit_MotherBrainRainbowBeam`, `EprojInit_MotherBrainsDrool`, `EprojInit_MotherBrainsDeathExplosion`, `EprojInit_MotherBrainEscapeDoorParticles`, `EprojInit_MotherBrainTubeFalling`, `EprojInit_MotherBrainGlassShatteringShard`, plus their pre-instr/instr helpers) |
| `src/eproj_combat.c` | `../sm/src/sm_86.c` | Remaining combat-facing enemy-projectile families from the old Bank 86 file after the core/environment/Tourian peel. This includes the Skree/Draygon/Crocomire/Kraid/Mini-Kraid clusters, pirate/Ceres/Torizo/Shaktool projectile families, Ki-Hunter/Kago/Maridia/WS robot/N00b tube/spore families, Botwoon/Yapping Maw handlers, and the pickup/death-explosion/sparks tail. |
| `src/enemy_math.c` | Mostly `../sm/src/sm_a0.c`; enemy-projectile trig helpers also from `../sm/src/sm_86.c` | Shared math layer now owns `Math_MultBySin`, `Math_MultByCos`, `Math_MultBySinCos`, and `Eproj_AngleToSamus` after phase-4 cleanup |

## Quick lookup workflow

1. Find the current topical file and function.
2. Read the function comment for the original SNES address if present.
3. Open the matching file in `../sm/src/` and search for either the function name
   or the address comment.
4. Compare control flow first; only then refactor or rename.

## Known high-risk paths

- Room load + scrolling: `room_transition.c` + `room_scrolling.c` + `samus_camera_map.c`
- Save/load or load-station placement: `room_setup.c` + `game_init.c`
- FX + scroll interactions: `room_fx.c` + `room_scrolling.c`

These are the first places to cross-check in `../sm/` when a refactor changes
camera pan, room alignment, tile streaming, or post-load behavior.
