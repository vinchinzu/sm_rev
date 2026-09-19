#include "funcs.h"
#include "ida_types.h"
#include "mini/mini_game.h"
#include "sm_rtl.h"
#include "types.h"
#include "variables.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(__GNUC__)
#define PICO_STUB_NORETURN __attribute__((noreturn))
#else
#define PICO_STUB_NORETURN
#endif

PICO_STUB_NORETURN void PicoStubDie(const char *name) {
  fprintf(stderr, "pico-stub: %s\n", name);
  abort();
}

const uint8 *RomPtr(uint32_t addr) {
  if (g_rom == NULL)
    Die("pico: RomPtr with g_rom == NULL (no ROM image)\n");
  return &g_rom[(((addr >> 16) << 15) | (addr & 0x7fff)) & 0x3fffff];
}

/* NO_SOUND: Mini-family builds compile without APU/SPC. KEEP Samus/PLM/game
 * still call these on shots, pickups, and GameState_8. Safe to drop. */
void QueueSfx1_Internal(uint16 a) { (void)a; }
void QueueSfx1_Max1(uint16 a) { (void)a; }
void QueueSfx1_Max15(uint16 a) { (void)a; }
void QueueSfx1_Max3(uint16 a) { (void)a; }
void QueueSfx1_Max6(uint16 a) { (void)a; }
void QueueSfx1_Max9(uint16 a) { (void)a; }
void QueueSfx2_Internal(uint16 a) { (void)a; }
void QueueSfx2_Max1(uint16 a) { (void)a; }
void QueueSfx2_Max15(uint16 a) { (void)a; }
void QueueSfx2_Max3(uint16 a) { (void)a; }
void QueueSfx2_Max6(uint16 a) { (void)a; }
void QueueSfx2_Max9(uint16 a) { (void)a; }
void QueueSfx3_Internal(uint16 a) { (void)a; }
void QueueSfx3_Max1(uint16 a) { (void)a; }
void QueueSfx3_Max15(uint16 a) { (void)a; }
void QueueSfx3_Max3(uint16 a) { (void)a; }
void QueueSfx3_Max6(uint16 a) { (void)a; }
void QueueSfx3_Max9(uint16 a) { (void)a; }
void QueueMusic_Delayed8(uint16 a) { (void)a; }
void QueueMusic_DelayedY(uint16 a, uint16 j) { (void)a; (void)j; }
void HandleSoundEffects(void) {}
void HandleMusicQueue(void) {}
void ResetSoundQueues(void) {}
uint8 HasQueuedMusic(void) { return 0; }

/* SHIP2 FX/HDMA/animtiles are not linked in ship1. MiniCreate enables them
 * and MiniStep/GameState_8 call the handlers every frame; with no objects
 * queued, a no-op leaves Samus physics unchanged. */
void EnablePaletteFx(void) {}
void EnableHdmaObjects(void) {}
void EnableAnimtiles(void) {}
void ClearPaletteFXObjects(void) {}
void PaletteFxHandler(void) {}
void HdmaObjectHandler(void) {}
void AnimtilesHandler(void) {}

/* SHIP2 NMI/VRAM. Mini original-runtime frames call Vector_NMI after
 * waiting_for_nmi=1; physics does not depend on the PPU copy. */
void Vector_NMI(void) {}
void NMI_ProcessVramWriteQueue(void) {}
void NMI_ProcessVramReadQueue(void) {}
void NmiProcessAnimtilesVramTransfers(void) {}
void ClearOamExt(void) {}
void ClearUnusedOam(void) {}
void CopyToVramNow(uint16 vram_dst, uint32 src, uint16 size) {
  (void)vram_dst;
  (void)src;
  (void)size;
}

/* SHIP2 enemy/eproj/HUD/room-main. GameState_8 and MiniAssetBootstrap call
 * these during Landing Site boot and every frame. Ship1 MiniStep only needs
 * Samus motion, so skip the ripped systems. */
void EnableEprojs(void) {}
void ClearEprojs(void) {}
void EprojRunAll(void) {}
void EprojSamusCollDetect(void) {}
void EprojProjCollDet(void) {}
void ProcessEnemyPowerBombInteraction(void) {}
void SamusProjectileInteractionHandler(void) {}
void DetermineWhichEnemiesToProcess(void) {}
void EnemyMain(void) {}
void LoadEnemies(void) {}
void DrawSamusEnemiesAndProjectiles(void) {}
void QueueEnemyBG2TilemapTransfers(void) {}
void HandleRoomShaking(void) {}
void HandleHudTilemap(void) {}
void RunRoomMainCode(void) {}
void HandleSamusOutOfHealthAndGameTile(void) {}
void DecrementSamusTimers(void) {}
void TransferEnemyTilesToVramAndInit(void) {}
void RefreshFxVisualsAfterLoad(void) {}
void InitializeSpecialEffectsForNewRoom(void) {}
void LoadFXHeader(void) {}

/* plm_rooms.c is STUB; this is the whole vanilla body. KEEP plm_blocks
 * calls it while spawning room PLMs during MiniCreate. */
void DeletePlm(uint16 j) {
  plm_header_ptr[j >> 1] = 0;
}

/* SHIP2 SRAM. Returning 1 is the vanilla checksum-fail path so MiniCreate
 * can fall through to demo/fallback instead of hanging. Save is unused. */
uint8 LoadFromSram(uint16 a) {
  (void)a;
  return 1;
}
void SaveToSram(uint16 a) { (void)a; }

/* OMIT mini TUs still called from KEEP mini: single-player MiniStep has no
 * multiplayer combat, and climb-endless is out of the ship1 content boundary. */
void MiniUpdateMultiplayerCombat(MiniGameState *state) { (void)state; }
int MiniClimbEndless_PirateShotCooldownFrames(const MiniGameState *state) {
  (void)state;
  return 60;
}

/* SHIP2 enemy_config. KEEP mini_enemy_metadata / room load may call this;
 * hang rather than dereference a NULL EnemyDef. */
EnemyDef *get_EnemyDef_A2(uint16 a) {
  (void)a;
  PicoStubDie("get_EnemyDef_A2");
}

#include "pico_stubs_generated.inc"
