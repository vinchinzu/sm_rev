#ifndef SM_CPU_INFRA_H_
#define SM_CPU_INFRA_H_

#include "types.h"
#include "snes/cpu.h"
#include "snes/snes.h"

typedef struct Snes Snes;
extern Snes *g_snes;
extern bool g_fail;

// RunMode selects whether each frame runs the original SNES emulator (THEIRS),
// the native C port (MINE), or BOTH (default: run both, reconcile to emulator
// on any divergence). Fun-build mods need RM_MINE — otherwise the per-frame
// reconcile undoes any modded physics.
enum RunMode { RM_BOTH, RM_MINE, RM_THEIRS };
extern uint8 g_runmode;

typedef struct Snes Snes;

Snes *SnesInit(const char *filename);

int RunAsmCode(uint32 pc, uint16 a, uint16 x, uint16 y, int flags);
bool ProcessHook(uint32 v);
bool DispatchSnesFunc(Cpu *cpu);

typedef void (*SnesFuncPtr)(void);
void RegisterSnesFunc(uint32 addr, SnesFuncPtr func, bool is_long);

void Call(uint32 addr);

void RunOneFrameOfGame();
void ClearUnusedOam();

void RunOneFrameOfGame_Both();

#endif  // SM_CPU_INFRA_H_  