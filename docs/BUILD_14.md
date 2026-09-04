# Chess Fart — Build 14

## Status

Build 14 adds local two-player hot-seat play without creating a second rules engine.

**14.0: match-mode foundation.**

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

## Frozen 14.0 contracts

14.0 does not change:

- chess legality
- Gas earning/spending or Fart displacement
- CPU search/evaluator/difficulty behavior
- title/menu pixels or controls
- save-file version or serialized contents
- audio behavior
- board geometry, assets or palette
- packaged version 1.0.0

Match mode is session-level infrastructure in 14.0. Persistence of the selected mode is intentionally deferred until a later Build 14 slice can define backward-compatible behavior explicitly.

## Chromium 14.0 contract

The browser hardening run must prove:

1. a fresh session defaults to CPU mode
2. review-selected local mode accepts White e2-e4 and leaves Black to move
3. local Black can respond e7-e5 through the normal board/input path
4. a fresh CPU-mode game still performs an automatic Black reply after White e2-e4
5. the existing Easy/Medium/Hard full games still terminate with zero browser errors
6. all 19 Build 13 visual-review states remain unchanged unless a later Build 14 presentation slice intentionally updates them

## Next

14.1 will make the mode player-selectable from the title screen. That slice will add canonical layout constants, keyboard/mouse hit testing and visual-review coverage for the new title choice.
