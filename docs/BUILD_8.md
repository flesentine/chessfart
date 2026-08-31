# Chess Fart — Build 8: Audio

## Status

**Complete in source/host validation when Build 4–8 CI is green.**

Build 8 adds the first real audio layer without changing any chess, Gas, Fart or presentation rule result from Builds 4–7.

## Audio architecture

The game-facing audio API is platform-neutral. `src/game/audio.c` owns device/volume state, event routing, deterministic fart-voice selection and volume scaling. Platform backends only handle output.

Build 8 supports:

- `AUTO`
- `SB`
- `PCSPK`
- `NONE`

SFX levels are `OFF`, `LOW`, `MED` and `HIGH`. Music level is represented in configuration but remains off because OPL music is optional and deliberately not allowed to block the audio milestone.

Build 9 can persist these settings without changing the Build 8 API.

## Original procedural PCM bank

No third-party fart samples are shipped. Build 8 generates its entire placeholder sample bank in C as unsigned 8-bit mono PCM at 11025 Hz.

Five fart voices are available:

- `TOOT_SHORT`
- `RASP_MEDIUM`
- `WET_LONG`
- `ROYAL_BOOM`
- `TINY_SQUEAK`

The generator combines low-frequency pulse waves, deterministic pseudo-noise, envelopes and occasional crackle. Selection is cosmetic and deterministic from an audio-only counter, so it cannot alter game state.

The same generator creates the current UI/move sounds:

- cursor tick
- select click
- invalid bonk
- move tap
- capture clack
- pressure hiss
- check alert
- checkmate fanfare
- promotion chime
- menu confirm click

These are intentionally small placeholder assets that can later be replaced by original recorded samples through the same event API.

## Sound Blaster backend

The DOS backend reads the conventional `BLASTER` environment variable for the `A` base address and defaults to `0x220` when absent.

Initialization performs the Sound Blaster DSP reset handshake and expects the standard `0xAA` acknowledgement.

For Build 8, digital playback uses the conservative DSP direct-DAC command (`0x10`) with software pacing. This keeps the first implementation small, polling-based and independent of DMA/IRQ setup. It is appropriate for the short one-shot effects used by this milestone, but DMA streaming is the obvious future optimization if DOSBox/real-hardware testing shows timing or CPU-use problems.

All PCM is 8-bit unsigned mono at 11025 Hz.

## PC-speaker fallback

When `AUTO` cannot initialize the Sound Blaster DSP, the DOS backend falls back to PIT/channel-2 PC-speaker cues.

The fallback includes:

- cursor/select clicks
- move/capture tones
- invalid bonk
- descending fart buzz
- check alarm
- checkmate arpeggio
- promotion arpeggio

Explicit `NONE` and SFX `OFF` remain silent while gameplay stays identical.

## Audio options

The Build 8 title screen keeps the Play/Quit menu and adds live audio status.

- `Left/Right` cycles device: AUTO → SB → PCSPK → NONE
- `F` cycles SFX volume: OFF → LOW → MED → HIGH
- `Up/Down` selects Play/Quit
- `Enter` confirms

The in-game panel shows the resolved audio device and current SFX level.

## Gameplay event hooks

Build 8 wraps the existing Build 6/7 loop rather than copying it.

Audio cues are attached to:

- cursor movement
- selection/confirmation
- invalid messages
- normal moves
- captures
- entering Fart mode
- PUFF/PUSH/BLOCKED fart results
- promotion, including pushed-pawn promotion
- check
- checkmate
- menu confirmation

Fart PCM is triggered after the Build 6 rule action has been validated and before the Build 7 visual fart animation is rendered. The presentation layer now exports its existing five-frame fart animator so audio can synchronize without duplicating rules.

## Host backend

The host backend does not attempt to use desktop audio hardware. Instead it provides deterministic verification artifacts:

```text
build/host/chessfart_build8_audio.log
build/host/chessfart_build8_fart.wav
```

The log records resolved device, event, selected sample, sample length and level. The WAV contains the most recent generated fart sample so the PCM output is directly inspectable/listenable.

The normal Build 8 title and game PPM previews are preserved too.

## Verification

Run:

```sh
make test-build8
```

The target performs:

1. complete Build 4 standard-chess regression/perft suite
2. complete Build 5 Gas suite
3. complete Build 6 displacement suite
4. Build 8 audio unit suite
5. deterministic Build 8 title/game/fart-push smoke run
6. existence checks for both PPMs, the audio log and the WAV

Build 8 audio tests verify:

- default device/volume configuration
- device cycling
- five distinct non-empty fart PCM voices
- event accounting
- fart sample rotation
- mute behavior
- host Sound Blaster simulation path

## DOS build

```bat
wmake -f makefile.build8.dos dos
```

or:

```bat
scripts\build_dos_build8.bat
```

Expected executable:

```text
build\dos\CHESSFRT.EXE
```

Open Watcom/DOSBox and real Sound Blaster-compatible hardware are not available in this environment, so DSP timing, fallback behavior and perceived volume still require platform validation.

## Build 8 exit criterion

**Met when CI is green:** the game has a platform-neutral sound event layer, generated 8-bit PCM fart/UI samples, a source-ready Sound Blaster backend, PC-speaker fallback, runtime device/SFX options, and host-verifiable audio output without altering Build 7 visuals or Build 6 rules.

## Build 9 handoff

Build 9 should persist the Build 8 device/SFX settings and add versioned save/load/config state. It should also preserve the exact Gas/history state from Build 6 while keeping audio configuration outside game-position identity.
