# Chess Fart — VGA Art & UI Style Guide

## 1. Era target

Aim for a premium 1991–1994 DOS game, not a modern pixel-art game pretending to be old.

Reference traits:

- 320x200 composition
- 256 indexed colors
- hard-edged sprites
- selective dithering
- dramatic bevels and embossed panels
- saturated highlight ramps
- black outlines used sparingly
- palette animation instead of alpha blending
- exaggerated one- or two-pixel motion

## 2. Screen layout

Logical screen: **320x200**.

Proposed match layout:

- Board: x=8..167, y=20..179
- Board size: 160x160
- Square size: 20x20
- Right UI panel: x=176..319
- Header/status strip: y=0..18
- Bottom message strip: y=182..199

This leaves narrow gutters for bevels and shadows.

## 3. Board

The board is the visual anchor and must remain legible under animation.

### Default theme: Royal Basement

- dark squares: burgundy/brown stone
- light squares: warm parchment/ivory
- thin gold outer frame
- tiny noise/dither texture, never enough to obscure pieces
- selected square: palette-ramped gold pulse
- legal normal move: small cyan/white corner dots
- legal fart direction: green wedge/arrow
- check: red/orange pulse behind king

## 4. Piece sprites

Target footprint: about **16x18 pixels** inside each 20x20 square.

Each piece needs:

- white-side base sprite
- black-side base sprite
- selected/highlight mask
- 2-frame idle option
- 2–3 frame move/squash option
- hit/pushed pose where useful

Silhouette must win over detail. At 16 pixels wide, rook/queen/bishop/king must be recognizable instantly.

### Color language

White pieces: ivory, tan shadow, gold accent.

Black pieces: charcoal, blue-purple midtone, steel highlight.

Both sides receive the same green gas colors; ownership is communicated by the originating piece and UI.

## 5. Gas meter

Each selected piece shows three small pips in the side panel:

- empty pip: dark inset
- full pip: sickly lime-to-yellow ramp
- 2+ Gas: subtle animated bubble pixel every few frames

Do not place permanent gas bars over every board piece; it would make the board noisy. For at-a-glance opponent planning, charged pieces can receive a tiny 1-pixel green glint at their base.

## 6. Fart animation

The animation should be funny in under half a second.

Suggested 5-frame sequence:

1. piece squashes down 1 pixel
2. rear green spark appears
3. directional cloud expands into neighboring square
4. target piece shifts/pops to destination
5. cloud breaks into 2–4 pixels and disappears

Use palette cycling within two dedicated gas ramps to create motion without extra sprite memory.

No transparency blending; use color-keyed sprites and ordered dithering if translucency is absolutely needed.

## 7. Screen effects

Use sparingly:

- 1–2 pixel screen shake for strong fart/capture
- palette flash for check/checkmate
- horizontal wipe or checker dissolve between screens
- palette fade to black for loading/quit
- subtle title-logo shimmer via palette cycling

Avoid modern particles, blur, bloom and smooth vector scaling.

## 8. Palette budget

Suggested allocation of the 256 entries:

- 0: transparent/key black
- 1–15: UI grayscale
- 16–47: board light-square ramp/textures
- 48–79: board dark-square ramp/textures
- 80–111: white piece ramp + gold accents
- 112–143: black piece ramp + cool highlights
- 144–175: green/yellow gas ramps
- 176–199: red/orange danger/check
- 200–223: cyan/blue selection/info
- 224–239: portraits/logo/special UI
- 240–255: reserved animation/effect entries

This is a planning allocation, not a hard file format. Final palette should be authored globally so screens share indexes where possible.

## 9. Typography

Use a custom bitmap font.

Recommended sizes:

- 5x7 small UI font
- 8x8 normal text font
- 8x12 or hand-drawn title/subtitle letters

Text should remain uppercase-heavy in true DOS style but not sacrifice readability.

Example status strings:

- `WHITE TO MOVE`
- `GAS: **.`
- `FART READY`
- `CHECK!`
- `PUFF — NO TARGET`
- `BLAST BLOCKED`

## 10. Title screen

Composition:

- huge `CHESS FART` stone/gold logo
- king and queen facing each other
- a suspicious green cloud drifting between them
- menu: PLAY / VS CPU / OPTIONS / CREDITS / QUIT
- tagline: `CHECK. MATE. VENTILATE.`

Add a delayed attract animation if the player does nothing.

## 11. Menus

DOS panel language:

- beveled rectangles
- bright top/left edge, dark bottom/right edge
- keyboard hotkeys underlined or color-coded
- moving glove/crown cursor or highlight bar
- instant response; no long easing animations

## 12. Asset source format

Authoring may use PNG, but runtime assets must be converted to indexed binary/sprite data during the build.

Rules:

- nearest-neighbor only
- no anti-aliased edges
- preserve explicit palette indexes where palette cycling matters
- no true-color runtime dependency
- keep source PNGs and conversion scripts in repo

## 13. Art acceptance checklist

An asset is ready when:

- it reads at native 320x200 with no zoom,
- it stays inside the approved palette,
- no accidental anti-alias colors exist,
- animation is readable in 2–5 frames,
- white/black pieces are distinguishable on both square colors,
- gas effects never hide critical board state for more than a few frames.