# Chess Fart — Roadmap

Milestones are numbered as builds so future work can proceed `build 0`, `build 1`, etc.

## Build 0 — Repository foundation
**Status: complete**

## Build 1 — VGA boot
**Status: complete in source; host smoke test passes**

## Build 2 — Cursor and pieces
**Status: complete in source; host tests pass**

## Build 3 — Normal chess movement
**Status: complete in source; host rules tests pass**

## Build 4 — Complete standard chess
**Status: complete in source; host rules tests pass**

Delivered castling, en passant, promotion, terminal states, draw bookkeeping, reversible special moves and multi-position perft regression coverage.

## Build 5 — Gas system
**Status: complete in source; host gas/rules tests pass**

Delivered per-piece Gas, earning/spending, Gas-aware repetition, eight-direction Fart mode and PUFF actions.

## Build 6 — Fart displacement
**Status: complete in source; CI passes**

Delivered one-square push, blocked outcomes, king safety, displaced castling rights, pushed promotion, check creation/resolution and reversible Fart actions.

## Build 7 — Presentation pass
**Status: complete in source; CI passes**

Delivered title/menu, Royal Basement palette, improved pieces, board coordinates/bevel, five-frame fart plume/push animation, palette flash and board shake.

## Build 8 — Audio
**Status: complete in source/host validation; CI target added**

Delivered:

- platform-neutral audio event/config layer
- five original procedural 8-bit PCM fart voices
- UI/move/capture/check/checkmate/promotion SFX
- Sound Blaster DSP initialization and short PCM playback source
- PC-speaker fallback
- device and SFX-level title-screen controls
- host event log and WAV artifact
- unchanged Build 4–7 gameplay/presentation regression baseline

Exit criterion: game actions have deterministic non-gameplay audio cues, sound can be disabled, and the host path proves event/sample output without changing rules.

Remaining integration check: Open Watcom + DOSBox/real Sound Blaster-compatible validation.

## Build 9 — Save/load and config
**Status: next**

Deliverables: config file, save UI, versioned save data, exact Gas/history restoration, audio-setting persistence, error handling.

## Build 10 — CPU opponent
Deliverables: negamax/alpha-beta, evaluation, action ordering, Gas/fart awareness, difficulty settings, move-time cap.

## Build 11 — UX/polish
Deliverables: move history, help/rules, polished promotion/checkmate presentation, attract/demo loop, credits, keyboard completeness, mouse support.

## Build 12 — Release candidate
Deliverables: compatibility matrix, profiling, bug burn-down, packaged DOS ZIP, sample DOSBox config, final README, version stamp.

## Post-v1 ideas

Alternate boards/pieces, replay browser, Turbo Gas, puzzle challenges, AI personalities, null-modem multiplayer, OPL2 expansion, and a shareware-style episode screen are post-v1 only.
