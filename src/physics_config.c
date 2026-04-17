#include "physics_config.h"
#include "variables_extra.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#include "sm_rtl.h"

PhysicsParams g_physics_params = {
  .jump_initial_speed = 4, .jump_initial_subspeed = 0xe000, 
  .jump_hi_initial_speed = 6, .jump_hi_initial_subspeed = 0x0000,
  .jump_underwater_initial_speed = 1, .jump_underwater_initial_subspeed = 0xc000,
  .jump_hi_underwater_initial_speed = 2, .jump_hi_underwater_initial_subspeed = 0x8000,
  .jump_lava_acid_initial_speed = 2, .jump_lava_acid_initial_subspeed = 0xc000,
  .jump_hi_lava_acid_initial_speed = 3, .jump_hi_lava_acid_initial_subspeed = 0x8000,
  .run_accel = 0, .run_accel_sub = 0x00a0,
  .run_decel = 0, .run_decel_sub = 0x0000,
  .run_max_speed = 3, .run_max_speed_sub = 0x0000,
  .gravity_accel = 0, .gravity_subaccel = 0x1c00,
  .gravity_underwater_subaccel = 0x0800,
  .gravity_lava_acid_subaccel = 0x0900
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

#define LOAD_FIELD(ctx, name) parse_json_int((ctx), #name, &p->name)
#define SAVE_FIELD(f, name, is_last) fprintf((f), "  \"%s\": %u%s\n", #name, p->name, (is_last) ? "" : ",")

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
    LOAD_FIELD(buf, jump_initial_speed);
    LOAD_FIELD(buf, jump_initial_subspeed);
    LOAD_FIELD(buf, jump_hi_initial_speed);
    LOAD_FIELD(buf, jump_hi_initial_subspeed);
    LOAD_FIELD(buf, jump_underwater_initial_speed);
    LOAD_FIELD(buf, jump_underwater_initial_subspeed);
    LOAD_FIELD(buf, jump_hi_underwater_initial_speed);
    LOAD_FIELD(buf, jump_hi_underwater_initial_subspeed);
    LOAD_FIELD(buf, jump_lava_acid_initial_speed);
    LOAD_FIELD(buf, jump_lava_acid_initial_subspeed);
    LOAD_FIELD(buf, jump_hi_lava_acid_initial_speed);
    LOAD_FIELD(buf, jump_hi_lava_acid_initial_subspeed);
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
    SAVE_FIELD(f, jump_initial_speed, false);
    SAVE_FIELD(f, jump_initial_subspeed, false);
    SAVE_FIELD(f, jump_hi_initial_speed, false);
    SAVE_FIELD(f, jump_hi_initial_subspeed, false);
    SAVE_FIELD(f, jump_underwater_initial_speed, false);
    SAVE_FIELD(f, jump_underwater_initial_subspeed, false);
    SAVE_FIELD(f, jump_hi_underwater_initial_speed, false);
    SAVE_FIELD(f, jump_hi_underwater_initial_subspeed, false);
    SAVE_FIELD(f, jump_lava_acid_initial_speed, false);
    SAVE_FIELD(f, jump_lava_acid_initial_subspeed, false);
    SAVE_FIELD(f, jump_hi_lava_acid_initial_speed, false);
    SAVE_FIELD(f, jump_hi_lava_acid_initial_subspeed, false);
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
