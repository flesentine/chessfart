# Chess Fart 1.0.0 — Compatibility Matrix

This document distinguishes automated evidence from targets that still require physical/manual testing. “UNVERIFIED” means exactly that; the 1.0 release does not claim compatibility evidence it does not have.

| Environment / subsystem | 1.0 status | Evidence / scope |
| --- | --- | --- |
| Ubuntu GitHub Actions + strict C89 host build | VERIFIED | `make test-build12`; warnings are errors |
| Standard chess / perft regressions | VERIFIED | Permanent Build 4 suite |
| Gas and Fart rules | VERIFIED | Permanent Build 5/6 suites |
| Audio generation / host hooks | VERIFIED | Permanent Build 8 suite |
| Save/config round-trip and validation | VERIFIED | Permanent Build 9 suite plus Build 14.3 v2 mode round-trip, transactional rejection, legacy-v1 CPU fallback, and Build 14.5 exact persisted-state LOCAL rollback/replay |
| CPU legal move/Fart search | VERIFIED | Permanent Build 10 suite plus v7 tactical Fart regressions |
| Fart-aware alpha-beta pruning | VERIFIED | RC hardening corrected move-dependent edge-bonus child windows; full regression gate passes |
| UX/history/hit testing | VERIFIED | Permanent Build 11 suite |
| Browser/WASM build and runtime | VERIFIED | Emscripten build plus headless Chromium runtime smoke |
| Browser mouse/Fart/save-load flow | VERIFIED | Chromium gameplay playtest covers CPU/local mode selection, Black Fart, both-color promotion/Fart-promotion, terminal locks, and save/load |
| Browser full games EASY/MED/HARD | VERIFIED | Deterministic Chromium CPU games terminate cleanly with zero browser errors and live Fart actions |
| Browser local two-player full match | VERIFIED | 33-ply Opera Game from real title selection to 17.Rd8#, including three CHECK states, O-O-O, history UI, exact persisted-state rollback/replay and terminal lock |
| Replay timeline / terminal integrity | VERIFIED | Opera Game proves 17→19→1→18 frame lifecycle across Save/Load; deterministic EASY/MED/HARD games end with newest replay frame/status matching live terminal state |
| Replay file v1 | VERIFIED | Transactional CHESSFRT.RPL host corruption tests, 600-frame ring round-trip, and Chromium export/import transient-view restore |
| Replay memory budget | VERIFIED | Host profile: 118-byte snapshot, 30,232-byte live timeline, 60,464-byte live+temporary-import peak |
| Native 320x200 visual review | VERIFIED | 32 deterministic states covering CPU/local title/HUD/help/history, Fart, promotion, save/load and terminal presentations |
| Open Watcom 2.0 16-bit DOS compile | VERIFIED | CI builds `CHESSFRT.EXE` with `-bt=dos -ml -3` |
| DOSBox VGA Mode 13h smoke | VERIFIED | DOS-resident smoke initializes/draws/presents VGA |
| DOSBox INT 33h mouse probe | VERIFIED (probe) | DOS-resident smoke calls mouse init/shutdown; full interactive pointer flow is manual |
| DOSBox Sound Blaster/PC-speaker initialization | VERIFIED (smoke) | AUTO audio initialized in DOS smoke with BLASTER A220/I7/D1/H5/T6 |
| Release ZIP structure/integrity | VERIFIED | `unzip -t`, required-entry checks and SHA-256 |
| DOSBox interactive full human match | MANUAL TARGET | Automated smoke is not a substitute for a human DOSBox session |
| DOSBox-X / DOSBox Staging | UNVERIFIED 1.0 | Expected compatible, not claimed tested |
| FreeDOS | UNVERIFIED 1.0 | Manual/community target |
| MS-DOS 6.x | UNVERIFIED 1.0 | Manual/community target |
| Physical 386DX / 486 VGA hardware | UNVERIFIED 1.0 | Manual/community target |
| Real Sound Blaster 1.x / 2.0 / Pro / 16 | UNVERIFIED 1.0 | Direct-DAC timing and BLASTER combinations need hardware coverage |
| Microsoft-compatible DOS mouse drivers | UNVERIFIED 1.0 | INT 33h code exists; driver/hardware combinations need manual coverage |

## Reference DOS target

- 80386 or newer CPU
- VGA-compatible graphics
- DOS with enough conventional memory for the 16-bit large-model executable
- keyboard required
- mouse optional
- Sound Blaster-compatible audio optional; PC speaker fallback available

## Performance notes

The CPU uses bounded iterative deepening. EASY/MED/HARD have hard node/time limits, so slower hardware degrades think depth/time predictably instead of searching indefinitely.

Post-hardening GitHub Actions node measurements:

| Fixture | Completed depth | Nodes | Cutoffs |
| --- | ---: | ---: | ---: |
| EASY start | 1 | 20 | 0 |
| MED start | 2 | 440 | 0 |
| HARD start | 3 | 6,349 | 325 |
| Fart-heavy MED | 2 | 6,240 | 0 |

Host wall-clock timings vary by CI runner. Node counts and depth limits are the useful cross-platform measurements; physical DOS timing must be measured separately.

## Audio caveat

1.0's Sound Blaster backend intentionally uses simple 8-bit direct-DAC polling instead of DMA. This keeps the implementation compact, but playback pacing depends more strongly on machine/emulator timing than a DMA mixer would. AUTO falls back to PC speaker if Sound Blaster initialization fails.
