# Chess Fart — Build 14

## Status

Build 14 adds local two-player hot-seat play without creating a second rules engine.

**14.1: title match selection.**

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

## Frozen 14.1 contracts

14.1 does not change:

- chess legality
- Gas earning/spending or Fart displacement
- CPU search/evaluator/difficulty behavior
- title/menu pixels or controls
- save-file version or serialized contents
- audio behavior
- board geometry, assets or palette
- packaged version 1.0.0

Match mode remains session-level infrastructure in 14.1. Persistence of the selected mode is intentionally deferred until a later Build 14 slice can define backward-compatible behavior explicitly.

## Chromium 14.1 contract

The browser hardening run must prove:

1. a fresh session defaults to CPU mode
2. keyboard Down + Enter on the title selects `2 PLAYERS` and enters local mode
3. a real mouse click on the `2 PLAYERS` title row enters local mode
4. local White e2-e4 leaves Black to move and local Black can answer e7-e5
5. local load of a Black-to-move save does not wake the CPU
6. CPU-mode load of a Black-to-move save still triggers the automatic Black reply
7. the existing Easy/Medium/Hard full games still terminate with zero browser errors
8. the visual-review suite contains 20 native 320x200 states, including the new local-mode title selection

## Next

14.2 will make local play read correctly during the match: White/Black player identity in the HUD and action history, with CPU-only labels suppressed in local mode.
