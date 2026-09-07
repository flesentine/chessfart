# Chess Fart — Build 17 Alternate Presentation Themes

Build 17 starts the first optional post-v1 roadmap item: **alternate boards/pieces**.

The implementation stays deliberately conservative. Themes are presentation policy only. They must never alter chess state, Gas, CPU behavior, persistence, replay content, Practice semantics, or package version.

**17.0–17.3 complete. Current slice: 17.4 theme-specific piece materials.**

## Frozen contracts

Build 17 does not change:

- legal chess, Gas, or Fart rules
- CPU search/evaluator/difficulty
- CPU/local/Practice match semantics
- version-2 `CHESSFRT.SAV`
- version-1 `CHESSFRT.RPL`
- game-save and replay-file formats
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

This keeps 17.0 focused on architecture and visual proof. Its exact-head certification passed strict host, Open Watcom DOS, DOSBox/package, Web/WASM, Chromium gameplay, and the 34-state native visual suite.

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


## 17.1 — Player-facing title theme selector

17.1 exposes the certified theme foundation without adding persistence coupling.

- `T` on the title screen cycles Royal Basement -> Crimson Cellar -> Royal Basement
- the title footer always names the active theme
- the selected theme carries into CPU, 2 PLAYERS, or PRACTICE for the current process/session
- `T` during gameplay is intentionally a no-op; theme selection remains a title policy
- Save/Load does not change the selected session theme
- startup still resets to Royal Basement
- audio config persistence remains unchanged
- game-save v2 and replay-file v1 remain unchanged

### 17.1 validation target

- strict C89 theme-cycle tests
- DOS and Web input backends map T/t to the appended theme key without renumbering earlier keys
- real Chromium title input proves two-way wrap
- Crimson Cellar carries from title into an actual CPU opening
- in-game T does not mutate theme or gameplay
- save/load preserves the current session theme without persisting it
- canonical native visual suite expands to 36 states:
  - real Crimson Cellar title selection
  - real Crimson Cellar game opening


## 17.2 — Persistent theme config

17.2 promotes the certified title selector from session-only state into the existing settings file while keeping game and replay persistence untouched.

- config format advances from version 1 to version 2
- v2 appends `THEME <id>` after the existing `AUDIO` record
- Royal Basement = 0; Crimson Cellar = 1
- legacy config v1 remains loadable and defaults theme to Royal Basement
- malformed/unsupported config loads are transactional and leave startup defaults intact
- changing theme on the title screen writes audio + theme atomically
- Web IDBFS restart coverage proves Crimson survives a real browser runtime reload
- changing a legacy v1 config rewrites it as current v2
- `CHESSFRT.SAV` remains version 2
- `CHESSFRT.RPL` remains version 1

### 17.2 validation target

- strict C89 config v2 round-trip and migration tests
- legacy audio-only persistence API remains source-compatible
- invalid theme data cannot partially mutate audio/theme outputs
- real Open Watcom 16-bit DOS compile
- DOSBox/package regression
- Web/WASM runtime
- Chromium config text proof, IDBFS flush/reload, v1 fallback/migration, and malformed-v2 fallback
- existing 36-state visual suite remains unchanged


## 17.3 — Authored board surfaces

17.3 moves the alternate-theme work beyond palette swaps while preserving the exact Royal Basement baseline.

- Royal Basement keeps its established sparse stone-grain square pixels exactly
- Crimson Cellar receives a distinct staggered brick/mortar treatment with chipped corner details
- texture marks stay near square edges so the 16x18 piece silhouettes remain dominant
- theme-to-surface mapping lives in the shared theme API rather than renderer-local theme checks
- the same authored surface path is used by host, Web/WASM and DOS renderers
- no new theme/config ID is added; config v2 continues to persist Royal/Crimson exactly as in 17.2
- piece sprite masks, board geometry, gameplay state, replay state and save formats remain unchanged

### 17.3 validation target

- strict C89 theme tests certify Royal -> stone grain and Crimson -> cellar brick mapping
- invalid theme values fail closed to the Royal stone surface
- Royal Basement retains its exact established native visual signature after theme switch/restore
- real Chromium review proves the active surface follows the title-selected theme
- Crimson title/game/checkmate states render through the authored brick surface
- board/Gas/history/replay hashes remain unchanged by presentation switching
- Open Watcom 16-bit DOS, DOSBox/package and Web/WASM gates remain green
- canonical visual suite remains 36 native 320x200 states


## 17.4 — Theme-specific piece materials

17.4 completes the first board/piece visual pairing without introducing a second sprite bank.

- Royal Basement keeps the existing classic gilt/neutral semantic piece mapping exactly
- Crimson Cellar remaps only the authored accent class to copper for both armies
- white ivory/base/highlight and black charcoal/base/highlight roles remain distinct
- all packed 16x16 piece masks remain byte-identical
- the shared theme API owns the piece-material selection; the asset renderer consumes it
- invalid theme values fail closed to the Royal classic-gilt material
- no new config/theme IDs are added and config v2 remains unchanged
- board geometry, Gas/Fart presentation, gameplay, CPU, save v2 and replay v1 remain unchanged

### 17.4 validation target

- strict C89 theme tests certify Royal -> classic gilt and Crimson -> cellar copper
- invalid theme values map to Royal classic gilt
- Chromium review exposes and checks the active piece material alongside the board surface
- Royal Basement retains its exact established native visual signature after Crimson switch/restore
- Crimson game/checkmate frames change while board/Gas/history/replay hashes remain invariant
- Open Watcom 16-bit DOS, DOSBox/package and Web/WASM gates remain green
- canonical visual suite remains 36 native 320x200 states
