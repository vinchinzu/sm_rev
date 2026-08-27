// Enemies
#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "enemy_types.h"
#include "torizo_config.h"

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

void EprojCollHandler_Multibox(void) {  // 0xA09B7F
  EnemyData *E = gEnemyData(cur_enemy_index);
  enemy_processing_stage = 3;
  if (!projectile_counter || !E->spritemap_pointer || E->spritemap_pointer == addr_kExtendedSpritemap_Nothing_A0)
    return;
  uint16 shot_ai = get_EnemyDef_A2(E->enemy_ptr)->shot_ai;
  if (shot_ai == FUNC16(nullsub_170) || shot_ai == FUNC16(nullsub_169))
    return;
  if ((E->properties & kEnemyProps_Intangible) != 0 || E->invincibility_timer || E->enemy_ptr == addr_kEnemyDef_DAFF)
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
          if ((E->properties & kEnemyProps_BlockPlasmaBeam) != 0 || (projectile_type[pidx] & 8) == 0)
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
  if (!E->spritemap_pointer || (E->properties & kEnemyProps_Intangible) != 0 || E->invincibility_timer)
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

void EprojCollHandler(void) {  // 0xA0A143
  EnemyData *E = gEnemyData(cur_enemy_index);
  enemy_processing_stage = 7;
  if (!projectile_counter)
    return;
  if (!E->spritemap_pointer || E->spritemap_pointer == addr_kSpritemap_Nothing_A0 || (E->properties & kEnemyProps_Intangible) != 0 ||
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
          if ((E->properties & kEnemyProps_BlockPlasmaBeam) != 0 || (projectile_type[pidx] & 8) == 0)
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
      E->properties |= kEnemyProps_ProcessedOffscreen;
    }
  }
}

void EnemyDeathAnimation(uint16 k, uint16 a) {  // 0xA0A3AF
  EnemyData *E = gEnemyData(cur_enemy_index);
  if (E->ai_handler_bits == kEnemyAiBits_Grapple)
    grapple_beam_function = FUNC16(GrappleBeam_Func2);
  if (!sign16(a - 5))
    a = 0;
//  varE20 = a;
  SpawnEprojWithGfx(a, cur_enemy_index, addr_kEproj_EnemyDeathExplosion);
  uint16 r18 = E->properties & kEnemyProps_RespawnIfKilled;
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
  uint16 r18 = E->properties & kEnemyProps_RespawnIfKilled;
  memset(E, 0, 64);
  if (r18) {
    E->enemy_ptr = addr_kEnemyDef_DAFF;
    E->bank = 0xa3;
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
        v6->ai_handler_bits |= kEnemyAiBits_Hurt;
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
      v13->ai_handler_bits |= kEnemyAiBits_Hurt;
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
          v16->ai_handler_bits |= kEnemyAiBits_Frozen;
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
  v8->ai_handler_bits |= kEnemyAiBits_Frozen;
  v8->invincibility_timer = 10;
  return varE2E;
}

void CreateDudShot(void) {  // 0xA0A8BC
  int v0 = collision_detection_index;
  CreateSpriteAtPos(projectile_x_pos[v0], projectile_y_pos[v0], 6, 0);
  QueueSfx1_Max3(0x3D);
  projectile_dir[collision_detection_index] |= 0x10;
}
