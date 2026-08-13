#include "multi_samus.h"
#include "variables.h"
#include <string.h>
#include <stdlib.h>

enum {
  kMultiSamusMaxPlayers = 2,
  kMultiSamusPlayerTwoSpawnOffsetX = 48,
  kMultiSamusJoypadStateOffset = 0x87,
  kMultiSamusJoypadStateSize = 0x20,
  kMultiSamusStateStart = 0x9A2,
  kMultiSamusProjectileStateStart = 0xB64,
  kMultiSamusProjectileStateEnd = 0xCCC,
  kMultiSamusStateEnd = 0xDC2,
  kMultiSamusXPositionOffset = 0xAF6,
  kMultiSamusPreProjectileSize =
      kMultiSamusProjectileStateStart - kMultiSamusStateStart,
  kMultiSamusPostProjectileSize =
      kMultiSamusStateEnd - kMultiSamusProjectileStateEnd,
};

typedef struct SamusState {
  uint8 joypad_block[kMultiSamusJoypadStateSize];
  uint8 samus_pre_projectile_block[kMultiSamusPreProjectileSize];
  uint8 samus_post_projectile_block[kMultiSamusPostProjectileSize];
} SamusState;

static SamusState g_samus_states[kMultiSamusMaxPlayers];
static int g_active_samus;
static int g_num_samus = 1;
static bool g_initialized;
static MultiSamusProjectileSpawnHook g_projectile_spawn_hook;
static void *g_projectile_spawn_context;
static MultiSamusPlayerInputHook g_player_input_hook;
static void *g_player_input_context;
static MultiSamusPlayerInputHook g_player_input_end_hook;
static void *g_player_input_end_context;
static int g_projectile_spawn_player = -1;

static void MultiSamus_InitWithCount(int num_samus) {
  if (num_samus < 1)
    num_samus = 1;
  if (num_samus > kMultiSamusMaxPlayers)
    num_samus = kMultiSamusMaxPlayers;
  g_num_samus = num_samus;
  g_active_samus = 0;
  g_projectile_spawn_player = -1;
  g_initialized = true;
  
  // Clear states
  memset(g_samus_states, 0, sizeof(g_samus_states));
  
  // Capture current state into Samus 0
  memcpy(g_samus_states[0].joypad_block, g_ram + kMultiSamusJoypadStateOffset,
         kMultiSamusJoypadStateSize);
  memcpy(g_samus_states[0].samus_pre_projectile_block,
         g_ram + kMultiSamusStateStart, kMultiSamusPreProjectileSize);
  memcpy(g_samus_states[0].samus_post_projectile_block,
         g_ram + kMultiSamusProjectileStateEnd, kMultiSamusPostProjectileSize);
  
  if (g_num_samus > 1) {
    // Clone Samus 0 to Samus 1 initially.
    memcpy(&g_samus_states[1], &g_samus_states[0], sizeof(SamusState));

    // Offset Samus 2 so both full sprites are visible at startup.
    uint16 *s2_x = (uint16 *)&g_samus_states[1].samus_pre_projectile_block[
        kMultiSamusXPositionOffset - kMultiSamusStateStart];
    *s2_x += kMultiSamusPlayerTwoSpawnOffsetX;
  }
}

void MultiSamus_Init(void) {
  const char *multi_samus = getenv("SM_MULTI_SAMUS");
  MultiSamus_InitWithCount(
      (multi_samus && multi_samus[0] == '1' && multi_samus[1] == '\0') ? 2 : 1);
}

void MultiSamus_SetNumSamus(int num_samus) {
  MultiSamus_InitWithCount(num_samus);
}

void MultiSamus_Switch(int index) {
  if (!g_initialized) MultiSamus_Init();
  if (index < 0 || index >= kMultiSamusMaxPlayers) return;
  if (g_active_samus == index) return;

  uint16 current_joypad2_last = joypad2_last;
  uint16 current_joypad2_new_keys = joypad2_new_keys;
  uint16 current_joypad2_newkeys2 = joypad2_newkeys2;
  uint16 current_joypad2_prev = joypad2_prev;

  // Save current
  memcpy(g_samus_states[g_active_samus].joypad_block,
         g_ram + kMultiSamusJoypadStateOffset, kMultiSamusJoypadStateSize);
  memcpy(g_samus_states[g_active_samus].samus_pre_projectile_block,
         g_ram + kMultiSamusStateStart, kMultiSamusPreProjectileSize);
  memcpy(g_samus_states[g_active_samus].samus_post_projectile_block,
         g_ram + kMultiSamusProjectileStateEnd, kMultiSamusPostProjectileSize);
  
  g_active_samus = index;
  
  // Load new
  memcpy(g_ram + kMultiSamusJoypadStateOffset,
         g_samus_states[g_active_samus].joypad_block, kMultiSamusJoypadStateSize);
  memcpy(g_ram + kMultiSamusStateStart,
         g_samus_states[g_active_samus].samus_pre_projectile_block,
         kMultiSamusPreProjectileSize);
  memcpy(g_ram + kMultiSamusProjectileStateEnd,
         g_samus_states[g_active_samus].samus_post_projectile_block,
         kMultiSamusPostProjectileSize);
  joypad2_last = current_joypad2_last;
  joypad2_new_keys = current_joypad2_new_keys;
  joypad2_newkeys2 = current_joypad2_newkeys2;
  joypad2_prev = current_joypad2_prev;
  
  if (g_active_samus == 1) {
    // The original Samus code reads controller 1, so run P2's snapshot with
    // this frame's controller-2 input mirrored into the controller-1 fields.
    joypad1_lastkeys = current_joypad2_last;
    joypad1_newkeys = current_joypad2_new_keys;
    joypad1_newkeys2_UNUSED = current_joypad2_newkeys2;
    joypad1_prev = current_joypad2_prev;
  }
}

int MultiSamus_GetNumSamus(void) {
  return g_num_samus;
}

void MultiSamus_SetProjectileSpawnHook(MultiSamusProjectileSpawnHook hook,
                                       void *context) {
  g_projectile_spawn_hook = hook;
  g_projectile_spawn_context = context;
}

void MultiSamus_SetPlayerInputHook(MultiSamusPlayerInputHook hook,
                                   void *context) {
  g_player_input_hook = hook;
  g_player_input_context = context;
}

void MultiSamus_NotifyPlayerInput(int player_index) {
  if (g_player_input_hook != NULL)
    g_player_input_hook(player_index, g_player_input_context);
}

void MultiSamus_SetPlayerInputEndHook(MultiSamusPlayerInputHook hook,
                                      void *context) {
  g_player_input_end_hook = hook;
  g_player_input_end_context = context;
}

void MultiSamus_NotifyPlayerInputEnd(int player_index) {
  if (g_player_input_end_hook != NULL)
    g_player_input_end_hook(player_index, g_player_input_end_context);
}

void MultiSamus_SetProjectileSpawnPlayer(int player_index) {
  g_projectile_spawn_player = player_index;
}

void MultiSamus_NotifyProjectileSpawn(uint16 projectile_slot) {
  if (g_projectile_spawn_hook != NULL)
    g_projectile_spawn_hook(g_projectile_spawn_player >= 0
                                ? g_projectile_spawn_player
                                : g_active_samus,
                            projectile_slot,
                            g_projectile_spawn_context);
}
