# Chess Fart — Roadmap

Milestones are numbered as builds so future work can proceed `build 0`, `build 1`, etc.

## Build 0 — Repository foundation
**Status: complete**

## Build 1 — VGA boot
**Status: complete in source; host smoke test passes**

## Build 2 — Cursor and pieces
**Status: complete in source; host tests pass**

## Build 3 — Normal chess movement
**Status: complete in source; host rules tests pass**

## Build 4 — Complete standard chess
**Status: complete in source; host rules tests pass**

Delivered castling, en passant, promotion, terminal states, draw bookkeeping, reversible special moves and multi-position perft regression coverage.

## Build 5 — Gas system
**Status: complete in source; host gas/rules tests pass**

Delivered per-piece Gas, earning/spending, Gas-aware repetition, eight-direction Fart mode and PUFF actions.

## Build 6 — Fart displacement
**Status: complete in source; CI passes**

Delivered one-square push, blocked outcomes, king safety, displaced castling rights, pushed promotion, check creation/resolution and reversible Fart actions.

## Build 7 — Presentation pass
**Status: complete in source; pending/verified by Build 7 CI**

Delivered:

- title screen and keyboard menu
- Royal Basement final-ish VGA palette
- improved piece silhouettes
- copper board bevel and rank/file coordinates
- shadowed title/header treatment
- five-frame fart plume/push animation
- palette impact flash
- board-frame shake
- Build 7 framebuffer artifact in CI
- unchanged Build 4–6 rules regression baseline

Exit criterion: a complete Build 6 match is presented with a coherent premium-DOS visual layer without altering rules.

## Build 8 — Audio
**Status: next**

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
