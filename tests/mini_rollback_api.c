#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "funcs.h"
#include "ida_types.h"
#include "mini/mini_defs.h"
#include "mini/mini_game.h"
#include "mini/mini_room_adapter.h"
#include "physics.h"
#include "samus_env.h"
#include "sm_rtl.h"
#include "types.h"
#include "variables.h"
#include "enemy_types.h"

#define kWalkedIntoSomethingPoseTable ((uint16 *)RomFixedPtr(0x91eb74))

enum {
  kStressFrameCount = 360,
  kRepeatedCycleCount = 16,
  kRepeatedCycleWindow = 24,
  kProjectileSearchFrames = 60,
  kProjectileAdvanceFrames = 6,
  kRomRoomFrameCount = 120,
  kPose_39_MaybeUnusedTransition = 0x39,
  kPoseCalcInput_ResetBallBounceLanding = 0x0401,
  kBlockCollisionInput_Landing = 1,
  kBlockCollisionInput_KeepPose = 3,
  kBlockCollisionInput_WallJump = 5,
  kBlockCollisionSubtype_JumpOrSpinLanding = 0,
  kBlockCollisionSubtype_BallLanding = 1,
  kBlockCollisionSubtype_AltBallLanding = 2,
  kBlockCollisionSubtype_SpringBallLanding = 3,
  kBlockCollisionSubtype_Ignore = 4,
  kSamusNoPendingPose = 0xFFFF,
  kSamusMomentumRoutine_None = 0,
  kSamusMomentumRoutine_BlockCollision = 5,
  kSamusMomentumRoutine_CrouchTransEtc = 7,
  kSamusHurtSwitch_Default = 0,
  kSamusHurtSwitch_Grapple = 3,
  kSamusSpecialTransGfx_None = 0,
  kSamusSpecialTransGfx_Grapple = 9,
  kBallBounce_KeepCurrentPose = 1,
  kSpringBallBounce_First = 0x0601,
  kSpringBallBounce_Second = 0x0602,
  kPose_42_BlockCollisionMorphLanding = 0x42,
  kKnockbackDir_NoHeldInput = 1,
  kKnockbackDir_XMomentum = 2,
  kKnockbackDir_HeldOpposite = 4,
  kKnockbackDir_XMomentumHeldOpposite = 5,
  kXrayAngle_FacingLeft = 64,
  kXrayAngle_FacingRight = 192,
  kXrayTransitionAnimFrame = 2,
  kXrayTransitionAnimTimer = 63,
  kXrayTransitionShineTimerDelay = 8,
  kGrappleTransitionMaxPrevDelta = 12,
  kTransitionA7SmallYOffset = 5,
  kCrateriaLandingGfxRoom_FxTypeGate = 0,
  kCrateriaLandingGfxRoom_YThresholdGate = 5,
  kCrateriaLandingGfxRoom_Always = 7,
  kCrateriaLandingGfxRoom_OutOfTable = 16,
  kCrateriaLandingGfxRoom_Norfair = 28,
  kCrateriaLandingGfxRequiredFxType = 10,
  kCrateriaLandingGfxYThreshold = 944,
  kLandingGfxMaridiaFrameType = 256,
  kLandingGfxNorfairFrameType = 1536,
  kPose_DD_TransitionBottomFrame2 = 0xDD,
  kXrayFirefleaFxType = 36,
  kXrayFirefleaBlendMask = 0x1000,
  kEnemyProps_SolidSamusCollisionContract = 0x8000,
  kMoonwalkTurnProjectileDirectionFlag = 0x100,
};

static void Require(bool condition, const char *message) {
  if (!condition) {
    fprintf(stderr, "%s\n", message);
    exit(1);
  }
}

static void ResetSfxQueuesForContracts(void);

static uint8 *AllocSnapshot(size_t *snapshot_size) {
  *snapshot_size = MiniSaveStateSize();
  uint8 *snapshot = (uint8 *)malloc(*snapshot_size);
  Require(snapshot != NULL, "snapshot allocation failed");
  return snapshot;
}

static uint16 PoseCalcInput(uint8 input, uint8 subtype) {
  return (uint16)((uint16)subtype << 8) | input;
}

static void SaveSnapshot(const MiniGameState *state, uint8 *snapshot, size_t snapshot_size) {
  Require(MiniSaveState(state, snapshot, snapshot_size), "MiniSaveState failed");
}

static void LoadSnapshot(MiniGameState *state, const uint8 *snapshot, size_t snapshot_size) {
  Require(MiniLoadState(state, snapshot, snapshot_size), "MiniLoadState failed");
}

static uint64_t StepShortSequenceAndHash(MiniGameState *state) {
  static const uint16 kInputs[] = {
    kButton_Right,
    kButton_Right | kButton_A,
    kButton_Right,
  };
  for (size_t i = 0; i < sizeof(kInputs) / sizeof(kInputs[0]); i++)
    MiniStepButtons(state, kInputs[i], false);
  return MiniStateHash(state);
}

static uint16 StressInputForFrame(int frame) {
  uint16 buttons = 0;
  switch ((frame / 18) % 4) {
  case 0:
    buttons |= kButton_Right;
    break;
  case 1:
    buttons |= kButton_Right | kButton_A;
    break;
  case 2:
    buttons |= kButton_Left;
    break;
  default:
    break;
  }
  if (frame % 29 == 5)
    buttons |= kButton_X;
  if (frame % 47 >= 10 && frame % 47 < 20)
    buttons |= kButton_B;
  return buttons;
}

static uint16 RomRoomInputForFrame(int frame) {
  switch ((frame / 20) % 4) {
  case 0:
    return kButton_Right;
  case 1:
    return kButton_Right | kButton_A;
  case 2:
    return 0;
  default:
    return kButton_Left;
  }
}

static uint64_t StepStressFrames(MiniGameState *state, int frames) {
  for (int i = 0; i < frames; i++)
    MiniStepButtons(state, StressInputForFrame(state->frame), false);
  return MiniStateHash(state);
}

static uint64_t StepRomRoomFrames(MiniGameState *state, int frames) {
  for (int i = 0; i < frames; i++)
    MiniStepButtons(state, RomRoomInputForFrame(state->frame), false);
  return MiniStateHash(state);
}

static void RequireTypedStateBoundary(const MiniGameState *state) {
  Require(state->viewport.width == state->viewport_width,
          "typed viewport width did not match compatibility field");
  Require(state->viewport.height == state->viewport_height,
          "typed viewport height did not match compatibility field");
  Require(state->viewport.camera_x == state->camera_x,
          "typed camera x did not match compatibility field");
  Require(state->viewport.camera_y == state->camera_y,
          "typed camera y did not match compatibility field");
  Require(state->room.has_room == state->has_room,
          "typed room availability did not match compatibility field");
  Require(state->room.uses_rom_room == state->uses_rom_room,
          "typed room source did not match compatibility field");
  Require(state->room.room_source == state->room_source,
          "typed room source enum did not match compatibility field");
  Require(state->samus.screen_x == state->samus_x,
          "typed Samus x did not match compatibility field");
  Require(state->samus.screen_y == state->samus_y,
          "typed Samus y did not match compatibility field");
  Require(state->samus.pose == state->samus_pose_value,
          "typed Samus pose did not match compatibility field");
  Require(state->samus.movement_type == state->samus_movement_type_value,
          "typed Samus movement type did not match compatibility field");
  Require(state->controls.buttons == state->last_buttons,
          "typed controls did not match compatibility field");
  Require(state->projectile_state.count == state->projectile_count,
          "typed projectile count did not match compatibility field");
  Require(state->collision_map.block_size == kMiniBlockSize,
          "typed collision map reported the wrong block size");
  Require(state->collision_map.width_blocks == state->room.room_width_blocks,
          "typed collision map width did not match typed room");
  Require(state->collision_map.height_blocks == state->room.room_height_blocks,
          "typed collision map height did not match typed room");
}

static const SamusProjectileView *FirstActiveProjectile(const MiniGameState *state) {
  for (int i = 0; i < kMiniProjectileViewCapacity; i++) {
    if (state->projectile_state.views[i].active)
      return &state->projectile_state.views[i];
  }
  return NULL;
}

static uint64_t StepProjectileAdvanceFrames(MiniGameState *state) {
  for (int i = 0; i < kProjectileAdvanceFrames; i++)
    MiniStepButtons(state, kButton_Right, false);
  return MiniStateHash(state);
}

static void RequireFallbackRoom(const MiniGameState *state) {
  RequireTypedStateBoundary(state);
  Require(state->has_room, "fallback mini state did not configure a room");
  Require(!state->uses_rom_room, "fallback mini test unexpectedly booted a ROM room");
  Require(state->room_source == kMiniRoomSource_Fallback,
          "fallback mini test did not use the fallback room");
  Require(MiniStubs_GetCollisionMaterial(-1, -1) == kBlockType_Solid,
          "typed collision material boundary did not treat out-of-room as solid");
}

static void TestLiquidEnvironmentContracts(void) {
  uint16 old_fx_y_pos = fx_y_pos;
  uint16 old_lava_acid_y_pos = lava_acid_y_pos;
  uint16 old_fx_liquid_options = fx_liquid_options;

  fx_liquid_options = 0;

  fx_y_pos = 100;
  lava_acid_y_pos = kLiquidYPos_Disabled;
  Require(Samus_GetLiquidEnvAt(120) == kSamusVerticalEnv_Water,
          "liquid environment did not classify active water as water");

  fx_y_pos = 200;
  lava_acid_y_pos = 100;
  Require(Samus_GetLiquidEnvAt(120) == kSamusVerticalEnv_Air,
          "active water layer did not take priority over submerged lava/acid");

  fx_y_pos = kLiquidYPos_Disabled;
  lava_acid_y_pos = 100;
  Require(Samus_GetLiquidEnvAt(120) == kSamusVerticalEnv_LavaAcid,
          "disabled water layer did not allow lava/acid classification");

  fx_y_pos = 100;
  lava_acid_y_pos = kLiquidYPos_Disabled;
  fx_liquid_options = kFxLiquidOptions_Passthrough;
  Require(Samus_GetLiquidEnvAt(120) == kSamusVerticalEnv_Air,
          "pass-through water did not classify as air");

  fx_y_pos = old_fx_y_pos;
  lava_acid_y_pos = old_lava_acid_y_pos;
  fx_liquid_options = old_fx_liquid_options;
}

static void TestBasicRollbackApi(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed");
  RequireFallbackRoom(state);

  MiniStepButtons(state, 0, false);
  MiniStepButtons(state, kButton_Right, false);
  uint64_t rewind_hash = MiniStateHash(state);

  size_t snapshot_size;
  uint8 *snapshot = AllocSnapshot(&snapshot_size);
  SaveSnapshot(state, snapshot, snapshot_size);
  Require(!MiniSaveState(state, snapshot, snapshot_size - 1),
          "MiniSaveState accepted a short buffer");

  uint64_t after_hash = StepShortSequenceAndHash(state);
  LoadSnapshot(state, snapshot, snapshot_size);
  Require(MiniStateHash(state) == rewind_hash, "loaded state hash did not match saved state");
  Require(StepShortSequenceAndHash(state) == after_hash, "re-simulated state hash did not match");

  snapshot[0] ^= 1;
  Require(!MiniLoadState(state, snapshot, snapshot_size),
          "MiniLoadState accepted a corrupted snapshot");

  free(snapshot);
  MiniDestroy(state);
}

static void TestLongScriptDeterminism(void) {
  MiniGameState *first = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(first != NULL, "MiniCreate failed for first long script test");
  RequireFallbackRoom(first);
  uint64_t first_hash = StepStressFrames(first, kStressFrameCount);
  MiniDestroy(first);

  MiniGameState *second = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(second != NULL, "MiniCreate failed for second long script test");
  RequireFallbackRoom(second);
  uint64_t second_hash = StepStressFrames(second, kStressFrameCount);
  Require(first_hash == second_hash, "long scripted mini run was not deterministic");
  Require(second->frame == kStressFrameCount, "long scripted mini run ended on the wrong frame");

  MiniDestroy(second);
}

static void TestRepeatedSaveLoadCycles(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for repeated save/load test");
  RequireFallbackRoom(state);

  size_t snapshot_size;
  uint8 *snapshot = AllocSnapshot(&snapshot_size);
  for (int cycle = 0; cycle < kRepeatedCycleCount; cycle++) {
    StepStressFrames(state, 7 + cycle);
    uint64_t saved_hash = MiniStateHash(state);
    SaveSnapshot(state, snapshot, snapshot_size);

    uint64_t branch_hash = StepStressFrames(state, kRepeatedCycleWindow);
    LoadSnapshot(state, snapshot, snapshot_size);
    Require(MiniStateHash(state) == saved_hash,
            "repeated MiniLoadState did not restore the saved hash");
    Require(StepStressFrames(state, kRepeatedCycleWindow) == branch_hash,
            "repeated rollback branch did not replay deterministically");
  }

  free(snapshot);
  MiniDestroy(state);
}

static void TestProjectileRollbackProgression(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for projectile rollback test");
  RequireFallbackRoom(state);

  for (int frame = 0; frame < kProjectileSearchFrames && state->projectile_state.count == 0; frame++) {
    MiniStepButtons(state, kButton_Right | kButton_X, false);
  }

  const SamusProjectileView *projectile = FirstActiveProjectile(state);
  Require(projectile != NULL, "shooting script did not spawn a projectile");
  Require(projectile->is_beam, "shooting script spawned a non-beam projectile");
  Require(projectile->is_basic_beam, "shooting script did not spawn a basic beam projectile");
  uint16 projectile_slot = projectile->slot_index;
  uint16 spawned_projectile_type = projectile->type;

  size_t snapshot_size;
  uint8 *snapshot = AllocSnapshot(&snapshot_size);
  uint64_t saved_hash = MiniStateHash(state);
  SaveSnapshot(state, snapshot, snapshot_size);

  uint64_t advanced_hash = StepProjectileAdvanceFrames(state);
  projectile = FirstActiveProjectile(state);
  Require(projectile != NULL, "projectile disappeared during short progression window");
  Require(projectile->slot_index == projectile_slot, "projectile changed slots unexpectedly");
  Require(projectile->type == spawned_projectile_type, "projectile type changed unexpectedly");
  Require(advanced_hash != saved_hash, "projectile progression window did not advance state");

  LoadSnapshot(state, snapshot, snapshot_size);
  Require(MiniStateHash(state) == saved_hash, "projectile snapshot did not restore saved hash");
  Require(StepProjectileAdvanceFrames(state) == advanced_hash,
          "projectile progression did not replay deterministically after load");

  free(snapshot);
  MiniDestroy(state);
}

static void ResetSuperMissileGateState(uint16 hud_item, uint16 counter, uint16 cooldown) {
  hud_item_index = hud_item;
  projectile_counter = counter;
  cooldown_timer = cooldown;
}

static void TestSuperMissileFireGateContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for super missile fire-gate test");
  RequireFallbackRoom(state);

  ResetSuperMissileGateState(2, 3, 0);
  Require(Samus_CanFireSuperMissile() == 1 &&
          projectile_counter == 4 &&
          cooldown_timer == 1,
          "selected super missile did not allow the fourth active projectile");

  ResetSuperMissileGateState(2, 4, 0);
  Require(Samus_CanFireSuperMissile() == 0 &&
          projectile_counter == 4 &&
          cooldown_timer == 0,
          "selected super missile allowed more than four active projectiles");

  ResetSuperMissileGateState(0, 4, 0);
  Require(Samus_CanFireSuperMissile() == 1 &&
          projectile_counter == 5 &&
          cooldown_timer == 1,
          "non-selected super missile path did not use the normal projectile limit");

  ResetSuperMissileGateState(0, 5, 0);
  Require(Samus_CanFireSuperMissile() == 0 &&
          projectile_counter == 5 &&
          cooldown_timer == 0,
          "non-selected super missile path allowed more than five active projectiles");

  ResetSuperMissileGateState(2, 3, 2);
  Require(Samus_CanFireSuperMissile() == 0 &&
          projectile_counter == 3 &&
          cooldown_timer == 2,
          "super missile fire gate ignored an active cooldown timer");

  MiniDestroy(state);
}

static void TestCrouchingTransitionContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for crouching transition test");
  RequireFallbackRoom(state);

  samus_pose = kPose_39_MaybeUnusedTransition;
  samus_y_dir = 0;
  samus_y_speed = 0;
  samus_y_subspeed = 0;
  enable_horiz_slope_coll = 0;
  UNUSEDword_7E0AA4 = 0xBEEF;
  input_to_pose_calc = 0;

  Samus_Movement_0F_CrouchingEtcTransition();
  Require(enable_horiz_slope_coll == 3,
          "crouching transition did not enable horizontal slope collision");
  Require(UNUSEDword_7E0AA4 == 0,
          "crouching transition did not clear the slope transition scratch word");
  Require(input_to_pose_calc == 0,
          "crouching transition did not clear pose-calc input after helper dispatch");

  samus_pose = kPose_DB;
  samus_y_subspeed = 0x1111;
  samus_y_speed = 0x2222;
  samus_y_dir = 2;
  used_for_ball_bounce_on_landing = 1;
  input_to_pose_calc = kPoseCalcInput_ResetBallBounceLanding;

  Samus_Movement_0F_CrouchingEtcTransition();
  Require(input_to_pose_calc == 0,
          "crouching transition did not clear pose-calc input after bounce reset");
  Require(samus_y_subspeed == 0 && samus_y_speed == 0 && samus_y_dir == 0,
          "crouching transition did not clear vertical speed for bounce reset");
  Require(used_for_ball_bounce_on_landing == 0,
          "crouching transition did not clear ball bounce landing state");

  MiniDestroy(state);
}

static void TestBlockCollisionTransitionContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for block collision transition test");
  RequireFallbackRoom(state);

  button_config_jump_a = kButton_A;
  button_config_shoot_x = kButton_X;
  joypad1_lastkeys = 0;

  samus_pose = kPose_13_FaceR_Jump_NoAim_NoMove_Gun;
  samus_pose_x_dir = kSamusPoseXDir_FaceRight;
  samus_prev_movement_type2 = kMovementType_03_SpinJumping;
  samus_new_pose = 0;
  samus_momentum_routine_index = 0;
  input_to_pose_calc = PoseCalcInput(kBlockCollisionInput_Landing,
                                     kBlockCollisionSubtype_JumpOrSpinLanding);
  Samus_HandleTransFromBlockColl();
  Require(samus_new_pose == kPose_A7_FaceL_LandSpinJump,
          "spin-jump block collision did not select the facing-right landing pose");
  Require(samus_momentum_routine_index == kSamusMomentumRoutine_BlockCollision,
          "spin-jump block collision did not select landing momentum handoff");

  samus_pose = kPose_31_FaceR_Morphball_Air;
  samus_pose_x_dir = kSamusPoseXDir_FaceLeft;
  samus_prev_movement_type2 = kMovementType_08_MorphBallFalling;
  samus_y_speed = 2;
  used_for_ball_bounce_on_landing = 0;
  samus_new_pose = 0;
  samus_momentum_routine_index = 0;
  input_to_pose_calc = PoseCalcInput(kBlockCollisionInput_Landing,
                                     kBlockCollisionSubtype_BallLanding);
  Samus_HandleTransFromBlockColl();
  Require(samus_new_pose == kPose_1D_FaceR_Morphball_Ground,
          "morph-ball block collision did not select the facing-left ground pose");
  Require(samus_momentum_routine_index == kSamusMomentumRoutine_BlockCollision,
          "morph-ball block collision did not select landing momentum handoff");

  samus_pose = kPose_31_FaceR_Morphball_Air;
  samus_pose_x_dir = kSamusPoseXDir_FaceRight;
  samus_new_pose = 0;
  input_to_pose_calc = PoseCalcInput(kBlockCollisionInput_Landing,
                                     kBlockCollisionSubtype_AltBallLanding);
  Samus_HandleTransFromBlockColl();
  Require(samus_new_pose == kPose_42_BlockCollisionMorphLanding,
          "alternate ball block collision did not select the facing-right legacy pose");

  samus_pose = kPose_7D_FaceR_Springball_Fall;
  samus_pose_x_dir = kSamusPoseXDir_FaceRight;
  samus_y_speed = 2;
  used_for_ball_bounce_on_landing = 0;
  samus_new_pose = 0;
  input_to_pose_calc = PoseCalcInput(kBlockCollisionInput_Landing,
                                     kBlockCollisionSubtype_SpringBallLanding);
  Samus_HandleTransFromBlockColl();
  Require(samus_new_pose == kPose_7A_FaceL_Springball_Ground,
          "spring-ball block collision did not select the facing-right ground pose");

  samus_pose = kPose_7D_FaceR_Springball_Fall;
  samus_pose_x_dir = kSamusPoseXDir_FaceLeft;
  used_for_ball_bounce_on_landing = kBallBounce_KeepCurrentPose;
  samus_new_pose = 0;
  input_to_pose_calc = PoseCalcInput(kBlockCollisionInput_Landing,
                                     kBlockCollisionSubtype_SpringBallLanding);
  Samus_HandleTransFromBlockColl();
  Require(samus_new_pose == kPose_7D_FaceR_Springball_Fall,
          "spring-ball bounce block collision did not keep the current pose");

  samus_pose = kPose_13_FaceR_Jump_NoAim_NoMove_Gun;
  samus_pose_x_dir = kSamusPoseXDir_FaceLeft;
  samus_new_pose = 0;
  samus_momentum_routine_index = 0;
  input_to_pose_calc = kBlockCollisionInput_WallJump;
  Samus_HandleTransFromBlockColl();
  Require(samus_new_pose == kPose_83_FaceR_Walljump,
          "wall-jump block collision did not select the facing-left wall-jump pose");
  Require(samus_momentum_routine_index == kSamusMomentumRoutine_BlockCollision,
          "wall-jump block collision did not select landing momentum handoff");

  samus_pose = kPose_13_FaceR_Jump_NoAim_NoMove_Gun;
  samus_new_pose = 0xBEEF;
  samus_momentum_routine_index = 0xBEEF;
  input_to_pose_calc = PoseCalcInput(kBlockCollisionInput_Landing,
                                     kBlockCollisionSubtype_Ignore);
  Samus_HandleTransFromBlockColl();
  Require(samus_new_pose == 0xBEEF,
          "ignored block collision transition changed the pending pose");
  Require(samus_momentum_routine_index == 0xBEEF,
          "ignored block collision transition changed the momentum handoff");

  MiniDestroy(state);
}

static EnemyData *ResetSolidEnemyCollisionContractState(uint16 direction) {
  samus_collision_direction = direction;
  samus_x_pos = 100;
  samus_x_subpos = 0;
  samus_y_pos = 100;
  samus_y_subpos = 0x1234;
  samus_x_radius = 5;
  samus_y_radius = 5;
  interactive_enemy_indexes_write_ptr = 2;
  interactive_enemy_indexes[0] = 0;
  interactive_enemy_indexes[1] = 0xFFFF;
  for (int i = 0; i < 4; i++)
    enemy_index_colliding_dirs[i] = 0xBEEF;
  collision_detection_index = 0xBEEF;

  EnemyData *enemy = gEnemyData(0);
  memset(enemy, 0, sizeof(*enemy));
  enemy->x_pos = 100;
  enemy->y_pos = 100;
  enemy->x_width = 10;
  enemy->y_height = 10;
  return enemy;
}

static void TestSolidEnemyCollisionContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for solid enemy collision test");
  RequireFallbackRoom(state);

  EnemyData *enemy = ResetSolidEnemyCollisionContractState(0);
  interactive_enemy_indexes_write_ptr = 0;
  CheckEnemyColl_Result result = Samus_CheckSolidEnemyColl(INT16_SHL16(3));
  Require(result.collision == 0 && result.amt == INT16_SHL16(3),
          "solid enemy collision changed the no-interactive-enemy result");

  enemy = ResetSolidEnemyCollisionContractState(0);
  enemy->x_pos = 85;
  result = Samus_CheckSolidEnemyColl(INT16_SHL16(1));
  Require(result.collision == 0 && result.amt == INT16_SHL16(1),
          "solid enemy collision did not skip non-solid active enemies");

  enemy = ResetSolidEnemyCollisionContractState(0);
  enemy->frozen_timer = 1;
  enemy->x_pos = 85;
  result = Samus_CheckSolidEnemyColl(INT16_SHL16(1));
  Require(result.collision == 0xFFFF && result.amt == 0 &&
          samus_y_subpos == 0 &&
          enemy_index_colliding_dirs[0] == 0,
          "solid enemy collision did not report a left touching hit");

  enemy = ResetSolidEnemyCollisionContractState(1);
  enemy->properties = kEnemyProps_SolidSamusCollisionContract;
  enemy->x_pos = 120;
  result = Samus_CheckSolidEnemyColl(INT16_SHL16(8));
  Require(result.collision == 0xFFFF && result.amt == INT16_SHL16(5) &&
          samus_y_subpos == 0x1234 &&
          enemy_index_colliding_dirs[1] == 0,
          "solid enemy collision did not preserve the right-side gap distance");

  enemy = ResetSolidEnemyCollisionContractState(2);
  enemy->frozen_timer = 1;
  enemy->y_pos = 80;
  result = Samus_CheckSolidEnemyColl(INT16_SHL16(8));
  Require(result.collision == 0xFFFF && result.amt == INT16_SHL16(5) &&
          samus_y_subpos == 0x1234 &&
          enemy_index_colliding_dirs[2] == 0,
          "solid enemy collision did not preserve the upward gap distance");

  enemy = ResetSolidEnemyCollisionContractState(3);
  enemy->frozen_timer = 1;
  enemy->y_pos = 115;
  result = Samus_CheckSolidEnemyColl(INT16_SHL16(1));
  Require(result.collision == 0xFFFF && result.amt == 0 &&
          samus_y_subpos == 0 &&
          enemy_index_colliding_dirs[3] == 0,
          "solid enemy collision did not report a downward touching hit");

  MiniDestroy(state);
}

static void TestPoseTransitionTableContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for pose transition table test");
  RequireFallbackRoom(state);

  samus_pose_x_dir = kSamusPoseXDir_FaceRight;
  joypad1_lastkeys = 0;
  knockback_x_dir = 0;
  knockback_dir = 0;
  samus_movement_handler = 0;
  Require(Samus_HandleTransitionsB_1_0() == 1,
          "damage-boost transition did not request knockback handling");
  Require(knockback_dir == kKnockbackDir_NoHeldInput,
          "damage-boost transition selected the wrong default knockback dir");
  Require(samus_movement_handler == FUNC16(Samus_MoveHandler_Knockback),
          "damage-boost transition did not install the knockback movement handler");

  joypad1_lastkeys = kButton_Left;
  knockback_x_dir = 0;
  Samus_HandleTransitionsB_1_0();
  Require(knockback_dir == kKnockbackDir_HeldOpposite,
          "damage-boost transition did not use the facing-right opposite input");

  joypad1_lastkeys = 0;
  knockback_x_dir = 1;
  Samus_HandleTransitionsB_1_0();
  Require(knockback_dir == kKnockbackDir_XMomentum,
          "damage-boost transition did not preserve x-momentum knockback");

  joypad1_lastkeys = kButton_Left;
  knockback_x_dir = 1;
  Samus_HandleTransitionsB_1_0();
  Require(knockback_dir == kKnockbackDir_XMomentumHeldOpposite,
          "damage-boost transition did not combine x-momentum and opposite input");

  samus_pose_x_dir = kSamusPoseXDir_FaceLeft;
  joypad1_lastkeys = kButton_Right;
  knockback_x_dir = 0;
  Samus_HandleTransitionsB_1_0();
  Require(knockback_dir == kKnockbackDir_HeldOpposite,
          "damage-boost transition did not use the facing-left opposite input");

  samus_pose_x_dir = kSamusPoseXDir_FaceRight;
  Require(Samus_HandleTransitionsB_1_4() == 0,
          "knockback transition subtype 4 reported the wrong return value");
  Require(knockback_dir == kKnockbackDir_NoHeldInput &&
          samus_movement_handler == FUNC16(Samus_MoveHandler_Knockback),
          "knockback transition subtype 4 did not set right-facing knockback");

  samus_pose_x_dir = kSamusPoseXDir_FaceLeft;
  Require(Samus_HandleTransitionsB_1_7() == 1,
          "knockback transition subtype 7 reported the wrong return value");
  Require(knockback_dir == kKnockbackDir_XMomentum &&
          samus_movement_handler == FUNC16(Samus_MoveHandler_Knockback),
          "knockback transition subtype 7 did not set left-facing knockback");

  samus_pose_x_dir = kSamusPoseXDir_FaceRight;
  samus_movement_type = kMovementType_01_Running;
  xray_angle = 0;
  samus_anim_frame = 0;
  samus_anim_frame_timer = 0;
  samus_movement_handler = 0;
  samus_input_handler = 0;
  timer_for_shine_timer = 0;
  special_samus_palette_timer = 0xBEEF;
  special_samus_palette_frame = 0xBEEF;
  samus_shine_timer = 0xBEEF;
  flare_counter = 0xBEEF;
  flare_animation_frame = 0xBEEF;
  flare_slow_sparks_anim_frame = 0xBEEF;
  flare_fast_sparks_anim_frame = 0xBEEF;
  flare_animation_timer = 0xBEEF;
  flare_slow_sparks_anim_timer = 0xBEEF;
  flare_fast_sparks_anim_timer = 0xBEEF;
  Samus_HandleTransitionsB_5();
  Require(xray_angle == kXrayAngle_FacingRight,
          "x-ray transition did not use the right-facing angle");
  Require(samus_anim_frame == kXrayTransitionAnimFrame &&
          samus_anim_frame_timer == kXrayTransitionAnimTimer,
          "x-ray transition did not initialize animation state");
  Require(samus_movement_handler == FUNC16(SamusMovementType_Xray) &&
          samus_input_handler == FUNC16(Samus_Func20_),
          "x-ray transition did not install x-ray handlers");
  Require(timer_for_shine_timer == kXrayTransitionShineTimerDelay &&
          special_samus_palette_timer == 1 &&
          special_samus_palette_frame == 0,
          "x-ray transition did not initialize shine/palette timers");
  Require(samus_shine_timer == 0 && flare_counter == 0 &&
          flare_animation_frame == 0 && flare_slow_sparks_anim_frame == 0 &&
          flare_fast_sparks_anim_frame == 0 && flare_animation_timer == 0 &&
          flare_slow_sparks_anim_timer == 0 && flare_fast_sparks_anim_timer == 0,
          "x-ray transition did not clear flare state");

  samus_pose_x_dir = kSamusPoseXDir_FaceLeft;
  samus_movement_type = kMovementType_05_Crouching;
  xray_angle = 0;
  Samus_HandleTransitionsB_5();
  Require(xray_angle == kXrayAngle_FacingLeft,
          "x-ray transition did not use the left-facing angle");

  samus_movement_type = kMovementType_03_SpinJumping;
  xray_angle = 0xBEEF;
  samus_movement_handler = 0xBEEF;
  Samus_HandleTransitionsB_5();
  Require(xray_angle == 0xBEEF && samus_movement_handler == 0xBEEF,
          "x-ray transition changed state for an unsupported movement type");

  samus_x_pos = 100;
  samus_prev_x_pos = 80;
  samus_y_pos = 200;
  samus_prev_y_pos = 220;
  samus_x_base_speed = 0x1111;
  samus_x_base_subspeed = 0x2222;
  samus_x_extra_run_speed = 0x3333;
  samus_x_extra_run_subspeed = 0x4444;
  samus_y_speed = 0x5555;
  samus_y_subspeed = 0x6666;
  Samus_HandleTransitionsB_9B();
  Require(samus_prev_x_pos == samus_x_pos - kGrappleTransitionMaxPrevDelta &&
          samus_prev_y_pos == samus_y_pos + kGrappleTransitionMaxPrevDelta,
          "grapple transition did not clamp previous position across both axes");
  Require(samus_x_base_speed == 0 && samus_x_base_subspeed == 0 &&
          samus_x_extra_run_speed == 0 && samus_x_extra_run_subspeed == 0 &&
          samus_y_speed == 0 && samus_y_subspeed == 0,
          "grapple transition did not clear motion speeds");

  samus_x_pos = 100;
  samus_prev_x_pos = 120;
  samus_y_pos = 200;
  samus_prev_y_pos = 180;
  Samus_HandleTransitionsB_9B();
  Require(samus_prev_x_pos == samus_x_pos + kGrappleTransitionMaxPrevDelta &&
          samus_prev_y_pos == samus_y_pos - kGrappleTransitionMaxPrevDelta,
          "grapple transition did not clamp previous position in the opposite direction");

  samus_x_pos = 100;
  samus_prev_x_pos = 89;
  samus_y_pos = 200;
  samus_prev_y_pos = 212;
  Samus_HandleTransitionsB_9B();
  Require(samus_prev_x_pos == 89 && samus_prev_y_pos == 212,
          "grapple transition changed previous position inside the clamp window");

  MiniDestroy(state);
}

static void TestBallBounceTransitionContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for ball bounce transition test");
  RequireFallbackRoom(state);

  button_config_jump_a = kButton_A;
  joypad1_lastkeys = 0;

  used_for_ball_bounce_on_landing = 0;
  samus_y_speed = 3;
  samus_y_subspeed = 0xBEEF;
  samus_y_dir = 2;
  Require(Samus_MorphBallBounceNoSpringballTrans() == 1,
          "morph-ball landing did not start the first bounce");
  Require(used_for_ball_bounce_on_landing == 1 && samus_y_dir == 1 &&
          samus_y_subspeed == 0 && samus_y_speed == 1,
          "morph-ball first bounce state was not initialized correctly");

  Require(Samus_MorphBallBounceNoSpringballTrans() == 1,
          "morph-ball landing did not continue the second bounce");
  Require(used_for_ball_bounce_on_landing == 2 && samus_y_dir == 1 &&
          samus_y_subspeed == 0 && samus_y_speed == 0,
          "morph-ball second bounce state was not initialized correctly");

  Require(Samus_MorphBallBounceNoSpringballTrans() == 0,
          "morph-ball landing did not finish after the second bounce");
  Require(used_for_ball_bounce_on_landing == 0 && samus_y_dir == 0 &&
          samus_y_subspeed == 0 && samus_y_speed == 0,
          "morph-ball completed bounce did not clear vertical state");

  used_for_ball_bounce_on_landing = 0;
  samus_y_speed = 2;
  samus_y_subspeed = 0xBEEF;
  samus_y_dir = 2;
  Require(Samus_MorphBallBounceNoSpringballTrans() == 0,
          "slow morph-ball landing unexpectedly bounced");
  Require(used_for_ball_bounce_on_landing == 0 && samus_y_dir == 0 &&
          samus_y_subspeed == 0 && samus_y_speed == 0,
          "slow morph-ball landing did not clear vertical state");

  used_for_ball_bounce_on_landing = 0;
  samus_y_speed = 3;
  samus_y_subspeed = 0xBEEF;
  samus_y_dir = 2;
  Require(Samus_MorphBallBounceSpringballTrans() == 1,
          "spring-ball landing did not start the first bounce");
  Require(used_for_ball_bounce_on_landing == kSpringBallBounce_First &&
          samus_y_dir == 1 && samus_y_subspeed == 0 && samus_y_speed == 1,
          "spring-ball first bounce state was not initialized correctly");

  Require(Samus_MorphBallBounceSpringballTrans() == 1,
          "spring-ball landing did not continue the second bounce");
  Require(used_for_ball_bounce_on_landing == kSpringBallBounce_Second &&
          samus_y_dir == 1 && samus_y_subspeed == 0 && samus_y_speed == 0,
          "spring-ball second bounce state was not initialized correctly");

  Require(Samus_MorphBallBounceSpringballTrans() == 0,
          "spring-ball landing did not finish after the second bounce");
  Require(used_for_ball_bounce_on_landing == 0 && samus_y_dir == 0 &&
          samus_y_subspeed == 0 && samus_y_speed == 0,
          "spring-ball completed bounce did not clear vertical state");

  used_for_ball_bounce_on_landing = 0xFFFF;
  enable_horiz_slope_coll = 0;
  Require(Samus_HandleTransitionsA_5_1_2() == 0,
          "slope ball transition did not report completion");
  Require(used_for_ball_bounce_on_landing == 0 && enable_horiz_slope_coll == 3,
          "slope ball transition did not clear bounce state and enable slope collision");

  MiniDestroy(state);
}

static void TestTransitionA7PoseAdjustmentContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for transition A7 test");
  RequireFallbackRoom(state);

  samus_pose = kPose_35_FaceR_CrouchTrans;
  samus_x_pos = 64;
  samus_y_pos = 64;
  samus_prev_y_pos = 0xBEEF;
  samus_y_radius = 0xBEEF;
  used_for_ball_bounce_on_landing = 0;
  Samus_HandleTransitionsA_7();
  Require(samus_y_pos == 64 + kTransitionA7SmallYOffset &&
          samus_prev_y_pos == samus_y_pos,
          "transition A7 did not apply the small crouch-transition Y offset");
  Require(samus_y_radius == kPoseParams[kPose_35_FaceR_CrouchTrans].y_radius,
          "transition A7 did not refresh the pose Y radius before collision");

  samus_pose = kPose_39_MaybeUnusedTransition;
  samus_y_pos = 88;
  samus_prev_y_pos = 0xBEEF;
  samus_y_subspeed = 0x1111;
  samus_y_speed = 0x2222;
  samus_y_dir = 2;
  used_for_ball_bounce_on_landing = 1;
  Samus_HandleTransitionsA_7();
  Require(samus_y_pos == 88 && samus_prev_y_pos == samus_y_pos,
          "transition A7 zero-offset pose did not preserve Y position and refresh previous Y");
  Require(used_for_ball_bounce_on_landing == 0 && samus_y_subspeed == 0 &&
          samus_y_speed == 0 && samus_y_dir == 0,
          "transition A7 zero-offset pose did not clear ball bounce landing state");

  samus_pose = kPose_F1_FaceR_CrouchTrans_AimU;
  samus_y_pos = 64;
  samus_prev_y_pos = 0xBEEF;
  samus_y_radius = 0xBEEF;
  used_for_ball_bounce_on_landing = 0;
  Samus_HandleTransitionsA_7();
  Require(samus_y_pos == 64 + kTransitionA7SmallYOffset &&
          samus_prev_y_pos == samus_y_pos,
          "transition A7 did not apply the aim-up transition Y offset");
  Require(samus_y_radius == kPoseParams[kPose_F1_FaceR_CrouchTrans_AimU].y_radius,
          "transition A7 did not refresh the aim-up transition Y radius");

  samus_pose = kPose_F7_FaceR_StandTrans_AimU;
  samus_y_pos = 64;
  samus_prev_y_pos = 0xBEEF;
  samus_y_subspeed = 0x1111;
  samus_y_speed = 0x2222;
  samus_y_dir = 2;
  used_for_ball_bounce_on_landing = 1;
  Samus_HandleTransitionsA_7();
  Require(samus_y_pos == 64 && samus_prev_y_pos == 0xBEEF &&
          used_for_ball_bounce_on_landing == 1 && samus_y_subspeed == 0x1111 &&
          samus_y_speed == 0x2222 && samus_y_dir == 2,
          "transition A7 changed state for an unsupported stand-transition pose");

  MiniDestroy(state);
}

static void ResetLandingGraphicsTestState(void) {
  cinematic_function = 0;
  room_index = kCrateriaLandingGfxRoom_FxTypeGate;
  fx_type = 0;
  fx_y_pos = kLiquidYPos_Disabled;
  lava_acid_y_pos = kLiquidYPos_Disabled;
  samus_x_pos = 64;
  samus_y_pos = 64;
  samus_y_radius = 8;
  atmospheric_gfx_frame_and_type[2] = 0xBEEF;
  atmospheric_gfx_frame_and_type[3] = 0xBEEF;
  atmospheric_gfx_anim_timer[2] = 0;
  atmospheric_gfx_anim_timer[3] = 0;
  atmospheric_gfx_x_pos[2] = 0;
  atmospheric_gfx_x_pos[3] = 0;
  atmospheric_gfx_y_pos[2] = 0;
  atmospheric_gfx_y_pos[3] = 0;
}

static bool LandingGraphicsFramesCleared(void) {
  return atmospheric_gfx_frame_and_type[2] == 0 &&
         atmospheric_gfx_frame_and_type[3] == 0;
}

static bool LandingGraphicsFramesMatch(uint16 frame_type) {
  return atmospheric_gfx_frame_and_type[2] == frame_type &&
         atmospheric_gfx_frame_and_type[3] == frame_type &&
         atmospheric_gfx_anim_timer[2] == 3 &&
         atmospheric_gfx_anim_timer[3] == 3;
}

static void ResetFootstepGraphicsTestState(void) {
  cinematic_function = 0;
  room_index = kCrateriaLandingGfxRoom_FxTypeGate;
  fx_type = 0;
  fx_y_pos = kLiquidYPos_Disabled;
  lava_acid_y_pos = kLiquidYPos_Disabled;
  fx_liquid_options = 0;
  speed_boost_counter = 0;
  samus_pose_x_dir = kSamusPoseXDir_FaceRight;
  samus_x_pos = 80;
  samus_y_pos = 80;
  samus_y_radius = 8;
  atmospheric_gfx_frame_and_type[0] = 0xBEEF;
  atmospheric_gfx_frame_and_type[1] = 0xBEEF;
  atmospheric_gfx_anim_timer[0] = 0;
  atmospheric_gfx_anim_timer[1] = 0;
  atmospheric_gfx_x_pos[0] = 0;
  atmospheric_gfx_x_pos[1] = 0;
  atmospheric_gfx_y_pos[0] = 0;
  atmospheric_gfx_y_pos[1] = 0;
}

static bool FootstepGraphicsUnchanged(void) {
  return atmospheric_gfx_frame_and_type[0] == 0xBEEF &&
         atmospheric_gfx_frame_and_type[1] == 0xBEEF;
}

static bool FootstepGraphicsFramesMatch(uint16 frame_type) {
  return atmospheric_gfx_frame_and_type[0] == frame_type &&
         atmospheric_gfx_frame_and_type[1] == frame_type &&
         atmospheric_gfx_anim_timer[0] == 0x8002 &&
         atmospheric_gfx_anim_timer[1] == 3;
}

static void TestCrateriaLandingGraphicsContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for Crateria landing graphics test");
  RequireFallbackRoom(state);

  ResetLandingGraphicsTestState();
  cinematic_function = 1;
  room_index = kCrateriaLandingGfxRoom_Always;
  fx_type = kCrateriaLandingGfxRequiredFxType;
  HandleLandingGraphics_Crateria();
  Require(LandingGraphicsFramesCleared(),
          "Crateria landing graphics did not clear during cinematics");

  ResetLandingGraphicsTestState();
  room_index = kCrateriaLandingGfxRoom_OutOfTable;
  fx_type = kCrateriaLandingGfxRequiredFxType;
  HandleLandingGraphics_Crateria();
  Require(LandingGraphicsFramesCleared(),
          "Crateria landing graphics did not clear out-of-table rooms");

  ResetLandingGraphicsTestState();
  room_index = kCrateriaLandingGfxRoom_FxTypeGate;
  fx_type = 0;
  HandleLandingGraphics_Crateria();
  Require(LandingGraphicsFramesCleared(),
          "Crateria landing graphics ignored the FX-type gate");

  ResetLandingGraphicsTestState();
  room_index = kCrateriaLandingGfxRoom_FxTypeGate;
  fx_type = kCrateriaLandingGfxRequiredFxType;
  HandleLandingGraphics_Crateria();
  Require(LandingGraphicsFramesMatch(kLandingGfxMaridiaFrameType),
          "Crateria landing graphics did not delegate the FX-type gated room to Maridia splashes");

  ResetLandingGraphicsTestState();
  room_index = kCrateriaLandingGfxRoom_YThresholdGate;
  samus_y_pos = kCrateriaLandingGfxYThreshold - 1;
  HandleLandingGraphics_Crateria();
  Require(LandingGraphicsFramesCleared(),
          "Crateria landing graphics ignored the Y-threshold lower side");

  ResetLandingGraphicsTestState();
  room_index = kCrateriaLandingGfxRoom_YThresholdGate;
  samus_y_pos = kCrateriaLandingGfxYThreshold;
  HandleLandingGraphics_Crateria();
  Require(LandingGraphicsFramesMatch(kLandingGfxMaridiaFrameType),
          "Crateria landing graphics ignored the Y-threshold upper side");

  ResetLandingGraphicsTestState();
  room_index = kCrateriaLandingGfxRoom_Always;
  HandleLandingGraphics_Crateria();
  Require(LandingGraphicsFramesMatch(kLandingGfxMaridiaFrameType),
          "Crateria landing graphics did not use the always-splash rule");

  ResetLandingGraphicsTestState();
  room_index = kCrateriaLandingGfxRoom_Norfair;
  HandleLandingGraphics_Crateria();
  Require(LandingGraphicsFramesMatch(kLandingGfxNorfairFrameType),
          "Crateria landing graphics did not delegate room 28 to Norfair splashes");

  MiniDestroy(state);
}

static void TestCrateriaFootstepGraphicsContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for Crateria footstep graphics test");
  RequireFallbackRoom(state);

  ResetFootstepGraphicsTestState();
  cinematic_function = 1;
  room_index = kCrateriaLandingGfxRoom_Always;
  fx_type = kCrateriaLandingGfxRequiredFxType;
  Samus_FootstepGraphics_Crateria();
  Require(FootstepGraphicsUnchanged(),
          "Crateria footstep graphics changed during cinematics");

  ResetFootstepGraphicsTestState();
  room_index = kCrateriaLandingGfxRoom_OutOfTable;
  fx_type = kCrateriaLandingGfxRequiredFxType;
  Samus_FootstepGraphics_Crateria();
  Require(FootstepGraphicsUnchanged(),
          "Crateria footstep graphics changed for out-of-table rooms");

  ResetFootstepGraphicsTestState();
  room_index = kCrateriaLandingGfxRoom_FxTypeGate;
  fx_type = 0;
  Samus_FootstepGraphics_Crateria();
  Require(FootstepGraphicsUnchanged(),
          "Crateria footstep graphics ignored the FX-type gate");

  ResetFootstepGraphicsTestState();
  room_index = kCrateriaLandingGfxRoom_FxTypeGate;
  fx_type = kCrateriaLandingGfxRequiredFxType;
  Samus_FootstepGraphics_Crateria();
  Require(FootstepGraphicsFramesMatch(kLandingGfxMaridiaFrameType),
          "Crateria footstep graphics did not delegate the FX-type gated room to Maridia splashes");

  ResetFootstepGraphicsTestState();
  room_index = kCrateriaLandingGfxRoom_YThresholdGate;
  samus_y_pos = kCrateriaLandingGfxYThreshold - 1;
  Samus_FootstepGraphics_Crateria();
  Require(FootstepGraphicsUnchanged(),
          "Crateria footstep graphics ignored the Y-threshold lower side");

  ResetFootstepGraphicsTestState();
  room_index = kCrateriaLandingGfxRoom_YThresholdGate;
  samus_y_pos = kCrateriaLandingGfxYThreshold;
  Samus_FootstepGraphics_Crateria();
  Require(FootstepGraphicsFramesMatch(kLandingGfxMaridiaFrameType),
          "Crateria footstep graphics ignored the Y-threshold upper side");

  ResetFootstepGraphicsTestState();
  room_index = kCrateriaLandingGfxRoom_Always;
  Samus_FootstepGraphics_Crateria();
  Require(FootstepGraphicsFramesMatch(kLandingGfxMaridiaFrameType),
          "Crateria footstep graphics did not use the always-splash rule");

  MiniDestroy(state);
}

static void ResetScrewSpeedPaletteContractState(void) {
  fx_y_pos = kLiquidYPos_Disabled;
  lava_acid_y_pos = kLiquidYPos_Disabled;
  fx_liquid_options = 0;
  samus_y_pos = 100;
  samus_y_radius = 8;
  samus_suit_palette_index = kSamusSuitPalette_Gravity;
  equipped_items = kSamusEquip_ScrewAttack;
  special_samus_palette_frame = 0xBEEF;
  special_samus_palette_timer = 0xBEEF;
  speed_boost_counter = 0;
}

static void TestScrewSpeedPaletteContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for screw/speed palette test");
  RequireFallbackRoom(state);

  ResetScrewSpeedPaletteContractState();
  samus_movement_type = kMovementType_03_SpinJumping;
  samus_anim_frame = 0;
  Require(Samus_HandleScrewAttackSpeedBoostingPals() == 1 &&
          special_samus_palette_frame == 0,
          "spin-jump screw palette did not reset the frame before the active window");

  ResetScrewSpeedPaletteContractState();
  samus_movement_type = kMovementType_03_SpinJumping;
  samus_anim_frame = 4;
  special_samus_palette_frame = 0;
  Require(Samus_HandleScrewAttackSpeedBoostingPals() == 1 &&
          special_samus_palette_frame == 2,
          "spin-jump screw palette did not advance during the active window");

  ResetScrewSpeedPaletteContractState();
  samus_movement_type = kMovementType_03_SpinJumping;
  samus_anim_frame = 27;
  special_samus_palette_frame = 0xBEEF;
  Require(Samus_HandleScrewAttackSpeedBoostingPals() == 0 &&
          special_samus_palette_frame == 0xBEEF,
          "spin-jump screw palette did not stop at the end of the active window");

  ResetScrewSpeedPaletteContractState();
  samus_movement_type = kMovementType_14_WallJumping;
  samus_anim_frame = 2;
  Require(Samus_HandleScrewAttackSpeedBoostingPals() == 1 &&
          special_samus_palette_frame == 0,
          "wall-jump screw palette did not reset before frame three");

  ResetScrewSpeedPaletteContractState();
  samus_movement_type = kMovementType_14_WallJumping;
  samus_anim_frame = 3;
  special_samus_palette_frame = 0;
  Require(Samus_HandleScrewAttackSpeedBoostingPals() == 1 &&
          special_samus_palette_frame == 2,
          "wall-jump screw palette did not advance at frame three");

  ResetScrewSpeedPaletteContractState();
  equipped_items = 0;
  samus_movement_type = kMovementType_03_SpinJumping;
  speed_boost_counter = 0x0400;
  special_samus_palette_timer = 1;
  special_samus_palette_frame = 0;
  Require(Samus_HandleScrewAttackSpeedBoostingPals() == 1 &&
          special_samus_palette_timer == 4 &&
          special_samus_palette_frame == 2,
          "speed-boost palette path did not advance when screw attack was unavailable");

  MiniDestroy(state);
}

static void ResetMiscPaletteContractState(void) {
  ResetSfxQueuesForContracts();
  samus_special_super_palette_flags = 0;
  samus_hurt_flash_counter = 2;
  cinematic_function = 0;
  frame_handler_beta = 0;
  samus_pose = kPose_00_FaceF_Powersuit;
  grapple_beam_function = FUNC16(GrappleBeamFunc_Inactive);
  samus_movement_type = kMovementType_00_Standing;
  flare_counter = 0;
  button_config_shoot_x = kButton_X;
  joypad1_lastkeys = 0;
  play_resume_charging_beam_sfx = 0;
}

static void TestMiscPaletteHurtFlashContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for misc palette hurt flash test");
  RequireFallbackRoom(state);

  ResetMiscPaletteContractState();
  HandleMiscSamusPalette();
  Require(samus_hurt_flash_counter == 3 &&
          sfx1_queue[0] == 0x35,
          "hurt flash palette did not queue the normal damage sound at counter two");

  ResetMiscPaletteContractState();
  frame_handler_beta = FUNC16(j_HandleDemoRecorder_2_0);
  samus_pose = kPose_54_FaceL_Knockback;
  HandleMiscSamusPalette();
  Require(samus_hurt_flash_counter == 3 &&
          sfx1_queue[0] == 0,
          "hurt flash palette did not suppress the demo knockback damage sound");

  ResetMiscPaletteContractState();
  samus_hurt_flash_counter = 39;
  samus_movement_type = kMovementType_03_SpinJumping;
  HandleMiscSamusPalette();
  Require(samus_hurt_flash_counter == 40 &&
          sfx1_queue[0] != 0,
          "hurt flash palette did not run the spin-jump sound refresh at counter forty");

  MiniDestroy(state);
}

static void ResetTransitionDispatcherState(SamusPose current_pose,
                                           SamusPose previous_pose) {
  samus_pose = current_pose;
  samus_pose_x_dir = kPoseParams[current_pose].pose_x_dir;
  samus_movement_type = kPoseParams[current_pose].movement_type;
  samus_prev_pose = previous_pose;
  samus_prev_pose_x_dir = kPoseParams[previous_pose].pose_x_dir;
  samus_prev_movement_type2 = kPoseParams[previous_pose].movement_type;
  samus_last_different_pose = 0xBEEF;
  samus_last_different_pose_x_dir = 0xBE;
  samus_last_different_pose_movement_type = 0xEF;
  samus_new_pose = kSamusNoPendingPose;
  samus_new_pose_interrupted = kSamusNoPendingPose;
  samus_new_pose_transitional = kSamusNoPendingPose;
  samus_momentum_routine_index = kSamusMomentumRoutine_None;
  samus_special_transgfx_index = kSamusSpecialTransGfx_None;
  samus_hurt_switch_index = kSamusHurtSwitch_Default;
  samus_collides_with_solid_enemy = 0;
  input_to_pose_calc = 0xBEEF;
  samus_anim_frame_skip = 0x8000;
  equipped_items = kSamusEquip_GravitySuit;
}

static void RequireTransitionPoseHistory(SamusPose expected_last_pose,
                                         SamusPose expected_prev_pose) {
  Require(samus_last_different_pose == expected_last_pose,
          "transition dispatcher did not preserve the previous pose as the last different pose");
  Require(samus_prev_pose == expected_prev_pose,
          "transition dispatcher did not record the committed pose as previous pose");
  Require(samus_prev_pose_x_dir == samus_pose_x_dir &&
          samus_prev_movement_type2 == samus_movement_type,
          "transition dispatcher did not mirror current pose direction/type into previous pose fields");
  Require(input_to_pose_calc == 0,
          "transition dispatcher did not clear pose-calc input");
}

static void TestTransitionDispatcherContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for transition dispatcher test");
  RequireFallbackRoom(state);

  ResetTransitionDispatcherState(kPose_01_FaceR_Normal, kPose_01_FaceR_Normal);
  samus_new_pose_transitional = kPose_02_FaceL_Normal;
  Samus_HandleTransitions();
  Require(samus_pose == kPose_02_FaceL_Normal,
          "transition dispatcher did not commit a regular transitional pose");
  RequireTransitionPoseHistory(kPose_01_FaceR_Normal, kPose_02_FaceL_Normal);

  ResetTransitionDispatcherState(kPose_01_FaceR_Normal, kPose_27_FaceR_Crouch);
  samus_new_pose_transitional = kPose_02_FaceL_Normal;
  samus_hurt_switch_index = kSamusHurtSwitch_Grapple;
  samus_special_transgfx_index = kSamusSpecialTransGfx_Grapple;
  samus_new_pose = kPose_27_FaceR_Crouch;
  Samus_HandleTransitions();
  Require(samus_pose == kPose_27_FaceR_Crouch,
          "transition dispatcher did not let grapple transitional state fall through to the normal pose");
  Require(samus_pose != kPose_02_FaceL_Normal,
          "transition dispatcher committed a grapple transitional pose that should fall through");
  RequireTransitionPoseHistory(kPose_27_FaceR_Crouch, kPose_27_FaceR_Crouch);

  ResetTransitionDispatcherState(kPose_01_FaceR_Normal, kPose_28_FaceL_Crouch);
  samus_new_pose_interrupted = kPose_28_FaceL_Crouch;
  samus_new_pose = kPose_27_FaceR_Crouch;
  Samus_HandleTransitions();
  Require(samus_pose == kPose_28_FaceL_Crouch,
          "transition dispatcher did not prioritize interrupted pose over normal pose");
  RequireTransitionPoseHistory(kPose_28_FaceL_Crouch, kPose_28_FaceL_Crouch);

  ResetTransitionDispatcherState(kPose_01_FaceR_Normal, kPose_27_FaceR_Crouch);
  samus_new_pose = kPose_27_FaceR_Crouch;
  Samus_HandleTransitions();
  Require(samus_pose == kPose_27_FaceR_Crouch,
          "transition dispatcher did not commit the normal pending pose");
  RequireTransitionPoseHistory(kPose_27_FaceR_Crouch, kPose_27_FaceR_Crouch);

  ResetTransitionDispatcherState(kPose_01_FaceR_Normal, kPose_02_FaceL_Normal);
  Samus_HandleTransitions();
  Require(samus_pose == kPose_01_FaceR_Normal &&
          samus_prev_pose == kPose_02_FaceL_Normal &&
          samus_last_different_pose == 0xBEEF &&
          input_to_pose_calc == 0,
          "transition dispatcher changed pose history when no pending pose existed");

  MiniDestroy(state);
}

static uint16 WalkedIntoSomethingPoseFor(SamusPose pose) {
  return kWalkedIntoSomethingPoseTable[kPoseParams[pose].direction_shots_fired];
}

static void TestWalkedIntoSomethingContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for walked-into-something test");
  RequireFallbackRoom(state);

  samus_pose = kPose_09_MoveR_NoAim;
  samus_movement_type = kMovementType_01_Running;
  samus_collides_with_solid_enemy = 1;
  samus_new_pose = 0;
  Samus_CheckWalkedIntoSomething();
  Require(samus_new_pose == WalkedIntoSomethingPoseFor(kPose_09_MoveR_NoAim),
          "walked-into-something did not convert current running pose to wall-bump pose");
  Require(samus_collides_with_solid_enemy == 0,
          "walked-into-something did not clear the solid-enemy collision flag");

  samus_pose = kPose_09_MoveR_NoAim;
  samus_movement_type = kMovementType_05_Crouching;
  samus_collides_with_solid_enemy = 1;
  samus_new_pose = kSamusNoPendingPose;
  Samus_CheckWalkedIntoSomething();
  Require(samus_new_pose == kSamusNoPendingPose &&
          samus_collides_with_solid_enemy == 0,
          "walked-into-something did not ignore non-running movement with no pending pose");

  MiniDestroy(state);
}

static void ResetPoseTurnTransitionState(SamusPose previous_pose,
                                         uint16 previous_movement_type,
                                         uint16 held_keys) {
  samus_prev_pose = previous_pose;
  samus_prev_movement_type2 = previous_movement_type;
  button_config_jump_a = kButton_A;
  joypad1_lastkeys = held_keys;
  samus_pose = 0;
  new_projectile_direction_changed_pose = 0;
  samus_x_base_speed = 1;
  samus_x_base_subspeed = 0;
  samus_x_extra_run_speed = 2;
  samus_x_extra_run_subspeed = 0;
  samus_x_accel_mode = kSamusXAccelMode_None;
}

static void RequirePoseTurnCarriedSpeed(const char *message) {
  Require(samus_x_base_speed == 3 &&
          samus_x_base_subspeed == 0 &&
          samus_x_extra_run_speed == 0 &&
          samus_x_extra_run_subspeed == 0 &&
          samus_x_accel_mode == kSamusXAccelMode_Decelerating,
          message);
}

static void TestPoseTurnTransitionTableContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for pose turn-transition table test");
  if (!state->uses_rom_room ||
      *((uint8 *)RomFixedPtr(0x91f9c2)) != kPose_8B_FaceR_Turn_Stand_AimU) {
    MiniDestroy(state);
    return;
  }

  ResetPoseTurnTransitionState(kPose_15_FaceR_Jump_AimU,
                               kMovementType_00_Standing, 0);
  Require(SamusFunc_F468_TurningAroundOnGround() == 1 &&
          samus_pose == kPose_8B_FaceR_Turn_Stand_AimU,
          "standing turn transition did not use the standing turn table");
  RequirePoseTurnCarriedSpeed("standing turn transition did not carry horizontal speed");

  ResetPoseTurnTransitionState(kPose_15_FaceR_Jump_AimU,
                               kMovementType_05_Crouching, 0);
  Require(SamusFunc_F468_TurningAroundOnGround() == 1 &&
          samus_pose == kPose_97_FaceR_Turn_Crouch_AimU,
          "crouching turn transition did not use the crouching turn table");
  RequirePoseTurnCarriedSpeed("crouching turn transition did not carry horizontal speed");

  ResetPoseTurnTransitionState(kPose_75_FaceL_Moonwalk_AimUL,
                               kMovementType_10_Moonwalking, kButton_A);
  Require(SamusFunc_F468_TurningAroundOnGround() == 1 &&
          samus_pose == kPose_C2_FaceL_Moonwalk_TurnjumpR_AimUL &&
          new_projectile_direction_changed_pose ==
              (kPoseParams[kPose_75_FaceL_Moonwalk_AimUL].direction_shots_fired |
               kMoonwalkTurnProjectileDirectionFlag),
          "moonwalk turn transition did not use the jump-held moonwalk table");
  RequirePoseTurnCarriedSpeed("moonwalk turn transition did not carry horizontal speed");

  ResetPoseTurnTransitionState(kPose_13_FaceR_Jump_NoAim_NoMove_Gun,
                               kMovementType_02_NormalJumping, 0);
  Require(SamusFunc_F468_TurnAroundJumping() == 1 &&
          samus_pose == kPose_2F_FaceR_Turn_Jump,
          "jump turn transition did not use the jumping turn table");
  RequirePoseTurnCarriedSpeed("jump turn transition did not carry horizontal speed");

  ResetPoseTurnTransitionState(kPose_2B_FaceR_Fall_AimU,
                               kMovementType_06_Falling, 0);
  Require(SamusFunc_F468_TurnAroundFalling() == 1 &&
          samus_pose == kPose_93_FaceR_Turn_Fall_AimU,
          "fall turn transition did not use the falling turn table");
  RequirePoseTurnCarriedSpeed("fall turn transition did not carry horizontal speed");

  MiniDestroy(state);
}

static void TestPoseFacingFlipSpeedCarryContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for pose facing flip test");
  RequireFallbackRoom(state);

  samus_prev_pose_x_dir = kSamusPoseXDir_FaceLeft;
  samus_pose_x_dir = kSamusPoseXDir_FaceRight;
  samus_x_base_speed = 1;
  samus_x_base_subspeed = 0;
  samus_x_extra_run_speed = 2;
  samus_x_extra_run_subspeed = 0;
  samus_x_accel_mode = kSamusXAccelMode_None;
  SamusFunc_FA0A();
  Require(samus_x_base_speed == 3 && samus_x_base_subspeed == 0 &&
          samus_x_extra_run_speed == 0 && samus_x_extra_run_subspeed == 0 &&
          samus_x_accel_mode == kSamusXAccelMode_Decelerating,
          "pose facing flip did not carry extra run speed into base speed");

  samus_prev_pose_x_dir = kSamusPoseXDir_FaceLeft;
  samus_pose_x_dir = kSamusPoseXDir_FaceLeft;
  samus_x_base_speed = 1;
  samus_x_base_subspeed = 0;
  samus_x_extra_run_speed = 2;
  samus_x_extra_run_subspeed = 0;
  samus_x_accel_mode = kSamusXAccelMode_None;
  SamusFunc_FA0A();
  Require(samus_x_base_speed == 1 && samus_x_base_subspeed == 0 &&
          samus_x_extra_run_speed == 2 && samus_x_extra_run_subspeed == 0 &&
          samus_x_accel_mode == kSamusXAccelMode_None,
          "pose facing no-flip path changed horizontal speed state");

  samus_prev_pose_x_dir = kSamusPoseXDir_FaceRight;
  samus_pose_x_dir = kSamusPoseXDir_FaceLeft;
  SamusFunc_FA0A();
  Require(samus_x_extra_run_speed == 0 &&
          samus_x_accel_mode == kSamusXAccelMode_Decelerating,
          "pose facing right-to-left flip did not carry speed state");

  MiniDestroy(state);
}

static void DisablePoseTransitionLiquid(void) {
  fx_y_pos = kLiquidYPos_Disabled;
  lava_acid_y_pos = kLiquidYPos_Disabled;
  fx_liquid_options = 0;
}

static void TestPoseNormalJumpTransitionContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for normal-jump pose-transition test");
  RequireFallbackRoom(state);

  samus_pose = kPose_69_FaceR_Jump_AimUR;
  samus_shine_timer = 1;
  samus_prev_movement_type2 = kMovementType_02_NormalJumping;
  samus_y_pos = 200;
  samus_prev_y_pos = 0xBEEF;
  Require(SamusFunc_F468_NormalJump() == 1,
          "normal-jump right shinespark transition did not report a pose change");
  Require(samus_pose == kPose_C7_FaceR_ShinesparkWindup_Vert &&
          samus_y_pos == 199 &&
          samus_prev_y_pos == samus_y_pos,
          "normal-jump right shinespark transition did not select the windup pose or y history");

  samus_pose = kPose_4E_FaceL_Jump_NoAim_NoMove_NoGun;
  samus_shine_timer = 1;
  samus_prev_movement_type2 = kMovementType_01_Running;
  samus_y_pos = 200;
  samus_prev_y_pos = 0xBEEF;
  Require(SamusFunc_F468_NormalJump() == 1,
          "normal-jump left shinespark transition did not report a pose change");
  Require(samus_pose == kPose_C8_FaceL_ShinesparkWindup_Vert &&
          samus_y_pos == 200 &&
          samus_prev_y_pos == 0xBEEF,
          "normal-jump left shinespark transition changed unrelated y history");

  samus_pose = kPose_15_FaceR_Jump_AimU;
  samus_prev_pose = kPose_55_FaceR_Jumptrans_AimU;
  samus_shine_timer = 0;
  samus_x_extra_run_speed = 1;
  samus_x_extra_run_subspeed = 0;
  samus_x_accel_mode = kSamusXAccelMode_None;
  samus_anim_frame_skip = 0;
  button_config_shoot_x = kButton_X;
  joypad1_newkeys = kButton_X;
  new_projectile_direction_changed_pose = 0;
  Require(SamusFunc_F468_NormalJump() == 0,
          "normal-jump non-shinespark path reported a pose change");
  Require(samus_x_accel_mode == kSamusXAccelMode_Accelerating &&
          samus_anim_frame_skip == 1 &&
          new_projectile_direction_changed_pose ==
              (kPoseParams[kPose_15_FaceR_Jump_AimU].direction_shots_fired | 0x8000),
          "normal-jump non-shinespark path did not update accel, aim-up skip, or shot direction");

  samus_pose = kPose_4D_FaceR_Jump_NoAim_NoMove_NoGun;
  samus_prev_pose = 0;
  samus_x_extra_run_speed = 0;
  samus_x_extra_run_subspeed = 0;
  samus_x_accel_mode = 0xBEEF;
  samus_anim_frame_skip = 0;
  joypad1_newkeys = 0;
  Require(SamusFunc_F468_NormalJump() == 0 &&
          samus_x_accel_mode == kSamusXAccelMode_None,
          "normal-jump idle path did not clear horizontal accel mode");

  MiniDestroy(state);
}

static void TestPoseSpinJumpTransitionContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for spin-jump pose-transition test");
  RequireFallbackRoom(state);

  DisablePoseTransitionLiquid();
  cinematic_function = 0;
  equipped_items = 0;
  samus_pose = kPose_19_FaceR_SpinJump;
  samus_prev_movement_type2 = kMovementType_03_SpinJumping;
  samus_prev_pose_x_dir = kSamusPoseXDir_FaceLeft;
  samus_pose_x_dir = kSamusPoseXDir_FaceRight;
  samus_movement_type = kMovementType_03_SpinJumping;
  samus_x_base_speed = 1;
  samus_x_base_subspeed = 0;
  samus_x_extra_run_speed = 2;
  samus_x_extra_run_subspeed = 0;
  samus_x_accel_mode = kSamusXAccelMode_None;
  samus_anim_frame_skip = 0;
  Require(SamusFunc_F468_SpinJump() == 0,
          "spin-jump carry path reported a pose change");
  Require(samus_x_base_speed == 3 &&
          samus_x_extra_run_speed == 0 &&
          samus_x_accel_mode == kSamusXAccelMode_Decelerating &&
          samus_anim_frame_skip == 1,
          "spin-jump opposite-facing carry did not fold extra speed into base speed");

  DisablePoseTransitionLiquid();
  samus_pose = kPose_19_FaceR_SpinJump;
  samus_prev_movement_type2 = kMovementType_01_Running;
  samus_pose_x_dir = kSamusPoseXDir_FaceRight;
  samus_movement_type = kMovementType_03_SpinJumping;
  samus_anim_frame_skip = 0;
  equipped_items = kSamusEquip_ScrewAttack;
  Require(SamusFunc_F468_SpinJump() == 0 &&
          samus_pose == kPose_82_FaceL_Screwattack,
          "spin-jump screw attack path did not select the opposite-facing screw pose");

  DisablePoseTransitionLiquid();
  samus_pose = kPose_1A_FaceL_SpinJump;
  samus_pose_x_dir = kSamusPoseXDir_FaceLeft;
  samus_anim_frame_skip = 0;
  equipped_items = kSamusEquip_HiJumpBoots;
  Require(SamusFunc_F468_SpinJump() == 0 &&
          samus_pose == kPose_1B_FaceR_SpaceJump,
          "spin-jump hi-jump path did not select the opposite-facing space-jump pose");

  samus_pose = kPose_19_FaceR_SpinJump;
  samus_pose_x_dir = kSamusPoseXDir_FaceRight;
  samus_movement_type = kMovementType_03_SpinJumping;
  samus_y_pos = 200;
  fx_y_pos = Samus_GetTop_R20() - 1;
  lava_acid_y_pos = kLiquidYPos_Disabled;
  fx_liquid_options = 0;
  equipped_items = kSamusEquip_ScrewAttack;
  Require(SamusFunc_F468_SpinJump() == 0 &&
          samus_pose == kPose_19_FaceR_SpinJump,
          "spin-jump liquid gate allowed screw attack without gravity suit");

  MiniDestroy(state);
}

static void ResetSfxQueuesForContracts(void) {
  memset(sfx_readpos, 0, 3);
  memset(sfx_writepos, 0, 3);
  memset(sfx1_queue, 0, 16);
  memset(sfx2_queue, 0, 16);
  memset(sfx3_queue, 0, 16);
  debug_disable_sounds = 0;
  power_bomb_explosion_status = 0;
  game_state = kGameState_8_MainGameplay;
}

static void TestRuntimeJumpSoundShootContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for runtime jump/sound/shoot test");
  RequireFallbackRoom(state);

  button_config_jump_a = kButton_A;
  joypad1_lastkeys = kButton_A;
  joypad1_input_samusfilter = kButton_A;
  joypad1_newkeys = kButton_A;
  autojump_timer = 4;
  samus_health = 50;
  samus_prev_health_for_flash = 100;
  samus_hurt_flash_counter = 0;
  debug_invincibility = 0;
  Samus_JumpCheck();
  Require(autojump_timer == 5 &&
          joypad1_input_samusfilter == kButton_A &&
          joypad1_newinput_samusfilter == kButton_A &&
          samus_hurt_flash_counter == 1 &&
          samus_prev_health_for_flash == 50,
          "jump check did not update autojump, input filter, or hurt-flash state");

  samus_health = 50;
  samus_prev_health_for_flash = 100;
  samus_hurt_flash_counter = 0;
  debug_invincibility = 7;
  Samus_JumpCheck();
  Require(samus_health == 100 &&
          samus_prev_health_for_flash == 100 &&
          samus_hurt_flash_counter == 1,
          "jump check did not preserve health during debug invincibility");

  ResetSfxQueuesForContracts();
  samus_movement_type = kMovementType_14_WallJumping;
  samus_anim_frame = 12;
  SamusCode_1C();
  Require(sfx1_queue[0] == 0x31, "wall-jump early frame did not queue normal spin sound");

  ResetSfxQueuesForContracts();
  samus_movement_type = kMovementType_14_WallJumping;
  samus_anim_frame = 13;
  SamusCode_1C();
  Require(sfx1_queue[0] == 0x3E, "wall-jump middle frame did not queue space-jump sound");

  ResetSfxQueuesForContracts();
  samus_movement_type = kMovementType_14_WallJumping;
  samus_anim_frame = 23;
  SamusCode_1C();
  Require(sfx1_queue[0] == 0x33, "wall-jump late frame did not queue screw-attack sound");

  ResetSfxQueuesForContracts();
  samus_movement_type = kMovementType_03_SpinJumping;
  samus_pose = kPose_81_FaceR_Screwattack;
  SamusCode_1C();
  Require(sfx1_queue[0] == 0x33, "spin-jump screw pose did not queue screw-attack sound");

  ResetSfxQueuesForContracts();
  samus_movement_type = kMovementType_03_SpinJumping;
  samus_pose = kPose_1B_FaceR_SpaceJump;
  SamusCode_1C();
  Require(sfx1_queue[0] == 0x3E, "spin-jump space-jump pose did not queue space-jump sound");

  ResetSfxQueuesForContracts();
  samus_movement_type = kMovementType_03_SpinJumping;
  samus_pose = kPose_19_FaceR_SpinJump;
  SamusCode_1C();
  Require(sfx1_queue[0] == 0x31, "plain spin-jump pose did not queue normal spin sound");

  ResetSfxQueuesForContracts();
  button_config_shoot_x = kButton_X;
  joypad1_lastkeys = kButton_X;
  play_resume_charging_beam_sfx = 0x8000;
  samus_echoes_sound_flag = 1;
  speed_boost_counter = 0;
  samus_prev_movement_type = 0;
  samus_movement_type = 0;
  enable_debug = 0;
  Samus_ShootCheck();
  Require(play_resume_charging_beam_sfx == 1 &&
          samus_echoes_sound_flag == 1,
          "shoot check high-bit resume path did not defer other sound cleanup");

  ResetSfxQueuesForContracts();
  play_resume_charging_beam_sfx = 1;
  samus_echoes_sound_flag = 1;
  speed_boost_counter = 0;
  samus_prev_movement_type = 0;
  samus_movement_type = 0;
  Samus_ShootCheck();
  Require(play_resume_charging_beam_sfx == 0 &&
          samus_echoes_sound_flag == 0 &&
          sfx1_queue[0] == 0x41 &&
          sfx3_queue[0] == 0x25,
          "shoot check did not clear resume/echo state or queue charging sounds");

  ResetSfxQueuesForContracts();
  play_resume_charging_beam_sfx = 0;
  samus_echoes_sound_flag = 0;
  samus_prev_movement_type = kMovementType_03_SpinJumping;
  samus_movement_type = kMovementType_01_Running;
  flare_counter = 16;
  Samus_ShootCheck();
  Require(play_resume_charging_beam_sfx == 1 &&
          sfx1_queue[0] == 0x32,
          "shoot check did not resume charge sound after leaving spin jump");

  ResetSfxQueuesForContracts();
  play_resume_charging_beam_sfx = 0;
  samus_echoes_sound_flag = 0;
  samus_prev_movement_type = 0;
  samus_movement_type = 0;
  enable_debug = 1;
  samus_pose = kPose_19_FaceR_SpinJump;
  debug_invincibility = 7;
  Samus_ShootCheck();
  Require(debug_invincibility == 7,
          "shoot check did not preserve active debug invincibility outside face-forward poses");

  debug_invincibility = 6;
  Samus_ShootCheck();
  Require(debug_invincibility == 0,
          "shoot check did not clear inactive debug invincibility outside face-forward poses");

  MiniDestroy(state);
}

static void TestInputMomentumAndReleaseContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for input contract test");
  RequireFallbackRoom(state);

  samus_movement_type = kMovementType_01_Running;
  samus_x_base_speed = 1;
  samus_x_base_subspeed = 0;
  samus_momentum_routine_index = 0xBEEF;
  Require(Samus_Pose_Func2() == 1 &&
          samus_momentum_routine_index == 1,
          "pose momentum helper did not keep running momentum with base speed");

  samus_movement_type = kMovementType_01_Running;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_momentum_routine_index = 0xBEEF;
  Require(Samus_Pose_Func2() == 0 &&
          samus_momentum_routine_index == 2,
          "pose momentum helper did not downgrade stopped running momentum");

  samus_movement_type = kMovementType_00_Standing;
  samus_momentum_routine_index = 0xBEEF;
  Require(Samus_Pose_Func2() == 0 &&
          samus_momentum_routine_index == 2,
          "pose momentum helper did not use the standing momentum table entry");

  joypad_released_keys = 0;
  joypad1_lastkeys = kButton_A;
  joypad1_newkeys = 0;
  timed_held_input_timer = 0;
  timed_held_input = kButton_B;
  previous_timed_held_input = 0;
  ReleaseButtonsFilter(5);
  Require(joypad_released_keys == kButton_A &&
          timed_held_input_timer == 5 &&
          timed_held_input == 0 &&
          newly_held_down_timed_held_input == 0,
          "release filter did not reset timer on a changed release set");

  joypad_released_keys = kButton_A;
  joypad1_lastkeys = kButton_A;
  joypad1_newkeys = 0;
  timed_held_input_timer = 2;
  timed_held_input = kButton_B;
  previous_timed_held_input = 0;
  ReleaseButtonsFilter(5);
  Require(timed_held_input_timer == 1 &&
          timed_held_input == 0 &&
          newly_held_down_timed_held_input == 0,
          "release filter did not count down a stable release set");

  joypad_released_keys = kButton_A;
  joypad1_lastkeys = kButton_A;
  joypad1_newkeys = 0;
  timed_held_input_timer = 0;
  timed_held_input = 0;
  previous_timed_held_input = 0;
  ReleaseButtonsFilter(5);
  Require(timed_held_input_timer == 0 &&
          previous_timed_held_input == 0 &&
          timed_held_input == kButton_A &&
          newly_held_down_timed_held_input == kButton_A,
          "release filter did not publish a newly stable held input");

  MiniDestroy(state);
}

static void TestSpeedPoseRemapContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for speed pose remap test");
  RequireFallbackRoom(state);

  samus_pose = kPose_9B_FaceF_VariaGravitySuit;
  samus_prev_pose = kPose_19_FaceR_SpinJump;
  equipped_items = 0;
  samus_anim_frame_skip = 0x8000;
  SamusFunc_E633_0();
  Require(samus_pose == kPose_00_FaceF_Powersuit &&
          samus_prev_pose == kPose_00_FaceF_Powersuit &&
          samus_last_different_pose == kPose_19_FaceR_SpinJump,
          "face-forward suit pose did not downgrade when Varia/Gravity were absent");

  samus_pose = kPose_00_FaceF_Powersuit;
  samus_prev_pose = kPose_19_FaceR_SpinJump;
  equipped_items = kSamusEquip_VariaSuit;
  samus_anim_frame_skip = 0x8000;
  SamusFunc_E633_0();
  Require(samus_pose == kPose_9B_FaceF_VariaGravitySuit &&
          samus_prev_pose == kPose_9B_FaceF_VariaGravitySuit &&
          samus_last_different_pose == kPose_19_FaceR_SpinJump,
          "face-forward suit pose did not upgrade when Varia/Gravity were present");

  samus_pose = kPose_1B_FaceR_SpaceJump;
  samus_pose_x_dir = kSamusPoseXDir_FaceRight;
  samus_prev_pose = kPose_19_FaceR_SpinJump;
  equipped_items = kSamusEquip_ScrewAttack;
  samus_anim_frame_skip = 0x8000;
  SamusFunc_E633_3();
  Require(samus_pose == kPose_82_FaceL_Screwattack &&
          samus_prev_pose == kPose_82_FaceL_Screwattack,
          "speed pose remap did not upgrade space jump to screw attack");

  samus_pose = kPose_81_FaceR_Screwattack;
  samus_pose_x_dir = kSamusPoseXDir_FaceRight;
  samus_prev_pose = kPose_19_FaceR_SpinJump;
  equipped_items = 0;
  samus_anim_frame_skip = 0x8000;
  SamusFunc_E633_3();
  Require(samus_pose == kPose_1A_FaceL_SpinJump &&
          samus_prev_pose == kPose_1A_FaceL_SpinJump,
          "speed pose remap did not downgrade screw attack without equipment");

  MiniDestroy(state);
}

static void TestShinesparkHorizontalMovementContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for shinespark horizontal movement test");
  RequireFallbackRoom(state);

  samus_pose_x_dir = kSamusPoseXDir_FaceRight;
  samus_x_pos = 200;
  samus_prev_x_pos = 100;
  samus_x_subpos = 0;
  samus_x_extra_run_speed = 20;
  samus_x_extra_run_subspeed = 0;
  samus_y_accel = 0;
  samus_y_subaccel = 0;
  extra_samus_x_displacement = 0;
  extra_samus_x_subdisplacement = 0;
  samus_collision_flag = 0;
  enable_horiz_slope_coll = 0;
  Samus_ShinesparkMove_X();
  Require(samus_shine_timer == 15 &&
          samus_x_extra_run_speed == 15 &&
          samus_x_extra_run_subspeed == 0 &&
          samus_prev_x_pos == samus_x_pos - 15,
          "horizontal shinespark did not cap speed or clamp large positive x history delta");

  samus_pose_x_dir = kSamusPoseXDir_FaceLeft;
  samus_x_pos = 200;
  samus_prev_x_pos = 250;
  samus_x_subpos = 0;
  samus_x_extra_run_speed = 0;
  samus_x_extra_run_subspeed = 0;
  samus_y_accel = 0;
  samus_y_subaccel = 0;
  extra_samus_x_displacement = 0;
  extra_samus_x_subdisplacement = 0;
  samus_collision_flag = 0;
  enable_horiz_slope_coll = 0;
  Samus_ShinesparkMove_X();
  Require(samus_shine_timer == 15 &&
          samus_prev_x_pos == samus_x_pos + 15,
          "horizontal shinespark did not clamp large negative x history delta");

  MiniDestroy(state);
}

static void ConfigureSlowGrappleScrollCase(uint16 layer_x, uint16 layer_y,
                                           uint16 samus_x, uint16 samus_y) {
  slow_grabble_scrolling_flag = 1;
  scrolling_finished_hook = 0;
  time_is_frozen_flag = 1;
  layer1_x_pos = layer_x;
  layer1_y_pos = layer_y;
  layer1_x_subpos = 0;
  layer1_y_subpos = 0;
  samus_x_pos = samus_x;
  samus_y_pos = samus_y;
  samus_x_subpos = 0x1111;
  samus_y_subpos = 0x2222;
  samus_prev_x_pos = 0xAAAA;
  samus_prev_y_pos = 0xBBBB;
  samus_prev_x_subpos = 0xCCCC;
  samus_prev_y_subpos = 0xDDDD;
  absolute_moved_last_frame_x = 0;
  absolute_moved_last_frame_y = 0;
  absolute_moved_last_frame_x_fract = 0;
  absolute_moved_last_frame_y_fract = 0;
}

static void RequireSlowGrapplePrevPositionCopied(void) {
  Require(samus_prev_x_pos == samus_x_pos &&
          samus_prev_y_pos == samus_y_pos &&
          samus_prev_x_subpos == samus_x_subpos &&
          samus_prev_y_subpos == samus_y_subpos,
          "slow grapple scrolling did not copy Samus previous position history");
}

static void TestSlowGrappleScrollingContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for slow grapple scrolling test");
  RequireFallbackRoom(state);

  ConfigureSlowGrappleScrollCase(100, 100, 260, 244);
  MainScrollingRoutine();
  Require(layer1_x_pos == 103 && layer1_y_pos == 103,
          "slow grapple scrolling did not move toward far bottom-right Samus position");
  RequireSlowGrapplePrevPositionCopied();

  ConfigureSlowGrappleScrollCase(200, 200, 150, 312);
  MainScrollingRoutine();
  Require(layer1_x_pos == 197 && layer1_y_pos == 200,
          "slow grapple scrolling did not handle leftward movement with vertical deadzone");
  RequireSlowGrapplePrevPositionCopied();

  ConfigureSlowGrappleScrollCase(100, 200, 196, 250);
  MainScrollingRoutine();
  Require(layer1_x_pos == 100 && layer1_y_pos == 197,
          "slow grapple scrolling did not handle horizontal deadzone with upward correction");
  RequireSlowGrapplePrevPositionCopied();

  ConfigureSlowGrappleScrollCase(100, 100, 0x8000, 244);
  MainScrollingRoutine();
  Require(layer1_x_pos == 100 && layer1_y_pos == 100,
          "slow grapple scrolling did not skip both axes for negative Samus X");
  RequireSlowGrapplePrevPositionCopied();

  slow_grabble_scrolling_flag = 0;
  time_is_frozen_flag = 0;
  MiniDestroy(state);
}

static void RequireTransitionBottomDrawn(uint16 pose, uint16 anim_frame,
                                         bool expected_drawn, const char *message) {
  samus_pose = pose;
  samus_anim_frame = anim_frame;
  samus_bottom_half_spritemap_index = 0xBEEF;
  uint8 drawn = SamusBottomDrawn_F_Transitions();
  Require((drawn != 0) == expected_drawn, message);
  if (expected_drawn) {
    Require(samus_bottom_half_spritemap_index == 0xBEEF,
            "transition bottom-half draw path unexpectedly cleared the spritemap index");
  } else {
    Require(samus_bottom_half_spritemap_index == 0,
            "transition bottom-half hidden path did not clear the spritemap index");
  }
}

static void TestTransitionBottomDrawContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for transition bottom draw test");
  RequireFallbackRoom(state);

  RequireTransitionBottomDrawn(kPose_F1_FaceR_CrouchTrans_AimU, 5, true,
                               "aim-up transition pose did not draw the bottom half");
  RequireTransitionBottomDrawn(kPose_DB, 0, true,
                               "DB transition pose did not draw on frame zero");
  RequireTransitionBottomDrawn(kPose_DB, 1, false,
                               "DB transition pose did not hide after frame zero");
  RequireTransitionBottomDrawn(kPose_DD_TransitionBottomFrame2, 2, true,
                               "DD transition pose did not draw on frame two");
  RequireTransitionBottomDrawn(kPose_DD_TransitionBottomFrame2, 1, false,
                               "DD transition pose did not hide outside frame two");
  RequireTransitionBottomDrawn(kPose_35_FaceR_CrouchTrans, 7, true,
                               "classic crouch transition pose did not draw the bottom half");
  RequireTransitionBottomDrawn(kPose_39_MaybeUnusedTransition, 0, false,
                               "non-special transition pose did not hide the bottom half");

  MiniDestroy(state);
}

static void FillXrayHdmaTable(uint16 value) {
  for (int i = 0; i < 256; i++)
    hdma_table_1[i] = value;
}

static bool XrayHdmaTableIsFilledWith(uint16 value) {
  for (int i = 0; i < 256; i++) {
    if (hdma_table_1[i] != value)
      return false;
  }
  return true;
}

static void TestXrayHdmaBehindSamusContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for x-ray HDMA test");
  RequireFallbackRoom(state);

  demo_input = 0;
  xray_angle = kXrayAngle_FacingRight;
  layer1_y_pos = 50;
  samus_y_pos = 150;
  samus_movement_type = kMovementType_00_Standing;

  FillXrayHdmaTable(0xBEEF);
  samus_pose_x_dir = kSamusPoseXDir_FaceRight;
  layer1_x_pos = 100;
  samus_x_pos = 100;
  CalculateXrayHdmaTable();
  Require(XrayHdmaTableIsFilledWith(255),
          "right-facing x-ray beam behind Samus did not blank the HDMA table");

  FillXrayHdmaTable(0xBEEF);
  samus_pose_x_dir = kSamusPoseXDir_FaceLeft;
  layer1_x_pos = 100;
  samus_x_pos = 400;
  CalculateXrayHdmaTable();
  Require(XrayHdmaTableIsFilledWith(255),
          "left-facing x-ray beam behind Samus did not blank the HDMA table");

  MiniDestroy(state);
}

static void TestXraySetupFirefleaContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for x-ray setup test");
  RequireFallbackRoom(state);

  fx_type = kXrayFirefleaFxType;
  fx_layer_blending_config_c = 0;
  reg_COLDATA[0] = 4;
  reg_COLDATA[1] = 0xAA;
  reg_COLDATA[2] = 0xBB;
  HdmaobjPreInstr_XraySetup(0);
  Require(fx_layer_blending_config_c == kXrayFirefleaBlendMask &&
          reg_COLDATA[0] == 0x27 &&
          reg_COLDATA[1] == 0x47 &&
          reg_COLDATA[2] == 0x87,
          "x-ray fireflea setup did not restore colors below the brightness threshold");

  fx_type = kXrayFirefleaFxType;
  fx_layer_blending_config_c = 0x0080;
  reg_COLDATA[0] = 7;
  reg_COLDATA[1] = 0xAA;
  reg_COLDATA[2] = 0xBB;
  HdmaobjPreInstr_XraySetup(0);
  Require(fx_layer_blending_config_c == (0x0080 | kXrayFirefleaBlendMask) &&
          reg_COLDATA[0] == 7 &&
          reg_COLDATA[1] == 0xAA &&
          reg_COLDATA[2] == 0xBB,
          "x-ray fireflea setup changed colors at or above the brightness threshold");

  MiniDestroy(state);
}

static void TestPoseCrouchTransitionDispatchContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for pose crouch-transition dispatch test");
  RequireFallbackRoom(state);

  samus_pose = kPose_35_FaceR_CrouchTrans;
  samus_momentum_routine_index = 0;
  speed_boost_counter = 0;
  equipped_items = kSamusEquip_MorphBall;
  Require(SamusFunc_F468_CrouchTransEtc() == 0,
          "crouch-transition low table returned an unexpected transition result");
  Require(samus_momentum_routine_index == kSamusMomentumRoutine_CrouchTransEtc,
          "crouch-transition low table did not select the transition momentum routine");

  samus_pose = kPose_DB;
  samus_momentum_routine_index = 0xBEEF;
  equipped_items = kSamusEquip_MorphBall;
  Require(SamusFunc_F468_CrouchTransEtc() == 0,
          "crouch-transition high table returned an unexpected transition result");
  Require(samus_momentum_routine_index == 0xBEEF,
          "crouch-transition high table unexpectedly changed the momentum routine");

  samus_pose = kPose_F1_FaceR_CrouchTrans_AimU;
  samus_momentum_routine_index = 0;
  speed_boost_counter = 0;
  equipped_items = kSamusEquip_MorphBall;
  Require(SamusFunc_F468_CrouchTransEtc() == 0,
          "crouch-transition aim-up table returned an unexpected transition result");
  Require(samus_momentum_routine_index == kSamusMomentumRoutine_CrouchTransEtc,
          "crouch-transition aim-up table did not select the transition momentum routine");

  samus_pose = kPose_F7_FaceR_StandTrans_AimU;
  samus_momentum_routine_index = 0;
  Require(SamusFunc_F468_CrouchTransEtc() == 0,
          "crouch-transition stand-transition range returned an unexpected transition result");
  Require(samus_momentum_routine_index == kSamusMomentumRoutine_CrouchTransEtc,
          "crouch-transition stand-transition range did not select the transition momentum routine");

  MiniDestroy(state);
}

static void TestUnsupportedAdvancedSpinPoseContracts(void) {
  MiniGameState *state = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(state != NULL, "MiniCreate failed for unsupported advanced spin pose test");
  RequireFallbackRoom(state);

  samus_pose = kPose_81_FaceR_Screwattack;
  samus_pose_x_dir = kSamusPoseXDir_FaceRight;
  samus_prev_pose = samus_pose;
  samus_anim_frame_skip = 0x8000;
  equipped_items = 0;
  sub_82A42A();
  Require(samus_pose == kPose_1A_FaceL_SpinJump,
          "unsupported screw attack did not downgrade to spin jump");

  samus_pose = kPose_82_FaceL_Screwattack;
  samus_pose_x_dir = kSamusPoseXDir_FaceLeft;
  samus_prev_pose = samus_pose;
  equipped_items = kSamusEquip_ScrewAttack;
  sub_82A42A();
  Require(samus_pose == kPose_82_FaceL_Screwattack,
          "supported screw attack pose changed unexpectedly");

  samus_pose = kPose_1B_FaceR_SpaceJump;
  samus_pose_x_dir = kSamusPoseXDir_FaceLeft;
  samus_prev_pose = samus_pose;
  equipped_items = 0;
  sub_82A42A();
  Require(samus_pose == kPose_19_FaceR_SpinJump,
          "unsupported space jump did not downgrade to spin jump");

  samus_pose = kPose_1C_FaceL_SpaceJump;
  samus_pose_x_dir = kSamusPoseXDir_FaceLeft;
  samus_prev_pose = samus_pose;
  equipped_items = kSamusEquip_GravitySuit;
  sub_82A42A();
  Require(samus_pose == kPose_1C_FaceL_SpaceJump,
          "gravity-suit space jump pose changed unexpectedly");

  MiniDestroy(state);
}

static void RunFallbackTestsInIsolatedCwd(void) {
  char original_cwd[1024];
  char temp_dir[] = "/tmp/sm_rev_mini_rollback_api_XXXXXX";
  Require(getcwd(original_cwd, sizeof(original_cwd)) != NULL, "getcwd failed");
  Require(mkdtemp(temp_dir) != NULL, "mkdtemp failed");
  Require(chdir(temp_dir) == 0, "chdir to fallback temp dir failed");

  MiniStubs_SetRoomExportPath("missing_room.json");
  TestBasicRollbackApi();
  TestProjectileRollbackProgression();
  TestSuperMissileFireGateContracts();
  TestLiquidEnvironmentContracts();
  TestCrouchingTransitionContracts();
  TestBlockCollisionTransitionContracts();
  TestSolidEnemyCollisionContracts();
  TestPoseTransitionTableContracts();
  TestBallBounceTransitionContracts();
  TestTransitionA7PoseAdjustmentContracts();
  TestCrateriaLandingGraphicsContracts();
  TestCrateriaFootstepGraphicsContracts();
  TestScrewSpeedPaletteContracts();
  TestMiscPaletteHurtFlashContracts();
  TestTransitionDispatcherContracts();
  TestWalkedIntoSomethingContracts();
  TestPoseTurnTransitionTableContracts();
  TestPoseFacingFlipSpeedCarryContracts();
  TestPoseNormalJumpTransitionContracts();
  TestPoseSpinJumpTransitionContracts();
  TestRuntimeJumpSoundShootContracts();
  TestInputMomentumAndReleaseContracts();
  TestSpeedPoseRemapContracts();
  TestShinesparkHorizontalMovementContracts();
  TestSlowGrappleScrollingContracts();
  TestTransitionBottomDrawContracts();
  TestXrayHdmaBehindSamusContracts();
  TestXraySetupFirefleaContracts();
  TestPoseCrouchTransitionDispatchContracts();
  TestUnsupportedAdvancedSpinPoseContracts();
  TestLongScriptDeterminism();
  TestRepeatedSaveLoadCycles();
  MiniStubs_SetRoomExportPath(NULL);

  Require(chdir(original_cwd) == 0, "failed to restore original cwd");
  (void)rmdir(temp_dir);
}

static void TestRomRoomDeterminismIfAvailable(void) {
  MiniStubs_SetRoomExportPath(NULL);
  TestPoseTurnTransitionTableContracts();
  MiniGameState *first = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(first != NULL, "MiniCreate failed for ROM room determinism test");
  if (!first->uses_rom_room) {
    MiniDestroy(first);
    return;
  }
  Require(first->uses_original_gameplay_runtime,
          "ROM room path did not enable original gameplay runtime");
  char first_room_handle[sizeof(first->room_handle)];
  memcpy(first_room_handle, first->room_handle, sizeof(first_room_handle));
  uint64_t first_hash = StepRomRoomFrames(first, kRomRoomFrameCount);
  MiniDestroy(first);

  MiniGameState *second = MiniCreate(kMiniGameWidth, kMiniGameHeight);
  Require(second != NULL, "MiniCreate failed for second ROM room determinism test");
  Require(second->uses_rom_room, "second ROM room boot did not use a ROM room");
  Require(memcmp(first_room_handle, second->room_handle, sizeof(first_room_handle)) == 0,
          "ROM room boot selected different room handles");

  uint64_t second_hash = StepRomRoomFrames(second, kRomRoomFrameCount);
  Require(first_hash == second_hash, "ROM-backed room path was not deterministic");

  size_t snapshot_size;
  uint8 *snapshot = AllocSnapshot(&snapshot_size);
  SaveSnapshot(second, snapshot, snapshot_size);
  uint64_t branch_hash = StepRomRoomFrames(second, kRepeatedCycleWindow);
  LoadSnapshot(second, snapshot, snapshot_size);
  Require(StepRomRoomFrames(second, kRepeatedCycleWindow) == branch_hash,
          "ROM-backed rollback branch did not replay deterministically");

  free(snapshot);
  MiniDestroy(second);
}

int main(void) {
  RunFallbackTestsInIsolatedCwd();
  TestRomRoomDeterminismIfAvailable();
  return 0;
}
