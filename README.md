# Chess Fart

**Chess Fart** is a deliberately ridiculous 256-color VGA strategy game: recognizable chess rebuilt as a loud early-1990s DOS game where pieces build Gas and eventually use directional fart blasts as tactical abilities.

The design goal is **real strategy first, toilet humor second**. Normal chess remains the foundation; the Gas/Fart layer arrives after the standard move engine is solid.

## Project status

**Build 2 — Cursor and Pieces: complete in source.**

Build 2 is the first interactive board milestone:

- 320x200 Mode 13h presentation
- all 32 pieces in the standard starting position
- compact placeholder VGA piece silhouettes
- arrow-key board cursor
- Enter to select/deselect a piece
- persistent selected-square highlight
- live cursor/piece/selection information in the side panel
- Escape restores text mode and exits
- strict host-side board tests and framebuffer smoke test

The host validation passes. The DOS target is ready for Open Watcom compilation; Open Watcom/DOSBox is not installed in the environment used to prepare this milestone, so no precompiled DOS executable is committed.

See [`docs/BUILD_2.md`](docs/BUILD_2.md) for the full milestone notes.

## Controls — Build 2

```text
Arrow keys   Move cursor
Enter        Select/deselect piece
Esc          Quit
```

Pieces do **not** move yet. Build 3 adds legal movement, captures, attack detection, and king-safety filtering.

## Build

### Host verification

```sh
make test-build2
```

This builds and runs the host shell, checks the board model, and writes:

```text
build/host/chessfart_build2.ppm
```

### DOS / Open Watcom

```bat
wmake -f makefile.dos dos
```

or:

```bat
scripts\build_dos.bat
```

Expected output:

```text
build\dos\CHESSFRT.EXE
```

## Target experience

- 320x200, 256-color VGA / Mode 13h presentation
- DOS-era visual language: chunky sprites, dithered gradients, palette cycling, oversized UI panels
- standard chess movement and check/checkmate as the foundation
- deterministic Gas/Fart tactics layered on top
- keyboard-first controls, optional mouse support later
- Sound Blaster-style sampled fart effects with PC speaker fallback
- DOSBox as the primary runtime target, with real DOS-era constraints kept in mind

## The core gimmick

Every piece eventually has a small **Gas meter**. Gas is earned through normal play. A sufficiently charged piece may spend its turn on a **Fart Action** instead of making a normal move. A fart projects a one-square blast in a chosen direction and can shove a neighboring piece one square farther away when legal.

The system is deterministic and visible to both players. Full rules are in [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md).

## Source layout

```text
include/
src/
  main.c
  game/
    board.c
    board_view.c
    font.c
  platform/
    dos/
    host/
tests/
scripts/
docs/
```

The board model is platform-independent. DOS-specific video/input stays under `src/platform/dos`, while the host backend exists for fast deterministic testing.

## Documentation

- [`docs/MASTER_PLAN.md`](docs/MASTER_PLAN.md) — full project blueprint
- [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md) — chess + Gas/Fart rules
- [`docs/VGA_ART_STYLE.md`](docs/VGA_ART_STYLE.md) — VGA art direction
- [`docs/TECHNICAL_ARCHITECTURE.md`](docs/TECHNICAL_ARCHITECTURE.md) — engine architecture
- [`docs/AUDIO_DESIGN.md`](docs/AUDIO_DESIGN.md) — audio plan
- [`docs/ASSET_MANIFEST.md`](docs/ASSET_MANIFEST.md) — planned assets
- [`docs/ROADMAP.md`](docs/ROADMAP.md) — milestone roadmap
- [`docs/TEST_PLAN.md`](docs/TEST_PLAN.md) — test strategy
- [`docs/BUILD_AND_TOOLCHAIN.md`](docs/BUILD_AND_TOOLCHAIN.md) — toolchain plan
- [`docs/BUILD_1.md`](docs/BUILD_1.md) — VGA boot milestone
- [`docs/BUILD_2.md`](docs/BUILD_2.md) — cursor/pieces milestone

## Design pillars

1. **Still chess.** Normal chess knowledge matters.
2. **No hidden randomness.** Gas state and fart outcomes are predictable.
3. **Fast to read.** The 320x200 screen must communicate state instantly.
4. **Juicy VGA presentation.** Palette flashes, screen shake, sprite squash, gas clouds and crude digitized audio are part of the identity.
5. **Small enough to finish.** One polished board and one strong ruleset beat oversized scope.

## Working title

**CHESS FART**

Tagline: **Check. Mate. Ventilate.**
