#include "enemy_ai_canon.h"
#include "ida_types.h"

/*
 * Bank-local identity wrappers live at the same 16-bit offset in every enemy
 * AI bank (A2-AA, B2, B3). Map those 24-bit aliases onto the Bank $A0
 * handlers. Unique creature AI is left unchanged.
 *
 * EnemyInstr_Call (*808A) is bank-specific (hardcoded bank in the wrapper)
 * and is not aliased.
 */

static bool IsEnemyHandlerBank(uint8 bank) {
  switch (bank) {
  case 0xA2:
  case 0xA3:
  case 0xA4:
  case 0xA5:
  case 0xA6:
  case 0xA7:
  case 0xA8:
  case 0xA9:
  case 0xAA:
  case 0xB2:
  case 0xB3:
    return true;
  default:
    return false;
  }
}

static bool IsSharedEnemyHandlerOffset(uint16 off) {
  switch (off) {
  case 0x8000: /* GrappleReact_NoInteract */
  case 0x8005: /* GrappleReact_SamusLatchesOn */
  case 0x800A: /* GrappleReact_KillEnemy */
  case 0x800F: /* GrappleReact_CancelBeam */
  case 0x8014: /* GrappleReact_SamusLatchesNoInvinc */
  case 0x8019: /* GrappleReact_SamusLatchesParalyze */
  case 0x801E: /* GrappleReact_HurtSamus */
  case 0x8023: /* NormalTouchAI */
  case 0x8028: /* NormalTouchAI_SkipDeathAnim */
  case 0x802D: /* NormalShotAI */
  case 0x8032: /* NormalShotAI_SkipSomeParts */
  case 0x8037: /* NormalPowerBombAI */
  case 0x803C: /* NormalPowerBombAI_SkipDeathAnim */
  case 0x8041: /* NormalFrozenAI */
  case 0x8046: /* CreateADudShot */
  case 0x804B: /* nullsub_169 */
  case 0x804C: /* nullsub_170 */
  case 0x806B: /* EnemyInstr_SetAiPreInstr */
  case 0x8074: /* EnemyInstr_ClearAiPreInstr */
  case 0x807B: /* nullsub_171 */
  case 0x807C: /* EnemyInstr_StopScript */
  case 0x80ED: /* EnemyInstr_Goto */
  case 0x8108: /* EnemyInstr_DecTimerAndGoto */
  case 0x8110: /* EnemyInstr_DecTimerAndGoto2 */
  case 0x8123: /* EnemyInstr_SetTimer */
  case 0x812F: /* EnemyInstr_Sleep */
  case 0x813A: /* EnemyInstr_WaitNframes */
  case 0x814B: /* EnemyInstr_CopyToVram */
  case 0x8173: /* EnemyInstr_EnableOffScreenProcessing */
  case 0x817D: /* EnemyInstr_DisableOffScreenProcessing */
    return true;
  default:
    return false;
  }
}

uint32 CanonicalizeEnemyHandler(uint32 ea) {
  uint8 bank = (uint8)(ea >> 16);
  uint16 off = (uint16)ea;
  if (!IsEnemyHandlerBank(bank) || !IsSharedEnemyHandlerOffset(off))
    return ea;
  return 0xA00000 | off;
}

static void CanonicalizeEnemyDefField(EnemyDef *ed, VoidP *field) {
  uint32 canon = CanonicalizeEnemyHandler(((uint32)ed->bank << 16) | *field);
  *field = (VoidP)canon;
}

void CanonicalizeEnemyDef(EnemyDef *ed) {
  if (!ed)
    return;
  CanonicalizeEnemyDefField(ed, &ed->ai_init);
  CanonicalizeEnemyDefField(ed, &ed->main_ai);
  CanonicalizeEnemyDefField(ed, &ed->grapple_ai);
  CanonicalizeEnemyDefField(ed, &ed->hurt_ai);
  CanonicalizeEnemyDefField(ed, &ed->frozen_ai);
  CanonicalizeEnemyDefField(ed, &ed->time_is_frozen_ai);
  CanonicalizeEnemyDefField(ed, &ed->powerbomb_reaction);
  CanonicalizeEnemyDefField(ed, &ed->touch_ai);
  CanonicalizeEnemyDefField(ed, &ed->shot_ai);
}
