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
| `src/room_header.c` | Shared room header/state metadata load helpers extracted from the door-transition path | `../sm/src/sm_82.c` | `LoadRoomHeader` `0x82DE6F`, `LoadStateHeader` `0x82DEF2` |
| `src/room_transition.c` | Door transition pipeline, background load, room asset load | `../sm/src/sm_82.c` | `GameState_9_HitDoorBlock` `0x82E169`, `DoorTransitionFunction_SetupScrolling` `0x82E38E`, `LoadLevelScrollAndCre` `0x82EA73`, `LoadLibraryBackground` |
| `src/room_setup.c` | Room setup callbacks and load-station setup | Mostly `../sm/src/sm_8f.c`; `LoadFromLoadStation` comes from Bank `$80` | `RunRoomSetupCode` `0x8FE88F`, `LoadFromLoadStation` `0x80C437` |
| `src/room_fx.c` | FX header load helpers and FX visual rebuild | `../sm/src/sm_89.c` | `LoadFXHeader`, `FxTypeFunc_*`, `RefreshFxVisualsAfterLoad` |
| `src/hdma_core.c` | Layer blending, HDMA object lifecycle, and generic HDMA dispatch | `../sm/src/sm_88.c` | `LayerBlendingHandler` `0x888000`, `EnableHdmaObjects` `0x888288`, `InitializeSpecialEffectsForNewRoom` `0x8882C1`, `SpawnHdmaObject` `0x888435`, `HdmaObjectHandler` `0x8884B9` |
| `src/hdma_power_bomb.c` | Power-bomb explosion and crystal-flash HDMA families | `../sm/src/sm_88.c` | `SpawnPowerBombExplosion` `0x888AA4`, `HdmaobjPreInstr_PowerBombExplode_*` `0x888B8F-0x8891A8`, `SpawnCrystalFlashHdmaObjs` `0x88A2A6` |
| `src/boss_hdma.c` | Boss and room-event HDMA families: Draygon room gate, Phantoon wave, Mother Brain rainbow beam, Morph Ball eye beam | `../sm/src/sm_88.c` | `sub_88DF34` `0x88DF34`, `HdmaobjPreInstr_DF94` `0x88DF94`, `sub_88E487` `0x88E487`, `InitializeRainbowBeam` `0x88E767`, `SpawnMorphBallEyeBeamHdma` `0x88E8D9` |
| `src/cinematic_hdma.c` | Title gradient and intro crossfade HDMA families | `../sm/src/sm_88.c` | `SpawnTitleScreenGradientObjs` `0x88EB58`, `SpawnIntroCutsceneCrossfadeHdma` `0x88EBF0`, `CinematicFunction_Intro_Func133` `0x88EC3B` |
| `src/room_fx_hdma.c` | Room FX HDMA runtime families: scrolling sky, liquids, weather, Tourian statue, haze | `../sm/src/sm_88.c` | `FxTypeFunc_22_ScrollingSky` `0x88A61B`, `FxTypeFunc_2_Lava` `0x88B279`, `FxTypeFunc_6_Water` `0x88C3FF`, `FxTypeFunc_2C_Haze` `0x88DDC7` |
| `src/samus_camera_map.c` | Main gameplay camera follow, scrolling handoff, minimap updates | `../sm/src/sm_90.c` | `MainScrollingRoutine` `0x9094EC`, `Samus_HandleScroll_X` `0x9095A0`, `Samus_HandleScroll_Y` `0x90964F`, `UpdateMinimap` `0x90A91B` |
| `src/game_init.c` | Game bootstrap, demo-room setup, main gameplay load path | `../sm/src/sm_82.c` | `InitAndLoadGameData_Async` `0x828000`, `LoadGameData_Async`, `LoadDemoRoomData` |
| `src/demo_manager.c` | Demo-room positioning and scroll init | `../sm/src/sm_82.c` | `LoadDemoRoomData` helpers and demo-room scroll setup |

## Samus Runtime Clusters

| Current file | Original bank file in `../sm/` | Notes |
| --- | --- | --- |
| `src/samus_runtime.c` | `../sm/src/sm_90.c` and `../sm/src/sm_91.c` | Main Samus frame handler/state dispatcher plus bootstrap helpers; now also owns `MoveSamusWithControlPad`, `Samus_UpdateSpeedEchoPos`, `Samus_Initialize`, and `Samus_Movement_1A_GrabbedByDraygon` from the old `physics.c` dispatch cluster |
| `src/samus_air.c` | `../sm/src/sm_90.c` | Air-state movement handlers peeled out of `physics.c`; currently owns `Samus_Movement_02_NormalJumping`, `Samus_Movement_03_SpinJumping` `0x90A436`, `Samus_Movement_06_Falling`, `Samus_Movement_14_WallJumping`, `Samus_Movement_17_TurningAroundJumping`, `Samus_Movement_18_TurningAroundFalling`, and `Samus_Movement_19_DamageBoost` |
| `src/samus_ball.c` | `../sm/src/sm_90.c` | Morph-ball and spring-ball movement handlers peeled out of `physics.c`; currently owns `Samus_Movement_04_MorphBallOnGround`, `Samus_Movement_08_MorphBallFalling`, `Samus_Movement_11_SpringBallOnGround`, `Samus_Movement_12_SpringBallInAir`, and `Samus_Movement_13_SpringBallFalling` |
| `src/samus_ground.c` | `../sm/src/sm_90.c` | Grounded movement handlers peeled out of `physics.c`; currently owns `Samus_Movement_00_Standing`, `Samus_Movement_01_Running`, `Samus_Movement_05_Crouching`, `Samus_Movement_0E_TurningAroundOnGround`, `Samus_Movement_10_Moonwalking`, and `Samus_Movement_15_RanIntoWall` |
| `src/samus_draw.c` | `../sm/src/sm_90.c` | Samus draw/spritemap composition |
| `src/samus_speed.c` | `../sm/src/sm_90.c` | Horizontal speed tables and speed booster helpers |
| `src/samus_jump.c` | `../sm/src/sm_90.c` | Jump/gravity selection |
| `src/samus_palette.c` | `../sm/src/sm_91.c` and `../sm/src/sm_88.c` | Samus palette runtime plus suit-pickup setup and HDMA state machine |
| `src/samus_collision.c` | `../sm/src/sm_90.c`, `../sm/src/sm_91.c`, Bank `$94` code paths | Movement-facing collision basics: displacement helpers, no-coll moves, slope align, wall-jump gates |
| `src/samus_collision_block.c` | `../sm/src/sm_90.c`, `../sm/src/sm_91.c`, Bank `$94` code paths | Block-type dispatch, slope/solid reactions, block traversal, and movement-facing block collision handlers peeled out of `samus_collision_advanced.c` on 2026-04-21 |
| `src/samus_collision_map.c` | `../sm/src/sm_90.c`, `../sm/src/sm_91.c`, Bank `$94` code paths | Shared block-index probe (`CalculateBlockAt`) peeled out of `samus_collision_advanced.c` on 2026-04-21 so mini and full build can share the same map lookup |
| `src/samus_collision_advanced.c` | `../sm/src/sm_90.c`, `../sm/src/sm_91.c`, Bank `$94` code paths | Pose-change collision reconciliation and inside-block detection; block traversal/dispatch peeled further into `samus_collision_block.c` on 2026-04-21 |
| `src/samus_transition.c` | `../sm/src/sm_91.c` and `../sm/src/sm_90.c` | Pose-transition state machine plus the crouch/stand transition movement handler `Samus_Movement_0F_CrouchingEtcTransition` peeled from `physics.c` |
| `src/samus_grapple.c` | `../sm/src/sm_9b.c` and `../sm/src/sm_90.c` | Grapple logic was split across Samus banks before extraction; now also owns `Samus_Movement_16_Grappling` from the old `physics.c` dispatch cluster |
| `src/samus_special_move.c` | `../sm/src/sm_90.c` | Shinespark, bomb-jump, knockback, and related special-movement helpers; now also owns `Samus_Movement_0A_KnockbackOrCrystalFlashEnding` and `Samus_Movement_1B_ShinesparkEtc` from the old `physics.c` dispatch cluster |
| `src/samus_xray.c` | `../sm/src/sm_91.c` and `../sm/src/sm_88.c` | X-Ray scope state machine, block scan, setup stages, and HDMA runtime |
| `src/samus_projectile_state.c` | `../sm/src/sm_90.c` | Projectile slot reset/clear/kill lifecycle from `ResetProjectileData` `0x90AD22`, `ClearProjectile` `0x90ADB7`, and `KillProjectile` `0x90AE06` |
| `src/samus_projectile_weapon.c` | `../sm/src/sm_90.c` | Beam palette/cooldown/fire setup from `Samus_HandleCooldown` `0x90AC1C`, `UpdateBeamTilesAndPalette` `0x90AC8D`, `FireUnchargedBeam` `0x90B8D6`, `FireChargedBeam` `0x90B99E`, `InitProjectilePositionDirection` `0x90BA56`, `FireHyperBeam` `0x90BCD1`, and `ProjectileReflection` `0x90BE00` |
| `src/samus_projectile_core.c` / `src/samus_projectile_beam.c` / `src/samus_projectile_block.c` / `src/samus_projectile_view.c` | `../sm/src/sm_90.c`, `../sm/src/sm_93.c`, and Bank `$94` code paths | Remaining projectile instruction/pre-instruction runtime, projectile draw/explosion helpers, beam/missile/bomb/SBA HUD behavior, block reactions, and read-only mini projectile views |

## Enemy / Combat Helpers

| Current file | Original bank file in `../sm/` | Notes |
| --- | --- | --- |
| `src/enemy_main.c` | `../sm/src/sm_a0.c` | Shared enemy lifecycle, spawn/load path, frame dispatch, and draw/runtime plumbing after the Bank `$A0` split |
| `src/enemy_tiles.c` | `../sm/src/sm_a0.c` | Shared enemy tileset selection, palette staging, RAM tile assembly, and enemy-VRAM transfer helpers extracted from Bank `$A0` |
| `src/enemy_gunship.c` | `../sm/src/sm_a2.c` | Gunship-only enemy runtime: landing-site idle/save interaction, event-driven departure, and takeoff choreography |
| `src/enemy_elevator.c` | `../sm/src/sm_a3.c` | Elevator runtime peeled from the mixed Bank `$A3` file; owns the Samus/platform sync state machine and elevator-triggered transition handoff |
| `src/enemy_fauna.c` | `../sm/src/sm_a3.c` | Remaining fauna/hazard runtime peeled from the mixed Bank `$A3` file; owns the Waver/Metalee/Fireflea/fish-crab-slug/Roach/Sidehopper/Bang/Skree/Maridia-snail/Reflec/Zoomer families and related small-enemy hazards |
| `src/enemy_metroid.c` | `../sm/src/sm_a3.c` | Metroid runtime peeled from the mixed Bank `$A3` file; owns the sprite-linked chase, latch, freeze, hurt, and item-drop behavior |
| `src/enemy_mochtroid.c` | `../sm/src/sm_a3.c` | Mochtroid chase/contact runtime peeled from the mixed Bank `$A3` file; now owns the full state machine between Roach and Sidehopper |
| `src/enemy_torizo.c` | `../sm/src/sm_aa.c` | Bomb/Golden Torizo runtime peeled from Bank `$AA`; owns the boss AI, instruction handlers, and palette choreography while Tourian statue/Shaktool remain in `sm_aa.c` |
| `src/enemy_collision.c` | `../sm/src/sm_a0.c` | Shared enemy-vs-Samus, enemy-vs-projectile, and block-collision helpers extracted from Bank `$A0` |
| `src/enemy_drops.c` | `../sm/src/sm_a0.c` | Enemy drops, grapple-death hooks, and respawn/item-drop helpers extracted from Bank `$A0` |
| `src/eproj_core.c` | `../sm/src/sm_86.c` | Enemy-projectile lifecycle, generic instruction handlers, shared block-collision/movement helpers, draw path, and screen-shake helpers |
| `src/eproj_environment.c` | `../sm/src/sm_86.c` | Environment/facility enemy-projectile families; currently owns the fake-wall / dust-cloud / shot-gate cluster (`EprojInit_TourianEscapeShaftFakeWallExplode`, `EprojInit_DustCloudOrExplosion`, `EprojPreInstr_DustCloudOrExplosion`, `EprojInit_SpawnedShotGate`, `EprojInit_ClosedDownwardsShotGate`, `EprojInit_ClosedUpwardsShotGate`, `EprojPreInstr_E605`, `CheckIfEprojIsOffScreen`), the lava / fireball cluster (`EprojInit_LavaSeahorseFireball`, `sub_86B535`, `EprojInit_NamiFuneFireball`, `EprojPreInstr_NamiFuneFireball`, `EprojInit_LavaThrownByLavaman`, `sub_86E049`), the eye-door/save-station cluster (`EprojInit_EyeDoorProjectile`, `EprojInit_EyeDoorSweat`, `EprojPreInstr_EyeDoorProjectile`, `EprojPreInstr_EyeDoorSweat`, `EprojInit_EyeDoorSmoke`, `EprojInit_SaveStationElectricity`), `EprojInit_NuclearWaffleBody`, and the Norfair lavaquake rocks cluster (`EprojInit_NorfairLavaquakeRocks` through `EprojPreInstr_NorfairLavaquakeRocks_Inner2`) |
| `src/eproj_tourian.c` | `../sm/src/sm_86.c` | Tourian and Mother Brain enemy-projectile families; currently owns the Tourian statue entrance/effects cluster (`EprojInstr_Earthquake`, `EprojInstr_SpawnTourianStatueUnlockingParticleTail`, `EprojInit_TourianStatueUnlockingParticleWaterSplash` through `EprojPreInstr_TourianStatueStuff`) and the Mother Brain room/projectile cluster (`EprojInit_MotherBrainRoomTurrets`, `Eproj_MotherBrainsBlueRingLasers`, `EprojInit_MotherBrainBomb`, `sub_86C605`, `EprojInit_MotherBrainDeathBeemFired`, `EprojInit_MotherBrainRainbowBeam`, `EprojInit_MotherBrainsDrool`, `EprojInit_MotherBrainsDeathExplosion`, `EprojInit_MotherBrainEscapeDoorParticles`, `EprojInit_MotherBrainTubeFalling`, `EprojInit_MotherBrainGlassShatteringShard`, plus their pre-instr/instr helpers) |
| `src/eproj_combat.c` | `../sm/src/sm_86.c` | Remaining combat-facing enemy-projectile families from the old Bank 86 file after the core/environment/Tourian peel. This includes the Skree/Draygon/Crocomire/Kraid/Mini-Kraid clusters, pirate/Ceres/Torizo/Shaktool projectile families, Ki-Hunter/Kago/Maridia/WS robot/N00b tube/spore families, Botwoon/Yapping Maw handlers, and the pickup/death-explosion/sparks tail. |
| `src/enemy_math.c` | Mostly `../sm/src/sm_a0.c`; enemy-projectile trig helpers also from `../sm/src/sm_86.c` | Shared math layer now owns `Math_MultBySin`, `Math_MultByCos`, `Math_MultBySinCos`, and `Eproj_AngleToSamus` after phase-4 cleanup |

## Save / Menu Infrastructure

| Current file | Original bank file in `../sm/` | Notes |
| --- | --- | --- |
| `src/save_sram.c` | `../sm/src/sm_81.c` | Save-slot SRAM read/write, checksum validation, and packed map persistence. Key anchors: `SaveToSram` `0x818000`, `LoadFromSram` `0x818085`, `UnpackMapFromSave` `0x8182E4`, `PackMapToSave` `0x81834B`, `SoftReset` |
| `src/spritemap_draw.c` | `../sm/src/sm_81.c` | Shared sprite/OAM draw helpers used across gameplay and menus. Key anchors: `DrawSpritemap` `0x81879F`, `DrawMenuSpritemap` `0x81891F`, `DrawSamusSpritemap` `0x8189AE`, `DrawProjectileSpritemap` `0x818A4B`, `DrawSpritemapWithBaseTile` `0x818AB8`, `DrawEprojSpritemapWithBaseTile` `0x818C0A` |
| `src/menu_common.c` | `../sm/src/sm_81.c` | Shared menu VRAM, palette, and tilemap helpers. Key anchors: `MapVramForMenu` `0x818DBA`, `LoadInitialMenuTiles` `0x818DDB`, `LoadMenuPalettes` `0x818E60`, `LoadFileSelectPalettes` `0x819486`, `LoadMenuTilemap` `0x81B3E2` |
| `src/game_over_menu.c` | `../sm/src/sm_81.c` | Game-over menu state machine and setup. Key anchors: `GameOverMenu` `0x8190AE`, `GameOverMenu_1_Init` `0x8191A4`, `GameOverMenu_4_Main` `0x81912B`, `GameOverMenu_6_LoadGameMapView` `0x819116` |
| `src/file_select_menu.c` | `../sm/src/sm_81.c` | File-select menu, copy/clear flows, save-slot summary draw, and helmet animation. Key anchors: `FileSelectMenu` `0x8193FB`, `FileSelectMenu_16` `0x819EF3`, `FileSelectMenu_4_Main` `0x81A1C2`, `FileSelectMenu_13_FileCopyDoIt` `0x819A2C`, `FileSelectMenu_25_FileClearDoClear` `0x819C9E` |
| `src/file_select_map.c` | `../sm/src/sm_81.c` | Area-select and room-select map flows, including expanding-square HDMA transitions. Key anchors: `FileSelectMap` `0x819E3E`, `FileSelectMap_6_AreaSelectMap` `0x81A800`, `FileSelectMap_10_RoomSelectMap` `0x81AD7F`, `SetupRoomSelectMapExpandingSquareTransHDMA` `0x81ABA7` |
| `src/menu_assets.h` | `../sm/src/sm_82.c` | Shared pause/map/options/equipment ROM tables and constants. Key anchors: `kPauseScreenPalettes` `0xB6F000`, `kMapIconDataPointers` `0x82C7CB`, `kOptionsMenuSpecialPtrs` `0x82F0AE`, `file_copy_arrow_stuff` `0x82BB0C` |

## Cinematics / Presentation

| Current file | Original bank file in `../sm/` | Notes |
| --- | --- | --- |
| `src/cinematics.c` | `../sm/src/sm_8b.c` | Title screen, intro, escape blackout, ending, credits, cinematic sprite/bg dispatch, and Mode 7 scene runtime. Key anchors: `SetupPpuForTitleSequence` `0x8B8000`, `CinematicFunctionOpening` `0x8B9B68`, `CinematicFunction_Intro_Initial` `0x8BA395`, `CinematicFunctionBlackoutFromCeres` `0x8BC11B`, `CinematicFunctionEscapeFromCebes` `0x8BD480` |
| `src/samus_death.c` | `../sm/src/sm_9b.c` | Samus death animation, palette swaps, and explosion game-state helpers. Key anchors: `StartSamusDeathAnimation` `0x9BB3A7`, `HandleSamusDeathSequence` `0x9BB441`, `GameState_24_SamusNoHealth_Explosion_1` `0x9BB710` |

## Quick lookup workflow

1. Find the current topical file and function.
2. Read the function comment for the original SNES address if present.
3. Open the matching file in `../sm/src/` and search for either the function name
   or the address comment.
4. Compare control flow first; only then refactor or rename.

## Known high-risk paths

- Room load + scrolling: `room_transition.c` + `room_scrolling.c` + `samus_camera_map.c`
- Save/load or load-station placement: `room_setup.c` + `game_init.c`
- FX + scroll interactions: `room_fx.c` + `room_fx_hdma.c` + `room_scrolling.c`

These are the first places to cross-check in `../sm/` when a refactor changes
camera pan, room alignment, tile streaming, or post-load behavior.
