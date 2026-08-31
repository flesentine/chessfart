# Chess Fart 1.0.0-rc1 — Compatibility Matrix

This document distinguishes automated evidence from targets that still require physical/manual testing.

| Environment / subsystem | RC1 status | Evidence / scope |
| --- | --- | --- |
| Ubuntu GitHub Actions + strict C89 host build | VERIFIED | `make test-build12`; warnings are errors |
| Standard chess / perft regressions | VERIFIED | Permanent Build 4 suite |
| Gas and Fart rules | VERIFIED | Permanent Build 5/6 suites |
| Audio generation / host hooks | VERIFIED | Permanent Build 8 suite |
| Save/config round-trip and validation | VERIFIED | Permanent Build 9 suite |
| CPU legal move/Fart search | VERIFIED | Permanent Build 10 suite |
| UX/history/hit testing | VERIFIED | Permanent Build 11 suite |
| Open Watcom 2.0 16-bit DOS compile | VERIFIED | CI builds `CHESSFRT.EXE` with `-bt=dos -ml -3` |
| DOSBox VGA Mode 13h smoke | VERIFIED | DOS-resident smoke initializes/draws/presents VGA |
| DOSBox INT 33h mouse probe | VERIFIED (probe) | DOS-resident smoke calls mouse init/shutdown; full interactive pointer flow is manual |
| DOSBox Sound Blaster/PC-speaker initialization | VERIFIED (smoke) | AUTO audio initialized in DOS smoke with BLASTER A220/I7/D1/H5/T6 |
| Release ZIP structure/integrity | VERIFIED | `unzip -t`, required-entry checks and SHA-256 |
| DOSBox interactive full match | MANUAL TARGET | Automated smoke is not a substitute for a human full session |
| DOSBox-X / DOSBox Staging | UNVERIFIED RC1 | Expected compatible, not claimed tested |
| FreeDOS | UNVERIFIED RC1 | Manual/community target |
| MS-DOS 6.x | UNVERIFIED RC1 | Manual/community target |
| Physical 386DX / 486 VGA hardware | UNVERIFIED RC1 | Manual/community target |
| Real Sound Blaster 1.x / 2.0 / Pro / 16 | UNVERIFIED RC1 | Direct-DAC timing and BLASTER combinations need hardware coverage |
| Microsoft-compatible DOS mouse drivers | UNVERIFIED RC1 | INT 33h code exists; driver/hardware combinations need manual coverage |

## Reference DOS target

- 80386 or newer CPU
- VGA-compatible graphics
- DOS with enough conventional memory for the 16-bit large-model executable
- keyboard required
- mouse optional
- Sound Blaster-compatible audio optional; PC speaker fallback available

## Performance notes

The CPU uses bounded iterative deepening. EASY/MED/HARD have hard node/time limits, so slower hardware degrades think depth/time predictably instead of searching indefinitely.

GitHub Actions host measurements for RC1:

| Fixture | Completed depth | Nodes | Cutoffs | CI elapsed |
| --- | ---: | ---: | ---: | ---: |
| EASY start | 1 | 20 | 0 | 0 ms |
| MED start | 2 | 440 | 0 | 1 ms |
| HARD start | 3 | 7,320 | 325 | 27 ms |
| Fart-heavy MED | 2 | 6,240 | 0 | 59 ms |

These timings characterize the CI host implementation only. The node counts and depth limits are the useful cross-platform measurements; physical DOS timing must be measured separately.

## Audio caveat

RC1's Sound Blaster backend intentionally uses simple 8-bit direct-DAC polling instead of DMA. This keeps the implementation compact, but playback pacing depends more strongly on machine/emulator timing than a DMA mixer would. AUTO falls back to PC speaker if Sound Blaster initialization fails.
