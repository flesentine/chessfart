# Chess Fart 1.0.0-rc1 — Release Checklist

## Release blockers

A Build 12 candidate is not shippable unless every automated item below is green.

- [ ] permanent Build 4 standard-chess regression suite
- [ ] permanent Build 5 Gas regression suite
- [ ] permanent Build 6 Fart displacement regression suite
- [ ] permanent Build 8 audio regression suite
- [ ] permanent Build 9 persistence regression suite
- [ ] permanent Build 10 CPU regression suite
- [ ] permanent Build 11 UX regression suite
- [ ] Build 11 scripted host game and VGA/audio/save artifacts
- [ ] Build 12 release audit
- [ ] Build 12 host CPU/memory profile report
- [ ] pinned Open Watcom 2026-07-13 snapshot checksum verification
- [ ] real 16-bit DOS `CHESSFRT.EXE` compile
- [ ] DOSBox execution of `DOSSMOKE.EXE`
- [ ] DOS smoke touches VGA Mode 13h, mouse INT 33h and audio initialization
- [ ] distributable ZIP contains all required files
- [ ] ZIP integrity check
- [ ] SHA-256 generated for final ZIP

## Bug burn-down policy

RC1 ships with zero known P0/P1 issues in the host/rules/build/package gates.

P0: data corruption, impossible legal game completion, crash on normal startup, or broken release package.

P1: deterministic rules error, save/load mismatch, CPU illegal action, unusable keyboard flow, or reference DOS build failure.

P2/platform notes may remain when documented in `release/KNOWNISS.TXT`, especially emulator/hardware timing differences that cannot be exhaustively validated in CI.

## Manual/community matrix after RC cut

- physical 386DX/486 with VGA
- MS-DOS 6.x / FreeDOS
- Sound Blaster 1.x/2.0/Pro/16 variants
- Microsoft-compatible DOS mouse drivers
- DOSBox-X and DOSBox Staging interactive sessions
- different host CPU speeds at EASY/MED/HARD

## Release contents

The ZIP must contain one `CHESSFRT/` directory with:

- `CHESSFRT.EXE`
- `DOSBOX.CONF`
- `README.TXT`
- `KEYS.TXT`
- `KNOWNISS.TXT`
- `BUILDINFO.TXT`
