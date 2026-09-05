# Changelog

All notable project milestones are recorded here.

## Build 14 — Local two-player

Build 14 restores the original hot-seat goal on top of the frozen 1.0 rules path.

- 14.0 introduces an explicit CPU/local match-mode contract while keeping CPU mode as the default.
- Automatic Black CPU replies are now gated through that contract instead of being unconditional.
- Review-only Chromium probes verify that local White e2-e4 leaves Black to move, local Black can answer e7-e5, and CPU mode still performs its automatic Black reply.
- The 1.0 save format and all player-facing title/menu pixels remain unchanged in 14.0.
- 14.1 adds a dedicated `2 PLAYERS` title row, canonical six-row keyboard/mouse hit testing, real Chromium keyboard/mouse mode-selection checks, and a twentieth native visual-review state.
- 14.2 labels local actions as `WHITE` / `BLACK`, replaces the CPU difficulty HUD row with `MODE / LOCAL 2P` in local play, and expands the native visual suite to 24 states, including real local Fart-mode HUD and mode-aware Help coverage. The same pass also fixes `ESC` in Fart mode so it cancels the action instead of exiting the match, matching the on-screen prompt.

## Build 13 — Post-v1 presentation refresh

Build 13 refreshes the 320x200 VGA presentation without changing the 1.0 rules, Gas/Fart mechanics, CPU evaluator, persistence format, or input semantics.

- 13.0 centralized retail layout and palette-role contracts.
- 13.1 added deterministic native 320x200 Chromium visual-review states and canonicalized review click geometry.
- 13.2 added deterministic indexed-PNG asset generation and fail-closed metadata validation.
- 13.3 moved the UI font to an authored bitmap atlas with generated 512-byte runtime data.
- 13.4 refreshed the board and piece art while preserving the 30-color active VGA contract.
- 13.5 replaced the normal diagnostic sidebar with the compact gameplay HUD and hardened draw-state bounds.
- 13.6 added the dedicated Fart-mode HUD/preview and clamped invalid edge trails to the board.
- 13.7 unified the title, Help, History, Credits, terminal overlays, and save/load notices; the visual suite now covers 19 canonical states.
- 13.8 closes the pass with canonical title-menu hit testing, regression coverage for every title row/boundary, defensive centered-text clipping, and documentation cleanup.

The historical `main_build11*` source/capture names remain implementation-lineage names; renaming them would add churn without changing the shipped interface.

## 1.0.0 — Final release

Promoted the frozen RC feature set to 1.0 after the release-hardening pass.

Hardening since RC1:

- corrected Fart-aware alpha-beta pruning so move-dependent tactical edge bonuses shift the recursive search window correctly
- retained the v7 deterministic CPU regressions for Fart check escape, forcing discovered check, castling-right destruction by king displacement and own-pawn Fart promotion
- promoted deterministic full Chromium games at Easy, Medium and Hard into the permanent browser playtest gate
- each full browser hardening game must terminate with zero browser errors and exercise live Fart mechanics
- kept natural CPU Fart frequency diagnostic rather than treating a particular frequency as correctness; tactical CPU Fart choices remain deterministic regression requirements
- revalidated the strict host regression stack, Open Watcom 2.0 16-bit DOS build, DOSBox smoke and release packaging after the search correction

Release policy:

- no gameplay rules, evaluator weights, save formats or audio systems were added between RC1 and 1.0
- zero known P0/P1 failures in the automated rules/build/browser/package gates at promotion
- physical 386/486 systems, MS-DOS/FreeDOS variants, real Sound Blaster models and third-party DOS mouse drivers remain explicitly unverified platform targets rather than claimed coverage

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
