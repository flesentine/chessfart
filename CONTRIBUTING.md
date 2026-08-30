# Contributing to Chess Fart

## Project rule #1

A funny feature is not worth corrupting chess state.

## Build discipline

Work in small vertical milestones matching `docs/ROADMAP.md`.

Each build should leave the repository in a runnable/testable state.

## Code organization

- pure rules in `src/core`
- AI in `src/ai`
- UI/game flow in `src/game`
- DOS-specific hardware code in `src/platform/dos`
- shared declarations in `include`
- host tests in `tests`
- asset converters in `tools`

Do not call VGA/audio/input APIs from core rules code.

## C style

- target C89/C90 unless there is an explicit decision to change
- favor small functions
- explicit integer/state transitions over clever macros
- braces even for short control blocks where ambiguity is possible
- no hidden heap allocation in frequently called move-generation paths
- comments explain *why*, not obvious syntax

## Rules changes

Any gameplay rule change involving Gas, fart displacement, kings, draws, castling or promotion must update:

1. `docs/GAME_DESIGN.md`
2. `docs/DECISIONS.md`
3. relevant tests

Do not change only the code.

## Art rules

- native-size readability first
- no anti-aliased runtime sprites
- remain within indexed palette constraints
- source assets and converters should be reproducible

## Commit guidance

Prefer focused commits such as:

- `Build 1: add Mode 13h framebuffer`
- `Core: implement knight move generation`
- `Rules: add legal fart displacement`
- `Art: add white piece sprite set`

Avoid commits that mix engine changes, unrelated art and docs unless they form one vertical feature.

## Testing requirement

Before a core rules change is considered complete:

- host tests pass
- apply/unapply round-trip is preserved
- no action can leave acting side's king illegally in check
- new edge cases are represented by regression tests

## Humor guideline

Keep the joke playful and game-like. The strongest tone is serious chess presentation interrupted by absurd gas mechanics, not constant random gross-out text.