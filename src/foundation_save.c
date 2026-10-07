#include "gtadv/foundation.h"
#include "gtadv/memory.h"
#include "gba/types.h"
#include "gba/bios.h"


// code_57d0, 5988, c668, ce2c — duplicate VMAs removed per coverage_blocker_audit.md §4
// Owners: _080057D0 → foundation_late.c (SaveSlotConfig), _08005988 → save.c (SaveSlotLoad),
// _0800C668/_0800CE2C → foundation_late.c (LateDispatch). No definitions here; see owning files.

// code_fa0 — VBlank/BIOS dispatch (0xFA0..0x15F4) — lifted in foundation_fa0.c
// (IntrMain_Dispatch + Helper_014A4/014B4/013BC/013E0/015B8) with sibling
// leaves in runtime_accessors.c; no stub here.
