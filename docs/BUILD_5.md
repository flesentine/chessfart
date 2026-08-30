# Chess Fart — Build 5: Gas System

## Status

**Complete in source. Strict C89 Build 4 regressions, Build 5 Gas tests, and framebuffer smoke test pass.**

Build 5 adds the first Chess Fart variant layer without changing the completed Build 4 standard-chess engine. Pieces now accumulate visible Gas and a charged piece can spend a turn on a directional no-displacement `PUFF`. Occupied-target displacement is intentionally reserved for Build 6.

## Architecture choice

Gas is semantically a property of each live chess piece, but Build 5 stores it in a parallel `CfGasState` 8x8 grid rather than changing `CfPiece` in the Build 4 board engine.

Normal chess moves are still generated and validated entirely by `src/game/board.c`. `src/game/gas.c` wraps an accepted chess move, transfers the source Gas with the moving piece, removes captured Gas, handles castling-rook Gas, awards the new Gas, and can restore the exact previous Gas state on unmake.

This gives the project a useful regression boundary: Build 4 standard-chess semantics remain unchanged while the variant state grows around them.

## Gas rules implemented

Each occupied square has a Gas value from 0 through 3.

After a legal normal move:

- quiet move: mover gains +1 Gas
- capture: mover gains +2 Gas total
- values clamp at 3

Castling is one chess move but both physically moved pieces gain +1 Gas. Promotion carries the pawn's existing Gas into the promoted piece and then applies the normal move/capture award.

Captured pieces lose their Gas with the captured piece. Make/unmake restores both board and Gas exactly.

## Fart mode

A selected current-side piece with at least 2 Gas may enter Fart mode with `F`.

Eight directions are represented explicitly:

- N
- NE
- E
- SE
- S
- SW
- W
- NW

Cardinal directions use the arrow keys. DOS keypad diagonal scan codes are also mapped for diagonal aim. `F` cancels Fart mode.

The VGA side panel displays the current direction and preview state before confirmation.

## PUFF action

Build 5 fully implements the no-target/off-board Fart Action.

If the adjacent target square in the chosen direction is empty or off-board, a legal `PUFF`:

1. spends 2 Gas from the acting piece
2. clears any en-passant opportunity
3. increments the halfmove clock
4. advances fullmove state after Black
5. changes side to move
6. records a Gas-aware repetition position

A PUFF is rejected while the acting side is in check because it does not alter piece geometry and therefore cannot resolve that check.

`CfFartAction` records enough state to unmake a PUFF exactly during testing and future search work.

## Occupied targets

If the adjacent square contains a piece, the preview reports `PUSH IN B6` / `CF_FART_PUSH_BUILD6`.

Build 5 deliberately does **not** spend Gas or end the turn for an occupied target because the legality of the resulting displacement belongs to Build 6. This prevents a temporary Build 5 implementation from creating rules that immediately need to be removed.

## VGA Gas display

Every occupied square now has a tiny three-segment Gas strip at its bottom edge. The selected/cursor piece also gets a numeric `GAS n/3` readout in the right-side panel.

The panel indicates whether the selected piece is Fart-ready. Fart mode adds a directional plume/target indicator and a `PUFF READY`, `PUSH IN B6`, or invalid preview message.

## Repetition identity

Build 4 repetition identity did not need Gas. Build 5 adds `CfGasHistory`, whose position key includes:

- piece type/color/square
- Gas value associated with every occupied square
- side to move
- castling rights
- effective en-passant state

This means the same visible chess arrangement with differently charged pieces is correctly treated as a different Chess Fart position.

A useful regression consequence is that the old knight-shuffle repetition sequence does not immediately repeat the same variant position while the knights are still accumulating Gas. Once their Gas values stabilize at the cap, later identical board states can repeat normally.

## Game-status integration

`gas_game_status()` keeps normal Build 4 mate/check/draw behavior but treats a legal PUFF as an available turn action. This prevents a position from being declared stalemate merely because the player has no normal chess move when a charged legal PUFF is available.

Insufficient-material logic continues to use the standard Build 4 rule as specified by the design document.

## Automated verification

Run:

```sh
make test-build5
```

The target compiles with:

```text
-std=c89 -pedantic -Wall -Wextra -Werror -O2
```

It performs three checks:

1. the complete Build 4 standard-chess regression/perft suite
2. Build 5 Gas/Fart unit tests
3. a deterministic host UI run that creates the Build 5 framebuffer preview

Build 5 tests cover:

- +1 quiet-move Gas
- +2 capture Gas and cap behavior
- captured-piece Gas removal/restoration
- castling Gas for king and rook
- promotion Gas inheritance
- PUFF Gas spend
- en-passant expiry on PUFF
- clock/turn accounting
- PUFF make/unmake
- in-check PUFF rejection
- occupied-target Build 6 preview
- Gas-sensitive repetition identity

The host script moves and charges a White knight, then performs a northward PUFF. The final image is written to:

```text
build/host/chessfart_build5.ppm
```

## DOS build

With Open Watcom installed:

```bat
wmake -f makefile.build5.dos dos
```

or:

```bat
scripts\build_dos_build5.bat
```

Expected executable:

```text
build\dos\CHESSFRT.EXE
```

Open Watcom/DOSBox is not installed in the environment used for this milestone, so DOS runtime validation remains a platform integration check rather than a claimed result.

## Build 5 exit criterion

**Met in source/host validation:** individual pieces accumulate visible Gas, a charged piece can enter eight-direction Fart mode, and a legal no-target PUFF spends Gas and consumes the turn without displacing another piece.

## Build 6 handoff

Build 6 should extend the same Fart preview/action model with actual occupied-target displacement:

- one-square push
- blocked/off-board push behavior
- full king-safety validation after displacement
- friendly and enemy king displacement rules
- castling-right loss when king/rook is pushed
- en-passant expiry
- pushed-pawn promotion
- check created or resolved by fart
- Gas-aware repetition after push
- make/unmake and edge-case fixtures

Normal Build 4 chess regressions and all Build 5 Gas tests should remain permanent baseline tests.
