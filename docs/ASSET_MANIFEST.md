# Chess Fart — Asset Manifest

This is the production checklist for art/audio content.

## Graphics

### Core board

- `board_tiles` — light/dark square textures
- `board_frame` — border/corners
- `cursor_select`
- `cursor_legal_move`
- `cursor_invalid`
- `check_overlay`
- `last_move_overlay`

### Piece set

For White and Black:

- pawn
- knight
- bishop
- rook
- queen
- king

Optional animation variants:

- idle frame B
- squash/pre-fart
- pushed/hit frame

### Gas effects

Eight-direction support:

- N
- NE
- E
- SE
- S
- SW
- W
- NW

Each direction can share mirrored frames where visually safe.

Effect phases:

- spark
- cloud small
- cloud full
- breakup

### UI

- title logo
- main menu panel
- match side panel
- gas pips empty/full
- fart-ready icon
- move history arrows/icons
- clock digits if clocks are included
- confirm/cancel buttons
- mouse cursor
- scroll arrows
- check/checkmate banners
- promotion picker

### Fonts

- 5x7 UI font
- 8x8 text font
- title lettering/logo art

### Screens

- title
- options
- credits
- victory/checkmate panel
- optional help/rules screen

## Audio

### SFX

- cursor tick
- select click
- cancel click
- invalid bonk
- move tap
- capture clack
- promotion chime
- check alarm
- checkmate stinger
- fart short
- fart medium
- fart wet
- fart boom
- fart squeak
- pushed-piece impact

### Music

Optional:

- title OPL loop
- game OPL loop
- victory phrase

## Data/utility assets

- default VGA palette
- piece-square tables for AI
- opening/test positions
- demo replay
- sample config file

## Source rules

Every shipped asset should have:

- source file where practical
- runtime converted file
- clear naming
- palette conformity
- origin/license note if not original

Keep conversion reproducible. Never make the only copy of an important asset a generated binary.