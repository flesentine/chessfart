# Chess Fart

**Chess Fart** is a deliberately ridiculous 256-color VGA strategy game: real chess rebuilt as a loud early-1990s DOS game where pieces build Gas and use directional fart blasts as tactical abilities.

The design goal is **real strategy first, toilet humor second**.

## Project status

**Chess Fart 1.0.0 — complete and maintenance-only.**

The shipped game now includes CPU play, local two-player, Practice/Undo, replay export/import, save/load, mouse support, audio, and the Royal Basement / Crimson Cellar presentation themes. Gameplay rules and the persisted contracts are frozen: game-save v2, replay-file v1, and config v2 remain the compatibility anchors.

The repository is intentionally **not** pursuing the old optional feature backlog. Current work is limited to defects, compatibility, test hardening, and simplification of the existing game.

The automated gate covers rules, Gas/Fart legality, deterministic CPU tactics and full games, local play through checkmate, save/load rollback, replay and Practice Undo, 36 native 320x200 visual states, generated assets, strict C89 host builds, real Open Watcom 16-bit DOS compilation, DOSBox smoke, and release packaging.

Physical 386/486 systems, MS-DOS/FreeDOS variants, real Sound Blaster models, and third-party DOS mouse drivers remain documented manual/community compatibility targets rather than claimed automated coverage.

See [`docs/COMPATIBILITY.md`](docs/COMPATIBILITY.md), [`docs/RELEASE_CHECKLIST.md`](docs/RELEASE_CHECKLIST.md), and [`CHANGELOG.md`](CHANGELOG.md).

## Play 1.0

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
Enter / Space    Choose (PLAY CPU / 2 PLAYERS / PRACTICE)
Left/Right       Audio device
F                SFX level
D                CPU difficulty
T                Theme: Royal Basement / Crimson Cellar
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
R                Replay viewer; Left/Right step, R/Enter/Esc close
U in Practice    Undo latest committed move or Fart; preserves prior log/replay
                 Latest 32 committed Practice actions are retained for Undo
Mouse in Practice Click U UNDO in the command bar
S / L in Practice Disabled
S / L in Replay  Export / import CHESSFRT.RPL
Mouse in Replay  Previous / Next / Export / Import / Close footer controls
Esc              Quit; cancel pending Fart/promotion choice
```

With a compatible DOS mouse driver, left-click targets board/menu items and right-click enters Fart mode.

## Verification

Host release gate:

```sh
make test
```

With Open Watcom set up for a DOS target:

```sh
sh scripts/build_dos_ci.sh
```

Package the compiled DOS executable:

```sh
sh scripts/package_release.sh
```


Automated verification is intentionally not described as exhaustive hardware compatibility. See `docs/COMPATIBILITY.md` for exactly what is verified and what remains unverified.

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
- [`docs/BUILD_13.md`](docs/BUILD_13.md)
- [`docs/BUILD_14.md`](docs/BUILD_14.md)
- [`docs/BUILD_15.md`](docs/BUILD_15.md)
- [`docs/BUILD_16.md`](docs/BUILD_16.md)
- [`docs/BUILD_17.md`](docs/BUILD_17.md)
- [`docs/REPLAY_FORMAT.md`](docs/REPLAY_FORMAT.md)

**CHESS FART** — *Check. Mate. Ventilate.*
