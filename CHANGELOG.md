# Changelog

All notable project milestones will be recorded here.

## Unreleased

### Build 6 — Fart displacement

Added:

- actual one-square Fart displacement for friendly and enemy pieces
- `PUFF`, `PUSH`, `BLOCKED`, `PROMOTE`, and `INVALID` preview states
- confirmed blocked pushes that still spend Gas and consume the turn
- scratch-board king-safety validation for displacement
- Fart responses that can resolve check
- enemy-king displacement and check creation
- king-adjacency rejection through acting-king safety
- castling-right loss when a home king/rook is displaced
- en-passant expiry and clock accounting on every Fart Action
- pushed-pawn promotion with Q/R/B/N choice
- pushed-piece Gas preservation
- complete reversible `CfFartAction` state
- game-status awareness of legal Fart responses
- Build 6 push/destination VGA overlays
- deterministic host push demo
- Build 6 regression suite
- GitHub Actions host CI
- `docs/BUILD_6.md`

Verified locally:

- Build 6 core, UI integration and tests compile with strict C89 warnings-as-errors
- displacement-specific test harness passes push, blocked, check-safety, king, castling, promotion and undo cases

### Build 5 — Gas system

Added per-piece Gas, Gas earning/cap, castling/promotion Gas handling, board Gas display, Fart mode, eight-direction targeting, empty-square PUFF, Gas-sensitive repetition, reversible Fart state, and Build 5 tests.

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

Build 6 complete in source. Build 7 — presentation pass — is next.
