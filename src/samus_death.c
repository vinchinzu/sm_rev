// Samus death sequence: animation, palette effects, and explosion handling.
// Extracted from sm_9b.c (Cluster B).

#include "sm_cpu_infra.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"

#define g_off_9BB5C8 ((uint16*)RomFixedPtr(0x9bb5c8))
#define g_byte_9BB823 ((uint8*)RomFixedPtr(0x9bb823))
#define g_off_9BB6D2 ((uint16*)RomFixedPtr(0x9bb6d2))
#define kDeathSequencePals_PowerSuit ((uint16*)RomFixedPtr(0x9bb7d3))
#define kDeathSequencePals_VariaSuit ((uint16*)RomFixedPtr(0x9bb7e7))
#define kDeathSequencePals_GravitySuit ((uint16*)RomFixedPtr(0x9bb7fb))
#define kDeathSequencePals_Suitless ((uint16*)RomFixedPtr(0x9bb80f))

static const uint8 kDeathAnimationFrames[28] = {
  5, 5, 5, 5, 1, 5, 5, 0, 1,
  0, 5, 5, 5, 5, 5, 5, 5, 1,
  1, 1, 5, 5, 5, 5, 5, 5, 5,
  5,
};

static const uint16 g_word_9BB7BF[5] = { 0x8400, 0x8800, 0x8c00, 0x9000, 0x8000 };
static const uint16 g_word_9BB7C9[5] = { 0x6200, 0x6400, 0x6600, 0x6800, 0x6000 };
static const uint16 kShadesOfWhite[22] = {
   0x421,  0xc63, 0x14a5, 0x1ce7, 0x2529, 0x2d6b, 0x35ad, 0x4210,
  0x4a52, 0x4e73, 0x5294, 0x56b5, 0x5ad6, 0x5ef7, 0x6318, 0x6739,
  0x6b5a, 0x6f7b, 0x739c, 0x77bd, 0x7bde, 0x7fff,
};

void StartSamusDeathAnimation(void) {  // 0x9BB3A7
  uint16 v0 = samus_movement_type;
  if (samus_movement_type == 3)
    QueueSfx1_Max6(0x32);
  uint16 v1 = kDeathAnimationFrames[v0];
  if (samus_pose_x_dir == 4)
    samus_pose = kPose_D8_FaceL_CrystalFlashEnd;
  else
    samus_pose = kPose_D7_FaceR_CrystalFlashEnd;
  SamusFunc_F433();
  Samus_SetAnimationFrameIfPoseChanged();
  samus_last_different_pose = samus_prev_pose;
  *(uint16 *)&samus_last_different_pose_x_dir = *(uint16 *)&samus_prev_pose_x_dir;
  samus_prev_pose = samus_pose;
  *(uint16 *)&samus_prev_pose_x_dir = *(uint16 *)&samus_pose_x_dir;
  samus_anim_frame_skip = 0;
  samus_anim_frame = v1;
  samus_x_pos -= layer1_x_pos;
  samus_y_pos -= layer1_y_pos;
}

void DrawSamusStartingDeathAnim_(void) {  // 0x9BB43C
  Samus_DrawStartingDeathAnim();
}

uint16 HandleSamusDeathSequence(void) {  // 0x9BB441
  if (sign16(g_word_7E0DE6 - 4))
    QueueTransferOfSamusDeathSequence(2 * g_word_7E0DE6);
  if (sign16(++g_word_7E0DE6 - 60)) {
    bool v0 = (--game_options_screen_index & 0x8000) != 0;
    if (!game_options_screen_index || v0) {
      if (g_word_7E0DE4) {
        g_word_7E0DE4 = 0;
        game_options_screen_index = 3;
      } else {
        g_word_7E0DE4 = 1;
        game_options_screen_index = 1;
      }
      CopyPalettesForSamusDeath(g_word_7E0DE4 * 2);
    }
    return 0;
  } else {
    HandleSamusDeathSequence_Helper2();
    substate = (joypad2_last & 0xB0) == (kButton_A | kButton_L | kButton_R);
    return 1;
  }
}

void HandleSamusDeathSequence_Helper2(void) {  // 0x9BB4B6
  const uint16 *v0 = (const uint16 *)RomPtr_9B(g_off_9BB5C8[samus_suit_palette_index >> 1]);
  memcpy(&palette_buffer[192], RomPtr_9B(*v0), 32);
  memcpy(&palette_buffer[240], RomPtr_9B(addr_word_9BA120), 32);
  QueueTransferOfSamusDeathSequence(8);
  game_options_screen_index = g_byte_9BB823[0];
  g_word_7E0DE4 = 0;
  g_word_7E0DE6 = 0;
  GameState_24_SamusNoHealth_Explosion_2();
}

void CopyPalettesForSamusDeath(uint16 v0) {  // 0x9BB5CE
  int r20 = g_off_9BB6D2[samus_suit_palette_index >> 1];
  const uint16 *v1 = (const uint16 *)RomPtr_9B(r20 + v0);
  memcpy(&palette_buffer[192], RomPtr_9B(*v1), 32);
  memcpy(&palette_buffer[240], RomPtr_9B(kDeathSequencePals_Suitless[v0 >> 1]), 32);
}

void QueueTransferOfSamusDeathSequence(uint16 v0) {  // 0x9BB6D8
  uint16 v1 = vram_write_queue_tail;
  gVramWriteEntry(vram_write_queue_tail)->size = 1024;
  v1 += 2;
  int v2 = v0 >> 1;
  gVramWriteEntry(v1)->size = g_word_9BB7BF[v2];
  v1 += 2;
  LOBYTE(gVramWriteEntry(v1++)->size) = 0x9B;
  gVramWriteEntry(v1)->size = g_word_9BB7C9[v2];
  vram_write_queue_tail = v1 + 2;
}

uint16 GameState_24_SamusNoHealth_Explosion_Helper(void) {  // 0x9BB701
  GameState_24_SamusNoHealth_Explosion_1();
  return GameState_24_SamusNoHealth_Explosion_2();
}

void GameState_24_SamusNoHealth_Explosion_1(void) {  // 0x9BB710
  if (!substate && g_word_7E0DE4) {
    int v0 = g_word_7E0DE6;
    uint16 *dst = palette_buffer;
    for(int i = 0; i < 384/2; i++)
      dst[i] = kShadesOfWhite[v0];
    for(int i = 416/2; i < 480/2; i++)
      dst[i] = kShadesOfWhite[v0];
    if (sign16(g_word_7E0DE6 - 20))
      ++g_word_7E0DE6;
  }
}

uint16 GameState_24_SamusNoHealth_Explosion_2(void) {  // 0x9BB758
  bool v0 = (--game_options_screen_index & 0x8000) != 0;
  if (!game_options_screen_index || v0) {
    if (!sign16(++g_word_7E0DE4 - 9)) {
      g_word_7E0DE6 = 21;
      GameState_24_SamusNoHealth_Explosion_1();
      substate = 0;
      return 1;
    }
    if (!substate || sign16(g_word_7E0DE4 - 2)) {
      game_options_screen_index = g_byte_9BB823[(2 * g_word_7E0DE4)];
      CopyPalettesForSamusDeath(2 * g_byte_9BB823[(2 * g_word_7E0DE4) + 1]);
    } else {
      game_options_screen_index = g_byte_9BB823[(2 * g_word_7E0DE4)];
    }
  }
  DrawSamusSuitExploding();
  return 0;
}
