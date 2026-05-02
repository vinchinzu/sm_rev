# sm_rev

`sm_rev` is a reverse-engineered C port of Super Metroid with two active goals:
- preserve full-game behavior in the main build
- grow a scoped `mini` build into a deterministic Samus / physics / modding sandbox

The mini build is intentionally scoped. The full build stays the parity target,
while mini constrains content and host behavior around a small, testable
gameplay surface.

## Build Targets

Full build:
- `make`

Mini:
- `make mini`
- `make mini-test`

Native macOS:
- `make NATIVE_MAC=1`
- `make mini-mac`
- `make mini NATIVE_MAC=1`

The current `sm_rev_mini` binary supports headless smoke tests, deterministic
replay/rollback checks, editor-export rooms, and a ROM-backed Landing Site path
when compatible assets are available.

## Mission

The main refactor mission has two tracks that must stay compatible:
- refactor ASM-like C into readable topic-based modules
- grow the subtractive mini build into a Samus-and-physics sandbox without regressing the full game

That means:
- keep full-build behavioral parity as the default constraint
- prefer negative-first build cuts over scattering wrappers through unrelated code
- move mixed logic out of monolithic `sm_*.c` files into topical modules before pulling it into mini

## Current Plan

The project has moved past the first mini bring-up milestone. The immediate work is now about proving that the mini gameplay kernel is deterministic enough to survive rollback pressure before widening scope or porting gameplay logic.

### What is already in place

- the mini gameplay API now exposes `MiniInit`, `MiniStep`, `MiniSaveState`, `MiniLoadState`, `MiniStateHash`, `MiniCreate`, and `MiniDestroy`
- `make mini-test` runs the mini smoke test plus a focused rollback seam check
- the Rust headless host in `src/mini/mini_rust_host.rs` can now run a rollback simulation over the C gameplay API
- ROM/save bootstrap and first-pass room FX have been split into mini-specific modules instead of continuing to grow `stubs_mini.c`

### Next workstreams

1. **Rollback stress coverage**
   - keep expanding the Rust rollback host beyond the current deterministic smoke path
   - exercise longer delayed-input windows, room-export inputs, projectile-heavy scripts, and ROM-backed room paths
   - make CI fail loudly when rollback hashes diverge from a clean reference run

2. **Determinism stress tests**
   - expand beyond the current short rollback seam check
   - cover longer scripted runs, repeated save/load cycles, projectile progression, and ROM-backed room paths
   - make CI fail loudly when the same input stream stops producing the same hash

3. **Replay artifact format**
   - define a minimal replay/checkpoint format for reproducible bug reports and future spectator support
   - store initial state metadata, frame inputs, and final hash

4. **Rust host evolution**
   - keep gameplay in C for now
   - move rollback orchestration, replay driving, and future netcode shell work into Rust
   - do not port gameplay modules to Rust until the deterministic seam is well tested

5. **Only then widen mini scope**
   - add more rooms, systems, or gameplay slices only after rollback/replay validation is boring and reliable
   - continue full-build topical cleanup in parallel where it improves shared code without changing behavior

## Validation

Use the smallest test that matches the change:
- full build integrity: `python3 -m pytest tests/test_build.py -q`
- full runtime smoke: `python3 tests/run_tests.py -v`
- mini smoke: `make mini-test`

## Docs

- Active roadmap: [docs/roadmap.md](docs/roadmap.md)
- Source layout: [docs/source_layout.md](docs/source_layout.md)
- Mini build notes: [docs/mini_build.md](docs/mini_build.md)
- Mini modability plan: [docs/mini_modability_plan.md](docs/mini_modability_plan.md)
- Refactor guide and standing instructions: [AGENTS.md](AGENTS.md)
