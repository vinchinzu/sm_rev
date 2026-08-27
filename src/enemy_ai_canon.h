#ifndef ENEMY_AI_CANON_H
#define ENEMY_AI_CANON_H

#include "types.h"

struct EnemyDef;

typedef void (*EnemyAiFn)(void);
typedef void (*EnemyPreInstrFn)(uint16 k);
typedef const uint16 *(*EnemyInstrFn)(uint16 k, const uint16 *j);

typedef struct EnemyDefAiFns {
  EnemyAiFn ai_init;
  EnemyAiFn main_ai;
  EnemyAiFn grapple_ai;
  EnemyAiFn hurt_ai;
  EnemyAiFn frozen_ai;
  EnemyAiFn time_is_frozen_ai;
  EnemyAiFn powerbomb_reaction;
  EnemyAiFn touch_ai;
  EnemyAiFn shot_ai;
} EnemyDefAiFns;

/* 24-bit in, canonical 24-bit A0 out; identity if the handler is unique. */
uint32 CanonicalizeEnemyHandler(uint32 ea);

/* Rewrite standard AI VoidP fields to the canonical 16-bit A0 offset. */
void CanonicalizeEnemyDef(struct EnemyDef *ed);

/* Bind 16-bit EnemyDef AI fields to C function pointers. */
void BindEnemyDefAi(struct EnemyDef *ed, EnemyDefAiFns *fns);
const EnemyDefAiFns *GetEnemyDefAiFns(uint16 addr);
void RebindEnemyDefAi(uint16 addr);

/* Leftover unique lookup for hitbox / unbound 16-bit pointers. Not the gameplay path. */
EnemyAiFn EnemyAiFromAddr(uint32 ea);

/* Instruction-list / pre-instr: 16-bit opcode, bank taken from the enemy. */
void EnemyRunPreInstr(uint16 off);
const uint16 *EnemyRunInstr(uint16 opcode, uint16 k, const uint16 *j);

static inline void RunEnemyAiFn(EnemyAiFn fn) {
  if (fn)
    fn();
}

#endif
