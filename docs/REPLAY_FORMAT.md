# Chess Fart Replay File Format

Replay files are separate from game saves. They use the default filename `CHESSFRT.RPL` and do not change or replace `CHESSFRT.SAV` version 2.

## Version 1

The file is UTF-8/ASCII text and begins with:

```text
CHESSFART_REPLAY 1
META <count> <total> <truncated>
```

- `count` is the number of retained frames, from 1 through 256.
- `total` is the absolute number of frames recorded before ring-buffer truncation.
- `truncated` is 0 or 1.
- An untruncated file requires `total == count`.
- A truncated file requires `count == 256` and `total > count`.

Each retained frame is written in logical oldest-to-newest order:

```text
FRAME <zero-based-index>
STATE <side> <castling> <ep-file> <ep-rank> <half-low> <half-high> <full-low> <full-high> <status> <mode>
SQUARES <64 packed-byte values>
LABEL <byte-length> <hex-data-or-dash>
END_FRAME
```

The file ends with:

```text
END
```

No trailing tokens are accepted.

## Snapshot encoding

Each square is the same one-byte representation used by the in-memory Build 15 replay timeline:

- bits 0-2: piece type
- bits 3-4: piece color
- bits 5-6: Gas
- bit 7: reserved and must be zero

Board metadata, game status, and match mode are validated by the existing replay snapshot decoder before an imported frame is accepted.

Labels are hex encoded so spaces and punctuation cannot change token parsing. Labels remain limited to 39 bytes plus the terminating NUL.

## Transactional import

Replay import parses into a temporary timeline. The caller-provided timeline is not changed unless the entire file passes:

- magic/version validation
- metadata/ring consistency
- every frame index and field
- all 64 packed squares per frame
- label length and hex validation
- snapshot restoration validation
- final `END`
- trailing-token rejection

Malformed, truncated, unsupported, or extra-data files therefore cannot partially replace an existing replay timeline.

## In-game behavior

Inside the Replay viewer:

- `S` exports the active live-session replay to `CHESSFRT.RPL`.
- `L` imports `CHESSFRT.RPL` into a temporary heap timeline for viewing.
- Imported replay data never replaces the active game's live replay timeline.
- Closing Replay restores the original live timeline and frees the temporary imported copy.
- Mouse footer controls expose Previous, Next, Export, Import, and Close.

Replay files contain snapshots only. They are not re-simulated through chess or Gas rules during playback.
