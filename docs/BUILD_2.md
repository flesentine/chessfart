# Chess Fart — Build 2: Cursor and Pieces

## Status

**Complete in source. Host tests and framebuffer smoke test pass.**

Build 2 turns the static VGA shell into the first interactive board. It deliberately stops before chess move rules: Enter selects a piece, but pieces do not move yet.

## Delivered

### Starting-position board model

`include/board.h` and `src/game/board.c` add a small platform-independent board representation with:

- piece type enum: pawn, knight, bishop, rook, queen, king
- piece color enum: white/black
- 8x8 square array
- standard 32-piece starting position
- guarded square lookup
- piece counting and display-name helpers

The internal rank convention is chess-native: rank 0 is White's home rank and rank 7 is Black's home rank. The renderer flips ranks vertically so Black appears at the top of the screen.

### Placeholder VGA pieces

All 32 pieces now render on the board. Each piece type has a compact 20x20-era silhouette assembled from framebuffer rectangles plus a one-letter identifier (`P N B R Q K`). White and Black use separate body/highlight colors with a tiny shadow for board readability.

These are intentionally placeholder sprites. Final pixel art belongs to the presentation pass.

### Board cursor

The cursor starts on E2 and uses a bright cyan outline. DOS controls:

- Arrow Up: rank +1
- Arrow Down: rank -1
- Arrow Left: file -1
- Arrow Right: file +1
- Enter: select/deselect the piece under the cursor
- Escape: exit and restore DOS text mode

Cursor movement clamps at the board edges.

### Selection state

Enter on a piece marks that square with a separate magenta selection outline. Moving the cursor does not erase the selection. Pressing Enter again on the same selected piece deselects it. Pressing Enter on an empty square clears selection.

This creates the interaction state Build 3 will use for source-square and destination-square move entry.

### Live status panel

The side panel now shows:

- current cursor coordinate
- piece color/type under the cursor, or `EMPTY SQUARE`
- selected coordinate and selected piece type
- arrow/Enter/Escape control reminders

### Input abstraction

Build 1's Escape-only function is replaced with `input_poll_key()` and a platform-neutral key enum. The DOS backend decodes BIOS/conio extended arrow-key scan codes and Enter/Escape.

The host backend feeds a short deterministic key script so CI/smoke runs do not hang. The script selects E2, moves the cursor to G3, then exits. The final framebuffer therefore demonstrates cursor and selection as separate states.

## Host verification

Run:

```sh
make test-build2
```

This:

1. compiles the project as strict C89
2. runs a board-state test executable
3. validates the 32-piece starting position and boundary-safe lookup
4. runs the host game shell with scripted input
5. writes `build/host/chessfart_build2.ppm`
6. verifies the preview exists and is non-empty

Verified compiler flags:

```text
-std=c89 -pedantic -Wall -Wextra -Werror -O2
```

## DOS build

With Open Watcom installed:

```bat
wmake -f makefile.dos dos
```

or:

```bat
scripts\build_dos.bat
```

Expected output:

```text
build\dos\CHESSFRT.EXE
```

Open Watcom/DOSBox still cannot be executed in the current build environment, so the DOS binary is not falsely claimed as runtime-verified here.

## Build 2 exit criterion

**Met in source/host validation:** all 32 pieces draw correctly in the starting position and the player can navigate/select squares through the input abstraction.

Remaining platform integration check: compile the DOS target with Open Watcom and confirm the arrow/Enter scan-code behavior in DOSBox.

## Build 3 handoff

Build 3 should treat a selected occupied square as the move source and add:

- pseudo-legal movement generation for all six piece types
- attack detection
- king-safety filtering
- legal destination highlights
- apply/unapply state
- captures
- source/destination interaction through the existing cursor

Do not add castling, en passant, promotion, or Gas yet; those remain Build 4 and Build 5 work.
