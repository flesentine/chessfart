# Chess Fart — Roadmap

Milestones are numbered as builds so future work can proceed `build 0`, `build 1`, etc.

## Build 0 — Repository foundation
**Status: complete**

## Build 1 — VGA boot
**Status: complete in source; host smoke test passes**

## Build 2 — Cursor and pieces
**Status: complete in source; host tests pass**

## Build 3 — Normal chess movement
**Status: complete in source; host rules tests pass**

## Build 4 — Complete standard chess
**Status: complete in source; host rules tests pass**

Castling, en passant, promotion, terminal states, draws, reversible special moves and perft regression coverage.

## Build 5 — Gas system
**Status: complete**

Per-piece Gas, earning/spending, Gas-aware repetition, eight-direction Fart mode and PUFF.

## Build 6 — Fart displacement
**Status: complete; CI passes**

One-square push, blocked outcomes, king safety, displaced castling rights, pushed promotion and reversible Fart actions.

## Build 7 — Presentation pass
**Status: complete; CI passes**

Title/menu, Royal Basement VGA palette, improved pieces, board polish and five-frame fart animation.

## Build 8 — Audio
**Status: complete; CI passes**

Sound Blaster source backend, generated 8-bit PCM, fart/UI/game SFX, PC-speaker fallback and audio options.

## Build 9 — Save/load and config
**Status: complete; CI passes**

Versioned game saves, exact Gas/history restoration, safe load validation, persistent audio config and save/load controls.

## Build 10 — CPU opponent
**Status: complete; CI passes**

Iterative-deepening negamax/alpha-beta, deterministic action ordering, Gas/Fart-aware evaluation, Easy/Medium/Hard limits, bounded node/time search, automatic Black CPU turns and CPU regression coverage.

## Build 11 — UX/polish
**Status: complete; CI passes**

Session move/action history, help/rules overlays, terminal-state presentation, title attract demo, credits, keyboard completeness and optional DOS mouse support.

## Build 12 — Release candidate
**Status: complete — 1.0.0-rc1**

Feature freeze, version stamp, full regression/profile/audit gate, real Open Watcom 16-bit DOS build, DOSBox platform smoke, compatibility matrix, DOSBox configuration, release documentation, verified distributable ZIP and SHA-256.

## RC follow-up / v1.0

Collect physical/emulator compatibility reports, fix any RC-discovered P0/P1 defects, then promote the same feature set to v1.0 without adding new gameplay systems.

## Post-v1 ideas

Alternate boards/pieces, replay browser, Turbo Gas, puzzle challenges, AI personalities, null-modem multiplayer, OPL2 expansion, and a shareware-style episode screen.
