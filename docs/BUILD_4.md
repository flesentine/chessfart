# Chess Fart — Build 4: Complete Standard Chess

## Status

**Complete in source. Strict C89 rule tests and framebuffer smoke test pass.**

Build 4 completes the conventional chess foundation so the Gas/Fart rules can be layered onto a stable legal game in Build 5.

## Delivered

### Castling

`CfBoard` now stores four castling-right bits. The move generator supports king- and queen-side castling for both colors only when:

- the corresponding right still exists
- the original-color rook is on its home square
- intervening squares are empty
- the king is not currently in check
- the king does not cross or land on an attacked square

King moves, rook moves, and captures of a rook on its home square permanently update the appropriate rights. Make/unmake restores the exact previous rights.

### En passant

Double pawn moves create a one-turn en-passant target. Pawn generation recognizes a valid adjacent enemy pawn and creates an en-passant move whose captured square differs from its destination.

Scratch-board king-safety filtering removes illegal en-passant captures that would expose the moving side's king through the vacated file/rank.

### Promotion

A pawn reaching the back rank generates four distinct legal actions:

- Queen
- Rook
- Bishop
- Knight

`board_make_move_ex()` applies a requested promotion; the compatibility `board_make_move()` defaults promotion to Queen.

The VGA shell pauses on a promotion target, lets the player cycle the four choices with arrow keys, and confirms with Enter.

### Game outcomes

`board_game_status()` reports:

- Playing
- Check
- Checkmate
- Stalemate
- Fifty-move draw
- Threefold-repetition draw
- Insufficient-material draw

For this game's UX, threefold and the 100-halfmove condition are treated as immediate draw outcomes rather than implementing a separate claim dialog.

### Draw/repetition state

The board tracks:

- halfmove clock
- fullmove number
- castling rights
- en-passant target

A compact repetition key stores all 64 pieces plus side to move, castling rights, and an **effective** en-passant file only when a legal en-passant capture exists. History is reset after irreversible moves, keeping the fixed DOS-friendly history buffer bounded while preserving relevant repetition positions.

### Make/unmake

`CfMove` now records enough previous state to exactly undo:

- normal moves
- captures
- castling rook movement
- en-passant captures
- promotions
- side to move
- castling rights
- en-passant target
- halfmove clock
- fullmove number

This keeps later AI/search work on the same reversible move model.

## Automated verification

Run:

```sh
make test-build4
```

Compiler flags:

```text
-std=c89 -pedantic -Wall -Wextra -Werror -O2
```

The suite includes direct fixtures for special rules plus reference perft checks:

| Position | Depths verified |
| --- | --- |
| Standard starting position | 20 / 400 / 8,902 / 197,281 |
| Kiwipete | 48 / 2,039 / 97,862 |
| En-passant-heavy reference | 14 / 191 / 2,812 / 43,238 / 674,624 |
| Promotion/castling reference | 6 / 264 / 9,467 / 422,333 |

Additional tests cover castling-through-check rejection, castling-right restoration, en-passant capture/unmake, en-passant discovered-check rejection, four promotion variants, Fool's Mate, stalemate, counters, fifty-move draw, insufficient material, and threefold repetition.

The host shell uses deterministic input to make E2-E4 and writes:

```text
build/host/chessfart_build4.ppm
```

The resulting panel exposes Black to move, the E3 en-passant target, castling rights, halfmove state, and the last move.

## DOS build

With Open Watcom installed:

```bat
wmake -f makefile.dos dos
```

or run `scripts\build_dos.bat`.

Expected executable:

```text
build\dos\CHESSFRT.EXE
```

Open Watcom/DOSBox is still unavailable in the environment used to prepare this milestone, so DOS runtime verification remains a platform integration check rather than a claimed result.

## Build 4 exit criterion

**Met in source and host validation:** Pure Chess can proceed from the standard start through every normal special rule to checkmate, stalemate, promotion, or supported draw outcome without bypassing king safety.

## Build 5 handoff

Build 5 should add Gas without changing the completed normal-chess semantics:

- per-piece Gas 0–3
- deterministic earning rules
- Gas display
- Fart Action input mode
- eight-direction targeting
- no-target puff

Fart displacement itself remains Build 6. Normal legal chess moves must remain a regression baseline throughout both builds.
