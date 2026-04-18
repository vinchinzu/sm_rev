#include "physics_config.h"
#include "variables_extra.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#include "sm_rtl.h"

// Env index order for flat JSON names: [0] "" (air / default), [1] "_underwater", [2] "_lava_acid".
// This string table exists purely so the load/save loops read like a
// single line per field rather than three near-identical copies.
static const char *const kEnvSuffix[3] = { "", "_underwater", "_lava_acid" };

PhysicsParams g_physics_params = {
  // --- Normal jumps (air / water / lava-acid) ---
  .jump_initial_speed         = { 4, 1, 2 },
  .jump_initial_subspeed      = { 0xe000, 0xc000, 0xc000 },
  .jump_hi_initial_speed      = { 6, 2, 3 },
  .jump_hi_initial_subspeed   = { 0x0000, 0x8000, 0x8000 },

  // --- Wall jumps ---
  .wall_jump_initial_speed    = { 4, 0, 2 },
  .wall_jump_initial_subspeed = { 0xa000, 0x4000, 0xa000 },
  .wall_jump_hi_initial_speed    = { 5, 0, 3 },
  .wall_jump_hi_initial_subspeed = { 0x8000, 0x8000, 0x8000 },

  // --- Bomb jumps ---
  .bomb_jump_initial_speed    = { 2, 0, 0 },
  .bomb_jump_initial_subspeed = { 0xc000, 0x1000, 0x1000 },

  // --- Knockback Y ---
  .knockback_y_initial_speed    = { 5, 2, 2 },
  .knockback_y_initial_subspeed = { 0, 0, 0 },

  // --- Horizontal run ---
  .run_accel = 0, .run_accel_sub = 0x00a0,
  .run_decel = 0, .run_decel_sub = 0x0000,
  .run_max_speed = 3, .run_max_speed_sub = 0x0000,

  // --- Gravity ---
  .gravity_accel = 0, .gravity_subaccel = 0x1c00,
  .gravity_underwater_subaccel = 0x0800,
  .gravity_lava_acid_subaccel = 0x0900,
};

static time_t g_last_physics_config_time;

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

// Flat-scalar load/save (for run_* and gravity_* fields).
#define LOAD_FIELD(ctx, name) parse_json_int((ctx), #name, &p->name)
#define SAVE_FIELD(f, name, is_last) \
    fprintf((f), "  \"%s\": %u%s\n", #name, p->name, (is_last) ? "" : ",")

// Per-env array load/save. JSON key is built as "<name><env_suffix>",
// e.g. "jump_initial_speed" / "jump_initial_speed_underwater" /
// "jump_initial_speed_lava_acid". Matches the pre-widening flat naming.
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

void LoadPhysicsConfig(void) {
    FILE *f = fopen("sm_physics.json", "rb");
    if (!f) return;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0) {
        fclose(f);
        return;
    }
    char *buf = malloc(size + 1);
    fread(buf, 1, size, f);
    buf[size] = 0;
    fclose(f);

    PhysicsParams *p = &g_physics_params;

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

    free(buf);
}

void SavePhysicsConfig(void) {
    FILE *f = fopen("sm_physics.json", "wb");
    if (!f) return;
    PhysicsParams *p = &g_physics_params;
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
    if (stat("sm_physics.json", &st) == 0)
      g_last_physics_config_time = st.st_mtime;
}

void CheckPhysicsConfigReload(void) {
    struct stat st;
    if (stat("sm_physics.json", &st) == 0) {
        if (g_last_physics_config_time == 0) {
            g_last_physics_config_time = st.st_mtime;
        } else if (st.st_mtime > g_last_physics_config_time) {
            g_last_physics_config_time = st.st_mtime;
            printf("Reloading physics configuration...\n");
            LoadPhysicsConfig();
            RtlApplyPhysicsParams();
        }
    }
}
