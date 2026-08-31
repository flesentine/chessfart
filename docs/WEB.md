# Chess Fart — Browser / WebAssembly Edition

The browser edition compiles the same Build 12 C game to WebAssembly with Emscripten. It is a platform port, not a JavaScript rewrite.

## What is shared with DOS

- chess legality and special moves
- Gas/Fart rules and displacement
- CPU opponent and difficulty levels
- VGA renderer and 256-color palette
- presentation/Fart animation
- procedural sound generation
- save/config file formats
- Build 11 help, history, credits and terminal UI

## Web platform backends

- VGA Mode 13h framebuffer -> HTML Canvas
- keyboard -> browser key queue
- DOS mouse -> Pointer Events
- Sound Blaster PCM -> Web Audio PCM buffers
- PC speaker -> Web Audio square oscillator
- `CHESSFRT.SAV` / `CHESSFRT.CFG` -> Emscripten IDBFS backed by IndexedDB

The original 320x200 framebuffer is scaled with nearest-neighbor/pixelated rendering.

## Build

Install Emscripten, then run:

```sh
make -f Makefile.web web
```

Output:

```text
build/web/index.html
build/web/index.js
build/web/index.wasm
```

Serve `build/web/` through HTTP; browsers should not load the WASM bundle directly from `file://`.

## Controls

Desktop keyboard controls are the same as DOS. The page also provides touch buttons for movement, confirmation, Fart, save/load, help, history and CPU difficulty.

Mouse/pointer input works directly on the VGA screen. Left-click selects/targets and right-click enters Fart mode.

## Persistence

The web build mounts `/persist` using Emscripten IDBFS. The existing text save/config formats are written there and synchronized to IndexedDB, so Save/Load survives page refreshes in the same browser profile.

## Runtime model

The DOS game uses polling loops. The web input backend calls `emscripten_sleep()` while no input is queued and the build enables Asyncify, yielding back to the browser event loop instead of busy-spinning or freezing the page.
