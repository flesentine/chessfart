# Changelog

All notable project milestones will be recorded here.

## Unreleased

### Build 8 — audio

Added:

- platform-neutral audio event/config API
- AUTO / SB / PCSPK / NONE device selection
- OFF / LOW / MED / HIGH SFX levels
- five deterministic procedural 8-bit PCM fart voices
- generated cursor/select/invalid/move/capture/check/checkmate/promotion/menu effects
- DOS Sound Blaster DSP reset/detection and direct-DAC playback path
- BLASTER environment base-address parsing with 0x220 default
- PIT/channel-2 PC-speaker fallback patterns
- audio status and device/volume controls on the title screen
- in-game resolved-device/SFX readout
- audio wrappers around move, capture, Fart, promotion and game-status events
- host audio event log
- host-generated fart WAV artifact
- Build 8 audio regression suite
- `docs/BUILD_8.md`

Architecture:

- Build 8 embeds the existing Build 6 loop and Build 7 renderer instead of copying rules
- presentation exports its existing fart animation so validated Fart actions can trigger audio before visual playback
- final sample assets remain replaceable behind the same event API
- Build 9 can persist audio configuration without putting it into game-position identity

Verification:

- strict C89 audio sources compile with warnings-as-errors
- procedural sample-bank tests pass
- device/volume/mute tests pass
- permanent Build 4–6 suites remain required by CI
- deterministic host run must produce Build 8 title/game PPMs, audio log and WAV

### Build 7 — presentation pass

Added title/menu, Royal Basement palette, improved piece silhouettes, board polish, five-frame fart animation, palette flash, board shake and deterministic VGA previews.

### Build 6 — fart displacement

Added one-square displacement, blocked outcomes, full king-safety filtering, castling-right displacement effects, pushed-pawn promotion, check creation/resolution, Gas-aware history and reversible fart actions.

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

Build 8 audio complete in source/host validation. Build 9 — save/load and config — is next.
