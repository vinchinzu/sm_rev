# Not A Samus: Armadillo

`sm_rev_mini` compatible Samus replacement pack using the existing `samusAssets`
binary schema. The gameplay pose IDs, animation timing, collision, and hit boxes
stay unchanged; only the visual tile data and palettes are replaced.

Character target: a black/graphite armadillo exosuit with warm shell plates,
amber eyes, claws, and a cyan hand cannon.

Generated assets:

- `samus_bank92.bin`, `samus_data.bin`, and `samus_*_palette.bin`: runtime pack.
- `samus_rendered_160.rgba`: fixed-origin rendered override frames for `sm_rev_mini`.
- `asset_pack.json`: standalone mini asset manifest.
- `../room_91F8_armadillo.json`: ready-to-run Landing Site room export.
- `reference/armadillo_canonical_sheet.png`: image-model canonical direction sheet.
- `previews/frames_160/`: 160 fixed-origin transparent PNG sprites.
- `previews/armadillo_160_sheet.png`: 16 x 10 source sheet for the requested 160 sprites.
- `previews/frames/`, `previews/frames_trimmed/`, `previews/animations/`, `previews/groups/`: full decoded animation preview set (1798 frames).

Run from `../sm_rev`:

```bash
./sm_rev_mini --room-export assets/local_mini/room_91F8_armadillo.json
```

Headless smoke test:

```bash
./sm_rev_mini --headless --frames 6 --room-export assets/local_mini/room_91F8_armadillo.json --screenshot out/armadillo_mini.bmp
```

Note: `../sm_rev_mini` was not present next to the editor checkout, so this pack
was written into the existing `../sm_rev/assets/local_mini` tree that owns the
`sm_rev_mini` executable and schema.
