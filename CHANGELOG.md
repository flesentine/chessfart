# Changelog

All notable project milestones will be recorded here.

## Unreleased

### Build 1 — VGA boot

Added:

- first C89/C90 application code
- Open Watcom DOS build target for `CHESSFRT.EXE`
- BIOS Mode 13h entry and text-mode 03h restoration
- 64,000-byte indexed software backbuffer
- VGA DAC palette programming
- clipped pixel and rectangle primitives
- compact built-in 5x7 bitmap font
- static 8x8 chessboard and right-side VGA status panel
- DOS keyboard polling with Escape-to-exit
- host VGA/input backends
- strict host compile and deterministic framebuffer smoke test
- `docs/BUILD_1.md`

Verified:

- `make test-build1` passes with `-std=c89 -pedantic -Wall -Wextra -Werror`
- host backend writes a valid 320x200 PPM preview

Not yet verified in this environment:

- Open Watcom compilation and DOSBox runtime, because Open Watcom is not installed in the build environment used for this milestone

### Build 0 — foundation

Added the game concept, 320x200/256-color VGA target, deterministic Gas/Fart ruleset, master plan, VGA art spec, architecture, audio plan, asset manifest, roadmap, tests, toolchain plan, decisions, and contribution conventions.

### Current phase

Build 1 complete in source. Build 2 — cursor and pieces — is next.
