# Chess Fart — Build 17 Alternate Presentation Themes

Build 17 starts the first optional post-v1 roadmap item: **alternate boards/pieces**.

The implementation stays deliberately conservative. Themes are presentation policy only. They must never alter chess state, Gas, CPU behavior, persistence, replay content, Practice semantics, or package version.

**Current slice: 17.0 alternate-theme foundation.**

## Frozen contracts

Build 17 does not change:

- legal chess, Gas, or Fart rules
- CPU search/evaluator/difficulty
- CPU/local/Practice match semantics
- version-2 `CHESSFRT.SAV`
- version-1 `CHESSFRT.RPL`
- config format
- replay timeline semantics
- logical 320x200 indexed-VGA geometry
- authored piece/puff sprite masks
- packaged version 1.0.0

## 17.0 — Palette-backed theme foundation

17.0 turns the Build 13 semantic VGA color roles into a runtime theme API.

Themes:

1. **Royal Basement** — the existing retail palette and unchanged default.
2. **Crimson Cellar** — a burgundy/copper alternate palette using the exact same semantic renderer and indexed sprite masks.

The theme API:

- validates named theme IDs
- exposes stable theme names
- builds the full 256-entry VGA palette from semantic roles
- preserves the grayscale tail used by existing tooling
- keeps flash-frame accent behavior theme-aware
- rejects invalid theme IDs without changing the active theme

Both the current host/Web retail renderer and the legacy DOS Build 7 renderer consume the same palette API.

### 17.0 player-facing policy

There is intentionally **no player-facing theme control yet**.

- startup remains Royal Basement
- no config field is added
- no save/replay field is added
- Crimson Cellar is reachable only through CF_WEB_REVIEW probes for certification

This keeps 17.0 focused on architecture and visual proof. A later slice can add a title/options control only after the foundation is certified.

## 17.0 validation target

- strict C89 host theme tests
- all VGA channels remain in the 0–63 DAC range
- Royal Basement and Crimson Cellar differ in board/panel/piece semantic roles
- flash palette changes the intended accent roles
- invalid theme IDs fail closed
- real Open Watcom 16-bit DOS compile
- Web/WASM build and runtime
- Chromium renders the exact same terminal board under both themes
- board, Gas, repetition history, action-log count, and replay accounting remain unchanged by a theme switch
- restoring Royal Basement reproduces the exact prior native-canvas signature
- canonical visual suite expands from 32 to 34 states
