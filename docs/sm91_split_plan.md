# sm_91.c Split Plan

`sm_91.c` (Bank $91, "Aran") — 4075 lines, 272 functions — contains seven largely independent subsystems lumped together by ROM bank. The goal is to split them into focused files that match the existing `samus_*.c` conventions, minimize cross-file dependency, and make each unit independently testable.

---

## Existing context

Already-split files from Bank $90:

| File | Contents |
|------|----------|
| `physics.c` | Movement handler dispatch, core movement loop |
| `samus_motion.c` | Horizontal/vertical movement primitives |
| `samus_jump.c` | Jump/walljump/knockback vertical-speed initialisation |
| `samus_collision.c` | Solid-block and enemy collision detection |
| `samus_speed.c` | Speed booster acceleration |
| `samus_special_move.c` | Shinespark, crystal-flash, bomb-jump, knockback entry |
| `samus_projectile.c` | Projectile-adjacent Samus state |
| `samus_camera_map.c` | Camera tracking and minimap updates |

---

## Logical clusters in sm_91.c

### Cluster A — Input dispatch (L1–L307)
**Functions:** `Samus_InputHandler`, `Samus_Input_00` – `Samus_Input_1B`, `Samus_LookupTransitionTable`, `TranslateCustomControllerBindingsToDefault`, `Samus_Pose_CancelGrapple`, `Samus_Pose_Func2`

**What it does:** Routes the per-frame joypad state through a per-movement-type handler, then walks the `kPoseTransitionTable` to derive `samus_new_pose`. Also remaps custom button bindings to default button indices.

**Target:** → new **`samus_input.c`**

---

### Cluster B — Demo / replay system (L308–L645)
**Functions:** `EnableDemoInput`, `DisableDemoInput`, `ResetDemoData`, `LoadDemoInputObject`, `DemoObjectInputHandler`, `CallDemoPreInstr`, `CallDemoInstr`, `ProcessDemoInputObject`, `DemoInstr_Finish/SetPreInstr/ClearPreInstr/Goto/DecTimerAndGoto/SetTimer/Func2/Func3/Disable`, `DemoPreInstr_864F/866A/CheckLeaveDemo/8AB0`, `DemoSetFunc_0`–`7`, `DemoSetFunc_Common`, `LoadDemoData`

**What it does:** Complete demo/attract-mode playback: loads a demo scene definition (items, position), drives a scripted input stream, and switches frame-handler pointers to replay handlers.

**Target:** → new **`samus_demo.c`**

---

### Cluster C — X-ray scope HDMA rendering (L646–L1294)
**Functions:** `XrayFillUp`, `XrayFillDown`, `XrayFillUpRamped`, `XrayFillDownRamped`, `XrayHdmaFunc_BeamAimed*`, `XrayHdmaFunc_BeamHoriz`, `XrayHdmaOnScreen_Beam*`, `CalculateXrayHdmaTableInner`, `XrayRunHandler`, `Xray_SetupStage1`–`8`, `Xray_SetupStage4_Func1`–`3`, `Xray_GetXrayedBlock`, `Xray_CombinedMove`, `Xray_Func7`–`16`, `CallXrayFunc`, `LoadBlockToXrayTilemap`, `CanXrayShowBlocks`, `HdmaobjPreInstr_XraySetup`, `Xray_SetupStage8_SetBackdropColor`

**What it does:** All HDMA table generation for the X-ray beam visual (ramp/fill geometry), the multi-stage setup that builds the x-ray tile maps from VRAM, and the per-object HDMA pre-instruction handler.

**Target:** → new **`samus_xray.c`** (rendering half)

---

### Cluster D — Suit-pickup cutscene HDMA (L1295–L1392)
**Functions:** `VariaSuitPickup`, `GravitySuitPickup`, `InitializeSuitPickupHdma`

**What it does:** Resets Samus physics state and spawns the light-beam HDMA object shown during Varia/Gravity suit pickups.

**Note:** Small (≈98 lines); cohesive with palette setup. Merge into **`samus_palette.c`** (below), since it primarily sets palette/HDMA registers and leads directly into the palette transition loop.

---

### Cluster E — Samus palette effects (L1393–L1852)
**Functions:** `Samus_HandlePalette`, `HandleBeamChargePalettes`, `HandleVisorPalette`, `HandleMiscSamusPalette`, `Samus_HandleScrewAttackSpeedBoostingPals`, `Samus_SpeedBoosterShinePals`, `Samus_HandleShinesparkingPals`, `Samus_HandleCrystalFlashPals`, `Samus_Copy10PalColors`, `Samus_Copy6PalColors`, `Samus_HandleXrayPals`, `nullsub_164`, `ApplyPad2PalettePrototype`, `ApplyPad2VisorFlare`, `CopyToSamusSuitPalette`, `CopyToSamusSuitTargetPalette`, `Samus_CancelSpeedBoost`, `Samus_LoadSuitPalette`, `Samus_LoadSuitTargetPalette`

**What it does:** Per-frame dispatch through the `off_91D72D` palette-handler table; handles beam charge glow, screw-attack / speed-booster / shinespark / crystal-flash / x-ray palette cycling; suit palette load/cancel.

**Target:** → new **`samus_palette.c`** (absorbs Cluster D)

---

### Cluster F — Resource management (L1854–L1912)
**Functions:** `Samus_RestoreHealth`, `Samus_DealDamage`, `Samus_RestoreMissiles`, `Samus_RestoreSuperMissiles`, `Samus_RestorePowerBombs`

**What it does:** Add/subtract health and ammo with max-cap and reserve-overflow logic.

**Target:** → new **`samus_resource.c`** (small, ≈60 lines, but logically distinct — item pickup routines from Bank $82/$84 already call these)

---

### Cluster G — Samus initialisation (L1913–L1993)
**Functions:** `Samus_Initialize`

**What it does:** Zeroes the Samus RAM block, sets up handler pointers and initial flags, triggers `LoadDemoData` for demo mode.

**Target:** → **`physics.c`** (alongside the other top-level Samus entry points; `Samus_Initialize` is called from Bank $82's game-state machine)

---

### Cluster H — X-ray trigger and standup glitch (L1994–L2081)
**Functions:** `Xray_Initialize`, `ResponsibleForXrayStandupGlitch`

**What it does:** Checks preconditions for activating x-ray (pose, state, liquid level), sets `time_is_frozen_flag`, spawns the HDMA object. `ResponsibleForXrayStandupGlitch` restores Samus after x-ray exits.

**Target:** → **`samus_xray.c`** (completes the x-ray subsystem, Cluster C + H together)

---

### Cluster I — Cinematic / scripted Samus state (L2082–L2220)
**Functions:** `MakeSamusFaceForward`, `SomeMotherBrainScripts`, `SomeMotherBrainScripts_0`–`4`

**What it does:** Puts Samus into the "facing the camera" pose used by demos and the opening. `SomeMotherBrainScripts` drives the Mother Brain Phase-2 cutscene (drained poses, hyper-beam enable, anim-frame overrides).

**Target:** → **`samus_special_move.c`** (already holds crystal-flash ending and shinespark setup; boss-fight scripted states fit that file's "exceptional movement states" theme)

---

### Cluster J — Speed-boost momentum state (L2221–L2296)
**Functions:** `nullsub_17`, `SamusFunc_E633`, `SamusFunc_E633_0/3/4/17/20`, `Samus_UpdatePreviousPose_0`

**What it does:** Per-frame check for speed-booster momentum: starts the boost counter, resets speed-echo positions, manages grapple / samus charge SFX. `Samus_UpdatePreviousPose_0` promotes `samus_pose` → `samus_prev_pose`.

**Target:** → **`samus_speed.c`** (existing file, logically contiguous with speed-booster acceleration code)

---

### Cluster K — Pose transition state machine (L2297–L3135)
**Functions:** `Samus_HandleTransFromBlockColl`, `Samus_HandleTransFromBlockColl_1`–`5`, `Samus_HandleTransFromBlockColl_1_0`–`5`, `Samus_CheckWalkedIntoSomething`, `Samus_HandleTransitions`, `Samus_HandleTransitionsA_1/2/5/6/7/8`, `Samus_HandleTransitionsB_1`–`10`, `Samus_HandleTransitionsA_5_1`–`6`, `Samus_HandleTransitionsC_1`–`8`, `HandleLandingSoundEffectsAndGfx`, `HandleLandingGraphics`, `HandleLandingGraphics_Crateria/Brinstar/Norfair/Maridia/Tourian/Ceres`, `SamusFunc_F1D3`, `Samus_MorphBallBounceNoSpringballTrans`, `Samus_MorphBallBounceSpringballTrans`, `Samus_HandleTransitionsA_5_1_0/2/5`

**What it does:** The three-tier transition machine (`samus_new_pose_transitional` → `samus_new_pose_interrupted` → `samus_new_pose`), dispatch tables A/B/C for momentum-routine index and hurt-switch index, landing sound/graphics by area, morph-ball bounce logic.

**Target:** → new **`samus_transition.c`**

---

### Cluster L — Pose parameter update and animation (L3136–L3796)
**Functions:** `SamusFunc_F404`, `SamusFunc_F433`, `SamusFunc_F468`, `SamusFunc_F468_Standing/Running/NormalJump/SpinJump/Crouching/Falling/MorphBall/CrouchTransEtc/Moonwalking/Springball/WallJumping/TurningAroundOnGround/TurnAroundJumping/TurnAroundFalling/DamageBoost/DamageBoost_/Shinespark/Unused`, `Samus_CrouchTrans`, `Samus_MorphTrans`, `Samus_StandOrUnmorphTrans`, `MaybeUnused_sub_91F7F4/91F840`, `MaybeUnused_sub_91F8B0`, `SamusFunc_FA0A`, `Samus_SetAnimationFrameIfPoseChanged`, `SamusFunc_EC80`

**What it does:** When a new pose is committed, `SamusFunc_F404` detects the change, calls collision adjustment, re-reads `pose_x_dir`, and dispatches through `SamusFunc_F468` to handle momentum carry-over, screw-attack / space-jump sound, shinespark movement-handler assignment, and animation-frame initialisation.

**Target:** → new **`samus_pose.c`**

---

### Cluster M — Jump transition initialisation (L3797–L3883)
**Functions:** `kHandleJumpTrans[]`, `HandleJumpTransition`, `HandleJumpTransition_NormalJump`, `HandleJumpTransition_SpinJump`, `HandleJumpTransition_SpringBallInAir`, `HandleJumpTransition_WallJump`, `UNUSED_sub_91FC42`, `Samus_Func20`

**What it does:** Per-movement-type dispatch that calls `Samus_InitJump` / `Samus_InitWallJump` when a jumping pose is entered. `Samus_Func20` handles the x-ray turn-around (angle flip + pose change) while frozen.

**Target:** → **`samus_jump.c`** (existing; `HandleJumpTransition` is the counterpart to `Samus_InitJump` already there)

---

### Cluster N — Collision-due-to-pose-change (L3884–L4075)
**Functions:** `HandleCollDueToChangedPose`, `HandleCollDueToChangedPose_Solid_NoColl/CollAbove/CollBelow/CollBoth`, `HandleCollDueToChangedPose_Block_NoColl/CollAbove/CollBelow/CollBoth`

**What it does:** When Samus's hitbox height changes (stand↔crouch↔morph), tests for solid-enemy and block collisions above and below, adjusts `samus_y_pos`, or reverts the pose if there is no room.

**Target:** → **`samus_collision.c`** (existing; logically extends the existing pose-aware collision helpers)

---

## Summary table

| Cluster | Lines | Destination file | New? |
|---------|-------|-----------------|------|
| A — Input dispatch | L1–307 | `samus_input.c` | ✅ new |
| B — Demo/replay | L308–645 | `samus_demo.c` | ✅ new |
| C+H — X-ray scope | L646–1294, L1994–2081 | `samus_xray.c` | ✅ new |
| D+E — Palette effects + suit pickup | L1295–1852 | `samus_palette.c` | ✅ new |
| F — Resource management | L1854–1912 | `samus_resource.c` | ✅ new |
| G — Samus initialisation | L1913–1993 | `physics.c` | extend |
| I — Cinematic/MB scripted state | L2082–2220 | `samus_special_move.c` | extend |
| J — Speed-boost momentum | L2221–2296 | `samus_speed.c` | extend |
| K — Transition state machine | L2297–3135 | `samus_transition.c` | ✅ new |
| L — Pose param update + anim | L3136–3796 | `samus_pose.c` | ✅ new |
| M — Jump transition init | L3797–3883 | `samus_jump.c` | extend |
| N — Collision due to pose change | L3884–4075 | `samus_collision.c` | extend |

**New files: 7** — `samus_input.c`, `samus_demo.c`, `samus_xray.c`, `samus_palette.c`, `samus_resource.c`, `samus_transition.c`, `samus_pose.c`

**Extended files: 4** — `physics.c`, `samus_special_move.c`, `samus_speed.c`, `samus_jump.c`, `samus_collision.c`

---

## Dependency notes

- `samus_pose.c` (`SamusFunc_F433`, `Samus_SetAnimationFrameIfPoseChanged`) is called from nearly every other cluster. Extract its header declarations first.
- `samus_transition.c` calls `HandleJumpTransition` (→ `samus_jump.c`) and `HandleCollDueToChangedPose` (→ `samus_collision.c`) — forward-declare or include those headers.
- `samus_xray.c` depends on `samus_palette.c` (`Samus_HandleXrayPals`) only indirectly via function-pointer table; no structural issue.
- `samus_demo.c` calls `EnableDemoInput`/`DisableDemoInput` (→ `samus_input.c`) and `Projectile_Func7_Shinespark` (→ `samus_special_move.c`).
- `samus_resource.c` is called-into only; no upstream dependencies beyond `variables.h`.

## Suggested porting order

1. `samus_pose.c` (Cluster L) — foundation called everywhere
2. `samus_transition.c` (Cluster K) — depends on pose, jump, collision
3. `samus_input.c` (Cluster A) — depends on pose/transition table
4. `samus_palette.c` (Clusters D+E) — self-contained
5. `samus_xray.c` (Clusters C+H) — depends on palette (load suit palette) and pose
6. `samus_demo.c` (Cluster B) — depends on input, pose, shinespark
7. `samus_resource.c` (Cluster F) — standalone
8. Extensions: physics.c + samus_speed.c + samus_jump.c + samus_collision.c + samus_special_move.c
