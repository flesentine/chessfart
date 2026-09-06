# Chess Fart — Build 16 Practice / Undo

Build 16 closes the remaining required item from the original master plan: **Undo in practice/debug mode**.

The goal is not to make normal CPU or local matches casually rewindable. Undo belongs to an explicitly marked Practice session so competitive/default match semantics stay unchanged.

**Current slice: 16.0 reversible-action journal foundation.**

## Frozen contracts

Build 16 does not change:

- legal chess
- Gas earning/spending
- Fart displacement or promotion rules
- CPU search/evaluator/difficulty
- normal `PLAY CPU` behavior
- normal `2 PLAYERS` behavior
- version-2 `CHESSFRT.SAV`
- version-1 `CHESSFRT.RPL`
- config format
- board geometry, palette, assets, or package version

Practice is a session policy, not a third serialized `CfMatchMode` value.

## 16.0 — Reversible-action journal foundation

16.0 adds no player-facing control.

It introduces a bounded journal for committed normal moves and Fart Actions using the existing trusted unmake payloads:

- `CfGasMove` for normal moves, captures, castling, en passant, and promotion
- `CfFartAction` for PUFF, blocked Farts, pushes, castling-right displacement, and pushed promotion
- a compact repetition-history delta for exact history rollback

### Repetition-history rule

The Gas repetition history holds at most 128 position keys. When full, recording a new position shifts out the oldest key.

A correct undo therefore cannot merely decrement `history.count`.

Each journal entry stores:

- the history count before the committed action was recorded
- whether the history buffer was already full
- the one history key that the next record will overwrite or displace

On undo:

- board and Gas are restored through the existing move/Fart unmake routine
- a non-full history restores the overwritten next slot and returns to its prior count
- a full history shifts right and restores the exact dropped oldest key

This keeps exact rollback without copying the ~8 KiB repetition history into every undo entry.

### Memory policy

The journal retains the latest 32 committed actions.

Host regression enforces:

- `sizeof(CfPracticeUndoEntry) < 256`
- `sizeof(CfPracticeUndoJournal) < 8192`

The journal is a bounded ring: once full, the oldest undo record is discarded.

### 16.0 regression gate

The host gate must prove:

1. a normal move restores board, Gas, and repetition history byte-for-byte
2. a Fart push restores board, Gas, and repetition history byte-for-byte
3. rollback remains exact when the 128-entry repetition history shifts out its oldest key
4. a history-shape mismatch refuses undo transactionally without changing board or Gas
5. the 32-entry undo journal remains bounded
6. the foundation compiles under strict C89, Web/WASM, and Open Watcom 16-bit DOS

## Planned Build 16 slices

### 16.1 — Local Practice session

- expose an explicit Practice entry from the title flow
- use local White/Black play on the trusted existing rules path
- add `U` to undo the latest committed action
- clear selection, promotion/Fart aim, and terminal overlays after a successful undo
- keep ordinary `2 PLAYERS` unchanged
- do not serialize the Practice policy into save v2

### 16.2 — Practice integration

- roll back session action-log and replay timeline entries with the same committed action
- add mouse/help/HUD discoverability
- define Load/new-game behavior for the journal
- harden ordinary promotion, Fart-push promotion, castling, en passant, and terminal undo

### 16.3 — Hardening / closeout

- repeated multi-action undo
- journal-overflow behavior
- repetition-history rollover
- long-session Chromium regression
- DOS memory/package review
- final documentation and certification
