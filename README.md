# Chess Fart

**Chess Fart** is a deliberately ridiculous 256-color VGA strategy game: real chess rebuilt as a loud early-1990s DOS game where pieces build Gas and use directional fart blasts as tactical abilities.

The design goal is **real strategy first, toilet humor second**.

## Project status

**Build 11 — UX / Polish: complete in source/host validation; CI passes.**

Chess Fart now includes:

- complete legal chess plus Gas/Fart displacement rules
- human White versus deterministic CPU Black
- 320x200 256-color VGA presentation
- Sound Blaster / PC-speaker audio architecture
- versioned save/load and audio config
- session action history
- in-game rules/help and credits
- polished checkmate/stalemate/draw overlays
- title-screen attract demo
- optional DOS mouse support

See [`docs/BUILD_11.md`](docs/BUILD_11.md) and [`docs/UX_DESIGN.md`](docs/UX_DESIGN.md).

## Controls

```text
Title:
Up/Down          Menu
Enter / Space    Choose
Left/Right       Audio device
F                 SFX level
D                 CPU difficulty
H                 Help
C                 Credits

Game:
Arrows           Cursor / cardinal Fart aim
Keypad diagonals Diagonal Fart aim
Enter / Space    Select / confirm
F                 Fart mode
S / L             Save / Load
H / ?             Help
Tab / M           Action log
C                 Credits
Esc               Quit
```

With a compatible DOS mouse driver, left-click targets board/menu items and right-click enters Fart mode.

## Build

### Host verification

```sh
make test-build11
```

This reruns the permanent Build 4–10 suites, runs Build 11 UX tests, executes the scripted VGA/audio/save-load CPU game, and verifies dedicated Build 11 title/demo/help/history/credits/terminal artifacts.

### DOS / Open Watcom

```bat
wmake -f makefile.build11.dos dos
```

or:

```bat
scripts\build_dos_build11.bat
```

Expected output: `build\dos\CHESSFRT.EXE`.

Open Watcom/DOSBox remains required for final DOS runtime, mouse-driver, Sound Blaster and CPU-speed validation.

## Documentation

- [`docs/MASTER_PLAN.md`](docs/MASTER_PLAN.md)
- [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md)
- [`docs/VGA_ART_STYLE.md`](docs/VGA_ART_STYLE.md)
- [`docs/AUDIO_DESIGN.md`](docs/AUDIO_DESIGN.md)
- [`docs/SAVE_FORMAT.md`](docs/SAVE_FORMAT.md)
- [`docs/CPU_DESIGN.md`](docs/CPU_DESIGN.md)
- [`docs/UX_DESIGN.md`](docs/UX_DESIGN.md)
- [`docs/ROADMAP.md`](docs/ROADMAP.md)
- [`docs/BUILD_11.md`](docs/BUILD_11.md)

**CHESS FART** — *Check. Mate. Ventilate.*
