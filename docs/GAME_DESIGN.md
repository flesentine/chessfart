# Chess Fart — Game Design Specification

## 1. Foundation

Chess Fart uses standard chess as its base ruleset. The variant adds a resource called **Gas** and a new turn action called **Fart**.

The intent is to add positional displacement while retaining check, checkmate, pins, discovered attacks, sacrifices, promotion, and familiar piece values.

## 2. Standard chess rules retained

Unless this document explicitly overrides something, use normal chess rules:

- 8x8 board
- standard starting position
- White moves first
- standard piece movement and captures
- check and checkmate
- stalemate
- castling
- en passant
- promotion
- threefold repetition
- fifty-move rule
- insufficient material

## 3. Gas

Every piece has an individual Gas value from 0 to 3.

### Earning Gas

After a legal normal move resolves:

- mover gains +1 Gas
- if that move captured a piece, mover gains +1 additional Gas
- cap at 3

Examples:

- Knight moves from g1 to f3: 0 → 1
- Same knight later captures e5: 1 → 3
- A piece already at 3 stays at 3

### Castling

Castling is a single king move for turn accounting, but both moved pieces gain +1 Gas because both physically moved.

### Promotion

A pawn that promotes carries its Gas value into the promoted piece. Gas awarded for the promotion move is applied before clamping to 3.

## 4. Fart Action

A Fart Action is an alternative to making a normal chess move.

Requirements:

- acting piece belongs to player whose turn it is
- acting piece has at least 2 Gas
- player selects one of eight directions: N, NE, E, SE, S, SW, W, NW

Resolution:

1. Spend 2 Gas from the acting piece.
2. Inspect the immediately adjacent square in the selected direction.
3. If it is empty, the fart has no displacement effect.
4. If it contains any piece, attempt to push that piece one additional square in the same direction.
5. Push succeeds only if destination is on-board and empty.
6. The entire resulting board must be legal with respect to the current player's king.
7. End turn.

A Fart Action never directly captures.

### Why empty-direction farts are legal

Allowing a no-effect fart keeps the system simple and permits intentional Gas dumping, tempo choices, comedy, and future puzzle design. The UI should distinguish `PUSH` from `PUFF` before confirmation.

## 5. King safety

King safety is the final authority for every action.

A normal move or Fart Action is illegal if the acting player's king is left in check after the complete action resolves.

### Farting while in check

Allowed only if the final board position removes the check.

Possible examples:

- push an attacking rook off its file
- push a blocking friendly piece into the checking line to interpose
- use the king's own fart to displace an adjacent attacker

### Pushing a king

A king may be displaced by a fart if and only if:

- destination is empty,
- destination is on the board,
- destination is not attacked by the opposing side after the displacement,
- the full resulting position is legal.

This applies to friendly and enemy kings. Kings may never become adjacent illegally.

The game still ends by checkmate, not by physically blasting a king off the board.

## 6. Consequences of displacement

A pushed piece is considered to have changed square even though it did not take a normal move.

### Castling rights

If an unmoved king or rook is displaced from its starting square by a fart, the corresponding castling right is permanently lost.

Returning the piece later does not restore castling rights.

### En passant

The en-passant opportunity lasts only until the opposing side completes its next turn, whether that turn is a normal move or a Fart Action.

A fart displacement itself never creates an en-passant target.

### Pawn direction

Fart displacement ignores pawn movement direction. A pawn can be pushed sideways or backward because this is external displacement, not a pawn move.

### Promotion via push

If a pawn ends a fart displacement on its promotion rank, it promotes immediately. The owner of the pawn chooses queen, rook, bishop or knight.

### Gas of pushed pieces

Being pushed does not earn or spend Gas.

## 7. Check caused by fart

A Fart Action may give check by changing line geometry or repositioning a piece.

Examples:

- push a blocker off a rook file
- push your bishop onto a new diagonal
- push an enemy king onto an attacked square only if the action is legal under king rules; in practice the resulting position is simply check

The UI should display `CHECK!` after the fart animation resolves.

## 8. Draw state

### Fifty-move rule

A Fart Action counts as one half-move without pawn movement or capture, so it increments the fifty-move counter.

Normal pawn moves or captures reset the counter as usual.

### Repetition

Position identity must include:

- piece type/color/square
- side to move
- castling rights
- en-passant target
- Gas value of every piece

Gas matters because two visually identical boards with different Gas are strategically different positions.

### Insufficient material

Use standard insufficient-material logic for v1. Gas cannot directly capture and cannot create additional mating material.

## 9. Interface rules

### Selecting a piece

The game highlights:

- normal legal destination squares
- whether Fart Action is available
- Gas meter 0–3

### Entering fart mode

Default keyboard proposal:

- arrows: move board cursor
- Enter: select/confirm normal action
- F: enter/exit fart mode for selected piece
- numpad or arrows: choose blast direction
- Escape: cancel selection/menu

Mouse support can map directly to the same state machine.

### Preview

Before confirmation, show:

- directional plume arrow
- pushed piece ghost at destination if applicable
- red invalid marker if push cannot occur
- whether the action produces check

## 10. Optional match modes

### Fart Chess

Default rules described above.

### Pure Chess

Normal chess with the same VGA presentation and no Gas. Useful for engine testing and players who want the art without the variant.

### Turbo Gas

Gas cost 1. Intended as an unlock/party mode after v1 balance is proven.

### Noisy Blitz

Fart Chess with a short clock. Lower priority.

## 11. Balance principles

- Fart should be useful but not strictly better than moving.
- Spending a whole turn is the primary cost.
- 2 Gas prevents immediate opening spam.
- 3 Gas cap makes resource state easy to read.
- One-square push prevents massive board rewrites.
- No random misfires.
- No area-of-effect multi-piece knockback in the core mode.

## 12. Piece-specific strategic identity

The same fart mechanic naturally changes by piece type:

- **Pawn:** cheap body that can charge, clog lanes and be repositioned in strange ways.
- **Knight:** excellent at getting adjacent to targets and disrupting structures.
- **Bishop:** can open or close diagonals via displacement.
- **Rook:** farting a blocker off a file can create immediate tactical pressure.
- **Queen:** powerful but spending a queen turn on a fart is a serious tempo decision.
- **King:** rare defensive emergency valve; risky because adjacency matters.

Do not add unique fart powers per piece until the shared system has been tested extensively.

## 13. AI evaluation implications

The computer should value:

- normal material
- mobility
- king safety
- pawn structure
- Gas reserve
- adjacency opportunities
- potential line-opening displacement
- danger of enemy charged pieces adjacent to tactically important units

Gas is not a flat material bonus. A charged trapped rook may have less fart utility than a charged knight deep in enemy territory.

## 14. Open tuning questions

These are intentionally deferred to playtesting:

- Is +2 Gas on capture too fast?
- Should a failed blocked push still spend the turn and Gas? Current design: yes, if confirmed.
- Should king displacement by enemy fart be allowed in ranked/default play? Current design: yes, with strict legality.
- Should a piece earn Gas after castling if already charged? Current design: clamp normally.
- Is no-effect PUFF tactically abusable for repetition? Repetition includes Gas, and Gas decreases, so it is self-limiting.

Record balance decisions in `DECISIONS.md` rather than silently changing rules.