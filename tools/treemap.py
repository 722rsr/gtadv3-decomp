#!/usr/bin/env python3
"""Generate cushion treemap visualizations for decompilation progress.

Visualizes every function in the cartridge binary as a 3D-shaded "cushion"
tile using the van Wijk (1999) cushion treemap algorithm. Verified report spans
include both functions and source-owned non-function code bytes, so the full
code region is represented. Tiles are grouped by subsystem/module.

Modes:
  --mode independent: Green = byte-exact function,
                      Teal = verified non-function source bytes,
                      Red = unmatched code.
  --mode c-lift:      Green = C-source function,
                      Red = assembly-owned function,
                      Slate = non-function source bytes.
  --mode subsystem:   Distinct color palette per game subsystem (Audio,
                      Physics, AI, Race, UI/Menus, Garage, Save, System/Boot).

Outputs:
  - High-resolution cushion PNG image
  - Interactive standalone HTML viewer with hover tooltips and statistics

Usage:
  python3 tools/treemap.py [--mode independent|c-lift|subsystem] [--out build/treemap/treemap.png] [--html build/treemap/treemap.html]
"""

from __future__ import annotations

import argparse
import collections
import html
import json
import math
import os
import sys
from pathlib import Path
from typing import Any, Dict, List, Tuple

import numpy as np
from PIL import Image

# Import existing repo coverage utilities
REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT))
import tools.coverage as cov
import tools.decomp_report as decomp_report

BOOT_SLICE_END = 0x0015F4

# Subsystem categorization based on source filename
SUBSYSTEM_MAP = [
    ("Audio", ["sound", "mixer", "audio"]),
    ("Physics", ["carphys", "surface"]),
    ("Course & AI", ["ai_", "course_"]),
    ("Race", ["race_", "go_start", "grant_delay", "car_award", "award_"]),
    ("UI & Menus", ["menu_", "menus"]),
    ("Garage", ["garage"]),
    ("Save & Replay", ["save", "ghost", "rec35"]),
    ("System & Boot", ["agbmain", "boot", "header", "softirq", "idle", "keypad", "handlers", "blocka", "blockb", "slice"]),
]

SUBSYSTEM_COLORS: Dict[str, np.ndarray] = {
    "Audio": np.array([220, 140, 20], dtype=float),       # Warm Amber
    "Physics": np.array([210, 50, 80], dtype=float),       # Crimson / Rose
    "Course & AI": np.array([140, 60, 200], dtype=float),  # Purple / Indigo
    "Race": np.array([20, 160, 220], dtype=float),        # Sky Blue
    "UI & Menus": np.array([0, 190, 70], dtype=float),     # Vibrant Green
    "Garage": np.array([230, 190, 30], dtype=float),       # Gold
    "Save & Replay": np.array([50, 120, 220], dtype=float),# Royal Blue
    "System & Boot": np.array([0, 210, 170], dtype=float), # Teal / Cyan
    "General Code": np.array([120, 140, 160], dtype=float),# Slate
}


def classify_subsystem(filename: str) -> str:
    f_lower = filename.lower()
    for name, patterns in SUBSYSTEM_MAP:
        if any(pat in f_lower for pat in patterns):
            return name
    return "General Code"


def gather_function_data() -> Tuple[List[Dict[str, Any]], Dict[str, Any]]:
    """Load exact function and non-function spans from the verified report."""
    decomp_report.check(REPO_ROOT)
    progress = json.loads((REPO_ROOT / "docs/data/report.json").read_text())
    defined_c, aliases = cov.parse_c(str(REPO_ROOT / "src"))

    names_by_vma: Dict[str, set[str]] = {}
    for name in list(defined_c) + list(aliases):
        v = cov.norm_vma(name)
        if v:
            names_by_vma.setdefault(v, set()).add(name)

    vma_to_name: Dict[str, str] = {}
    for name in defined_c:
        v = cov.norm_vma(name)
        if v and v not in vma_to_name:
            vma_to_name[v] = name
    for name in aliases:
        v = cov.norm_vma(name)
        if v and v not in vma_to_name:
            vma_to_name[v] = name

    functions: List[Dict[str, Any]] = []
    for unit in progress["units"]:
        owner = unit.get("metadata", {}).get("source_path", unit["name"])
        if not owner.startswith("asm/"):
            owner = f"asm/{owner}"
        subsystem = classify_subsystem(owner)

        for record in unit.get("functions", []):
            addr = int(record["metadata"]["virtual_address"])
            offset = addr - 0x08000000
            vma = cov.norm_vma(f"{addr:08X}") or f"{offset:06X}"
            is_c_function = any(
                cov.resolve(name, defined_c, aliases) == "real"
                for name in names_by_vma.get(vma, ())
            )
            functions.append({
                "kind": "function",
                "vma": vma,
                "addr": addr,
                "name": vma_to_name.get(vma, record["name"]),
                "size": int(record["size"]),
                "asm_file": owner,
                "subsystem": subsystem,
                "is_boot_slice": offset < BOOT_SLICE_END,
                "is_c_lifted": is_c_function,
                "is_byte_matched": float(record.get("fuzzy_match_percent", 0)) == 100.0,
            })

        for record in unit.get("sections", []):
            addr = int(record["metadata"]["virtual_address"])
            offset = addr - 0x08000000
            functions.append({
                "kind": "section",
                "vma": f"{offset:06X}",
                "addr": addr,
                "name": record["name"],
                "size": int(record["size"]),
                "asm_file": owner,
                "subsystem": subsystem,
                "is_boot_slice": offset < BOOT_SLICE_END,
                "is_c_lifted": False,
                "is_byte_matched": float(record.get("fuzzy_match_percent", 0)) == 100.0,
            })

    measures = progress["measures"]
    total_code_bytes = int(measures["total_code"])
    if sum(item["size"] for item in functions) != total_code_bytes:
        raise ValueError("treemap spans do not partition the verified code region")
    function_items = [item for item in functions if item["kind"] == "function"]
    stats = {
        "total_functions": int(measures["total_functions"]),
        "matched_functions": int(measures["matched_functions"]),
        "function_match_percent": float(measures["matched_functions_percent"]),
        "total_code_bytes": total_code_bytes,
        "matched_code_bytes": int(measures["matched_code"]),
        "code_match_percent": float(measures["matched_code_percent"]),
        "non_function_code_bytes": sum(item["size"] for item in functions
                                        if item["kind"] == "section"),
        "non_function_spans": sum(item["kind"] == "section" for item in functions),
        "boot_slice_functions": sum(item["is_boot_slice"] for item in function_items),
        "c_lifted_functions": sum(item["is_c_lifted"] for item in function_items),
        "unmatched_functions": sum(not item["is_byte_matched"] for item in function_items),
        "unmatched_code_spans": sum(not item["is_byte_matched"] for item in functions),
    }
    return functions, stats


def squarify(
    items: List[Any],
    x: float,
    y: float,
    dx: float,
    dy: float,
    size_fn=lambda it: it["size"],
) -> List[Tuple[Any, float, float, float, float]]:
    """Squarified Treemap layout algorithm (Bruls, Huizing, van Wijk, 2000)."""
    if not items:
        return []
    total = sum(size_fn(it) for it in items)
    if total <= 0:
        return []

    area = dx * dy
    norm_items = [{"item": it, "norm_size": size_fn(it) * area / total} for it in items]
    norm_items.sort(key=lambda it: it["norm_size"], reverse=True)

    rects = []

    def layout_row(row, rx, ry, rdx, rdy):
        row_area = sum(r["norm_size"] for r in row)
        if rdx >= rdy:
            rw = row_area / rdy if rdy > 0 else 0
            cy = ry
            res = []
            for r in row:
                rh = r["norm_size"] / rw if rw > 0 else 0
                res.append((r["item"], rx, cy, rw, rh))
                cy += rh
            rem = (rx + rw, ry, rdx - rw, rdy)
        else:
            rh = row_area / rdx if rdx > 0 else 0
            cx = rx
            res = []
            for r in row:
                rw = r["norm_size"] / rh if rh > 0 else 0
                res.append((r["item"], cx, ry, rw, rh))
                cx += rw
            rem = (rx, ry + rh, rdx, rdy - rh)
        return res, rem

    def worst_ratio(row, side):
        if not row or side == 0:
            return float("inf")
        s = sum(r["norm_size"] for r in row)
        if s == 0:
            return float("inf")
        side2 = side * side
        s2 = s * s
        max_r = 0.0
        for r in row:
            val = r["norm_size"]
            if val <= 0:
                continue
            ratio = max(val * side2 / s2, s2 / (val * side2))
            if ratio > max_r:
                max_r = ratio
        return max_r

    cur_x, cur_y, cur_dx, cur_dy = x, y, dx, dy
    cur_row: List[Dict[str, Any]] = []
    remaining = norm_items[:]

    while remaining:
        side = min(cur_dx, cur_dy)
        if side <= 0:
            break
        item = remaining[0]
        new_row = cur_row + [item]
        if not cur_row or worst_ratio(new_row, side) <= worst_ratio(cur_row, side):
            cur_row = new_row
            remaining.pop(0)
        else:
            row_rects, (cur_x, cur_y, cur_dx, cur_dy) = layout_row(
                cur_row, cur_x, cur_y, cur_dx, cur_dy
            )
            rects.extend(row_rects)
            cur_row = []

    if cur_row:
        row_rects, _ = layout_row(cur_row, cur_x, cur_y, cur_dx, cur_dy)
        rects.extend(row_rects)

    return rects


def compute_layout(
    functions: List[Dict[str, Any]], width: int, height: int
) -> Tuple[List[Tuple[Dict[str, Any], float, float, float, float]], List[Tuple[str, float, float, float, float]]]:
    """Compute hierarchical treemap layout (Subsystem -> Code spans)."""
    subsystems: Dict[str, List[Dict[str, Any]]] = collections.OrderedDict()
    for fn in functions:
        subsystems.setdefault(fn["subsystem"], []).append(fn)

    subsys_items = [
        {"name": name, "size": sum(f["size"] for f in fns), "fns": fns}
        for name, fns in subsystems.items()
    ]
    subsys_rects = squarify(subsys_items, 0, 0, width, height)

    fn_rects: List[Tuple[Dict[str, Any], float, float, float, float]] = []
    subsys_bounds: List[Tuple[str, float, float, float, float]] = []

    for sub_item, sx, sy, sw, sh in subsys_rects:
        subsys_bounds.append((sub_item["name"], sx, sy, sw, sh))
        inner_rects = squarify(sub_item["fns"], sx, sy, sw, sh)
        fn_rects.extend(inner_rects)

    return fn_rects, subsys_bounds


def get_tile_color(fn: Dict[str, Any], mode: str) -> np.ndarray:
    """Determine base RGB color for a code-span tile based on display mode."""
    if mode == "subsystem":
        return SUBSYSTEM_COLORS.get(fn["subsystem"], np.array([120, 140, 160], dtype=float))

    if mode == "independent":
        if not fn["is_byte_matched"]:
            return np.array([225, 40, 25], dtype=float)   # Red (unmatched)
        if fn["kind"] == "section":
            return np.array([25, 175, 115], dtype=float)  # Teal (verified non-function bytes)
        return np.array([0, 195, 45], dtype=float)        # Green (byte-exact function)

    # mode == "c-lift"
    if fn["kind"] == "section":
        return np.array([105, 115, 130], dtype=float)     # Slate (non-function bytes)
    if fn["is_c_lifted"]:
        return np.array([0, 195, 30], dtype=float)        # Decompiled C Green
    else:
        return np.array([225, 40, 25], dtype=float)       # Red (ASM gap)


def render_cushion_treemap(
    fn_rects: List[Tuple[Dict[str, Any], float, float, float, float]],
    subsys_bounds: List[Tuple[str, float, float, float, float]],
    width: int,
    height: int,
    mode: str,
) -> Image.Image:
    """Render spans using van Wijk's (1999) parabolic cushion algorithm."""
    img = np.zeros((height, width, 3), dtype=np.uint8)

    # Directional light source from top-left:
    lx, ly, lz = -0.50, -0.50, 0.707
    l_len = math.sqrt(lx * lx + ly * ly + lz * lz)
    lx, ly, lz = lx / l_len, ly / l_len, lz / l_len

    Ia = 0.32  # Ambient light
    Id = 0.68  # Diffuse light

    for fn, rx, ry, rw, rh in fn_rects:
        ix0 = max(0, min(width, int(round(rx))))
        iy0 = max(0, min(height, int(round(ry))))
        ix1 = max(0, min(width, int(round(rx + rw))))
        iy1 = max(0, min(height, int(round(ry + rh))))

        bw = ix1 - ix0
        bh = iy1 - iy0
        if bw <= 0 or bh <= 0:
            continue

        base_col = get_tile_color(fn, mode)

        if bw <= 2 or bh <= 2:
            img[iy0:iy1, ix0:ix1] = (base_col * 0.75).astype(np.uint8)
            continue

        xs = np.arange(bw)
        ys = np.arange(bh)
        X, Y = np.meshgrid(xs, ys)

        # Adaptive cushion height scaled with tile dimension
        h_max = min(12.0, 0.28 * min(bw, bh) + 1.0)

        # Van Wijk parabolic normal derivatives:
        dh_dx = 4.0 * h_max * (bw - 2.0 * X) / (bw * bw)
        dh_dy = 4.0 * h_max * (bh - 2.0 * Y) / (bh * bh)

        Nx = -dh_dx
        Ny = -dh_dy
        Nz = np.ones_like(Nx)
        norm = np.sqrt(Nx * Nx + Ny * Ny + Nz * Nz)
        Nx /= norm
        Ny /= norm
        Nz /= norm

        dot = np.maximum(0.0, Nx * lx + Ny * ly + Nz * lz)
        intensity = Ia + Id * dot

        # Dark border (1px edge)
        border_mask = (X == 0) | (X == bw - 1) | (Y == 0) | (Y == bh - 1)

        rgb = intensity[:, :, None] * base_col[None, None, :]
        rgb[border_mask] *= 0.30
        rgb = np.clip(rgb, 0, 255).astype(np.uint8)

        img[iy0:iy1, ix0:ix1] = rgb

    # Draw crisp black subsystem separators
    for _, sx, sy, sw, sh in subsys_bounds:
        ix0 = max(0, min(width - 1, int(round(sx))))
        ix1 = max(0, min(width - 1, int(round(sx + sw))))
        iy0 = max(0, min(height - 1, int(round(sy))))
        iy1 = max(0, min(height - 1, int(round(sy + sh))))
        img[iy0:iy1, ix0] = 0
        img[iy0:iy1, ix1] = 0
        img[iy0, ix0:ix1] = 0
        img[iy1, ix0:ix1] = 0

    return Image.fromarray(img)


def generate_interactive_html(
    fn_rects: List[Tuple[Dict[str, Any], float, float, float, float]],
    subsys_bounds: List[Tuple[str, float, float, float, float]],
    width: int,
    height: int,
    mode: str,
    stats: Dict[str, Any],
    png_rel_path: str,
) -> str:
    """Generate self-contained HTML page with interactive hover tooltips."""
    rects_json = []
    for fn, rx, ry, rw, rh in fn_rects:
        rects_json.append({
            "kind": fn["kind"],
            "name": fn["name"],
            "vma": fn["vma"],
            "addr": f"0x{fn['addr']:08X}",
            "size": fn["size"],
            "source": fn["asm_file"],
            "subsystem": fn["subsystem"],
            "is_boot_slice": fn["is_boot_slice"],
            "is_c_lifted": fn["is_c_lifted"],
            "is_byte_matched": fn["is_byte_matched"],
            "x": round(rx, 1),
            "y": round(ry, 1),
            "w": round(rw, 1),
            "h": round(rh, 1),
        })

    title = "GT Advance 3 — Decompilation Treemap"
    if mode == "subsystem":
        legend_items = "".join(
            f'<div class="legend-item"><div class="color-swatch" style="background:rgb({int(color[0])},{int(color[1])},{int(color[2])});"></div>{html.escape(name)}</div>'
            for name, color in SUBSYSTEM_COLORS.items()
        )
    elif mode == "c-lift":
        legend_items = (
            '<div class="legend-item"><div class="color-swatch" style="background:#00c31e;"></div>C source function</div>'
            '<div class="legend-item"><div class="color-swatch" style="background:#e12819;"></div>Assembly-owned function</div>'
            '<div class="legend-item"><div class="color-swatch" style="background:#697382;"></div>Non-function source bytes</div>'
        )
    else:
        legend_items = (
            '<div class="legend-item"><div class="color-swatch" style="background:#00c32d;"></div>Byte-exact function</div>'
            '<div class="legend-item"><div class="color-swatch" style="background:#19af73;"></div>Verified non-function source bytes</div>'
            '<div class="legend-item"><div class="color-swatch" style="background:#e12819;"></div>Unmatched code</div>'
        )
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>{title}</title>
<style>
  body {{
    margin: 0;
    padding: 24px;
    background: #0f1117;
    color: #e2e8f0;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
  }}
  h1 {{ margin: 0 0 8px 0; font-size: 24px; font-weight: 600; color: #f8fafc; }}
  .subtitle {{ color: #94a3b8; font-size: 14px; margin-bottom: 20px; }}
  .stats-bar {{
    display: flex;
    gap: 24px;
    margin-bottom: 20px;
    background: #1e293b;
    padding: 12px 18px;
    border-radius: 8px;
    font-size: 14px;
    flex-wrap: wrap;
  }}
  .stat-item span {{ font-weight: bold; color: #38bdf8; }}
  .container {{
    position: relative;
    display: inline-block;
    border: 2px solid #334155;
    border-radius: 6px;
    overflow: hidden;
    background: #000;
  }}
  canvas {{ display: block; cursor: crosshair; }}
  #tooltip {{
    position: absolute;
    display: none;
    pointer-events: none;
    background: rgba(15, 23, 42, 0.94);
    border: 1px solid #475569;
    border-radius: 6px;
    padding: 10px 14px;
    font-size: 13px;
    line-height: 1.4;
    color: #f1f5f9;
    box-shadow: 0 10px 25px rgba(0,0,0,0.5);
    z-index: 100;
  }}
  #tooltip .fn-name {{ font-weight: 600; color: #38bdf8; font-size: 14px; }}
  #tooltip .fn-badge {{
    display: inline-block;
    padding: 2px 6px;
    border-radius: 4px;
    font-size: 11px;
    font-weight: 600;
    margin-top: 4px;
  }}
  .badge-match {{ background: #15803d; color: #dcfce7; }}
  .badge-lifted {{ background: #0369a1; color: #e0f2fe; }}
  .badge-asm {{ background: #b91c1c; color: #fee2e2; }}
  .legend {{
    margin-top: 16px;
    display: flex;
    gap: 20px;
    font-size: 13px;
    color: #cbd5e1;
    align-items: center;
  }}
  .legend-item {{ display: flex; align-items: center; gap: 8px; }}
  .color-swatch {{ width: 16px; height: 16px; border-radius: 3px; }}
</style>
</head>
<body>

<h1>{title}</h1>
<div class="subtitle">Verified ROM code-region spans: functions and source-owned non-function bytes. Tiles are sized by their exact byte ranges.</div>

<div class="stats-bar">
  <div class="stat-item">Byte-exact functions: <span>{stats['matched_functions']:,} / {stats['total_functions']:,} ({stats['function_match_percent']:.2f}%)</span></div>
  <div class="stat-item">Byte-exact code region: <span>{stats['matched_code_bytes']:,} / {stats['total_code_bytes']:,} ({stats['code_match_percent']:.2f}%)</span></div>
  <div class="stat-item">Non-function source ranges: <span>{stats['non_function_spans']:,} ({stats['non_function_code_bytes']:,} bytes)</span></div>
  <div class="stat-item">Unmatched code spans: <span>{stats['unmatched_code_spans']:,}</span></div>
</div>

<div class="container" id="treemap-container">
  <canvas id="treemap-canvas" width="{width}" height="{height}"></canvas>
  <div id="tooltip"></div>
</div>

<div class="legend">
  <strong>Display Mode ({mode}):</strong>
  {legend_items}
</div>

<script>
const rects = {json.dumps(rects_json)};
const canvas = document.getElementById('treemap-canvas');
const ctx = canvas.getContext('2d');
const tooltip = document.getElementById('tooltip');
const container = document.getElementById('treemap-container');

// Load rendered cushion image
const img = new Image();
img.src = '{png_rel_path}';
img.onload = () => {{
  ctx.drawImage(img, 0, 0);
}};

let hoveredRect = null;

canvas.addEventListener('mousemove', (e) => {{
  const rect = canvas.getBoundingClientRect();
  const scaleX = canvas.width / rect.width;
  const scaleY = canvas.height / rect.height;
  const mx = (e.clientX - rect.left) * scaleX;
  const my = (e.clientY - rect.top) * scaleY;

  // Find topmost rect under mouse
  let found = null;
  for (let i = rects.length - 1; i >= 0; i--) {{
    const r = rects[i];
    if (mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h) {{
      found = r;
      break;
    }}
  }}

  if (found) {{
    hoveredRect = found;
    const badgeClass = found.is_byte_matched ? 'badge-match' : 'badge-asm';
    const badgeText = found.kind === 'section'
      ? (found.is_byte_matched ? 'Verified non-function source bytes' : 'Unmatched non-function source bytes')
      : (found.is_byte_matched ? 'Byte-identical function' : 'Not yet byte-matched');

    tooltip.innerHTML = `
      <div class="fn-name">${{found.name}}</div>
      <div><strong>Address:</strong> ${{found.addr}}</div>
      <div><strong>Size:</strong> ${{found.size}} bytes</div>
      <div><strong>Subsystem:</strong> ${{found.subsystem}}</div>
      <div><strong>Source:</strong> ${{found.source}}</div>
      <div class="fn-badge ${{badgeClass}}">${{badgeText}}</div>
    `;
    tooltip.style.display = 'block';

    // Position tooltip
    const tipX = Math.min(e.clientX - rect.left + 15, rect.width - 240);
    const tipY = Math.min(e.clientY - rect.top + 15, rect.height - 130);
    tooltip.style.left = tipX + 'px';
    tooltip.style.top = tipY + 'px';
  }} else {{
    tooltip.style.display = 'none';
    hoveredRect = null;
  }}
}});

canvas.addEventListener('mouseleave', () => {{
  tooltip.style.display = 'none';
  hoveredRect = null;
}});
</script>
</body>
</html>
"""


def main():
    parser = argparse.ArgumentParser(description="Generate cushion treemap of decompilation progress.")
    parser.add_argument(
        "--mode",
        choices=["independent", "c-lift", "subsystem"],
        default="independent",
        help="Coloring mode: 'independent' (slice 1 vs hybrid vs asm), 'c-lift' (C vs ASM), 'subsystem' (by subsystem).",
    )
    parser.add_argument("--width", type=int, default=1024, help="Output image width in pixels (default 1024).")
    parser.add_argument("--height", type=int, default=512, help="Output image height in pixels (default 512).")
    parser.add_argument("--out", default="build/treemap/treemap.png", help="Output PNG path.")
    parser.add_argument("--html", default="build/treemap/index.html", help="Output interactive HTML path.")
    args = parser.parse_args()

    out_png = Path(args.out).resolve()
    out_html = Path(args.html).resolve()
    out_png.parent.mkdir(parents=True, exist_ok=True)
    out_html.parent.mkdir(parents=True, exist_ok=True)

    print("Gathering functions and coverage data from repo...")
    functions, stats = gather_function_data()
    print(f"Found {stats['total_functions']:,} functions plus "
          f"{stats['non_function_spans']:,} non-function source ranges "
          f"({stats['total_code_bytes']:,} bytes total).")
    print(f"  - Byte-exact functions:      {stats['matched_functions']:,}/"
          f"{stats['total_functions']:,} ({stats['function_match_percent']:.2f}%)")
    print(f"  - Byte-exact code region:    {stats['matched_code_bytes']:,}/"
          f"{stats['total_code_bytes']:,} ({stats['code_match_percent']:.2f}%)")
    print(f"  - Unmatched functions:       {stats['unmatched_functions']:,}")

    print(f"Computing hierarchical squarified treemap layout ({args.width}x{args.height})...")
    fn_rects, subsys_bounds = compute_layout(functions, args.width, args.height)

    print(f"Rendering 3D cushion treemap (mode: {args.mode})...")
    img = render_cushion_treemap(fn_rects, subsys_bounds, args.width, args.height, args.mode)
    img.save(out_png)
    print(f"Saved PNG to {out_png}")

    png_rel_path = os.path.relpath(out_png, out_html.parent)
    html_content = generate_interactive_html(
        fn_rects, subsys_bounds, args.width, args.height, args.mode, stats, png_rel_path
    )
    out_html.write_text(html_content, encoding="utf-8")
    print(f"Saved interactive HTML to {out_html}")


if __name__ == "__main__":
    main()
