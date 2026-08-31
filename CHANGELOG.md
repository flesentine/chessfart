# Changelog

All notable project milestones are recorded here.

## Unreleased

### Build 10 — CPU opponent

Added:

- deterministic iterative-deepening negamax search
- alpha-beta pruning
- deterministic action ordering
- Easy / Medium / Hard difficulty profiles
- node and time search caps
- material/Gas/Fart-aware evaluation
- CPU generation of ordinary moves, promotions and every legal Fart Action
- automatic Black CPU replies after White actions
- automatic CPU continuation for Black-to-move loaded saves
- CPU search depth/node/cutoff/score VGA status
- `D` difficulty control on the title screen
- `docs/CPU_DESIGN.md`
- `docs/BUILD_10.md`
- Build 10 CPU regression suite

Architecture:

- Build 10 embeds the proven Build 9 game shell and wraps human action calls
- search uses the same reversible `gas_make_*` / `gas_unmake_*` rules as gameplay
- the real Black-to-move intermediate position is preserved in repetition history before the CPU action
- Build 9 save/config formats remain unchanged

Verification:

- all permanent Build 4–9 suites remain required
- deterministic opening search
- mate-in-one search
- Fart PUSH generation and exact unmake
- node-budget cutoff
- scripted save/load/Fart demo with CPU replies

### Build 9 — save/load and config

Added versioned transactional saves, exact Gas/history restoration, save/load controls and persistent audio config.

### Build 8 — audio

Added generated 8-bit PCM, Sound Blaster source backend, PC-speaker fallback, fart/UI/game effects and audio options.

### Build 7 — presentation pass

Added title/menu, Royal Basement VGA palette, improved pieces and five-frame fart/push animation.

### Build 6 — fart displacement

Added one-square displacement, blocked outcomes, king safety, castling-right displacement effects, pushed promotion and reversible fart actions.

### Build 5 — Gas system

Added per-piece Gas, earning/spending, Fart mode, eight-direction targeting, PUFF turns and Gas-aware repetition.

### Build 4 — complete standard chess

Added castling, en passant, promotion, terminal/draw states and reversible special moves.

### Current phase

Build 10 CPU opponent complete in source when CI is green. Build 11 — UX/polish — is next.
