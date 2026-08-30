# Chess Fart

**Chess Fart** is a deliberately ridiculous 256-color VGA strategy game: recognizable chess rebuilt as a loud early-1990s DOS game where pieces build Gas and eventually use directional fart blasts as tactical abilities.

The design goal is **real strategy first, toilet humor second**. Normal chess remains the foundation; the Gas/Fart layer arrives after the standard move engine is solid.

## Project status

**Build 3 — Legal Chess Movement: complete in source.**

The board is now genuinely playable for ordinary chess moves:

- all six piece types generate movement
- White/Black turns alternate
- legal destinations are highlighted
- ordinary captures work
- attack maps and check detection work
- moves that expose your own king are rejected
- pinned pieces are handled through king-safety filtering
- make/unmake move records are available for later AI/search work
- strict C89 regression tests pass

Special rules — castling, en passant, promotion, mate/stalemate, and draw state — are intentionally Build 4 work.

See [`docs/BUILD_3.md`](docs/BUILD_3.md) for implementation and verification details.

## Controls — Build 3

```text
Arrow keys   Move cursor
Enter        Select piece / make legal move / change source
Esc          Quit
```

Visual hints:

- cyan = cursor
- magenta = selected source
- green marker = legal quiet move
- orange/red outline = legal capture

## Build

### Host verification

```sh
make test-build3
```

This runs the rules regression suite, exercises a scripted E2-E4 interaction, and writes `build/host/chessfart_build3.ppm`.

### DOS / Open Watcom

```bat
wmake -f makefile.dos dos
```

or `scripts\build_dos.bat`.

Expected output: `build\dos\CHESSFRT.EXE`.

The DOS target is ready for Open Watcom compilation. Open Watcom/DOSBox is not available in the environment used to prepare this build, so a precompiled DOS executable is not committed.

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

## Design pillars

1. **Still chess.** Normal chess knowledge matters.
2. **No hidden randomness.** Gas state and fart outcomes are predictable.
3. **Fast to read.** The 320x200 screen must communicate state instantly.
4. **Juicy VGA presentation.** Palette flashes, screen shake, sprite squash, gas clouds and crude digitized audio are part of the identity.
5. **Small enough to finish.** One polished board and one strong ruleset beat oversized scope.

## Working title

**CHESS FART**

Tagline: **Check. Mate. Ventilate.**
