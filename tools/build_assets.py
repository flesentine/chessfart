#!/usr/bin/env python3
"""Build Chess Fart indexed UI assets from checked-in PNG source sheets.

Build 13.2 deliberately uses only the Python standard library so the asset
pipeline has no runtime image-library dependency in CI.
"""

from __future__ import print_function

import argparse
import hashlib
import json
import os
import struct
import sys
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MANIFEST_PATH = os.path.join(ROOT, "assets_src", "ui", "manifest.json")
GENERATED_PATH = os.path.join(ROOT, "src", "generated", "ui_assets_generated.inc")
REPORT_PATH = os.path.join(ROOT, "build", "assets", "ASSET_REPORT.txt")
PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


class AssetError(Exception):
    pass


def _manifest_int(value, label):
    if isinstance(value, bool) or not isinstance(value, int):
        raise AssetError("%s must be an integer" % label)
    return value


def _paeth(a, b, c):
    p = a + b - c
    pa = abs(p - a)
    pb = abs(p - b)
    pc = abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    if pb <= pc:
        return b
    return c


def read_indexed_png(path):
    with open(path, "rb") as handle:
        data = handle.read()
    if not data:
        raise AssetError("%s is empty" % path)
    if not data.startswith(PNG_SIGNATURE):
        raise AssetError("%s is not a PNG" % path)

    offset = len(PNG_SIGNATURE)
    width = height = bit_depth = color_type = interlace = None
    palette_entries = 0
    transparency = None
    idat = bytearray()
    saw_ihdr = False
    saw_plte = False
    saw_iend = False

    while offset + 12 <= len(data):
        length = struct.unpack(">I", data[offset:offset + 4])[0]
        kind = data[offset + 4:offset + 8]
        start = offset + 8
        end = start + length
        crc_end = end + 4
        if crc_end > len(data):
            raise AssetError("%s has a truncated PNG chunk" % path)
        payload = data[start:end]
        expected_crc = struct.unpack(">I", data[end:crc_end])[0]
        actual_crc = zlib.crc32(kind + payload) & 0xffffffff
        if expected_crc != actual_crc:
            raise AssetError("%s has a bad %s CRC" % (path, kind.decode("ascii", "replace")))

        if kind == b"IHDR":
            if saw_ihdr:
                raise AssetError("%s has duplicate IHDR" % path)
            if length != 13:
                raise AssetError("%s has an invalid IHDR" % path)
            width, height, bit_depth, color_type, compression, filter_method, interlace = \
                struct.unpack(">IIBBBBB", payload)
            if compression != 0 or filter_method != 0:
                raise AssetError("%s uses unsupported PNG compression/filter methods" % path)
            saw_ihdr = True
        elif kind == b"PLTE":
            if saw_plte:
                raise AssetError("%s has duplicate PLTE" % path)
            if length == 0 or length % 3 != 0 or length > 256 * 3:
                raise AssetError("%s has an invalid palette" % path)
            palette_entries = length // 3
            saw_plte = True
        elif kind == b"tRNS":
            transparency = bytes(payload)
        elif kind == b"IDAT":
            idat.extend(payload)
        elif kind == b"IEND":
            if length != 0:
                raise AssetError("%s has an invalid IEND" % path)
            saw_iend = True
            offset = crc_end
            break
        offset = crc_end

    if not saw_ihdr or width is None or height is None:
        raise AssetError("%s is missing IHDR" % path)
    if not saw_iend:
        raise AssetError("%s is missing IEND" % path)
    if width <= 0 or height <= 0:
        raise AssetError("%s has invalid dimensions" % path)
    if bit_depth != 8 or color_type != 3:
        raise AssetError("%s must be an 8-bit indexed PNG" % path)
    if interlace != 0:
        raise AssetError("%s must not be interlaced" % path)
    if palette_entries <= 0:
        raise AssetError("%s is missing PLTE" % path)
    if not idat:
        raise AssetError("%s is missing IDAT" % path)

    try:
        raw = zlib.decompress(bytes(idat))
    except zlib.error as exc:
        raise AssetError("%s has invalid compressed data: %s" % (path, exc))

    stride = width
    expected = height * (stride + 1)
    if len(raw) != expected:
        raise AssetError("%s decoded to %d bytes, expected %d" % (path, len(raw), expected))

    rows = []
    pos = 0
    prior = bytearray(stride)
    for _y in range(height):
        filter_type = raw[pos]
        pos += 1
        scan = bytearray(raw[pos:pos + stride])
        pos += stride
        recon = bytearray(stride)
        for x in range(stride):
            left = recon[x - 1] if x > 0 else 0
            up = prior[x]
            up_left = prior[x - 1] if x > 0 else 0
            value = scan[x]
            if filter_type == 0:
                decoded = value
            elif filter_type == 1:
                decoded = (value + left) & 255
            elif filter_type == 2:
                decoded = (value + up) & 255
            elif filter_type == 3:
                decoded = (value + ((left + up) // 2)) & 255
            elif filter_type == 4:
                decoded = (value + _paeth(left, up, up_left)) & 255
            else:
                raise AssetError("%s uses unsupported PNG filter %d" % (path, filter_type))
            recon[x] = decoded
        if any(index >= palette_entries for index in recon):
            raise AssetError("%s references a palette index outside PLTE" % path)
        rows.append(bytes(recon))
        prior = recon

    return {
        "width": width,
        "height": height,
        "rows": rows,
        "transparency": transparency,
        "sha256": hashlib.sha256(data).hexdigest(),
    }


def load_manifest():
    try:
        with open(MANIFEST_PATH, "r") as handle:
            manifest = json.load(handle)
    except (IOError, ValueError) as exc:
        raise AssetError("cannot read manifest: %s" % exc)
    if manifest.get("schema") != 1:
        raise AssetError("unsupported asset manifest schema")
    if manifest.get("transparent_index") != 0:
        raise AssetError("Build 13 assets require transparent palette index 0")
    sheets = manifest.get("sheets")
    if not isinstance(sheets, list) or not sheets:
        raise AssetError("manifest must contain at least one sheet")
    return manifest


def build_assets():
    manifest = load_manifest()
    generated = []
    report_sheets = []
    seen_names = set()

    for sheet_index, sheet in enumerate(manifest["sheets"]):
        if not isinstance(sheet, dict):
            raise AssetError("sheet %d must be an object" % sheet_index)
        rel_path = sheet.get("path")
        if not isinstance(rel_path, str) or not rel_path:
            raise AssetError("sheet path is missing")
        path = os.path.join(ROOT, rel_path)
        png = read_indexed_png(path)
        expected_width = _manifest_int(sheet.get("width"), "%s width" % rel_path)
        expected_height = _manifest_int(sheet.get("height"), "%s height" % rel_path)
        if expected_width <= 0 or expected_height <= 0:
            raise AssetError("%s manifest dimensions must be positive" % rel_path)
        if png["width"] != expected_width or png["height"] != expected_height:
            raise AssetError("%s dimensions are %dx%d, expected %dx%d" % (
                rel_path, png["width"], png["height"],
                expected_width, expected_height))
        transparency = png["transparency"]
        if transparency is None or len(transparency) < 1 or transparency[0] != 0:
            raise AssetError("%s must make palette index 0 transparent" % rel_path)

        allowed = sheet.get("allowed_indices")
        if not isinstance(allowed, list) or not allowed:
            raise AssetError("%s has no allowed_indices" % rel_path)
        allowed_set = set()
        for allowed_index, value in enumerate(allowed):
            parsed = _manifest_int(value, "%s allowed_indices[%d]" % (rel_path, allowed_index))
            if parsed < 0 or parsed > 255:
                raise AssetError("%s allowed index %d is outside 0..255" % (rel_path, parsed))
            allowed_set.add(parsed)
        if 0 not in allowed_set:
            raise AssetError("%s must allow transparent index 0" % rel_path)

        packing = sheet.get("packing")
        if packing not in ("nibble", "bit"):
            raise AssetError("%s has unsupported packing %r" % (rel_path, packing))
        if packing == "nibble" and any(value > 15 for value in allowed_set):
            raise AssetError("%s nibble packing only supports indices 0..15" % rel_path)
        if packing == "bit" and not allowed_set.issubset({0, 1}):
            raise AssetError("%s bit packing only supports indices 0 and 1" % rel_path)
        sprites = sheet.get("sprites")
        if not isinstance(sprites, list) or not sprites:
            raise AssetError("%s defines no sprites" % rel_path)

        for sprite_index, sprite in enumerate(sprites):
            if not isinstance(sprite, dict):
                raise AssetError("%s sprite %d must be an object" % (rel_path, sprite_index))
            name = sprite.get("name")
            if not isinstance(name, str) or not name or not name.replace("_", "").isalnum():
                raise AssetError("%s contains an invalid sprite name" % rel_path)
            if name in seen_names:
                raise AssetError("duplicate sprite name %s" % name)
            seen_names.add(name)

            x = _manifest_int(sprite.get("x"), "%s %s x" % (rel_path, name))
            y = _manifest_int(sprite.get("y"), "%s %s y" % (rel_path, name))
            w = _manifest_int(sprite.get("w"), "%s %s w" % (rel_path, name))
            h = _manifest_int(sprite.get("h"), "%s %s h" % (rel_path, name))
            if w <= 0 or h <= 0:
                raise AssetError("%s has empty sprite %s" % (rel_path, name))
            if x < 0 or y < 0 or x + w > png["width"] or y + h > png["height"]:
                raise AssetError("%s sprite %s falls outside its sheet" % (rel_path, name))

            pixels = []
            for row in png["rows"][y:y + h]:
                pixels.extend(row[x:x + w])
            unexpected = sorted(set(pixels) - allowed_set)
            if unexpected:
                raise AssetError("%s sprite %s uses disallowed indices %s" % (
                    rel_path, name, unexpected))
            generated.append((name, w, h, packing, pixels))

        report_sheets.append((rel_path, png["width"], png["height"], png["sha256"]))

    expected_count = _manifest_int(manifest.get("expected_sprite_count"), "expected_sprite_count")
    if expected_count <= 0:
        raise AssetError("expected_sprite_count must be positive")
    if len(generated) != expected_count:
        raise AssetError("generated %d sprites, expected %d" % (len(generated), expected_count))

    return generated, report_sheets


def _pack_pixels(packing, pixels):
    packed = []
    if packing == "nibble":
        if any(value < 0 or value > 15 for value in pixels):
            raise AssetError("nibble-packed sprite contains a value outside 0..15")
        if len(pixels) % 2 != 0:
            raise AssetError("nibble-packed sprite has an odd pixel count")
        for index in range(0, len(pixels), 2):
            packed.append((pixels[index] << 4) | pixels[index + 1])
    elif packing == "bit":
        if any(value not in (0, 1) for value in pixels):
            raise AssetError("bit-packed sprite contains a value other than 0 or 1")
        if len(pixels) % 8 != 0:
            raise AssetError("bit-packed sprite pixel count is not divisible by 8")
        for index in range(0, len(pixels), 8):
            value = 0
            for bit in range(8):
                if pixels[index + bit]:
                    value |= 1 << (7 - bit)
            packed.append(value)
    else:
        raise AssetError("unsupported packing %r" % packing)
    return packed


def render_include(generated):
    packed_assets = []
    for name, w, h, packing, pixels in generated:
        packed_assets.append((name, w, h, packing, _pack_pixels(packing, pixels)))
    total_pixels = sum(w * h for _name, w, h, _packing, _packed in packed_assets)
    total_bytes = sum(len(packed) for _name, _w, _h, _packing, packed in packed_assets)
    lines = [
        "/* Auto-generated by tools/build_assets.py. DO NOT EDIT. */",
        "/* Indexed PNG semantic classes packed for 16-bit DOS runtime use. */",
        "",
        "#define CF_UI_GENERATED_ASSET_COUNT %d" % len(generated),
        "#define CF_UI_GENERATED_PIXEL_COUNT %dU" % total_pixels,
        "#define CF_UI_GENERATED_ASSET_BYTES %dU" % total_bytes,
        "",
    ]
    for name, w, h, packing, packed in packed_assets:
        lines.append("/* %s: %dx%d, %s packed */" % (name, w, h, packing))
        lines.append("static const cf_u8 %s[%d] = {" % (name, len(packed)))
        for row in range(0, len(packed), 16):
            values = packed[row:row + 16]
            lines.append("    " + ", ".join("0x%02X" % value for value in values) + ",")
        lines.append("};")
        lines.append("")
    return ("\n".join(lines) + "\n").encode("ascii")


def write_report(generated, sheets):
    total_pixels = sum(len(pixels) for _name, _w, _h, _packing, pixels in generated)
    total_bytes = sum(len(_pack_pixels(packing, pixels)) for _name, _w, _h, packing, pixels in generated)
    lines = [
        "Chess Fart Build 13 asset report",
        "sprites=%d" % len(generated),
        "semantic_pixels=%d" % total_pixels,
        "packed_runtime_bytes=%d" % total_bytes,
        "soft_gameplay_art_budget_bytes=16384",
        "budget_remaining_bytes=%d" % (16384 - total_bytes),
    ]
    for rel_path, width, height, sha256 in sheets:
        lines.append("source=%s %dx%d sha256=%s" % (rel_path, width, height, sha256))
    directory = os.path.dirname(REPORT_PATH)
    if not os.path.isdir(directory):
        os.makedirs(directory)
    with open(REPORT_PATH, "w") as handle:
        handle.write("\n".join(lines) + "\n")


def main():
    parser = argparse.ArgumentParser()
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true", help="write generated C asset include")
    mode.add_argument("--check", action="store_true", help="verify generated include is current")
    args = parser.parse_args()

    try:
        generated, sheets = build_assets()
        rendered = render_include(generated)
        if args.write:
            directory = os.path.dirname(GENERATED_PATH)
            if not os.path.isdir(directory):
                os.makedirs(directory)
            with open(GENERATED_PATH, "wb") as handle:
                handle.write(rendered)
        else:
            try:
                with open(GENERATED_PATH, "rb") as handle:
                    current = handle.read()
            except IOError:
                raise AssetError("generated asset include is missing; run --write")
            if current != rendered:
                raise AssetError("generated asset include is stale; run tools/build_assets.py --write")
        write_report(generated, sheets)
    except AssetError as exc:
        print("ASSET ERROR: %s" % exc, file=sys.stderr)
        return 1

    print("Build 13 assets OK: %d sprites, %d packed runtime bytes" % (
        len(generated), sum(len(_pack_pixels(packing, pixels)) for _name, _w, _h, packing, pixels in generated)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
