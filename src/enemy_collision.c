// Enemies
#include <stdio.h>
#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"
#include "torizo_config.h"

#define g_off_A0C2DA ((uint16*)RomFixedPtr(0xa0c2da))
#define CHECK_locret_A0C434(Ek) (byte_A0C435[Ek] & 0x80 ? -1 : 0)
#define g_word_A0C49F ((uint16*)RomFixedPtr(0xa0c49f))
#define kAlignYPos_Tab0 ((uint8*)RomFixedPtr(0x948b2b))

typedef struct EnemyBlockCollInfo {
  int32 ebci_r18_r20;
  uint16 ebci_r24;
  uint16 ebci_r26;
  uint16 ebci_r28;
  uint16 ebci_r30;
  uint16 ebci_r32;
} EnemyBlockCollInfo;

typedef uint8 Func_EnemyBlockCollInfo_U8(EnemyBlockCollInfo *ebci);

static uint8 EnemyBlockCollReact_Vert(EnemyBlockCollInfo *ebci, uint16 k);
static uint8 EnemyBlockCollReact_Horiz(EnemyBlockCollInfo *ebci, uint16 k);
static uint8 Enemy_MoveRight_IgnoreSlopes_Inner(uint16 k, int32 amount32, uint16 r32);
static uint8 EnemyBlockCollHorizReact_Slope_NonSquare(EnemyBlockCollInfo *ebci);
static uint8 EnemyBlockCollHorizReact_Slope_Square(EnemyBlockCollInfo *ebci, uint16 k, uint16 a);
static uint8 EnemyBlockCollVertReact_Slope_NonSquare(EnemyBlockCollInfo *ebci);
static uint8 EnemyBlockCollVertReact_Slope_Square(EnemyBlockCollInfo *ebci, uint16 a, uint16 k);

void func_nullsub_169(void);
void func_nullsub_170(void);

void EnemyCollisionHandler(void) {  // 0xA09758
  if ((gEnemyData(cur_enemy_index)->extra_properties & 4) != 0) {
    EprojCollHandler_Multibox();
    EnemyBombCollHandler_Multibox();
    EnemySamusCollHandler_Multibox();
  } else {
    EprojCollHandler();
    EnemyBombCollHandler();
    EnemySamusCollHandler();
  }
}

void func_nullsub_4(void) {
  ;
}

PairU16 GrappleBeam_CollDetect_Enemy(void) {  // 0xA09E9A
  VoidP grapple_ai;
  EnemyData *E;

  CallSomeSamusCode(0xD);
  collision_detection_index = 0;
  for (int i = 0;; i++) {
    cur_enemy_index = interactive_enemy_indexes[i];
    if (cur_enemy_index == 0xFFFF)
      return (PairU16){0, 0};
    E = gEnemyData(cur_enemy_index);
    if (!E->invincibility_timer) {
      uint16 dx = abs16(E->x_pos - grapple_beam_end_x_pos);
      bool within_x = dx < E->x_width;
      uint16 x_gap = dx - E->x_width;
      if (within_x || x_gap < 8) {
        uint16 dy = abs16(E->y_pos - grapple_beam_end_y_pos);
        bool within_y = dy < E->y_height;
        uint16 y_gap = dy - E->y_height;
        if (within_y || y_gap < 8)
          break;
      }
    }
  }
  E->ai_handler_bits = 1;
  uint16 reaction = 0;
  uint16 enemy_ptr = E->enemy_ptr;
  grapple_ai = get_EnemyDef_A2(E->enemy_ptr)->grapple_ai;
  if (grapple_ai != FUNC16(Enemy_GrappleReact_NoInteract_A0)) {
    reaction = 1;
    if (grapple_ai != FUNC16(Enemy_GrappleReact_SamusLatchesOn_A0)) {
      reaction = 2;
      if (grapple_ai != FUNC16(Enemy_GrappleReact_KillEnemy_A0)) {
        reaction = 3;
        if (grapple_ai != FUNC16(Enemy_GrappleReact_CancelBeam_A0)) {
          reaction = 4;
          if (grapple_ai != FUNC16(Enemy_GrappleReact_SamusLatchesNoInvinc_A0)) {
            reaction = 5;
            if (grapple_ai != FUNC16(Enemy_GrappleReact_SamusLatchesParalyze_A0)) {
              reaction = 6;
              if (grapple_ai != FUNC16(Enemy_GrappleReact_HurtSamus_A0))
                reaction = 0;
            }
          }
        }
      }
    }
  }
  if (reaction == 1 || reaction == 4 || reaction == 5) {
    EnemyData *enemy = gEnemyData(cur_enemy_index);
    grapple_beam_end_x_pos = enemy->x_pos;
    grapple_beam_end_y_pos = enemy->y_pos;
  }
  return (PairU16){reaction, enemy_ptr};
}

void SwitchEnemyAiToMainAi(void) {  // 0xA09F6D
  EnemyData *E = gEnemyData(cur_enemy_index);
  E->ai_handler_bits = 0;
  E->invincibility_timer = 0;
  E->frozen_timer = 0;
  E->shake_timer = 0;
}

void SamusLatchesOnWithGrapple(void) {  // 0xA09F7D
  EnemyData *E = gEnemyData(cur_enemy_index);
  grapple_beam_end_x_pos = E->x_pos;
  grapple_beam_end_y_pos = E->y_pos;
  if (E->frozen_timer) {
    E->ai_handler_bits = 4;
  } else {
    EnemyDef *ED = get_EnemyDef_A2(E->enemy_ptr);
    E->flash_timer = ED->hurt_ai_time ? ED->hurt_ai_time : 4;
    E->ai_handler_bits = 0;
  }
}

void EnemyGrappleDeath(void) {  // 0xA09FC4
  gEnemySpawnData(cur_enemy_index)->cause_of_death = 4;
  EnemyDeathAnimation(cur_enemy_index, 0);
  gEnemyData(cur_enemy_index)->ai_handler_bits = 0;
}

void Enemy_SwitchToFrozenAi(void) {  // 0xA09FDF
  gEnemyData(cur_enemy_index)->ai_handler_bits = 4;
}

void SamusLatchesOnWithGrappleNoInvinc(void) {  // 0xA09FE9
  EnemyData *E = gEnemyData(cur_enemy_index);
  if (E->frozen_timer) {
    E->ai_handler_bits = 4;
  } else {
    CallEnemyAi(E->bank << 16 | get_EnemyDef_A2(E->enemy_ptr)->main_ai);
    E->ai_handler_bits = 0;
  }
  grapple_beam_end_x_pos = E->x_pos;
  grapple_beam_end_y_pos = E->y_pos;
}

void SamusLatchesOnWithGrappleParalyze(void) {  // 0xA0A03E
  EnemyData *E = gEnemyData(cur_enemy_index);
  EnemyDef *ED = get_EnemyDef_A2(E->enemy_ptr);
  E->flash_timer = ED->hurt_ai_time ? ED->hurt_ai_time : 4;
  E->ai_handler_bits = 0;
  E->extra_properties |= 1;
}

void SamusHurtFromGrapple(void) {  // 0xA0A070
  gEnemyData(cur_enemy_index)->ai_handler_bits = 4;
}

void SamusProjectileInteractionHandler(void) {  // 0xA09785
  enemy_processing_stage = 10;
  if (flag_disable_projectile_interaction)
    return;
  int num_colls_to_check = 5;
  if (bomb_counter) {
    num_colls_to_check = 10;
  } else if (!projectile_counter) {
    return;
  }
  if (projectile_invincibility_timer || samus_contact_damage_index)
    return;
  for (int pidx = 0; pidx != num_colls_to_check;pidx++) {
    collision_detection_index = pidx;
    if (!projectile_damage[pidx] || sign16(projectile_type[pidx]) || !sign16((projectile_type[pidx] & 0xF00) - 1792))
      continue;
    if ((projectile_dir[pidx] & 0x10) != 0)
      continue;
    if (abs16(projectile_x_pos[pidx] - samus_x_pos) - projectile_x_radius[pidx] < samus_x_radius &&
        abs16(projectile_y_pos[pidx] - samus_y_pos) - projectile_y_radius[pidx] < samus_y_radius) {
      if ((projectile_type[pidx] & 0xFF00) != 768 && (projectile_type[pidx] & 0xFF00) != 1280) {
        projectile_dir[pidx] |= 0x10;
        Samus_DealDamage(SuitDamageDivision(projectile_damage[pidx]));
        samus_invincibility_timer = 96;
        samus_knockback_timer = 5;
        assert(0);
        uint16 v0 = 0;
        knockback_x_dir = (int16)(samus_x_pos - eproj_x_pos[v0 >> 1]) >= 0;
        return;
      }
      if (projectile_variables[pidx] == 8) {
        bomb_jump_dir = (samus_x_pos == projectile_x_pos[pidx]) ? 2 :
            (int16)(samus_x_pos - projectile_x_pos[pidx]) < 0 ? 1 : 3;
      }
    }
  }
}

void EprojSamusCollDetect(void) {  // 0xA09894
  enemy_processing_stage = 11;
  if (samus_invincibility_timer || samus_contact_damage_index)
    return;
  for(int i = 17; i >= 0; i--) {
    if (eproj_id[i] && (eproj_properties[i] & 0x2000) == 0 && eproj_radius[i]) {
      uint16 varE20 = LOBYTE(eproj_radius[i]);
      uint16 varE22 = HIBYTE(eproj_radius[i]);
      if (abs16(samus_x_pos - eproj_x_pos[i]) - samus_x_radius < varE20 &&
          abs16(samus_y_pos - eproj_y_pos[i]) - samus_y_radius < varE22) {
        collision_detection_index = i * 2;
        HandleEprojCollWithSamus(i * 2);
      }
    }
  }
}

void HandleEprojCollWithSamus(uint16 k) {  // 0xA09923
  samus_invincibility_timer = 96;
  samus_knockback_timer = 5;
  int v3 = k >> 1;
  uint16 eproj_def = eproj_id[v3];
  uint16 v1 = *((uint16 *)RomPtr_86(*(uint16 *)((uint8 *)eproj_id + k)) + 5);
  if (v1) {
    int v2 = k >> 1;
    eproj_instr_list_ptr[v2] = v1;
    eproj_instr_timers[v2] = 1;
  }
  if ((eproj_properties[v3] & 0x4000) == 0)
    *(uint16 *)((uint8 *)eproj_id + k) = 0;
  Samus_DealDamage(SuitDamageDivision(eproj_properties[v3] & 0xFFF));
  knockback_x_dir = (int16)(samus_x_pos - eproj_x_pos[v3]) >= 0;
  if (TorizoConfig_IsChozoOrbEproj(eproj_def))
    TorizoConfig_OnChozoOrbHitSamus();
}

void EprojProjCollDet(void) {  // 0xA0996C
  enemy_processing_stage = 12;
  if (!projectile_counter)
    return;
  for(int i = 17; i >= 0; i--) {
    if (!eproj_id[i] || (eproj_properties[i] & 0x8000) == 0)
      continue;
    for(int j = 0; j < 5; j++) {
      if (eproj_flags[i] == 2)
        break;
      uint16 v4 = projectile_type[j];
      if (v4 && (v4 & 0xF00) != 768 && (v4 & 0xF00) != 1280 && sign16((v4 & 0xF00) - 1792)) {
        if ((eproj_x_pos[i] & 0xFFE0) == (projectile_x_pos[j] & 0xFFE0) && 
            (eproj_y_pos[i] & 0xFFE0) == (projectile_y_pos[j] & 0xFFE0)) {
          HandleEprojCollWithProj(i * 2, j * 2);
        }
      }
    }
  }
}

void HandleEprojCollWithProj(uint16 k, uint16 j) {  // 0xA099F9
  int i = j >> 1;
  if ((projectile_type[i] & 8) == 0)
    projectile_dir[i] |= 0x10;
  if (eproj_flags[k >> 1] == 1) {
    int v4 = k >> 1;
    CreateSpriteAtPos(projectile_x_pos[v4], projectile_y_pos[v4], 6, 0);
    QueueSfx1_Max6(0x3D);
  } else {
    int j = k >> 1;
    eproj_G[j] = projectile_type[i];
    eproj_instr_list_ptr[j] = get_EprojDef(eproj_id[j])->shot_instruction_list;
    eproj_instr_timers[j] = 1;
    eproj_pre_instr[j] = FUNC16(EprojPreInstr_nullsub_83);
    eproj_properties[j] &= 0xFFF;
  }
}

void CallHitboxTouch(uint32 ea) {
  switch (ea) {
  case fnEnemy_NormalTouchAI_A0: Enemy_NormalTouchAI_A0(); return;  // 0xa08023
  case fnMaridiaLargeSnail_Func_12: MaridiaLargeSnail_Func_12(); return;  // 0xa2d388
  case fnMaridiaLargeSnail_Touch: MaridiaLargeSnail_Touch(); return;  // 0xa2d38c
  case fnEnemy_NormalTouchAI_A4: Enemy_NormalTouchAI_A4(); return;  // 0xa48023
  case fnCrocomire_Func_92: Crocomire_Func_92(); return;  // 0xa4b93d
  case fnnullsub_34: return;  // 0xa4b950
  case fnnullsub_170_A5: return;  // 0xa5804c
  case fnDraygon_Touch: Draygon_Touch(); return;  // 0xa595ea
  case fnSporeSpawn_Touch: SporeSpawn_Touch(); return;  // 0xa5edec
  case fnsub_A6DF59: sub_A6DF59(); return;  // 0xa6df59
  case fnCeresSteam_Touch: CeresSteam_Touch(); return;  // 0xa6f03f
  case fnnullsub_170_A7: return;  // 0xa7804c
  case fnKraid_Touch_ArmFoot: Kraid_Touch_ArmFoot(); return;  // 0xa7948b
  case fnKraidsArm_Touch: KraidsArm_Touch(); return;  // 0xa79490
  case fnPhantoon_Touch: Phantoon_Touch(); return;  // 0xa7dd95
  case fnnullsub_47: return;  // 0xa9b5c5
  case fnMotherBrainsBrain_Touch: MotherBrainsBrain_Touch(); return;  // 0xa9b5c6
  case fnGoldTorizo_Touch: GoldTorizo_Touch(); return;  // 0xaac977
  case fnWalkingSpacePirates_Touch: WalkingSpacePirates_Touch(); return;  // 0xb2876c
  default: Unreachable();
  }
}
void CallHitboxShot(uint32 ea, uint16 j) {  // 0xA09D17
  switch (ea) {
  case fnEnemy_NormalShotAI_A0: Enemy_NormalShotAI_A0(); return;  // 0xa0802d
  case fnnullsub_170_A2: return;  // 0xa2804c
  case fnMaridiaLargeSnail_Shot: MaridiaLargeSnail_Shot(); return;  // 0xa2d3b4
  case fnEnemy_NormalShotAI_A4: Enemy_NormalShotAI_A4(); return;  // 0xa4802d
  case fnCrocomire_Func_93: Crocomire_Func_93(); return;  // 0xa4b951
  case fnCrocomire_Func_94: Crocomire_Func_94(); return;  // 0xa4b968
  case fnCrocomire_Func_95: Crocomire_Func_95(); return;  // 0xa4ba05
  case fnCrocomire_Func_1: Crocomire_Func_1(); return;  // 0xa4bab4
  case fnEnemy_NormalShotAI_A5: Enemy_NormalShotAI_A5(); return;  // 0xa5802d
  case fnCreateADudShot_A5: CreateADudShot(); return;  // 0xa58046
  case fnnullsub_170_A5: return;  // 0xa5804c
  case fnDraygon_Shot: Draygon_Shot(); return;  // 0xa595f0
  case fnSporeSpawn_Shot: SporeSpawn_Shot(); return;  // 0xa5ed5a
  case fnnullsub_170_A6: return;  // 0xa6804c
  case fnRidley_Shot: Ridley_Shot(); return;  // 0xa6df8a
  case fnnullsub_170_A7: return;  // 0xa7804c
  case fnnullsub_43: return;  // 0xa794b5
  case fnKraid_Arm_Shot: Kraid_Arm_Shot(j); return;  // 0xa794b6
  case fnPhantoon_Shot: Phantoon_Shot(); return;  // 0xa7dd9b
  case fnMotherBrainsBody_Shot: MotherBrainsBody_Shot(); return;  // 0xa9b503
  case fnMotherBrainsBrain_Shot: MotherBrainsBrain_Shot(); return;  // 0xa9b507
  case fnTorizo_Shot: Torizo_Shot(); return;  // 0xaac97c
  case fnnullsub_271: return;  // 0xaac9c1
  case fnTorizo_Func_8: Torizo_Func_8(); return;  // 0xaac9c2
  case fnWalkingSpacePirates_Shot: WalkingSpacePirates_Shot(); return;  // 0xb28779
  case fnWalkingSpacePirates_87C8: WalkingSpacePirates_87C8(); return;  // 0xb287c8
  case fnWalkingSpacePirates_883E: WalkingSpacePirates_883E(); return;  // 0xb2883e
  default: Unreachable();
  }
}

void EnemySamusCollHandler_Multibox(void) {  // 0xA09A5A
  EnemyData *E = gEnemyData(cur_enemy_index);
  enemy_processing_stage = 6;
  if (!E->spritemap_pointer)
    return;

  uint16 touch_ai = get_EnemyDef_A2(E->enemy_ptr)->touch_ai;
  if (touch_ai == FUNC16(nullsub_170) || touch_ai == FUNC16(nullsub_169))
    return;

  if (samus_contact_damage_index) {
    samus_invincibility_timer = 0;
  } else if (samus_invincibility_timer) {
    return;
  }
  if (!sign16(E->spritemap_pointer))
    return;

  uint16 samus_right_border_coll = samus_x_radius + samus_x_pos;
  uint16 samus_left_border_coll = samus_x_pos - samus_x_radius;
  uint16 samus_bottom_border_coll = samus_y_radius + samus_y_pos;
  uint16 samus_top_border_coll = samus_y_pos - samus_y_radius;
  int n = *RomPtrWithBank(E->bank, E->spritemap_pointer);
  uint16 enemy_spritemap_entry_pointer = E->spritemap_pointer + 2;
  do {
    ExtendedSpriteMap *ES = get_ExtendedSpriteMap(E->bank, enemy_spritemap_entry_pointer);
    uint16 coll_x_pos = ES->xpos + E->x_pos;
    uint16 coll_y_pos = ES->ypos + E->y_pos;
    const uint8 *p = RomPtrWithBank(E->bank, ES->hitbox_ptr_);
    int m = GET_WORD(p);
    for(Hitbox *hb = (Hitbox *)(p + 2); m; m--, hb++) {
      if ((int16)(hb->left + coll_x_pos - samus_right_border_coll) < 0
          && (int16)(hb->right + coll_x_pos - samus_left_border_coll) >= 0
          && (int16)(hb->top + coll_y_pos - samus_bottom_border_coll) < 0
          && (int16)(hb->bottom + coll_y_pos - samus_top_border_coll) >= 0) {
        CallHitboxTouch(E->bank << 16 | hb->func_ptr);
        return;
      }
    }
    enemy_spritemap_entry_pointer += 8;
  } while (--n);
}

void EprojCollHandler_Multibox(void) {  // 0xA09B7F
  EnemyData *E = gEnemyData(cur_enemy_index);
  enemy_processing_stage = 3;
  if (!projectile_counter || !E->spritemap_pointer || E->spritemap_pointer == addr_kExtendedSpritemap_Nothing_A0)
    return;
  uint16 shot_ai = get_EnemyDef_A2(E->enemy_ptr)->shot_ai;
  if (shot_ai == FUNC16(nullsub_170) || shot_ai == FUNC16(nullsub_169))
    return;
  if ((E->properties & 0x400) != 0 || E->invincibility_timer || E->enemy_ptr == addr_kEnemyDef_DAFF)
    return;
  for(int pidx = 0; pidx < 5; pidx++) {
    uint16 v4 = projectile_type[pidx];
    if (!(v4 && (v4 & 0xF00) != 768 && (v4 & 0xF00) != 1280 && sign16((v4 & 0xF00) - 1792)))
      continue;
    if (!sign16(E->spritemap_pointer))
      Unreachable();
    const uint8 *esep = RomPtrWithBank(E->bank, E->spritemap_pointer);
    int n = esep[0];
    for(ExtendedSpriteMap *ES = (ExtendedSpriteMap *)(esep + 2); n; n--, ES++) {
      uint16 coll_x_pos = ES->xpos + E->x_pos, coll_y_pos = ES->ypos + E->y_pos;
      const uint8 *p = RomPtrWithBank(E->bank, ES->hitbox_ptr_);
      int m = GET_WORD(p);
      for (Hitbox *hb = (Hitbox *)(p + 2); m; m--, hb++) {
        if ((int16)(projectile_x_radius[pidx] + projectile_x_pos[pidx] - (coll_x_pos + hb->left)) >= 0 && 
            (int16)(projectile_x_pos[pidx] - projectile_x_radius[pidx] - (coll_x_pos + hb->right)) < 0 &&
            (int16)(projectile_y_radius[pidx] + projectile_y_pos[pidx] - (coll_y_pos + hb->top)) >= 0 &&
            (int16)(projectile_y_pos[pidx] - projectile_y_radius[pidx] - (coll_y_pos + hb->bottom)) < 0) {
          if ((projectile_type[pidx] & 0xF00) == 512) {
            earthquake_timer = 30;
            earthquake_type = 18;
          }
          if ((E->properties & 0x1000) != 0 || (projectile_type[pidx] & 8) == 0)
            projectile_dir[pidx] |= 0x10;
          collision_detection_index = pidx;
          CallHitboxShot(E->bank << 16 | hb->func_ptrA, pidx * 2);
          return;
        }
      }
    }
  }
}

void EnemyBombCollHandler_Multibox(void) {  // 0xA09D23
  EnemyData *E = gEnemyData(cur_enemy_index);
  enemy_processing_stage = 4;
  if (!E->spritemap_pointer || (E->properties & 0x400) != 0 || E->invincibility_timer)
    return;
  uint16 shot_ai = get_EnemyDef_A2(E->enemy_ptr)->shot_ai;
  if (shot_ai == FUNC16(nullsub_170) || shot_ai == FUNC16(nullsub_169) || !bomb_counter)
    return;
  for (int pidx = 5; pidx != 10; pidx++) {
    if (!projectile_x_pos[pidx])
      continue;
    uint16 v4 = projectile_type[pidx];
    if (!(v4 && (v4 & 0xF00) == 1280 && !projectile_variables[pidx]))
      continue;
    if (!sign16(E->spritemap_pointer))
      Unreachable();
    const uint8 *esep = RomPtrWithBank(E->bank, E->spritemap_pointer);
    int n = GET_WORD(esep);
    for (ExtendedSpriteMap *ES = (ExtendedSpriteMap *)(esep + 2); n; n--, ES++) {
      uint16 coll_x_pos = ES->xpos + E->x_pos, coll_y_pos = ES->ypos + E->y_pos;
      const uint8 *p = RomPtrWithBank(E->bank, ES->hitbox_ptr_);
      int m = GET_WORD(p);
      for (Hitbox *hb = (Hitbox *)(p + 2); m; m--, hb++) {
        if ((int16)(projectile_x_radius[pidx] + projectile_x_pos[pidx] - (coll_x_pos + hb->left)) >= 0 &&
            (int16)(projectile_x_pos[pidx] - projectile_x_radius[pidx] - (coll_x_pos + hb->right)) < 0 &&
            (int16)(projectile_y_radius[pidx] + projectile_y_pos[pidx] - (coll_y_pos + hb->top)) >= 0 &&
            (int16)(projectile_y_pos[pidx] - projectile_y_radius[pidx] - (coll_y_pos + hb->bottom)) < 0) {
          projectile_dir[pidx] |= 0x10;
          collision_detection_index = pidx;
          CallHitboxShot(E->bank << 16 | hb->func_ptrA, pidx * 2);
          return;
        }
      }
    }
  }
}

void EnemySamusCollHandler(void) {  // 0xA0A07A
  EnemyData *E = gEnemyData(cur_enemy_index);
  enemy_processing_stage = 9;
  if (!E->spritemap_pointer)
    return;
  if (samus_contact_damage_index) {
    samus_invincibility_timer = 0;
  } else if (samus_invincibility_timer) {
    if (E->enemy_ptr != addr_kEnemyDef_DAFF)
      return;
    uint16 some_flag = gEnemySpawnData(cur_enemy_index)->some_flag;
    if (some_flag == 0 || some_flag == 8)
      return;
  }
  EnemyDef *ED = get_EnemyDef_A2(E->enemy_ptr);
  if (ED->touch_ai == FUNC16(nullsub_170) || ED->touch_ai == FUNC16(nullsub_169))
    return;
  if (abs16(samus_x_pos - E->x_pos) - samus_x_radius < E->x_width &&
      abs16(samus_y_pos - E->y_pos) - samus_y_radius < E->y_height) {
    // r20 = 2 * E->spritemap_pointer;
    if (E->enemy_ptr == addr_kEnemyDef_DAFF || !E->frozen_timer)
      CallEnemyAi(E->bank << 16 | ED->touch_ai);
  }
}

void EprojCollHandler(void) {  // 0xA0A143
  EnemyData *E = gEnemyData(cur_enemy_index);
  enemy_processing_stage = 7;
  if (!projectile_counter)
    return;
  if (!E->spritemap_pointer || E->spritemap_pointer == addr_kSpritemap_Nothing_A0 || (E->properties & 0x400) != 0 ||
      E->enemy_ptr == addr_kEnemyDef_DAFF || E->invincibility_timer)
    return;
  for (int pidx = 0; pidx < 5; pidx++) {
    uint16 j = projectile_type[pidx];
    if (j && (j & 0xF00) != 768 && (j & 0xF00) != 1280 && sign16((j & 0xF00) - 1792)) {
      uint16 x = abs16(projectile_x_pos[pidx] - E->x_pos);
      uint16 y = abs16(projectile_y_pos[pidx] - E->y_pos);
      if (x - projectile_x_radius[pidx] < E->x_width) {
        if (y - projectile_y_radius[pidx] < E->y_height) {
          if ((projectile_type[pidx] & 0xF00) == 512) {
            earthquake_timer = 30;
            earthquake_type = 18;
          }
          if ((E->properties & 0x1000) != 0 || (projectile_type[pidx] & 8) == 0)
            projectile_dir[pidx] |= 0x10;
          collision_detection_index = pidx;
          CallEnemyAi(E->bank << 16 | get_EnemyDef_A2(E->enemy_ptr)->shot_ai);
          return;
        }
      }
    }
  }
}

void EnemyBombCollHandler(void) {  // 0xA0A236
  EnemyData *E = gEnemyData(cur_enemy_index);
  enemy_processing_stage = 8;
  if (!bomb_counter || !E->spritemap_pointer ||
      E->invincibility_timer || E->enemy_ptr == addr_kEnemyDef_DAFF)
    return;
  for(int pidx = 5; pidx < 10; pidx++) {
    if (!projectile_type[pidx] || projectile_variables[pidx] ||
        (projectile_type[pidx] & 0xF00) != 1280 && (projectile_type[pidx] & 0x8000) == 0)
      continue;
    if (abs16(projectile_x_pos[pidx] - E->x_pos) - projectile_x_radius[pidx] < E->x_width && 
        abs16(projectile_y_pos[pidx] - E->y_pos) - projectile_y_radius[pidx] < E->y_height) {
      collision_detection_index = pidx;
      projectile_dir[pidx] |= 0x10;
      CallEnemyAi(E->bank << 16 | get_EnemyDef_A2(E->enemy_ptr)->shot_ai);
      return;
    }
  }
}

void ProcessEnemyPowerBombInteraction(void) {  // 0xA0A306
  enemy_processing_stage = 5;
  uint16 rx = HIBYTE(power_bomb_explosion_radius);
  uint16 ry = (rx + (rx >> 1)) >> 1;
  for(int i = 0x7c0; rx && i >= 0; i -= 0x40) {
    EnemyData *E = gEnemyData(i);
    if (E->invincibility_timer || !E->enemy_ptr || E->enemy_ptr == addr_kEnemyDef_DAFF)
      continue;
    EnemyDef *ED = get_EnemyDef_A2(E->enemy_ptr);
    if ((get_Vulnerability(ED->vulnerability_ptr ? ED->vulnerability_ptr : addr_stru_B4EC1C)->power_bomb & 0x7F) == 0)
      continue;
    if (abs16(power_bomb_explosion_x_pos - E->x_pos) < rx && abs16(power_bomb_explosion_y_pos - E->y_pos) < ry) {
      cur_enemy_index = i;
      uint16 func = ED->powerbomb_reaction ? ED->powerbomb_reaction : FUNC16(Enemy_NormalPowerBombAI_A0);
      CallEnemyAi(E->bank << 16 | func);
      E->properties |= 0x800;
    }
  }
}

void EnemyDeathAnimation(uint16 k, uint16 a) {  // 0xA0A3AF
  EnemyData *E = gEnemyData(cur_enemy_index);
  if (E->ai_handler_bits == 1)
    grapple_beam_function = FUNC16(GrappleBeam_Func2);
  if (!sign16(a - 5))
    a = 0;
//  varE20 = a;
  SpawnEprojWithGfx(a, cur_enemy_index, addr_kEproj_EnemyDeathExplosion);
  uint16 r18 = E->properties & 0x4000;
  memset(E, 0, 64);
  if (r18) {
    E->enemy_ptr = addr_kEnemyDef_DAFF;
    E->bank = 0xa3;
  }
  num_enemies_killed_in_room++;
}

void RinkasDeathAnimation(uint16 a) {  // 0xA0A410
  EnemyData *E = gEnemyData(cur_enemy_index);
  if (!sign16(a - 3))
    a = 0;
//  varE20 = a;
  SpawnEprojWithGfx(a, cur_enemy_index, addr_kEproj_EnemyDeathExplosion);
  uint16 r18 = E->properties & 0x4000;
  memset(E, 0, 64);
  if (r18) {
    E->enemy_ptr = addr_kEnemyDef_DAFF;
    E->bank = 0xa3;
  }
}

uint16 SuitDamageDivision(uint16 a) {  // 0xA0A45E
  if ((equipped_items & 0x20) != 0)
    return a >> 2;
  if (equipped_items & 1)
    return a >> 1;
  return a;
}

void NormalEnemyTouchAi(void) {  // 0xA0A477
  NormalEnemyTouchAiSkipDeathAnim();
  if (!gEnemyData(cur_enemy_index)->health) {
    gEnemySpawnData(cur_enemy_index)->cause_of_death = 6;
    EnemyDeathAnimation(cur_enemy_index, 1);
  }
}

void NormalEnemyTouchAiSkipDeathAnim_CurEnemy(void) {  // 0xA0A497
  NormalEnemyTouchAiSkipDeathAnim();
}

void NormalEnemyTouchAiSkipDeathAnim(void) {  // 0xA0A4A1
  EnemyData *E = gEnemyData(cur_enemy_index);
  EnemyDef *ED = get_EnemyDef_A2(E->enemy_ptr);
  if (samus_contact_damage_index == 0) {
    Samus_DealDamage(SuitDamageDivision(ED->damage));
    samus_invincibility_timer = 96;
    samus_knockback_timer = 5;
    knockback_x_dir = (int16)(samus_x_pos - E->x_pos) >= 0;
    return;
  }
  uint16 r20 = samus_contact_damage_index + 15;
  uint16 r22;
  if (samus_contact_damage_index == 1) {
    r22 = 500;
  } else if (samus_contact_damage_index == 2) {
    r22 = 300;
  } else if (samus_contact_damage_index == 3) {
    r22 = 2000;
  } else {
    ++r20;
    if (samus_contact_damage_index == 4)
      CallSomeSamusCode(4);
    r22 = 200;
  }
  uint16 vp = ED->vulnerability_ptr ? ED->vulnerability_ptr : addr_stru_B4EC1C;
  last_enemy_power = *(uint16 *)&get_Vulnerability(r20 + vp)->power;
  uint16 varE32 = last_enemy_power & 0x7F;
  if ((last_enemy_power & 0x7F) != 0) {
    uint16 dmg = (r22 >> 1) * varE32;
    if (dmg) {
      E->flash_timer = ED->hurt_ai_time ? ED->hurt_ai_time : 4;
      E->ai_handler_bits |= kEnemyAiBits_Hurt;
      samus_invincibility_timer = 0;
      samus_knockback_timer = 0;
      E->health = (int16)(E->health - dmg) < 0 ? 0 : (int16)(E->health - dmg);
      QueueSfx2_Max1(0xB);
    }
  }
}

void NormalEnemyPowerBombAi(void) {  // 0xA0A597
  NormalEnemyPowerBombAiSkipDeathAnim();
  if (!gEnemyData(cur_enemy_index)->health) {
    gEnemySpawnData(cur_enemy_index)->cause_of_death = 3;
    EnemyDeathAnimation(cur_enemy_index, 0);
  }
}

void NormalEnemyPowerBombAiSkipDeathAnim_CurEnemy(void) {  // 0xA0A5B7
  NormalEnemyPowerBombAiSkipDeathAnim();
}

void NormalEnemyPowerBombAiSkipDeathAnim(void) {  // 0xA0A5C1
  EnemyDef *EnemyDef_A2;
  int16 hurt_ai_time;

  EnemyData *v0 = gEnemyData(cur_enemy_index);
  uint16 vulnerability_ptr = get_EnemyDef_A2(v0->enemy_ptr)->vulnerability_ptr;
  if (!vulnerability_ptr)
    vulnerability_ptr = -5092;
  uint8 power_bomb = get_Vulnerability(vulnerability_ptr)->power_bomb;
  if (power_bomb != 255) {
    uint16 varE32 = power_bomb & 0x7F;
    if ((power_bomb & 0x7F) != 0) {
      uint32 ttt = 100 * varE32;
      if (ttt) {
        gEnemyData(cur_enemy_index)->invincibility_timer = 48;
        EnemyData *j = gEnemyData(cur_enemy_index);
        EnemyDef_A2 = get_EnemyDef_A2(j->enemy_ptr);
        hurt_ai_time = EnemyDef_A2->hurt_ai_time;
        if (!EnemyDef_A2->hurt_ai_time)
          hurt_ai_time = 4;
        EnemyData *v6 = gEnemyData(cur_enemy_index);
        v6->flash_timer = hurt_ai_time + 8;
        v6->ai_handler_bits |= 2;
        uint16 health = v6->health;
        bool v9 = health < ttt;
        uint16 v8 = health - ttt;
        v9 = !v9;
        if (!v8 || !v9)
          v8 = 0;
        v6->health = v8;
      }
    }
  }
}

void NormalEnemyShotAi(void) {  // 0xA0A63D
  uint16 varE2E = NormalEnemyShotAiSkipDeathAnim();
  EnemyData *E = gEnemyData(cur_enemy_index);
  if (varE2E) {
    CreateSpriteAtPos(E->x_pos, E->y_pos, 55, 0);
  }
  if (!E->health) {
    uint16 j = HIBYTE(projectile_type[collision_detection_index]) & 0xF;
    gEnemySpawnData(cur_enemy_index)->cause_of_death = j;
    uint16 death_anim = 2;
    if (j == 2) {
      EnemyDef *ED = get_EnemyDef_A2(E->enemy_ptr);
      if (!sign16(ED->death_anim - 3))
        death_anim = ED->death_anim;
    } else {
      death_anim = get_EnemyDef_A2(E->enemy_ptr)->death_anim;
    }
    EnemyDeathAnimation(E->enemy_ptr, death_anim);
  }
}

void NormalEnemyShotAiSkipDeathAnim_CurEnemy(void) {  // 0xA0A6A7
  NormalEnemyShotAiSkipDeathAnim();
}

void EnemyFunc_A6B4_UsedBySporeSpawn(void) {  // 0xA0A6B4
  uint16 varE2E = NormalEnemyShotAiSkipDeathAnim();
  if (varE2E) {
    EnemyData *ED = gEnemyData(cur_enemy_index);
    CreateSpriteAtPos(ED->x_pos, ED->y_pos, 55, 0);
  }
}

uint16 NormalEnemyShotAiSkipDeathAnim(void) {  // 0xA0A6DE
  uint16 varE2E = 0;
  int16 v5;
  int16 v6;
  int16 v9;
  EnemyDef *EnemyDef_A2;
  int16 hurt_ai_time;
  int16 v20;
  uint16 varE32;

  int v0 = collision_detection_index;
  uint16 pd = projectile_damage[v0];
  uint16 r18 = projectile_type[v0];
  EnemyData *j = gEnemyData(cur_enemy_index);
  uint16 vulnerability_ptr = get_EnemyDef_A2(j->enemy_ptr)->vulnerability_ptr;
  if (!vulnerability_ptr)
    vulnerability_ptr = addr_stru_B4EC1C;
  uint16 r20 = vulnerability_ptr;
  if ((r18 & 0xF00) != 0) {
    v5 = r18 & 0xF00;
    if ((r18 & 0xF00) == 256 || v5 == 512) {
      v6 = (r18 & 0xF00) >> 8;
      varE32 = get_Vulnerability(r20 + v6)->plasma_ice_wave & 0x7F;
    } else if (v5 == 1280) {
      varE32 = get_Vulnerability(r20)->bomb & 0x7F;
    } else {
      if (v5 != 768)
        goto LABEL_18;
      varE32 = get_Vulnerability(r20)->power_bomb & 0x7F;
    }
LABEL_9:;
    uint32 ttt = (uint32)(pd >> 1) * (uint32)varE32;
    uint16 r42 = ttt;
    if (r42) {
      pd = r42;
      EnemyData *v10 = gEnemyData(cur_enemy_index);
      EnemyDef_A2 = get_EnemyDef_A2(v10->enemy_ptr);
      hurt_ai_time = EnemyDef_A2->hurt_ai_time;
      if (!EnemyDef_A2->hurt_ai_time)
        hurt_ai_time = 4;
      EnemyData *v13 = gEnemyData(cur_enemy_index);
      v13->flash_timer = hurt_ai_time + 8;
      v13->ai_handler_bits |= 2;
      if (!v13->frozen_timer) {
        uint16 hurt_sfx = get_EnemyDef_A2(v13->enemy_ptr)->hurt_sfx;
        if (hurt_sfx)
          QueueSfx2_Max3(hurt_sfx);
        ++varE2E;
      }
      uint16 v15 = cur_enemy_index;
      if ((projectile_type[collision_detection_index] & 8) != 0)
        gEnemyData(cur_enemy_index)->invincibility_timer = 16;
      EnemyData *v16 = gEnemyData(v15);
      uint16 health = v16->health;
      bool v19 = health < pd;
      uint16 v18 = health - pd;
      v19 = !v19;
      if (!v18 || !v19) {
        if ((projectile_type[collision_detection_index] & 2) != 0
            && (last_enemy_power & 0xF0) != 128
            && !v16->frozen_timer) {
          v20 = 400;
          if (area_index == 2)
            v20 = 300;
          v16->frozen_timer = v20;
          v16->ai_handler_bits |= 4;
          v16->invincibility_timer = 10;
          QueueSfx3_Max3(0xA);
          return varE2E;
        }
        v18 = 0;
      }
      v16->health = v18;
      return varE2E;
    }
LABEL_18:;
    int v7 = collision_detection_index;
    projectile_dir[v7] |= 0x10;
    CreateSpriteAtPos(projectile_x_pos[v7], projectile_y_pos[v7], 6, 0);
    QueueSfx1_Max3(0x3D);
    return varE2E;
  }
  last_enemy_power = get_Vulnerability(r20 + (r18 & 0xF))->power;
  varE32 = last_enemy_power & 0x7F;
  if (last_enemy_power != 255) {
    if ((r18 & 0x10) != 0) {
      uint8 charged_beam = get_Vulnerability(r20)->charged_beam;
      if (charged_beam == 255)
        goto LABEL_18;
      uint16 v4 = charged_beam & 0xF;
      if (!v4)
        goto LABEL_18;
      varE32 = v4;
    }
    goto LABEL_9;
  }
  EnemyData *v8 = gEnemyData(cur_enemy_index);
  if (!v8->frozen_timer)
    QueueSfx3_Max3(0xA);
  v9 = 400;
  if (area_index == 2)
    v9 = 300;
  v8->frozen_timer = v9;
  v8->ai_handler_bits |= 4;
  v8->invincibility_timer = 10;
  return varE2E;
}

void CreateDudShot(void) {  // 0xA0A8BC
  int v0 = collision_detection_index;
  CreateSpriteAtPos(projectile_x_pos[v0], projectile_y_pos[v0], 6, 0);
  QueueSfx1_Max3(0x3D);
  projectile_dir[collision_detection_index] |= 0x10;
}

static uint8 ClearCarry_13(EnemyBlockCollInfo *ebci) {  // 0xA0C2BC
  return 0;
}

static uint8 SetCarry_4(EnemyBlockCollInfo *ebci) {  // 0xA0C2BE
  return 1;
}

static uint8 EnemyBlockCollReact_Spike(EnemyBlockCollInfo *ebci) {  // 0xA0C2C0
  uint16 v0 = g_off_A0C2DA[BTS[cur_block_index] & 0x7F];
  if (!v0)
    return 1;
  SpawnPLM(v0);
  return 0;
}

static uint8 EnemyBlockCollHorizReact_Slope(EnemyBlockCollInfo *ebci) {  // 0xA0C2FA
  if ((BTS[cur_block_index] & 0x1F) >= 5) {
    current_slope_bts = BTS[cur_block_index];
    return EnemyBlockCollHorizReact_Slope_NonSquare(ebci);
  } else {
    return EnemyBlockCollHorizReact_Slope_Square(ebci, cur_block_index, BTS[cur_block_index] & 0x1F);
  }
}

static uint8 EnemyBlockCollVertReact_Slope(EnemyBlockCollInfo *ebci) {  // 0xA0C319
  if ((BTS[cur_block_index] & 0x1F) >= 5)
    return EnemyBlockCollVertReact_Slope_NonSquare(ebci);
  else
    return EnemyBlockCollVertReact_Slope_Square(ebci, BTS[cur_block_index] & 0x1F, cur_block_index);
}

static const uint8 byte_A0C435[20] = {  // 0xA0C32E
     0,    1, 0x82, 0x83,
     0, 0x81,    2, 0x83,
     0,    1,    2, 0x83,
     0, 0x81, 0x82, 0x83,
  0x80, 0x81, 0x82, 0x83,
};

static uint8 EnemyBlockCollHorizReact_Slope_Square(EnemyBlockCollInfo *ebci, uint16 k, uint16 a) {
  EnemyData *E = gEnemyData(cur_enemy_index);

  uint16 temp_collision_DD4 = 4 * a;
  uint16 temp_collision_DD6 = BTS[k] >> 6;
  uint16 i = 4 * a + (temp_collision_DD6 ^ ((ebci->ebci_r26 & 8) >> 3));
  if (!ebci->ebci_r28) {
    if (((LOBYTE(E->y_height) + LOBYTE(E->y_pos) - 1) & 8) == 0)
      return CHECK_locret_A0C434(i) < 0;
    goto LABEL_7;
  }
  if (ebci->ebci_r28 != ebci->ebci_r30 || ((E->y_pos - E->y_height) & 8) == 0) {
LABEL_7:
    if (CHECK_locret_A0C434(i) < 0)
      return 1;
  }
  return CHECK_locret_A0C434(i ^ 2) < 0;
}

static uint8 EnemyBlockCollVertReact_Slope_Square(EnemyBlockCollInfo *ebci, uint16 a, uint16 k) {  // 0xA0C3B2
  EnemyData *E = gEnemyData(cur_enemy_index);
  uint16 temp_collision_DD4 = 4 * a;
  uint16 temp_collision_DD6 = BTS[k] >> 6;
  uint16 i = 4 * a + (temp_collision_DD6 ^ ((ebci->ebci_r26 & 8) >> 2));
  if (!ebci->ebci_r28) {
    if (((LOBYTE(E->x_width) + LOBYTE(E->x_pos) - 1) & 8) == 0)
      return CHECK_locret_A0C434(i) < 0;
    goto LABEL_7;
  }
  if (ebci->ebci_r28 != ebci->ebci_r30 || ((E->x_pos - E->x_width) & 8) == 0) {
LABEL_7:
    if (CHECK_locret_A0C434(i) < 0)
      return 1;
  }
  return CHECK_locret_A0C434(i ^ 1) < 0;
}

static uint8 EnemyBlockCollHorizReact_Slope_NonSquare(EnemyBlockCollInfo *ebci) {  // 0xA0C449
  if ((ebci->ebci_r32 & 0x8000) == 0)
    return (ebci->ebci_r32 & 0x4000) != 0;
  int i = 2 * (current_slope_bts & 0x1F);
  if (ebci->ebci_r18_r20 >= 0) {
    ebci->ebci_r18_r20 = (ebci->ebci_r18_r20 >> 8) * g_word_A0C49F[i + 1];
  } else {
    ebci->ebci_r18_r20 = -(-(int16)(ebci->ebci_r18_r20 >> 8) * g_word_A0C49F[i + 1]);
  }
  return 0;
}

static uint8 EnemyBlockCollVertReact_Slope_NonSquare(EnemyBlockCollInfo *ebci) {  // 0xA0C51F
  int16 v3;
  int16 v5;
  int16 v6;
  uint16 v7;
  int16 v8;
  int16 v11;
  int16 v12;
  int16 x_pos;
  uint16 v14;
  int16 v15;
  uint16 mod = SnesModulus(cur_block_index, room_width_in_blocks);
  EnemyData *E = gEnemyData(cur_enemy_index);
  uint16 v1 = cur_block_index;

  if (ebci->ebci_r18_r20 < 0) {
    v11 = E->x_pos >> 4;
    if (v11 != mod)
      return 0;
    uint16 temp_collision_DD4 = (ebci->ebci_r24 - E->y_height) & 0xF ^ 0xF;
    uint16 temp_collision_DD6 = 16 * (BTS[v1] & 0x1F);
    v12 = BTS[v1] << 8;
    if (v12 < 0
        && ((v12 & 0x4000) != 0 ? (x_pos = E->x_pos ^ 0xF) : (x_pos = E->x_pos),
        (v14 = temp_collision_DD6 + (x_pos & 0xF),
        v15 = (kAlignYPos_Tab0[v14] & 0x1F) - temp_collision_DD4 - 1, v15 <= 0))) {
      E->y_pos = ebci->ebci_r24 - v15;
      E->y_subpos = 0;
      return 1;
    } else {
      return 0;
    }
  } else {
    v3 = E->x_pos >> 4;
    if (v3 != mod)
      return 0;
    uint16 temp_collision_DD4 = (E->y_height + ebci->ebci_r24 - 1) & 0xF;
    uint16 temp_collision_DD6 = 16 * (BTS[v1] & 0x1F);
    v5 = BTS[v1] << 8;
    if (v5 >= 0
      && ((v5 & 0x4000) != 0 ? (v6 = E->x_pos ^ 0xF) : (v6 = E->x_pos),
      (v7 = temp_collision_DD6 + (v6 & 0xF),
      v8 = (kAlignYPos_Tab0[v7] & 0x1F) - temp_collision_DD4 - 1,
      (kAlignYPos_Tab0[v7] & 0x1F) - temp_collision_DD4 == 1) || v8 < 0)) {
      E->y_pos = ebci->ebci_r24 + v8;
      E->y_subpos = -1;
      return 1;
    } else {
      return 0;
    }
  }
}

static uint8 EnemyBlockCollReact_HorizExt(EnemyBlockCollInfo *ebci) {  // 0xA0C619
  uint8 t = BTS[cur_block_index];
  if (t) {
    cur_block_index += (int8)t;
    return 0xff;
  }
  return 0;
}

static uint8 EnemyBlockCollReact_VertExt(EnemyBlockCollInfo *ebci) {  // 0xA0C64F
  if (BTS[cur_block_index]) {
    cur_block_index += (int8)BTS[cur_block_index] * room_width_in_blocks;
    return 0xff;
  }
  return 0;
}

uint8 Enemy_MoveRight_SlopesAsWalls(uint16 k, int32 amt) {  // 0xA0C69D
  return Enemy_MoveRight_IgnoreSlopes_Inner(k, amt, 0x4000);
}

uint8 Enemy_MoveRight_ProcessSlopes(uint16 k, int32 amt) {  // 0xA0C6A4
  return Enemy_MoveRight_IgnoreSlopes_Inner(k, amt, 0x8000);
}

uint8 Enemy_MoveRight_IgnoreSlopes(uint16 k, int32 amt) {  // 0xA0C6AB
  return Enemy_MoveRight_IgnoreSlopes_Inner(k, amt, 0);
}

// amount32 is r20:r18
static uint8 Enemy_MoveRight_IgnoreSlopes_Inner(uint16 k, int32 amount32, uint16 r32) {  // 0xA0C6AD
  if (!amount32)
    return 0;
  EnemyData *E = gEnemyData(k);
  uint16 r28 = (uint16)(E->y_height + E->y_pos - 1 - ((E->y_pos - E->y_height) & 0xFFF0)) >> 4;
  uint16 r30 = r28;
  uint16 prod = Mult8x8((uint16)(E->y_pos - E->y_height) >> 4, room_width_in_blocks);
  uint32 new_pos = amount32 + __PAIR32__(E->x_pos, E->x_subpos);
  uint16 v5;
  uint16 r24 = new_pos >> 16;  // read by EnemyBlockCollVertReact_Slope_NonSquare
  if (sign32(amount32))
    v5 = (new_pos >> 16) - E->x_width;
  else
    v5 = E->x_width + (new_pos >> 16) - 1;
  EnemyBlockCollInfo ebci = { .ebci_r18_r20 = amount32, .ebci_r24 = r24, .ebci_r26 = v5, .ebci_r28 = r28, .ebci_r30 = r30, .ebci_r32 = r32};
  uint16 v6 = 2 * (prod + (v5 >> 4));
  while (!(EnemyBlockCollReact_Horiz(&ebci, v6) & 1)) {
    v6 += room_width_in_blocks * 2;
    if ((--ebci.ebci_r28 & 0x8000) != 0) {
      AddToHiLo(&E->x_pos, &E->x_subpos, ebci.ebci_r18_r20);
      return 0;
    }
  }
  if (sign32(amount32)) {
    uint16 v12 = E->x_width + 1 + (ebci.ebci_r26 | 0xF);
    if (v12 <= E->x_pos)
      E->x_pos = v12;
    E->x_subpos = 0;
    return 1;
  } else {
    uint16 v10 = (ebci.ebci_r26 & 0xFFF0) - E->x_width;
    if (v10 >= E->x_pos)
      E->x_pos = v10;
    E->x_subpos = -1;
    return 1;
  }
}

uint8 Enemy_MoveDown(uint16 k, int32 amount32) {  // 0xA0C786
  int16 v6;
  uint16 v5;
  if (!amount32)
    return 0;
  EnemyData *E = gEnemyData(k);
  uint16 r28 = (uint16)(E->x_width + E->x_pos - 1 - ((E->x_pos - E->x_width) & 0xFFF0)) >> 4;
  uint16 r30 = r28;
  uint32 new_pos = amount32 + __PAIR32__(E->y_pos, E->y_subpos);
  uint16 r24 = new_pos >> 16;  // read by EnemyBlockCollVertReact_Slope_NonSquare
  if (sign32(amount32))
    v5 = (new_pos >> 16) - E->y_height;
  else
    v5 = E->y_height + (new_pos >> 16) - 1;
  uint16 prod = Mult8x8(v5 >> 4, room_width_in_blocks);
  v6 = (uint16)(E->x_pos - E->x_width) >> 4;
  EnemyBlockCollInfo ebci = { .ebci_r18_r20 = amount32, .ebci_r24 = r24, .ebci_r26 = v5, .ebci_r28 = r28, .ebci_r30 = r30, .ebci_r32 = 0 };
  for (int i = 2 * (prod + v6); !(EnemyBlockCollReact_Vert(&ebci, i) & 1); i += 2) {
    if ((--ebci.ebci_r28 & 0x8000) != 0) {
      E->y_subpos = new_pos, E->y_pos = new_pos >> 16;
      return 0;
    }
  }
  if (sign32(amount32)) {
    uint16 v13 = E->y_height + 1 + (ebci.ebci_r26 | 0xF);
    if (v13 <= E->y_pos)
      E->y_pos = v13;
    E->y_subpos = 0;
    return 1;
  } else {
    uint16 v10 = (ebci.ebci_r26 & 0xFFF0) - E->y_height;
    if (v10 >= E->y_pos)
      E->y_pos = v10;
    E->y_subpos = -1;
    return 1;
  }
}

static Func_EnemyBlockCollInfo_U8 *const off_A0C859[16] = {  // 0xA0C845
  ClearCarry_13,
  EnemyBlockCollHorizReact_Slope,
  ClearCarry_13,
  ClearCarry_13,
  ClearCarry_13,
  EnemyBlockCollReact_HorizExt,
  ClearCarry_13,
  ClearCarry_13,
  SetCarry_4,
  SetCarry_4,
  EnemyBlockCollReact_Spike,
  SetCarry_4,
  SetCarry_4,
  EnemyBlockCollReact_VertExt,
  SetCarry_4,
  SetCarry_4,
};

static uint8 EnemyBlockCollReact_Horiz(EnemyBlockCollInfo *ebci, uint16 k) {
  cur_block_index = k >> 1;
  uint8 rv = 0;
  do {
    rv = off_A0C859[(level_data[cur_block_index] & 0xf000) >> 12](ebci);
  } while (rv & 0x80);
  return rv;
}

static Func_EnemyBlockCollInfo_U8 *const off_A0C88D[16] = {  // 0xA0C879
  ClearCarry_13,
  EnemyBlockCollVertReact_Slope,
  ClearCarry_13,
  ClearCarry_13,
  ClearCarry_13,
  EnemyBlockCollReact_HorizExt,
  ClearCarry_13,
  ClearCarry_13,
  SetCarry_4,
  SetCarry_4,
  EnemyBlockCollReact_Spike,
  SetCarry_4,
  SetCarry_4,
  EnemyBlockCollReact_VertExt,
  SetCarry_4,
  SetCarry_4,
};

static uint8 EnemyBlockCollReact_Vert(EnemyBlockCollInfo *ebci, uint16 k) {
  cur_block_index = k >> 1;
  uint8 rv = 0;
  do {
    rv = off_A0C88D[(level_data[cur_block_index] & 0xf000) >> 12](ebci);
  } while (rv & 0x80);
  return rv;
}

void CalculateBlockContainingPixelPos(uint16 xpos, uint16 ypos) {
  uint16 prod = Mult8x8(ypos >> 4, room_width_in_blocks);
  cur_block_index = prod + (xpos >> 4);
}

uint8 EnemyFunc_C8AD(uint16 k) {  // 0xA0C8AD
  uint8 result = 0;

  EnemyData *E = gEnemyData(k);
  CalculateBlockContainingPixelPos(E->x_pos, E->y_pos + E->y_height - 1);
  if ((level_data[cur_block_index] & 0xF000) == 4096 && (BTS[cur_block_index] & 0x1F) >= 5) {
    result = 1;
    uint16 temp_collision_DD4 = (E->y_height + E->y_pos - 1) & 0xF;
    uint16 temp_collision_DD6 = 16 * (BTS[cur_block_index] & 0x1F);
    if ((BTS[cur_block_index] & 0x80) == 0) {
      uint16 j = (BTS[cur_block_index] & 0x40) != 0 ? E->x_pos ^ 0xF : E->x_pos;
      int16 v4 = (kAlignYPos_Tab0[temp_collision_DD6 + (j & 0xF)] & 0x1F) - temp_collision_DD4 - 1;
      if (v4 < 0)
        E->y_pos += v4;
    }
  }
  CalculateBlockContainingPixelPos(E->x_pos, E->y_pos - E->y_height);
  if ((level_data[cur_block_index] & 0xF000) == 4096 && (BTS[cur_block_index] & 0x1F) >= 5) {
    result = 1;
    uint16 temp_collision_DD4 = (E->y_pos - E->y_height) & 0xF ^ 0xF;
    uint16 temp_collision_DD6 = 16 * (BTS[cur_block_index] & 0x1F);
    if (BTS[cur_block_index] & 0x80) {
      uint16 j = (BTS[cur_block_index] & 0x40) != 0 ? E->x_pos ^ 0xF : E->x_pos;
      int16 v7 = (kAlignYPos_Tab0[temp_collision_DD6 + (j & 0xF)] & 0x1F) - temp_collision_DD4 - 1;
      if (v7 < 0)
        E->y_pos -= v7;
    }
  }
  return result;
}
