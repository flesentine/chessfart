# Changelog

All notable project milestones will be recorded here.

## Unreleased

### Build 4 — complete standard chess

Added:

- persistent castling-right state and both castling moves
- castling-through-check/path validation
- en-passant target state and en-passant captures
- en-passant self-check filtering
- four-way pawn promotion: queen, rook, bishop, knight
- interactive VGA promotion chooser
- checkmate and stalemate status
- halfmove/fullmove counters
- fifty-move draw handling
- threefold repetition history with side/castling/effective-en-passant state
- insufficient-material draw detection
- complete special-state make/unmake restoration
- terminal game-state handling in the VGA shell
- `docs/BUILD_4.md`

Verified:

- strict C89 compilation succeeds with `-std=c89 -pedantic -Wall -Wextra -Werror`
- opening perft matches 20 / 400 / 8,902 / 197,281 through depth 4
- Kiwipete matches 48 / 2,039 / 97,862 through depth 3
- en-passant reference position matches 14 / 191 / 2,812 / 43,238 / 674,624 through depth 5
- promotion/castling reference position matches 6 / 264 / 9,467 / 422,333 through depth 4
- castling, en passant, promotion, mate, stalemate, draw, repetition, counters, and unmake fixtures pass
- scripted host interaction legally moves E2 to E4 and renders Build 4 state

### Build 3 — legal chess movement

Added ordinary legal movement, attack/check detection, king-safety filtering, captures, legal-move highlights, make/unmake records, turn enforcement, and opening perft validation.

### Build 2 — cursor and pieces

Added the 32-piece starting-position board model, placeholder VGA pieces, arrow-key cursor, persistent selection, side-panel piece information, and host board-state tests.

### Build 1 — VGA boot

Added the C89 application shell, Mode 13h DOS backend, software backbuffer, VGA palette, bitmap font, keyboard input abstraction, and host framebuffer smoke test.

### Build 0 — foundation

Added the game concept, VGA target, deterministic Gas/Fart ruleset, master plan, art/audio/architecture specs, roadmap, test plan, toolchain plan, and project conventions.

### Current phase

Build 4 complete in source. Build 5 — Gas system — is next.
