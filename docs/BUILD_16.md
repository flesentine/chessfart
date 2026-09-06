# Chess Fart — Build 16 Practice / Undo

Build 16 closes the remaining required item from the original master plan: **Undo in practice/debug mode**.

The goal is not to make normal CPU or local matches casually rewindable. Undo belongs to an explicitly marked Practice session so competitive/default match semantics stay unchanged.

**Build 16 complete — 16.3 hardening / closeout certified.**

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

- add a third playable title choice: `PRACTICE`
- Practice uses local White/Black turns on the trusted existing rules path, with no CPU reply
- `U` undoes the latest committed move or Fart through the 16.0 journal
- successful undo clears selection, promotion/Fart aim, and terminal presentation state
- replay resets to one honest `UNDO START` baseline after an undo; exact replay/action-log rollback remains 16.2
- Save and Load return `DISABLED` in Practice so the session policy is never smuggled into save v2
- normal `PLAY CPU` and `2 PLAYERS` remain unchanged
- Practice HUD/help explicitly identify `U UNDO` and the session-only policy

### 16.2 — Practice integration

16.2 upgrades Undo from an honest-baseline reset to exact session-history rollback.

- each committed Practice action captures compact deltas for the UX action-log ring and replay timeline ring before those rings append
- `U` restores board, Gas, repetition history, action log, and replay timeline as one synchronized transaction
- full-ring rollback restores the displaced oldest UX line/replay snapshot instead of merely decrementing counters
- replay `count`, `total`, `start`, and `truncated` return to their exact pre-action values
- the Practice command-bar `U UNDO` region is mouse-clickable
- entering a playable title mode resets both the game undo journal and presentation undo journal
- Save/Load remain disabled in Practice; no save-v2 policy change
- Chromium proves exact move/Fart/terminal/repeated rollback while retaining the prior replay/action history

### 16.3 — Hardening / closeout

16.3 freezes the bounded-session semantics and proves them across all retained-history limits.

- the undo window remains exactly 32 committed Practice actions
- when a 33rd newer action displaces the oldest undo record, Undo never crosses that discarded boundary
- a host stress sequence runs beyond the 128-position repetition ring, then undoes the latest 32 actions and requires byte-for-byte board, Gas, and repetition-history restoration
- Chromium separately proves a real-input threefold draw is terminal-locked and that `U` reopens the exact pre-draw Practice state
- a review-only 300-action production-commit-path stress rolls the 32-entry undo journals, 32-line action log, 128-position repetition history, and 256-frame replay timeline past their retention boundaries
- Chromium then undoes exactly 32 actions and requires the full board/Gas/history state plus raw action-log and replay-ring hashes to match the 268-action checkpoint
- a 33rd Undo must be a transactional no-op
- compile-time guards keep the presentation undo journal under 8 KiB and both Practice undo journals together under 16 KiB
- Save v2, replay-file v1, rules, CPU behavior, and packaged version remain unchanged

### 16.3 certification evidence

The closeout gate proves:

- the 169-action host stress crosses the 128-position repetition-history boundary, retains only the newest 32 undo records, and restores the exact checkpoint after 32 undos
- real mouse/keyboard Practice play reaches a genuine threefold draw, rejects further gameplay input, and `U` restores a live pre-draw state
- the separate 300-action review commit-path stress overflows the 32-entry game/presentation undo journals, 32-line action log, 128-position repetition history, and 256-frame replay ring together
- after 32 undos, board, Gas, repetition history, action log, and replay timeline match the 268-action checkpoint exactly
- a 33rd undo is a transactional no-op because the older journal boundary has been deliberately discarded
- ordinary promotion, Fart-push promotion, castling, en passant, normal moves, Farts, repeated undo, terminal undo, and mouse Undo all remain certified
- the canonical browser visual suite remains at 32 states
- Open Watcom still produces the real 16-bit DOS `CHESSFRT.EXE`; the certified executable is 135,154 bytes
- the host profile still reports a 118-byte replay snapshot, 30,232-byte live replay timeline, and 60,464-byte live+transient-import peak
- compile-time guards keep the presentation undo journal below 8 KiB and both Practice undo journals together below 16 KiB

Build 16 closes with no save/config/replay-file format bump and no change to the packaged version number.
