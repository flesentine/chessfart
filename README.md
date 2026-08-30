# Chess Fart

**Chess Fart** is a deliberately ridiculous 256-color VGA strategy game: recognizable chess, rebuilt as a loud early-1990s DOS game where pieces can build gas and use directional fart blasts as tactical abilities.

The design goal is **real strategy first, toilet humor second**. A good chess player should immediately understand the board, but the gas system creates new positional tactics, traps, escapes, and combo opportunities without turning the game into random chaos.

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

## Project status

**Phase 0 — design foundation.** No gameplay code has been committed yet. The repository currently contains the build blueprint and production documentation needed to begin implementation cleanly.

## Documentation

- [`docs/MASTER_PLAN.md`](docs/MASTER_PLAN.md) — full project blueprint and build order
- [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md) — game rules, gas system, game modes, balance principles
- [`docs/VGA_ART_STYLE.md`](docs/VGA_ART_STYLE.md) — Mode 13h art direction, palette, sprite sizes, UI layout
- [`docs/TECHNICAL_ARCHITECTURE.md`](docs/TECHNICAL_ARCHITECTURE.md) — engine structure, board model, rendering, input, AI and save format
- [`docs/AUDIO_DESIGN.md`](docs/AUDIO_DESIGN.md) — music/SFX plan and Sound Blaster/PC speaker strategy
- [`docs/ASSET_MANIFEST.md`](docs/ASSET_MANIFEST.md) — planned graphics, animation and audio assets
- [`docs/ROADMAP.md`](docs/ROADMAP.md) — milestone-by-milestone implementation plan
- [`docs/TEST_PLAN.md`](docs/TEST_PLAN.md) — rules, regression, DOSBox and performance testing
- [`docs/BUILD_AND_TOOLCHAIN.md`](docs/BUILD_AND_TOOLCHAIN.md) — proposed compiler/toolchain and project layout
- [`CONTRIBUTING.md`](CONTRIBUTING.md) — repository conventions for future builds

## Proposed implementation stack

The reference implementation plan uses **C89/C90 + Open Watcom C/C++**, direct Mode 13h framebuffer rendering, fixed-size data structures, and a small platform layer for VGA, timer, keyboard, mouse and audio access. The chess/rules engine remains platform-independent so it can be unit-tested outside the DOS renderer.

## Design pillars

1. **Still chess.** Normal chess knowledge matters.
2. **No hidden randomness.** Gas state and fart outcomes are predictable.
3. **Fast to read.** The 320x200 screen must communicate legal moves, check, gas and blast direction instantly.
4. **Juicy VGA presentation.** Palette flashes, screen shake, sprite squash, cartoon gas clouds and crude digitized audio are part of the identity.
5. **Small enough to finish.** One polished board, one strong ruleset and a few game modes beat an oversized unfinished game.

## Working title

**CHESS FART**

Tagline: **Check. Mate. Ventilate.**
