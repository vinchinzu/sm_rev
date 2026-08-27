// Enemy AI - Reflec — peeled from Bank $A3
#include "ida_types.h"
#include "variables.h"
#include "sm_rtl.h"
#include "funcs.h"
#include "enemy_types.h"
#include "enemy_ai_canon.h"

enum {
  kReflecBank = 163,
  kReflecCycleTimer = 16,
  kReflecPaletteBase = 137,
  kReflecPaletteColors = 4,
  kReflecPaletteFrames = 7,
  kReflecInvincibilityTimer = 10,
  kReflecNoReflection = 0x8000,
  kProjectileDir_Hit = 0x10,
  kSfx2_ReflecShot = 0x57,
};

static const uint16 kReflecGlowColors[8][4] = {
  { 0x241f, 0x1c17, 0x142f, 0x0c47 },
  { 0x211f, 0x18d8, 0x10b1, 0x086a },
  { 0x221f, 0x1999, 0x1113, 0x08ad },
  { 0x1eff, 0x163a, 0x0d95, 0x04d0 },
  { 0x1bff, 0x12fb, 0x09f7, 0x00f3 },
  { 0x1bff, 0x12fb, 0x09f7, 0x00f3 },
  { 0x1eff, 0x163a, 0x0d95, 0x04d0 },
  { 0x221f, 0x1999, 0x1113, 0x08ad },
};
static const uint16 kReflecInitIlists[8] = {
  addr_kReflec_Ilist_DB4C,
  addr_kReflec_Ilist_DB58,
  addr_kReflec_Ilist_DB64,
  addr_kReflec_Ilist_DB6E,
  addr_kReflec_Ilist_DB78,
  addr_kReflec_Ilist_DB82,
  addr_kReflec_Ilist_DB8C,
  addr_kReflec_Ilist_DB96,
};
static const uint16 kReflecDeadIlists[4] = {
  addr_kReflec_Ilist_DBA0,
  addr_kReflec_Ilist_DBAA,
  addr_kReflec_Ilist_DBB4,
  addr_kReflec_Ilist_DBBE,
};
static const uint16 kReflecBounceDir[4][16] = {
  { 0x8000, 0xfff8, 0x0007, 0xfffa, 0x8000, 0x8000, 0xfffd, 0x0002, 0xffff, 0x8000, 0, 0, 0, 0, 0, 0 },
  { 0xfffe, 0x8000, 0xfff7, 0x0008, 0xfff9, 0xfff9, 0x8000, 0xfffb, 0x0003, 0xfffe, 0, 0, 0, 0, 0, 0 },
  { 0x0004, 0xfffd, 0x8000, 0xffff, 0x0000, 0x0009, 0xfff8, 0x8000, 0xfffa, 0x0005, 0, 0, 0, 0, 0, 0 },
  { 0xfff9, 0x0006, 0xfffc, 0x8000, 0xfffe, 0xfffe, 0x0001, 0xfff7, 0x8000, 0xfff9, 0, 0, 0, 0, 0, 0 },
};

void Reflec_Func_1(void) {  // 0xA3DB0C
  if (!door_transition_flag_enemies && !--variables_for_enemy_graphics_drawn_hook[2]) {
    variables_for_enemy_graphics_drawn_hook[2] = kReflecCycleTimer;
    uint16 pal_off = variables_for_enemy_graphics_drawn_hook[0] >> 1;
    uint16 frame = variables_for_enemy_graphics_drawn_hook[1] & kReflecPaletteFrames;
    for (int i = 0; i < kReflecPaletteColors; i++)
      palette_buffer[pal_off + kReflecPaletteBase + i] = kReflecGlowColors[frame][i];
    variables_for_enemy_graphics_drawn_hook[1] = (LOBYTE(variables_for_enemy_graphics_drawn_hook[1]) + 1) & kReflecPaletteFrames;
  }
}

const uint16 *Reflec_Instr_1(uint16 k, const uint16 *jp) {  // 0xA3DBC8
  Get_Reflec(k)->reflec_parameter_2 = jp[0];
  return jp + 1;
}

void Reflec_Init(void) {  // 0xA3DBD3
  Enemy_Reflec *E = Get_Reflec(cur_enemy_index);
  E->base.properties |= kEnemyProps_BlockPlasmaBeam;
  E->base.current_instruction = kReflecInitIlists[E->reflec_parameter_1];
  enemy_gfx_drawn_hook.addr = FUNC16(Reflec_Func_1);
  *(uint16 *)&enemy_gfx_drawn_hook.bank = kReflecBank;
  variables_for_enemy_graphics_drawn_hook[0] = ((16 * E->base.palette_index) & 0xFF00) >> 8;
  variables_for_enemy_graphics_drawn_hook[2] = kReflecCycleTimer;
}

void Reflec_Shot(void) {
  uint16 proj_index = collision_detection_index;
  Enemy_Reflec *E = Get_Reflec(cur_enemy_index);
  E->base.invincibility_timer = kReflecInvincibilityTimer;
  uint16 bounce_offset = 32 * E->reflec_parameter_2 + 2 * (projectile_dir[proj_index] & 0xF);
  uint16 bounce = kReflecBounceDir[E->reflec_parameter_2][projectile_dir[proj_index] & 0xF];
  if (bounce == kReflecNoReflection) {
    projectile_dir[proj_index] |= kProjectileDir_Hit;
    printf("Possible bug. What is X?\n");
    Enemy_Reflec *ET = Get_Reflec(bounce_offset);
    if (ET->base.health) {
      NormalEnemyShotAiSkipDeathAnim_CurEnemy();
      if (!ET->base.health) {
        ET->base.current_instruction = kReflecDeadIlists[ET->reflec_parameter_2];
        ET->base.instruction_timer = 1;
        ET->base.timer = 0;
      }
    }
  } else {
    int16 reflected_dir = bounce;
    if (reflected_dir < 0)
      reflected_dir = -reflected_dir;
    projectile_dir[proj_index] = reflected_dir;
    projectile_type[proj_index] &= ~kProjectileType_DontInteractWithSamus;
    ProjectileReflection(proj_index * 2);
    QueueSfx2_Max6(kSfx2_ReflecShot);
  }
}
