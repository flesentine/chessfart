# Chess Fart

**Chess Fart** is a deliberately ridiculous 256-color VGA strategy game: real chess rebuilt as a loud early-1990s DOS game where pieces build Gas and use directional fart blasts as tactical abilities.

The design goal is **real strategy first, toilet humor second**.

## Project status

**Build 7 — Presentation Pass: complete in source.**

The rule set from Builds 4–6 is intact, but the game now looks much closer to the intended DOS release:

- dedicated pixel-art title/menu screen
- `Royal Basement` VGA palette
- improved pawn/knight/bishop/rook/queen/king silhouettes
- board bevel plus rank/file coordinates
- visible Gas pips and existing move/fart previews retained
- five-frame fart plume and push animation
- impact palette flash and board-frame shake
- deterministic host preview artifact

Build 7 changes presentation only. Standard chess, Gas, PUFF/PUSH/BLOCKED, displaced kings/rooks, pushed promotion, repetition and all other Build 6 rules remain governed by the existing core engine.

See [`docs/BUILD_7.md`](docs/BUILD_7.md) for implementation details.

## Controls

```text
Title screen:
Up/Down          Choose menu item
Enter            Confirm
Esc              Quit

In game:
Arrow keys        Move cursor / aim cardinal Fart direction
Enter             Select / move / confirm action
F                 Enter or cancel Fart mode
Keypad diagonals  Aim NE / SE / SW / NW
Esc               Quit
```

## Build

### Host verification

```sh
make test-build7
```

This builds the Build 7 presentation shell, reruns all permanent Build 4–6 rule suites, performs the scripted C3→D4→E5 fart-push demo, and writes:

```text
build/host/chessfart_build7.ppm
```

### DOS / Open Watcom

```bat
wmake -f makefile.build7.dos dos
```

or:

```bat
scripts\build_dos_build7.bat
```

Expected output: `build\dos\CHESSFRT.EXE`.

Open Watcom/DOSBox is not installed in the environment used to prepare this build, so DOS runtime validation remains the platform integration check.

## Core rules

Every piece has Gas 0–3. Normal moves earn +1 Gas; captures earn +2 total. At 2+ Gas, a piece can spend its turn on a deterministic Fart Action in one of eight directions. An empty adjacent square produces a PUFF. An adjacent piece can be pushed one square farther when the destination and resulting king safety are legal. Farting never directly captures.

Full rules are in [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md).

## Documentation

- [`docs/MASTER_PLAN.md`](docs/MASTER_PLAN.md)
- [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md)
- [`docs/VGA_ART_STYLE.md`](docs/VGA_ART_STYLE.md)
- [`docs/TECHNICAL_ARCHITECTURE.md`](docs/TECHNICAL_ARCHITECTURE.md)
- [`docs/AUDIO_DESIGN.md`](docs/AUDIO_DESIGN.md)
- [`docs/ROADMAP.md`](docs/ROADMAP.md)
- [`docs/TEST_PLAN.md`](docs/TEST_PLAN.md)
- [`docs/BUILD_7.md`](docs/BUILD_7.md)

## Design pillars

1. **Still chess.** Normal chess knowledge matters.
2. **No hidden randomness.** Gas state and fart outcomes are predictable.
3. **Fast to read.** The 320x200 screen must communicate state instantly.
4. **Juicy VGA presentation.** Palette flashes, screen shake, sprites and gas clouds are part of the identity.
5. **Small enough to finish.** One polished board and one strong ruleset beat oversized scope.

**CHESS FART** — *Check. Mate. Ventilate.*
