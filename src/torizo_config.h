#ifndef SM_TORIZO_CONFIG_H_
#define SM_TORIZO_CONFIG_H_

#include "types.h"

typedef struct TorizoConfig {
  int chozo_orb_spawn_multiplier;
  int chozo_orb_contact_damage_multiplier;
  int chozo_orb_palette;
  int chozo_orb_freeze_samus_frames;
  int chozo_orb_spawn_jitter_pixels;
  int bomb_chozo_orb_burst_count;
  int golden_chozo_orb_burst_count;
  int bomb_opening_attack;
  int bomb_opening_chozo_orb_burst_count;
  int bomb_opening_chozo_orb_wave_count;
  int bomb_opening_chozo_orb_wave_interval_frames;
  int max_chozo_orbs_per_burst;
  int finish_explosion_body_burst_multiplier;
  int finish_explosion_body_burst_count;
  int finish_explosion_final_burst_multiplier;
  int finish_explosion_final_burst_count;
  int finish_explosion_flash_frames;
  int finish_explosion_position_jitter_pixels;
  int max_finish_explosions_per_burst;
} TorizoConfig;

extern TorizoConfig g_torizo_config;

void LoadTorizoConfig(void);
void CheckTorizoConfigReload(void);
bool TorizoModsActive(void);
void TorizoConfig_TickSamusFreeze(void);
bool TorizoConfig_SamusFreezeActive(void);
uint16 TorizoConfig_BombChozoOrbBurstCount(void);
uint16 TorizoConfig_BombOpeningChozoOrbBurstCount(void);
bool TorizoConfig_BombOpeningAttackIsChozoOrbs(void);
bool TorizoConfig_BombOpeningUsesScheduledChozoOrbWaves(void);
uint16 TorizoConfig_BombOpeningChozoOrbWaveCount(void);
uint16 TorizoConfig_BombOpeningChozoOrbWaveIntervalFrames(void);
uint16 TorizoConfig_GoldenChozoOrbBurstCount(void);
uint16 TorizoConfig_ChozoOrbContactDamage(uint16 vanilla_damage);
uint16 TorizoConfig_ChozoOrbSpawnJitterPixels(void);
void TorizoConfig_ApplyChozoOrbAppearance(uint16 eproj_idx);
bool TorizoConfig_IsChozoOrbEproj(uint16 projectile_id);
void TorizoConfig_OnChozoOrbHitSamus(void);
uint16 TorizoConfig_FinishExplosionBodyBurstCount(void);
uint16 TorizoConfig_FinishExplosionFinalBurstCount(void);
uint16 TorizoConfig_FinishExplosionFlashFrames(void);
uint16 TorizoConfig_FinishExplosionJitterPixels(void);

#endif  // SM_TORIZO_CONFIG_H_
