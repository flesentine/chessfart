# Chess Fart — UX Design

## Principles

1. UX state never participates in chess/Gas repetition identity.
2. UI input cannot bypass normal move/Fart legality.
3. Modal screens and replay review do not replace the live game state.
4. Keyboard remains the complete primary DOS input path.
5. Mouse support is optional and synthesizes the same game-facing actions.
6. Save, replay, Practice, match mode, and theme behavior remain explicit and testable.

## Title flow

The current title menu is:

1. Play CPU
2. 2 Players
3. Practice
4. Attract Demo
5. Help / Rules
6. Credits
7. Quit to DOS

Title controls also expose audio device, SFX level, CPU difficulty, and the Royal Basement / Crimson Cellar theme selector. Keyboard and mouse title selection share the canonical coordinates in `include/ui_layout.h`.

## Match HUD

The board, right-side HUD, and bottom command bar share fixed 320x200 layout constants.

- CPU mode shows CPU difficulty.
- Local two-player shows `LOCAL 2P`.
- Practice shows `PRACTICE` and exposes Undo while disabling game Save/Load.
- Fart mode replaces the normal detail panel with source, Gas, direction, preview, and confirmation information.
- Terminal states overlay the final board instead of replacing it.

The latest rendered game state is cached for UI redraws only. That cache never becomes rules authority.

## Modal surfaces

- Help: two compact pages for controls and Fart rules.
- Action Log: a bounded 32-entry session ring.
- Credits: project/toolchain presentation.
- Replay: bounded read-only timeline viewer with keyboard/mouse stepping and replay-file export/import.
- Terminal overlay: checkmate or draw result over the final live board.

Replay review restores snapshots into dedicated scratch state. Closing replay returns to the untouched live match.

## Mouse mapping

DOS INT 33h is used only when a compatible driver reports present.

- Left board click: synthesize cursor steps to the target, then Select/Confirm.
- Right board click: target the square and enter Fart mode.
- Fart mode + adjacent left click: synthesize the matching eight-way direction and confirm.
- Practice command-bar Undo: synthesize the same Undo action as the keyboard.
- Replay footer buttons: synthesize viewer navigation/export/import/close behavior.

This keeps pointer input on the same state machine as keyboard input rather than creating a second rules path.

## Persistence and session boundaries

- Game saves persist CPU/local match mode but not the transient replay timeline.
- Practice is a session policy and is not written as a new save-mode value.
- Successful Load starts a fresh replay baseline at the loaded state.
- Config persistence owns audio settings and the selected presentation theme.
