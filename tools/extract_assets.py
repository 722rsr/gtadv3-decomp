#!/usr/bin/env python3
"""Comprehensive end-to-end asset extraction suite for GT Advance 3 (GBA).

Extracts and exports all game assets into structured directories under assets/:
  assets/
    tracks/         63 course headers (bin+json), 63 surface maps (bin+png),
                    theme graphics/palettes, surface variants, scenery/car gfx
    lz77/           All 331 LZ77 blobs (compressed .lz, decompressed .bin, preview .png)
    mto/            All 209 valid MTO directory entries (.bin + manifest)
    containers/     Nested MTO container packages unpacked
    sound/          Song directory, bank descriptors, speeds, pitch tables, PCM wave samples (.wav+.pcm)
    palettes/       Extracted GBA color palettes (.gpl + swatch .png)
    manifest.json   Master asset manifest with stats and inventory

Dependencies: Python 3 standard library + PIL (Pillow) for PNG export.
"""

import argparse
import json
import os
import struct
import sys
import wave
from pathlib import Path
from typing import Dict, List, Optional, Tuple

try:
    from PIL import Image
    HAS_PIL = True
except ImportError:
    HAS_PIL = False

# Import local tools
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from lz77 import decompress  # noqa: E402
from track_dump import read_package, DIR_OFF as TRACK_DIR_OFF, NGROUPS  # noqa: E402
from mto_dump import DIR_OFF as MTO_DIR_OFF, N_ANNOUNCED as MTO_N_ANNOUNCED, classify as mto_classify  # noqa: E402


# ---------------------------------------------------------------------------
# Helper functions: GBA Color & Tile Decoders
# ---------------------------------------------------------------------------

def gba_bgr555_to_rgb888(val: int) -> Tuple[int, int, int]:
    """Convert GBA 15-bit BGR (RGB555) to 24-bit RGB."""
    r = ((val >> 0) & 0x1F) << 3 | (((val >> 0) & 0x1F) >> 2)
    g = ((val >> 5) & 0x1F) << 3 | (((val >> 5) & 0x1F) >> 2)
    b = ((val >> 10) & 0x1F) << 3 | (((val >> 10) & 0x1F) >> 2)
    return (r, g, b)


def parse_gba_palette(data: bytes, offset: int = 0, count: int = 16) -> List[Tuple[int, int, int]]:
    """Parse a run of 16-bit GBA colors into RGB tuples."""
    colors = []
    for k in range(count):
        if offset + (k + 1) * 2 > len(data):
            break
        val = struct.unpack_from("<H", data, offset + k * 2)[0]
        colors.append(gba_bgr555_to_rgb888(val))
    return colors


def export_palette_gpl(colors: List[Tuple[int, int, int]], out_path: Path, name: str = "GBA Palette"):
    """Export palette in GIMP Palette (.gpl) format."""
    lines = [
        "GIMP Palette",
        f"Name: {name}",
        "Columns: 16",
        "#",
    ]
    for r, g, b in colors:
        lines.append(f"{r:3d} {g:3d} {b:3d}")
    out_path.write_text("\n".join(lines) + "\n")


def export_palette_png(colors: List[Tuple[int, int, int]], out_path: Path, swatch_size: int = 16):
    """Render a palette swatch image as PNG."""
    if not HAS_PIL or not colors:
        return
    n = len(colors)
    cols = min(16, n)
    rows = (n + cols - 1) // cols
    img = Image.new("RGB", (cols * swatch_size, rows * swatch_size), color=(30, 30, 30))
    pixels = img.load()
    for idx, (r, g, b) in enumerate(colors):
        cx = (idx % cols) * swatch_size
        cy = (idx // cols) * swatch_size
        for y in range(swatch_size):
            for x in range(swatch_size):
                if x == 0 or y == 0 or x == swatch_size - 1 or y == swatch_size - 1:
                    pixels[cx + x, cy + y] = (0, 0, 0)
                else:
                    pixels[cx + x, cy + y] = (r, g, b)
    img.save(out_path)


def decode_4bpp_tiles(tile_data: bytes, width_in_tiles: int = 16,
                      palette_rgb: Optional[List[Tuple[int, int, int]]] = None) -> Optional["Image.Image"]:
    """Decode raw GBA 4bpp tile data into a PIL Image."""
    if not HAS_PIL:
        return None
    num_tiles = len(tile_data) // 32
    if num_tiles == 0:
        return None

    # Calculate dimensions
    actual_width_tiles = min(num_tiles, width_in_tiles)
    height_in_tiles = (num_tiles + actual_width_tiles - 1) // actual_width_tiles

    img = Image.new("RGB", (actual_width_tiles * 8, height_in_tiles * 8), color=(0, 0, 0))
    pixels = img.load()

    # Default 16-color grayscale palette if none provided
    if not palette_rgb or len(palette_rgb) < 16:
        palette_rgb = [(int(i * 255 / 15), int(i * 255 / 15), int(i * 255 / 15)) for i in range(16)]

    for tile_idx in range(num_tiles):
        tile_x = (tile_idx % actual_width_tiles) * 8
        tile_y = (tile_idx // actual_width_tiles) * 8
        tile_bytes = tile_data[tile_idx * 32 : (tile_idx + 1) * 32]

        for y in range(8):
            row_bytes = tile_bytes[y * 4 : (y + 1) * 4]
            for x in range(4):
                b = row_bytes[x]
                p0 = b & 0x0F
                p1 = (b >> 4) & 0x0F
                pixels[tile_x + x * 2, tile_y + y] = palette_rgb[p0]
                pixels[tile_x + x * 2 + 1, tile_y + y] = palette_rgb[p1]

    return img


def decode_8bpp_tiles(tile_data: bytes, width_in_tiles: int = 16,
                      palette_rgb: Optional[List[Tuple[int, int, int]]] = None) -> Optional["Image.Image"]:
    """Decode raw GBA 8bpp tile data into a PIL Image."""
    if not HAS_PIL:
        return None
    num_tiles = len(tile_data) // 64
    if num_tiles == 0:
        return None
    actual_width_tiles = min(num_tiles, width_in_tiles)
    height_in_tiles = (num_tiles + actual_width_tiles - 1) // actual_width_tiles

    img = Image.new("RGB", (actual_width_tiles * 8, height_in_tiles * 8), color=(0, 0, 0))
    pixels = img.load()

    if not palette_rgb or len(palette_rgb) < 256:
        palette_rgb = [(i, i, i) for i in range(256)]

    for tile_idx in range(num_tiles):
        tile_x = (tile_idx % actual_width_tiles) * 8
        tile_y = (tile_idx // actual_width_tiles) * 8
        tile_bytes = tile_data[tile_idx * 64 : (tile_idx + 1) * 64]

        for y in range(8):
            row_bytes = tile_bytes[y * 8 : (y + 1) * 8]
            for x in range(8):
                idx = row_bytes[x]
                pixels[tile_x + x, tile_y + y] = palette_rgb[idx] if idx < len(palette_rgb) else (255, 0, 255)

    return img


def surface_byte_to_rgb(b: int) -> Tuple[int, int, int]:
    """Map a course surface map byte to an RGB color representation."""
    if b == 0x00:
        return (25, 25, 28)           # Void / outer wall
    elif 0x01 <= b <= 0x0F:
        if b in (0x06, 0x0C):
            return (225, 55, 55)      # Curbs / rumble strips
        return (35, 130 + (b * 6), 55) # Grass / run-off variants
    elif 0x10 <= b <= 0x1F:
        return (185, 160, 105)        # Dirt / gravel / sand
    elif 0x20 <= b <= 0x2F:
        if b == 0x26:
            return (85, 85, 90)       # Primary asphalt circuit road
        return (115, 115, 120)        # Road variant / markings
    elif b == 0x31:
        return (245, 245, 245)        # Barrier / pit wall
    else:
        # Other feature codes
        return (100 + (b * 5) % 150, 60 + (b * 3) % 150, 180)


def render_surface_map_png(surf_data: bytes, out_path: Path, width: int = 256, height: int = 256):
    """Render a 256x256 course surface grid to a color-coded PNG image."""
    if not HAS_PIL or len(surf_data) < width * height:
        return
    img = Image.new("RGB", (width, height))
    grid = [surface_byte_to_rgb(surf_data[y * width + x]) for y in range(height) for x in range(width)]
    img.putdata(grid)
    img.save(out_path)


def write_pcm_to_wav(pcm_s8_data: bytes, wav_path: Path, sample_rate: int = 13379):
    """Write 8-bit signed PCM audio data as a standard mono 8-bit WAV file."""
    # Convert signed 8-bit PCM [-128..127] to unsigned 8-bit PCM [0..255]
    u8_data = bytes((b + 128) & 0xFF for b in pcm_s8_data)
    with wave.open(str(wav_path), "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(1)
        wav.setframerate(sample_rate)
        wav.writeframes(u8_data)


# ---------------------------------------------------------------------------
# Section 1: Track / Course Package Extraction
# ---------------------------------------------------------------------------

def extract_track_package(rom: bytes, out_dir: Path) -> Dict:
    """Extract and decode the 249 records in the MTO track package (0x080CE020)."""
    track_dir = out_dir / "tracks"
    track_dir.mkdir(parents=True, exist_ok=True)

    courses_dir = track_dir / "courses"
    themes_dir = track_dir / "themes"
    variants_dir = track_dir / "variants"
    scenery_dir = track_dir / "scenery_car"

    for d in [courses_dir, themes_dir, variants_dir, scenery_dir]:
        d.mkdir(parents=True, exist_ok=True)

    counts, offsets, recs = read_package(rom)
    manifest = {
        "dir_offset": hex(TRACK_DIR_OFF),
        "vma": hex(TRACK_DIR_OFF + 0x08000000),
        "group_counts": counts,
        "total_records": len(recs),
        "groups": {}
    }

    # Store extracted palettes for tile rendering
    theme_palettes = {}
    variant_palettes = {}

    # Pass 1: Extract Palettes (Group 4, Group 6, Group 9)
    for off, g, i, size, pay in recs:
        payload = rom[pay : pay + size]
        if g == 4:  # Theme palettes (320 bytes = 160 colors = 10 16-color palettes)
            pal_colors = parse_gba_palette(payload, 0, len(payload) // 2)
            theme_palettes[i] = pal_colors
            pal_bin = themes_dir / f"theme_{i:02d}_palette.bin"
            pal_bin.write_bytes(payload)
            export_palette_gpl(pal_colors, themes_dir / f"theme_{i:02d}_palette.gpl", f"Theme {i:02d} Palette")
            export_palette_png(pal_colors, themes_dir / f"theme_{i:02d}_palette.png")
        elif g == 6:  # Variant palettes (32 bytes = 16 colors)
            pal_colors = parse_gba_palette(payload, 0, len(payload) // 2)
            variant_palettes[i] = pal_colors
            pal_bin = variants_dir / f"variant_{i:02d}_palette.bin"
            pal_bin.write_bytes(payload)
            export_palette_gpl(pal_colors, variants_dir / f"variant_{i:02d}_palette.gpl", f"Variant {i:02d} Palette")
            export_palette_png(pal_colors, variants_dir / f"variant_{i:02d}_palette.png")
        elif g == 9:  # Scenery/Car palettes (32-128 bytes)
            pal_colors = parse_gba_palette(payload, 0, len(payload) // 2)
            pal_bin = scenery_dir / f"pal_set_{i:02d}.bin"
            pal_bin.write_bytes(payload)
            if pal_colors:
                export_palette_gpl(pal_colors, scenery_dir / f"pal_set_{i:02d}.gpl", f"Scenery/Car Set {i:02d} Palette")
                export_palette_png(pal_colors, scenery_dir / f"pal_set_{i:02d}.png")

    # Pass 2: Extract all records
    for idx, (off, g, i, size, pay) in enumerate(recs):
        payload = rom[pay : pay + size]
        is_lz77 = (payload[0] == 0x10 and len(payload) >= 4)
        dec_payload = None
        if is_lz77:
            try:
                dec_payload = decompress(payload, 0)
            except Exception:
                dec_payload = None

        rec_info = {
            "record_index": idx,
            "group": g,
            "index": i,
            "file_offset": hex(pay),
            "vma": hex(pay + 0x08000000),
            "size": size,
            "is_lz77": is_lz77,
            "decompressed_size": len(dec_payload) if dec_payload else None,
        }

        # Group 0: Geometry Headers (63 courses)
        if g == 0:
            bin_path = courses_dir / f"course_{i:02d}_header.bin"
            bin_path.write_bytes(payload)

            # Parse header fields
            hw = struct.unpack_from("<16H", payload, 0)
            na, nb, nc, nd, c5 = hw[1], hw[2], hw[3], hw[4], hw[5]
            parsed_len = 28 + na * 20 + nb * 12 + nc * 12 + nd * 8
            tail_len = size - 8 - parsed_len

            header_json = {
                "course_index": i,
                "magic": hex(hw[0]),
                "section_a_count": na,
                "section_b_count": nb,
                "section_c_count": nc,
                "section_d_count": nd,
                "tail_count": c5,
                "cup_ref_a": hw[6],
                "cup_ref_b": hw[7],
                "const_8192": hw[8],
                "const_32": hw[9],
                "map_width": hw[10],
                "map_height": hw[11],
                "const_20345": hw[12],
                "id_checksum": hw[13],
                "sections": {
                    "section_a_offset": 28,
                    "section_b_offset": 28 + na * 20,
                    "section_c_offset": 28 + na * 20 + nb * 12,
                    "section_d_offset": 28 + na * 20 + nb * 12 + nc * 12,
                    "tail_offset": parsed_len,
                    "tail_size_bytes": tail_len,
                },
                "files": {
                    "header_bin": str(bin_path.relative_to(out_dir)),
                }
            }
            json_path = courses_dir / f"course_{i:02d}_header.json"
            json_path.write_text(json.dumps(header_json, indent=2))
            rec_info["parsed"] = header_json

        # Group 1: Surface Maps (63 courses, 256x256 = 64KB uncompressed)
        elif g == 1:
            raw_data = dec_payload if dec_payload else payload
            bin_path = courses_dir / f"course_{i:02d}_surface.bin"
            bin_path.write_bytes(raw_data)

            png_path = courses_dir / f"course_{i:02d}_surface.png"
            render_surface_map_png(raw_data, png_path, 256, 256)
            rec_info["files"] = {
                "surface_bin": str(bin_path.relative_to(out_dir)),
                "surface_png": str(png_path.relative_to(out_dir)),
            }

        # Group 2: Theme Textures / Tilemaps (8 themes)
        elif g == 2:
            raw_data = dec_payload if dec_payload else payload
            bin_path = themes_dir / f"theme_{i:02d}_texture.bin"
            bin_path.write_bytes(raw_data)
            rec_info["files"] = {"texture_bin": str(bin_path.relative_to(out_dir))}

        # Group 3: Theme LUT A (8 themes)
        elif g == 3:
            bin_path = themes_dir / f"theme_{i:02d}_lut_a.bin"
            bin_path.write_bytes(payload)
            rec_info["files"] = {"lut_a_bin": str(bin_path.relative_to(out_dir))}

        # Group 5: Surface Variant Tiles (7 variants)
        elif g == 5:
            raw_data = dec_payload if dec_payload else payload
            bin_path = variants_dir / f"variant_{i:02d}_tiles.bin"
            bin_path.write_bytes(raw_data)

            # Preview image using corresponding variant palette
            pal = variant_palettes.get(i, None)
            img = decode_4bpp_tiles(raw_data, width_in_tiles=16, palette_rgb=pal)
            if img:
                png_path = variants_dir / f"variant_{i:02d}_tiles.png"
                img.save(png_path)
                rec_info["files"] = {
                    "tiles_bin": str(bin_path.relative_to(out_dir)),
                    "tiles_png": str(png_path.relative_to(out_dir)),
                }
            else:
                rec_info["files"] = {"tiles_bin": str(bin_path.relative_to(out_dir))}

        # Group 7: Surface Variant Overlays (7 variants)
        elif g == 7:
            raw_data = dec_payload if dec_payload else payload
            bin_path = variants_dir / f"variant_{i:02d}_overlay.bin"
            bin_path.write_bytes(raw_data)

            pal = variant_palettes.get(i, None)
            img = decode_4bpp_tiles(raw_data, width_in_tiles=16, palette_rgb=pal)
            if img:
                png_path = variants_dir / f"variant_{i:02d}_overlay.png"
                img.save(png_path)
                rec_info["files"] = {
                    "overlay_bin": str(bin_path.relative_to(out_dir)),
                    "overlay_png": str(png_path.relative_to(out_dir)),
                }
            else:
                rec_info["files"] = {"overlay_bin": str(bin_path.relative_to(out_dir))}

        # Group 8: Scenery / Car GFX sets (26 sets)
        elif g == 8:
            raw_data = dec_payload if dec_payload else payload
            bin_path = scenery_dir / f"gfx_set_{i:02d}.bin"
            bin_path.write_bytes(raw_data)

            img = decode_4bpp_tiles(raw_data, width_in_tiles=16)
            if img:
                png_path = scenery_dir / f"gfx_set_{i:02d}.png"
                img.save(png_path)
                rec_info["files"] = {
                    "gfx_bin": str(bin_path.relative_to(out_dir)),
                    "gfx_png": str(png_path.relative_to(out_dir)),
                }
            else:
                rec_info["files"] = {"gfx_bin": str(bin_path.relative_to(out_dir))}

        # Group 10: Scenery / Car Tilemaps (26 sets)
        elif g == 10:
            raw_data = dec_payload if dec_payload else payload
            bin_path = scenery_dir / f"tilemap_set_{i:02d}.bin"
            bin_path.write_bytes(raw_data)
            rec_info["files"] = {"tilemap_bin": str(bin_path.relative_to(out_dir))}

        grp_key = f"group_{g}"
        if grp_key not in manifest["groups"]:
            manifest["groups"][grp_key] = []
        manifest["groups"][grp_key].append(rec_info)

    (track_dir / "manifest.json").write_text(json.dumps(manifest, indent=2))
    print(f"  [tracks] Extracted {len(recs)} track records into {track_dir}")
    return manifest


# ---------------------------------------------------------------------------
# Section 2: All LZ77 Blobs Extraction
# ---------------------------------------------------------------------------

def extract_all_lz77_blobs(rom: bytes, out_dir: Path, scan_file: Optional[Path] = None) -> Dict:
    """Decompress and classify all 331 LZ77 blobs across the ROM."""
    lz_dir = out_dir / "lz77"
    lz_dir.mkdir(parents=True, exist_ok=True)

    blobs = []
    if scan_file and scan_file.exists():
        with open(scan_file) as f:
            for line in f:
                if line.startswith("#") or not line.strip():
                    continue
                parts = line.split()
                off = int(parts[0], 16)
                dec_size = int(parts[1], 16)
                blobs.append((off, dec_size))
    else:
        # Fallback inline scan
        for i in range(0, len(rom) - 4):
            if rom[i] != 0x10:
                continue
            size = int.from_bytes(rom[i + 1 : i + 4], "little")
            if 0x200 <= size <= 0x18000:
                try:
                    decompress(rom, i)
                    blobs.append((i, size))
                except Exception:
                    pass

    manifest = {
        "total_blobs": len(blobs),
        "blobs": []
    }

    tsv_lines = [
        "index\toffset\tvma\tcompressed_size\tdecompressed_size\ttype\toutput_bin\tpreview_png"
    ]

    for idx, (off, decl_size) in enumerate(blobs):
        # Calculate compressed size by decompressing
        try:
            dec_data = decompress(rom, off)
        except Exception as e:
            print(f"Warning: Failed to decompress blob {idx} @ 0x{off:06X}: {e}")
            continue

        # Replay compression stream to find exact compressed end
        j = off + 4
        produced = 0
        while produced < len(dec_data) and j < len(rom):
            flags = rom[j]
            j += 1
            for bit in range(8):
                if produced >= len(dec_data):
                    break
                if flags & (0x80 >> bit):
                    j += 2
                    length = ((rom[j - 2] >> 4) & 0xF) + 3
                    produced += length
                else:
                    j += 1
                    produced += 1
        comp_size = j - off
        comp_data = rom[off : off + comp_size]

        # Save raw compressed and uncompressed files
        base_name = f"blob_{idx:03d}_0x{off:06X}"
        bin_path = lz_dir / f"{base_name}.bin"
        lz_path = lz_dir / f"{base_name}.lz"
        bin_path.write_bytes(dec_data)
        lz_path.write_bytes(comp_data)

        # Classification & visual export
        blob_type = "DATA"
        png_path = None

        if len(dec_data) == 0x10000:  # 64KB = 256x256 surface grid
            blob_type = "SURFACE_GRID_256x256"
            png_file = lz_dir / f"{base_name}_surface.png"
            render_surface_map_png(dec_data, png_file, 256, 256)
            png_path = png_file
        elif len(dec_data) in (0x800, 0x1000, 0x2000, 0x4000):  # Screenblocks / Tilemaps
            blob_type = "TILEMAP"
        elif len(dec_data) % 32 == 0 and len(dec_data) >= 0x200:  # 4bpp tile graphics
            blob_type = "TILES_4BPP"
            img = decode_4bpp_tiles(dec_data, width_in_tiles=16)
            if img:
                png_file = lz_dir / f"{base_name}_tiles4bpp.png"
                img.save(png_file)
                png_path = png_file

        blob_info = {
            "index": idx,
            "file_offset": hex(off),
            "vma": hex(off + 0x08000000),
            "compressed_size": comp_size,
            "decompressed_size": len(dec_data),
            "declared_size": decl_size,
            "type": blob_type,
            "files": {
                "decompressed_bin": str(bin_path.relative_to(out_dir)),
                "compressed_lz": str(lz_path.relative_to(out_dir)),
            }
        }
        if png_path:
            blob_info["files"]["preview_png"] = str(png_path.relative_to(out_dir))

        manifest["blobs"].append(blob_info)
        png_rel = str(png_path.relative_to(out_dir)) if png_path else ""
        tsv_lines.append(f"{idx}\t{hex(off)}\t{hex(off+0x08000000)}\t{comp_size}\t{len(dec_data)}\t{blob_type}\t{str(bin_path.relative_to(out_dir))}\t{png_rel}")

    (lz_dir / "manifest.json").write_text(json.dumps(manifest, indent=2))
    (lz_dir / "manifest.tsv").write_text("\n".join(tsv_lines) + "\n")
    print(f"  [lz77] Extracted {len(manifest['blobs'])} LZ77 blobs into {lz_dir}")
    return manifest


# ---------------------------------------------------------------------------
# Section 3: MTO Directory Entries Extraction
# ---------------------------------------------------------------------------

def extract_mto_directory(rom: bytes, out_dir: Path) -> Dict:
    """Extract all 209 valid parameter/config entries from the MTO directory (0x08060378)."""
    mto_dir = out_dir / "mto"
    mto_dir.mkdir(parents=True, exist_ok=True)

    entries = []
    tsv_lines = ["index\toffset\tvma\tsize\tclass\toutput_bin"]
    manifest = {
        "dir_offset": hex(MTO_DIR_OFF),
        "vma": hex(MTO_DIR_OFF + 0x08000000),
        "stride": 12,
        "announced": MTO_N_ANNOUNCED,
        "entries": []
    }

    for i in range(MTO_N_ANNOUNCED):
        e = MTO_DIR_OFF + i * 12
        magic = rom[e : e + 4]
        off = int.from_bytes(rom[e + 4 : e + 8], "little")
        size = int.from_bytes(rom[e + 8 : e + 12], "little")

        if magic != b"MTO\x00" or off + size > len(rom):
            continue

        payload = rom[off : off + size]
        cls = mto_classify(payload)
        base_name = f"entry_{i:03d}_0x{off:06X}"
        bin_path = mto_dir / f"{base_name}.bin"
        bin_path.write_bytes(payload)

        entry_info = {
            "index": i,
            "file_offset": hex(off),
            "vma": hex(off + 0x08000000),
            "size": size,
            "classification": cls,
            "file": str(bin_path.relative_to(out_dir))
        }
        manifest["entries"].append(entry_info)
        tsv_lines.append(f"{i}\t{hex(off)}\t{hex(off+0x08000000)}\t{size}\t{cls}\t{str(bin_path.relative_to(out_dir))}")

    manifest["valid_entries"] = len(manifest["entries"])
    (mto_dir / "manifest.json").write_text(json.dumps(manifest, indent=2))
    (mto_dir / "manifest.tsv").write_text("\n".join(tsv_lines) + "\n")
    print(f"  [mto] Extracted {len(manifest['entries'])} MTO entries into {mto_dir}")
    return manifest


# ---------------------------------------------------------------------------
# Section 4: Nested Container Packages Extraction
# ---------------------------------------------------------------------------

def extract_nested_containers(rom: bytes, out_dir: Path) -> Dict:
    """Extract nested container packages found in the ROM."""
    cont_dir = out_dir / "containers"
    cont_dir.mkdir(parents=True, exist_ok=True)

    # Known container roots referenced by game tables
    container_bases = [
        ("ui_menu_set", 0x2C1268),
        ("dialog_set", 0x2C4228),
        ("garage_parts_set", 0x2F0FF0),
        ("records_set", 0x31F244),
        ("award_screen_set", 0x346910),
        ("car_model_set", 0x2A798C),
    ]

    manifest = {"containers": []}

    for name, base_off in container_bases:
        w0, w1, count = struct.unpack_from("<3I", rom, base_off)
        if count == 0 or count > 64:
            continue
        rel_offsets = [struct.unpack_from("<I", rom, base_off + 12 + 4 * k)[0] for k in range(count)]

        target_dir = cont_dir / f"{name}_0x{base_off:06X}"
        target_dir.mkdir(parents=True, exist_ok=True)

        cont_info = {
            "name": name,
            "base_offset": hex(base_off),
            "vma": hex(base_off + 0x08000000),
            "count": count,
            "records": []
        }

        for idx, rel_off in enumerate(rel_offsets):
            abs_off = base_off + rel_off
            next_off = rel_offsets[idx + 1] if idx + 1 < count else None
            sz = (next_off - rel_off) if next_off else 512  # approximate if last

            payload = rom[abs_off : abs_off + sz]
            bin_path = target_dir / f"rec_{idx:02d}_0x{abs_off:06X}.bin"
            bin_path.write_bytes(payload)

            cont_info["records"].append({
                "index": idx,
                "relative_offset": hex(rel_off),
                "absolute_offset": hex(abs_off),
                "vma": hex(abs_off + 0x08000000),
                "size": len(payload),
                "file": str(bin_path.relative_to(out_dir))
            })

        (target_dir / "manifest.json").write_text(json.dumps(cont_info, indent=2))
        manifest["containers"].append(cont_info)

    (cont_dir / "manifest.json").write_text(json.dumps(manifest, indent=2))
    print(f"  [containers] Unpacked {len(manifest['containers'])} container packages into {cont_dir}")
    return manifest


# ---------------------------------------------------------------------------
# Section 5: Sound Driver Tables & Audio PCM Samples
# ---------------------------------------------------------------------------

def extract_sound_assets(rom: bytes, out_dir: Path) -> Dict:
    """Extract sound driver tables, song directory, and PCM waveform samples."""
    snd_dir = out_dir / "sound"
    samples_dir = snd_dir / "samples"
    snd_dir.mkdir(parents=True, exist_ok=True)
    samples_dir.mkdir(parents=True, exist_ok=True)

    # 1. Song Directory @ 0x08061FA4 (file 0x61FA4)
    song_dir_off = 0x61FA4
    songs = []
    songs_tsv = ["song_id\tgate\tstruct_vma\tstruct_file\tpriority"]

    for i in range(128):
        ent_off = song_dir_off + i * 8
        if ent_off + 8 > len(rom):
            break
        ptr = int.from_bytes(rom[ent_off : ent_off + 4], "little")
        gate = int.from_bytes(rom[ent_off + 4 : ent_off + 6], "little")
        unused = int.from_bytes(rom[ent_off + 6 : ent_off + 8], "little")

        if ptr == 0 and gate == 0:
            continue

        file_ptr = ptr - 0x08000000 if 0x08000000 <= ptr < 0x08800000 else 0
        prio = rom[file_ptr + 2] if file_ptr > 0 and file_ptr + 3 <= len(rom) else 0

        song_info = {
            "song_id": i,
            "gate": gate,
            "struct_vma": hex(ptr),
            "struct_file": hex(file_ptr),
            "priority": prio,
        }
        songs.append(song_info)
        songs_tsv.append(f"{i}\t{gate}\t{hex(ptr)}\t{hex(file_ptr)}\t{prio}")

    (snd_dir / "songs.json").write_text(json.dumps({"songs": songs}, indent=2))
    (snd_dir / "songs.tsv").write_text("\n".join(songs_tsv) + "\n")

    # 2. Bank Descriptors @ 0x08061F74
    bank_table_off = 0x61F74
    banks = []
    for k in range(4):
        ent = bank_table_off + k * 12
        state_blk, ch_arr, n_ch = struct.unpack_from("<3I", rom, ent)
        banks.append({
            "bank_id": k,
            "state_block": hex(state_blk),
            "channel_array": hex(ch_arr),
            "num_channels": n_ch
        })
    (snd_dir / "banks.json").write_text(json.dumps({"banks": banks}, indent=2))

    # 3. Pitch / Period Tables @ 0x08061570 and 0x08061624
    (snd_dir / "pitch_tables.bin").write_bytes(rom[0x61570 : 0x61750])

    # 4. Extract PCM Audio Samples from ROM sample banks
    # Scan the 0x290000..0x3D0000 data region for continuous PCM waveforms
    sample_manifest = {"samples": []}
    sample_runs = []

    # Known sample offsets or heuristic waveform scan
    curr_start = None
    curr_len = 0
    in_sample = False

    # Scan 0x290000..0x3C0000 in 128-byte blocks
    for off in range(0x290000, 0x3C0000, 128):
        block = rom[off : off + 128]
        # Ignore constant fill sequences
        if all(b == block[0] for b in block) and block[0] in (0x00, 0x11, 0xEE, 0xFF, 0xAA):
            if in_sample and curr_len >= 1024:
                sample_runs.append((curr_start, curr_len))
            in_sample = False
            curr_start = None
            curr_len = 0
            continue

        # Check standard 8-bit PCM audio variance
        signed_avg = sum((b if b < 128 else b - 256) for b in block) / len(block)
        if -40.0 <= signed_avg <= 40.0:
            if not in_sample:
                in_sample = True
                curr_start = off
                curr_len = 128
            else:
                curr_len += 128
        else:
            if in_sample and curr_len >= 1024:
                sample_runs.append((curr_start, curr_len))
            in_sample = False
            curr_start = None
            curr_len = 0

    if in_sample and curr_len >= 1024:
        sample_runs.append((curr_start, curr_len))

    for s_idx, (s_off, s_len) in enumerate(sample_runs):
        pcm_data = rom[s_off : s_off + s_len]
        base_name = f"sample_{s_idx:02d}_0x{s_off:06X}"
        pcm_path = samples_dir / f"{base_name}.pcm"
        wav_path = samples_dir / f"{base_name}.wav"

        pcm_path.write_bytes(pcm_data)
        write_pcm_to_wav(pcm_data, wav_path, sample_rate=13379)

        sample_manifest["samples"].append({
            "sample_index": s_idx,
            "file_offset": hex(s_off),
            "vma": hex(s_off + 0x08000000),
            "size_bytes": s_len,
            "sample_rate": 13379,
            "files": {
                "pcm": str(pcm_path.relative_to(out_dir)),
                "wav": str(wav_path.relative_to(out_dir)),
            }
        })

    (samples_dir / "manifest.json").write_text(json.dumps(sample_manifest, indent=2))
    print(f"  [sound] Extracted {len(songs)} song entries and {len(sample_runs)} audio wave samples into {snd_dir}")
    return {"songs": songs, "banks": banks, "samples": sample_manifest}


# ---------------------------------------------------------------------------
# Section 6: Master Pipeline Orchestrator
# ---------------------------------------------------------------------------

def extract_all_assets(rom_path: Path, out_dir: Path, lz_scan_file: Optional[Path] = None):
    """Execute full end-to-end asset extraction."""
    print(f"=== Starting full asset extraction from {rom_path} ===")
    rom = rom_path.read_bytes()
    out_dir.mkdir(parents=True, exist_ok=True)

    print("\n1. Extracting Track / Course Package (0x080CE020)...")
    track_res = extract_track_package(rom, out_dir)

    print("\n2. Extracting & Decompressing All LZ77 Blobs (331 blobs)...")
    lz_res = extract_all_lz77_blobs(rom, out_dir, lz_scan_file)

    print("\n3. Extracting MTO Directory Entries (0x08060378)...")
    mto_res = extract_mto_directory(rom, out_dir)

    print("\n4. Extracting Nested Container Packages...")
    cont_res = extract_nested_containers(rom, out_dir)

    print("\n5. Extracting Sound Driver Tables & Audio PCM Samples...")
    snd_res = extract_sound_assets(rom, out_dir)

    # Master manifest
    master_manifest = {
        "rom": str(rom_path.name),
        "rom_size": len(rom),
        "summary": {
            "track_records_extracted": track_res.get("total_records", 0),
            "lz77_blobs_extracted": lz_res.get("total_blobs", 0),
            "mto_entries_extracted": mto_res.get("valid_entries", 0),
            "container_packages_unpacked": len(cont_res.get("containers", [])),
            "songs_cataloged": len(snd_res.get("songs", [])),
            "audio_samples_extracted": len(snd_res.get("samples", {}).get("samples", [])),
        },
        "directories": {
            "tracks": "assets/tracks",
            "lz77": "assets/lz77",
            "mto": "assets/mto",
            "containers": "assets/containers",
            "sound": "assets/sound",
        }
    }

    master_path = out_dir / "manifest.json"
    master_path.write_text(json.dumps(master_manifest, indent=2))
    print(f"\n=== Asset extraction complete! Master manifest written to {master_path} ===")


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("rom", nargs="?", default="baserom.gba", help="Path to GBA ROM (default baserom.gba)")
    p.add_argument("-o", "--outdir", default="assets", help="Output directory (default assets/)")
    p.add_argument("--lz-scan", default="docs/data/lz77_blobs.txt", help="Path to LZ77 scan file")
    p.add_argument("--subsystem", choices=["all", "tracks", "lz77", "mto", "containers", "sound"], default="all",
                   help="Specific subsystem to extract (default: all)")
    args = p.parse_args()

    rom_path = Path(args.rom)
    if not rom_path.exists():
        sys.exit(f"Error: ROM file {rom_path} not found")

    out_dir = Path(args.outdir)
    lz_scan_file = Path(args.lz_scan) if args.lz_scan else None

    rom = rom_path.read_bytes()
    if args.subsystem == "all":
        extract_all_assets(rom_path, out_dir, lz_scan_file)
    elif args.subsystem == "tracks":
        extract_track_package(rom, out_dir)
    elif args.subsystem == "lz77":
        extract_all_lz77_blobs(rom, out_dir, lz_scan_file)
    elif args.subsystem == "mto":
        extract_mto_directory(rom, out_dir)
    elif args.subsystem == "containers":
        extract_nested_containers(rom, out_dir)
    elif args.subsystem == "sound":
        extract_sound_assets(rom, out_dir)


if __name__ == "__main__":
    main()
