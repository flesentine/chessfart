# Changelog

All notable project milestones are recorded here.

## Unreleased

### Build 2 — cursor and pieces

Added:

- platform-independent 8x8 board model
- standard 32-piece starting position
- piece/color/type helpers and guarded square access
- placeholder VGA silhouettes for pawn, knight, bishop, rook, queen, and king
- distinct White/Black piece palette treatment
- cyan board cursor starting on E2
- DOS arrow-key decoding
- Enter selection/deselection
- persistent magenta selected-square highlight
- live cursor coordinate and piece details in the side panel
- deterministic host input script demonstrating separate cursor/selection states
- Build 2 board-state tests
- `docs/BUILD_2.md`

Verified:

- `make test-build2` passes with `-std=c89 -pedantic -Wall -Wextra -Werror`
- starting position contains exactly 32 pieces
- known king/queen/rook/pawn squares are correct
- invalid board coordinates safely return null
- host backend produces a valid 320x200 Build 2 framebuffer preview

Not yet verified in this environment:

- Open Watcom compilation and DOSBox runtime because those tools are not installed here

### Build 1 — VGA boot

Added the C89 application shell, Open Watcom DOS target, Mode 13h entry/exit, 64,000-byte backbuffer, VGA palette programming, framebuffer primitives, bitmap font, static board, Escape handling, and deterministic host renderer.

### Build 0 — foundation

Added the game concept, VGA target, deterministic Gas/Fart ruleset, master plan, art/audio/architecture specs, roadmap, tests, toolchain plan, decisions, and contribution conventions.

### Current phase

Build 2 complete in source. Build 3 — normal chess movement — is next.
