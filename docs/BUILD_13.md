# Chess Fart — Build 13

## Status

Build 13 is a post-v1 presentation refresh. The packaged/versioned game remains **1.0.0**; Build 13 deliberately does not introduce a new ruleset or save format.

Build 13.8 is the closeout candidate for this pass.

## Frozen gameplay contract

Build 13 does not intentionally change:

- standard chess legality or terminal-state detection
- Gas earning/spending or Fart displacement rules
- CPU action generation, evaluator weights, or search policy
- persistence format or save/load semantics
- keyboard/mouse gameplay semantics
- logical resolution: 320x200 indexed VGA
- the 1.0 release/package version

## 13.0 — Shared layout/theme foundation

- centralized retail screen geometry in `include/ui_layout.h`
- centralized VGA semantic color roles in `include/ui_theme.h`
- moved renderer/input consumers onto shared constants with no intended pixel change

## 13.1 — Canonical visual review

- added a Chromium harness that captures the canvas backing store at native 320x200
- added deterministic review-only fixtures for rare states
- derived review clicks from the same board geometry used by the game

## 13.2 — Authored indexed assets

- added indexed PNG source sheets for pieces and fart puffs
- added deterministic standard-library-only conversion/validation
- packed runtime payload remains 1,696 bytes
- malformed metadata/palette/PNG structures fail closed

## 13.3 — Bitmap font

- replaced the hand-maintained glyph table with an authored indexed PNG atlas
- generated a 512-byte bit-packed runtime font
- preserved the existing 6-pixel text advance
- added punctuation/freshness/transparent-cell regression checks

Combined compiled Build 13 visual payload: **2,208 bytes** (1,696 asset bytes + 512 font bytes).

## 13.4 — Board and piece refresh

- improved black-piece readability and white-piece shading
- added restrained square grain and a beveled board frame
- switched to transparent piece blits so board markers remain readable
- retained exactly **30 intentional active VGA colors**

## 13.5 — Normal gameplay HUD

- replaced the legacy diagnostic sidebar with TURN/STATE, piece, GAS, CPU, and LAST ACTION hierarchy
- replaced the normal boxed command bar with one compact prompt strip
- added layout bounds and a compact `DRAW 3-FOLD` HUD label

## 13.6 — Fart-mode presentation

- added dedicated source/gas/direction/preview HUD
- added truthful PUSH/PUFF/BLOCKED/PROMOTION preview text
- added restrained board-space gas trail and target/destination emphasis
- hardened invalid edge previews so trail pixels stop at the 144x144 board boundary

## 13.7 — Title and modal consistency

- unified Help, History, Credits, and terminal overlays
- refreshed title menu/status/footer presentation
- bound title mouse input to shared layout constants
- added explicit save/load notices
- expanded the canonical visual suite to **19 states**

## 13.8 — Final hardening

- makes the title-menu item count and mouse hit rectangle canonical layout data
- moves title hit testing into a pure UX helper and tests every row plus all outer boundaries
- removes duplicated title-menu count/math from rendering and input
- clips unexpectedly long centered labels to their containing box instead of allowing a negative centered origin
- documents the completed Build 13 architecture and validation contract

The `main_build11*` implementation and host-capture names are historical lineage from the 1.0 UX layer. They are intentionally retained: renaming them would create broad churn without changing runtime behavior.

## Canonical native visual states

1. title
2. normal cursor
3. Help page 1
4. selected piece + legal moves
5. normal after e4
6. Gas-ready H4
7. real Fart mode
8. immediately after Fart
9. action log
10. save confirmation
11. load confirmation
12. deterministic Fart push preview
13. CHECK
14. CHECKMATE
15. invalid edge Fart preview
16. STALEMATE
17. Help page 2
18. Credits
19. title with Credits selected

## Closeout validation contract

A Build 13.8 closeout is acceptable only when all of these remain green on the exact PR head:

- Build 13 asset/font freshness and regression tests
- strict C89 host release suite
- Build 11 UX regressions, including title hit-test boundaries
- Open Watcom 2.0 real 16-bit DOS build
- DOSBox platform smoke
- Web/WASM build and runtime smoke
- Chromium mouse/Fart/history/save-load playtest
- deterministic Easy/Medium/Hard full Chromium games
- all 19 native 320x200 visual-review states
- no unexpected browser errors

Because 13.8 is intended to be pixel-neutral, the 19 visual-review PNGs/signatures should remain unchanged from the merged 13.7 baseline.
