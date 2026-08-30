# Chess Fart — Build & Toolchain Plan

## 1. Primary target

Authentic DOS-style executable with VGA Mode 13h.

Recommended compiler: **Open Watcom C/C++**.

Language policy: C89/C90-compatible C for portability and period-appropriate simplicity.

## 2. Proposed repository layout

```text
chessfart/
  README.md
  CONTRIBUTING.md
  docs/
  src/
    core/
    ai/
    game/
    platform/
      dos/
  include/
  tests/
  assets/
    source/
    runtime/
  tools/
  scripts/
  build/
```

Generated build output should not be committed unless it is a deliberate release artifact.

## 3. DOS build concept

Initial output:

```text
build/dos/CHESSFRT.EXE
```

Potential make targets:

- `make dos`
- `make host-tests`
- `make assets`
- `make clean`
- `make release`

Exact Open Watcom make syntax can be chosen in Build 1.

## 4. Development environments

### Modern host

Use host compiler for pure rules tests.

Benefits:

- fast iteration
- sanitizers/static analysis where available
- easy CI

### DOSBox

Primary runtime integration environment.

Test:

- graphics
- timer
- keyboard
- mouse
- Sound Blaster
- packaged launch flow

### Real DOS / PCem / 86Box

Compatibility stretch target after core game is stable.

## 5. VGA plan

Mode 13h:

- BIOS set mode 0x13
- 320x200
- 256 indexed colors
- framebuffer at A000:0000

On exit:

- restore text mode 0x03

All exits should pass through platform cleanup.

## 6. Asset pipeline

Author source assets in indexed PNG or other reproducible source formats.

Build tools convert them into runtime arrays/files:

- palette conversion
- sprite packing
- font packing
- PCM normalization

Tool source belongs in `tools/` and generated asset rules in the build system.

## 7. Host test build

`src/core` should compile with minimal/no platform defines.

A host test executable can load text fixtures and print pass/fail without VGA.

Suggested fixture notation can use FEN plus an extension for Gas, or a project-specific board text format.

Do not distort runtime design purely to support standard FEN; FEN does not represent Gas.

## 8. CI idea

GitHub Actions can run host-side unit tests even though the primary game is DOS.

Future workflow:

1. compile core tests with GCC/Clang
2. run regression suite
3. optionally build DOS target if Open Watcom environment is practical in CI
4. attach DOS binary only for tagged/release builds

## 9. Compiler discipline

- warning level high
- no implicit function declarations
- fixed-width assumptions documented
- avoid undefined signed overflow
- central typedefs for byte/word/dword if compiler headers vary
- avoid compiler-specific extensions in core logic

## 10. Debug build

Useful debug-only options:

- square coordinates
- Gas values on every piece
- legal action count
- current position hash
- FPS/tick counter
- AI nodes/second
- save state dump
- instant undo

Release builds hide these overlays.

## 11. Configuration file

Proposed `CHESSFRT.CFG` text format so users can edit it manually:

```ini
sound=auto
sfx_volume=3
music_volume=2
mouse=on
animations=on
ai_level=2
```

Unknown keys should be ignored safely.

## 12. Release package

Target ZIP contents:

```text
CHESSFRT.EXE
CHESSFRT.CFG
DATA/...
README.TXT
LICENSE.TXT (once a license is chosen)
DOSBOX.CONF (optional)
```

Do not choose a source-code license implicitly. Add one only after the repository owner decides.