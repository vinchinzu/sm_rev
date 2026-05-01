#include "torizo_config.h"

#include "../third_party/cJSON.h"
#include "ida_types.h"
#include "util.h"
#include "variables.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

enum {
  kTorizoOpeningAttack_Vanilla = 0,
  kTorizoOpeningAttack_ChozoOrbs = 1,
  kTorizoChozoOrbPalette_Vanilla = -1,
  kTorizoChozoOrbPalette_Blue = -2,
  kTorizoChozoOrbBluePaletteIndex = 0xe00,
  kTorizoChozoOrbBluePaletteBase = 240,
  kTorizoBaseBombChozoOrbBurstCount = 3,
  kTorizoBaseGoldenChozoOrbBurstCount = 1,
  kTorizoDefaultMaxChozoOrbsPerBurst = 18,
  kTorizoMaxChozoOrbDamageMultiplier = 64,
  kTorizoMaxChozoOrbFreezeFrames = 600,
  kTorizoMaxChozoOrbSpawnJitterPixels = 64,
  kTorizoMaxEnemyProjectileDamage = 0xfff,
  kTorizoDefaultOpeningChozoOrbWaveIntervalFrames = 16,
  kTorizoMaxOpeningChozoOrbWaveCount = 64,
  kTorizoMaxOpeningChozoOrbWaveIntervalFrames = 255,
  kTorizoBaseFinishExplosionBodyBurstCount = 6,
  kTorizoBaseFinishExplosionFinalBurstCount = 1,
  kTorizoDefaultFinishExplosionFlashFrames = 40,
  kTorizoDefaultMaxFinishExplosionsPerBurst = 18,
  kTorizoMaxFinishExplosionFlashFrames = 1024,
  kTorizoMaxFinishExplosionJitterPixels = 64,
};

static const char *const kTorizoConfigPath = "sm_torizo.json";

static const uint16 kTorizoChozoOrbBluePalette[16] = {
  0x3800, 0x7fff, 0x7fe0, 0x7f20, 0x7e40, 0x7d80, 0x7cc0, 0x7c00,
  0x5c00, 0x4000, 0x2c00, 0x1800, 0x7fff, 0x7fe0, 0x7d80, 0,
};

static const TorizoConfig kTorizoConfigDefaults = {
  .chozo_orb_spawn_multiplier = 1,
  .chozo_orb_contact_damage_multiplier = 1,
  .chozo_orb_palette = kTorizoChozoOrbPalette_Vanilla,
  .chozo_orb_freeze_samus_frames = 0,
  .chozo_orb_spawn_jitter_pixels = 0,
  .bomb_chozo_orb_burst_count = -1,
  .golden_chozo_orb_burst_count = -1,
  .bomb_opening_attack = kTorizoOpeningAttack_Vanilla,
  .bomb_opening_chozo_orb_burst_count = -1,
  .bomb_opening_chozo_orb_wave_count = -1,
  .bomb_opening_chozo_orb_wave_interval_frames = kTorizoDefaultOpeningChozoOrbWaveIntervalFrames,
  .max_chozo_orbs_per_burst = kTorizoDefaultMaxChozoOrbsPerBurst,
  .finish_explosion_body_burst_multiplier = 1,
  .finish_explosion_body_burst_count = -1,
  .finish_explosion_final_burst_multiplier = 1,
  .finish_explosion_final_burst_count = -1,
  .finish_explosion_flash_frames = kTorizoDefaultFinishExplosionFlashFrames,
  .finish_explosion_position_jitter_pixels = 0,
  .max_finish_explosions_per_burst = kTorizoDefaultMaxFinishExplosionsPerBurst,
};

TorizoConfig g_torizo_config = {
  .chozo_orb_spawn_multiplier = 1,
  .chozo_orb_contact_damage_multiplier = 1,
  .chozo_orb_palette = kTorizoChozoOrbPalette_Vanilla,
  .chozo_orb_freeze_samus_frames = 0,
  .chozo_orb_spawn_jitter_pixels = 0,
  .bomb_chozo_orb_burst_count = -1,
  .golden_chozo_orb_burst_count = -1,
  .bomb_opening_attack = kTorizoOpeningAttack_Vanilla,
  .bomb_opening_chozo_orb_burst_count = -1,
  .bomb_opening_chozo_orb_wave_count = -1,
  .bomb_opening_chozo_orb_wave_interval_frames = kTorizoDefaultOpeningChozoOrbWaveIntervalFrames,
  .max_chozo_orbs_per_burst = kTorizoDefaultMaxChozoOrbsPerBurst,
  .finish_explosion_body_burst_multiplier = 1,
  .finish_explosion_body_burst_count = -1,
  .finish_explosion_final_burst_multiplier = 1,
  .finish_explosion_final_burst_count = -1,
  .finish_explosion_flash_frames = kTorizoDefaultFinishExplosionFlashFrames,
  .finish_explosion_position_jitter_pixels = 0,
  .max_finish_explosions_per_burst = kTorizoDefaultMaxFinishExplosionsPerBurst,
};

static time_t g_torizo_config_mtime;
static uint16 g_torizo_samus_freeze_timer;

static int TorizoConfig_ClampInt(int value, int min_value, int max_value) {
  if (value < min_value)
    return min_value;
  if (value > max_value)
    return max_value;
  return value;
}

static bool TorizoConfig_ReadInt(cJSON *obj, const char *key, int *out_value) {
  cJSON *field = cJSON_GetObjectItemCaseSensitive(obj, key);
  if (cJSON_IsNumber(field)) {
    *out_value = field->valueint;
    return true;
  }
  return false;
}

static bool TorizoConfig_ReadOpeningAttack(cJSON *obj, const char *key, int *out_value) {
  cJSON *field = cJSON_GetObjectItemCaseSensitive(obj, key);
  if (cJSON_IsNumber(field)) {
    *out_value = field->valueint;
    return true;
  }
  if (!cJSON_IsString(field) || !field->valuestring)
    return false;
  if (strcmp(field->valuestring, "chozo_orbs") == 0) {
    *out_value = kTorizoOpeningAttack_ChozoOrbs;
    return true;
  }
  if (strcmp(field->valuestring, "vanilla") == 0) {
    *out_value = kTorizoOpeningAttack_Vanilla;
    return true;
  }
  return false;
}

static bool TorizoConfig_ReadChozoOrbPalette(cJSON *obj, const char *key, int *out_value) {
  cJSON *field = cJSON_GetObjectItemCaseSensitive(obj, key);
  if (cJSON_IsNumber(field)) {
    *out_value = field->valueint;
    return true;
  }
  if (!cJSON_IsString(field) || !field->valuestring)
    return false;
  if (strcmp(field->valuestring, "blue") == 0) {
    *out_value = kTorizoChozoOrbPalette_Blue;
    return true;
  }
  if (strcmp(field->valuestring, "vanilla") == 0) {
    *out_value = kTorizoChozoOrbPalette_Vanilla;
    return true;
  }
  return false;
}

static void TorizoConfig_LoadChozoOrbConfig(cJSON *root) {
  cJSON *chozo_orbs = cJSON_GetObjectItemCaseSensitive(root, "chozo_orbs");
  if (!cJSON_IsObject(chozo_orbs))
    chozo_orbs = root;

  TorizoConfig_ReadInt(chozo_orbs, "spawn_multiplier", &g_torizo_config.chozo_orb_spawn_multiplier);
  TorizoConfig_ReadInt(chozo_orbs, "contact_damage_multiplier",
                       &g_torizo_config.chozo_orb_contact_damage_multiplier);
  TorizoConfig_ReadChozoOrbPalette(chozo_orbs, "palette", &g_torizo_config.chozo_orb_palette);
  TorizoConfig_ReadInt(chozo_orbs, "freeze_samus_frames",
                       &g_torizo_config.chozo_orb_freeze_samus_frames);
  TorizoConfig_ReadInt(chozo_orbs, "spawn_jitter_pixels",
                       &g_torizo_config.chozo_orb_spawn_jitter_pixels);
  TorizoConfig_ReadInt(chozo_orbs, "bomb_burst_count", &g_torizo_config.bomb_chozo_orb_burst_count);
  TorizoConfig_ReadInt(chozo_orbs, "golden_burst_count", &g_torizo_config.golden_chozo_orb_burst_count);
  TorizoConfig_ReadInt(chozo_orbs, "max_per_burst", &g_torizo_config.max_chozo_orbs_per_burst);

  TorizoConfig_ReadInt(root, "chozo_orb_spawn_multiplier", &g_torizo_config.chozo_orb_spawn_multiplier);
  TorizoConfig_ReadInt(root, "chozo_orb_contact_damage_multiplier",
                       &g_torizo_config.chozo_orb_contact_damage_multiplier);
  TorizoConfig_ReadChozoOrbPalette(root, "chozo_orb_palette", &g_torizo_config.chozo_orb_palette);
  TorizoConfig_ReadInt(root, "chozo_orb_freeze_samus_frames",
                       &g_torizo_config.chozo_orb_freeze_samus_frames);
  TorizoConfig_ReadInt(root, "chozo_orb_spawn_jitter_pixels",
                       &g_torizo_config.chozo_orb_spawn_jitter_pixels);
  TorizoConfig_ReadInt(root, "bomb_chozo_orb_burst_count", &g_torizo_config.bomb_chozo_orb_burst_count);
  TorizoConfig_ReadInt(root, "golden_chozo_orb_burst_count", &g_torizo_config.golden_chozo_orb_burst_count);
  TorizoConfig_ReadInt(root, "max_chozo_orbs_per_burst", &g_torizo_config.max_chozo_orbs_per_burst);
}

static void TorizoConfig_LoadAttackOrderConfig(cJSON *root) {
  cJSON *attack_order = cJSON_GetObjectItemCaseSensitive(root, "attack_order");
  if (!cJSON_IsObject(attack_order))
    attack_order = root;

  TorizoConfig_ReadOpeningAttack(attack_order, "bomb_opening_attack", &g_torizo_config.bomb_opening_attack);
  TorizoConfig_ReadInt(attack_order, "bomb_opening_chozo_orb_burst_count",
                       &g_torizo_config.bomb_opening_chozo_orb_burst_count);
  TorizoConfig_ReadInt(attack_order, "bomb_opening_chozo_orb_wave_count",
                       &g_torizo_config.bomb_opening_chozo_orb_wave_count);
  TorizoConfig_ReadInt(attack_order, "bomb_opening_chozo_orb_wave_interval_frames",
                       &g_torizo_config.bomb_opening_chozo_orb_wave_interval_frames);

  TorizoConfig_ReadOpeningAttack(root, "bomb_opening_attack", &g_torizo_config.bomb_opening_attack);
  TorizoConfig_ReadInt(root, "bomb_opening_chozo_orb_burst_count",
                       &g_torizo_config.bomb_opening_chozo_orb_burst_count);
  TorizoConfig_ReadInt(root, "bomb_opening_chozo_orb_wave_count",
                       &g_torizo_config.bomb_opening_chozo_orb_wave_count);
  TorizoConfig_ReadInt(root, "bomb_opening_chozo_orb_wave_interval_frames",
                       &g_torizo_config.bomb_opening_chozo_orb_wave_interval_frames);
}

static void TorizoConfig_LoadFinishExplosionConfig(cJSON *root) {
  cJSON *finish_explosion = cJSON_GetObjectItemCaseSensitive(root, "finish_explosion");
  if (!cJSON_IsObject(finish_explosion))
    finish_explosion = root;

  TorizoConfig_ReadInt(finish_explosion, "body_burst_multiplier",
                       &g_torizo_config.finish_explosion_body_burst_multiplier);
  TorizoConfig_ReadInt(finish_explosion, "body_burst_count",
                       &g_torizo_config.finish_explosion_body_burst_count);
  TorizoConfig_ReadInt(finish_explosion, "final_burst_multiplier",
                       &g_torizo_config.finish_explosion_final_burst_multiplier);
  TorizoConfig_ReadInt(finish_explosion, "final_burst_count",
                       &g_torizo_config.finish_explosion_final_burst_count);
  TorizoConfig_ReadInt(finish_explosion, "flash_frames",
                       &g_torizo_config.finish_explosion_flash_frames);
  TorizoConfig_ReadInt(finish_explosion, "position_jitter_pixels",
                       &g_torizo_config.finish_explosion_position_jitter_pixels);
  TorizoConfig_ReadInt(finish_explosion, "max_per_burst",
                       &g_torizo_config.max_finish_explosions_per_burst);

  TorizoConfig_ReadInt(root, "finish_explosion_body_burst_multiplier",
                       &g_torizo_config.finish_explosion_body_burst_multiplier);
  TorizoConfig_ReadInt(root, "finish_explosion_body_burst_count",
                       &g_torizo_config.finish_explosion_body_burst_count);
  TorizoConfig_ReadInt(root, "finish_explosion_final_burst_multiplier",
                       &g_torizo_config.finish_explosion_final_burst_multiplier);
  TorizoConfig_ReadInt(root, "finish_explosion_final_burst_count",
                       &g_torizo_config.finish_explosion_final_burst_count);
  TorizoConfig_ReadInt(root, "finish_explosion_flash_frames",
                       &g_torizo_config.finish_explosion_flash_frames);
  TorizoConfig_ReadInt(root, "finish_explosion_position_jitter_pixels",
                       &g_torizo_config.finish_explosion_position_jitter_pixels);
  TorizoConfig_ReadInt(root, "max_finish_explosions_per_burst",
                       &g_torizo_config.max_finish_explosions_per_burst);
}

static uint16 TorizoConfig_ChozoOrbBurstCount(int base_count, int override_count) {
  int max_count = TorizoConfig_ClampInt(g_torizo_config.max_chozo_orbs_per_burst, 0,
                                        kTorizoDefaultMaxChozoOrbsPerBurst);
  int count = override_count >= 0
      ? override_count
      : base_count * TorizoConfig_ClampInt(g_torizo_config.chozo_orb_spawn_multiplier, 0, max_count);
  return (uint16)TorizoConfig_ClampInt(count, 0, max_count);
}

static uint16 TorizoConfig_FinishExplosionBurstCount(int base_count, int multiplier, int override_count) {
  int max_count = TorizoConfig_ClampInt(g_torizo_config.max_finish_explosions_per_burst, 0,
                                        kTorizoDefaultMaxFinishExplosionsPerBurst);
  int count = override_count >= 0
      ? override_count
      : base_count * TorizoConfig_ClampInt(multiplier, 0, max_count);
  return (uint16)TorizoConfig_ClampInt(count, 0, max_count);
}

static bool TorizoConfig_ChozoOrbModsActive(void) {
  return g_torizo_config.chozo_orb_spawn_multiplier != 1 ||
         g_torizo_config.chozo_orb_contact_damage_multiplier != 1 ||
         g_torizo_config.chozo_orb_palette != kTorizoChozoOrbPalette_Vanilla ||
         g_torizo_config.chozo_orb_freeze_samus_frames != 0 ||
         g_torizo_config.chozo_orb_spawn_jitter_pixels != 0 ||
         g_torizo_config.bomb_chozo_orb_burst_count >= 0 ||
         g_torizo_config.golden_chozo_orb_burst_count >= 0 ||
         g_torizo_config.bomb_opening_chozo_orb_burst_count >= 0 ||
         g_torizo_config.max_chozo_orbs_per_burst != kTorizoDefaultMaxChozoOrbsPerBurst;
}

static bool TorizoConfig_AttackOrderModsActive(void) {
  return g_torizo_config.bomb_opening_attack != kTorizoOpeningAttack_Vanilla ||
         g_torizo_config.bomb_opening_chozo_orb_wave_count >= 0 ||
         g_torizo_config.bomb_opening_chozo_orb_wave_interval_frames !=
             kTorizoDefaultOpeningChozoOrbWaveIntervalFrames;
}

static bool TorizoConfig_FinishExplosionModsActive(void) {
  return g_torizo_config.finish_explosion_body_burst_multiplier != 1 ||
         g_torizo_config.finish_explosion_body_burst_count >= 0 ||
         g_torizo_config.finish_explosion_final_burst_multiplier != 1 ||
         g_torizo_config.finish_explosion_final_burst_count >= 0 ||
         g_torizo_config.finish_explosion_flash_frames != kTorizoDefaultFinishExplosionFlashFrames ||
         g_torizo_config.finish_explosion_position_jitter_pixels != 0 ||
         g_torizo_config.max_finish_explosions_per_burst != kTorizoDefaultMaxFinishExplosionsPerBurst;
}

void LoadTorizoConfig(void) {
  g_torizo_config = kTorizoConfigDefaults;
  struct stat st;

  size_t len = 0;
  char *data = (char *)ReadWholeFile(kTorizoConfigPath, &len);
  if (!data) {
    g_torizo_config_mtime = 0;
    g_torizo_samus_freeze_timer = 0;
    return;
  }

  cJSON *root = cJSON_ParseWithLength(data, len);
  free(data);
  if (!root) {
    if (stat(kTorizoConfigPath, &st) == 0)
      g_torizo_config_mtime = st.st_mtime;
    g_torizo_samus_freeze_timer = 0;
    printf("Failed to parse %s; using vanilla Torizo behavior.\n", kTorizoConfigPath);
    return;
  }

  TorizoConfig_LoadChozoOrbConfig(root);
  TorizoConfig_LoadAttackOrderConfig(root);
  TorizoConfig_LoadFinishExplosionConfig(root);
  cJSON_Delete(root);

  if (g_torizo_config.chozo_orb_freeze_samus_frames <= 0)
    g_torizo_samus_freeze_timer = 0;

  if (stat(kTorizoConfigPath, &st) == 0)
    g_torizo_config_mtime = st.st_mtime;

  if (TorizoModsActive()) {
    printf("[torizo] mods active: chozo_orb_multiplier=%d chozo_orb_damage=%dx orb_palette=%s "
           "orb_freeze=%u orb_jitter=%u bomb_orbs=%u golden_orbs=%u bomb_opening=%s bomb_opening_orbs=%u "
           "opening_waves=%d opening_interval=%u finish_body=%u finish_final=%u finish_flash=%u finish_jitter=%u\n",
           g_torizo_config.chozo_orb_spawn_multiplier,
           g_torizo_config.chozo_orb_contact_damage_multiplier,
           g_torizo_config.chozo_orb_palette == kTorizoChozoOrbPalette_Blue ? "blue" :
               g_torizo_config.chozo_orb_palette == kTorizoChozoOrbPalette_Vanilla ? "vanilla" : "custom",
           TorizoConfig_ClampInt(g_torizo_config.chozo_orb_freeze_samus_frames, 0,
                                 kTorizoMaxChozoOrbFreezeFrames),
           TorizoConfig_ChozoOrbSpawnJitterPixels(),
           TorizoConfig_BombChozoOrbBurstCount(),
           TorizoConfig_GoldenChozoOrbBurstCount(),
           TorizoConfig_BombOpeningAttackIsChozoOrbs() ? "chozo_orbs" : "vanilla",
           TorizoConfig_BombOpeningChozoOrbBurstCount(),
           g_torizo_config.bomb_opening_chozo_orb_wave_count,
           TorizoConfig_BombOpeningChozoOrbWaveIntervalFrames(),
           TorizoConfig_FinishExplosionBodyBurstCount(),
           TorizoConfig_FinishExplosionFinalBurstCount(),
           TorizoConfig_FinishExplosionFlashFrames(),
           TorizoConfig_FinishExplosionJitterPixels());
  }
}

void CheckTorizoConfigReload(void) {
  struct stat st;
  if (stat(kTorizoConfigPath, &st) == 0) {
    if (g_torizo_config_mtime == 0 || st.st_mtime > g_torizo_config_mtime) {
      printf("Reloading Torizo configuration...\n");
      LoadTorizoConfig();
    }
  } else if (g_torizo_config_mtime != 0) {
    printf("Reloading Torizo configuration...\n");
    LoadTorizoConfig();
  }
}

static uint16 TorizoConfig_ChozoOrbPaletteIndex(void) {
  if (g_torizo_config.chozo_orb_palette == kTorizoChozoOrbPalette_Blue)
    return kTorizoChozoOrbBluePaletteIndex;
  int palette = g_torizo_config.chozo_orb_palette;
  if (palette < 0)
    return 0xffff;
  if (palette <= 7)
    return (uint16)(palette << 9);
  return (uint16)(palette & 0x0e00);
}

static void TorizoConfig_ClearSamusMotionForFreeze(void) {
  input_to_pose_calc = 0;
  joypad1_input_samusfilter = 0;
  joypad1_newinput_samusfilter = 0;
  samus_x_extra_run_speed = 0;
  samus_x_extra_run_subspeed = 0;
  samus_x_base_speed = 0;
  samus_x_base_subspeed = 0;
  samus_x_accel_mode = 0;
  samus_y_speed = 0;
  samus_y_subspeed = 0;
}

void TorizoConfig_TickSamusFreeze(void) {
  if (g_torizo_samus_freeze_timer == 0)
    return;
  TorizoConfig_ClearSamusMotionForFreeze();
  --g_torizo_samus_freeze_timer;
}

bool TorizoConfig_SamusFreezeActive(void) {
  return g_torizo_samus_freeze_timer != 0;
}

uint16 TorizoConfig_BombChozoOrbBurstCount(void) {
  return TorizoConfig_ChozoOrbBurstCount(kTorizoBaseBombChozoOrbBurstCount,
                                         g_torizo_config.bomb_chozo_orb_burst_count);
}

uint16 TorizoConfig_BombOpeningChozoOrbBurstCount(void) {
  if (g_torizo_config.bomb_opening_chozo_orb_burst_count < 0)
    return TorizoConfig_BombChozoOrbBurstCount();
  int max_count = TorizoConfig_ClampInt(g_torizo_config.max_chozo_orbs_per_burst, 0,
                                        kTorizoDefaultMaxChozoOrbsPerBurst);
  return (uint16)TorizoConfig_ClampInt(g_torizo_config.bomb_opening_chozo_orb_burst_count, 0,
                                       max_count);
}

bool TorizoConfig_BombOpeningAttackIsChozoOrbs(void) {
  return g_torizo_config.bomb_opening_attack == kTorizoOpeningAttack_ChozoOrbs;
}

bool TorizoConfig_BombOpeningUsesScheduledChozoOrbWaves(void) {
  return g_torizo_config.bomb_opening_chozo_orb_wave_count >= 0;
}

uint16 TorizoConfig_BombOpeningChozoOrbWaveCount(void) {
  return (uint16)TorizoConfig_ClampInt(g_torizo_config.bomb_opening_chozo_orb_wave_count, 0,
                                       kTorizoMaxOpeningChozoOrbWaveCount);
}

uint16 TorizoConfig_BombOpeningChozoOrbWaveIntervalFrames(void) {
  return (uint16)TorizoConfig_ClampInt(g_torizo_config.bomb_opening_chozo_orb_wave_interval_frames, 1,
                                       kTorizoMaxOpeningChozoOrbWaveIntervalFrames);
}

uint16 TorizoConfig_GoldenChozoOrbBurstCount(void) {
  return TorizoConfig_ChozoOrbBurstCount(kTorizoBaseGoldenChozoOrbBurstCount,
                                         g_torizo_config.golden_chozo_orb_burst_count);
}

uint16 TorizoConfig_ChozoOrbContactDamage(uint16 vanilla_damage) {
  int multiplier = TorizoConfig_ClampInt(g_torizo_config.chozo_orb_contact_damage_multiplier, 0,
                                         kTorizoMaxChozoOrbDamageMultiplier);
  int damage = vanilla_damage * multiplier;
  return (uint16)TorizoConfig_ClampInt(damage, 0, kTorizoMaxEnemyProjectileDamage);
}

uint16 TorizoConfig_ChozoOrbSpawnJitterPixels(void) {
  return (uint16)TorizoConfig_ClampInt(g_torizo_config.chozo_orb_spawn_jitter_pixels, 0,
                                       kTorizoMaxChozoOrbSpawnJitterPixels);
}

void TorizoConfig_ApplyChozoOrbAppearance(uint16 eproj_idx) {
  uint16 palette = TorizoConfig_ChozoOrbPaletteIndex();
  if (palette == 0xffff)
    return;
  if (g_torizo_config.chozo_orb_palette == kTorizoChozoOrbPalette_Blue)
    memcpy(&palette_buffer[kTorizoChozoOrbBluePaletteBase], kTorizoChozoOrbBluePalette,
           sizeof(kTorizoChozoOrbBluePalette));
  eproj_gfx_idx[eproj_idx] = (eproj_gfx_idx[eproj_idx] & 0xf1ff) | palette;
}

bool TorizoConfig_IsChozoOrbEproj(uint16 projectile_id) {
  return projectile_id == addr_kEproj_BombTorizosChozoOrbs ||
         projectile_id == addr_kEproj_GoldenTorizosChozoOrbs;
}

void TorizoConfig_OnChozoOrbHitSamus(void) {
  uint16 freeze_frames = (uint16)TorizoConfig_ClampInt(g_torizo_config.chozo_orb_freeze_samus_frames, 0,
                                                       kTorizoMaxChozoOrbFreezeFrames);
  if (freeze_frames == 0)
    return;
  if (g_torizo_samus_freeze_timer < freeze_frames)
    g_torizo_samus_freeze_timer = freeze_frames;
  if (samus_knockback_timer < freeze_frames)
    samus_knockback_timer = freeze_frames;
  TorizoConfig_ClearSamusMotionForFreeze();
}

uint16 TorizoConfig_FinishExplosionBodyBurstCount(void) {
  return TorizoConfig_FinishExplosionBurstCount(kTorizoBaseFinishExplosionBodyBurstCount,
                                               g_torizo_config.finish_explosion_body_burst_multiplier,
                                               g_torizo_config.finish_explosion_body_burst_count);
}

uint16 TorizoConfig_FinishExplosionFinalBurstCount(void) {
  return TorizoConfig_FinishExplosionBurstCount(kTorizoBaseFinishExplosionFinalBurstCount,
                                               g_torizo_config.finish_explosion_final_burst_multiplier,
                                               g_torizo_config.finish_explosion_final_burst_count);
}

uint16 TorizoConfig_FinishExplosionFlashFrames(void) {
  return (uint16)TorizoConfig_ClampInt(g_torizo_config.finish_explosion_flash_frames, 1,
                                       kTorizoMaxFinishExplosionFlashFrames);
}

uint16 TorizoConfig_FinishExplosionJitterPixels(void) {
  return (uint16)TorizoConfig_ClampInt(g_torizo_config.finish_explosion_position_jitter_pixels, 0,
                                       kTorizoMaxFinishExplosionJitterPixels);
}

bool TorizoModsActive(void) {
  return TorizoConfig_ChozoOrbModsActive() ||
         TorizoConfig_AttackOrderModsActive() ||
         TorizoConfig_FinishExplosionModsActive();
}
