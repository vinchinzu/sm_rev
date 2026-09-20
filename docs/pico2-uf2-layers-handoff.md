# Handoff: pico2 Landing Site — gunship restored + Samus animation (flashed)

> **STALE IN PLACES — read the source first.** Written before commit `f44cf8e`.
> Since then `sm_rev-k5q.3` (real Landing Site collision packed into mini),
> `sm_rev-k5q.6` (ROM bank 0x91 packed; both stand-ins retired) and
> `sm_rev-k5q.7` (single-core raster wins) have landed. Where this file and
> the tree disagree, the tree is right. Kept for the standing facts —
> board serials, the `0x0338` air-word isolation trick, the
> never-use-`explorer_st7789_test.c` warning — and for the `k5q.4` / `k5q.5`
> investigations, which are still open.


**Date:** 2026-09-19
**Workspace:** `/home/v/01_projects/11_games/snes_editor/super_metroid_rl/sm_rev`
**Supersedes** the 13:26 handoff (packed pose-1, gunship unhooked).

Nothing is committed. Everything below is staged in the working tree.

---

## Standing facts (do not re-prove)

Board **`3973D48FD625B2E8`**, Explorer ST7789 240x240. Leave **`C89554A4009B3D21`** unplugged — `flash.sh` refuses to run if it is present.

**Flashing needs no BOOTSEL and no unplug.** `src/pico/rp2350/flash.sh` uses `picotool load -f` via the SDK USB vendor reset. An agent can flash directly:

```
src/pico/rp2350/flash.sh build/pico2/sm_rev_pico2.uf2
```

CDC is `/dev/ttyACM0`. It prints one `step=/raster=/spi=` line per 30 frames; at ~7 fps allow ~20s of capture before concluding anything.

Isolate a BG layer by filling the other map with **air word `0x0338`**. Zeroing a map draws opaque **tile 0**.

Never use `explorer_st7789_test.c` settings (rowstart=80, GP20-as-DC) or CircuitPython's. Keep window (0,0,240,240) and GP20 PWM.

`sm_rev-ul9` (MADCTL SWAP_XY / rotation) stays a **no-op** — sky is on top and orientation matches the composite.

---

## On the board right now

`build/pico2/sm_rev_pico2.uf2`, ELF text **439756** / bss **292356**. Flashed and confirmed:

```
pico2 step=48 pack=60 raster=87646 spi=47614 present=135261 frame=135370 us
x=80 y=160 pose=1 af=3 joy=0000 btn=0
```

`af` cycling 0..3 is the idle stand animation running. Earlier images (501760 B pose-1, 12:47 gunship, 12:32 layers-only) are all obsolete.

`out/pico_ls_composite.ppm` is now rendered at the **exact device boot framing** (`cam=1025,1008 scroll=1,0 samus origin=128,74`), so glass and PPM are directly comparable.

---

## Landed this session

- **MADCTL colour order.** `MADCTL_RGB` (0x08) is D3, and **D3=1 selects BGR**. The macro name was backwards: we send RGB565 and were telling the panel BGR. D3 is now clear in `st7789_explorer.c`. The visor is pure green either way, so the visor is the control if this needs re-checking.
- **Gunship restored.** `pack_gunship()` re-enabled → `src/pico/assets/pico_ls_gunship.inc`. 56 OAM entries at slots 64..119, Samus 0..63; lower slot draws on top. Priority 3 → 2 so OAM index decides overlap. `pico_oam_gunship.c` is now in the Makefile and CMakeLists — it previously linked nowhere and had no data behind its `extern`s.
  **Why it matters:** the spawn floor is blocks `0x80ff` — solid collision, metatile `0xFF`, *air graphics*. The ship is drawn entirely by sprites. Without it Samus stands on nothing. The old pink square was standing in for the floor, not for decoration.
- **World-space re-basing** in `pico2_main.c` (see k5q.3 below for why it is a workaround).
- **Animation.** 109 frames / 26 poses (111,616 B CHR, 3933 B spritemaps): stand, run L/R, run-with-gun, turn, jump, spin, crouch, morphball, crouch/stand transitions. Frame counts are **derived** — a pose's frame list ends at the next pose's pointer in the bank92 table — not guessed. CHR is re-DMAd to the OBJ name base whenever the frame index changes, as `NmiTransferSamusToVram` does.
- **`kPicoLsRoomSpawnX/Y`** public in `pico_ls_assets.h`, `_Static_assert`ed against the packed values.
- **Packer validation** moved ahead of both writes, so a bad camera no longer overwrites checked-in `.inc` files and *then* fails.
- **Stack.** `present_frame`'s two line buffers are `static` (~1KB off the 2KiB stack; bss rose exactly 992 B confirming it).
- **Boot splash** is black, not magenta — a `MiniCreate` hang no longer looks like the old build.
- **Side trim** verified, not changed: 8 columns each side, 8 rows letterboxed, no scale. Already centred.

Host green: `magenta=0`, gunship cluster `n=3738`, run frames differ by 488 px, right-then-left visits `0x09 0x0a` and both are packed.

```
make pico-ls-layers-test pico-move-test pico-explorer-buttons-test
```

---

## Open beads

| Bead | P | What |
| --- | --- | --- |
| `sm_rev-k5q.3` | **P0** | Pack LS collision; mini still runs its fallback room |
| `sm_rev-k5q.4` | P1 | Gunship top layer renders with its hull cut flat |
| `sm_rev-k5q.5` | P1 | BG2 below the ship is uniform wallpaper |
| `sm_rev-k5q.6` | P1 | Pack ROM bank 0x91, retire two stand-ins |
| `sm_rev-k5q.7` | P1 | 135ms/frame baseline and the single-core wins |

### k5q.3 — the one that matters (do this first)

On-device mini has no filesystem (`MiniEditorPath_Exists` returns false), so `MiniCreate` uses its built-in fallback room and spawns at **(80,160)** while Landing Site's real spawn is **(1153,1088)**. The *display* half is worked around by capturing mini's boot origin into `s_sim_origin_x/y` and re-basing onto `kPicoLsRoomSpawnX/Y`. The *physics* is not worked around.

One cause, both reported symptoms: she walks off the ship without falling, and she can never get down onto the terrain.

Room facts to verify against: spawn block (72,68); ship deck collision by=69..73; real terrain floor by=76..79 (world y 1216+). Walking off the deck should drop her ~112 px.

**When the real room loads, delete the re-basing** — `s_sim_origin_x/y` and the remap in `pack_from_samus`. With the real room, `samus_x_pos`/`layer1_x_pos` are already world-space.

### k5q.6 — the only two invented numbers in the Samus path

`assets/local_mini` has Samus bank 0x92 but **no bank 0x91**.

1. `Samus_HandleAnimDelay()` reads `RomPtr_91(kSamusAnimationDelayData[pose])`, and separately mini never points `frame_handler_beta` at the handler that calls `Samus_Animate()`. **Measured:** `frame_handler_beta` stays `0x0000`, `samus_anim_frame` and its timer both stay 0 for an entire walk. *That*, not the missing tiles, is why she slid — the previous handoff's "pose-1 CHR is planted once" was only half the story. Stand-in: `anim_frame_for()` in `pico2_main.c`, flat 66ms cadence, which already defers to `samus_anim_frame` if it ever becomes non-zero.
2. `kPoseParams[pose].y_offset_to_gfx` at ROM 0x91B629. Foot extents vary a lot (stand +31, crouch +15, morphball +8, jump +23), so a fixed 6 would float morphball 23px. Stand-in: the packer aligns each **ground** pose's lowest sprite row to the reference pose's floor line — determined for floor-anchored poses. **Airborne poses are not floor-anchored** and keep 6, so jump and spin can sit a few px off. `y_radius` has the same problem (`samus_collision_advanced.c`).

### k5q.4 — already ruled out, do not redo

Cut flat in the **OBJ-only** pass (`out/pico_ls_obj.ppm`) with both BG maps transparent, so it is **not occlusion**. Also ruled out: vertical clipping (all 56 y bytes 110..142, none wrapped), BG1 high-priority tiles over the ship (0/175 have the 0x2000 bit), per-sprite tile sheets (all three share one), OAM slot exhaustion. Strongest remaining suspect: `pack_gunship()` reads `tileDataPath` only from `K_GUNSHIP_KEYS[0]` and assumes a flat 144-tile block from offset 0.

---

## Performance, before the two-core split (k5q.7)

`raster` went 79.7 → 87.6ms when the gunship landed: `blit_obj()` walks all 128 slots **four times per line**, once per priority. 135ms/frame, 7.4 fps.

Order of value on one core:
1. `decode_4bpp()` re-reads all four bitplane bytes **per pixel** — 32 loads per tile row where 4 would do. Decode a tile row at a time.
2. CGRAM→RGB565 LUT once per frame instead of converting per pixel.
3. Hoist per-pixel bounds checks to per-tile.
4. Build a per-line sprite candidate list instead of rescanning 128 slots per priority.
5. Skip the backdrop fill that BG2 overwrites.
6. Render only the 240 visible columns (~6% of raster is currently thrown away).

SPI is 47.6ms for 115200 B at 32MHz against a 28.8ms floor; DMA with a pre-swapped buffer plus 62.5MHz gets it to ~15ms. Raster and SPI are **serialized** today — core1 + double buffering makes the frame cost `max()` not `sum()`.

~30ms/frame (30-33 fps) looks reachable for this scene. **60 fps should not be promised** for a full room, and the vanilla 32-sprite / 34-tile-per-line limits are still unimplemented — adding them costs time rather than saving it.

Also unresolvable in software: **256x224 does not fit a 240x240 panel.** Current behaviour throws away 16 columns. If pixel-exact is a hard requirement that is a panel decision (320x240 keeps the full width), not a code one.

---

## Also still true

- BG2 is the `door_896A` variant from `room_91F8.json`'s own `defaultVariantKey`; all six variants are door-transition captures. `generated_backgrounds/` exists and was never examined.
- `origin_by = cameraY//16 + 2` is a hand-tuned +32px fudge, and the packer guard hardcodes `origin_by != 63`, so the check **enforces the fudge** rather than validating anything.
- The sim flips `0x09` → `0x0A` directly and never visits turn poses `0x25`/`0x26`. They are packed and ready.
- `pico_samus_pose1.inc` is replaced by `pico_samus_anim.inc` and removed from the index.
- Mini was not touched. `sm_rev_mini` not rebuilt.

## Leftovers from the earlier code review, not yet done

Low severity, all still open in the tree: `PicoOam_PlantGunshipGfx`/`PlantSamusFrame` return values are logged but not acted on; `emit_u8`/`emit_u16` still carry a `static` parameter now used only by the gunship inc; `K_SAMUS_Y_OFFSET_TO_GFX` is still described as unverified in the packer comment (correct — see k5q.6).
