"""Pinned agbcc options for source translation units."""
from __future__ import annotations

from pathlib import Path


# The ROM's memory-set helper ends with `pop {r4,r5,pc}`. old_agbcc emits that
# epilogue only without interworking, so keep this ABI exception isolated to
# its own translation unit.
NON_INTERWORKING_TUS = {"runtime_memset.c"}


def options_for(source: str | Path, *, function_sections: bool = True) -> tuple[str, ...]:
    flags = ["-O2"]
    flags.append("-mno-thumb-interwork" if Path(source).name in NON_INTERWORKING_TUS
                 else "-mthumb-interwork")
    if function_sections:
        flags.append("-ffunction-sections")
    return tuple(flags)
