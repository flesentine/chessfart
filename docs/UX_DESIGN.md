# Chess Fart — UX Design Notes

Build 11 treats UX as a shell around stable game truth.

## Principles

1. No UX state belongs in repetition identity.
2. No UX state can alter move/Fart legality.
3. Modal screens must restore the exact board view when closed.
4. Keyboard remains the primary DOS input path.
5. Mouse support is additive and optional.
6. All important controls must be discoverable in-game.

## Modal surfaces

- Help: two compact pages covering controls and special Fart rules.
- Action Log: latest 12 of a 32-entry session ring.
- Credits: game identity/toolchain/presentation card.
- Terminal overlay: winner/draw state rendered over the final board.

The last normal game render is cached so Help/Log/Credits can redraw the underlying game immediately after closing without changing cursor or selection state.

## Title flow

The Build 11 title menu is:

1. Play CPU
2. Attract Demo
3. Help / Rules
4. Credits
5. Quit to DOS

Audio and CPU difficulty controls remain available from the title screen.

## Mouse mapping

DOS INT 33h is used only when a compatible driver reports present.

- Left board click: synthesize cursor steps until the clicked square is reached, then synthesize Enter.
- Right click: synthesize F.
- Fart mode + adjacent left click: synthesize the corresponding 8-way aim key, then Enter.

This keeps mouse input on the same keyboard-facing game path rather than adding a second rules path.
