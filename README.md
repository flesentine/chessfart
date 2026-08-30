# Chess Fart

**Chess Fart** is a deliberately ridiculous 256-color VGA strategy game: real chess rebuilt as a loud early-1990s DOS game where pieces build Gas and use directional fart blasts as tactical abilities.

The design goal is **real strategy first, toilet humor second**. Build 4 completed the normal chess foundation; Build 5 puts the Gas system online.

## Project status

**Build 5 — Gas System: complete in source.**

Chess Fart now has the complete standard chess engine plus its first variant layer:

- Gas 0–3 belongs to every piece
- normal moves earn +1 Gas
- captures earn +2 Gas total
- castling charges both moved pieces
- promotion carries Gas into the promoted piece
- Gas is visible directly on the VGA board
- `F` enters Fart mode for a selected piece with at least 2 Gas
- all eight blast directions are targetable
- empty/off-board directions can be confirmed as a turn-consuming `PUFF`
- Gas is included in Chess Fart repetition identity
- occupied targets are previewed but actual displacement is deliberately Build 6 work

Build 5 uses a parallel `CfGasState` around the unchanged Build 4 chess core, keeping standard chess as a permanent regression baseline.

See [`docs/BUILD_5.md`](docs/BUILD_5.md) for implementation and verification details.

## Controls — Build 5

```text
Arrow keys        Move cursor / aim cardinal Fart direction
Enter             Select / move / confirm PUFF
F                 Enter or cancel Fart mode
Keypad diagonals  Aim NE / SE / SW / NW
Esc               Quit
```

Visual Gas pips appear at the bottom of every occupied square. A piece needs at least **2 Gas** to enter Fart mode.

## Build

### Host verification

```sh
make test-build5
```

This reruns the complete Build 4 chess suite, runs the Build 5 Gas/Fart tests, executes the scripted UI smoke test, and writes `build/host/chessfart_build5.ppm`.

### DOS / Open Watcom

```bat
wmake -f makefile.build5.dos dos
```

or:

```bat
scripts\build_dos_build5.bat
```

Expected output: `build\dos\CHESSFRT.EXE`.

Open Watcom/DOSBox is not available in the environment used to prepare this build, so the DOS target is source-ready but not runtime-verified here.

## Target experience

- 320x200, 256-color VGA / Mode 13h presentation
- DOS-era visual language: chunky sprites, dithered gradients, palette cycling, oversized UI panels
- complete standard chess as the rules foundation
- deterministic Gas/Fart tactics layered on top
- keyboard-first controls, optional mouse support later
- Sound Blaster-style sampled fart effects with PC speaker fallback
- DOSBox as the primary runtime target, with real DOS-era constraints kept in mind

## The core gimmick

Every piece has a small **Gas meter**. Gas is earned through normal play. A sufficiently charged piece may spend its turn on a **Fart Action** instead of making a normal move. A fart projects a one-square blast in a chosen direction and, beginning in Build 6, can shove a neighboring piece one square farther away when legal.

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
- [`docs/BUILD_5.md`](docs/BUILD_5.md) — Gas-system milestone

## Design pillars

1. **Still chess.** Normal chess knowledge matters.
2. **No hidden randomness.** Gas state and fart outcomes are predictable.
3. **Fast to read.** The 320x200 screen must communicate state instantly.
4. **Juicy VGA presentation.** Palette flashes, screen shake, sprite squash, gas clouds and crude digitized audio are part of the identity.
5. **Small enough to finish.** One polished board and one strong ruleset beat oversized scope.

## Working title

**CHESS FART**

Tagline: **Check. Mate. Ventilate.**
