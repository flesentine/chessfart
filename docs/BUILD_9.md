# Chess Fart — Build 9: Save/Load and Config

## Status

**Complete when Build 4–9 CI is green.**

Build 9 adds persistence without changing chess, Gas/Fart, presentation, or audio rules.

## Save file

Default DOS filename:

```text
CHESSFRT.SAV
```

The save is a versioned text format (`CHESSFART_SAVE 1`) written field-by-field rather than as a raw C struct. That keeps saves portable across compiler padding/layout differences.

Saved state includes:

- all 64 pieces and colors
- side to move
- castling rights
- en-passant square
- halfmove/fullmove counters
- all 64 Gas values
- exact Gas-aware repetition history keys

Selection/cursor/menu animation state is intentionally not part of game truth. Loading clears transient UI modes while restoring the exact rules position/history.

## Transactional load behavior

Loads are parsed and validated into temporary structures first. The live game is replaced only after the entire file passes validation.

Rejected cases include wrong magic, unsupported version, truncated data, invalid piece/color/Gas values, invalid state fields, and invalid/oversized history. A failed load leaves the current game untouched.

## Save writes

Writes go to `*.TMP` first and are renamed into place only after a complete successful write/close.

## Config

Default DOS filename:

```text
CHESSFRT.CFG
```

Version 1 stores the Build 8 audio settings: device, SFX level, and music-level field. Missing or corrupt config falls back to defaults. Title-screen audio changes are persisted. Audio configuration is not part of repetition/game-position identity.

## Controls

```text
S     Save current game
L     Load current game
```

Existing chess/Fart controls are unchanged.

## Host verification

```sh
make test-build9
```

The target runs the permanent Build 4, 5, 6, and 8 suites, the Build 9 persistence suite, and a scripted save → fart-push → load → fart-push smoke run.

Host artifacts include:

```text
build/host/chessfart_build9_title.ppm
build/host/chessfart_build9.ppm
build/host/chessfart_build9_audio.log
build/host/chessfart_build9_fart.wav
build/host/CHESSFRT.SAV
build/host/CHESSFRT.CFG
```

## DOS build

```bat
wmake -f makefile.build9.dos dos
```

or:

```bat
scripts\build_dos_build9.bat
```

Expected executable: `build\dos\CHESSFRT.EXE`.

Open Watcom/DOSBox remains the platform integration check.

## Build 9 exit criterion

A saved match can be loaded with identical board, Gas, counters, and repetition history; corrupt/unsupported saves fail safely; and audio settings persist separately.

## Build 10 handoff

Build 10 is the CPU opponent: negamax/alpha-beta, action ordering, evaluation, Gas/Fart awareness, difficulty levels, and a move-time cap.
