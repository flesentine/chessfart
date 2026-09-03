import importlib.util
import os
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MODULE_PATH = os.path.join(ROOT, "tools", "build_assets.py")
SPEC = importlib.util.spec_from_file_location("build_assets", MODULE_PATH)
build_assets = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(build_assets)


class Build13AssetTests(unittest.TestCase):
    def test_current_assets_have_expected_budget(self):
        generated, sheets = build_assets.build_assets()
        self.assertEqual(len(generated), 17)
        packed = sum(len(build_assets._pack_pixels(packing, pixels))
                     for _name, _w, _h, packing, pixels in generated)
        self.assertEqual(packed, 1696)
        self.assertEqual(len(sheets), 2)

    def test_generation_is_deterministic(self):
        generated, _sheets = build_assets.build_assets()
        self.assertEqual(build_assets.render_include(generated),
                         build_assets.render_include(generated))

    def test_empty_png_is_rejected(self):
        handle, path = tempfile.mkstemp(suffix=".png")
        os.close(handle)
        try:
            with self.assertRaises(build_assets.AssetError):
                build_assets.read_indexed_png(path)
        finally:
            os.unlink(path)

    def test_non_png_is_rejected(self):
        handle, path = tempfile.mkstemp(suffix=".png")
        try:
            os.write(handle, b"not a png")
        finally:
            os.close(handle)
        try:
            with self.assertRaises(build_assets.AssetError):
                build_assets.read_indexed_png(path)
        finally:
            os.unlink(path)


if __name__ == "__main__":
    unittest.main()
