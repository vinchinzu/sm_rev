#include "enemy_config.h"
#include "ida_types.h"
#include "../third_party/cJSON.h"
#include "sm_rtl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_ENEMY_OVERRIDES 256

static EnemyDef g_enemy_overrides[MAX_ENEMY_OVERRIDES];
static uint16 g_enemy_override_addrs[MAX_ENEMY_OVERRIDES];
static int g_num_enemy_overrides = 0;
static time_t g_last_enemy_config_time;

EnemyDef *GetEnemyDefOverride(uint16 addr) {
  for (int i = 0; i < g_num_enemy_overrides; i++) {
    if (g_enemy_override_addrs[i] == addr) {
      return &g_enemy_overrides[i];
    }
  }
  return NULL;
}

static uint16 parse_json_hex_or_int(cJSON *obj) {
  if (cJSON_IsNumber(obj)) return (uint16)obj->valueint;
  if (cJSON_IsString(obj)) return (uint16)strtol(obj->valuestring, NULL, 0);
  return 0;
}

void LoadEnemyConfig(void) {
  FILE *f = fopen("sm_enemies.json", "rb");
  if (!f) {
    printf("No sm_enemies.json found, skipping enemy overrides.\n");
    return;
  }

  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *data = malloc(size + 1);
  if (!data) {
    fclose(f);
    return;
  }
  fread(data, 1, size, f);
  data[size] = 0;
  fclose(f);

  cJSON *json = cJSON_Parse(data);
  free(data);
  if (!json) {
    printf("Failed to parse sm_enemies.json\n");
    return;
  }

  cJSON *overrides = cJSON_GetObjectItem(json, "enemy_overrides");
  if (cJSON_IsArray(overrides)) {
    g_num_enemy_overrides = 0;
    cJSON *item;
    cJSON_ArrayForEach(item, overrides) {
      if (g_num_enemy_overrides >= MAX_ENEMY_OVERRIDES) break;

      cJSON *addr_obj = cJSON_GetObjectItem(item, "address");
      if (!addr_obj) continue;

      uint16 addr = parse_json_hex_or_int(addr_obj);
      if (addr == 0) continue;
      
      const uint8 *rom_ptr = RomPtr(0xA00000 | addr);
      EnemyDef *target = &g_enemy_overrides[g_num_enemy_overrides];
      memcpy(target, rom_ptr, sizeof(EnemyDef));
      g_enemy_override_addrs[g_num_enemy_overrides] = addr;

      cJSON *health = cJSON_GetObjectItem(item, "health");
      if (health) target->health = parse_json_hex_or_int(health);

      cJSON *damage = cJSON_GetObjectItem(item, "damage");
      if (damage) target->damage = parse_json_hex_or_int(damage);

      cJSON *x_radius = cJSON_GetObjectItem(item, "x_radius");
      if (x_radius) target->x_radius = parse_json_hex_or_int(x_radius);

      cJSON *y_radius = cJSON_GetObjectItem(item, "y_radius");
      if (y_radius) target->y_radius = parse_json_hex_or_int(y_radius);

      cJSON *main_ai = cJSON_GetObjectItem(item, "main_ai");
      if (main_ai) target->main_ai = parse_json_hex_or_int(main_ai);

      cJSON *hurt_ai = cJSON_GetObjectItem(item, "hurt_ai");
      if (hurt_ai) target->hurt_ai = parse_json_hex_or_int(hurt_ai);

      printf("Applied enemy override for 0x%04X (Health: %u, Damage: %u)\n", 
             addr, target->health, target->damage);
      
      g_num_enemy_overrides++;
    }
  }

  cJSON_Delete(json);

  struct stat st;
  if (stat("sm_enemies.json", &st) == 0)
    g_last_enemy_config_time = st.st_mtime;
}

void CheckEnemyConfigReload(void) {
  struct stat st;
  if (stat("sm_enemies.json", &st) == 0) {
    if (g_last_enemy_config_time == 0) {
      g_last_enemy_config_time = st.st_mtime;
    } else if (st.st_mtime > g_last_enemy_config_time) {
      g_last_enemy_config_time = st.st_mtime;
      printf("Reloading enemy configuration...\n");
      LoadEnemyConfig();
    }
  }
}
