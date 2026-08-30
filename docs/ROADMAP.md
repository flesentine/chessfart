# Chess Fart — Roadmap

Milestones are numbered as builds so future work can proceed `build 0`, `build 1`, etc.

## Build 0 — Repository foundation

**Status: complete**

Exit criterion met: enough decisions exist to begin code without inventing architecture ad hoc.

## Build 1 — VGA boot

**Status: complete in source; host smoke test passes**

Delivered: Open Watcom project skeleton, `CHESSFRT.EXE` build target, Mode 13h enter/exit, software backbuffer, 256-color palette, bitmap font, static board, Escape handling, and host framebuffer smoke test.

Remaining integration check: compile/run the DOS target under Open Watcom + DOSBox.

## Build 2 — Cursor and pieces

**Status: complete in source; host tests pass**

Delivered: 32-piece starting board, placeholder sprites, keyboard cursor, selection state, side-panel information, and host board-state tests.

## Build 3 — Normal chess movement

**Status: complete in source; host rules tests pass**

Delivered: ordinary movement for all six pieces, attack/check detection, king-safety filtering, move highlights, captures, turns, apply/unapply, pinned-piece tests, and opening perft validation.

## Build 4 — Complete standard chess

**Status: complete in source; host rules tests pass**

Delivered:

- castling rights and both castling directions
- en-passant target/capture state
- queen/rook/bishop/knight promotion
- promotion-choice UI
- checkmate and stalemate
- halfmove/fullmove bookkeeping
- fifty-move draw
- threefold repetition
- insufficient-material draw
- special-state make/unmake
- multi-position perft regression suite

Exit criterion met in source/host validation: Pure Chess can play through all conventional move types and reach terminal game outcomes.

Remaining integration check: Open Watcom + DOSBox runtime validation.

## Build 5 — Gas system

**Status: next**

Deliverables: per-piece Gas 0–3, gas earning, gas UI, fart-mode input, eight directions, no-target puff.

Exit criterion: charged pieces can spend Gas and consume a turn without yet displacing another piece.

## Build 6 — Fart displacement

Deliverables: one-square push, edge/occupied handling, king legality, castling-right displacement effects, pushed-pawn promotion, check caused/resolved by fart, edge-case tests.

Exit criterion: every documented Fart Action rule is executable and test-covered.

## Build 7 — Presentation pass

Deliverables: final-ish board palette, final piece silhouettes, gas plume/push animation, screen shake/palette flashes, title/menu screens, improved fonts.

## Build 8 — Audio

Deliverables: Sound Blaster initialization, PCM playback, fart sample bank, UI/move/check SFX, PC speaker fallback, audio options.

## Build 9 — Save/load and config

Deliverables: config file, save UI, versioned save data, exact Gas/history restoration, error handling.

## Build 10 — CPU opponent

Deliverables: negamax/alpha-beta, evaluation, action ordering, Gas/fart awareness, difficulty settings, move-time cap.

## Build 11 — UX/polish

Deliverables: move history, help/rules, polished promotion/checkmate presentation, attract/demo loop, credits, keyboard completeness, mouse support.

## Build 12 — Release candidate

Deliverables: compatibility matrix, profiling, bug burn-down, packaged DOS ZIP, sample DOSBox config, final README, version stamp.

## Post-v1 ideas

Alternate boards/pieces, replay browser, Turbo Gas, puzzle challenges, AI personalities, null-modem multiplayer, OPL2 expansion, and a shareware-style episode screen are post-v1 only.
