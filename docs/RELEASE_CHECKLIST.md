# Chess Fart 1.0.0 — Release Checklist

## Automated release gate

- [x] permanent Build 4 standard-chess regression suite
- [x] permanent Build 5 Gas regression suite
- [x] permanent Build 6 Fart displacement regression suite
- [x] permanent Build 8 audio regression suite
- [x] permanent Build 9 persistence regression suite
- [x] permanent Build 10 CPU regression suite
- [x] deterministic tactical CPU Fart regressions for check escape, forcing check, castling-right damage and pushed promotion
- [x] Fart-aware alpha-beta edge-bonus window correction
- [x] permanent Build 11 UX regression suite
- [x] Build 11 scripted host game and VGA/audio/save artifacts
- [x] browser/WASM build and runtime smoke
- [x] Chromium mouse/Fart/history/save-load playtest
- [x] deterministic full Chromium games at EASY/MED/HARD, each reaching a terminal result with zero browser errors and exercising live Fart mechanics
- [x] Build 12 release audit
- [x] Build 12 host CPU/memory profile report
- [x] Open Watcom 2.0 environment through maintained `setup-watcom` action
- [x] real 16-bit DOS `CHESSFRT.EXE` compile
- [x] DOSBox execution of `DOSSMOKE.EXE`
- [x] DOS smoke touches VGA Mode 13h, mouse INT 33h and audio initialization
- [x] distributable ZIP contains all required files
- [x] ZIP integrity check
- [x] SHA-256 generated for final ZIP

## Reproducibility reference

The Open Watcom `2026-07-13-Build` `ow-snapshot.tar.xz` asset is recorded as a reproducibility reference with SHA-256:

`35e9c56ca93234ba0b8b596387815399317ea0548aeac1318f0674dca3921314`

Release CI uses the maintained `open-watcom/setup-watcom@v1` action with `version: 2.0` and `target: dos`; `BUILDINFO.TXT` distinguishes that actual CI compiler provenance from the separately pinned reference snapshot.

## Bug burn-down policy

1.0.0 ships with zero known P0/P1 issues in the automated rules/build/browser/package gates.

P0: data corruption, impossible legal game completion, crash on normal startup, or broken release package.

P1: deterministic rules error, save/load mismatch, CPU illegal action, unusable keyboard flow, or reference DOS build failure.

P2/platform notes may remain when documented in `release/KNOWNISS.TXT`, especially emulator/hardware timing differences that cannot be exhaustively validated in CI.

## Manual/community compatibility expansion

These are deliberately **not** marked verified by the 1.0 release:

- [ ] physical 386DX/486 with VGA
- [ ] MS-DOS 6.x
- [ ] FreeDOS
- [ ] Sound Blaster 1.x/2.0/Pro/16 variants
- [ ] Microsoft-compatible DOS mouse drivers
- [ ] DOSBox-X interactive session
- [ ] DOSBox Staging interactive session
- [ ] full human match at EASY/MED/HARD on slow DOS-class CPU profiles

The unchecked items are documented compatibility-expansion targets, not hidden automated-test failures. See `docs/COMPATIBILITY.md`.

## Release contents

The ZIP contains one `CHESSFRT/` directory with:

- `CHESSFRT.EXE`
- `DOSBOX.CONF`
- `README.TXT`
- `KEYS.TXT`
- `KNOWNISS.TXT`
- `BUILDINFO.TXT`
