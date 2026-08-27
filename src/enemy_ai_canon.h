#ifndef ENEMY_AI_CANON_H
#define ENEMY_AI_CANON_H

#include "types.h"

struct EnemyDef;

/* 24-bit in, canonical 24-bit A0 out; identity if the handler is unique. */
uint32 CanonicalizeEnemyHandler(uint32 ea);

/* Rewrite standard AI VoidP fields to the canonical 16-bit A0 offset. */
void CanonicalizeEnemyDef(struct EnemyDef *ed);

#endif
