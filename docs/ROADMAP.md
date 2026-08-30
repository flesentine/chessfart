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

Delivered:

- 32-piece starting-position board model
- placeholder piece silhouettes
- keyboard board cursor
- arrow-key navigation
- Enter selection/deselection
- persistent selection state
- side-panel cursor/piece/selection information
- host board-state tests

Exit criterion met in source/host validation: all 32 pieces draw and the player can navigate/select squares.

Remaining integration check: Open Watcom + DOSBox keyboard/runtime validation.

## Build 3 — Normal chess movement

**Status: complete in source; host rules tests pass**

Delivered:

- pseudo-legal movement for all six piece types
- attack detection and check detection
- scratch-board king-safety filtering
- legal destination highlights
- apply/unapply move records
- ordinary captures
- turn enforcement
- source/destination cursor interaction
- pinned-piece and king-safety regression tests
- opening perft validation through depth 3

Exit criterion met in source/host validation: ordinary moves cannot leave the moving side in self-check.

Remaining integration check: Open Watcom + DOSBox runtime validation.

## Build 4 — Complete standard chess

**Status: next**

Deliverables: castling, en passant, promotion, check/checkmate/stalemate, draw state, regression tests.

Exit criterion: Pure Chess mode can play a complete legal match.

## Build 5 — Gas system

Deliverables: per-piece Gas 0–3, gas earning, gas UI, fart-mode input, eight directions, no-target puff.

Exit criterion: charged pieces can spend Gas and consume a turn.

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

Deliverables: move history, help/rules, promotion UI, check/checkmate presentation, attract/demo loop, credits, keyboard completeness, mouse support.

## Build 12 — Release candidate

Deliverables: compatibility matrix, profiling, bug burn-down, packaged DOS ZIP, sample DOSBox config, final README, version stamp.

## Post-v1 ideas

Alternate boards/pieces, replay browser, Turbo Gas, puzzle challenges, AI personalities, null-modem multiplayer, OPL2 expansion, and a shareware-style episode screen are post-v1 only.
