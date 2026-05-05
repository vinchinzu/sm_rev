#include "multi_samus.h"
#include "variables.h"
#include <string.h>
#include <stdlib.h>

#define MAX_SAMUS 2
#define PLAYER_TWO_SPAWN_OFFSET_X 48
#define SAMUS_STATE_START 0x9A2
#define SAMUS_PROJECTILE_STATE_START 0xB64
#define SAMUS_PROJECTILE_STATE_END 0xCCC
#define SAMUS_STATE_END 0xDC2
#define SAMUS_PRE_PROJECTILE_SIZE (SAMUS_PROJECTILE_STATE_START - SAMUS_STATE_START)
#define SAMUS_POST_PROJECTILE_SIZE (SAMUS_STATE_END - SAMUS_PROJECTILE_STATE_END)

typedef struct SamusState {
  uint8 joypad_block[0x20]; // 0x87 to 0xA7
  uint8 samus_pre_projectile_block[SAMUS_PRE_PROJECTILE_SIZE];
  uint8 samus_post_projectile_block[SAMUS_POST_PROJECTILE_SIZE];
} SamusState;

SamusState g_samus_states[MAX_SAMUS];
int g_active_samus = 0;
int g_num_samus = 1;
bool g_initialized = false;

static void MultiSamus_InitWithCount(int num_samus) {
  if (num_samus < 1)
    num_samus = 1;
  if (num_samus > MAX_SAMUS)
    num_samus = MAX_SAMUS;
  g_num_samus = num_samus;
  g_active_samus = 0;
  g_initialized = true;
  
  // Clear states
  memset(g_samus_states, 0, sizeof(g_samus_states));
  
  // Capture current state into Samus 0
  memcpy(g_samus_states[0].joypad_block, g_ram + 0x87, 0x20);
  memcpy(g_samus_states[0].samus_pre_projectile_block, g_ram + SAMUS_STATE_START,
         SAMUS_PRE_PROJECTILE_SIZE);
  memcpy(g_samus_states[0].samus_post_projectile_block, g_ram + SAMUS_PROJECTILE_STATE_END,
         SAMUS_POST_PROJECTILE_SIZE);
  
  if (g_num_samus > 1) {
    // Clone Samus 0 to Samus 1 initially.
    memcpy(&g_samus_states[1], &g_samus_states[0], sizeof(SamusState));

    // Offset Samus 2 so both full sprites are visible at startup.
    uint16 *s2_x = (uint16 *)&g_samus_states[1].samus_pre_projectile_block[0xAF6 - SAMUS_STATE_START];
    *s2_x += PLAYER_TWO_SPAWN_OFFSET_X;
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
  if (index < 0 || index >= MAX_SAMUS) return;
  if (g_active_samus == index) return;

  uint16 current_joypad2_last = joypad2_last;
  uint16 current_joypad2_new_keys = joypad2_new_keys;
  uint16 current_joypad2_newkeys2 = joypad2_newkeys2;
  uint16 current_joypad2_prev = joypad2_prev;

  // Save current
  memcpy(g_samus_states[g_active_samus].joypad_block, g_ram + 0x87, 0x20);
  memcpy(g_samus_states[g_active_samus].samus_pre_projectile_block, g_ram + SAMUS_STATE_START,
         SAMUS_PRE_PROJECTILE_SIZE);
  memcpy(g_samus_states[g_active_samus].samus_post_projectile_block, g_ram + SAMUS_PROJECTILE_STATE_END,
         SAMUS_POST_PROJECTILE_SIZE);
  
  g_active_samus = index;
  
  // Load new
  memcpy(g_ram + 0x87, g_samus_states[g_active_samus].joypad_block, 0x20);
  memcpy(g_ram + SAMUS_STATE_START, g_samus_states[g_active_samus].samus_pre_projectile_block,
         SAMUS_PRE_PROJECTILE_SIZE);
  memcpy(g_ram + SAMUS_PROJECTILE_STATE_END,
         g_samus_states[g_active_samus].samus_post_projectile_block,
         SAMUS_POST_PROJECTILE_SIZE);
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
