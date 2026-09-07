# Chess Fart — Build & Toolchain

## Supported targets

Chess Fart has three maintained build paths:

- **Host / regression:** `make test`
- **WebAssembly:** `make -f Makefile.web web`
- **16-bit DOS:** `sh scripts/build_dos_ci.sh` with Open Watcom configured

Release packaging uses `sh scripts/package_release.sh`.

Historical per-build DOS makefiles and batch scripts were retired after the game was consolidated; Git history and the Build documents preserve them.

## Language and compiler

- C89/C90-compatible source
- high warning levels on the host build
- Open Watcom 2.0 for the automated DOS target
- Emscripten for the browser edition

The DOS CI command uses a 16-bit large-memory-model target and writes `build/dos/CHESSFRT.EXE`.

## Repository layout

```text
src/main.c               current runtime entry
src/main_*.inc           current flow/UI/review pieces
src/game/                rules, CPU, presentation, persistence, replay
src/platform/dos/        DOS hardware backends
src/platform/host/       deterministic test/capture backends
src/platform/web/        browser backends
include/                 shared interfaces
tests/                   C/Python regressions and DOS smoke
assets_src/              authored indexed source assets
tools/                   deterministic asset/font converters
scripts/                 supported DOS build, audit, package, browser harnesses
```

Generated output lives under `build/` and is not committed.

## Host gate

`make test` compiles with strict C89 warnings-as-errors, runs the permanent rules/CPU/persistence/replay/undo/theme tests, executes the scripted host game, validates capture/audio/save artifacts, records CPU/memory/size diagnostics, and runs the release-source audit.

## DOS gate

With `WATCOM` pointing at the Open Watcom installation:

```sh
sh scripts/build_dos_ci.sh
```

This builds both `CHESSFRT.EXE` and `DOSSMOKE.EXE`. CI then runs the smoke executable in DOSBox and requires `DOSSMOKE.OK`.

## Web gate

```sh
make -f Makefile.web web
```

The browser bundle uses the same game sources, an Emscripten platform layer, Asyncify for the polling runtime, Canvas for the indexed framebuffer, Web Audio, and IDBFS persistence.

## Assets

`tools/build_assets.py` and `tools/build_font.py` convert authored indexed PNG sources into deterministic generated C data. CI runs both converters in `--check` mode and executes their Python regression tests.

## Release package

`scripts/package_release.sh` packages the already-built DOS executable with DOSBox configuration and release text, verifies the ZIP, and emits SHA-256 metadata.

Do not introduce a second build system for maintenance work unless the existing supported paths cannot express a demonstrated requirement.
