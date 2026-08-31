# Chess Fart

**Chess Fart** is a deliberately ridiculous 256-color VGA strategy game: real chess rebuilt as a loud early-1990s DOS game where pieces build Gas and use directional fart blasts as tactical abilities.

The design goal is **real strategy first, toilet humor second**.

## Project status

**Build 9 — Save/Load and Config: complete in source/host validation when CI is green.**

Chess Fart now has the complete Build 6 rules, Build 7 presentation, Build 8 audio, and persistent game/config state:

- `S` saves the current match
- `L` loads the current match
- versioned `CHESSFRT.SAV`
- exact board, Gas, counters and Gas-aware repetition-history restoration
- transactional load validation: failed loads leave the live match unchanged
- versioned `CHESSFRT.CFG`
- persistent audio device/SFX/music-level settings
- temp-file replacement writes
- corrupt/truncated/unsupported save rejection
- host save/config artifacts for inspection

Save/config state does **not** alter chess/Fart legality or repetition identity.

See [`docs/BUILD_9.md`](docs/BUILD_9.md) and [`docs/SAVE_FORMAT.md`](docs/SAVE_FORMAT.md).

## Controls

```text
Title screen:
Up/Down          Choose Play / Quit
Left/Right       Cycle audio device
F                 Cycle SFX volume
Enter             Confirm
Esc               Quit

In game:
Arrow keys        Move cursor / aim cardinal Fart direction
Enter             Select / move / confirm action
F                 Enter or cancel Fart mode
S                 Save game
L                 Load game
Keypad diagonals  Aim NE / SE / SW / NW
Esc               Quit
```

## Build

### Host verification

```sh
make test-build9
```

This reruns the permanent Build 4–6 rule suites, Build 8 audio suite, Build 9 persistence tests, and a scripted save → fart-push → load → fart-push smoke path.

Artifacts:

```text
build/host/chessfart_build9.ppm
build/host/chessfart_build9_title.ppm
build/host/chessfart_build9_audio.log
build/host/chessfart_build9_fart.wav
build/host/CHESSFRT.SAV
build/host/CHESSFRT.CFG
```

### DOS / Open Watcom

```bat
wmake -f makefile.build9.dos dos
```

or:

```bat
scripts\build_dos_build9.bat
```

Expected output: `build\dos\CHESSFRT.EXE`.

Open Watcom/DOSBox and real Sound Blaster-compatible hardware are not installed in the environment used for this build, so DOS runtime validation remains the platform integration check.

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
- [`docs/ROADMAP.md`](docs/ROADMAP.md)
- [`docs/TEST_PLAN.md`](docs/TEST_PLAN.md)
- [`docs/BUILD_7.md`](docs/BUILD_7.md)
- [`docs/BUILD_8.md`](docs/BUILD_8.md)
- [`docs/BUILD_9.md`](docs/BUILD_9.md)

## Design pillars

1. **Still chess.** Normal chess knowledge matters.
2. **No hidden gameplay randomness.** Gas state and fart outcomes are predictable.
3. **Fast to read.** The 320x200 screen must communicate state instantly.
4. **Juicy DOS presentation.** VGA effects and crude digital audio are part of the identity.
5. **Small enough to finish.** One polished board and one strong ruleset beat oversized scope.

**CHESS FART** — *Check. Mate. Ventilate.*
