# Source Layout

This repo is now organized by subsystem first and original ROM bank second.
Keep file moves small and mechanical so full-build parity remains easy to verify.

## Current Folders

- `src/`: shared gameplay and runtime modules. Most files stay flat here while
  they are still cross-linked by generated headers and legacy globals.
- `src/block_reaction.h`: shared block material, tile-attribute, and slope-BTS
  constants used by Samus collision, projectile collision, mini room import, and
  mini collision rendering.
- `src/host/`: desktop host-only rendering code. This is excluded from `mini`
  and is a safe place for SDL/OpenGL front-end code that should not leak into
  the gameplay kernel.
- `src/mini/`: mini host, deterministic replay/rollback seams, Landing Site
  content scope, editor bridge, and mini-owned shims.
- `src/snes/`: emulator hardware layer. Treat this as infrastructure unless a
  portability task directly targets it.
- `third_party/`: vendored dependencies.

The Makefile mirrors those categories with `CORE_SRCS`, `HOST_SRCS`,
`SNES_SRCS`, and `MINI_RUNTIME_SRCS`.

## Naming Rules

- Samus behavior stays in `samus_*.c`.
- Enemy behavior stays in `enemy_*.c`.
- Room load, scrolling, and FX stay in `room_*.c`.
- PLM behavior stays in `plm_*.c`.
- Enemy projectiles stay in `eproj_*.c`.
- HDMA runtime families use descriptive names such as `room_fx_hdma.c` and
  `boss_hdma.c`.

Avoid generic folders like `core/`, `common/`, or `misc/`. A new folder should
mean a real dependency boundary, not just fewer files in a directory listing.

## Move Policy

Move files only when the new location makes ownership clearer and does not hide
gameplay dependencies.

Good candidates:
- host-only code with no gameplay state dependency
- mini-only shims or tests
- portable front-end shells around a stable C gameplay API

Poor candidates:
- Samus/room/enemy files still sharing `variables.h` and `funcs.h`
- files whose only relationship is original ROM bank number
- broad moves that require behavior refactors in the same patch

When moving a gameplay file, update:
- `Makefile`
- [bank_origin_map.md](bank_origin_map.md), if original-bank lookup changes
- [coverage.md](coverage.md), if the structural audit would become misleading
- the smallest relevant build or runtime test

## Portability Direction

The desired portable shape is:

1. thin replaceable host layer
2. deterministic mini gameplay API
3. named mod/config surfaces around movement, collision, room content, and boss
   tuning
4. full-build parity adapters around the same shared modules

Keep the gameplay kernel C and deterministic before porting host orchestration or
rollback shells to another language.
