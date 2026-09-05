# Chess Fart — Build 14

## Status

Build 14 adds local two-player hot-seat play without creating a second rules engine.

**14.4: local edge hardening.**

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

## Frozen 14.4 contracts

14.4 does not change:

- chess legality
- Gas earning/spending or Fart displacement
- CPU search/evaluator/difficulty behavior
- title/menu pixels or controls
- audio behavior
- board geometry, assets or palette
- packaged version 1.0.0

The packaged game version remains 1.0.0. Only the internal game-save format advances to version 2; legacy version-1 saves remain loadable with their historical CPU semantics.

## Chromium 14.4 contract

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

## Next

14.5 will run a longer end-to-end two-player Chromium scenario plus the full DOS regression pass before Build 14 closeout.
