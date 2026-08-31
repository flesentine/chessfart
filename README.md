# Chess Fart

**Chess Fart** is a deliberately ridiculous 256-color VGA strategy game: real chess rebuilt as a loud early-1990s DOS game where pieces build Gas and use directional fart blasts as tactical abilities.

The design goal is **real strategy first, toilet humor second**.

## Project status

**Build 8 — Audio: complete in source/host validation.**

Chess Fart now has its complete Build 6 rules, Build 7 VGA presentation and the first working DOS-style audio layer:

- 8-bit unsigned mono PCM at 11025 Hz
- five procedurally generated original fart voices
- move/capture/select/invalid/check/checkmate/promotion/menu SFX
- Sound Blaster DSP detection/playback source for DOS
- PC-speaker fallback
- AUTO / SB / PCSPK / NONE device selection
- OFF / LOW / MED / HIGH SFX levels
- audio status on the title screen and in-game panel
- host audio event log plus a playable generated fart WAV

No chess or Fart rule behavior changed in Build 8.

See [`docs/BUILD_8.md`](docs/BUILD_8.md) for implementation and verification details.

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
Keypad diagonals  Aim NE / SE / SW / NW
Esc               Quit
```

## Build

### Host verification

```sh
make test-build8
```

This reruns the permanent Build 4–6 rule suites, runs the Build 8 audio tests, executes the Build 7-style visual push demo with audio hooks, and writes:

```text
build/host/chessfart_build8.ppm
build/host/chessfart_build8_title.ppm
build/host/chessfart_build8_audio.log
build/host/chessfart_build8_fart.wav
```

### DOS / Open Watcom

```bat
wmake -f makefile.build8.dos dos
```

or:

```bat
scripts\build_dos_build8.bat
```

Expected output: `build\dos\CHESSFRT.EXE`.

Open Watcom/DOSBox and real Sound Blaster-compatible hardware are not installed in the environment used for this build, so DOS audio timing remains the platform integration check.

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
- [`docs/BUILD_8.md`](docs/BUILD_8.md)

## Design pillars

1. **Still chess.** Normal chess knowledge matters.
2. **No hidden gameplay randomness.** Gas state and fart outcomes are predictable.
3. **Fast to read.** The 320x200 screen must communicate state instantly.
4. **Juicy DOS presentation.** VGA effects and crude digital audio are part of the identity.
5. **Small enough to finish.** One polished board and one strong ruleset beat oversized scope.

**CHESS FART** — *Check. Mate. Ventilate.*
