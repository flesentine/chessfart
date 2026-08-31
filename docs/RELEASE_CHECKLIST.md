# Chess Fart 1.0.0-rc1 — Release Checklist

## Automated RC1 gate

- [x] permanent Build 4 standard-chess regression suite
- [x] permanent Build 5 Gas regression suite
- [x] permanent Build 6 Fart displacement regression suite
- [x] permanent Build 8 audio regression suite
- [x] permanent Build 9 persistence regression suite
- [x] permanent Build 10 CPU regression suite
- [x] permanent Build 11 UX regression suite
- [x] Build 11 scripted host game and VGA/audio/save artifacts
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

RC1 CI uses the maintained `open-watcom/setup-watcom@v1` action with `version: 2.0` and `target: dos`; `BUILDINFO.TXT` distinguishes that actual CI compiler provenance from the separately pinned reference snapshot.

## Bug burn-down policy

RC1 ships with zero known P0/P1 issues in the host/rules/build/package gates.

P0: data corruption, impossible legal game completion, crash on normal startup, or broken release package.

P1: deterministic rules error, save/load mismatch, CPU illegal action, unusable keyboard flow, or reference DOS build failure.

P2/platform notes may remain when documented in `release/KNOWNISS.TXT`, especially emulator/hardware timing differences that cannot be exhaustively validated in CI.

## Manual/community matrix after RC cut

- [ ] physical 386DX/486 with VGA
- [ ] MS-DOS 6.x
- [ ] FreeDOS
- [ ] Sound Blaster 1.x/2.0/Pro/16 variants
- [ ] Microsoft-compatible DOS mouse drivers
- [ ] DOSBox-X interactive session
- [ ] DOSBox Staging interactive session
- [ ] full human match at EASY/MED/HARD on slow DOS-class CPU profiles

These unchecked items are compatibility expansion, not hidden automated-test failures. See `docs/COMPATIBILITY.md`.

## Release contents

The ZIP contains one `CHESSFRT/` directory with:

- `CHESSFRT.EXE`
- `DOSBOX.CONF`
- `README.TXT`
- `KEYS.TXT`
- `KNOWNISS.TXT`
- `BUILDINFO.TXT`
