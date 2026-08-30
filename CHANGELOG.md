# Changelog

All notable project milestones will be recorded here.

## Unreleased

### Build 5 — Gas system

Added:

- parallel per-piece Gas state from 0 to 3
- +1 Gas on legal quiet move and +2 total on capture
- Gas cap at 3
- castling Gas award to both king and rook
- Gas inheritance through promotion
- Gas-aware move/unmake wrappers
- three-segment Gas strips on every occupied VGA square
- selected/cursor Gas readout in the side panel
- F-key Fart mode
- eight directional targets
- no-target/off-board PUFF action
- Gas spending, en-passant expiry and turn accounting on PUFF
- reversible `CfFartAction` state
- Gas-sensitive repetition keys/history
- Build 5 Gas regression suite
- `docs/BUILD_5.md`

Verified:

- strict C89 build succeeds with warnings-as-errors
- the full Build 4 standard-chess/perft suite remains green without changing its chess core
- normal move/capture/castling/promotion Gas rules pass
- PUFF spend/unmake/check-safety tests pass
- repetition distinguishes otherwise identical positions with different Gas
- host interaction legally charges a knight and performs `PUFF N`
- Build 5 framebuffer preview is generated successfully

Occupied-piece displacement remains intentionally deferred to Build 6.

### Build 4 — complete standard chess

Added castling, en passant, four-way promotion, checkmate/stalemate, draw state, repetition, counters, complete special-state make/unmake, promotion UI, and multi-position perft regression testing.

### Build 3 — legal chess movement

Added ordinary legal movement, attack/check detection, king-safety filtering, captures, legal-move highlights, make/unmake records, turn enforcement, and opening perft validation.

### Build 2 — cursor and pieces

Added the 32-piece starting-position board model, placeholder VGA pieces, arrow-key cursor, persistent selection, side-panel piece information, and host board-state tests.

### Build 1 — VGA boot

Added the C89 application shell, Mode 13h DOS backend, software backbuffer, VGA palette, bitmap font, keyboard input abstraction, and host framebuffer smoke test.

### Build 0 — foundation

Added the game concept, VGA target, deterministic Gas/Fart ruleset, master plan, art/audio/architecture specs, roadmap, test plan, toolchain plan, and project conventions.

### Current phase

Build 5 complete in source. Build 6 — Fart displacement — is next.
