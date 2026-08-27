// Enemies
#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"
#include "samus_status.h"
#include "enemy_ai_canon.h"

enum EnemyGrappleReact {
  kEnemyGrappleReact_NoInteract = 0,
  kEnemyGrappleReact_SamusLatchesOn = 1,
  kEnemyGrappleReact_KillEnemy = 2,
  kEnemyGrappleReact_CancelBeam = 3,
  kEnemyGrappleReact_SamusLatchesNoInvinc = 4,
  kEnemyGrappleReact_SamusLatchesParalyze = 5,
  kEnemyGrappleReact_HurtSamus = 6,
};

// EnemyDef stores bank-local grapple_ai offsets; match A0 copies only.
static const struct {
  uint16 fn;
  uint16 reaction;
} kEnemyGrappleReactTable[] = {
  { FUNC16(Enemy_GrappleReact_NoInteract_A0), kEnemyGrappleReact_NoInteract },
  { FUNC16(Enemy_GrappleReact_SamusLatchesOn_A0), kEnemyGrappleReact_SamusLatchesOn },
  { FUNC16(Enemy_GrappleReact_KillEnemy_A0), kEnemyGrappleReact_KillEnemy },
  { FUNC16(Enemy_GrappleReact_CancelBeam_A0), kEnemyGrappleReact_CancelBeam },
  { FUNC16(Enemy_GrappleReact_SamusLatchesNoInvinc_A0), kEnemyGrappleReact_SamusLatchesNoInvinc },
  { FUNC16(Enemy_GrappleReact_SamusLatchesParalyze_A0), kEnemyGrappleReact_SamusLatchesParalyze },
  { FUNC16(Enemy_GrappleReact_HurtSamus_A0), kEnemyGrappleReact_HurtSamus },
};

static uint16 LookupEnemyGrappleReact(uint16 grapple_ai) {
  for (int i = 0; i < arraysize(kEnemyGrappleReactTable); i++) {
    if (grapple_ai == kEnemyGrappleReactTable[i].fn)
      return kEnemyGrappleReactTable[i].reaction;
  }
  return kEnemyGrappleReact_NoInteract;
}

PairU16 GrappleBeam_CollDetect_Enemy(void) {  // 0xA09E9A
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
  E->ai_handler_bits = kEnemyAiBits_Grapple;
  uint16 enemy_ptr = E->enemy_ptr;
  uint16 reaction = LookupEnemyGrappleReact(get_EnemyDef_A2(E->enemy_ptr)->grapple_ai);
  if (reaction == kEnemyGrappleReact_SamusLatchesOn ||
      reaction == kEnemyGrappleReact_SamusLatchesNoInvinc ||
      reaction == kEnemyGrappleReact_SamusLatchesParalyze) {
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
    E->ai_handler_bits = kEnemyAiBits_Frozen;
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
  gEnemyData(cur_enemy_index)->ai_handler_bits = kEnemyAiBits_Frozen;
}

void SamusLatchesOnWithGrappleNoInvinc(void) {  // 0xA09FE9
  EnemyData *E = gEnemyData(cur_enemy_index);
  if (E->frozen_timer) {
    E->ai_handler_bits = kEnemyAiBits_Frozen;
  } else {
    RunEnemyAiFn(GetEnemyDefAiFns(E->enemy_ptr)->main_ai);
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
  E->extra_properties |= kEnemyExtraProps_DisableEnemyAI;
}

void SamusHurtFromGrapple(void) {  // 0xA0A070
  gEnemyData(cur_enemy_index)->ai_handler_bits = kEnemyAiBits_Frozen;
}

void EnemyCollisionHandler(void) {  // 0xA09758
  if ((gEnemyData(cur_enemy_index)->extra_properties & kEnemyExtraProps_MultiHitbox) != 0) {
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

static void CallHitboxTouch(uint32 ea) {
  EnemyAiFromAddr(ea)();
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
      RunEnemyAiFn(GetEnemyDefAiFns(E->enemy_ptr)->touch_ai);
  }
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
