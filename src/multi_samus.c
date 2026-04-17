#include "multi_samus.h"
#include "variables.h"
#include <string.h>
#include <stdio.h>

#define MAX_SAMUS 2

typedef struct SamusState {
  uint8 joypad_block[0x20]; // 0x87 to 0xA7
  uint8 samus_block[0x420];  // 0x9A2 to 0xDC2
} SamusState;

SamusState g_samus_states[MAX_SAMUS];
int g_active_samus = 0;
int g_num_samus = 1;
bool g_initialized = false;

void MultiSamus_Init(void) {
  g_num_samus = 2; 
  g_active_samus = 0;
  g_initialized = true;
  
  // Clear states
  memset(g_samus_states, 0, sizeof(g_samus_states));
  
  // Capture current state into Samus 0
  memcpy(g_samus_states[0].joypad_block, g_ram + 0x87, 0x20);
  memcpy(g_samus_states[0].samus_block, g_ram + 0x9A2, 0x420);
  
  // Clone Samus 0 to Samus 1 initially
  memcpy(&g_samus_states[1], &g_samus_states[0], sizeof(SamusState));
  
  // Offset Samus 2 slightly so they don't overlap perfectly
  uint16 *s2_x = (uint16*)&g_samus_states[1].samus_block[0xAF6 - 0x9A2];
  *s2_x += 20;
}

void MultiSamus_Switch(int index) {
  if (!g_initialized) MultiSamus_Init();
  if (index < 0 || index >= MAX_SAMUS) return;
  if (g_active_samus == index) return;

  // Save current
  memcpy(g_samus_states[g_active_samus].joypad_block, g_ram + 0x87, 0x20);
  memcpy(g_samus_states[g_active_samus].samus_block, g_ram + 0x9A2, 0x420);
  
  g_active_samus = index;
  
  // Load new
  memcpy(g_ram + 0x87, g_samus_states[g_active_samus].joypad_block, 0x20);
  memcpy(g_ram + 0x9A2, g_samus_states[g_active_samus].samus_block, 0x420);
  
  // Map player 2 inputs to Samus 2 if needed
  if (g_active_samus == 1) {
    // We want the logic that uses joypad1_lastkeys to use player 2's input
    // Player 2's input is kept in joypad2_last (0x8D)
    // Actually, ReadJoypadInputs already filled joypad2_last.
    // So we copy joypad2 -> joypad1 area for the P1 logic to work on Samus 2.
    
    // joypad1_lastkeys = 0x8B, joypad2_last = 0x8D
    *(uint16*)(g_ram + 0x8B) = *(uint16*)(g_ram + 0x8D);
    *(uint16*)(g_ram + 0x8F) = *(uint16*)(g_ram + 0x91); // newkeys
    *(uint16*)(g_ram + 0x97) = *(uint16*)(g_ram + 0x99); // prev
  }
}

int MultiSamus_GetNumSamus(void) {
  return g_num_samus;
}
