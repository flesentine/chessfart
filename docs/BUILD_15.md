# Chess Fart — Build 15 Replay / Postgame Review

Build 15 adds replay and postgame review without changing chess legality, Gas/Fart rules, CPU evaluation, or the version-2 game-save contract.

**Build 15 complete — 15.4 replay hardening / closeout certified.**

## Why replay

Replay files were explicitly listed in the original master plan as a post-core feature. Build 14 leaves the gameplay core unusually well certified, so replay is a good next line: it adds player value while remaining a read-only consumer of trusted game state.

The replay system must never become a second rules engine.

## 15.0 — Compact session timeline

15.0 adds a bounded in-memory timeline only. There is no new player-facing replay screen or replay file format yet.

Contract:

- record the initial rendered game state as frame 0
- record exactly one frame after every committed human move or Fart
- in CPU mode, record the human frame and the automatic CPU frame separately
- do not record cursor movement, selection, Help/History/Credits, Save, cancelled Farts, cancelled promotions, or aim/preview state
- a successful Load starts a new replay baseline because save format v2 does not serialize prior replay history
- do not change `CHESSFRT.SAV`, `CHESSFRT.CFG`, package version, chess/Gas rules, or CPU behavior
- keep the existing 32-state visual suite unchanged

### Memory policy

The timeline has 256 snapshot slots. This covers the master-plan 100+ full-move target with room for the initial frame.

Each square is packed into one byte containing piece type, color, and Gas. A snapshot also stores:

- side to move
- castling rights
- en-passant file/rank
- halfmove/fullmove clocks
- game status
- CPU/local match mode
- a short action label

The host regression enforces:

- `sizeof(CfReplaySnapshot) < 128`
- `sizeof(CfReplayTimeline) < 32768`

If more than 256 frames are recorded, the timeline becomes a bounded ring and marks itself truncated rather than allocating more memory.

### Chromium 15.0 gate

The browser hardening run must prove:

1. local title selection creates exactly one replay baseline
2. White e2-e4 creates frame 2 with the pawn on e4 and Gas 1
3. Black e7-e5 creates frame 3
4. selection, History, and Save create no replay frame
5. a successful local Load resets the timeline to one baseline matching the loaded state
6. a fresh CPU game starts with one frame
7. White e2-e4 plus the automatic Black CPU reply produces exactly three frames
8. the last replay frame exactly matches current board/Gas/metadata/status/mode

## Planned Build 15 slices

### 15.1 — Replay viewer modal

- open the read-only replay screen with `R`
- start on the newest retained frame and step with Left/Right
- show absolute frame number, turn, state, match mode and action label
- render through replay scratch state without mutating the live board, Gas, repetition history, mode or render cache
- avoid gameplay-audio side effects while reviewing old CHECK/CHECKMATE frames
- clamp stepping at the oldest/newest retained frame
- return to the exact live game when closed with R, Enter or Esc
- keep HUD/help discoverability and mouse replay controls for 15.2

### 15.2 — Postgame replay UX

- expose `R RPLY` in the normal command bar, full `R REPLAY` in Help, and Replay in GAME OVER
- add footer mouse zones for Previous / Next / Close inside the replay viewer
- keep keyboard Left/Right stepping and R/Enter/Esc close behavior
- consume gameplay keys and board clicks while replay is open so old-frame review cannot mutate the live match
- harden pending and committed ordinary-promotion replay behavior
- harden pending and committed Fart-push-promotion replay behavior
- harden terminal CHECKMATE replay and exact return to the postgame overlay
- preserve the existing board geometry, rules, CPU, save format and package version

### 15.3 — Replay files

- define separate version-1 `CHESSFRT.RPL` format without changing `CHESSFRT.SAV` v2
- serialize the retained logical replay order, absolute frame total, and truncation state
- hex-encode action labels so whitespace/punctuation cannot alter parsing
- reject bad magic/version, invalid metadata, malformed squares/labels, and trailing junk transactionally
- export the live session from Replay with `S`
- import `CHESSFRT.RPL` with `L` into a temporary heap timeline for viewing only
- restore the untouched live replay timeline when the imported viewer closes
- document the format in `docs/REPLAY_FORMAT.md`

### 15.4 — Replay hardening / closeout

15.4 adds no gameplay feature. It turns the completed replay line into a permanent certification target.

Closeout evidence:

- replay-file ring round-trip is exercised through 600 recorded frames; the bounded 256-frame import preserves logical frames 345 through 600, absolute total 600, and truncated state
- host layout is 118 bytes per replay snapshot and 30,232 bytes per 256-frame timeline
- the worst normal import-viewer state holds the live timeline plus one transient imported timeline: 60,464 bytes total, below the frozen 64 KiB replay-memory ceiling
- the 33-ply local Opera Game proves replay accounting across its production Save/Load path: 17 frames at the checkpoint, 19 after two more plies, one honest baseline after Load, and 18 frames at final checkmate after replaying the continuation
- the Opera terminal viewer opens on the CHECKMATE snapshot and returns to the exact unchanged postgame state
- deterministic CPU full games finish with their newest replay snapshot matching the live terminal state:
  - EASY: draw by insufficient material after 36 White turns, 73/73 replay frames
  - MEDIUM: checkmate after 25 White turns, 51/51 replay frames
  - HARD: checkmate after 7 White turns, 15/15 replay frames
- all three CPU replay rings report the correct non-truncated state and zero browser errors
- the existing replay export/import transient-swap browser regression remains green
- strict C89 host tests, Open Watcom 16-bit DOS compilation, DOSBox platform smoke, 1.0.0 packaging, Web/WASM runtime, and the existing 32 native visual-review states all remain green

Build 15 therefore closes with replay as a bounded read-only snapshot system and a separate versioned file format. Playback never re-simulates chess or Gas rules.

## Frozen contracts

Build 15 does not change:

- legal chess
- Gas earning/spending
- Fart displacement or promotion rules
- CPU search/evaluator/difficulty weights
- CPU mode default
- local two-player semantics
- version-2 save format
- version-1 save compatibility
- audio behavior
- board geometry, assets, or palette
- packaged version 1.0.0

Any future replay file is a separate artifact and must not overload the game-save format.
