# Chess Fart

**Chess Fart** is a deliberately ridiculous 256-color VGA strategy game: real chess rebuilt as a loud early-1990s DOS game where pieces will build Gas and use directional fart blasts as tactical abilities.

The design goal is **real strategy first, toilet humor second**. Build 4 completes the normal chess layer; Build 5 is where the Gas system begins.

## Project status

**Build 4 — Complete Standard Chess: complete in source.**

Pure Chess now supports:

- legal movement for every piece
- captures and king-safety filtering
- castling on both sides with persistent castling rights
- en passant, including king-safety edge cases
- promotion to queen, rook, bishop, or knight
- check, checkmate, and stalemate
- halfmove/fullmove bookkeeping
- fifty-move draw handling
- threefold repetition tracking
- insufficient-material draws
- make/unmake restoration of all special-rule state

The interactive VGA shell includes a promotion chooser and terminal game-state display.

See [`docs/BUILD_4.md`](docs/BUILD_4.md) for implementation and verification details.

## Controls — Build 4

```text
Arrow keys   Move cursor
Enter        Select piece / make legal move / confirm promotion
Esc          Quit
```

During promotion, use the arrow keys to cycle **Queen / Rook / Bishop / Knight**, then press Enter.

Visual hints:

- cyan = cursor
- magenta = selected source
- green marker = legal quiet move
- orange/red outline = legal capture
- red status = check/checkmate

## Build

### Host verification

```sh
make test-build4
```

This runs the strict C89 rule suite, exercises the host UI, and writes `build/host/chessfart_build4.ppm`.

The regression suite includes standard opening perft through depth 4 plus special-rule reference positions covering castling, en passant, and promotion-heavy trees.

### DOS / Open Watcom

```bat
wmake -f makefile.dos dos
```

or `scripts\build_dos.bat`.

Expected output: `build\dos\CHESSFRT.EXE`.

The DOS target remains ready for Open Watcom compilation. Open Watcom/DOSBox is not installed in the environment used for this milestone, so no precompiled DOS executable is claimed here.

## Target experience

- 320x200, 256-color VGA / Mode 13h presentation
- DOS-era visual language: chunky sprites, dithered gradients, palette cycling, oversized UI panels
- complete standard chess as the rules foundation
- deterministic Gas/Fart tactics layered on top starting in Build 5
- keyboard-first controls, optional mouse support later
- Sound Blaster-style sampled fart effects with PC speaker fallback
- DOSBox as the primary runtime target, with real DOS-era constraints kept in mind

## The core gimmick

Every piece eventually has a small **Gas meter**. Gas is earned through normal play. A sufficiently charged piece may spend its turn on a **Fart Action** instead of making a normal move. A fart projects a one-square blast in a chosen direction and can shove a neighboring piece one square farther away when legal.

The system is deterministic and visible to both players. Full rules are in [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md).

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
- [`docs/BUILD_3.md`](docs/BUILD_3.md) — legal-movement milestone
- [`docs/BUILD_4.md`](docs/BUILD_4.md) — complete standard-chess milestone

## Design pillars

1. **Still chess.** Normal chess knowledge matters.
2. **No hidden randomness.** Gas state and fart outcomes are predictable.
3. **Fast to read.** The 320x200 screen must communicate state instantly.
4. **Juicy VGA presentation.** Palette flashes, screen shake, sprite squash, gas clouds and crude digitized audio are part of the identity.
5. **Small enough to finish.** One polished board and one strong ruleset beat oversized scope.

## Working title

**CHESS FART**

Tagline: **Check. Mate. Ventilate.**
