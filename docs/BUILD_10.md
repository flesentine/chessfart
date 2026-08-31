# Chess Fart — Build 10: CPU Opponent

## Status

**Complete when Build 4–10 CI is green.**

Build 10 adds a deterministic CPU opponent without changing any chess, Gas/Fart, presentation, audio, save, or config rule from Builds 4–9.

## What shipped

- human White versus CPU Black
- iterative-deepening negamax
- alpha-beta pruning
- deterministic action ordering
- Easy / Medium / Hard difficulty
- hard node and time budgets
- material/Gas/Fart-aware evaluation
- CPU search over normal moves and every legal Fart Action
- ordinary and pushed-pawn promotion choices in search
- checkmate/stalemate recognition inside search
- automatic CPU turn after a human action
- automatic CPU turn after loading a Black-to-move save
- CPU search statistics on the VGA side panel

## Difficulty control

On the title screen:

```text
D     Cycle EASY / MED / HARD
```

The chosen level is runtime-only in Build 10. The existing `CHESSFRT.CFG` remains Build 9's audio configuration format and is not silently version-changed.

## Search safety

The CPU is bounded by both node count and clock time. Iterative deepening means an interrupted deeper search never destroys the best result from a completed shallower search.

Default limits:

- EASY: depth 1, 800 nodes, 100 ms
- MED: depth 2, 8,000 nodes, 500 ms
- HARD: depth 3, 50,000 nodes, 1,500 ms

See `docs/CPU_DESIGN.md` for evaluation and ordering details.

## History correctness

When White moves, the CPU wrapper preserves the real intermediate Black-to-move position in Gas-aware repetition history before making the CPU action. Build 9 then records the resulting White-to-move position exactly once.

Loading a Black-to-move save performs the same two history transitions itself because Build 9's load path does not append a move history entry.

## Host verification

Run:

```sh
make test-build10
```

The target runs the permanent Build 4, 5, 6, 8 and 9 suites, the Build 10 CPU suite, and the scripted save/load/fart-push host game with a CPU reply after each White action.

Build 10 tests verify:

- difficulty profiles and cycling
- deterministic starting-position choice
- search leaves the inspected position unchanged
- mate-in-one selection
- Fart PUSH generation
- Fart apply/unmake restoration
- hard node-budget termination

Host artifacts remain available as Build 10 VGA, audio, save and config files.

## DOS build

```bat
wmake -f makefile.build10.dos dos
```

or:

```bat
scripts\build_dos_build10.bat
```

Expected executable: `build\dos\CHESSFRT.EXE`.

Open Watcom/DOSBox remains the platform integration and performance check.

## Exit criterion

A human can play White against a deterministic Black CPU that evaluates both chess and Fart actions, respects a bounded search budget, survives save/load, and leaves all earlier regression suites green.

## Build 11 handoff

Build 11 is UX/polish: move/action history, help/rules, stronger terminal-state screens, attract/demo loop, credits, keyboard completeness and optional mouse support.
