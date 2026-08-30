# Chess Fart — Roadmap

Milestones are numbered as builds so future work can proceed `build 0`, `build 1`, etc.

## Build 0 — Repository foundation

**Status: complete**

Design, architecture, art/audio direction, toolchain and test plan established.

## Build 1 — VGA boot

**Status: complete in source; host smoke test passes**

Mode 13h shell, software backbuffer, palette, bitmap font, static board and clean exit.

## Build 2 — Cursor and pieces

**Status: complete in source; host tests pass**

32-piece starting board, placeholder sprites, keyboard cursor and selection state.

## Build 3 — Normal chess movement

**Status: complete in source; host rules tests pass**

All ordinary piece movement, captures, attack/check detection, king safety and apply/unapply.

## Build 4 — Complete standard chess

**Status: complete in source; host rules tests pass**

Castling, en passant, four-way promotion, mate/stalemate, draw state and multi-position perft regression.

## Build 5 — Gas system

**Status: complete in source; host gas/rules tests pass**

Per-piece Gas 0–3, earning, board Gas UI, Fart mode, eight directions, empty-square PUFF and Gas-aware repetition.

## Build 6 — Fart displacement

**Status: complete in source**

Delivered:

- one-square push of friendly or enemy pieces
- blocked/edge push handling
- full acting-king legality after displacement
- check resolved or created by Fart Action
- king displacement
- castling-right displacement effects
- en-passant expiry
- pushed-pawn promotion
- pushed-piece Gas preservation
- Gas-aware history after pushes
- reversible Fart Actions
- Build 6 UI preview overlays
- displacement regression suite

Exit criterion met in source: every documented core Fart Action outcome is executable and represented in tests.

Remaining platform integration check: Open Watcom + DOSBox runtime validation.

## Build 7 — Presentation pass

**Status: next**

Deliverables: final-ish board palette, improved piece silhouettes, gas plume animation, push animation, screen shake/palette flashes, title/menu screens and improved fonts.

Exit criterion: the mechanics read clearly and feel like a finished VGA game rather than a debug prototype.

## Build 8 — Audio

Deliverables: Sound Blaster initialization, PCM playback, fart sample bank, UI/move/check SFX, PC speaker fallback and audio options.

## Build 9 — Save/load and config

Deliverables: config file, save UI, versioned save data, exact Gas/history restoration and error handling.

## Build 10 — CPU opponent

Deliverables: negamax/alpha-beta, evaluation, action ordering, Gas/Fart awareness, difficulty settings and move-time cap.

## Build 11 — UX/polish

Deliverables: move history, help/rules, polished promotion/checkmate presentation, attract/demo loop, credits, keyboard completeness and mouse support.

## Build 12 — Release candidate

Deliverables: compatibility matrix, profiling, bug burn-down, packaged DOS ZIP, sample DOSBox config, final README and version stamp.

## Post-v1 ideas

Alternate boards/pieces, replay browser, Turbo Gas, puzzle challenges, AI personalities, null-modem multiplayer, OPL2 expansion, and a shareware-style episode screen remain post-v1.
