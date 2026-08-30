# Chess Fart — Decision Log

This file records design/technical choices that should not silently drift between builds.

## D-001 — Authentic VGA target

**Decision:** Use a 320x200, 256-color Mode 13h presentation as the primary game target.

## D-002 — Chess remains the base game

**Decision:** Standard chess rules remain intact except where the Gas/Fart system explicitly interacts with board state.

## D-003 — Fart is deterministic

**Decision:** No random knockback, misfire, damage, stun or status effect in the default mode.

## D-004 — Individual Gas meters

**Decision:** Each piece stores Gas 0–3. Normal move earns 1; capture earns one additional point; Fart costs 2.

## D-005 — One-direction, one-square displacement

**Decision:** A Fart Action selects one of eight directions and may push one adjacent piece one square farther away.

## D-006 — Fart consumes the turn

**Decision:** Farting is an alternative action, not a bonus attached to a normal move.

## D-007 — No direct fart captures

**Decision:** The pushed destination must be empty. Pieces cannot be blasted off the board or into occupied squares to capture.

## D-008 — Kings may be displaced

**Decision:** A king can be pushed if the resulting position obeys all king-safety rules.

## D-009 — Core code platform-neutral

**Decision:** Chess/Fart rules must not depend on VGA, DOS input or audio code.

## D-010 — Open Watcom reference toolchain

**Decision:** Use C89/C90 and Open Watcom for the DOS build.

## D-011 — A blocked shove still commits the Fart Action

**Decision:** If the adjacent square contains a piece but the destination beyond it is occupied or off-board, the player may confirm the blocked Fart Action. It spends 2 Gas and consumes the turn without displacement.

**Reason:** This preserves the documented Build 0 tuning decision, keeps fart outcomes deterministic, and makes preview/commit behavior consistent.

## D-012 — Displacing the opponent king may give check

**Decision:** A Fart Action may push the opponent king onto a square attacked by the acting side. The action is legal as long as the acting player's own king is safe after the complete displacement. The opponent then begins its turn in check.

**Reason:** Chess legality constrains the moving side's king, while giving check to the opponent is a normal legal outcome. This also matches the game-design rule that Fart Actions can create check through displacement. King adjacency remains impossible because it would leave the acting king attacked.

## Pending decisions

- exact final palette
- whether OPL2 music ships in v1
- mouse support timing
- source-code license
- final AI difficulty tiers
- whether replay support is v1 or post-v1
