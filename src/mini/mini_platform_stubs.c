#include "sm_rtl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "physics_config.h"
#include "types.h"
#include "variables.h"

uint8 g_ram[0x20000];

bool g_use_my_apu_code;
bool g_debug_flag;
int snes_frame_counter;
SpcPlayer *g_spc_player;
uint16 currently_installed_bug_fix_counter;

void RtlApuWrite(uint32 adr, uint8 val) {
  (void)adr;
  (void)val;
}

void RtlApuUpload(const uint8 *p) {
  (void)p;
}

void RtlWriteSram(void) {
}

void RtlReadSram(void) {
}

void Call(uint32 addr) {
  (void)addr;
  Die("mini: unsupported raw ASM trampoline in Landing Site runtime\n");
}

void MemCpy(void *dst, const void *src, int size) {
  memcpy(dst, src, size);
}

void RtlApplyPhysicsParams(void) {
  samus_y_accel = g_physics_params.gravity_accel;
  samus_y_subaccel = g_physics_params.gravity_subaccel;
}

void mov24(struct LongPtr *dst, uint32 src) {
  dst->addr = src;
  dst->bank = src >> 16;
}

uint32 Load24(const LongPtr *src) {
  return src->addr | (src->bank << 16);
}

PairU16 MakePairU16(uint16 k, uint16 j) {
  PairU16 pair = { .k = k, .j = j };
  return pair;
}

const uint8 *RomPtr(uint32_t addr) {
  return &g_rom[(((addr >> 16) << 15) | (addr & 0x7fff)) & 0x3fffff];
}

uint16 Mult8x8(uint8 a, uint8 b) {
  return a * b;
}

uint16 SnesDivide(uint16 a, uint8 b) {
  return (b == 0) ? 0xffff : a / b;
}

uint16 SnesModulus(uint16 a, uint8 b) {
  return (b == 0) ? a : a % b;
}

bool Unreachable(void) {
  Die("Unreachable\n");
}

NORETURN void Die(const char *error) {
  fprintf(stderr, "%s", error);
  abort();
}

void Warning(const char *error) {
  fprintf(stderr, "%s", error);
}
