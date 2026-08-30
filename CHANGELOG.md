# Changelog

All notable project milestones will be recorded here.

## Unreleased

### Build 7 — presentation pass

Added:

- dedicated VGA title screen and keyboard menu
- Royal Basement palette override
- improved six-piece silhouettes without letter labels
- board bevel and rank/file coordinates
- shadowed header treatment
- five-frame fart/push animation wrapper
- gas plume particles
- impact palette flash
- one-pixel alternating board-frame shake
- deterministic Build 7 host preview
- CI upload of the Build 7 PPM artifact
- `docs/BUILD_7.md`

Architecture:

- Build 7 embeds the Build 6 game loop instead of copying its rules logic
- rendering is redirected to the Build 7 presentation layer
- fart execution is wrapped for before/after animation only
- `board.c` and `gas.c` rule outcomes remain the Build 6 source of truth

Verification target:

- strict C89 host build with warnings-as-errors
- complete Build 4 chess regression/perft suite
- complete Build 5 Gas suite
- complete Build 6 displacement suite
- scripted title/game/fart-push presentation smoke run

### Build 6 — fart displacement

Added one-square displacement, blocked outcomes, full king-safety filtering, castling-right displacement effects, pushed-pawn promotion, check creation/resolution, Gas-aware history and reversible fart actions.

### Build 5 — Gas system

Added per-piece Gas, earning/spending, Fart mode, eight-direction targeting, PUFF turns and Gas-aware repetition.

### Build 4 — complete standard chess

Added castling, en passant, four-way promotion, checkmate/stalemate, draw state, repetition, counters and complete special-state make/unmake.

### Build 3 — legal chess movement

Added ordinary legal movement, attack/check detection, king-safety filtering, captures and perft validation.

### Build 2 — cursor and pieces

Added the 32-piece starting board, VGA pieces, keyboard cursor and selection state.

### Build 1 — VGA boot

Added the Mode 13h shell, backbuffer, palette, font and input abstraction.

### Build 0 — foundation

Added the design, architecture, rules and milestone documentation.

### Current phase

Build 7 presentation complete in source. Build 8 — audio — is next.
