# Chess Fart — Build 6: Fart Displacement

## Status

**Complete in source. Build 6 adds the full deterministic Fart Action rules layer.**

Build 5 could charge pieces and spend Gas on an empty-square `PUFF`. Build 6 makes the core gimmick real: an adjacent piece can now be blasted one square farther away when the destination is legal.

## Delivered

### One-square displacement

A charged current-side piece spends 2 Gas and selects one of eight directions. If the adjacent square contains a piece and the square one step beyond it is on-board and empty, that adjacent piece is displaced exactly one square.

The pushed piece may be friendly or enemy. Farting never captures. The pushed piece keeps its Gas value and earns no Gas. The acting piece alone spends 2 Gas.

### PUFF, PUSH, BLOCKED and INVALID previews

Build 6 distinguishes:

- `PUFF` — adjacent on-board square is empty
- `PUSH` — adjacent piece will move one square
- `BLOCKED` — adjacent piece exists but the destination is occupied or off-board
- `PROMOTE` — a pushed pawn reaches its promotion rank
- `INVALID` — the action cannot legally be confirmed

A confirmed `BLOCKED` fart still spends 2 Gas and consumes the turn. An aim whose immediately adjacent square is already off-board remains invalid.

### King safety

Every displacement is simulated before confirmation. The Fart Action is legal only if the acting player's king is safe after the complete displacement.

This supports pushing a checking attacker away, pushing a friendly blocker into a line, rejecting pushes that expose the acting king, moving the acting side's king through a fart, and pushing the opponent king to create check.

Kings can never become illegally adjacent because that would leave the acting king attacked and the action is rejected.

### Castling, en passant and counters

If a king is displaced from its home square, both castling rights for that color are permanently cleared. The same helper clears the corresponding right if a home rook is ever displaced.

Every Fart Action clears the existing en-passant opportunity and increments the halfmove clock. Black's Fart Action advances the fullmove number.

### Pushed-pawn promotion

A pawn pushed onto its final rank promotes immediately to Queen, Rook, Bishop or Knight. The owner chooses. The promoted piece keeps the pawn's Gas and earns no Gas for being pushed.

### Reversible action state

`CfFartAction` stores enough board/Gas metadata to undo PUFF, blocked attempts, normal pushes and pushed promotions exactly.

### Game-status integration

Checkmate and stalemate detection now consider legal Fart Actions. If no normal chess move exists but a legal fart response does, play continues. While in check, only a push that leaves the acting king safe counts as an escape.

### VGA interaction

Build 6 layers push-target and destination outlines over the Build 5 VGA renderer. The host smoke scene uses a charged White knight on C3 and a Black pawn on D4; scripted input fires NE and pushes the pawn to E5.

## Verification

Run:

```sh
make test-build6
```

The target runs the permanent Build 4 standard-chess suite, Build 5 Gas suite, Build 6 displacement suite, and the host framebuffer smoke run.

Build 6 tests cover enemy/friendly push, Gas preservation/spend, blocked/edge cases, self-check rejection, check resolution, opponent-king displacement/check, king adjacency, castling-right loss, en-passant expiry, pushed-pawn promotion, halfmove accounting, history, and exact unmake.

The new Build 6 core/UI source and displacement-specific harness also compile locally with strict C89 warnings-as-errors.

## DOS build

```bat
wmake -f makefile.build6.dos dos
```

or:

```bat
scripts\build_dos_build6.bat
```

Expected executable: `build\dos\CHESSFRT.EXE`.

Open Watcom/DOSBox remains the platform integration check for this milestone.

## Build 6 exit criterion

**Met in source:** every documented core Fart Action outcome is represented: PUFF, one-square displacement, blocked shove, king-safety filtering, state side effects, pushed promotion, and check generation/resolution.

## Build 7 handoff

Build 7 should stop adding rules and focus on presentation: improved board/pieces, gas plume and push animation, screen shake, palette flashes, title/menu screens and better fonts. Visual effects must not alter Build 6 rule results.
