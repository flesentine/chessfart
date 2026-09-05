# Chess Fart Save Format

Current game-save format version: **2**  
Current config format version: **1**

Files are human-readable text to avoid binary struct-layout dependence.

## Game save grammar

```text
CHESSFART_SAVE 2
MODE <match_mode>
STATE <side> <castling> <ep_file> <ep_rank> <halfmove> <fullmove>
SQUARES
<type> <color> <gas>
... 64 square records ...
HISTORY <count>
KEY <state_byte> <effective_ep_file>
<64 encoded Gas-position bytes>
... repeated count times ...
END
```

\`match_mode\` is \"0\" for CPU play and \"1\" for local two-player play.

Piece types/colors use the engine enum numeric values. Gas is 0–3.

Each history square byte is the same Gas-aware position encoding used by the engine: piece code in the low nibble and Gas in bits 4–5. `effective_ep_file` is 0–7 or 8 for none.

## Legacy version 1 compatibility

Version-1 game saves do not contain a `MODE` record:

```text
CHESSFART_SAVE 1
STATE <side> <castling> <ep_file> <ep_rank> <halfmove> <fullmove>
...
END
```

The version-2 loader continues to accept these files and restores them as CPU mode, matching the only shipped match mode that existed when version 1 was written. A successful load is transactional: board, Gas, repetition history, and match mode are committed only after the entire file validates.

## Config grammar

```text
CHESSFART_CONFIG 1
AUDIO <device> <sfx_level> <music_level>
END
```

Future incompatible changes must increment the corresponding version number rather than silently reinterpreting old files.
