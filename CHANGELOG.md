# Changelog

All notable project milestones are recorded here.

## Unreleased

### Build 11 — UX / polish

Added:

- 32-entry session action history
- `Tab`/`M` action-log overlay
- two-page `H`/`?` rules/help overlay
- `C` credits screen
- polished terminal-state overlays
- Play / Attract Demo / Help / Credits / Quit title menu
- idle title-screen attract demo using a real Fart PUSH
- Space as an Enter synonym
- optional DOS INT 33h mouse support
- left-click board targeting and right-click Fart mode
- clickable title menu with mouse driver
- Build 11 host previews for title/game/demo/help/history/credits/terminal
- `docs/UX_DESIGN.md`
- `docs/BUILD_11.md`
- Build 11 UX regression suite

Architecture:

- Build 11 keeps UX/session state outside save files and repetition identity
- action log clears on load while Build 9 rules history is restored exactly
- mouse input synthesizes the existing keyboard actions instead of bypassing rules
- Build 10 CPU and all Build 4–10 game semantics remain unchanged

### Build 10 — CPU opponent

Added deterministic iterative-deepening negamax/alpha-beta search, Gas/Fart-aware evaluation, bounded Easy/Medium/Hard CPU play and automatic Black turns.

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

Build 11 UX/polish is complete; CI passes. Build 12 — release candidate — is next.
