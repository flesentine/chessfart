# Chess Fart — Build 3: Legal Chess Movement

## Status

**Complete in source. Strict C89 rules tests and framebuffer smoke test pass.**

Build 3 turns the selectable board into a playable core chess-movement layer. It implements ordinary moves and captures while enforcing king safety. Special chess rules remain intentionally deferred to Build 4.

## Delivered

### Legal move engine

`src/game/board.c` now provides:

- pseudo-legal movement for pawn, knight, bishop, rook, queen, and king
- pawn single-step and initial double-step movement
- pawn diagonal captures
- sliding-piece ray traversal with blockers
- knight jumps
- king one-square movement
- enemy-king capture suppression
- color/turn enforcement

Build 3 intentionally stops pawns short of the promotion rank because Build 4 owns promotion choice/state.

### Attack detection

`board_square_is_attacked()` evaluates attacks independently of move legality. It handles pawn attack direction, knight jumps, sliding rays, and king adjacency.

`board_is_in_check()` locates a side's king and tests whether the opponent attacks that square.

### King-safety filtering

Legal move generation follows one consistent rule:

1. generate candidate movement for the selected piece
2. apply each candidate to a scratch board
3. reject the candidate if the moving side's king is attacked afterward

This automatically rejects pinned-piece moves that expose the king, king moves into attacked squares, and moves that fail to answer check.

### Apply / unapply

`board_make_move()` applies a verified legal move, records the moved/captured pieces in `CfMove`, and changes side to move.

`board_unmake_move()` restores source, destination, captured piece, and side to move. This is the foundation needed by later AI/search work.

### Captures and interaction

Ordinary captures work for every piece type. Arrow keys move the cursor; Enter selects a current-side piece, changes the source, cancels, or makes a legal destination move; Escape exits cleanly.

### VGA move hints

- cyan outline: cursor
- magenta outline: selected source
- green center marker: legal non-capture destination
- orange/red outline: legal capture destination
- red `CHECK!` status: side to move is currently in check

The right panel shows current turn, cursor square, selected source, legal destination count, and the last action message.

## Automated verification

Run:

```sh
make test-build3
```

The test target compiles with:

```text
-std=c89 -pedantic -Wall -Wextra -Werror -O2
```

Coverage includes starting-position move counts, E2/E3/E4 pawn behavior, blocked sliders, knight jumps, turns, captures, attack/check detection, pin filtering, king safety, make/unmake restoration, and opening perft sanity counts of **20 / 400 / 8,902** at depths 1 / 2 / 3.

The host shell uses scripted input to select E2 and legally move it to E4, then writes `build/host/chessfart_build3.ppm`.

## DOS build

With Open Watcom installed:

```bat
wmake -f makefile.dos dos
```

or run `scripts\build_dos.bat`.

Expected executable: `build\dos\CHESSFRT.EXE`.

Open Watcom and DOSBox are not present in the build environment used for this milestone, so DOS runtime verification remains an integration check rather than a claimed result.

## Explicitly deferred to Build 4

- castling
- en passant
- promotion
- checkmate
- stalemate
- draw counters / draw-state rules

## Build 3 exit criterion

**Met in source and host tests:** an ordinary move cannot be applied if it leaves the moving side's own king in check.

## Build 4 handoff

Build 4 should extend the move/state model rather than replace it. Add castling rights, en-passant target state, promotion action/state, checkmate/stalemate detection, and draw bookkeeping with regression fixtures.
