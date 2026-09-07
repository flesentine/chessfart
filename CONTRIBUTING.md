# Contributing to Chess Fart

## Project state

Chess Fart is feature-complete. Contributions should default to **maintenance**, not expansion: fix defects, improve compatibility, strengthen tests, simplify code, or improve documentation without changing the established game.

A funny feature is not worth corrupting chess state.

## Supported build paths

- host regression gate: `make test`
- browser build: `make -f Makefile.web web`
- Open Watcom DOS build: `sh scripts/build_dos_ci.sh`
- release package: `sh scripts/package_release.sh`

Historical `docs/BUILD_*.md` files describe how the game evolved; they are not separate supported build systems.

## Code organization

- rules and game-state truth: `src/game/board.c`, `src/game/gas.c`
- CPU: `src/game/cpu_*.c`
- current game flow: `src/main.c`, `src/main_loop.inc`, `src/main_hooks.inc`, `src/main_ui.inc`
- presentation/assets/themes: `src/game/board_view_art.c`, `src/game/presentation.c`, `src/game/ui_*.c`
- persistence/replay/undo: `src/game/persistence.c`, `src/game/replay*.c`, `src/game/practice_undo.c`
- platform backends: `src/platform/{dos,host,web}`
- shared declarations: `include/`
- regression tests: `tests/`
- asset converters: `tools/`

Rules code must remain independent of platform VGA/audio/input implementations.

## C style

- stay within the C89/C90 subset used by the DOS target
- favor small functions and explicit state transitions
- avoid clever macro indirection when a normal function or module boundary is clearer
- no hidden heap allocation in hot move-generation/search paths
- comments explain invariants and reasons, not build-history trivia

## Compatibility contracts

Do not casually change:

- chess/Gas/Fart rules
- game-save format v2
- replay-file format v1
- config format v2
- deterministic CPU regression expectations
- certified native visual signatures

A deliberate contract change requires matching tests and documentation.

## Testing

Before merging maintenance work, run the relevant focused tests and the full gate. Changes touching presentation or browser input also require the Chromium playtest and 36-state visual review. Changes touching DOS/platform code require the Open Watcom and DOSBox gates.

## Humor guideline

Keep the joke playful and game-like. The strongest tone is serious chess presentation interrupted by absurd gas mechanics, not constant random gross-out text.
