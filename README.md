# Chess Fart

**Chess Fart** is a deliberately ridiculous 256-color VGA strategy game: real chess rebuilt as a loud early-1990s DOS game where pieces build Gas and use directional fart blasts as tactical abilities.

The design goal is **real strategy first, toilet humor second**.

## Project status

**Chess Fart 1.0.0 — final release.**

The source on `main` also includes the post-v1 **Build 13 presentation refresh** and **Build 14 local two-player expansion**. Build 13 modernizes the authored VGA presentation; Build 14 adds `2 PLAYERS` hot-seat play with backward-compatible v1 saves and mode-aware v2 saves. Build 15 completes replay/postgame review in DOS-safe slices: a bounded session timeline, read-only keyboard/mouse viewer, postgame UX, separate versioned CHESSFRT.RPL export/import, and 15.4 certification across long sessions, CPU/local terminal games, DOS memory, packaging, and browser invariants—without changing game-save v2. Build 16 begins the remaining master-plan Practice/Undo requirement with a bounded exact-rollback journal foundation.

The automated release gate covers the entire game from rules through the DOS package:

- complete legal chess plus Gas/Fart displacement rules
- `PLAY CPU`: human White versus deterministic CPU Black
- `2 PLAYERS`: local hot-seat White versus Black on the same rules path
- Fart-aware CPU tactical regressions, including check escape, forcing check, castling-right damage and pushed promotion
- corrected Fart edge-bonus alpha-beta pruning
- deterministic full Chromium CPU games at Easy, Medium and Hard, each exercising live Fart mechanics
- a 33-ply local two-player Opera Game Chromium regression from title selection through checkmate
- exact version-2 LOCAL save/load rollback of board metadata, all piece/Gas squares and repetition history
- bounded replay timeline plus transactional version-1 CHESSFRT.RPL export/import
- 600-frame replay-file ring round-trip and full CPU/local terminal replay certification
- 32 native 320x200 visual-review states covering CPU/local HUDs, Fart, promotion and terminal states
- 320x200 256-color VGA presentation
- Sound Blaster / PC-speaker audio architecture
- versioned save/load and audio config
- session history, help, credits and attract demo
- optional DOS mouse support
- strict C89 Build 4–11 regression stack
- host CPU/memory profiling and release-source audit
- real 16-bit DOS `CHESSFRT.EXE` built with Open Watcom 2.0
- DOSBox platform smoke covering VGA, mouse probe and audio initialization
- verified `chessfart-1.0.0-dos.zip` with SHA-256

The final release does **not** claim exhaustive physical-hardware compatibility. Physical 386/486 systems, MS-DOS/FreeDOS variants, real Sound Blaster models and third-party DOS mouse drivers remain documented manual/community targets.

See [`docs/BUILD_12.md`](docs/BUILD_12.md), [`docs/COMPATIBILITY.md`](docs/COMPATIBILITY.md), and [`docs/RELEASE_CHECKLIST.md`](docs/RELEASE_CHECKLIST.md).

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
U in Practice    Undo latest committed move or Fart
S / L in Practice Disabled
S / L in Replay  Export / import CHESSFRT.RPL
Mouse in Replay  Previous / Next / Export / Import / Close footer controls
Esc              Quit; cancel pending Fart/promotion choice
```

With a compatible DOS mouse driver, left-click targets board/menu items and right-click enters Fart mode.

## Verification

Host release gate:

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
- [`docs/REPLAY_FORMAT.md`](docs/REPLAY_FORMAT.md)

**CHESS FART** — *Check. Mate. Ventilate.*
