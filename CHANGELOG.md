# Changelog

All notable project milestones will be recorded here.

## Unreleased

### Build 3 — legal chess movement

Added:

- pseudo-legal movement generation for all six piece types
- board turn state
- pawn one/two-step movement and diagonal capture
- slider ray/blocker handling
- knight jumps and king steps
- attack detection and check detection
- king-safety filtering through scratch-board simulation
- legal destination markers and capture outlines
- ordinary captures
- make/unmake move records
- source/destination move interaction
- pinned-piece and king-safety regression tests
- `docs/BUILD_3.md`

Verified:

- strict C89 compilation succeeds with `-std=c89 -pedantic -Wall -Wextra -Werror`
- starting position reports 20 legal ordinary moves per side
- opening perft sanity matches 20 / 400 / 8,902 nodes at depths 1 / 2 / 3
- capture, check, pin, king-safety, turn, and unmake tests pass
- scripted host interaction legally moves E2 to E4
- host framebuffer preview is generated successfully

Deferred to Build 4: castling, en passant, promotion, checkmate/stalemate, and draw state.

### Build 2 — cursor and pieces

Added the 32-piece starting-position board model, placeholder VGA pieces, arrow-key cursor, persistent selection, side-panel piece information, and host board-state tests.

### Build 1 — VGA boot

Added the C89 application shell, Mode 13h DOS backend, software backbuffer, VGA palette, bitmap font, keyboard input abstraction, and host framebuffer smoke test.

### Build 0 — foundation

Added the game concept, VGA target, deterministic Gas/Fart ruleset, master plan, art/audio/architecture specs, roadmap, test plan, toolchain plan, and project conventions.

### Current phase

Build 3 complete in source. Build 4 — complete standard chess — is next.
