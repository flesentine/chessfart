# Chess Fart

**Chess Fart** is a deliberately ridiculous 256-color VGA strategy game: real chess rebuilt as a loud early-1990s DOS game where pieces build Gas and use directional fart blasts as tactical abilities.

The design goal is **real strategy first, toilet humor second**.

## Project status

**Build 12 — Chess Fart 1.0.0-rc1: release candidate complete.**

The automated RC gate now covers the entire game from rules through the DOS package:

- complete legal chess plus Gas/Fart displacement rules
- human White versus deterministic CPU Black
- 320x200 256-color VGA presentation
- Sound Blaster / PC-speaker audio architecture
- versioned save/load and audio config
- session history, help, credits and attract demo
- optional DOS mouse support
- strict C89 Build 4–11 regression stack
- host CPU/memory profiling and release-source audit
- real 16-bit DOS `CHESSFRT.EXE` built with Open Watcom 2.0
- DOSBox platform smoke covering VGA, mouse probe and audio initialization
- verified `chessfart-1.0.0-rc1-dos.zip` with SHA-256

See [`docs/BUILD_12.md`](docs/BUILD_12.md), [`docs/COMPATIBILITY.md`](docs/COMPATIBILITY.md), and [`docs/RELEASE_CHECKLIST.md`](docs/RELEASE_CHECKLIST.md).

## Play the RC

The CI/release package contains:

```text
CHESSFRT/
  CHESSFRT.EXE
  DOSBOX.CONF
  README.TXT
  KEYS.TXT
  KNOWNISS.TXT
  BUILDINFO.TXT
```

Extract the directory and run `CHESSFRT.EXE` in DOS/DOSBox. The included `DOSBOX.CONF` provides a reference Sound Blaster configuration.

## Controls

```text
Title:
Up/Down          Menu
Enter / Space    Choose
Left/Right       Audio device
F                SFX level
D                CPU difficulty
H                Help
C                Credits

Game:
Arrows           Cursor / cardinal Fart aim
Keypad diagonals Diagonal Fart aim
Enter / Space    Select / confirm
F                Fart mode
S / L            Save / Load
H / ?            Help
Tab / M          Action log
C                Credits
Esc              Quit
```

With a compatible DOS mouse driver, left-click targets board/menu items and right-click enters Fart mode.

## Verification

Host RC gate:

```sh
make test-build12
```

With Open Watcom set up for a DOS target:

```sh
sh scripts/build_dos_ci.sh
```

Package the compiled DOS executable:

```sh
sh scripts/package_release.sh
```

Classic Open Watcom users may use:

```bat
wmake -f makefile.build12.dos dos
```

Automated verification is intentionally not described as exhaustive hardware compatibility. Physical 386/486 systems, MS-DOS/FreeDOS versions, real Sound Blaster models and third-party mouse drivers remain manual/community RC targets.

## Documentation

- [`docs/MASTER_PLAN.md`](docs/MASTER_PLAN.md)
- [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md)
- [`docs/VGA_ART_STYLE.md`](docs/VGA_ART_STYLE.md)
- [`docs/AUDIO_DESIGN.md`](docs/AUDIO_DESIGN.md)
- [`docs/SAVE_FORMAT.md`](docs/SAVE_FORMAT.md)
- [`docs/CPU_DESIGN.md`](docs/CPU_DESIGN.md)
- [`docs/UX_DESIGN.md`](docs/UX_DESIGN.md)
- [`docs/COMPATIBILITY.md`](docs/COMPATIBILITY.md)
- [`docs/RELEASE_CHECKLIST.md`](docs/RELEASE_CHECKLIST.md)
- [`docs/ROADMAP.md`](docs/ROADMAP.md)
- [`docs/BUILD_12.md`](docs/BUILD_12.md)

**CHESS FART** — *Check. Mate. Ventilate.*
