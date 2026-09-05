# Chess Fart — Build 14

## Status

Build 14 adds local two-player hot-seat play without creating a second rules engine.

**Complete: 14.6 closeout.**

## Why this build

The original master plan listed local hot-seat two-player as a required product goal. The shipped 1.0 flow currently presents human White versus CPU Black. Build 14 closes that gap while preserving the trusted chess, Gas/Fart, persistence and presentation contracts.

## 14.0 — Match-mode foundation

14.0 deliberately does not expose a new title option yet. It establishes and verifies the architectural seam first:

- add an explicit `CfMatchMode` contract with CPU and local modes
- keep CPU mode as the startup default
- gate automatic Black CPU replies through the match mode
- let local mode leave the board on Black after a legal White move
- let the same existing input/rules path accept a legal Black human move
- keep load-time automatic CPU reply behavior in CPU mode while suppressing it in local mode
- expose review-only match-mode controls to Chromium
- regression-test both local and CPU turn models in the browser gate

## 14.1 — Title match selection

14.1 makes the 14.0 match-mode contract player-selectable without changing the underlying rules path:

- add a dedicated `2 PLAYERS` row immediately after `PLAY CPU`
- expand the canonical title menu from five to six named items
- tighten title-row spacing from 10 to 9 pixels so all six hit regions remain inside the existing panel/footer boundary
- select CPU or local mode only when the corresponding play row is confirmed
- preserve Attract, Help, Credits, Quit, audio, SFX and CPU-difficulty title behavior
- keep keyboard Up/Down/Enter selection and mouse row hit-testing on the same canonical layout data
- verify local selection through both real keyboard input and real Chromium mouse input
- extend the native visual-review suite from 19 to 20 states with `2 PLAYERS` selected

## 14.2 — Local-player presentation

14.2 makes local play read like a two-human match while leaving CPU mode unchanged:

- local White actions are logged as `WHITE`
- local Black actions are logged as `BLACK`
- CPU games retain `YOU` and `CPU` history labels
- the gameplay and Fart HUD replace the CPU difficulty row with `MODE / LOCAL 2P` during local play
- `ESC` now cancels Fart mode as the command bar has always advertised instead of exiting the match
- the command bar continues to show the side to move
- Chromium asserts the real local history buffer contains White then Black after e2-e4 / e7-e5
- the native visual suite adds local Black-to-move HUD, local action-log, local Fart-mode HUD and local Help states

## 14.3 — Match-mode persistence

14.3 makes save/load semantics independent of the mode that happens to be active when Load is pressed:

- move `CfMatchMode` into a shared contract header used by UX and persistence
- bump newly written game saves from format version 1 to version 2
- write one explicit `MODE 0|1` record before the existing board state
- continue accepting legacy version-1 saves without rewriting them in place
- interpret every successful version-1 load as CPU mode because CPU was the only shipped mode when those files were created
- restore a version-2 save's mode before the load hook decides whether Black should take an automatic CPU turn
- keep load transactional: malformed/unsupported files do not change board, Gas, history or match mode
- keep config format/version unchanged

## 14.4 — Local Fart, promotion and terminal hardening

14.4 exercises the rare local-only edge paths through real production input after deterministic review-only save setup:

- Black can enter Fart mode, execute a push, spend Gas and hand the turn back to White without waking the CPU
- White and Black ordinary pawn promotions both enter the real promotion chooser and commit the selected Q/R/B/N piece
- ordinary promotion now has a dedicated command-bar prompt matching the Fart-promotion chooser
- `ESC` during ordinary promotion cancels the pending choice and keeps the match running instead of quitting
- board mouse clicks are ignored while either ordinary-promotion or Fart-promotion choice is pending, preventing mouse auto-target arrows from cycling or auto-confirming the chooser
- White and Black Fart-push promotions both preserve the pushed pawn's Gas and use the selected promotion piece
- White and Black can each deliver checkmate in local mode
- after checkmate, further select/Fart/confirm input cannot mutate the board, turn counters or action history
- review-only fixture creation writes normal version-2 LOCAL saves; Chromium still loads and acts through the production game loop
- the canonical native visual suite expands from 24 to 32 states

## 14.5 — Full local-match regression

14.5 stops adding isolated edge fixtures and proves that the complete local mode survives a long ordinary game from the real title screen:

- select `2 PLAYERS` with real keyboard title input
- play Paul Morphy's 33-ply Opera Game from the normal starting position using real Chromium board mouse input for both colors
- verify the expected `CHECK` states at 11.Bxb5+, 15.Bxd7+ and 16.Qb8+
- verify White's real queenside castling at 12.O-O-O, including king C1, rook D1, cleared E1/A1 and `WHITE O-O-O` history
- after 8...c6, save the exact version-2 LOCAL persisted state
- play 9.Bg5 b5, load the checkpoint, and prove exact rollback of mode, side, castling rights, en-passant state, halfmove/fullmove clocks, all piece+Gas squares and the complete repetition-history keys before replaying those two plies
- open and close the real action-history UI during the continued match
- finish with 17.Rd8# and verify the expected Black-to-move checkmate state
- attempt further select/Fart/confirm input after mate and require board, history and turn counters to remain unchanged
- keep the 32-state canonical visual suite unchanged; the long-game screenshots are diagnostics rather than new visual baselines
- pair the Chromium game with the existing Release CI strict host, Open Watcom 16-bit DOS, DOSBox smoke and package regression pass

## 14.6 — Closeout and certification

14.6 makes no gameplay changes. It closes the Build 14 line by aligning the repository and packaged documentation with the shipped source and by turning the most important documentation assumptions into release-audit checks:

- root README documents both `PLAY CPU` and `2 PLAYERS`, the 33-ply local full-game gate, exact v2 LOCAL rollback and the 32-state visual suite
- packaged README documents local hot-seat play and the final browser certification scope
- packaged `KEYS.TXT` no longer carries the stale `1.0.0-rc1` heading and explicitly notes title-mode selection
- packaged known-issues scope includes CPU/local-match browser coverage
- compatibility matrix and release checklist record the completed Build 14 evidence
- roadmap marks Build 14 complete
- release audit now fails if packaged controls regress to an RC heading or if the release README loses the local-mode/v2-save contract
- the final Build 14 branch must pass the unchanged strict host, Open Watcom 16-bit DOS, DOSBox, package, Web/Chromium and 32-state visual gates before merge
- after merge, the exact `main` merge commit receives one final Release/Web/Deploy certification

## Frozen Build 14 contracts

Build 14 closeout does not change:

- chess legality
- Gas earning/spending or Fart displacement
- CPU search/evaluator/difficulty behavior
- title/menu pixels or controls
- audio behavior
- board geometry, assets or palette
- packaged version 1.0.0

The packaged game version remains 1.0.0. Only the internal game-save format advances to version 2; legacy version-1 saves remain loadable with their historical CPU semantics.

## Chromium Build 14 contract

The browser hardening run must prove:

1. a fresh session defaults to CPU mode
2. keyboard Down + Enter on the title selects `2 PLAYERS` and enters local mode
3. a real mouse click on the `2 PLAYERS` title row enters local mode
4. local White e2-e4 leaves Black to move and local Black can answer e7-e5
5. local load of a Black-to-move save does not wake the CPU
6. CPU-mode load of a Black-to-move save still triggers the automatic Black reply
7. the existing Easy/Medium/Hard full games still terminate with zero browser errors
8. the local action log records `WHITE` then `BLACK` after e2-e4 / e7-e5
9. local gameplay shows `MODE / LOCAL 2P` instead of CPU difficulty
10. local Fart mode also shows `MODE / LOCAL 2P`, and `ESC` cancels it without exiting the match
11. local Help page 2 explains two-player turns instead of CPU Black
12. the visual-review suite contains 24 native 320x200 states, including local normal, history, Fart HUD and Help states
13. a version-2 LOCAL save restores LOCAL even when CPU mode is active immediately before Load
14. a version-2 CPU save restores CPU even when LOCAL mode is active immediately before Load
15. a rewritten legacy version-1 Black-to-move save defaults to CPU and triggers the historical automatic Black reply
16. a real local Black Fart pushes a White piece, spends exactly 2 Gas, logs `BLACK FART`, advances the fullmove counter and leaves White to move
17. ordinary promotion works for White and Black; White promotion can be cancelled with `ESC`, re-entered and completed without exiting the match; board mouse clicks cannot alter either promotion chooser
18. Fart-push promotion works for White and Black and preserves the pushed pawn's Gas on the promoted destination
19. White and Black can each deliver checkmate through a real local move
20. after either local checkmate, board state, side/fullmove counters and action history remain locked against further gameplay input
21. the visual-review suite contains 32 native 320x200 states, adding Black Fart, ordinary/Fart promotion and both local checkmate results
22. a complete 33-ply local Opera Game runs from title selection through 17.Rd8# using real board mouse input for both players
23. the full-game gate reports CHECK on the three known checking plies and proves queenside castling
24. a move-9 version-2 LOCAL checkpoint rolls back the complete persisted game state after two additional plies—including mode, board metadata, all piece+Gas squares and repetition history—and can be replayed cleanly
25. the long-game gate opens the real history UI and finishes with a terminal lock that rejects further gameplay input
26. Release CI simultaneously passes strict host tests, real Open Watcom 16-bit DOS compilation, DOSBox smoke and release packaging

## Closeout

Build 14 is complete after 14.6. Future work belongs to a new post-v1 build line rather than extending the local-two-player milestone.
