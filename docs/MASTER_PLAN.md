# Chess Fart — Master Plan

## 1. Vision

Chess Fart should feel like a forgotten 1992 DOS shareware game that somehow contains a genuinely good strategy game.

The pitch is simple:

> **It is chess, except pieces can charge up and fart other pieces across the board.**

The joke gets somebody to try it. The tactical depth gets them to keep playing.

## 2. Non-negotiable product goals

1. Render at **320x200 in 256-color VGA Mode 13h**.
2. Preserve the recognizable rules and mental model of chess.
3. Make the fart mechanic deterministic, readable, and strategically useful.
4. Support local two-player before AI.
5. Feel excellent in DOSBox and stay friendly to real DOS-era hardware constraints.
6. Keep scope disciplined enough to reach a polished v1.

## 3. Core gameplay

Standard chess is the base game. Each piece also owns a 0–3 point Gas meter.

- Normal move: +1 Gas to the piece that moved.
- Capture: +1 additional Gas.
- Gas cap: 3.
- Fart Action cost: 2 Gas.
- A Fart Action consumes the player's turn.
- The acting piece chooses one of eight adjacent directions.
- If a piece occupies the adjacent square, the blast attempts to push it one square farther in the same direction.
- The destination must be on the board and empty.
- A fart never directly captures a piece.
- All resulting positions must be legal with respect to king safety.

This creates tactics based on displacement rather than adding arbitrary new attack patterns.

## 4. Player fantasy

The board should feel dignified and absurd at the same time: stone or wood chessmen, royal heraldry, velvet panels, then a knight visibly clenches and launches a green pixel cloud with a crunchy 8-bit sample.

Humor should come from presentation and timing, not from making the rules incoherent.

## 5. v1 feature set

### Required

- Title screen and attract/demo loop
- Local hot-seat two-player
- Human vs CPU
- Legal standard chess moves
- Castling, en passant, promotion
- Check, checkmate, stalemate and draw handling
- Gas meters and Fart Actions
- Undo in practice/debug mode — completed by Build 16
- Move history panel
- VGA sprite animation
- Sound Blaster digital SFX path
- PC speaker fallback
- Config file
- Save/load match
- DOSBox-friendly executable and launch instructions

### Nice to have after the core is solid

- Multiple fart sample banks
- Alternate board themes
- Tournament time controls
- Replay files
- AI personalities
- Campaign/challenge puzzles
- Modifiable palette packs
- Network/null-modem play

### Explicitly not v1

- Online multiplayer
- Procedural boards
- RPG progression
- Large story campaign
- 3D graphics
- Physics simulation
- Random fart outcomes

## 6. Visual target

- Logical resolution: 320x200
- Color depth: 8-bit indexed, 256 colors
- Board: 160x160, 20x20 pixels per square
- Main board area: left side of screen
- Right-side panel: turn, selected piece, gas, move/fart buttons, clocks/history/status
- Piece sprites: approximately 16x18 with 1–2 pixel breathing/idle movement
- Fart plume: 3–5 frame directional sprite animation with palette cycling
- Captures: short squash/pop animation, never long enough to slow repeated play

See `VGA_ART_STYLE.md`.

## 7. Technology target

Reference implementation:

- C89/C90
- Open Watcom C/C++
- DOS 32-bit protected mode target
- VGA Mode 13h framebuffer
- software backbuffer
- fixed-timestep simulation
- keyboard polling + BIOS/mouse interrupt abstraction
- Sound Blaster 8-bit mono PCM initially
- PC speaker fallback

Rules code must not depend on VGA or DOS APIs. That separation lets us compile a host-side test executable and catch chess-rule bugs quickly.

## 8. Architecture layers

### `core/`
Pure rules and state:

- board representation
- move generation
- attack detection
- check/checkmate
- castling/en passant/promotion
- gas accounting
- Fart Action generation
- history and repetition state
- serialization-friendly game state

### `ai/`

- legal action enumeration
- alpha-beta / negamax
- transposition table
- evaluation
- move ordering
- difficulty limits

### `platform/dos/`

- Mode 13h enter/leave
- VGA palette
- keyboard
- mouse
- timer
- Sound Blaster
- PC speaker
- file I/O wrappers

### `game/`

- screens
- UI state
- match flow
- animations
- menu/config
- save/load

### `assets/`
Source and packed runtime data.

## 9. Production order

Build the game in vertical slices, not subsystem silos.

### Slice 1 — board boots

A DOS executable opens Mode 13h, displays the board and pieces, accepts cursor movement, and exits cleanly.

### Slice 2 — real chess

Selecting a piece shows legal moves and completes normal chess play with check/checkmate.

### Slice 3 — fart prototype

Gas appears, charges, and the first legal directional push works end-to-end with placeholder graphics/audio.

### Slice 4 — full rules

Resolve all edge cases: kings, castling rights, en passant, promotion, repetition and save/load.

### Slice 5 — presentation

Replace placeholder visuals, add animation, palette effects, menus and sound.

### Slice 6 — AI

Add computer player only after action generation is trusted.

### Slice 7 — polish/release

Performance tuning, compatibility matrix, docs, replayable demo, packaged ZIP.

## 10. Definition of fun prototype

The prototype is successful when two people can play a complete local match and, within ten minutes, deliberately use gas to create a tactic that could not exist in normal chess.

If testers merely spam fart whenever available, the system needs more positional cost or better UI explanation.

## 11. Major risks and mitigations

### Risk: fart action destroys chess readability

Mitigation: no random results, only one-square displacement, visible gas, legal-action highlighting.

### Risk: too many legal actions for AI

Mitigation: fart actions require 2 Gas and adjacency; aggressive move ordering and selective fart search can reduce branching.

### Risk: VGA UI becomes cramped

Mitigation: lock board to 160x160 and design the right panel around 160x200 from day one.

### Risk: sound becomes annoying

Mitigation: short samples, varied bank, volume setting, PC speaker alternative and mute toggle.

### Risk: authentic DOS work slows iteration

Mitigation: keep rules platform-neutral and compile host tests separately.

## 12. Initial success criteria

A v1 release is ready when:

- 100% of normal chess legality regression tests pass.
- Fart legality edge-case tests pass.
- A full 100+ move game can be completed without state corruption.
- The game holds its target frame pacing in DOSBox at conservative cycle settings.
- Keyboard-only operation is complete.
- Audio can be disabled with no gameplay dependency.
- Save/load reproduces an identical board, gas state, rights and history.
- AI can finish games without illegal actions.
- The final package boots from a clean DOSBox setup using documented steps.

## 13. First implementation target

The first coding milestone should create a minimal executable named `CHESSFRT.EXE` that:

1. switches to 320x200x256,
2. renders the full board,
3. draws placeholder pieces,
4. moves a selection cursor,
5. returns safely to text mode on Escape.

Do not start AI, audio, or final art before this vertical slice works.