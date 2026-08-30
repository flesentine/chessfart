# Chess Fart — Build 7: Presentation Pass

## Status

**Complete in source when the Build 4–6 regressions and Build 7 host smoke target pass.**

Build 7 deliberately adds no chess or Fart rules. It wraps the completed Build 6 game with the visual language expected from a polished early-1990s 320x200 VGA shareware title.

## Delivered

### Title and menu

The game now opens on a dedicated title screen with a large pixel logo, green gas clouds, the `Check. Mate. Ventilate.` tagline, shareware-era checker strip, and a keyboard menu for `Play Fart Chess` or `Quit to DOS`.

The host smoke build auto-enters the game so automated testing remains deterministic.

### Final-ish VGA palette

Build 7 overrides the earlier engineering palette with the `Royal Basement` direction from the art spec:

- warm parchment / bronze light squares
- deep green-black dark squares
- navy/charcoal UI panels
- ivory White pieces
- blue-black Black pieces
- gold/copper framing
- acid-green Gas and fart effects
- dedicated capture/check/promotion accents

The frame-2 fart impact temporarily brightens the gold/Gas entries for a palette-flash effect.

### Improved pieces

The placeholder letter-heavy pieces are redrawn after the Build 6 board pass with more recognizable 16-pixel silhouettes for pawn, knight, bishop, rook, queen and king. The redraw keeps Gas pips, legal-move markers, capture markers and selection state readable.

### Board polish

Build 7 adds a copper board bevel, rank/file coordinates, shadowed header typography and a quieter presentation footer while retaining the compact 160x160 board and right-side rules panel.

### Fart animation

`presentation_make_fart()` wraps the unchanged Build 6 `gas_make_fart()` result. A successful action plays a five-frame deterministic animation:

1. initial gas burst
2. expanding plume
3. bright impact / palette flash
4. post-displacement frame
5. settling frame

The animation adds plume particles, destination emphasis and a one-pixel alternating board-frame shake. It never changes the rule result; it renders snapshots from immediately before and after the already-validated Build 6 action.

### Architecture

Build 7 does not fork the game loop. `src/main_build7.c` embeds the Build 6 main loop with two presentation substitutions:

- Build 6 renderer calls are routed to `board_view_render_build7()`
- Build 6 fart execution calls are routed through `presentation_make_fart()`

The actual board and Gas engines remain the Build 6 implementations. This keeps rules and presentation sharply separated.

## Verification

Run:

```sh
make test-build7
```

This target:

- compiles the Build 7 host shell with strict C89 warnings-as-errors
- runs the permanent Build 4 standard-chess regression/perft suite
- runs the permanent Build 5 Gas suite
- runs the permanent Build 6 displacement suite
- executes the deterministic Build 7 title/game/push smoke path
- verifies `build/host/chessfart_build7.ppm` exists

The host demo still uses the charged White knight on C3 and Black pawn on D4. The automated sequence enters Fart mode and pushes D4 to E5, now with the Build 7 presentation wrapper around the same Build 6 rule result.

GitHub Actions also uploads the final PPM as the `chessfart-build7-preview` artifact.

## DOS build

```bat
wmake -f makefile.build7.dos dos
```

or:

```bat
scripts\build_dos_build7.bat
```

Expected executable: `build\dos\CHESSFRT.EXE`.

Open Watcom/DOSBox remains the platform integration check in this environment.

## Build 7 exit criterion

**Met when CI is green:** Chess Fart has a coherent title/menu, upgraded VGA palette and pieces, readable board presentation, and a visible multi-frame fart/push effect without changing Build 6 legality or state results.

## Build 8 handoff

Build 8 is audio: Sound Blaster initialization, 8-bit PCM fart samples, move/check/UI effects, PC-speaker fallback and audio settings. Presentation timing should become audio-aware without changing Build 7's deterministic visual sequence.
