# Chess Fart — CPU Design

## Goal

The CPU must play the same game the human plays. A Fart Action is a first-class search action, not a post-processing trick layered on ordinary chess.

## Search

Build 10 uses deterministic iterative-deepening negamax with alpha-beta pruning.

Each completed iteration becomes the fallback result for the next one. Search stops when it reaches any configured limit:

- depth ceiling
- node budget
- wall/CPU clock budget

If a deeper iteration is interrupted, the best action from the last completely searched depth is retained. If even depth 1 is interrupted, deterministic action ordering supplies a legal fallback.

## Difficulty

| Level | Depth | Node budget | Time cap |
|---|---:|---:|---:|
| EASY | 1 | 800 | 100 ms |
| MED | 2 | 8,000 | 500 ms |
| HARD | 3 | 50,000 | 1,500 ms |

The limits are deliberately conservative for the DOS target and can be tuned after DOSBox/real-hardware profiling.

## Action generation

At every node the CPU generates:

- all legal standard chess moves
- all four legal ordinary promotions
- every legal Fart direction for every charged piece
- all four pushed-pawn promotion choices
- PUFF, PUSH and BLOCKED Fart outcomes when legal

All actions use the same `gas_make_*` / `gas_unmake_*` functions as human play.

## Ordering

Search ordering is deterministic. High-priority actions include:

- captures, especially high-value victims
- promotions
- Fart promotions
- productive enemy pushes
- castling

PUFF and BLOCKED actions remain searchable but are ordered behind immediately productive actions unless deeper search proves them useful.

## Evaluation

The Build 10 evaluation combines:

- material
- Gas carried by pieces
- bonus for Fart-ready pieces
- approximate Fart pressure on adjacent pieces
- centralization
- pawn advancement
- check pressure
- remaining castling rights

Mate/stalemate are resolved by actual legal action generation, not by evaluation heuristics.

## Draw handling

The root position uses the real Gas-aware history and therefore respects an already-earned threefold draw. Search also respects fifty-move and insufficient-material states. Build 10 does not extend the real repetition-history array down speculative search branches; doing that without large DOS stack copies is reserved for a later search optimization if testing shows it matters.

## Player model

Build 10 is human White versus CPU Black. The CPU moves immediately after a successful White move/Fart. Save/load remains exact; if a loaded save is Black to move, the CPU immediately consumes that turn and records the history transition correctly.

## Future tuning

Possible later work:

- transposition table
- killer/history ordering
- quiescence search
- opening book
- endgame tables
- AI personalities
- smarter Fart-specific positional terms
