# Chess Fart

**Chess Fart** is a deliberately ridiculous 256-color VGA strategy game: recognizable chess, rebuilt as a loud early-1990s DOS game where pieces can build gas and use directional fart blasts as tactical abilities.

The design goal is **real strategy first, toilet humor second**. A good chess player should immediately understand the board, but the gas system creates new positional tactics, traps, escapes, and combo opportunities without turning the game into random chaos.

## Project status

**Build 1 — VGA Boot: complete in source.**

Build 1 adds the first executable code:

- C89/C90 project skeleton
- 320x200 Mode 13h DOS video backend
- 64,000-byte software backbuffer
- 256-entry VGA DAC palette
- tiny built-in bitmap font
- static 8x8 board and status panel
- keyboard Escape handling
- text-mode restoration on shutdown
- modern host backend that renders the exact indexed frame to a PPM image for smoke testing

The host version compiles with strict C89 warnings and its Build 1 smoke test passes. The DOS source is prepared for Open Watcom; the DOS executable must be compiled in an Open Watcom environment.

See [`docs/BUILD_1.md`](docs/BUILD_1.md) for the exact milestone notes.

## Build

### Host smoke build

```sh
make test-build1
```

This builds `build/host/chessfart_host` and writes `build/host/chessfart_build1.ppm`.

### DOS / Open Watcom

```bat
wmake -f makefile.dos dos
```

or run `scripts\build_dos.bat`.

Expected output: `build\dos\CHESSFRT.EXE`.

Run it in DOSBox and press **Esc** to restore text mode and exit.

## Target experience

- 320x200, 256-color VGA / Mode 13h presentation
- DOS-era visual language: chunky sprites, dithered gradients, palette cycling, oversized UI panels
- Standard chess movement and check/checkmate as the foundation
- A deterministic gas/fart mechanic layered on top
- Keyboard-first controls, optional mouse support
- Sound Blaster-style sampled fart effects with PC speaker fallback
- Runs in DOSBox and is designed to remain compatible with real DOS-era constraints

## The core gimmick

Every piece has a small **Gas meter**. Gas is earned through normal play. A sufficiently charged piece may spend its turn on a **Fart Action** instead of making a normal move. A fart projects a one-square blast in a chosen direction and can shove a neighboring piece one square farther away if that destination is valid.

This turns familiar chess geometry into something new: pieces can be displaced, lines can be opened or closed, defenders can be pushed off squares, and kings can sometimes escape by using gas. The system is deterministic and completely visible to both players.

The exact rules, king-safety constraints, edge cases, and tuning are defined in [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md).

## Source layout

```text
include/
src/
  main.c
  game/
  platform/
    dos/
    host/
scripts/
docs/
```

The application and game-view layers use platform interfaces; DOS-specific code stays under `src/platform/dos`.

## Documentation

- [`docs/MASTER_PLAN.md`](docs/MASTER_PLAN.md) — full project blueprint and build order
- [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md) — game rules and Gas system
- [`docs/VGA_ART_STYLE.md`](docs/VGA_ART_STYLE.md) — Mode 13h art direction
- [`docs/TECHNICAL_ARCHITECTURE.md`](docs/TECHNICAL_ARCHITECTURE.md) — engine architecture
- [`docs/AUDIO_DESIGN.md`](docs/AUDIO_DESIGN.md) — audio plan
- [`docs/ASSET_MANIFEST.md`](docs/ASSET_MANIFEST.md) — planned assets
- [`docs/ROADMAP.md`](docs/ROADMAP.md) — milestone roadmap
- [`docs/TEST_PLAN.md`](docs/TEST_PLAN.md) — test strategy
- [`docs/BUILD_AND_TOOLCHAIN.md`](docs/BUILD_AND_TOOLCHAIN.md) — toolchain plan
- [`docs/BUILD_1.md`](docs/BUILD_1.md) — Build 1 implementation/verification

## Design pillars

1. **Still chess.** Normal chess knowledge matters.
2. **No hidden randomness.** Gas state and fart outcomes are predictable.
3. **Fast to read.** The 320x200 screen must communicate legal moves, check, gas and blast direction instantly.
4. **Juicy VGA presentation.** Palette flashes, screen shake, sprite squash, cartoon gas clouds and crude digitized audio are part of the identity.
5. **Small enough to finish.** One polished board, one strong ruleset and a few game modes beat an oversized unfinished game.

## Working title

**CHESS FART**

Tagline: **Check. Mate. Ventilate.**
