// Physics configuration pipeline.
//
//   sm_physics.json (base)  ──┐
//                             ├─► PhysicsConfig_RecomputeEffective ─► g_physics_params
//   sm_mods.json (scale %)  ──┘                                        ▲
//                                                                      │
//                                          consumers (samus_jump.c,    │
//                                          samus_speed.c, sm_rtl.c)    │
//                                          read this directly ─────────┘
//
// All Samus-physics consumers read g_physics_params. They do NOT know about
// mods — the mod knobs are collapsed into the effective values at load time.
// To disable all mods: delete sm_mods.json or set every *_scale_percent back
// to 100. To edit vanilla physics: edit sm_physics.json (or delete it to
// revert to compile-time defaults). Hot-reload watches both files.

#include "physics_config.h"
#include "variables_extra.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#include "sm_rtl.h"

static const char *const kEnvSuffix[3] = { "", "_underwater", "_lava_acid" };

// --- Vanilla base values -----------------------------------------------------
// The single source of truth for "what Super Metroid originally used". Both
// g_physics_base and the initial g_physics_params are seeded from this at
// startup (via the shared _LITERAL macro — duplicating literal inits is the
// one clean way to compile-time-initialize two distinct globals from the same
// field layout in C).
#define PHYSICS_BASE_DEFAULTS_LITERAL { \
  .jump_initial_speed         = { 4, 1, 2 }, \
  .jump_initial_subspeed      = { 0xe000, 0xc000, 0xc000 }, \
  .jump_hi_initial_speed      = { 6, 2, 3 }, \
  .jump_hi_initial_subspeed   = { 0x0000, 0x8000, 0x8000 }, \
  .wall_jump_initial_speed    = { 4, 0, 2 }, \
  .wall_jump_initial_subspeed = { 0xa000, 0x4000, 0xa000 }, \
  .wall_jump_hi_initial_speed    = { 5, 0, 3 }, \
  .wall_jump_hi_initial_subspeed = { 0x8000, 0x8000, 0x8000 }, \
  .bomb_jump_initial_speed    = { 2, 0, 0 }, \
  .bomb_jump_initial_subspeed = { 0xc000, 0x1000, 0x1000 }, \
  .knockback_y_initial_speed    = { 5, 2, 2 }, \
  .knockback_y_initial_subspeed = { 0, 0, 0 }, \
  .run_accel = 0, .run_accel_sub = 0x00a0, \
  .run_decel = 0, .run_decel_sub = 0x0000, \
  .run_max_speed = 3, .run_max_speed_sub = 0x0000, \
  .gravity_accel = 0, .gravity_subaccel = 0x1c00, \
  .gravity_underwater_subaccel = 0x0800, \
  .gravity_lava_acid_subaccel = 0x0900, \
}

// Every mod knob defaults to 100% (no-op). Missing sm_mods.json ⇒ vanilla.
#define PHYSICS_MODS_DEFAULTS_LITERAL { \
  .gravity_scale_percent   = 100, \
  .run_speed_scale_percent = 100, \
  .jump_scale_percent      = 100, \
}

static const PhysicsParams kPhysicsBaseDefaults = PHYSICS_BASE_DEFAULTS_LITERAL;
static const PhysicsMods   kPhysicsModsDefaults = PHYSICS_MODS_DEFAULTS_LITERAL;

// Private pre-mods snapshot. Consumers cannot read this — they must go
// through g_physics_params, which is base × mods.
static PhysicsParams g_physics_base    = PHYSICS_BASE_DEFAULTS_LITERAL;

// Public: effective values (base × mods). What the physics engine reads.
// Compile-time initialized to vanilla so anything that touches physics
// before LoadPhysicsConfig runs sees sane values.
PhysicsParams g_physics_params = PHYSICS_BASE_DEFAULTS_LITERAL;

// Public: mod knobs. Filled by the loader; vanilla (no-op) until then.
PhysicsMods   g_physics_mods   = PHYSICS_MODS_DEFAULTS_LITERAL;

static time_t g_last_physics_mtime;
static time_t g_last_mods_mtime;

// --- JSON helpers ------------------------------------------------------------
static bool parse_json_int(const char *json, const char *key, uint16 *out_val) {
   char search[128];
   snprintf(search, sizeof(search), "\"%s\"", key);
   const char *p = strstr(json, search);
   if (!p) return false;
   p += strlen(search);
   while(*p == ' ' || *p == ':' || *p == '\t' || *p == '\n' || *p == '\r') p++;
   char *endptr;
   long val = strtol(p, &endptr, 10);
   if (p != endptr) {
       *out_val = (uint16)val;
       return true;
   }
   return false;
}

#define LOAD_FIELD(ctx, name) parse_json_int((ctx), #name, &p->name)
#define SAVE_FIELD(f, name, is_last) \
    fprintf((f), "  \"%s\": %u%s\n", #name, p->name, (is_last) ? "" : ",")

static void load_env_field(const char *buf, const char *name, uint16 arr[3]) {
    char key[128];
    for (int i = 0; i < 3; i++) {
        snprintf(key, sizeof(key), "%s%s", name, kEnvSuffix[i]);
        parse_json_int(buf, key, &arr[i]);
    }
}
static void save_env_field(FILE *f, const char *name, const uint16 arr[3]) {
    for (int i = 0; i < 3; i++)
        fprintf(f, "  \"%s%s\": %u,\n", name, kEnvSuffix[i], arr[i]);
}
#define LOAD_ENV(ctx, name) load_env_field((ctx), #name, p->name)
#define SAVE_ENV(f, name)   save_env_field((f),   #name, p->name)

// Read whole file into a NUL-terminated heap buffer. Returns NULL on missing
// or empty file. Caller frees.
static char *slurp_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0) { fclose(f); return NULL; }
    char *buf = malloc(size + 1);
    fread(buf, 1, size, f);
    buf[size] = 0;
    fclose(f);
    return buf;
}

// --- Mods: scale-pair math ---------------------------------------------------
// Fixed-point (whole:sub) pair is treated as a single 32-bit value for
// scaling, so percentages above 100 don't drop carries into the whole half.
static void scale_pair(uint16 *whole, uint16 *sub, uint16 pct) {
    if (pct == 100) return;
    uint32 full = ((uint32)*whole << 16) | *sub;
    full = (full * pct) / 100;
    *whole = (uint16)(full >> 16);
    *sub   = (uint16)full;
}

// Sub-only scalar (used for gravity subaccels where the whole-pixel field is
// separate and shared across env variants).
static void scale_sub(uint16 *sub, uint16 pct) {
    if (pct == 100) return;
    *sub = (uint16)(((uint32)*sub * pct) / 100);
}

// Apply mods to the effective table. The base is copied in first; every mod
// then mutates specific g_physics_params fields. Adding a new knob = adding
// one line here and one field in PhysicsMods — no physics consumers change.
static void PhysicsConfig_RecomputeEffective(void) {
    g_physics_params = g_physics_base;
    const PhysicsMods *m = &g_physics_mods;

    // Gravity: scale the (accel, subaccel) pair plus the two liquid-env
    // subaccel flavors. They share the same whole-pixel gravity_accel.
    scale_pair(&g_physics_params.gravity_accel,
               &g_physics_params.gravity_subaccel, m->gravity_scale_percent);
    scale_sub(&g_physics_params.gravity_underwater_subaccel, m->gravity_scale_percent);
    scale_sub(&g_physics_params.gravity_lava_acid_subaccel,  m->gravity_scale_percent);

    // Run speed: scale accel and max speed (pairs). Leave decel alone so
    // braking still feels vanilla.
    scale_pair(&g_physics_params.run_accel,     &g_physics_params.run_accel_sub,
               m->run_speed_scale_percent);
    scale_pair(&g_physics_params.run_max_speed, &g_physics_params.run_max_speed_sub,
               m->run_speed_scale_percent);

    // Jump impulses: every env index of every jump flavor. Knockback left
    // untouched (damage feel is a combat concern, not a movement mod).
    for (int env = 0; env < 3; env++) {
      scale_pair(&g_physics_params.jump_initial_speed[env],
                 &g_physics_params.jump_initial_subspeed[env], m->jump_scale_percent);
      scale_pair(&g_physics_params.jump_hi_initial_speed[env],
                 &g_physics_params.jump_hi_initial_subspeed[env], m->jump_scale_percent);
      scale_pair(&g_physics_params.wall_jump_initial_speed[env],
                 &g_physics_params.wall_jump_initial_subspeed[env], m->jump_scale_percent);
      scale_pair(&g_physics_params.wall_jump_hi_initial_speed[env],
                 &g_physics_params.wall_jump_hi_initial_subspeed[env], m->jump_scale_percent);
      scale_pair(&g_physics_params.bomb_jump_initial_speed[env],
                 &g_physics_params.bomb_jump_initial_subspeed[env], m->jump_scale_percent);
    }
}

// --- Base file (sm_physics.json) load/save -----------------------------------
static void load_base_from_buf(const char *buf) {
    PhysicsParams *p = &g_physics_base;
    LOAD_ENV(buf, jump_initial_speed);
    LOAD_ENV(buf, jump_initial_subspeed);
    LOAD_ENV(buf, jump_hi_initial_speed);
    LOAD_ENV(buf, jump_hi_initial_subspeed);
    LOAD_ENV(buf, wall_jump_initial_speed);
    LOAD_ENV(buf, wall_jump_initial_subspeed);
    LOAD_ENV(buf, wall_jump_hi_initial_speed);
    LOAD_ENV(buf, wall_jump_hi_initial_subspeed);
    LOAD_ENV(buf, bomb_jump_initial_speed);
    LOAD_ENV(buf, bomb_jump_initial_subspeed);
    LOAD_ENV(buf, knockback_y_initial_speed);
    LOAD_ENV(buf, knockback_y_initial_subspeed);

    LOAD_FIELD(buf, run_accel);
    LOAD_FIELD(buf, run_accel_sub);
    LOAD_FIELD(buf, run_decel);
    LOAD_FIELD(buf, run_decel_sub);
    LOAD_FIELD(buf, run_max_speed);
    LOAD_FIELD(buf, run_max_speed_sub);
    LOAD_FIELD(buf, gravity_accel);
    LOAD_FIELD(buf, gravity_subaccel);
    LOAD_FIELD(buf, gravity_underwater_subaccel);
    LOAD_FIELD(buf, gravity_lava_acid_subaccel);
}

static void load_mods_from_buf(const char *buf) {
    PhysicsMods *p = &g_physics_mods;
    LOAD_FIELD(buf, gravity_scale_percent);
    LOAD_FIELD(buf, run_speed_scale_percent);
    LOAD_FIELD(buf, jump_scale_percent);
}

void LoadPhysicsConfig(void) {
    // Reset to defaults first so deleting a key from JSON restores the
    // vanilla value instead of keeping the stale prior-loaded override.
    g_physics_base = kPhysicsBaseDefaults;
    g_physics_mods = kPhysicsModsDefaults;

    char *buf = slurp_file("sm_physics.json");
    if (buf) { load_base_from_buf(buf); free(buf); }

    buf = slurp_file("sm_mods.json");
    if (buf) { load_mods_from_buf(buf); free(buf); }

    PhysicsConfig_RecomputeEffective();
}

void SavePhysicsConfig(void) {
    // --- Base ---
    FILE *f = fopen("sm_physics.json", "wb");
    if (f) {
      PhysicsParams *p = &g_physics_base;
      fprintf(f, "{\n");
      SAVE_ENV(f, jump_initial_speed);
      SAVE_ENV(f, jump_initial_subspeed);
      SAVE_ENV(f, jump_hi_initial_speed);
      SAVE_ENV(f, jump_hi_initial_subspeed);
      SAVE_ENV(f, wall_jump_initial_speed);
      SAVE_ENV(f, wall_jump_initial_subspeed);
      SAVE_ENV(f, wall_jump_hi_initial_speed);
      SAVE_ENV(f, wall_jump_hi_initial_subspeed);
      SAVE_ENV(f, bomb_jump_initial_speed);
      SAVE_ENV(f, bomb_jump_initial_subspeed);
      SAVE_ENV(f, knockback_y_initial_speed);
      SAVE_ENV(f, knockback_y_initial_subspeed);
      SAVE_FIELD(f, run_accel, false);
      SAVE_FIELD(f, run_accel_sub, false);
      SAVE_FIELD(f, run_decel, false);
      SAVE_FIELD(f, run_decel_sub, false);
      SAVE_FIELD(f, run_max_speed, false);
      SAVE_FIELD(f, run_max_speed_sub, false);
      SAVE_FIELD(f, gravity_accel, false);
      SAVE_FIELD(f, gravity_subaccel, false);
      SAVE_FIELD(f, gravity_underwater_subaccel, false);
      SAVE_FIELD(f, gravity_lava_acid_subaccel, true);
      fprintf(f, "}\n");
      fclose(f);
      struct stat st;
      if (stat("sm_physics.json", &st) == 0) g_last_physics_mtime = st.st_mtime;
    }

    // --- Mods ---
    f = fopen("sm_mods.json", "wb");
    if (f) {
      PhysicsMods *p = &g_physics_mods;
      fprintf(f, "{\n");
      SAVE_FIELD(f, gravity_scale_percent, false);
      SAVE_FIELD(f, run_speed_scale_percent, false);
      SAVE_FIELD(f, jump_scale_percent, true);
      fprintf(f, "}\n");
      fclose(f);
      struct stat st;
      if (stat("sm_mods.json", &st) == 0) g_last_mods_mtime = st.st_mtime;
    }
}

void CheckPhysicsConfigReload(void) {
    bool reload = false;
    struct stat st;

    if (stat("sm_physics.json", &st) == 0) {
        if (g_last_physics_mtime == 0) g_last_physics_mtime = st.st_mtime;
        else if (st.st_mtime > g_last_physics_mtime) {
            g_last_physics_mtime = st.st_mtime;
            reload = true;
        }
    }
    if (stat("sm_mods.json", &st) == 0) {
        if (g_last_mods_mtime == 0) g_last_mods_mtime = st.st_mtime;
        else if (st.st_mtime > g_last_mods_mtime) {
            g_last_mods_mtime = st.st_mtime;
            reload = true;
        }
    }

    if (reload) {
        printf("Reloading physics configuration...\n");
        LoadPhysicsConfig();
        RtlApplyPhysicsParams();
    }
}
