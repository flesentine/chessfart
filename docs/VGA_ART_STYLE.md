# Chess Fart — VGA Art & UI Style Guide

## 1. Era target

The finished presentation aims for a premium 1991–1994 DOS game rather than modern pixel art wearing a retro skin.

Core traits:

- 320x200 composition
- 256 indexed colors
- hard-edged authored sprites
- nearest-neighbor display
- dramatic DOS-panel bevels
- restrained texture and palette effects
- no runtime alpha blending

## 2. Current screen layout

Canonical geometry lives in `include/ui_layout.h`.

- Logical screen: 320x200
- Board origin: x=18, y=27
- Square size: 18x18
- Board pixels: 144x144
- Board frame: x=14, y=23, 152x154
- Right HUD: x=170, y=24, 143x151
- Header: y=0..20
- Command bar: y=181..199

Rendering and mouse hit testing use these same constants.

## 3. Board themes

### Royal Basement

Royal is the default and compatibility fallback. Its sparse stone-grain pixels and semantic palette roles are pinned by the native visual regression suite.

### Crimson Cellar

Crimson keeps the same geometry and overlays but uses staggered cellar-brick/mortar edge texture and copper piece accents. Texture stays away from the center of each square so piece silhouettes remain dominant.

Theme changes never alter rules, piece masks, legal-move geometry, Gas, or input semantics.

## 4. Piece sprites

Authored piece masks are approximately 16x18 inside the 18x18 square footprint.

- White base/highlight and Black base/highlight stay semantically stable.
- Royal uses the classic gilt/neutral accent mapping.
- Crimson remaps the authored accent class to copper.
- The exact same piece masks are used by both themes.

Silhouette wins over detail; every piece must remain readable on both square colors.

## 5. Gas and Fart presentation

Gas uses the shared green semantic roles. The selected piece HUD shows three Gas pips.

Fart feedback uses:

- directional aim/preview geometry
- short five-frame animation
- PUFF/PUSH/BLOCKED/promotion-specific presentation
- bounded effects that never obscure critical board state for long

No gameplay result is inferred from animation; presentation receives already-resolved state.

## 6. Title and modal language

The title uses the `CHESS FART` identity, `CHECK. MATE. VENTILATE.` tagline, and the current seven-row menu: Play CPU, 2 Players, Practice, Attract Demo, Help / Rules, Credits, Quit to DOS.

Help, History, Credits, Replay, save/load notices, and terminal overlays use the same panel, typography, and semantic color system.

## 7. Typography and assets

The UI uses an authored bitmap font and generated indexed runtime data.

Source assets live under `assets_src/`; deterministic converters in `tools/` generate the committed runtime includes. Asset checks reject stale generated output.

Rules:

- nearest-neighbor only
- no anti-aliased sprite edges
- preserve semantic palette indexes
- no true-color runtime dependency
- source PNGs and converters remain reproducible

## 8. Acceptance contract

Presentation maintenance is acceptable only when:

- native 320x200 readability remains strong
- all pixels stay in bounds
- selection/legal/check/Fart overlays remain distinguishable
- both sides remain readable on both square colors
- the 36-state Chromium visual suite passes
- pinned Royal/Crimson signatures remain unchanged unless a deliberate visual contract change is approved
