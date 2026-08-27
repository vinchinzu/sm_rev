#include <stdio.h>
#include <string.h>

#include "enemy_ai_canon.h"
#include "ida_types.h"

static int g_failures;

static void expect_u32(const char *name, uint32 got, uint32 want) {
  if (got != want) {
    fprintf(stderr, "FAIL %s: got 0x%06X want 0x%06X\n", name, got, want);
    g_failures++;
  }
}

static void expect_u16(const char *name, uint16 got, uint16 want) {
  if (got != want) {
    fprintf(stderr, "FAIL %s: got 0x%04X want 0x%04X\n", name, got, want);
    g_failures++;
  }
}

int main(void) {
  expect_u32("touch A2", CanonicalizeEnemyHandler(0xA28023), 0xA08023);
  expect_u32("touch A0 identity", CanonicalizeEnemyHandler(0xA08023), 0xA08023);
  expect_u32("touch A8", CanonicalizeEnemyHandler(0xA88023), 0xA08023);
  expect_u32("touch B3", CanonicalizeEnemyHandler(0xB38023), 0xA08023);
  expect_u32("shot A3", CanonicalizeEnemyHandler(0xA3802D), 0xA0802D);
  expect_u32("frozen A8", CanonicalizeEnemyHandler(0xA88041), 0xA08041);
  expect_u32("powerbomb A2", CanonicalizeEnemyHandler(0xA28037), 0xA08037);
  expect_u32("skip-death touch B3", CanonicalizeEnemyHandler(0xB38028), 0xA08028);
  expect_u32("skip-some shot A3", CanonicalizeEnemyHandler(0xA38032), 0xA08032);
  expect_u32("skip-death pb A7", CanonicalizeEnemyHandler(0xA7803C), 0xA0803C);
  expect_u32("grapple latch A2", CanonicalizeEnemyHandler(0xA28005), 0xA08005);
  expect_u32("grapple cancel A5", CanonicalizeEnemyHandler(0xA5800F), 0xA0800F);
  expect_u32("instr goto A2", CanonicalizeEnemyHandler(0xA280ED), 0xA080ED);
  expect_u32("instr sleep A9", CanonicalizeEnemyHandler(0xA9812F), 0xA0812F);
  expect_u32("nullsub_170 A2", CanonicalizeEnemyHandler(0xA2804C), 0xA0804C);
  expect_u32("nullsub_171 A2", CanonicalizeEnemyHandler(0xA2807B), 0xA0807B);

  /* Unique creature main/init must stay bank+offset. */
  expect_u32("BouncingGoofball_Main", CanonicalizeEnemyHandler(0xA2879C), 0xA2879C);
  expect_u32("Rinka_Init", CanonicalizeEnemyHandler(0xA2B602), 0xA2B602);
  expect_u32("unrelated bank", CanonicalizeEnemyHandler(0x818023), 0x818023);

  /* EnemyInstr_Call is bank-local, not an identity alias. */
  expect_u32("instr call A7 stays", CanonicalizeEnemyHandler(0xA7808A), 0xA7808A);

  EnemyDef ed;
  memset(&ed, 0, sizeof(ed));
  ed.bank = 0xA2;
  ed.touch_ai = 0x8023;
  ed.shot_ai = 0x802D;
  ed.frozen_ai = 0x8041;
  ed.grapple_ai = 0x8005;
  ed.main_ai = 0x879C;
  ed.ai_init = 0x871C;
  CanonicalizeEnemyDef(&ed);
  expect_u16("def touch_ai", ed.touch_ai, 0x8023);
  expect_u16("def shot_ai", ed.shot_ai, 0x802D);
  expect_u16("def frozen_ai", ed.frozen_ai, 0x8041);
  expect_u16("def grapple_ai", ed.grapple_ai, 0x8005);
  expect_u16("def unique main_ai", ed.main_ai, 0x879C);
  expect_u16("def unique ai_init", ed.ai_init, 0x871C);

  EnemyDef skip;
  memset(&skip, 0, sizeof(skip));
  skip.bank = 0xB3;
  skip.touch_ai = 0x8028;
  skip.shot_ai = 0x8032;
  skip.powerbomb_reaction = 0x803C;
  CanonicalizeEnemyDef(&skip);
  expect_u16("skip-death touch stays SkipDeathAnim", skip.touch_ai, 0x8028);
  expect_u16("skip-some shot stays SkipSomeParts", skip.shot_ai, 0x8032);
  expect_u16("skip-death pb stays SkipDeathAnim", skip.powerbomb_reaction, 0x803C);

  if (g_failures) {
    fprintf(stderr, "%d failure(s)\n", g_failures);
    return 1;
  }
  printf("ok: CanonicalizeEnemyHandler alias table\n");
  return 0;
}
