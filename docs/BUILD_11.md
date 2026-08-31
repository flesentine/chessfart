# Chess Fart — Build 11: UX / Polish

## Status

**Complete; Build 4–11 CI is green.**

Build 11 is a presentation/input milestone. It does not change chess legality, Gas/Fart rules, CPU search semantics, save files, repetition identity, or audio configuration.

## What shipped

- 32-entry session action log with human and CPU moves/Fart actions
- in-game action-log overlay (`Tab` or `M`)
- two-page rules/help overlay (`H` or `?`)
- credits screen (`C`)
- polished terminal-state overlay for checkmate/stalemate/draws
- five-item title menu: Play / Attract Demo / Help / Credits / Quit
- title-screen idle attract demo
- scripted Fart PUSH attract sequence
- Space as an Enter/confirm synonym
- optional DOS mouse support through INT 33h
- left-click board targeting and right-click Fart-mode entry
- clickable title menu when a DOS mouse driver is present
- Build 11 VGA status/footer cleanup
- dedicated host previews for title, game, demo, help, history, credits and terminal state

## Session action history

The UX history is deliberately not game truth. It records the current session's human and CPU actions and system save/load events, but it is not stored in `CHESSFRT.SAV` and it is not included in repetition identity.

Loading a game clears the display log and begins a fresh session log with `SYSTEM LOAD OK`. The actual rules-position repetition history remains restored exactly by Build 9 persistence.

## Help and controls

In game:

```text
Arrows / keypad diagonals   Cursor / Fart aim
Enter or Space              Select / confirm
F                           Enter/cancel Fart mode
S                           Save
L                           Load
H or ?                      Help/rules
Tab or M                    Action log
C                           Credits
Esc                         Quit
```

Title screen:

```text
Up/Down                     Menu
Enter/Space                 Choose
Left/Right                  Audio device
F                           SFX level
D                           CPU difficulty
H                           Help
C                           Credits
Mouse left                  Choose menu item (if driver present)
```

## Mouse support

Build 11 adds optional Microsoft-compatible DOS mouse-driver support using interrupt `33h`.

When present:

- the title menu is clickable
- left-clicking a board square moves the software cursor to that square and confirms it
- right-click enters Fart mode
- while aiming a Fart, clicking an adjacent square selects that direction and confirms it

The host backend intentionally reports no mouse so automated tests remain deterministic.

## Attract loop

The title screen exposes `ATTRACT DEMO` directly and automatically runs the same demo after approximately 12 seconds without keyboard/mouse activity.

The demo uses the real rules engine: a charged White knight on C3 Farts northeast and pushes a Black pawn from D4 to E5, including the existing Build 7 animation.

## Terminal-state presentation

Checkmate, stalemate and draw states now receive a centered modal-style VGA overlay rather than being represented only by the side-panel message. The overlay includes the winner/draw label and quick Help/Log/Quit hints.

## Host verification

Run:

```sh
make test-build11
```

The target runs every permanent Build 4, 5, 6, 8, 9 and 10 suite plus Build 11 UX tests and the scripted host game.

Build 11 tests verify:

- action-log ring behavior
- board mouse hit testing and rank inversion
- terminal labels/winner selection
- deterministic attract Fart PUSH state

The host run additionally preserves dedicated title/game/demo/help/history/credits/terminal previews, Build 11 audio artifacts, and the existing save/config files.

## DOS build

```bat
wmake -f makefile.build11.dos dos
```

or:

```bat
scripts\build_dos_build11.bat
```

Expected executable: `build\dos\CHESSFRT.EXE`.

Open Watcom/DOSBox remains the platform integration check, especially real mouse-driver behavior, title idle timing and CPU speed.

## Exit criterion

A new player can discover the controls/rules from inside the game, review recent actions, understand terminal states, watch an attract demo, use a keyboard consistently, and optionally navigate with a DOS mouse without changing the underlying game state semantics.

## Build 12 handoff

Build 12 is the release candidate: DOS compatibility matrix, performance profiling, bug burn-down, packaging, DOSBox configuration, final version stamp and release ZIP.
