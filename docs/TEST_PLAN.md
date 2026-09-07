# Chess Fart — Test Plan

## 1. Testing rule

The comedy can be loose. Board state cannot.

Every committed move or Fart must be deterministic, legal, reversible where required, and reproducible across the maintained host/DOS/web paths.

## 2. Automated gates

### Host

`make test` compiles with strict C89 warnings-as-errors and runs:

- standard chess/perft regression
- Gas accounting and repetition
- Fart displacement and king-safety cases
- audio generation/hooks
- save/config validation and legacy compatibility
- CPU legality/search/pruning regressions
- UX/history formatting and hit testing
- replay timeline and replay-file corruption/ring tests
- Practice Undo
- theme policy
- scripted host runtime artifacts
- CPU/memory/size profile
- release-source audit

### DOS

`sh scripts/build_dos_ci.sh` produces real Open Watcom 16-bit DOS executables. CI then runs `DOSSMOKE.EXE` in DOSBox and requires the VGA/mouse/audio smoke proof.

### Web / Chromium

The WebAssembly gate builds the same game sources and runs:

- browser runtime smoke
- real keyboard/mouse gameplay
- CPU/local/Practice mode behavior
- save/load and legacy-save behavior
- replay viewer/file lifecycle
- long Practice ring stress
- full 33-ply local Opera Game through checkmate
- deterministic EASY/MED/HARD CPU games
- 36 native 320x200 visual states with pinned final-theme signatures

## 3. Standard chess regression

The permanent rules suite covers opening and reference-position perft, starting-state contracts, attack/check APIs, pinned-piece king safety, castling, en passant including self-check, all promotion choices, checkmate, stalemate, fifty-move draw, insufficient material, threefold repetition, and apply/unapply restoration.

## 4. Gas and Fart regression

Required invariants include:

- Gas starts at 0, caps at 3, and earns/spends exact amounts.
- All eight directions obey board edges with no wrap.
- PUFF/PUSH/BLOCKED/promotion outcomes are deterministic.
- Pushed-piece Gas remains attached to that piece.
- Fart actions cannot leave the acting king illegally in check.
- Displaced kings/rooks update castling rights permanently.
- Pushed pawns promote correctly.
- Fart actions round-trip through unmake/replay/Practice Undo where applicable.

## 5. Persistence regression

Game-save v2, replay-file v1, and config v2 are compatibility contracts.

Tests cover valid round-trip, legacy versions, malformed/truncated/unsupported input, transactional failure, match-mode semantics, theme migration/fallback, repetition history, and exact LOCAL rollback in Chromium.

## 6. CPU regression

CPU-selected actions must be legal and deterministic under the frozen difficulty policy. Tactical fixtures cover Fart check escape, forcing check, castling-right damage, pushed promotion, and the corrected move-dependent alpha-beta edge-bonus window. Full Chromium games at EASY/MED/HARD must terminate without browser errors.

## 7. Presentation/input regression

The native visual suite covers title, normal HUD, Fart mode, Help, History, Credits, save/load notices, local play, promotions, terminal states, Practice-related presentation, and both certified themes.

Input tests verify keyboard completeness, mouse targeting, modal locking, promotion locking, title hit boundaries, terminal locks, replay controls, and Practice Undo.

## 8. Manual/community compatibility

Automation does not claim exhaustive physical DOS compatibility. Manual/community targets remain physical 386/486 hardware, MS-DOS/FreeDOS variants, real Sound Blaster models, third-party DOS mouse drivers, DOSBox-X, and DOSBox Staging.

See `docs/COMPATIBILITY.md` for the exact verified/unverified matrix.
