# Chess Fart

**Chess Fart** is a deliberately ridiculous 256-color VGA strategy game: real chess rebuilt as a loud early-1990s DOS game where pieces build Gas and use directional fart blasts as tactical abilities.

The design goal is **real strategy first, toilet humor second**. Standard chess is complete, Gas is live, and Build 6 puts the actual displacement mechanic online.

## Project status

**Build 6 — Fart Displacement: complete in source.**

Chess Fart now supports the full core variant loop:

- complete standard chess, including castling, en passant, promotion and terminal states
- Gas 0–3 on every piece
- +1 Gas on a normal move, +2 total on capture
- Fart Action costs 2 Gas and consumes the turn
- eight-direction fart targeting
- empty-square `PUFF`
- one-square push of friendly or enemy pieces
- blocked pushes that still spend Gas/turn when confirmed
- full acting-king safety validation after displacement
- king displacement and check creation
- castling-right loss when a king/rook is displaced from home
- en-passant expiration on every Fart Action
- pushed-pawn promotion with owner choice
- Gas-aware repetition identity
- reversible move and Fart Action state for future AI/search

See [`docs/BUILD_6.md`](docs/BUILD_6.md) for the exact implementation and edge-case rules.

## Controls — Build 6

```text
Arrow keys        Move cursor / aim cardinal Fart direction
Enter             Select / move / confirm Fart Action
F                 Enter or cancel Fart mode
Keypad diagonals  Aim NE / SE / SW / NW
Esc               Quit
```

A selected piece needs at least **2 Gas** to enter Fart mode. The preview identifies `PUFF`, `PUSH`, `BLOCKED`, `PROMOTE`, or `INVALID` before confirmation.

## Build

### Host verification

```sh
make test-build6
```

This runs the permanent Build 4 chess suite, Build 5 Gas suite, Build 6 displacement suite, and a scripted host smoke scene that pushes a pawn from D4 to E5. The framebuffer is written to:

```text
build/host/chessfart_build6.ppm
```

### DOS / Open Watcom

```bat
wmake -f makefile.build6.dos dos
```

or:

```bat
scripts\build_dos_build6.bat
```

Expected output: `build\dos\CHESSFRT.EXE`.

## Target experience

- 320x200, 256-color VGA / Mode 13h presentation
- DOS-era chunky sprites, indexed palettes and oversized UI
- real chess remains strategically relevant
- deterministic Gas/Fart displacement layered on top
- keyboard-first controls
- Sound Blaster-style sampled effects with PC speaker fallback later
- DOSBox as the primary runtime target

## Documentation

- [`docs/MASTER_PLAN.md`](docs/MASTER_PLAN.md) — full project blueprint
- [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md) — chess + Gas/Fart rules
- [`docs/VGA_ART_STYLE.md`](docs/VGA_ART_STYLE.md) — VGA art direction
- [`docs/TECHNICAL_ARCHITECTURE.md`](docs/TECHNICAL_ARCHITECTURE.md) — engine architecture
- [`docs/ROADMAP.md`](docs/ROADMAP.md) — milestone roadmap
- [`docs/TEST_PLAN.md`](docs/TEST_PLAN.md) — test strategy
- [`docs/BUILD_4.md`](docs/BUILD_4.md) — complete standard chess
- [`docs/BUILD_5.md`](docs/BUILD_5.md) — Gas/PUFF system
- [`docs/BUILD_6.md`](docs/BUILD_6.md) — full Fart displacement

## Design pillars

1. **Still chess.** Normal chess knowledge matters.
2. **No hidden randomness.** Gas state and fart outcomes are predictable.
3. **Fast to read.** The 320x200 screen must communicate state instantly.
4. **Juicy VGA presentation.** Palette flashes, screen shake, gas clouds and crude digitized audio are part of the identity.
5. **Small enough to finish.** One polished board and one strong ruleset beat oversized scope.

## Working title

**CHESS FART**

Tagline: **Check. Mate. Ventilate.**
