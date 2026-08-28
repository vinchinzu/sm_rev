// Enemy AI - Zebes escape typewriter — peeled from Bank $A6
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"

enum {
  kTypewriterCmd_SetTimer = 1,
  kTypewriterCmd_Newline = 13,
  kTypewriterChar_Space = 32,
  kTypewriterChar_Exclamation = 33,
  kTypewriterExclamationTile = 91,
  kTypewriterLetterBase = 65,
  kTypewriterVramSrcBank = 0x7E00,
  kTypewriterTileBytes = 2,
  kTypewriterQueueEntrySize = 7,
  kTypewriterStrokePeriod = 2,
  kArea_Ceres = 6,
  kSfx2_TypewriterCeres = 0x45,
  kSfx3_TypewriterIntro = 0xD,
};

void SetupZebesEscapeTypewriter(void) {  // 0xA6C23F
  palette_buffer[157] = palette_buffer[125];
  palette_buffer[158] = palette_buffer[126];
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  E->mbn_var_3B = addr_loc_A9C49C;
  E->mbn_var_3C = 0;
  E->mbn_var_3D = 0;
  E->mbn_var_3E = 0;
  E->mbn_var_3F = 0;
  reg_BG2HOFS = 0;
  reg_BG2VOFS = 0;
}

uint8 ProcessEscapeTimerTileTransfers(void) {  // 0xA6C26E
  EnemyData *E = gEnemyData(0);
  uint16 list_ip = E->ai_var_E;
  uint16 queue_tail = vram_write_queue_tail;
  const uint8 *entry = RomPtr_A6(list_ip);
  uint8 finished = 1;
  if (GET_WORD(entry)) {
    VramWriteEntry *q = gVramWriteEntry(vram_write_queue_tail);
    q->size = GET_WORD(entry);
    *(VoidP *)((uint8 *)&q->src.addr + 1) = GET_WORD(entry + 3);
    q->src.addr = GET_WORD(entry + 2);
    q->vram_dst = GET_WORD(entry + 5);
    vram_write_queue_tail = queue_tail + kTypewriterQueueEntrySize;
    E->ai_var_E = list_ip + kTypewriterQueueEntrySize;
    if (*(uint16 *)RomPtr_A6(E->ai_var_E))
      return 0;
  }
  return finished;
}

uint8 HandleTypewriterText_Ext(uint16 a) {  // 0xA6C2A7
  uint16 base_tile = a;
  Enemy_MotherBrain *E = Get_MotherBrain(0);
  uint16 instr_timer = E->mbn_var_3D;
  if (instr_timer) {
    E->mbn_var_3D = instr_timer - 1;
    return 0;
  }

  E->mbn_var_3D = E->mbn_var_3E;
  uint16 ip = E->mbn_var_3B;
  int16 cmd;
  for (;;) {
    cmd = *(uint16 *)RomPtr_A6(ip);
    if (!cmd)
      return 1;
    if (cmd == kTypewriterCmd_SetTimer) {
      ip += 2;
      E->mbn_var_3E = *(uint16 *)RomPtr_A6(ip);
      ip += 2;
      continue;
    }
    if (cmd != kTypewriterCmd_Newline)
      break;
    ip += 2;
    E->mbn_var_3C = *(uint16 *)RomPtr_A6(ip);
    ip += 2;
  }

  uint16 ch = (uint8)cmd;
  if (ch == kTypewriterChar_Space) {
    ++E->mbn_var_3C;
    E->mbn_var_3B = ip + 1;
    return 0;
  }

  if (ch == kTypewriterChar_Exclamation)
    ch = kTypewriterExclamationTile;
  E->mbn_var_3B = ip + 1;
  uint16 queue_tail = vram_write_queue_tail;
  VramWriteEntry *q = gVramWriteEntry(vram_write_queue_tail);
  q->size = kTypewriterTileBytes;
  *(VoidP *)((uint8 *)&q->src.addr + 1) = kTypewriterVramSrcBank;
  E->mbn_var_3A = base_tile + ch - kTypewriterLetterBase;
  q->src.addr = ADDR16_OF_RAM(*extra_enemy_ram8000) + 52;
  uint16 vram_dst = E->mbn_var_3C;
  q->vram_dst = vram_dst;
  E->mbn_var_3C = vram_dst + 1;
  vram_write_queue_tail = queue_tail + kTypewriterQueueEntrySize;
  uint16 stroke = E->mbn_var_3F + 1;
  E->mbn_var_3F = stroke;
  if (!sign16(stroke - kTypewriterStrokePeriod)) {
    E->mbn_var_3F = 0;
    if (area_index == kArea_Ceres)
      QueueSfx2_Max3(kSfx2_TypewriterCeres);
    else
      QueueSfx3_Max3(kSfx3_TypewriterIntro);
  }
  return 0;
}
