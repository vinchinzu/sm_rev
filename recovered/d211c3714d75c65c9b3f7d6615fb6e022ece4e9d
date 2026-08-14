#include "sm_dispatcher.h"
#include "sm_cpu_infra.h"
#include "funcs.h"

#define REG_FUNC(addr, name, is_long) RegisterSnesFunc(addr, (SnesFuncPtr)name, is_long)

// Helper macros for different function signatures to minimize boilerplate hooks
#define HOOK_V_V(name) static void name##_Hook(void) { name(); }
#define HOOK_A_V(name) static void name##_Hook(void) { name(g_cpu->a); }
#define HOOK_V_A(name) static void name##_Hook(void) { g_cpu->a = name(); }
#define HOOK_A_A(name) static void name##_Hook(void) { g_cpu->a = name(g_cpu->a); }

#define REG_HOOK(addr, name, is_long) RegisterSnesFunc(addr, name##_Hook, is_long)

#include <stdio.h>

// Bank 80 Hooks
HOOK_V_V(ReadJoypadInputs)
HOOK_V_A(NextRandom)
HOOK_A_A(PrepareBitAccess)
HOOK_A_V(SetBossBitForCurArea)
HOOK_A_V(ClearBossBitForCurArea)
HOOK_A_A(CheckBossBitForCurArea)
HOOK_A_V(SetEventHappened)
HOOK_A_V(ClearEventHappened)
HOOK_A_A(CheckEventHappened)

void ConfigureSnesDispatcher(void) {
  // Foundational logic: ReadJoypadInputs (Bank 80)
  // This replaces the original ASM at 0x809459 with the C implementation.
  REG_HOOK(0x809459, ReadJoypadInputs, true);

  // Bank 80 Utility Routines (Foundational Sub-bank)
  // These are core math and state utilities used throughout the game logic.
  REG_HOOK(0x808111, NextRandom, true);
  REG_HOOK(0x80818E, PrepareBitAccess, true);
  REG_HOOK(0x8081A6, SetBossBitForCurArea, true);
  REG_HOOK(0x8081C0, ClearBossBitForCurArea, true);
  REG_HOOK(0x8081DC, CheckBossBitForCurArea, true);
  REG_HOOK(0x8081FA, SetEventHappened, true);
  REG_HOOK(0x808212, ClearEventHappened, true);
  REG_HOOK(0x808233, CheckEventHappened, true);
}
