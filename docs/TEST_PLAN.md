# Chess Fart — Test Plan

## 1. Testing philosophy

The comedy can be loose. The board state cannot.

Every action that reaches the core engine must be deterministic, reversible, legal, and reproducible.

## 2. Test layers

### Host unit tests

Fast tests compiled on a modern host against `core/`.

### DOS integration tests

Exercise actual platform/render/input/audio code in DOSBox.

### Manual gameplay tests

Focus on UX, animation, sound fatigue and emergent tactical behavior.

## 3. Standard chess regression

Required categories:

- pawn one/two-step
- pawn blocked movement
- pawn diagonal capture
- knight movement
- bishop blockers
- rook blockers
- queen movement
- king attack exclusion
- pinned pieces
- discovered checks
- double checks
- castling rights
- castling through check prohibited
- en passant
- en-passant self-check edge case
- all promotion choices
- checkmate
- stalemate
- threefold repetition
- fifty-move counter
- insufficient material

Use known perft positions where practical for normal chess validation.

## 4. Gas tests

- initial pieces start at 0
- normal move adds exactly 1
- capture adds 2 total
- Gas clamps at 3
- fart spends exactly 2
- pushed piece Gas unchanged
- castling increments both moved pieces
- promotion preserves/updates Gas correctly

## 5. Fart geometry tests

For all eight directions:

- adjacent empty -> legal puff when king remains safe
- adjacent occupied + destination empty -> push
- destination occupied -> no push
- adjacent off-board -> no push
- destination off-board -> no push
- no row-wrap/column-wrap bugs

Test from corners, edges and center.

## 6. King-safety fart tests

- fart exposes own king -> illegal
- fart blocks check -> legal
- fart pushes checking piece away -> legal if check removed
- fart fails to remove check -> illegal
- push friendly king into attacked square -> illegal
- push enemy king into attacked square -> legal only insofar as resulting position is a valid checked position and kings are not illegally adjacent
- fart creates discovered check
- fart creates double check

## 7. Castling displacement tests

- push unmoved rook off starting square -> corresponding right lost
- push it back later -> right stays lost
- push unmoved king -> both rights lost
- unrelated pushed piece -> rights unchanged

## 8. Pawn displacement tests

- push pawn backward
- push pawn sideways
- push pawn onto promotion rank
- push pawn away from potential en-passant interaction
- fart does not create en-passant target

## 9. Apply/unapply tests

For every action type:

1. snapshot state bytes/logical fields
2. apply action
3. unapply action
4. compare with original

Run this recursively during move generation tests to catch hidden state damage.

## 10. Hash/repetition tests

- same board + same Gas -> same key
- same board + different Gas -> different key
- side-to-move changes key
- castling rights change key
- en-passant changes key
- apply/unapply restores original key

## 11. Save/load tests

- fresh game
- midgame with castling rights
- active en-passant target
- multiple Gas values
- promoted piece
- near fifty-move draw
- repetition history
- corrupted header
- unsupported version
- truncated file

## 12. AI legality tests

Across thousands of generated positions:

- AI-selected action appears in legal action list
- apply never leaves own king in check
- AI can handle zero legal moves
- AI handles promotion
- AI handles fart responses to check
- no search-state corruption after iterative deepening

## 13. VGA/render tests

Manual/automated screenshot checks where feasible:

- no draw outside 320x200 buffer
- all board squares aligned
- sprites stay within intended cells during idle state
- palette fades restore correct palette
- selection/check colors remain distinguishable
- text never writes outside panel bounds

## 14. Input tests

- keyboard-only complete match
- rapid key repeat does not skip states
- Escape cancels safely
- Escape from game/menu eventually restores text mode
- mouse and keyboard can alternate without selection corruption

## 15. Audio tests

- no Sound Blaster present
- explicit audio NONE
- PC speaker fallback
- rapid consecutive SFX
- quit while sound active
- invalid BLASTER config
- volume settings persist

## 16. Performance targets

Initial goals, measured in DOSBox at conservative settings:

- match UI animation feels stable at 60-ish Hz update pacing
- normal input feedback under one frame
- legal move generation feels instant to human player
- AI easy difficulty responds quickly
- no audible stutter from routine rendering

Exact cycle target will be recorded after Build 3 establishes baseline.

## 17. Playtest questions

Ask testers:

1. Did you understand how Gas is earned?
2. Could you predict what a fart would do before confirming?
3. Did you ever create a deliberate tactic with a push?
4. Did the fart mechanic feel useful, overpowered, or pointless?
5. Was the board readable during effects?
6. Did sounds become annoying after 20 minutes?
7. Would you play another game?

The most important signal is #3.