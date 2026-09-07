# Chess Fart — Technical Architecture

## 1. Design rule

Game truth stays separate from presentation and platform code. Chess/Gas legality never depends on VGA, audio, mouse, browser, or DOS hardware behavior.

## 2. Runtime target

The production DOS build uses C89/C90-compatible C and Open Watcom 2.0 to produce a real 16-bit DOS executable using the large memory model. The same gameplay sources also compile as a modern host executable and as WebAssembly.

The fixed presentation target is 320x200, 8-bit indexed VGA-style output.

## 3. Active runtime structure

### Game entry and flow

- `src/main.c` owns process-level state and initializes the final runtime.
- `src/main_loop.inc` contains the keyboard-driven match loop.
- `src/main_hooks.inc` integrates CPU turns, mouse input, save/load mode handling, replay, and Practice Undo.
- `src/main_ui.inc` implements the title screen, HUD, modals, replay viewer, terminal overlays, and input hit regions.
- `src/main_review.inc` exposes review-only hooks used by the browser regression harness.

The include files are intentionally part of one translation unit because they share bounded DOS-era state. Historical standalone build entrypoints are no longer part of the supported tree.

### Rules and state

- `board.c`: standard chess state, move generation, attacks, special moves, terminal/draw state.
- `gas.c`: per-piece Gas, Fart actions, displacement, Gas-aware history and legality.
- `practice_undo.c`: bounded reversible Practice journal.

### CPU

`cpu_config.c`, `cpu_actions.c`, `cpu_eval.c`, `cpu_search.c`, and `cpu_format.c` provide deterministic bounded search over the same legal move/Fart path used by human play.

### Presentation

- `board_view_art.c`: current board renderer and title base.
- `ui_assets.c` / `ui_theme.c`: generated sprites and theme policy.
- `font.c`: bitmap text.
- `presentation.c`: visual rendering and Fart animation without gameplay-audio side effects.
- `audio_game.c`: move/Fart audio and gameplay-status sound cues layered around presentation.

### Persistence and review

- `persistence.c`: game-save v2 and config v2.
- `replay.c`: bounded in-memory replay timeline.
- `replay_file.c`: transactional replay-file v1 import/export.
- `ux.c`: compact action-history formatting/state.

## 4. Platform boundary

The shared runtime calls interfaces in `include/`. Backends live in:

- `src/platform/dos/`: Mode 13h, keyboard, mouse, Sound Blaster/PC-speaker paths
- `src/platform/host/`: deterministic test input, PPM capture, WAV/log diagnostics
- `src/platform/web/`: Canvas framebuffer, browser keyboard/pointer input, Web Audio, IDBFS

Platform code must not invent gameplay state transitions.

## 5. Persistence contracts

The current compatibility anchors are deliberately frozen:

- `CHESSFRT.SAV`: game-save format v2; legacy v1 remains loadable
- `CHESSFRT.RPL`: replay-file format v1
- `CHESSFRT.CFG`: config format v2 with legacy v1 migration

Serialization is explicit; raw compiler structs are not persisted.

## 6. Memory discipline

The DOS build avoids heap allocation in hot rules/search paths. The replay timeline is the main intentional runtime heap allocation and is bounded. Practice Undo is bounded at compile time. CI profiles structure sizes and executable size so DOS memory regressions are visible.

## 7. Verification architecture

The supported gates are:

- `make test`: strict host C89 regression, scripted host run, profile/size report, release audit
- Python generated-asset/font verification
- `sh scripts/build_dos_ci.sh`: Open Watcom 16-bit executable + DOS smoke binary
- DOSBox smoke of VGA/mouse/audio initialization
- `make -f Makefile.web web` plus Chromium runtime/gameplay/visual regression
- release ZIP audit and integrity checks

The 36-state browser visual review includes pinned final-theme signatures. Full CPU and local-game browser scenarios validate real input and terminal behavior.

## 8. Change policy

Maintenance should reduce complexity or fix a demonstrated issue. Do not add compatibility wrappers merely to preserve historical build filenames; Git history and `docs/BUILD_*.md` already preserve that history.
