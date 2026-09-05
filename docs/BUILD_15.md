# Chess Fart — Build 15 Replay / Postgame Review

Build 15 adds replay and postgame review without changing chess legality, Gas/Fart rules, CPU evaluation, or the version-2 game-save contract.

**15.0 complete. Current slice: 15.1 replay viewer modal.**

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

- expose replay from terminal state and normal in-game controls
- add optional mouse stepping on the same input abstraction
- harden promotion/Fart/checkmate frames in the viewer
- keep live-game input locked while reviewing old frames

### 15.3 — Replay files

- define a separate versioned replay format, likely `CHESSFRT.RPL`
- export/import replay data without changing `CHESSFRT.SAV` v2
- validate malformed/truncated replay files transactionally
- document compatibility separately from save files

### 15.4 — Replay hardening / closeout

- full CPU/local replay regression
- long-game replay coverage
- DOS memory/package review
- final documentation and certification

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
