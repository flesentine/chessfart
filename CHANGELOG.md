# Changelog

All notable project milestones will be recorded here.

## Unreleased

### Build 9 — save/load and config

Added:

- versioned `CHESSFRT.SAV`
- field-by-field portable save serialization
- exact board/Gas/counter/Gas-history restoration
- transactional load validation
- unsupported-version, corruption and truncation rejection
- `S` save and `L` load controls
- versioned `CHESSFRT.CFG`
- persisted Build 8 audio settings
- temp-file replacement writes
- Build 9 persistence regression suite
- scripted save → push → load → push host smoke test
- `docs/BUILD_9.md`
- `docs/SAVE_FORMAT.md`

### Build 8 — audio

Added platform-neutral sound events, original procedural 8-bit PCM, five fart voices, Sound Blaster source backend, PC-speaker fallback, audio options, host audio log/WAV and CI coverage.

### Build 7 — presentation pass

Added title/menu presentation, Royal Basement palette, improved pieces, board framing, coordinates, five-frame fart animation, palette flash and board shake.

### Build 6 — fart displacement

Added one-square displacement, blocked outcomes, full king-safety filtering, displaced castling rights, pushed-pawn promotion, check creation/resolution, Gas-aware history and reversible fart actions.

### Build 5 — Gas system

Added per-piece Gas, earning/spending, Fart mode, eight-direction targeting, PUFF turns and Gas-aware repetition.

### Build 4 — complete standard chess

Added castling, en passant, four-way promotion, checkmate/stalemate, draw state, repetition, counters and complete special-state make/unmake.

### Build 3 — legal chess movement

Added ordinary legal movement, attack/check detection, king-safety filtering, captures and perft validation.

### Build 2 — cursor and pieces

Added the 32-piece starting board, VGA pieces, keyboard cursor and selection state.

### Build 1 — VGA boot

Added the Mode 13h shell, backbuffer, palette, font and input abstraction.

### Build 0 — foundation

Added the design, architecture, rules and milestone documentation.

### Current phase

Build 9 persistence complete in source/host validation. Build 10 — CPU opponent — is next.
