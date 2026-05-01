# Torizo Mods

`sm_torizo.json` is an optional runtime mod file. If the file is missing or
cannot be parsed, the code uses vanilla Torizo behavior. Missing keys are
allowed. Fields with the wrong JSON type or unknown string values are ignored
and keep their default value.

The preferred schema is grouped into `chozo_orbs`, `attack_order`, and
`finish_explosion`. Legacy root-level aliases are still accepted for the same
fields; if both grouped and root keys are present, the root key is read last and
wins.

## Chozo Orbs

- `chozo_orbs.spawn_multiplier`: multiplies the vanilla burst count when a
  direct burst count is not set. Vanilla is `1`; values are clamped to
  `0..max_per_burst` before use.
- `chozo_orbs.contact_damage_multiplier`: multiplies each orb's vanilla contact
  damage. Vanilla is `1`; the multiplier is clamped to `0..64`, and final
  projectile damage is clamped to `0..0xfff`.
- `chozo_orbs.palette`: orb sprite palette. Use `vanilla`, `blue`, or a numeric
  OAM palette value. Numeric `0..7` is treated as a palette index and shifted
  into OAM palette bits; larger numbers are masked with `0x0e00`. The built-in
  `blue` preset writes a blue palette into enemy sprite palette 7 so it does not
  recolor Samus shots.
- `chozo_orbs.freeze_samus_frames`: frames to pin Samus in place after an orb
  hit. Vanilla is `0`; values are clamped to `0..600`. The same clamped value
  also extends Samus knockback if it is larger than the current knockback timer.
- `chozo_orbs.spawn_jitter_pixels`: random X/Y spread applied as each orb
  spawns. Vanilla is `0`; values are clamped to `0..64`.
- `chozo_orbs.bomb_burst_count`: direct Bomb Torizo burst count. Any negative
  value uses `3 * spawn_multiplier`; non-negative values are clamped to
  `0..max_per_burst`.
- `chozo_orbs.golden_burst_count`: direct Golden Torizo burst count. Any
  negative value uses `1 * spawn_multiplier`; non-negative values are clamped to
  `0..max_per_burst`.
- `chozo_orbs.max_per_burst`: safety clamp for one Chozo orb burst. Values are
  clamped to `0..18`, matching the enemy-projectile table's 18 slots.

Legacy root aliases: `chozo_orb_spawn_multiplier`,
`chozo_orb_contact_damage_multiplier`, `chozo_orb_palette`,
`chozo_orb_freeze_samus_frames`, `chozo_orb_spawn_jitter_pixels`,
`bomb_chozo_orb_burst_count`, `golden_chozo_orb_burst_count`,
`max_chozo_orbs_per_burst`.

## Attack Order

- `attack_order.bomb_opening_attack`: Bomb Torizo's first active attack. Use
  `vanilla` for original behavior or `chozo_orbs` to open with the orb spray.
  Numeric `0` and `1` are also accepted for vanilla and Chozo orbs. Other
  numbers do not select the custom attack.
- `attack_order.bomb_opening_chozo_orb_burst_count`: direct count for each
  burst in the opening orb spray. Any negative value uses the normal Bomb
  Torizo orb burst count; non-negative values are clamped to `0..max_per_burst`.
- `attack_order.bomb_opening_chozo_orb_wave_count`: if set to `0` or greater,
  replaces the ROM list's three orb-spawn calls with scheduled opening waves.
  Values are clamped to `0..64`; `0` enables the scheduled path but schedules no
  waves.
- `attack_order.bomb_opening_chozo_orb_wave_interval_frames`: frame delay
  between scheduled opening waves. Values are clamped to `1..255`.

The current opening config uses five scheduled waves of 18 blue orbs. That is
90 spawn attempts, or 10x the vanilla opening spray total of 9, while still
respecting the 18 enemy-projectile slots available at one time.

Legacy root aliases: `bomb_opening_attack`,
`bomb_opening_chozo_orb_burst_count`,
`bomb_opening_chozo_orb_wave_count`,
`bomb_opening_chozo_orb_wave_interval_frames`.

## Finish Explosion

- `finish_explosion.body_burst_multiplier`: multiplies the vanilla body burst
  count when a direct body count is not set. Vanilla is `1`; the multiplier is
  clamped to `0..max_per_burst`.
- `finish_explosion.body_burst_count`: direct body burst count. Any negative
  value uses `6 * body_burst_multiplier`; non-negative values are clamped to
  `0..max_per_burst`.
- `finish_explosion.final_burst_multiplier`: multiplies the vanilla final
  centered burst count when a direct final count is not set. Vanilla is `1`; the
  multiplier is clamped to `0..max_per_burst`.
- `finish_explosion.final_burst_count`: direct final centered burst count. Any
  negative value uses `1 * final_burst_multiplier`; non-negative values are
  clamped to `0..max_per_burst`.
- `finish_explosion.flash_frames`: frames to hold each body burst. Vanilla is
  `40`; values are clamped to `1..1024`.
- `finish_explosion.position_jitter_pixels`: random X/Y spread applied to finish
  explosion projectiles. Vanilla is `0`; values are clamped to `0..64`.
- `finish_explosion.max_per_burst`: safety clamp for one finish-explosion burst.
  Values are clamped to `0..18`, matching the enemy-projectile table's 18 slots.

Legacy root aliases: `finish_explosion_body_burst_multiplier`,
`finish_explosion_body_burst_count`,
`finish_explosion_final_burst_multiplier`,
`finish_explosion_final_burst_count`, `finish_explosion_flash_frames`,
`finish_explosion_position_jitter_pixels`,
`max_finish_explosions_per_burst`.

## Save Fixtures

Reusable boss regression save states live under `saves/boss/`. The current
Torizo fixture is `saves/boss/torizo_pre_fight.sav`, which starts in the Bomb
Torizo room and is used by `tests/test_torizo_mods.py` to advance to the first
blue-orb opening wave deterministically.

Top-level `saves/save*.sav`, `saves/bug-*.sav`, `.srm`, and ad hoc named saves
are treated as local iteration artifacts unless a test points at a canonical
fixture path.

## Vanilla Config

To restore original behavior, delete `sm_torizo.json` or set:

```json
{
  "chozo_orbs": {
    "spawn_multiplier": 1,
    "contact_damage_multiplier": 1,
    "palette": "vanilla",
    "freeze_samus_frames": 0,
    "spawn_jitter_pixels": 0,
    "bomb_burst_count": -1,
    "golden_burst_count": -1,
    "max_per_burst": 18
  },
  "attack_order": {
    "bomb_opening_attack": "vanilla",
    "bomb_opening_chozo_orb_burst_count": -1,
    "bomb_opening_chozo_orb_wave_count": -1,
    "bomb_opening_chozo_orb_wave_interval_frames": 16
  },
  "finish_explosion": {
    "body_burst_multiplier": 1,
    "body_burst_count": -1,
    "final_burst_multiplier": 1,
    "final_burst_count": -1,
    "flash_frames": 40,
    "position_jitter_pixels": 0,
    "max_per_burst": 18
  }
}
```
