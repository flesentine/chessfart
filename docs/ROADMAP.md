# Chess Fart — Roadmap

Milestones are numbered as builds so future work can proceed `build 0`, `build 1`, etc.

## Build 0 — Repository foundation

**Status: current documentation phase**

Deliverables:

- README
- master plan
- game design rules
- VGA style guide
- architecture
- audio plan
- asset manifest
- test plan
- toolchain plan

Exit criterion: enough decisions exist to begin code without inventing architecture ad hoc.

## Build 1 — VGA boot

Deliverables:

- Open Watcom project skeleton
- `CHESSFRT.EXE`
- enter/exit Mode 13h
- software backbuffer
- palette load
- basic bitmap font
- static board
- keyboard Escape returns to DOS cleanly

Exit criterion: executable reliably displays the 320x200 board in DOSBox.

## Build 2 — Cursor and pieces

Deliverables:

- placeholder piece sprites
- starting position
- keyboard board cursor
- selection state
- side panel placeholder

Exit criterion: all 32 pieces draw correctly and player can select squares.

## Build 3 — Normal chess movement

Deliverables:

- board state module
- pseudo-legal move generation
- attack detection
- king safety
- legal moves highlighted
- apply/unapply
- captures

Exit criterion: ordinary piece movement cannot produce an illegal self-check position.

## Build 4 — Complete standard chess

Deliverables:

- castling
- en passant
- promotion
- check/checkmate/stalemate
- draw counters/state
- regression tests

Exit criterion: Pure Chess mode can play a complete legal match.

## Build 5 — Gas system

Deliverables:

- per-piece Gas 0–3
- gas earning rules
- gas UI
- fart mode input
- eight directions
- no-target puff

Exit criterion: charged pieces can spend Gas and consume a turn.

## Build 6 — Fart displacement

Deliverables:

- one-square push
- board-edge/occupied-destination handling
- king legality
- castling-right displacement effects
- pushed-pawn promotion
- check caused/resolved by fart
- full edge-case test suite

Exit criterion: every documented Fart Action rule is executable and test-covered.

## Build 7 — Presentation pass

Deliverables:

- final-ish board palette
- final piece silhouettes
- gas plume animation
- push animation
- screen shake/palette flashes
- title/menu screens
- improved fonts

Exit criterion: game visually communicates the concept without debug text.

## Build 8 — Audio

Deliverables:

- Sound Blaster initialization
- PCM playback
- fart sample bank
- UI/move/check SFX
- PC speaker fallback
- audio options

Exit criterion: audio is reliable and can be disabled entirely.

## Build 9 — Save/load and config

Deliverables:

- config file
- save slots or filename UI
- versioned save data
- exact Gas/history restoration
- error handling

Exit criterion: loaded match reproduces the original legal state exactly.

## Build 10 — CPU opponent

Deliverables:

- negamax/alpha-beta
- evaluation
- action ordering
- Gas/fart awareness
- difficulty settings
- move-time cap

Exit criterion: CPU completes matches and never emits an illegal action.

## Build 11 — UX/polish

Deliverables:

- move history
- help/rules pages
- promotion UI
- check/checkmate presentation
- attract/demo loop
- credits
- keyboard-only completeness
- mouse support if not already finished

Exit criterion: new player can understand and finish a game without external instructions.

## Build 12 — Release candidate

Deliverables:

- compatibility test matrix
- performance profiling
- bug burn-down
- packaged DOS ZIP
- sample `DOSBOX.CONF`
- final README instructions
- version stamp

Exit criterion: clean install/play on fresh DOSBox environment.

## Post-v1 ideas

Only after v1 is stable:

- alternate boards/pieces
- replay browser
- Turbo Gas
- puzzle challenges
- AI personalities
- null-modem multiplayer
- OPL2 soundtrack expansion
- shareware-style episode screen for fun

Do not promote post-v1 ideas into core scope without an explicit design decision.