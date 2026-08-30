# Chess Fart — Decision Log

This file records design/technical choices that should not silently drift between builds.

## D-001 — Authentic VGA target

**Decision:** Use a 320x200, 256-color Mode 13h presentation as the primary game target.

**Reason:** The constraint is central to the project's identity and keeps art/scope disciplined.

## D-002 — Chess remains the base game

**Decision:** Standard chess rules remain intact except where the Gas/Fart system explicitly interacts with board state.

**Reason:** The game needs a stable strategic foundation.

## D-003 — Fart is deterministic

**Decision:** No random knockback, misfire, damage, stun or status effect in the default mode.

**Reason:** Players should be able to calculate tactics.

## D-004 — Individual Gas meters

**Decision:** Each piece stores Gas 0–3. Normal move earns 1; capture earns one additional point; Fart costs 2.

**Reason:** Small readable state, delayed opening spam, and interesting piece-specific resource decisions.

## D-005 — One-direction, one-square displacement

**Decision:** A Fart Action selects one of eight directions and may push one adjacent piece one square farther away.

**Reason:** Easy to preview, test and understand while still altering chess geometry.

## D-006 — Fart consumes the turn

**Decision:** Farting is an alternative action, not a bonus attached to a normal move.

**Reason:** Tempo is the balancing cost.

## D-007 — No direct fart captures

**Decision:** The pushed destination must be empty. Pieces cannot be blasted off the board or into occupied squares to capture.

**Reason:** Keeps capture semantics recognizably chess-like and avoids physics edge cases.

## D-008 — Kings may be displaced

**Decision:** A king can be pushed if the resulting position obeys all king-safety rules.

**Reason:** It makes the core gimmick matter around the most important piece without changing the victory condition.

## D-009 — Core code platform-neutral

**Decision:** Chess/Fart rules must not depend on VGA, DOS input or audio code.

**Reason:** Enables fast host-side testing and protects correctness.

## D-010 — Open Watcom reference toolchain

**Decision:** Start with C89/C90 and Open Watcom for the DOS build.

**Reason:** Suitable DOS support, period-appropriate output, and maintainable C code.

## Pending decisions

- exact final palette
- whether OPL2 music ships in v1
- mouse support timing
- source-code license
- final AI difficulty tiers
- whether replay support is v1 or post-v1

Add new decisions as `D-011`, `D-012`, etc., including reason and any superseded decision.