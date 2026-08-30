# Chess Fart — Build 1: VGA Boot

## Status

**Complete in source. Host smoke test passes.**

The DOS target is ready for Open Watcom compilation. Open Watcom was not available in the environment used to prepare this build, so no precompiled `CHESSFRT.EXE` is committed and DOSBox runtime validation is still an integration check.

## Goal

Establish the minimum real game executable layer before adding pieces or chess logic.

Build 1 intentionally does not contain chess state, movement, sprites, mouse input, audio, animation, or Gas mechanics.

## Delivered

### Platform-neutral shell

`src/main.c` initializes video and input, renders the Build 1 screen into the software framebuffer, presents the frame, waits for Escape, and restores the platform on shutdown. The application does not include DOS headers.

### VGA interface

`include/vga.h` defines 320x200 dimensions, the 64,000-byte frame size, 256-color palette constants, initialization/shutdown, palette load, clear, pixel plot, filled rectangle, and present.

### DOS backend

`src/platform/dos/vga_dos.c` uses BIOS interrupt `10h` mode `13h`, VGA framebuffer `A000:0000`, a far-allocated 64,000-byte software backbuffer, VGA DAC ports `3C8h`/`3C9h`, and BIOS mode `03h` on shutdown.

Every normal program exit passes through `vga_shutdown()`.

### Input

The DOS backend uses `kbhit()`/`getch()` for Build 1 polling. Escape exits. Extended keys are safely consumed and ignored for now.

### Bitmap font

A built-in 5x7 uppercase/digit font is rendered entirely through framebuffer pixels. No BIOS text rendering is needed after entering graphics mode.

### Build 1 screen

The static screen includes the `CHESS FART` title, tagline, 8x8 board, gold board border, right-side status panel, Mode 13h/320x200/256-color labels, mock Gas meter, and visible Escape instruction. This is deliberately a UI shell, not final art.

### Host backend

The host backend implements the same VGA interface in memory. `vga_present()` writes `build/host/chessfart_build1.ppm`, giving us a deterministic framebuffer artifact that can be smoke-tested without DOS. The host input backend returns Escape immediately after the first frame so automated tests never hang.

## Build commands

### Host

```sh
make test-build1
```

Expected final line:

```text
Build 1 host smoke test passed.
```

### DOS

Use an Open Watcom environment:

```bat
wmake -f makefile.dos dos
```

or:

```bat
scripts\build_dos.bat
```

Expected output:

```text
build\dos\CHESSFRT.EXE
```

## Verification performed

The host build was compiled with `-std=c89 -pedantic -Wall -Wextra -Werror -O2`. The smoke target successfully compiled and linked the host version, ran it, produced a non-empty preview, and confirmed the 320x200 framebuffer path works.

## DOSBox verification checklist

Once Open Watcom is available:

1. run the DOS build
2. launch `CHESSFRT.EXE` in DOSBox
3. confirm 320x200 graphics mode
4. confirm board/panel colors match the host preview closely
5. press random extended keys and confirm no exit
6. press Escape
7. confirm DOS text mode is restored
8. launch/exit repeatedly to catch cleanup problems

## Known Build 1 constraints

- backbuffer present is a straightforward 64,000-byte copy; optimization can wait
- keyboard polling is busy-waiting; timer-driven pacing arrives later
- palette is code-defined rather than loaded from an asset file
- host preview is PPM to avoid an image-library dependency
- only glyphs needed by the current UI are implemented

## Build 2 handoff

Build 2 should add a real board model sufficient to place 32 starting pieces, placeholder VGA piece sprites, a board cursor, arrow-key navigation, selected-square state, and side-panel selection information. Do not add legal move rules yet; those belong to Build 3.
