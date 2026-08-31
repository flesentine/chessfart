# Changelog

All notable project milestones are recorded here.

## 1.0.0-rc1 — Build 12 release candidate

Added/released:

- feature freeze and `VERSION` / `include/version.h` stamp for 1.0.0-rc1
- permanent Build 4–11 regression stack promoted to the RC gate
- host CPU/memory profiling report
- release-source audit for conflict markers, TODO/FIXME/XXX markers and whitespace errors
- real 16-bit DOS `CHESSFRT.EXE` build in CI with Open Watcom 2.0
- DOSBox-resident platform smoke covering VGA, mouse probe and audio initialization
- sample `DOSBOX.CONF`
- DOS release README, complete controls and known-issues notes
- compatibility matrix and release checklist
- reproducible release packager with ZIP integrity checks and SHA-256
- CI artifact containing the DOS ZIP, raw EXE, smoke proof, profile/size reports and Build 11 visual/audio artifacts

Measured on the RC CI host:

- DOS executable: 111,988 bytes
- EASY start: depth 1 / 20 nodes
- MED start: depth 2 / 440 nodes
- HARD start: depth 3 / 7,320 nodes / 325 cutoffs
- Fart-heavy MED fixture: depth 2 / 6,240 nodes

Release policy:

- zero known P0/P1 failures in the automated rules/build/package gates
- physical DOS hardware, real Sound Blaster variants and third-party mouse drivers remain explicit manual/community compatibility targets
- no new gameplay features are planned between RC1 and v1.0; RC feedback is for defects/compatibility

## Build 11 — UX / polish

Added session action history, rules/help, credits, terminal-state overlays, title attract demo, keyboard completeness, Space confirm and optional DOS INT 33h mouse support.

## Build 10 — CPU opponent

Added deterministic iterative-deepening negamax/alpha-beta search, Gas/Fart-aware evaluation, bounded Easy/Medium/Hard CPU play and automatic Black turns.

## Build 9 — save/load and config

Added versioned transactional saves, exact Gas/history restoration, save/load controls and persistent audio config.

## Build 8 — audio

Added generated 8-bit PCM, Sound Blaster source backend, PC-speaker fallback, fart/UI/game effects and audio options.

## Build 7 — presentation pass

Added title/menu, Royal Basement VGA palette, improved pieces and five-frame fart/push animation.

## Build 6 — fart displacement

Added one-square displacement, blocked outcomes, king safety, castling-right displacement effects, pushed promotion and reversible fart actions.

## Build 5 — Gas system

Added per-piece Gas, earning/spending, Fart mode, eight-direction targeting, PUFF turns and Gas-aware repetition.

## Build 4 — complete standard chess

Added castling, en passant, promotion, terminal/draw states and reversible special moves.
