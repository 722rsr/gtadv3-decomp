# Integrated data regions

`tools/data_regions.json` is the registry. Each entry has a file-offset range,
format, SHA-256, private editable/binary paths under `build/data/`, consumer,
and evidence. End addresses are exclusive.

| id | range | bytes | consumer |
| --- | --- | ---: | --- |
| `early_data` | `0x2E158..0xCE438` | 656096 | MTO dir + early tail via `tools/mto_dump.py` / `tools/track_dump.py` |
| `mto_group0` | `0xCE438..0x16BA2C` | 644596 | group-0 course chain via `_08006590` → `CourseOrch_080064EC` |
| `surface_map` | `0x16BA2C..0x23CEFC` | 857296 | group 1 LZ77 (verbatim) to `0x02000000` via `_08006574` |
| `theme_texture` | `0x23CEFC..0x252DC4` | 89800 | group 2 LZ77 (verbatim) to `0x06000000` via `CourseOrch_08006468` |
| `theme_lut_a` | `0x252DC4..0x253604` | 2112 | group 3 LUT pointer via `CourseOrch_08006468` |
| `theme_palette` | `0x253604..0x254044` | 2624 | group 4 DMA to `0x05000000` via `CourseOrch_08006468` |
| `surface_tile` | `0x254044..0x25A118` | 24788 | group 5 LZ77 (verbatim) to `0x06008000` via `CourseOrch_080061B8` |
| `variant_palette` | `0x25A118..0x25A230` | 280 | group 6 DMA to `0x050001E0` via `CourseOrch_080061B8` |
| `variant_overlay` | `0x25A230..0x25AF78` | 3400 | group 7 LZ77 (verbatim) to `0x0600E000` via `CourseOrch_080061B8` |
| `big_gfx` | `0x25AF78..0x284E2C` | 171700 | group 8 LZ77 (verbatim) to `0x0600A000` via `CourseOrch_0800628C` |
| `scenery_table` | `0x284E2C..0x2856DC` | 2224 | group 9 DMA to `0x050001C0` via `CourseOrch_0800628C` |
| `minimap` | `0x2856DC..0x28A35C` | 19584 | group 10 LZ77 (verbatim) to `0x02000000` via `CourseOrch_0800628C` |
| `mid_gap` | `0x28A35C..0x3D7DA4` | 1366600 | post-chain tail; see `docs/data/ptr_tables.txt`, `docs/data/lz77_blobs.txt` |
| `blob_field` | `0x3D7DA4..0x799640` | 3938460 | LZ77 blobs decoded by `tools/lz77.py` |
| `tail_end` | `0x799640..0x7B04C4` | 93828 | tail through content end `0x7B04C4` |

Bounds come from `docs/data/track_resources.txt` and `tools/track_dump.py`.
Compressed streams are kept verbatim; no encoder policy is claimed.

## Formats

- `variant-palette-v1`: 7 records of 8-byte header + sixteen BGR555 halfwords (bit 15 preserved).
- `mto-raw-v1`: N records of 8-byte header (`group`, `index`, `size`) + verbatim payload as base64.
- `raw-span-v1`: one verbatim byte span as base64.

## Build and verification

```sh
make data-check
```

Extract validates the ROM span against the registry and `track_dump.py`, then
writes the editable JSON. Generate reads only the editable JSON and fails on
any byte/index/size mismatch without overwriting the edit. `data-check`
checks round trips, span hashes, `build-code/data.o`, and the full ROM.
`asm/data_tail.s` includes each generated binary; `tools/ownership_map.py`
credits them as `verified-generated-data`. Never commit `build/data/` or
extracted bytes; `make clean` removes them.

## Adding a region

Document format, consumer, bounds, and storage first. Add the registry entry,
`data_tail.s` include, Makefile extract/generate rules, and `ownership_map.py`
raw splits. Cover invalid fields, truncation, stale output, and overlaps in
`tools/test_data_regions.py`. Then run `make ownership-map`, `make data-check`,
`make progress`.
