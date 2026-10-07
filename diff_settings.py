#!/usr/bin/env python3
"""asm-differ configuration for GT Advance 3 decompilation."""
from pathlib import Path

ROOT = Path(__file__).resolve().parent

def apply(config, args):
    config["baseimg"] = str(ROOT / "baserom.gba")
    config["myimg"] = str(ROOT / "build/gtadv3.gba")
    config["mapfile"] = str(ROOT / "build/rom.map")
    config["source_directories"] = [
        str(ROOT / "src"),
        str(ROOT / "include"),
        str(ROOT / "asm")
    ]
    config["arch"] = "arm32"
    config["map_format"] = "gnu"
    config["objdump_executable"] = "arm-none-eabi-objdump"
