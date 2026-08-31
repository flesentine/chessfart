# Chess Fart

**Chess Fart** is a deliberately ridiculous 256-color VGA strategy game: real chess rebuilt as a loud early-1990s DOS game where pieces build Gas and use directional fart blasts as tactical abilities.

The design goal is **real strategy first, toilet humor second**.

## Project status

**Build 10 — CPU Opponent: complete in source/host validation when CI is green.**

Chess Fart now combines the complete rules engine, VGA presentation, digital audio, persistence, and a Black CPU opponent:

- full legal chess plus Gas/Fart displacement rules
- 320x200 256-color VGA presentation
- Sound Blaster / PC-speaker audio architecture
- versioned save/load and audio config
- deterministic iterative-deepening CPU
- alpha-beta pruning
- Easy / Medium / Hard search budgets
- CPU evaluates and searches both chess moves and Fart Actions
- automatic CPU reply after White actions and Black-to-move loads

See [`docs/BUILD_10.md`](docs/BUILD_10.md) and [`docs/CPU_DESIGN.md`](docs/CPU_DESIGN.md).

## Controls

```text
Title screen:
Up/Down          Choose Play / Quit
Left/Right       Cycle audio device
F                 Cycle SFX volume
D                 Cycle CPU difficulty
Enter             Confirm
Esc               Quit

In game:
Arrow keys        Move cursor / aim cardinal Fart direction
Enter             Select / move / confirm action
F                 Enter or cancel Fart mode
Keypad diagonals  Aim NE / SE / SW / NW
S                 Save
L                 Load
Esc               Quit
```

Build 10 is **human White versus CPU Black**.

## Build

### Host verification

```sh
make test-build10
```

This reruns the permanent Build 4–9 suites, runs CPU search tests, executes the scripted VGA/audio/save-load game with automatic CPU replies, and verifies all generated artifacts.

### DOS / Open Watcom

```bat
wmake -f makefile.build10.dos dos
```

or:

```bat
scripts\build_dos_build10.bat
```

Expected output: `build\dos\CHESSFRT.EXE`.

Open Watcom/DOSBox is still required for final DOS runtime and CPU-speed validation.

## CPU

The CPU uses deterministic iterative-deepening negamax with alpha-beta pruning. Every legal chess move and every legal Fart Action is generated through the same rules engine used for human play. Search is capped by both nodes and time so old DOS hardware cannot disappear into an unbounded think.

Default levels:

- EASY — depth 1 / 800 nodes / 100 ms
- MED — depth 2 / 8,000 nodes / 500 ms
- HARD — depth 3 / 50,000 nodes / 1,500 ms

## Core rules

Every piece has Gas 0–3. Normal moves earn +1 Gas; captures earn +2 total. At 2+ Gas, a piece can spend its turn on a deterministic Fart Action in one of eight directions. An empty adjacent square produces a PUFF. An adjacent piece can be pushed one square farther when the destination and resulting king safety are legal. Farting never directly captures.

Full rules are in [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md).

## Documentation

- [`docs/MASTER_PLAN.md`](docs/MASTER_PLAN.md)
- [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md)
- [`docs/VGA_ART_STYLE.md`](docs/VGA_ART_STYLE.md)
- [`docs/TECHNICAL_ARCHITECTURE.md`](docs/TECHNICAL_ARCHITECTURE.md)
- [`docs/AUDIO_DESIGN.md`](docs/AUDIO_DESIGN.md)
- [`docs/SAVE_FORMAT.md`](docs/SAVE_FORMAT.md)
- [`docs/CPU_DESIGN.md`](docs/CPU_DESIGN.md)
- [`docs/ROADMAP.md`](docs/ROADMAP.md)
- [`docs/BUILD_10.md`](docs/BUILD_10.md)

**CHESS FART** — *Check. Mate. Ventilate.*
