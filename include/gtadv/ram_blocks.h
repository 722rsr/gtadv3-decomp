#ifndef GTADV_RAM_BLOCKS_H
#define GTADV_RAM_BLOCKS_H

// ---------------------------------------------------------------------------
// Declared RAM blocks — the base address of an IWRAM/EWRAM object, spelled as
// a SYMBOL rather than as an integer constant.
//
// Why this exists: spelling a field as `0x030002d8 + 68` lets agbcc fold the
// sum at compile time, so it materialises the FIELD address in the literal pool
// and reaches it with a zero displacement. The ROM frequently instead holds the
// BLOCK address in the pool and reaches the field with a separate
// `adds rN,#imm`. Measured on 0x08004cc4 (`src/foundation_subsys.c`): 1/16 for
// every plain-C spelling of the same store (pointer locals of every width, a
// struct with a named field at that offset, `p += n/2` on a u16*, a u32 base
// stepped by `+= n`, a runtime index variable) and 16/16 the moment the base is
// a symbol. `ldr r0,=Block` is a relocation against a named object, so the
// constant cannot be folded into it.
//
// A block is registered here once, with its address, and a translation unit
// uses two macros:
//
//     RAM_DEF_<ADDRESS>;                       // inside the function body
//     volatile u16 *p = (volatile u16 *)
//         (RAM_SYM_<ADDRESS> + offset);
//
// `RAM_DEF_*` MUST be a statement inside the body, not a file-scope `__asm__`:
// `match_c_slice.py` splices the function's own agbcc section, so a file-scope
// definition is dropped from the spliced text and the reference goes undefined
// at link time (src/save.c's `SaveBumpBase` records the same constraint, and
// 0x08005780 is the promoted entry that proves the in-body form links).
//
// The host build has no absolute-symbol mechanism, so it uses a pointer to the
// literal address; the host bodies are not byte-checked.
// ---------------------------------------------------------------------------

// 0x030000E4 — State Block A slot (holds the pointer to block A).
#ifdef __APPLE__
#define RAM_DEF_030000E4 ((void)0)
#define RAM_SYM_030000E4 ((volatile u8 *)(uintptr_t)0x030000E4u)
#else
extern volatile u8 RamBlock_030000E4[];
#define RAM_DEF_030000E4 __asm__(".set RamBlock_030000E4, 0x030000E4")
#define RAM_SYM_030000E4 ((volatile u8 *)RamBlock_030000E4)
#endif

// 0x03000198 — subsystem manager block (see src/foundation_subsys.c).
#ifdef __APPLE__
#define RAM_DEF_03000198 ((void)0)
#define RAM_SYM_03000198 ((volatile u8 *)(uintptr_t)0x03000198u)
#else
extern volatile u8 RamBlock_03000198[];
#define RAM_DEF_03000198 __asm__(".set RamBlock_03000198, 0x03000198")
#define RAM_SYM_03000198 ((volatile u8 *)RamBlock_03000198)
#endif

// 0x03000260 — event/DMA queue head block.
#ifdef __APPLE__
#define RAM_DEF_03000260 ((void)0)
#define RAM_SYM_03000260 ((volatile u8 *)(uintptr_t)0x03000260u)
#else
extern volatile u8 RamBlock_03000260[];
#define RAM_DEF_03000260 __asm__(".set RamBlock_03000260, 0x03000260")
#define RAM_SYM_03000260 ((volatile u8 *)RamBlock_03000260)
#endif

// 0x030002D8 — save/heap bump cursor block.
#ifdef __APPLE__
#define RAM_DEF_030002D8 ((void)0)
#define RAM_SYM_030002D8 ((volatile u8 *)(uintptr_t)0x030002D8u)
#else
extern volatile u8 RamBlock_030002D8[];
#define RAM_DEF_030002D8 __asm__(".set RamBlock_030002D8, 0x030002D8")
#define RAM_SYM_030002D8 ((volatile u8 *)RamBlock_030002D8)
#endif

// 0x030003E4 — course record table slot (holds the table pointer).
#ifdef __APPLE__
#define RAM_DEF_030003E4 ((void)0)
#define RAM_SYM_030003E4 ((volatile u8 *)(uintptr_t)0x030003E4u)
#else
extern volatile u8 RamBlock_030003E4[];
#define RAM_DEF_030003E4 __asm__(".set RamBlock_030003E4, 0x030003E4")
#define RAM_SYM_030003E4 ((volatile u8 *)RamBlock_030003E4)
#endif

// 0x030013D0 — save block 2 buffer (SaveGarageMaskTest reads it at +0x2E).
#ifdef __APPLE__
#define RAM_DEF_030013D0 ((void)0)
#define RAM_SYM_030013D0 ((volatile u8 *)(uintptr_t)0x030013D0u)
#else
extern volatile u8 RamBlock_030013D0[];
#define RAM_DEF_030013D0 __asm__(".set RamBlock_030013D0, 0x030013D0")
#define RAM_SYM_030013D0 ((volatile u8 *)RamBlock_030013D0)
#endif

// 0x03001780 — work area (WA).
#ifdef __APPLE__
#define RAM_DEF_03001780 ((void)0)
#define RAM_SYM_03001780 ((volatile u8 *)(uintptr_t)0x03001780u)
#else
extern volatile u8 RamBlock_03001780[];
#define RAM_DEF_03001780 __asm__(".set RamBlock_03001780, 0x03001780")
#define RAM_SYM_03001780 ((volatile u8 *)RamBlock_03001780)
#endif

#endif // GTADV_RAM_BLOCKS_H
