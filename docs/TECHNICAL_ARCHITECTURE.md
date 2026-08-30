# Chess Fart — Technical Architecture

## 1. Principle

Keep **game truth** separate from **presentation**.

The renderer may animate a rook flying one square, but the core engine should only care that an action transformed state A into state B legally.

## 2. Language and runtime

Primary target:

- C89/C90 subset
- Open Watcom C/C++
- 32-bit DOS protected-mode executable
- no heap allocation in hot gameplay paths unless proven necessary
- no floating point required for game logic

## 3. State model

Recommended types conceptually:

```c
typedef enum {
    PIECE_NONE,
    PIECE_PAWN,
    PIECE_KNIGHT,
    PIECE_BISHOP,
    PIECE_ROOK,
    PIECE_QUEEN,
    PIECE_KING
} PieceType;

typedef struct {
    unsigned char type;
    unsigned char color;
    unsigned char gas;
    unsigned char flags;
} Piece;

typedef struct {
    Piece squares[64];
    unsigned char side_to_move;
    unsigned char castling_rights;
    signed char en_passant_square;
    unsigned short halfmove_clock;
    unsigned short fullmove_number;
    unsigned long position_key;
} GameState;
```

Exact packing can change after profiling.

## 4. Action model

Treat normal moves and fart actions through one action pipeline.

```c
typedef enum {
    ACTION_MOVE,
    ACTION_FART
} ActionType;

typedef struct {
    unsigned char type;
    unsigned char from;
    unsigned char to_or_dir;
    unsigned char flags;
    unsigned char promotion;
} Action;
```

For `ACTION_FART`, `from` is acting piece square and `to_or_dir` is one of eight direction constants.

## 5. Legal action pipeline

1. Generate pseudo-legal normal moves.
2. Generate pseudo-legal fart actions for pieces with Gas >= 2.
3. Apply each candidate to a temporary/state-stack position.
4. Reject action if acting side's king is attacked afterward.
5. Return legal action list.

The same final king-safety filter should cover normal moves and fart moves. Avoid special-case legality code scattered through UI.

## 6. Apply/unapply

AI requires fast reversible state changes.

Create an `UndoState` containing only changed data:

- source and destination piece snapshots
- pushed square snapshots for fart
- captured piece if any
- prior Gas
- prior castling rights
- prior en-passant square
- prior clocks
- prior hash
- promotion details

`apply_action()` and `unapply_action()` must round-trip perfectly.

## 7. Attack detection

Implement `is_square_attacked(state, square, by_color)` independent of action generation. It is required for:

- check
- king moves
- castling
- king displacement by fart
- check/checkmate UI

## 8. Fart resolution algorithm

For a selected actor and direction:

```text
adjacent = actor + dir
destination = adjacent + dir

spend 2 gas
if adjacent off-board: no push
else if adjacent empty: no push
else if destination off-board: no push
else if destination occupied: no push
else move adjacent piece to destination
resolve promotion if pushed pawn reaches last rank
update castling rights if displaced king/rook leaves origin
final king-safety validation
```

Important: board-edge wrap must be prevented explicitly when translating 0..63 indexes.

## 9. Position hashing

Use Zobrist hashing for repetition and AI transposition.

Hash dimensions must include:

- piece type/color on each square
- Gas value 0–3 for occupied squares
- side to move
- castling rights
- en-passant file/square

Gas is part of the position identity.

## 10. Rendering

Mode 13h framebuffer is 64,000 bytes at segment A000h.

Use a 64,000-byte software backbuffer, then copy to VGA memory each frame or dirty-copy regions if necessary.

Renderer responsibilities:

- clear/draw background
- board tiles
- pieces
- selection overlays
- UI panels and bitmap text
- animation sprites
- final blit

The core engine must never write pixels.

## 11. Fixed timing

Turn logic is event-driven, but animation uses a fixed update rate.

Recommended:

- timer tick target around 60–70 Hz
- gameplay actions resolve logically once, then presentation interpolates via integer frame counters
- never tie rule timing to CPU speed

## 12. Input

Abstract input to actions such as:

- CURSOR_UP/DOWN/LEFT/RIGHT
- SELECT
- CANCEL
- FART_MODE
- MENU

DOS keyboard and mouse drivers populate the same input state.

Avoid blocking BIOS calls during main match loop if they break animation/audio timing.

## 13. AI

Start with a correct, modest engine.

### Search

- negamax
- alpha-beta pruning
- iterative deepening if timer system is stable
- transposition table
- quiescence for captures/checks later

### Action ordering

Prioritize:

1. checkmates/checks
2. captures
3. forcing fart pushes that give check or attack high-value pieces
4. promotions
5. killer/history moves
6. quiet moves

### Evaluation baseline

- material
- piece-square tables
- mobility
- king safety
- pawn structure
- Gas utility
- immediate displacement threats

Do not attempt sophisticated AI until rules tests are comprehensive.

## 14. Save format

Use a versioned binary save plus optional human-readable debug dump.

Header proposal:

```text
magic: CFSG
version: 1
payload_length
checksum
```

Payload must preserve:

- board pieces
- Gas per piece
- side
- castling rights
- en passant
- clocks
- repetition/history information required to continue legal draw detection
- match settings
- optional AI settings

Never serialize raw structs without explicit versioning; compiler padding can change.

## 15. Replay format

Post-v1 or late v1:

- initial rule config
- ordered Action list
- promotion choices

Because gameplay is deterministic, no RNG stream is needed for core moves.

## 16. Asset packing

Build-time converters should produce compact runtime data:

- palette: 768 bytes RGB6 values or target-specific equivalent
- sprite headers + indexed pixels
- font glyphs
- PCM samples
- optional packed resource file

Start with loose files during development. Add one resource archive only when the asset set stabilizes.

## 17. Error handling

At minimum:

- always restore text mode on normal exit
- register cleanup path for recoverable failures
- validate asset headers and sizes
- reject incompatible save versions cleanly
- display readable text error before exit where possible

## 18. Host-side tests

Compile `core/` as a normal host executable on modern systems in addition to DOS.

This enables fast regression tests for:

- FEN parsing/debug fixtures
- perft-like normal move counts
- fart edge cases
- apply/unapply round trips
- repetition hashing
- save serialization

The DOS build should consume the same core source files.