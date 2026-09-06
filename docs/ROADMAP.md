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

Castling, en passant, promotion, terminal states, draws, reversible special moves and perft regression coverage.

## Build 5 — Gas system
**Status: complete**

Per-piece Gas, earning/spending, Gas-aware repetition, eight-direction Fart mode and PUFF.

## Build 6 — Fart displacement
**Status: complete; CI passes**

One-square push, blocked outcomes, king safety, displaced castling rights, pushed promotion and reversible Fart actions.

## Build 7 — Presentation pass
**Status: complete; CI passes**

Title/menu, Royal Basement VGA palette, improved pieces, board polish and five-frame fart animation.

## Build 8 — Audio
**Status: complete; CI passes**

Sound Blaster source backend, generated 8-bit PCM, fart/UI/game SFX, PC-speaker fallback and audio options.

## Build 9 — Save/load and config
**Status: complete; CI passes**

Versioned game saves, exact Gas/history restoration, safe load validation, persistent audio config and save/load controls.

## Build 10 — CPU opponent
**Status: complete; CI passes**

Iterative-deepening negamax/alpha-beta, deterministic action ordering, Gas/Fart-aware evaluation, Easy/Medium/Hard limits, bounded node/time search, automatic Black CPU turns and CPU regression coverage.

## Build 11 — UX/polish
**Status: complete; CI passes**

Session move/action history, help/rules overlays, terminal-state presentation, title attract demo, credits, keyboard completeness and optional DOS mouse support.

## Build 12 — Release candidate
**Status: complete — 1.0.0-rc1**

Feature freeze, version stamp, full regression/profile/audit gate, real Open Watcom 16-bit DOS build, DOSBox platform smoke, compatibility matrix, DOSBox configuration, release documentation, verified distributable ZIP and SHA-256.

## RC follow-up / v1.0
**Status: complete — 1.0.0**

The frozen RC feature set was hardened rather than expanded:

- corrected the alpha-beta child window for move-dependent Fart tactical bonuses
- locked the sound tactical Fart behaviors into deterministic CPU regressions
- promoted deterministic Easy/Medium/Hard full Chromium games into the browser gate
- revalidated strict host tests, Open Watcom 16-bit DOS compilation, DOSBox smoke and packaging
- promoted final package/version metadata to 1.0.0 with no rule, save-format or evaluator-weight changes

Physical 386/486 systems, MS-DOS/FreeDOS variants, real Sound Blaster hardware and third-party DOS mouse drivers remain explicitly unverified manual/community compatibility targets. They are documented platform scope, not claimed test coverage.

## Build 13 — Post-v1 VGA presentation refresh
**Status: complete — 13.8 merged**

Build 13 keeps the frozen 1.0 gameplay/save contract and refreshes presentation in small reviewable slices:

- 13.0 shared layout/theme foundation
- 13.1 canonical Chromium visual-review harness
- 13.2 indexed PNG asset pipeline
- 13.3 bitmap font and accent text
- 13.4 board and piece art refresh
- 13.5 normal gameplay HUD
- 13.6 dedicated Fart-mode presentation
- 13.7 title/modal consistency
- 13.8 final input/layout hardening and documentation closeout

The permanent browser review suite covers 19 native 320x200 states, including normal play, Fart states, save/load notices, CHECK/CHECKMATE/STALEMATE, both Help pages, Credits, and a non-default title selection. See `docs/BUILD_13.md`.

## Build 14 — Local two-player
**Status: complete — 14.6 closeout**

Build 14 closes the original local hot-seat requirement without forking the trusted chess/Gas rules path:

- 14.0 explicit CPU/local match-mode foundation and browser regression coverage
- 14.1 title-menu selection for PLAY CPU versus 2 PLAYERS, with keyboard/mouse coverage and a 20-state visual suite
- 14.2 local-player turn/history/HUD presentation, with WHITE/BLACK action identity and LOCAL 2P HUD mode
- 14.3 version-2 save mode persistence with backward-compatible version-1 CPU semantics
- 14.4 local Fart/promotion/terminal-state hardening across White and Black, including promotion cancel
- 14.5 full 33-ply two-player Opera Game Chromium scenario, mid-game save/load rollback, and DOS regression pass
- 14.6 documentation/package consistency cleanup and final certification

The existing CPU game remains the startup default. Build 14.3 advances newly written game saves to format version 2 while keeping legacy version-1 saves loadable as CPU-mode saves. Build 14 closes with 32 native visual states and a full 33-ply local Opera Game Chromium regression with exact persisted-state rollback/replay.

## Build 15 — Replay / postgame review
**Status: complete — 15.4 closeout**

Build 15 takes the master plan's replay-file idea in staged, DOS-safe slices:

- 15.0 compact 256-frame session replay timeline with CPU/local Chromium coverage
- 15.1 read-only replay viewer modal
- 15.2 terminal/postgame replay UX and mouse hardening
- 15.3 separate versioned replay-file export/import without changing game-save v2
- 15.4 replay hardening, 600-frame ring/file certification, full CPU/local terminal replay checks, memory/package review, and closeout

Build 15 closes with 118-byte snapshots, a 30,232-byte 256-frame live timeline, a 60,464-byte live+transient-import peak, separate version-1 `CHESSFRT.RPL` files, and unchanged game-save v2 semantics. See `docs/BUILD_15.md`.

## Build 16 — Practice / undo
**Status: 16.0 complete; 16.1 in progress**

Build 16 closes the remaining master-plan required item: Undo in practice/debug mode.

- 16.0 bounded reversible-action journal foundation
- 16.1 local Practice session and keyboard undo UX
- 16.2 practice-state/replay/history integration and mouse polish
- 16.3 hardening, long-session rollback, DOS memory review, and closeout

Practice is a session policy rather than a new persisted match-mode value, so the version-2 game-save contract remains unchanged. See `docs/BUILD_16.md`.

## Post-v1 ideas

Alternate boards/pieces, Turbo Gas, puzzle challenges, AI personalities, null-modem multiplayer, OPL2 expansion, and a shareware-style episode screen.
