import importlib.util
import os
import sys
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOLS = os.path.join(ROOT, "tools")

BUILD_ASSETS_PATH = os.path.join(TOOLS, "build_assets.py")
SPEC_ASSETS = importlib.util.spec_from_file_location("build_assets", BUILD_ASSETS_PATH)
build_assets = importlib.util.module_from_spec(SPEC_ASSETS)
SPEC_ASSETS.loader.exec_module(build_assets)
sys.modules["build_assets"] = build_assets

BUILD_FONT_PATH = os.path.join(TOOLS, "build_font.py")
SPEC_FONT = importlib.util.spec_from_file_location("build_font", BUILD_FONT_PATH)
build_font = importlib.util.module_from_spec(SPEC_FONT)
SPEC_FONT.loader.exec_module(build_font)


class Build13FontTests(unittest.TestCase):
    def test_font_atlas_budget_and_shape(self):
        packed, sha256 = build_font.build_font()
        self.assertEqual(len(packed), 512)
        self.assertEqual(len(sha256), 64)
        self.assertEqual(build_font.ATLAS_W, 128)
        self.assertEqual(build_font.ATLAS_H, 32)
        self.assertEqual(build_font.GLYPH_COUNT, 64)

    def test_font_generation_is_deterministic(self):
        packed, _sha256 = build_font.build_font()
        first = build_font.render_include(packed)
        second = build_font.render_include(packed)
        self.assertEqual(first, second)
        self.assertIn(b"CF_FONT_GLYPH_COUNT 64", first)
        self.assertIn(b"CF_FONT_RUNTIME_BYTES 512U", first)

    def test_required_ui_punctuation_is_present(self):
        png = build_assets.read_indexed_png(build_font.SOURCE_PATH)
        for ch in (">", "?", "+", "=", "/", "-", ":"):
            glyph = ord(ch) - build_font.FIRST_CHAR
            cell_x = (glyph & 15) * build_font.CELL_W
            cell_y = (glyph >> 4) * build_font.CELL_H
            lit = 0
            for y in range(build_font.CELL_H):
                row = png["rows"][cell_y + y]
                for x in range(build_font.CELL_W):
                    if row[cell_x + x] != 0:
                        lit += 1
            self.assertGreater(lit, 0, ch)


if __name__ == "__main__":
    unittest.main()
