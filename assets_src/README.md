# Build 13 UI source assets

These PNGs are the editable source of truth for the Chess Fart Build 13 gameplay
asset pipeline. Production code never decodes PNGs at runtime.

## Files

- `ui/pieces.png` — 96x32 indexed PNG, twelve 16x16 piece cells.
  - row 0: white king, queen, rook, bishop, knight, pawn
  - row 1: black king, queen, rook, bishop, knight, pawn
- `ui/puffs.png` — 80x16 indexed PNG, five 16x16 fart-puff frames.
- `ui/manifest.json` — dimensions, slices, allowed semantic palette indices.

## Semantic authoring palette

The PNG palette indices are semantic classes, not final VGA colors:

- `0` transparent
- `1` keyline / shadow / puff mask
- `2` dark shade
- `3` body
- `4` highlight

`ui_assets.c` maps piece classes to the canonical VGA theme at runtime. The
converter nibble-packs piece classes and bit-packs puff masks, so the current
4,352 source pixels occupy only 1,696 bytes of compiled runtime art data. This
lets later art PRs change shape/detail without baking DOS palette numbers into
the source artwork.

## Generate / verify

```sh
python3 tools/build_assets.py --write
python3 tools/build_assets.py --check
python3 -m unittest tests/test_build13_assets.py
```

The checked-in `src/generated/ui_assets_generated.inc` is deterministic build
output. Edit the PNG/manifest sources, never the generated include directly.

Build 13.2 intentionally reproduces the pre-13.2 piece and puff pixels exactly;
the actual art refresh starts in later Build 13 PRs.
