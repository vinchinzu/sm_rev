#include "enemy_config.h"
#include "ida_types.h"
#include "enemy_ai_canon.h"
#include "../third_party/cJSON.h"
#include "sm_rtl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_ENEMY_OVERRIDES 256
#define MAX_ENEMY_DEF_CACHE 256

typedef struct EnemyDefRam {
  EnemyDef def;
  EnemyDefAiFns ai;
} EnemyDefRam;

static EnemyDefRam g_enemy_overrides[MAX_ENEMY_OVERRIDES];
static uint16 g_enemy_override_addrs[MAX_ENEMY_OVERRIDES];
static int g_num_enemy_overrides = 0;
static time_t g_last_enemy_config_time;

static EnemyDefRam g_enemy_def_cache[MAX_ENEMY_DEF_CACHE];
static uint16 g_enemy_def_cache_addrs[MAX_ENEMY_DEF_CACHE];
static int g_num_enemy_def_cache = 0;
static EnemyDefRam g_enemy_def_overflow;
static uint16 g_enemy_def_overflow_addr = 0xffff;

typedef struct EnemySpeciesName {
  const char *name;
  uint16 addr;
} EnemySpeciesName;

static const EnemySpeciesName kEnemySpeciesNames[] = {
  { "roach", 0xD87F },
  { "space_pirate", addr_kEnemyDef_F353 },
  { "spacepirate", addr_kEnemyDef_F353 },
  { "dummy", addr_kEnemyDef_DAFF },
  { "metroid", addr_kEnemyDef_DD7F },
  { "crocomire", addr_kEnemyDef_DDBF },
  { "draygon", addr_kEnemyDef_DE3F },
  { "draygon_eye", addr_kEnemyDef_DE7F },
  { "spore_spawn", addr_kEnemyDef_DF3F },
  { "sporespawn", addr_kEnemyDef_DF3F },
  { "mini_kraid", addr_kEnemyDef_E0FF },
  { "minikraid", addr_kEnemyDef_E0FF },
  { "ridley", addr_kEnemyDef_E17F },
  { "kraid", addr_kEnemyDef_E2BF },
  { "phantoon", addr_kEnemyDef_E4BF },
  { "kago", addr_kEnemyDef_E7FF },
  { "mother_brain_bomb", addr_kEnemyDef_EC3F },
  { "torizo", addr_kEnemyDef_EEFF },
  { "bomb_torizo", addr_kEnemyDef_EEFF },
  { "golden_torizo", addr_kEnemyDef_EEFF },
  { "botwoon", addr_kEnemyDef_F293 },
  { "lower_norfair_space_pirate", addr_kEnemyDef_F593 },
};

static const char *kAiPointerKeys[] = {
  "main_ai", "hurt_ai", "touch_ai", "shot_ai", "grapple_ai", "ai_init",
};

static void FillEnemyDefRam(EnemyDefRam *slot, uint16 addr) {
  memcpy(&slot->def, RomPtr(0xA00000 | addr), sizeof(EnemyDef));
  CanonicalizeEnemyDef(&slot->def);
  BindEnemyDefAi(&slot->def, &slot->ai);
}

EnemyDef *GetEnemyDefOverride(uint16 addr) {
  for (int i = 0; i < g_num_enemy_overrides; i++) {
    if (g_enemy_override_addrs[i] == addr) {
      return &g_enemy_overrides[i].def;
    }
  }
  return NULL;
}

static EnemyDefRam *FindEnemyDefRam(uint16 a) {
  for (int i = 0; i < g_num_enemy_overrides; i++) {
    if (g_enemy_override_addrs[i] == a)
      return &g_enemy_overrides[i];
  }
  for (int i = 0; i < g_num_enemy_def_cache; i++) {
    if (g_enemy_def_cache_addrs[i] == a)
      return &g_enemy_def_cache[i];
  }
  if (g_enemy_def_overflow_addr == a)
    return &g_enemy_def_overflow;
  return NULL;
}

EnemyDef *get_EnemyDef_A2(uint16 a) {
  EnemyDefRam *ram = FindEnemyDefRam(a);
  if (ram)
    return &ram->def;

  if (g_num_enemy_def_cache >= MAX_ENEMY_DEF_CACHE) {
    FillEnemyDefRam(&g_enemy_def_overflow, a);
    g_enemy_def_overflow_addr = a;
    return &g_enemy_def_overflow.def;
  }

  EnemyDefRam *slot = &g_enemy_def_cache[g_num_enemy_def_cache];
  FillEnemyDefRam(slot, a);
  g_enemy_def_cache_addrs[g_num_enemy_def_cache] = a;
  g_num_enemy_def_cache++;
  return &slot->def;
}

const EnemyDefAiFns *GetEnemyDefAiFns(uint16 addr) {
  get_EnemyDef_A2(addr);
  return &FindEnemyDefRam(addr)->ai;
}

void RebindEnemyDefAi(uint16 addr) {
  EnemyDefRam *ram = FindEnemyDefRam(addr);
  if (!ram) {
    get_EnemyDef_A2(addr);
    ram = FindEnemyDefRam(addr);
  }
  if (ram)
    BindEnemyDefAi(&ram->def, &ram->ai);
}

static uint16 parse_json_hex_or_int(cJSON *obj) {
  if (cJSON_IsNumber(obj)) return (uint16)obj->valueint;
  if (cJSON_IsString(obj)) return (uint16)strtol(obj->valuestring, NULL, 0);
  return 0;
}

static uint16 lookup_enemy_species(const char *name) {
  for (size_t i = 0; i < sizeof(kEnemySpeciesNames) / sizeof(kEnemySpeciesNames[0]); i++) {
    if (strcasecmp(name, kEnemySpeciesNames[i].name) == 0)
      return kEnemySpeciesNames[i].addr;
  }
  return 0;
}

static uint16 resolve_enemy_override_addr(cJSON *item) {
  cJSON *species = cJSON_GetObjectItem(item, "species");
  if (cJSON_IsString(species) && species->valuestring) {
    uint16 addr = lookup_enemy_species(species->valuestring);
    if (addr)
      return addr;
    printf("Warning: unknown enemy species '%s', skipping override.\n", species->valuestring);
    return 0;
  }

  cJSON *addr_obj = cJSON_GetObjectItem(item, "address");
  if (!addr_obj)
    addr_obj = cJSON_GetObjectItem(item, "addr");
  if (!addr_obj)
    return 0;
  return parse_json_hex_or_int(addr_obj);
}

static void warn_if_ai_pointer_keys(cJSON *item) {
  for (size_t i = 0; i < sizeof(kAiPointerKeys) / sizeof(kAiPointerKeys[0]); i++) {
    if (cJSON_GetObjectItem(item, kAiPointerKeys[i])) {
      printf("Warning: AI pointers (main_ai, hurt_ai, touch_ai, shot_ai, grapple_ai, ai_init) are not overridable; ignoring.\n");
      return;
    }
  }
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

      uint16 addr = resolve_enemy_override_addr(item);
      if (addr == 0) continue;

      EnemyDefRam *slot = &g_enemy_overrides[g_num_enemy_overrides];
      FillEnemyDefRam(slot, addr);
      EnemyDef *target = &slot->def;
      g_enemy_override_addrs[g_num_enemy_overrides] = addr;

      cJSON *health = cJSON_GetObjectItem(item, "health");
      if (health) target->health = parse_json_hex_or_int(health);

      cJSON *damage = cJSON_GetObjectItem(item, "damage");
      if (damage) target->damage = parse_json_hex_or_int(damage);

      cJSON *x_radius = cJSON_GetObjectItem(item, "x_radius");
      if (x_radius) target->x_radius = parse_json_hex_or_int(x_radius);

      cJSON *y_radius = cJSON_GetObjectItem(item, "y_radius");
      if (y_radius) target->y_radius = parse_json_hex_or_int(y_radius);

      warn_if_ai_pointer_keys(item);

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
