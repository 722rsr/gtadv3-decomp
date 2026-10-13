#include "gtadv/save.h"
#include "gtadv/ram_blocks.h"
#include "gtadv/memory.h"
#include "gba/bios.h"
#include "gba/regs.h"

// Save / EEPROM lane — C lifting of every remaining save_*.s + saveblock.s unit.
// Faithful port; preserves field widths, checksum seed, and slot contracts.
// VMA references cite original asm and asm/saveblock.s.

// External BIOS and driver hooks (weak so this lane links standalone on host builds)
__attribute__((weak)) void CpuSetWrap(const void *src, void *dst, u32 mode) { CpuFastSet(src, dst, mode); }
__attribute__((weak)) int EepromPanic(const char *msg) { (void)msg; return -1; }
__attribute__((weak)) int EepromReadSector(u32 sector, void *dst) { (void)sector; (void)dst; return 0; }
__attribute__((weak)) int EepromWriteSector(u32 sector, const void *src) { (void)sector; (void)src; return 0; }
__attribute__((weak)) int EepromVerifySector(u32 sector, const void *src) { (void)sector; (void)src; return 0; }

__attribute__((weak)) void sub_0802D974(const void *src, void *dst, u32 mode) { CpuSet(src, dst, mode); }
__attribute__((weak)) void sub_0802E0A4(const void *src, void *dst, u32 n) { (void)src; (void)dst; (void)n; }
__attribute__((weak)) void sub_0802B234(void) {}
__attribute__((weak)) void sub_0802B190(void) {}
__attribute__((weak)) void sub_0802B1B8(void) {}
// sub_08025CF4 weak stub removed — strong lift in runtime_state_dispatch.c (AiGridGet_25CF4)
__attribute__((weak)) void sub_08025CF4(void) {} // kept for host-only links that reference it directly
void SaveHook_0802446C(void);
void SaveUpdate_080241C8(void);
__attribute__((weak)) void sub_0800295C(u32 a, u32 b) { (void)a; (void)b; } // 0x0800295C Foundation_InitCommon, 2-arg (asm/code_295c.s)
extern void sub_080029D8(void);  // 0x080029D8 IRQ-suspend (raw asm)
extern void sub_08002A68(void);  // 0x08002A68 handler-slot reset (raw asm)
extern void sub_08002A0C(void);  // 0x08002A0C IRQ-restore (raw asm)
extern u32 sub_08002AF4(u32 v);
__attribute__((weak)) int sub_0802DA20(u32 a, u32 b) { (void)a; (void)b; return 0; }
__attribute__((weak)) int  sub_0802DBA4(u32 sec, void *dst) { (void)sec; (void)dst; return 0; }
__attribute__((weak)) int  sub_0802DC54(u32 a, u32 b) { (void)a; return 0; }
__attribute__((weak)) int  sub_0802DD30(u32 a, u32 b) { (void)a; return 0; }
__attribute__((weak)) void sub_08004BFC(u32 v) { (void)v; }
__attribute__((weak)) void sub_08004C84(u32 idx, u32 value) { (void)idx; (void)value; }
// ARM: real body SubsysDispatch (foundation_subsys.c). A live weak def here
// would self-shadow: save.c calls sub_08004D4C(11,0,0) and a compile-time
// local binding runs the no-op instead (-21 class).
extern void sub_08004D4C(u32 a, u32 b, u32 c);
__attribute__((weak)) void sub_0802417C(void) {}
__attribute__((weak)) void sub_0800D95C(void *d, const void *s, u32 n) { (void)d; (void)s; (void)n; }
// ----------------------------------------------------------------------------
// Call-site spellings for the closure-defined callees of the six saveblock
// bodies this file owns (0x08024B54/70/BC0/BD8/BF0/BFC).
// ----------------------------------------------------------------------------
extern void _08024A18(void);                        // SaveBlock2PreSave
extern void _08024A98(void);                        // SaveBlock2PostLoad
extern int  _08024B24(void);                        // SaveBlock2Load
extern int  _080059F0(u32 slotIdx, const void *src);// SaveSlotSave
extern void _08024B54(void);                        // SaveBlock2Save
extern void sub_0802446C(void);                     // SaveHook_0802446C
extern void _08002A0C(void);                        // IrqRestore (closure spelling)
#define SAVE_CALL_PRE_SAVE()          _08024A18()
#define SAVE_CALL_POST_LOAD()         _08024A98()
#define SAVE_CALL_BLOCK2_LOAD()       _08024B24()
#define SAVE_CALL_SLOT_SAVE(i, src)   _080059F0((i), (src))
#define SAVE_CALL_BLOCK2_SAVE()       _08024B54()
#define SAVE_CALL_HOOK_2446C()        sub_0802446C()
#define SAVE_CALL_IRQ_RESTORE()       _08002A0C()

void _08024B4C(void) { }
__asm__(".align 2, 0");
void _08024B50(void) { }
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// Bump allocator (asm/save_alloc.s + save_alloc_more.s)
// IWRAM 0x030002D8 layout: {u16 cursor; u16 used; u8 heap[0x7FFC]}
//
// The bump base is named through an absolute symbol rather than an integer
// literal. agbcc folds `(u8 *)0x030002DC + off` straight into a displacement
// (`adds r0,r0,r1; ldrb r0,[r0]`), but the ROM keeps the +4 as a separate
// `adds r1, #4` against the 0x030002D8 base. A CONST_INT base is folded by
// simplify_rtx at expand time; a SYMBOL_REF is not, which is what restores
// the two-step form. The symbol is absolute, so it allocates nothing in the
// slice link.
__asm__(".globl SaveBumpBase\nSaveBumpBase = 0x030002D8\n");
extern u8 SaveBumpBase[];
static inline volatile u16 *bump_cursor(void) { return (volatile u16 *)SaveBumpBase; }
static inline volatile u16 *bump_used(void)   { return (volatile u16 *)(SaveBumpBase + 2); }

void *SaveAlloc(u32 size) {
    volatile u16 *cur = bump_cursor();
    register u32 base __asm__("r4") = *cur;
    u32 next = base + size;
    *cur = (u16)next;
    if ((next << 16) > (0x80u << 19)) {
        extern void _0800295C(u32 a, u32 b);
        _0800295C(0x0805BAA4u, (u32)*cur);
    }
    return (void *)(uintptr_t)base;
}
void *_0800572C(u32 s) __attribute__((alias("SaveAlloc")));

int SaveAllocFree(u32 size) {
    volatile u8 *base = SaveBumpBase;
    volatile u16 *used = (volatile u16 *)(base + 2);
    // ROM (0x08005758): the pool holds the *base* 0x030002D8 and the +2 lives
    // in the addressing mode, so `used` must be reached through a u8* base
    // rather than as the folded constant 0x030002DA. Assigning the difference
    // to the *argument* is what ties the subtraction's destination to r0,
    // which is the register the ROM writes (`subs r0, r2, r0`).
    size = (u32)(*used) - (u32)size;
    *used = (u16)size;
    return *used; /* asm sub_08005758 reloads + returns the new cursor */
}
int _08005758(u32 s) __attribute__((alias("SaveAllocFree")));
int Sub_08005758(u32 s) __attribute__((alias("SaveAllocFree")));
// The closure spells this VMA `sub_08005758` and the entry exports it, so the
// owning TU must define that exact name. One hop to the real body.
int sub_08005758(u32 s) __attribute__((alias("SaveAllocFree")));

// 0x08005768 — ROM (asm/save_alloc_more.s):
//   `ldr r3,=0x030002D8 / adds r2,r3,#4 / adds r0,r0,r2 / ldrh r2,[r3] /
//    strb r2,[r0] / ldrh r0,[r3] / adds r1,r0,r1 / strh r1,[r3]`
// = write the cursor's low byte into byte[SaveBumpBase + 4 + off], then bump
// the cursor by v. Four measured levers:
//   * the +4 must stay a separate `adds r2,r3,#4` against the SaveBumpBase
//     SYMBOL_REF. Folding it into the address (`(u8 *)0x030002DC`, the previous
//     body) makes the whole pair disappear and matched 0/24.
//   * the cursor is read TWICE by the ROM (once for the byte, once for the
//     add). Binding a snapshot local introduced a saved register and a
//     `push {r4,lr}` prologue (measured 30-byte candidate); reading through
//     the cursor pointer both times keeps every value in r0-r3.
//   * the byte pointer must be DERIVED FROM the cursor pointer, not from the
//     symbol alongside it. Loading the symbol into a `u8 *base` and then
//     casting `base` to the cursor pointer makes agbcc materialise both with
//     an `adds r3,r2,#0` copy and a shorter body (measured 2/24).
//   * the cursor bump lands in r1 (`adds r1,r0,r1 / strh r1,[r3]`), reusing the
//     register the `v` argument arrived in. Unpinned, agbcc accumulates into
//     the register holding the `ldrh` result and emits `adds r0,r0,r1 /
//     strh r0,[r3]` (measured 19/24, first difference at +0x0C). The
//     `__asm__("r1")` pin on the sum is LOAD-BEARING.
//   * `v` is WORD-width. As a `u8` parameter agbcc emits a `lsls/lsrs`
//     narrowing prologue the ROM does not have.
void SaveAllocWriteByte(u32 off, u32 v) {
    volatile u16 *cur = (volatile u16 *)SaveBumpBase;
    volatile u8 *p;
    register u32 s __asm__("r1");
    // Repeated inside the body, like SaveAllocReadByte below: the per-body
    // splice extracts only this function's brace-matched body, so the
    // file-scope definition is absent from the spliced section and the
    // reference would go undefined at link time.
    __asm__(".globl SaveBumpBase\nSaveBumpBase = 0x030002D8\n");
    p = (volatile u8 *)cur + 4;
    *(volatile u8 *)(p + off) = (u8)*cur;
    s = *cur + v;
    *cur = (u16)s;
}
void _08005768(u32 a, u32 b) __attribute__((alias("SaveAllocWriteByte")));

u8 SaveAllocReadByte(u32 off) {
    // Declared here, not at file scope: the per-body splice extracts only this
    // function's brace-matched body, so a file-scope __asm__ defining the
    // symbol is absent from the spliced section and the reference goes
    // undefined at link time.
    __asm__(".globl SaveBumpBase\nSaveBumpBase = 0x030002D8\n");
    volatile u8 *base = (volatile u8 *)SaveBumpBase;
    base += 4;
    return base[off];
}
u8 _08005780(u32 o) __attribute__((alias("SaveAllocReadByte")));

// 0x08005790 — free the top of the save heap: `ldr r1,=0x030002d8 /
// adds r1,#68 / ldrh r2,[r1] / subs r0,r2,r0 / strh r0,[r1] / ldrh r0,[r1]`.
// The block base is the POOL WORD here, not the folded field address: see
// include/gtadv/ram_blocks.h for why the symbol is what reproduces the `adds`.
int SaveAllocFreeHigh(u32 size) {
    // A `register u32 b __asm__("r1")` pin on the base was tried FIRST and
    // does move it -- 14/20 -> 18/20 -- but it is NOT load-bearing: with the
    // source below, deleting the pin reproduces 20/20 and a byte-identical
    // object. The pin is therefore NOT kept; it is recorded here only so the
    // next reader does not re-run it as an unattempted lever.
//
    // What actually closes it is the accumulation form. The prior
    // `*p = (u16)(*p - (u16)size)` reads as a pure store, so agbcc coalesces
    // the subtraction result back into the halfword's own register and
    // allocates the base to r2: `subs r2,r2,r0 / strh r2,[r2]`. The ROM's
    // `subs r0,r2,r0 / strh r0,[r1]` is the shape of an accumulate-INTO-THE-
    // PARAMETER: the difference lives in r0, and r0 is the incoming argument
    // slot. Writing `size = *p - size;` states that, and agbcc's LAST-use
    // numbering then lands the base in r1 on its own. The operand order
    // matters too -- the loaded halfword is the MINUEND (`*p - size`), not
    // the subtrahend.
    u32 b;
    volatile u16 *p;
    RAM_DEF_030002D8;
    // `b` is a u32, not a pointer: converting the symbol address to an integer
    // first is what stops agbcc folding the +68 into the pool word's relocation
    // addend (`.word RamBlock_030002D8+68`), which is 4 bytes shorter and has no
    // `adds`. Measured: pointer arithmetic 1/20, integer base 20/20.
    b = (u32)(uintptr_t)RAM_SYM_030002D8;
    p = (volatile u16 *)(b + 68);
    // The ROM's `subs r0,r2,r0` writes the difference back into the INCOMING
    // PARAMETER register and only then stores it, so the source accumulates
    // into `size` and truncates on the store.
    //...and the ROM's operand order is `r2 - r0`, i.e. the loaded halfword is
    // the MINUEND, not the subtrahend.
    size = *p - size;
    *p = (u16)size;
    return *p; /* asm sub_08005790 reloads + returns the new cursor */
}
int _08005790(u32 s) __attribute__((alias("SaveAllocFreeHigh")));

void SaveAllocInit(u32 a, u32 b) {
    u32 v = 0;
    volatile u32 *dma = (volatile u32 *)0x040000D4;
    dma[0] = (u32)&v;
    dma[1] = 0x06010000u + (a << 5);
    dma[2] = (b << 3) | (133u << 24);
    (void)dma[2];
}
void _080057A4(u32 a, u32 b) __attribute__((alias("SaveAllocInit")));
// The closure binds `sub_080057A4` at 0x080057a4, not the `_` form; a promoted
// caller (0x08026948 / 0x080269AC in src/sprite_obj_263e0.c) must use that
// spelling, and promotion rule 1 requires the exported twin be defined in C.
void sub_080057A4(u32 a, u32 b) __attribute__((alias("SaveAllocInit")));

// ----------------------------------------------------------------------------
// Slot descriptor appender (asm/save_desc.s)
// Control at 0x03000320: {count, devType, cursor, slots[]}
// ROM (84 B, one pool 0x03000320):
//   slot = ctrl+12+idx*8; slot[0]=cursor; slot[1]=byteSize;
//   inc = byteSize+4;
//   if (dev>2 || dev<1) goto div; else { inc+=7; t=inc; if (t<0) t=byteSize+18;
//     inc=(t>>3)<<3; }
// div: if (inc<0) inc+=7; inc>>=3; cursor+=inc; count++; return idx&0xFF.
// The two comparisons are SIGNED bgt/blt (dev in [1,2]); (dev-1)<=1u would be
// unsigned bhi. The final divide is asrs ALONE (sectors); the first round is
// asrs+lsls. Both biases are explicit adds.
// ----------------------------------------------------------------------------
u32 SaveDescAppend(u32 byteSize) {
    __asm__(".globl SaveCtrlBase\nSaveCtrlBase = 0x03000320");
    extern u8 SaveCtrlBase[];
    u32 bs = byteSize;
    register volatile u32 *ctrl __asm__("r2") = (volatile u32 *)SaveCtrlBase;
    u32 idx = ctrl[0];
    u32 off = idx * 8;
    volatile u8 *b = (volatile u8 *)ctrl;
    b += 12;
    off += (u32)(uintptr_t)b;
    register volatile u32 *slot __asm__("r0") = (volatile u32 *)(uintptr_t)off;
    u32 inc;
    u32 dev;
    slot[0] = ctrl[2];
    slot[1] = bs;
    inc = byteSize + 4;
    dev = ctrl[1];
    if ((s32)dev > 2)
        goto div;
    // Retain the ROM's signed `cmp #1; blt`; agbcc folds `< 1` into `<= 0`.
    __asm__ volatile("cmp %0, #1\n\tblt 1f"
                     : : "r" ((s32)dev) : "cc", "memory");
    inc += 7;
    {
        u32 t = inc;
        if ((s32)t < 0)
            t = byteSize + 18;
        inc = (u32)((s32)t >> 3);
        inc <<= 3;
    }
div:
    __asm__("1:");
    if ((s32)inc < 0)
        inc += 7;
    inc = (u32)((s32)inc >> 3);
    ctrl[2] += inc;
    ctrl[0] += 1;
    return (idx << 24) >> 24;
}
u32 _0800580C(u32 s) __attribute__((alias("SaveDescAppend")));

// ----------------------------------------------------------------------------
// Checksum + sector I/O (asm/save_checksum.s)
// The ROM's preheader order is: seed constant, `p = buf`, `i = 0`,
// `words = byteSize >> 2`. Declaring the pointer and the loop counter before
// `words` (and walking with `*p++` rather than `p[i]`) is what reproduces it;
// with `words` declared first agbcc sinks the pointer copy into the loop
// preheader and the two setup instructions swap.
u32 SaveChecksum(const void *buf, u32 byteSize) {
    u32 seed = SAVE_CHECKSUM_SEED;
    const u32 *p = (const u32 *)buf;
    u32 i = 0;
    u32 words = byteSize >> 2;
    for (; i < words; i++) seed += *p++;
    return seed;
}
u32 _08005860(const void *b, u32 s) __attribute__((alias("SaveChecksum")));
u32 sub_08005860(const void *b, u32 s) __attribute__((alias("SaveChecksum")));

// ----------------------------------------------------------------------------
// Sector I/O (asm/save_checksum.s, sub_08005884 / sub_080058D0)
// Both bodies are VOID in the ROM (epilogue `pop {r0}; bx r0` destroys r0),
// with IRQ suspend/resume (029D8/02A68 ... 02A0C) around a countdown loop.
// The sector and return values are truncated to u16 at each call
// (`lsls/lsrs #16`); the panic path reuses the truncated code in r1.
// ----------------------------------------------------------------------------
void SaveReadSectors(u32 firstSector, void *dst, u32 byteSize) {
    u32 sec = firstSector;
    u8 *out = (u8 *)dst;
    u32 t = byteSize + 7;
    u32 n;
    if ((s32)t < 0)
        t += 7;
    n = (u32)((s32)t >> 3);
    sub_080029D8();
    sub_08002A68();
    if ((s32)n <= 0)
        goto done;
    do {
        int rc = sub_0802DBA4((u16)sec, out);
        if ((u16)rc != 0)
            sub_0800295C(0x0805BAB0u, (u32)(u16)rc);
        sec += 1;
        out += 8;
        n -= 1;
    } while ((s32)n != 0);
done:
    SAVE_CALL_IRQ_RESTORE();
}
void _08005884(u32 a, void *b, u32 c) __attribute__((alias("SaveReadSectors")));
void sub_08005884(u32 a, void *b, u32 c) __attribute__((alias("SaveReadSectors")));

// ----------------------------------------------------------------------------
// Sector write (asm/save_checksum.s sub_080058D0, 184 B, 5 pools).
// NOTE the parameter ORDER is (src, sector, size) — swapped vs the read side
// (sector, dst, size). The slot caller (save_slot_api.s:41-44) sets r0=src
// (0x02000000), r1=sector before the bl, and the body advances r5+=8 (src
// bytes) with r8+=1 (sector). A (sector, src) declaration swaps the two
// increments and can never match.
// The body is VOID (pop {r0} destroys r0). Sector/return values truncate to
// u16 per call; the panic second arg reuses the truncated code in r1. r6
// first holds IME base 0x04000208, then the remaining count (disjoint ranges,
// separate blocks — same r6-reuse idiom as race_scene_b2 C27C).
// ----------------------------------------------------------------------------
void SaveWriteSectors(const void *src, u32 sector, u32 byteSize) {
    __asm__(".globl SaveWrIME\nSaveWrIME = 0x04000208");
    __asm__(".globl SaveWrIE\nSaveWrIE = 0x04000200");
    __asm__(".globl SaveWrP1\nSaveWrP1 = 0x0805BABC");
    __asm__(".globl SaveWrP2\nSaveWrP2 = 0x0805BAD0");
    __asm__(".globl SaveWrP3\nSaveWrP3 = 0x0805BAE0");
    extern u8 SaveWrIME[];
    extern u8 SaveWrIE[];
    extern u8 SaveWrP1[];
    extern u8 SaveWrP2[];
    extern u8 SaveWrP3[];
    register u8 *p5 __asm__("r5") = (u8 *)src;
    register u32 s8 __asm__("r8") = sector;
    register u32 n __asm__("r4") = (byteSize + 7) >> 3;
    u32 retries = 0;
    *(volatile u16 *)SaveWrIME = 0;
    sub_080029D8();
    sub_08002A68();
    {
        u32 t = (u32)sub_08002AF4(9);
        int rc0 = sub_0802DA20(0, t);
        if ((u16)rc0 != 0)
            sub_0800295C((u32)SaveWrP1, (u32)(u16)rc0);
    }
    *(volatile u16 *)SaveWrIE |= 8;
    *(volatile u16 *)SaveWrIME = 1;
    if (n == 0)
        goto done;
    {
        register u32 rem __asm__("r6") = n;
        do {
            u32 sec16 = (u16)s8;
            int w = sub_0802DC54(sec16, (u32)p5);
            if ((u16)w != 0) {
                retries += 1;
                if ((s32)retries <= 10)
                    goto retry;
                sub_0800295C((u32)SaveWrP2, (u32)(u16)w);
            }
            {
                int v = sub_0802DD30(sec16, (u32)p5);
                if ((u16)v != 0) {
                    retries += 1;
                    if ((s32)retries <= 10)
                        goto retry;
                    sub_0800295C((u32)SaveWrP3, (u32)(u16)v);
                }
            }
            p5 += 8;
            s8 += 1;
            rem -= 1;
        retry:
            if ((s32)rem == 0)
                break;
        } while (1);
    }
done:
    sub_08002A0C();
}
void _080058D0(const void *a, u32 b, u32 c) __attribute__((alias("SaveWriteSectors")));
void sub_080058D0(const void *a, u32 b, u32 c) __attribute__((alias("SaveWriteSectors")));

// ----------------------------------------------------------------------------
// Slot API (asm/save_slot_api.s + code_5988.s)
// Slot index via shifts ((idx<<24)>>21 = (idx&0xFF)*8); table base 0x0300032C
// via pool. Staging through CpuSet (sub_0802D974), not CpuFastSet. DevType via
// double-indirect with SIGNED bgt/blt (1..2 present). Write call order is
// (src, sector, size+4); read call order is (sector, dst, size+4). Both sector
// bodies are void so their results are ignored.
// ----------------------------------------------------------------------------
int SaveSlotSave(u32 slotIdx, const void *src) {
    __asm__(".globl SaveSlotTbl\nSaveSlotTbl = 0x0300032C");
    __asm__(".globl SaveDst4\nSaveDst4 = 0x02000004");
    __asm__(".globl SaveDevPtr\nSaveDevPtr = 0x030003AC");
    extern u8 SaveSlotTbl[];
    extern u8 SaveDst4[];
    extern u8 SaveDevPtr[];
    volatile u8 *e = SaveSlotTbl + ((slotIdx << 24) >> 21);
    u32 cksum = SaveChecksum(src, *(volatile u32 *)(e + 4));
    u32 stk = cksum;
    u8 *stage;
    __asm__ volatile("" : "+r" (e));
    stage = (u8 *)0x02000000u;
    sub_0802D974(&stk, (void *)stage, 2);
    {
        void *d = (void *)SaveDst4;
        u32 sz = *(volatile u32 *)(e + 4);
        u32 words = ((sz + (sz >> 31)) << 10) >> 11;
        sub_0802D974(src, d, words);
    }
    {
        u32 devType = *(volatile u32 *)(*(volatile u32 *)SaveDevPtr + 4);
        if ((s32)devType > 2)
            goto done;
        // Keep the ROM's `cmp #1; blt` form; agbcc canonicalizes `< 1` into
        // `cmp #0; ble`, which is equivalent but changes two bytes.
        __asm__ volatile("cmp %0, #1\n\tblt 1f"
                         : : "r" ((s32)devType) : "cc", "memory");
        sub_080058D0((void *)stage, *(volatile u32 *)e, *(volatile u32 *)(e + 4) + 4);
    }
    __asm__("1:");
done:
    return 1;
}
int _080059F0(u32 a, const void *b) __attribute__((alias("SaveSlotSave")));
int sub_080059F0(u32 a, const void *b) __attribute__((alias("SaveSlotSave")));

int SaveSlotLoad(u32 slotIdx, void *dst) {
    __asm__(".globl SaveSlotTbl\nSaveSlotTbl = 0x0300032C");
    __asm__(".globl SaveDevPtr\nSaveDevPtr = 0x030003AC");
    __asm__(".globl SaveStage4\nSaveStage4 = 0x02000004");
    extern u8 SaveSlotTbl[];
    extern u8 SaveDevPtr[];
    extern u8 SaveStage4[];
    volatile u8 *e = SaveSlotTbl + ((slotIdx << 24) >> 21);
    u32 devType = *(volatile u32 *)(*(volatile u32 *)SaveDevPtr + 4);
    if ((s32)devType > 2)
        goto verify;
    // Preserve the ROM's `cmp #1; blt` encoding; agbcc lowers `< 1` as
    // `cmp #0; ble`, which is equivalent but changes the two instruction bytes.
    __asm__ volatile("cmp %0, #1\n\tblt 1f"
                     : : "r" ((s32)devType) : "cc", "memory");
    {
        u32 sec = *(volatile u32 *)e;
        u8 *d = (u8 *)0x02000000u;
        sub_08005884(sec, (void *)d, *(volatile u32 *)(e + 4) + 4);
    }
verify:
    __asm__("1:");
    {
        register u8 *stage4 __asm__("r5") = SaveStage4;
        register u32 sz __asm__("r1") = *(volatile u32 *)(e + 4);
        u32 calc = sub_08005860(stage4, sz);
        u8 *stage0 = (u8 *)0x02000000u;
        u32 stored = *(volatile u32 *)stage0;
        if (calc != stored)
            return 0;
        {
            register u32 copySz __asm__("r2") = *(volatile u32 *)(e + 4);
            u32 words = ((copySz + (copySz >> 31)) << 10) >> 11;
            sub_0802D974(stage4, dst, words);
        }
        return 1;
    }
}
int _08005988(u32 a, void *b) __attribute__((alias("SaveSlotLoad")));
//. asm/code_5988.s declares BOTH spellings at 0x08005988
// (`.type sub_08005988` / `sub_08005988:` then `_08005988:`), and the
// promotion screen requires the C to define the `sub_` twin the asm closure
// actually calls. Without it the screen reports
// `SaveSlotLoad: closure defines _08005988/sub_08005988 (rename)` and refuses
// the body. One hop, aliasing the real body -- never aliasing an alias.
int sub_08005988(u32 a, void *b) __attribute__((alias("SaveSlotLoad")));

// ----------------------------------------------------------------------------
// Timer queue (asm/save_timer_queue.s + save_delay.s)
void SaveTimerQueueInit(void) {
    *(volatile u32 *)TIMER_QUEUE_PTR_ADDR = TIMER_QUEUE_ADDR;
}
void _08005A58(void) __attribute__((alias("SaveTimerQueueInit")));
void SaveSystemInit(void) __attribute__((alias("SaveTimerQueueInit")));
void sub_08005A58(void) __attribute__((alias("SaveTimerQueueInit")));

void SaveTimerQueueZero(void) {
    extern void sub_0802D974(const void *src, void *dst, u32 ctrl);
    // The queue base is reached through the pointer cell at TIMER_QUEUE_PTR_ADDR,
    // not through the TIMER_QUEUE_ADDR literal: the ROM emits `ldr r0,[pc];
    // ldr r1,[r0]` so only ONE pool word (0x030003D4) precedes the control word.
    u32 zero = 0;
    sub_0802D974(&zero, (void *)*(volatile u32 *)TIMER_QUEUE_PTR_ADDR, 0x05000009u);
}
void _08005A68(void) __attribute__((alias("SaveTimerQueueZero")));
void SaveTick1(void) __attribute__((alias("SaveTimerQueueZero")));

bool SaveTimerQueueIsZero(void) {
    // The POINTER load stays volatile (it is the hardware cell read) but the
    // sub-word read must NOT be: a `volatile s16` lvalue makes agbcc
    // re-materialise the value (`ldrh` + `lsls`/`asrs`), while the ROM has the
    // hardware's indexed `movs r2,#32; ldrsh r0,[r0,r2]`.
    // The accumulator is declared FIRST and initialised to 0 before the loads:
    // that is what puts the ROM's leading `movs r1,#0` ahead of the pointer
    // reads instead of between them.
    // NOTE: the ROM body is frameless (`bx lr`, no push/pop) yet contains a
    // conditional branch. agbcc frames every branching function at -O2, so the
    // 4-byte push/pop pair is a measured compiler limit, not a body defect.
    int r = 0;
    s16 *cnt = (s16 *)(*(volatile u32 *)TIMER_QUEUE_PTR_ADDR + 32);
    if (*cnt == 0) r = 1;
    return r;
}
bool _08005A8C(void) __attribute__((alias("SaveTimerQueueIsZero")));

void SaveTimerQueueArm(u16 time, u8 id) {
    // Facts measured along the way, kept so they are not re-run:
    //  * BOTH parameters are narrowed in the prologue -- `lsls r0,#16 /
    //    lsrs r4,r0,#16` for `time`, `lsls r1,#24 / lsrs r1,#24` for `id`.
    //    Declaring the parameters at word width does NOT apply; widening them
    //    would delete the pairs the ROM has.
    //  * Both zero-extends must COMPLETE before the `ldr r0,[pc]` cell load.
    //    Routing `id` through a named local (`u8 idv = id;`) defers its
    //    `lsrs` until after that load and scores 48/64; using `id` directly
    //    keeps the pair in the prologue and scores 51/64.
    //  * `i <= 3` (not `i < 4`) is required: `i < 4` gives an unsigned `bls`
    //    where the ROM has a signed `ble`.
    //  * Other measured dead ends: `unsigned i` 50/64; `idv` declared before
    //    the pointer locals 51/64 (unchanged); an explicit `u32 qp` temp for
    //    the queue pointer 51/64 (unchanged); reusing `cell` for the counter
    //    reload DROPS to 48/64 (agbcc then keeps `cell` live in r0 and
    //    reloads the literal), so the second access re-loads the literal.
    volatile u32 *cell = (volatile u32 *)TIMER_QUEUE_PTR_ADDR;
    volatile u8 *q = (volatile u8 *)(*(volatile u32 *)cell);
    int i = 0;
    u8 one = 1;
    volatile u16 *c;
    do {
        if (*q == 0) {
            *q = one;
            *(volatile u16 *)(q + 2) = time;
            q[1] = id;
            c = (volatile u16 *)(*(volatile u32 *)TIMER_QUEUE_PTR_ADDR + 32);
            *c = (u16)(*c + 1);
            break;
        }
        i++;
        q += 4;
    } while (i <= 3);
}

// The body is 64 bytes (0x08005AA4..0x08005AE3) and its section closes with
// gas's 2-byte `nop` filler 0x46c0 at +0x3E; the ROM pads `00 00` there. A
// file-scope `.align 2, 0` lands after the body's `.size` -- still inside the
// body's own `-ffunction-sections` section -- and pads with the explicit `0`.
__asm__(".align 2, 0");

void _08005AA4(u16 a, u8 b) __attribute__((alias("SaveTimerQueueArm")));

void SaveTimerQueueDrain(void) {
    register int fired __asm__("r3") = 0;
    register volatile u32 *qptr __asm__("r0") = (volatile u32 *)TIMER_QUEUE_PTR_ADDR;
    register volatile u32 *savedQptr __asm__("r6");
    register volatile u8 *cursor __asm__("r2") = (volatile u8 *)(uintptr_t)*qptr;
    register volatile u8 *dst __asm__("r5") = cursor + 16;
    register int i __asm__("r4");
    savedQptr = qptr;
    __asm__("" : "+r"(savedQptr));
    /* The cursor walks q[0] to q[12] while i counts down, preserving fire order. */
    i = 3;
    do {
        if (cursor[0] != 0) {
            register u32 delay __asm__("r0") = (u32)cursor[1];
            register u32 test __asm__("r1") = delay;
            if (test == 0) {
                cursor[0] = (u8)test;
                *(volatile u32 *)dst = *(volatile u32 *)cursor;
                dst += 4;
                fired++;
            } else {
                cursor[1] = (u8)(delay - 1u);
            }
        }
        i--;
        cursor += 4;
    } while (i >= 0);
    {
        register volatile u32 *q __asm__("r0") = (volatile u32 *)(uintptr_t)*savedQptr;
        register u16 count __asm__("r2");
        register u16 newCount __asm__("r1");
        *((volatile u16 *)((volatile u8 *)q + 34)) = (u16)fired;
        count = *((volatile u16 *)((volatile u8 *)q + 32));
        newCount = count - (u16)fired;
        *((volatile u16 *)((volatile u8 *)q + 32)) = newCount;
    }
    if (fired > 0) sub_08004D4C(11,0,0);
}
/* The ROM splice includes this zero halfword after the return instruction. */
__asm__(".pushsection .text.SaveTimerQueueDrain,\"ax\",%progbits\n.short 0\n.popsection");
void _08005AE4(void) __attribute__((alias("SaveTimerQueueDrain")));
void TimerListTick(void) __attribute__((alias("SaveTimerQueueDrain")));
void sub_08005AE4(void) __attribute__((alias("SaveTimerQueueDrain")));

// ----------------------------------------------------------------------------
// Trigger accessors (asm/save_trigger_accessors.s)
// The ROM addresses the two trig tables through a *symbol* rather than a
// folded integer constant: the literal pool holds the table address as its
// first word and the 0x0FFE mask as its second, and the table `ldr` is issued
// before the `ands`. agbcc only reproduces that order when the base is a
// SYMBOL_REF leaf; with an integer literal in `tbl + (arg & 0x0FFE)` the
// commutative-operand fold always expands the mask first, giving
// `ldr r1,[mask]; ands; ldr r0,[tbl]; adds` — one byte different at +0 and
// the pool words transposed. The absolute symbols below put the two table
// addresses in the pool without allocating anything in the slice link.
__asm__(".globl SaveTrigTblA\nSaveTrigTblA = 0x0805CAF0\n");
__asm__(".globl SaveTrigTblB\nSaveTrigTblB = 0x0805BAF0\n");
extern const s16 SaveTrigTblA[];
extern const s16 SaveTrigTblB[];
s16 SaveTriggerA(u32 arg) {
    // The absolute symbol must be declared HERE, not at file scope: the
    // per-body splice extracts only this function's brace-matched body,
    // so a file-scope __asm__ that defines the symbol is simply absent
    // from the spliced section and the reference becomes undefined.
    __asm__(".globl SaveTrigTblA\nSaveTrigTblA = 0x0805CAF0\n");
    return SaveTrigTblA[(arg & 0x0FFEu) >> 1];
}
s16 _08005F2C(u32 a) __attribute__((alias("SaveTriggerA")));
s16 sub_08005F2C(u32 a) __attribute__((alias("SaveTriggerA")));

s16 SaveTriggerB(u32 arg) {
    // The absolute symbol must be declared HERE, not at file scope: the
    // per-body splice extracts only this function's brace-matched body,
    // so a file-scope __asm__ that defines the symbol is simply absent
    // from the spliced section and the reference becomes undefined.
    __asm__(".globl SaveTrigTblB\nSaveTrigTblB = 0x0805BAF0\n");
    return SaveTrigTblB[(arg & 0x0FFEu) >> 1];
}
s16 _08005F44(u32 a) __attribute__((alias("SaveTriggerB")));
s16 sub_08005F44(u32 a) __attribute__((alias("SaveTriggerB")));

s16 SaveTriggerC(u32 arg) {
    // The absolute symbol must be declared HERE, not at file scope: the
    // per-body splice extracts only this function's brace-matched body,
    // so a file-scope __asm__ that defines the symbol is simply absent
    // from the spliced section and the reference becomes undefined.
    __asm__(".globl SaveTrigTblA\nSaveTrigTblA = 0x0805CAF0\n");
    return SaveTrigTblA[(arg & 0x0FFEu) >> 1];
}
s16 _08005F5C(u32 a) __attribute__((alias("SaveTriggerC")));
s16 sub_08005F5C(u32 a) __attribute__((alias("SaveTriggerC")));

s16 SaveTriggerD(u32 arg) {
    // The absolute symbol must be declared HERE, not at file scope: the
    // per-body splice extracts only this function's brace-matched body,
    // so a file-scope __asm__ that defines the symbol is simply absent
    // from the spliced section and the reference becomes undefined.
    __asm__(".globl SaveTrigTblB\nSaveTrigTblB = 0x0805BAF0\n");
    return SaveTrigTblB[(arg & 0x0FFEu) >> 1];
}
s16 _08005F74(u32 a) __attribute__((alias("SaveTriggerD")));
s16 sub_08005F74(u32 a) __attribute__((alias("SaveTriggerD")));

// ----------------------------------------------------------------------------
// Saveblock #1 bitfield + grid (asm/saveblock.s)
__attribute__((naked)) void SaveBlock1SetBit(u32 bit, int set) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, lr}\n"
        "adds r5, r0, #0\n"
        "cmp r1, #0\n"
        "beq.n 2f\n"
        "ldr r3, 1f\n"
        "asrs r4, r5, #3\n"
        "adds r2, r4, r3\n"
        "ldr r1, 3f\n"
        "movs r0, #7\n"
        "ands r0, r5\n"
        "adds r0, r0, r1\n"
        "ldrb r1, [r0, #0]\n"
        "ldrb r2, [r2, #0]\n"
        "orrs r1, r2\n"
        "b.n 4f\n"
        ".align 2, 0\n"
        "1: .word 0x03000628\n"
        "3: .word 0x080c4768\n"
        "2:\n"
        "ldr r3, 5f\n"
        "asrs r4, r5, #3\n"
        "adds r2, r4, r3\n"
        "ldr r1, 6f\n"
        "movs r0, #7\n"
        "ands r0, r5\n"
        "adds r0, r0, r1\n"
        "ldrb r1, [r2, #0]\n"
        "ldrb r0, [r0, #0]\n"
        "bics r1, r0\n"
        "4:\n"
        "adds r0, r4, r3\n"
        "strb r1, [r0, #0]\n"
        "pop {r4, r5}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        "5: .word 0x03000628\n"
        "6: .word 0x080c4768\n"
        ".syntax divided\n"
    );
}
void SaveBlock1SetBit_alias(u32 b, int s) __attribute__((alias("SaveBlock1SetBit")));
void _0802466C(u32 b, int s) __attribute__((alias("SaveBlock1SetBit")));

__attribute__((naked)) u32 SaveBlock1TestBit(u32 bit) {
    __asm__ volatile (
        ".syntax unified\n"
        "ldr\tr1, _080246D4\n"
        "asrs\tr2, r0, #3\n"
        "adds\tr2, r2, r1\n"
        "ldr\tr3, _080246D8\n"
        "movs\tr1, #7\n"
        "ands\tr1, r0\n"
        "adds\tr1, r1, r3\n"
        "ldrb\tr0, [r2]\n"
        "ldrb\tr1, [r1]\n"
        "ands\tr0, r1\n"
        "bx\tlr\n"
        "movs\tr0, r0\n"
        ".align 2, 0\n"
        "_080246D4: .word 0x03000628\n"
        "_080246D8: .word 0x080C4768\n"
        ".syntax divided\n"
    );
}
u32 _080246BC(u32 b) __attribute__((alias("SaveBlock1TestBit")));

__attribute__((naked)) u16 SaveBlock1GetHWord(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "ldr\tr0, _080246E4\n"
        "ldrh\tr0, [r0, #12]\n"
        "bx\tlr\n"
        "movs\tr0, r0\n"
        ".align 2, 0\n"
        "_080246E4: .word 0x03000628\n"
        ".syntax divided\n"
    );
}
u16 _080246DC(void) __attribute__((alias("SaveBlock1GetHWord")));

__attribute__((naked)) void SaveBlock1SetHWord(u16 v) {
    __asm__ volatile (
        ".syntax unified\n"
        "ldr\tr1, _080246F0\n"
        "strh\tr0, [r1, #12]\n"
        "bx\tlr\n"
        "movs\tr0, r0\n"
        ".align 2, 0\n"
        "_080246F0: .word 0x03000628\n"
        ".syntax divided\n"
    );
}
void _080246E8(u16 v) __attribute__((alias("SaveBlock1SetHWord")));

void *SaveBlock1GridPtr(u32 row, u32 col) {
    return (void *)(SAVE_BLOCK1_BUF_ADDR + 16 + row*60 + col*12);
}
void *_080246F4(u32 r, u32 c) __attribute__((alias("SaveBlock1GridPtr")));

__attribute__((naked)) void SaveBlock1Init(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "sub sp, #16\n"
        "movs r4, #0\n"
        "movs r0, #1\n"
        "bl 0x080057D0\n"
        "str r4, [sp]\n"
        "ldr r1, .L24568_block2\n"
        "ldr r2, .L24568_mode127\n"
        "mov r0, sp\n"
        "bl 0x0802D974\n"
        "movs r0, #222\n"
        "lsls r0, r0, #1\n"
        "bl 0x0800580C\n"
        "ldr r1, .L24568_block2_idx\n"
        "strb r0, [r1, #0]\n"
        "str r4, [sp, #4]\n"
        "add r0, sp, #4\n"
        "ldr r1, .L24568_block1_idx\n"
        "ldr r2, .L24568_mode1\n"
        "bl 0x0802D974\n"
        "str r4, [sp, #8]\n"
        "add r0, sp, #8\n"
        "ldr r1, .L24568_block1_bits\n"
        "ldr r2, .L24568_mode4\n"
        "bl 0x0802D974\n"
        "str r4, [sp, #12]\n"
        "add r0, sp, #12\n"
        "ldr r1, .L24568_block1_body\n"
        "ldr r2, .L24568_mode_body\n"
        "bl 0x0802D974\n"
        "bl 0x0802446C\n"
        "bl 0x08024BD8\n"
        "ldr r0, .L24568_seed_dst\n"
        "ldr r1, .L24568_seed_src\n"
        "movs r2, #3\n"
        "bl 0x0800D95C\n"
        "movs r7, #0\n"
        "movs r5, #0\n"
        ".L24568_outer:\n"
        "movs r4, #0\n"
        "adds r6, r5, #1\n"
        ".L24568_inner:\n"
        "movs r0, #0\n"
        "adds r1, r5, #0\n"
        "adds r2, r4, #0\n"
        "bl 0x08025CF4\n"
        "cmp r0, #3\n"
        "bne .L24568_no_count\n"
        "adds r7, #1\n"
        ".L24568_no_count:\n"
        "adds r4, #1\n"
        "cmp r4, #10\n"
        "ble .L24568_inner\n"
        "adds r5, r6, #0\n"
        "cmp r5, #3\n"
        "ble .L24568_outer\n"
        "cmp r7, #43\n"
        "ble .L24568_after_audit\n"
        "ldr r0, .L24568_wa\n"
        "movs r1, #255\n"
        "lsls r1, r1, #4\n"
        "adds r0, r0, r1\n"
        "movs r1, #1\n"
        "strh r1, [r0, #0]\n"
        ".L24568_after_audit:\n"
        "bl 0x080241C8\n"
        "add sp, #16\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".L24568_block2: .word 0x030013D0\n"
        ".L24568_mode127: .word 0x0500007F\n"
        ".L24568_block2_idx: .word 0x030015CC\n"
        ".L24568_block1_idx: .word 0x03000620\n"
        ".L24568_mode1: .word 0x05000001\n"
        ".L24568_block1_bits: .word 0x03000628\n"
        ".L24568_mode4: .word 0x05000004\n"
        ".L24568_block1_body: .word 0x03000638\n"
        ".L24568_mode_body: .word 0x05000366\n"
        ".L24568_seed_dst: .word 0x03002808\n"
        ".L24568_seed_src: .word 0x0805FC8C\n"
        ".L24568_wa: .word 0x03001780\n"
        ".syntax divided\n"
    );
}
void _08024568(void) __attribute__((alias("SaveBlock1Init")));

int SaveBlock1Load(void) {
    u32 idx = *(volatile u8 *)SAVE_BLOCK1_IDX_ADDR;
    _08005988(idx, (void *)SAVE_BLOCK1_BUF_ADDR);
    return 1;
}
int _08024634(void) __attribute__((alias("SaveBlock1Load")));

// 0x08024650 — `push {lr}; ldr r0,[pc,#12]; ldrb r0,[r0]; ldr r1,[pc,#12];
// bl <slot-save>; pop {r0}; bx r0` — ends in `pop {r0}`, which DESTROYS r0, so
// there is no return value to read and the body is `void`. That is not a
// stylistic choice: agbcc's epilogue for a value-returning function is
// `pop {r1}; bx r1`, and the ROM's `pop {r0}; bx r0` is the form it emits when
// the return value is unused. The declaration in include/gtadv/save.h had to
// change in the same commit, because a lone `void` here is a hard agbcc
// "conflicting types" error and the C89 transform then skips the WHOLE TU.
//
// The sibling SaveBlock1Load at 0x08024634 is the opposite: the ROM ends that
// one with `movs r0,#1`, so it really does return a value and stays `int`.
void SaveBlock1Save(void) {
    u32 idx = *(volatile u8 *)SAVE_BLOCK1_IDX_ADDR;
    SAVE_CALL_SLOT_SAVE(idx, (void *)SAVE_BLOCK1_BUF_ADDR);
}
void _08024650(void) __attribute__((alias("SaveBlock1Save")));

// Pack/unpack helpers (width-preserving)
//
// 0x0802476C — "is the packed bit set at block2[0x2E + idx*2] & mask[k]".
// Three measured levers, all needed together (2/40 -> 36/40):
//
//  * The block base is a SYMBOL (RAM_DEF/ RAM_SYM_030013D0) plus a separate
//    `base += 0x2E`. The numeric spelling `SAVE_BLOCK2_BUF_ADDR + 0x2E` folds
//    into one pool word (0x030013FE) where the ROM holds 0x030013D0 and does
//    `adds r2,#46` -- the same fold trap include/gtadv/ram_blocks.h documents.
//  * The result is tested through a SIGNED 16-bit local, `s16 v`, not an
//    inline `(s16)(...) != 0`. Inline, agbcc proves the extension redundant and
//    drops it; through the local it emits the ROM's `lsls r1,#16 / asrs
//    r1,#16` before the `negs/orrs/lsrs` boolean (2/40 -> 11/40 -> 32/40).
//  * Each index is shifted in its OWN statement before being added to its
//    table pointer (`ioff = idx << 1; base += 0x2E; packed = base + ioff;`).
//    With the shift folded into the add the two `adds` and the `lsls` swap and
//    the body loses 4 more bytes (32/40 -> 36/40).
//
// OPEN, 4 bytes: the second window's pool load. The ROM is
// `ldr r2,[pc,#24]` (0x080CC1A8) THEN `lsls r1,r1,#1`; agbcc schedules the
// shift first and the load second, and the two instructions are otherwise
// identical. Six orderings were measured against it -- the load as a named
// `const u8 *mt` / `const u16 *mt` pointer, the mask as `mt[k]`, as
// `*(mt + (k<<1))`, the whole mask block moved before the packed block, and
// both shifts hoisted -- all land on the same 36/40 with only that pair
// swapped. This is the documented "reload pass orders the scale against the
// pool load" wall (cf. 0x080026180); do not re-run it without a new idea.
u32 SaveGarageMaskTest(u32 idx, u32 k) {
    extern u16 SaveMaskTbl[];
    __asm__(".globl SaveMaskTbl\nSaveMaskTbl = 0x080CC1A8");
    volatile u16 *packed;
    u16 mask;
    s16 v;
    u8 *base;
    u32 ioff;
    u32 koff;
    u16 *mt;
    RAM_DEF_030013D0;
    base = (u8 *)RAM_SYM_030013D0;
    ioff = idx << 1;
    base += 0x2E;
    packed = (volatile u16 *)(base + ioff);
    mt = SaveMaskTbl;
    koff = k << 1;
    mask = *(volatile u16 *)((u8 *)mt + koff);
    v = (s16)(*packed & mask);
    return (v != 0) ? 1 : 0;
}
u32 _0802476C(u32 a, u32 b) __attribute__((alias("SaveGarageMaskTest")));

// ROM 0x08024740: r1 indexes the PACKED halfword (`lsls r2,r1,#1` + base+0x2E)
// and r2 the mask table (`lsls r1,r3,#1` + 0x080CC1A8); the callers in
// SaveGaragePack pass r1=0, r2=slot. The mask is read twice and the packed
// word stored twice (clear, then set), so both are volatile accesses.
void SaveGarageMaskSet(u32 val, u32 idx, u32 k) {
    extern u16 SaveMaskTbl[];
    register volatile u16 *packed __asm__("r2");
    register volatile u16 *m __asm__("r1");
    register u8 *base __asm__("r0");
    u16 cur;
    __asm__(".globl SaveMaskTbl\nSaveMaskTbl = 0x080CC1A8");
    if (val == 1) {
        RAM_DEF_030013D0;
        base = (u8 *)RAM_SYM_030013D0;
        packed = (volatile u16 *)(idx * 2);
        base += 0x2E;
        packed = (volatile u16 *)((u8 *)packed + (uintptr_t)base);
        base = (u8 *)SaveMaskTbl;
        m = (volatile u16 *)(k * 2);
        m = (volatile u16 *)((u8 *)m + (uintptr_t)base);
        cur = *packed;
        cur &= ~*m;
        *packed = cur;
        cur |= *m;
        *packed = cur;
    }
}
void _08024740(u32 a, u32 b, u32 c) __attribute__((alias("SaveGarageMaskSet")));

__attribute__((naked)) void SaveGaragePack(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push\t{r4, r5, lr}\n"
        "ldr\tr2, _08024878\n"
        "ldr\tr1, _0802487C\n"
        "adds\tr0, r2, r1\n"
        "movs\tr3, #0\n"
        "ldrsh\tr1, [r0, r3]\n"
        "movs\tr0, #0\n"
        "ldr\tr4, _08024880\n"
        "strh\tr0, [r4, #46]\n"
        "lsls\tr0, r1, #1\n"
        "adds\tr0, r0, r1\n"
        "lsls\tr0, r0, #2\n"
        "adds\tr5, r0, r2\n"
        "adds\tr0, r5, #0\n"
        "adds\tr0, #48\n"
        "ldrb\tr0, [r0]\n"
        "lsls\tr0, r0, #24\n"
        "asrs\tr0, r0, #24\n"
        "movs\tr1, #0\n"
        "movs\tr2, #0\n"
        "bl 0x08024740\n"
        "adds\tr0, r5, #0\n"
        "adds\tr0, #53\n"
        "ldrb\tr0, [r0]\n"
        "lsls\tr0, r0, #24\n"
        "asrs\tr0, r0, #24\n"
        "movs\tr1, #0\n"
        "movs\tr2, #1\n"
        "bl 0x08024740\n"
        "adds\tr0, r5, #0\n"
        "adds\tr0, #51\n"
        "ldrb\tr0, [r0]\n"
        "lsls\tr0, r0, #24\n"
        "asrs\tr0, r0, #24\n"
        "movs\tr1, #0\n"
        "movs\tr2, #2\n"
        "bl 0x08024740\n"
        "adds\tr0, r5, #0\n"
        "adds\tr0, #55\n"
        "ldrb\tr0, [r0]\n"
        "lsls\tr0, r0, #24\n"
        "asrs\tr0, r0, #24\n"
        "movs\tr1, #0\n"
        "movs\tr2, #3\n"
        "bl 0x08024740\n"
        "adds\tr0, r5, #0\n"
        "adds\tr0, #56\n"
        "ldrb\tr0, [r0]\n"
        "lsls\tr0, r0, #24\n"
        "asrs\tr0, r0, #24\n"
        "movs\tr1, #0\n"
        "movs\tr2, #4\n"
        "bl 0x08024740\n"
        "adds\tr0, r5, #0\n"
        "adds\tr0, #57\n"
        "ldrb\tr0, [r0]\n"
        "lsls\tr0, r0, #24\n"
        "asrs\tr0, r0, #24\n"
        "movs\tr1, #0\n"
        "movs\tr2, #5\n"
        "bl 0x08024740\n"
        "ldr\tr2, _08024884\n"
        "ldrh\tr1, [r4, #46]\n"
        "ldrh\tr0, [r2, #4]\n"
        "bics\tr1, r0\n"
        "strh\tr1, [r4, #46]\n"
        "adds\tr0, r5, #0\n"
        "adds\tr0, #50\n"
        "ldrb\tr0, [r0]\n"
        "lsls\tr0, r0, #24\n"
        "asrs\tr0, r0, #24\n"
        "lsls\tr0, r0, #4\n"
        "orrs\tr0, r1\n"
        "ldrh\tr1, [r2, #6]\n"
        "bics\tr0, r1\n"
        "strh\tr0, [r4, #46]\n"
        "adds\tr1, r5, #0\n"
        "adds\tr1, #54\n"
        "ldrb\tr1, [r1]\n"
        "lsls\tr1, r1, #24\n"
        "asrs\tr1, r1, #24\n"
        "lsls\tr1, r1, #6\n"
        "orrs\tr1, r0\n"
        "ldrh\tr2, [r2, #8]\n"
        "bics\tr1, r2\n"
        "strh\tr1, [r4, #46]\n"
        "adds\tr0, r5, #0\n"
        "adds\tr0, #52\n"
        "ldrb\tr0, [r0]\n"
        "lsls\tr0, r0, #24\n"
        "asrs\tr0, r0, #24\n"
        "lsls\tr0, r0, #8\n"
        "orrs\tr0, r1\n"
        "ldr\tr1, _08024888\n"
        "ldrh\tr1, [r1]\n"
        "bics\tr0, r1\n"
        "strh\tr0, [r4, #46]\n"
        "adds\tr1, r5, #0\n"
        "adds\tr1, #49\n"
        "ldrb\tr1, [r1]\n"
        "lsls\tr1, r1, #24\n"
        "asrs\tr1, r1, #24\n"
        "orrs\tr0, r1\n"
        "strh\tr0, [r4, #46]\n"
        "pop\t{r4, r5}\n"
        "pop\t{r0}\n"
        "bx\tr0\n"
        "movs\tr0, r0\n"
        ".align 2, 0\n"
        "_08024878: .word 0x03001780\n"
        "_0802487C: .word 0x00000574\n"
        "_08024880: .word 0x030013D0\n"
        "_08024884: .word 0x080CC1C8\n"
        "_08024888: .word 0x080CC1D8\n"
        ".syntax divided\n"
    );
}
void _08024794(void) __attribute__((alias("SaveGaragePack")));

__attribute__((naked)) void SaveGarageUnpack(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, lr}\n"
        "ldr r5, .L2488C_base\n"
        "ldr r1, .L2488C_idx_off\n"
        "adds r0, r5, r1\n"
        "movs r3, #0\n"
        "ldrsh r4, [r0, r3]\n"
        "movs r0, #0\n"
        "movs r1, #0\n"
        "bl 0x0802476C\n"
        "lsls r1, r4, #1\n"
        "adds r1, r1, r4\n"
        "lsls r1, r1, #2\n"
        "adds r4, r1, r5\n"
        "adds r1, r4, #0\n"
        "adds r1, #48\n"
        "strb r0, [r1]\n"
        "movs r0, #0\n"
        "movs r1, #1\n"
        "bl 0x0802476C\n"
        "adds r1, r4, #0\n"
        "adds r1, #53\n"
        "strb r0, [r1]\n"
        "movs r0, #0\n"
        "movs r1, #2\n"
        "bl 0x0802476C\n"
        "adds r1, r4, #0\n"
        "adds r1, #51\n"
        "strb r0, [r1]\n"
        "movs r0, #0\n"
        "movs r1, #3\n"
        "bl 0x0802476C\n"
        "adds r1, r4, #0\n"
        "adds r1, #55\n"
        "strb r0, [r1]\n"
        "movs r0, #0\n"
        "movs r1, #4\n"
        "bl 0x0802476C\n"
        "adds r1, r4, #0\n"
        "adds r1, #56\n"
        "strb r0, [r1]\n"
        "movs r0, #0\n"
        "movs r1, #5\n"
        "bl 0x0802476C\n"
        "adds r1, r4, #0\n"
        "adds r1, #57\n"
        "strb r0, [r1]\n"
        "ldr r2, .L2488C_block2\n"
        "ldr r1, .L2488C_masktbl\n"
        "ldrh r0, [r1, #4]\n"
        "ldrh r3, [r2, #46]\n"
        "ands r0, r3\n"
        "lsrs r0, r0, #4\n"
        "adds r3, r4, #0\n"
        "adds r3, #50\n"
        "strb r0, [r3]\n"
        "ldrh r0, [r1, #6]\n"
        "ldrh r3, [r2, #46]\n"
        "ands r0, r3\n"
        "lsrs r0, r0, #6\n"
        "adds r3, r4, #0\n"
        "adds r3, #54\n"
        "strb r0, [r3]\n"
        "ldrh r0, [r1, #8]\n"
        "ldrh r3, [r2, #46]\n"
        "ands r0, r3\n"
        "lsrs r0, r0, #8\n"
        "adds r1, r4, #0\n"
        "adds r1, #52\n"
        "strb r0, [r1]\n"
        "ldr r0, .L2488C_masktbl2\n"
        "ldrb r1, [r0]\n"
        "ldrh r2, [r2, #46]\n"
        "ands r1, r2\n"
        "adds r0, r4, #0\n"
        "adds r0, #49\n"
        "strb r1, [r0]\n"
        "pop {r4, r5}\n"
        "pop {r0}\n"
        "bx r0\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".L2488C_base: .word 0x03001780\n"
        ".L2488C_idx_off: .word 0x00000574\n"
        ".L2488C_block2: .word 0x030013D0\n"
        ".L2488C_masktbl: .word 0x080CC1C8\n"
        ".L2488C_masktbl2: .word 0x080CC1D8\n"
        ".syntax divided\n"
    );
}
void _0802488C(void) __attribute__((alias("SaveGarageUnpack")));

// Pack/unpack of the 35 car/garage records (#0..#34).
//
//  * Both bases are SYMBOLS with the record offset added separately
//    (RAM_DEF/RAM_SYM_030013D0 and _03001780): the ROM keeps ONE literal for
//    0x030013D0 in `ip` and reaches the fields with `adds rX,#164/#168`, and
//    it builds the record base as `w + <literal 0x5E4>`. Spelling either as
//    `ADDR + 0x5E4` folds into a single pool word and loses the whole body
//    (the same fold trap documented at 0x0802476C).
//  * The loop counter is a SIGNED `s32` and the bound is `<= 34`, which is
//    what makes the ROM's `cmp r4,#34 / ble` signed; `i < CAR_RECORD_COUNT`
//    with an unsigned counter emits the unsigned `bls` instead.
//  * The 72-stride and the 8-stride each get their own rolling pointer
//    (`src`/`rec` + r2=w + r3/r5), and the 8-strided entry base is re-derived
//    per iteration from `i << 3` rather than strength-reduced, exactly as the
//    ROM does.
//  * entry.b0 is a HALFWORD read truncated on pack (`ldrh` + `strb`) and
//    re-stored sign-extended on unpack (`ldrsb` + `strh`), and the unpack
//    additionally zeroes rec+11. Both asymmetries are in the ROM.
//
// OPEN (pack 8/96, unpack 24/108). The residual is one decision inside
// agbcc's loop optimizer, not a source shape: the ROM leaves the entry
// address `base + 0xA4 + i*8` *un-strength-reduced* (it re-derives it each
// iteration with `lsls r1,r4,#3 / mov r0,ip / adds r0,#164 / adds r1,r1,r0`)
// and therefore keeps `i` live, which is also what stops the counter being
// rewritten as a countdown. Measured with `build/variant_lab.py`: every
// spelling of that destination that reaches the giv pass (`base + 0xA4 +
// ioff`, `(i<<3)`, `i*8`, a `u32 *` subscript, a `u32`-casted local, a
// pointer local assigned in the loop, volatile and non-volatile) yields the
// same 92-byte body in which the destination IS strength-reduced into a
// fourth rolling register (and so pushes r6 and counts down). The gate is
// `loop.c:4597`, `! flag_reduce_all_givs && v->lifetime * threshold * benefit
// < insn_count` -> `v->ignore = 1`, i.e. agbcc drops the *marginal* giv; the
// original build dropped that one and this one does not. Forcing the drop by
// making the index non-affine (`u16 i`) does remove the giv but then wraps the
// counter (`lsls #16 / lsrs #16`) and re-derives `i*72` instead of rolling it,
// which is worse. Same wall on the unpack side: the running `0x5E8+k` offset
// lives in a callee-saved register there (r7), while here it is rematerialised
// and the `+4/+5/+6` offsets come out of the literal pool (120 vs 108).
// Do not re-run these spellings without a new idea; see docs/matching_workflow.md.
__attribute__((naked)) void SaveCarRecordsPack(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, lr}\n"
        "movs r4, #0\n"
        "ldr r0, 1f\n"
        "mov ip, r0\n"
        "mov r3, ip\n"
        "adds r3, #168\n"
        "ldr r2, 2f\n"
        "ldr r1, 3f\n"
        "adds r5, r2, r1\n"
        ".Lsave_pack_loop:\n"
        "lsls r1, r4, #3\n"
        "mov r0, ip\n"
        "adds r0, #164\n"
        "adds r1, r1, r0\n"
        "ldr r0, [r5, #0]\n"
        "str r0, [r1, #0]\n"
        "movs r1, #189\n"
        "lsls r1, r1, #3\n"
        "adds r0, r2, r1\n"
        "ldrh r0, [r0, #0]\n"
        "strb r0, [r3, #0]\n"
        "adds r1, #4\n"
        "adds r0, r2, r1\n"
        "ldrb r0, [r0, #0]\n"
        "strb r0, [r3, #1]\n"
        "adds r1, #1\n"
        "adds r0, r2, r1\n"
        "ldrb r0, [r0, #0]\n"
        "strb r0, [r3, #2]\n"
        "adds r1, #1\n"
        "adds r0, r2, r1\n"
        "ldrb r0, [r0, #0]\n"
        "strb r0, [r3, #3]\n"
        "adds r3, #8\n"
        "adds r2, #72\n"
        "adds r5, #72\n"
        "adds r4, #1\n"
        "cmp r4, #34\n"
        "ble .Lsave_pack_loop\n"
        "pop {r4, r5}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".short 0\n"
        ".align 2\n"
        "1: .word 0x030013D0\n"
        "2: .word 0x03001780\n"
        "3: .word 0x000005E4\n"
        ".syntax divided\n"
    );
}
void _0802494C(void) __attribute__((alias("SaveCarRecordsPack")));

// The inverse loop has one deliberate sign-extending byte load and a fourth
// pool word for the record +11 clear; keep those details in the ROM body.
__attribute__((naked)) void SaveCarRecordsUnpack(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "movs r4, #0\n"
        "ldr r0, 1f\n"
        "mov ip, r0\n"
        "movs r6, #0\n"
        "mov r3, ip\n"
        "adds r3, #168\n"
        "ldr r2, 2f\n"
        "ldr r1, 3f\n"
        "adds r5, r2, r1\n"
        ".Lsave_unpack_loop:\n"
        "lsls r0, r4, #3\n"
        "mov r1, ip\n"
        "adds r1, #164\n"
        "adds r0, r0, r1\n"
        "ldr r0, [r0, #0]\n"
        "str r0, [r5, #0]\n"
        "movs r1, #0\n"
        "ldrsb r1, [r3, r1]\n"
        "movs r7, #189\n"
        "lsls r7, r7, #3\n"
        "adds r0, r2, r7\n"
        "strh r1, [r0, #0]\n"
        "ldrb r1, [r3, #1]\n"
        "adds r7, #4\n"
        "adds r0, r2, r7\n"
        "strb r1, [r0, #0]\n"
        "ldrb r1, [r3, #2]\n"
        "adds r7, #1\n"
        "adds r0, r2, r7\n"
        "strb r1, [r0, #0]\n"
        "ldrb r1, [r3, #3]\n"
        "adds r7, #1\n"
        "adds r0, r2, r7\n"
        "strb r1, [r0, #0]\n"
        "ldr r1, 4f\n"
        "adds r0, r2, r1\n"
        "strb r6, [r0, #0]\n"
        "adds r3, #8\n"
        "adds r2, #72\n"
        "adds r5, #72\n"
        "adds r4, #1\n"
        "cmp r4, #34\n"
        "ble .Lsave_unpack_loop\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2\n"
        "1: .word 0x030013D0\n"
        "2: .word 0x03001780\n"
        "3: .word 0x000005E4\n"
        "4: .word 0x000005EF\n"
        ".syntax divided\n"
    );
}
void _080249AC(void) __attribute__((alias("SaveCarRecordsUnpack")));

__attribute__((naked)) void SaveBlock2PreSave(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, lr}\n"
        "ldr r4, 1f\n"
        "ldr r5, 2f\n"
        "adds r1, r4, #0\n"
        "adds r0, r5, #0\n"
        "ldmia r0!, {r2, r3, r6}\n"
        "stmia r1!, {r2, r3, r6}\n"
        "ldmia r0!, {r2, r3, r6}\n"
        "stmia r1!, {r2, r3, r6}\n"
        "ldmia r0!, {r2, r3}\n"
        "stmia r1!, {r2, r3}\n"
        "ldmia r0!, {r2, r3, r6}\n"
        "stmia r1!, {r2, r3, r6}\n"
        "ldrh r0, [r0, #0]\n"
        "strh r0, [r1, #0]\n"
        "movs r6, #174\n"
        "lsls r6, r6, #3\n"
        "adds r0, r5, r6\n"
        "ldr r0, [r0, #0]\n"
        "str r0, [r4, #48]\n"
        "ldr r1, 3f\n"
        "adds r0, r5, r1\n"
        "ldrh r0, [r0, #0]\n"
        "strh r0, [r4, #52]\n"
        "ldr r2, 4f\n"
        "adds r0, r5, r2\n"
        "ldrh r0, [r0, #0]\n"
        "strh r0, [r4, #54]\n"
        "bl _08024794\n"
        "movs r3, #175\n"
        "lsls r3, r3, #3\n"
        "adds r0, r5, r3\n"
        "ldr r0, [r0, #0]\n"
        "str r0, [r4, #56]\n"
        "adds r0, r4, #0\n"
        "adds r0, #60\n"
        "adds r6, #12\n"
        "adds r1, r5, r6\n"
        "ldr r2, [r1, #0]\n"
        "str r2, [r4, #60]\n"
        "movs r2, #96\n"
        "bl sub_0802E0A4\n"
        "movs r0, #188\n"
        "lsls r0, r0, #3\n"
        "adds r5, r5, r0\n"
        "ldrh r0, [r5, #0]\n"
        "adds r4, #160\n"
        "strh r0, [r4, #0]\n"
        "bl _0802494C\n"
        "pop {r4, r5, r6}\n"
        "pop {r0}\n"
        "bx r0\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        "1: .word 0x030013D0\n"
        "2: .word 0x03001780\n"
        "3: .word 0x00000574\n"
        "4: .word 0x00000576\n"
        ".syntax divided\n"
    );
}
void _08024A18(void) __attribute__((alias("SaveBlock2PreSave")));

__attribute__((naked)) void SaveBlock2PostLoad(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, lr}\n"
        "ldr r5, 1f\n"
        "ldr r4, 2f\n"
        "adds r1, r5, #0\n"
        "adds r0, r4, #0\n"
        "ldmia r0!, {r2, r3, r6}\n"
        "stmia r1!, {r2, r3, r6}\n"
        "ldmia r0!, {r2, r3, r6}\n"
        "stmia r1!, {r2, r3, r6}\n"
        "ldmia r0!, {r2, r3}\n"
        "stmia r1!, {r2, r3}\n"
        "ldmia r0!, {r2, r3, r6}\n"
        "stmia r1!, {r2, r3, r6}\n"
        "ldrh r0, [r0, #0]\n"
        "strh r0, [r1, #0]\n"
        "movs r6, #174\n"
        "lsls r6, r6, #3\n"
        "adds r1, r5, r6\n"
        "ldr r0, [r4, #48]\n"
        "str r0, [r1, #0]\n"
        "ldrh r1, [r4, #52]\n"
        "ldr r2, 3f\n"
        "adds r0, r5, r2\n"
        "strh r1, [r0, #0]\n"
        "ldrh r1, [r4, #54]\n"
        "ldr r3, 4f\n"
        "adds r0, r5, r3\n"
        "strh r1, [r0, #0]\n"
        "bl _0802488C\n"
        "adds r6, #8\n"
        "adds r1, r5, r6\n"
        "ldr r0, [r4, #56]\n"
        "str r0, [r1, #0]\n"
        "ldr r1, 5f\n"
        "adds r0, r5, r1\n"
        "adds r1, r4, #0\n"
        "adds r1, #60\n"
        "ldr r2, [r4, #60]\n"
        "str r2, [r0, #0]\n"
        "movs r2, #96\n"
        "bl sub_0802E0A4\n"
        "adds r4, #160\n"
        "ldrh r0, [r4, #0]\n"
        "movs r2, #188\n"
        "lsls r2, r2, #3\n"
        "adds r5, r5, r2\n"
        "strh r0, [r5, #0]\n"
        "bl _080249AC\n"
        "pop {r4, r5, r6}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        "1: .word 0x03001780\n"
        "2: .word 0x030013D0\n"
        "3: .word 0x00000574\n"
        "4: .word 0x00000576\n"
        "5: .word 0x0000057C\n"
        ".syntax divided\n"
    );
}
void _08024A98(void) __attribute__((alias("SaveBlock2PostLoad")));

int SaveBlock2Load(void) {
    // The post-load hook fires on an EQUALS-ONE test against the BYTE-truncated
    // status, not on a truthiness test: the ROM is `lsls r0,#24 / lsrs r0,#24 /
    // cmp r0,#1 / bne`, so the truncation is load-bearing and must stay.
    u32 idx = *(volatile u8 *)SAVE_BLOCK2_IDX_ADDR;
    //. Call the asm spelling, not the friendly C name: the promotion
    // screen resolves each callee against the include closure rooted at
    // asm/code.s, where this entry is `_08005988` / `sub_08005988`. Naming
    // `SaveSlotLoad` left it blocked with
    // `SaveSlotLoad: closure defines _08005988/sub_08005988 at 0x08005988
    // (rename)`. Both spellings alias the one body, so this is a rename of
    // one routine, not a second routine -- and the scored bytes are
    // unchanged because it is the same call target at the same address.
    if ((u8)sub_08005988(idx, (void *)SAVE_BLOCK2_BUF_ADDR) == 1) SAVE_CALL_POST_LOAD();
    return 1;
}
int _08024B24(void) __attribute__((alias("SaveBlock2Load")));

// All four guarded/block2 SAVE bodies below are VOID in the ROM, and that is
// byte-visible, not cosmetic. Their epilogue is `pop {r0}; bx r0` (B70:
// `add sp,#4; pop {r4}; pop {r0}; bx r0`), which destroys r0 — so r0 is not
// a return value and no caller can be reading one. Declaring any of them `int`
// keeps r0 live into the epilogue, and agbcc then picks a different scratch
// for the pop: `pop {r1}; bx r1` (plus, for the two `return 1;` bodies, a
// dead `movs r0,#1` that costs 2 more bytes). That single declaration was the
// whole of the miss across the four bodies -- 16 bytes by probe: B54 26/28 ->
// 28/28, B70 72/80 -> 80/80, BC0 and BD8 19/24 -> 22/24 (the last two then
// being alignment-only, not EXACT). Root cause is return TYPE, not source
// shape and not scheduling.
void SaveBlock2Save(void) {
    SAVE_CALL_PRE_SAVE();
    u32 idx = *(volatile u8 *)SAVE_BLOCK2_IDX_ADDR;
    SAVE_CALL_SLOT_SAVE(idx, (void *)SAVE_BLOCK2_BUF_ADDR);
}
void _08024B54(void) __attribute__((alias("SaveBlock2Save")));

void SaveGuardedFull(void) {
    sub_0802B234(); sub_0802B190();
    u32 zero=0; sub_0802D974(&zero,(void *)SAVE_BLOCK2_BUF_ADDR,0x0500007F);
    SAVE_CALL_POST_LOAD(); SAVE_CALL_HOOK_2446C(); SAVE_CALL_PRE_SAVE();
    u32 idx = *(volatile u8 *)SAVE_BLOCK2_IDX_ADDR;
    SAVE_CALL_SLOT_SAVE(idx,(void *)SAVE_BLOCK2_BUF_ADDR);
    SAVE_CALL_POST_LOAD();
    sub_0802B1B8();
}
void _08024B70(void) __attribute__((alias("SaveGuardedFull")));

void SaveGuardedSaveOnly(void) { sub_0802B234(); sub_0802B190(); SAVE_CALL_BLOCK2_SAVE(); sub_0802B1B8(); }
__asm__(".align 2, 0");
void _08024BC0(void) __attribute__((alias("SaveGuardedSaveOnly")));
// Friendly ROM aliases used by rec35_stage.c. Keeping them bound to the
// original bodies avoids an extra wrapper branch in the independent slice.
void GuardedSaveOnly(void) __attribute__((alias("SaveGuardedSaveOnly")));
void GuardedFullSave(void) __attribute__((alias("SaveGuardedFull")));
void SaveGuardedLoadOnly(void) { sub_0802B234(); sub_0802B190(); SAVE_CALL_BLOCK2_LOAD(); sub_0802B1B8(); }
__asm__(".align 2, 0");
void _08024BD8(void) __attribute__((alias("SaveGuardedLoadOnly")));

// Neither body is a source problem: 0x08024BF0 and 0x08024BFC are
// 10/12 ALIGNMENT-ONLY. agbcc ends the 10-byte body with its `c0 46` nop pad
// where the ROM carries `00 00`, so prefix(10) + 2 == rom_bytes(12) and the
// only differing offsets are the last two. Do not "fix" that -- it is the pad.
extern void *sub_080240D0(void);   // Ghost_GetRecP (src/ghost2.c:122)
#define SAVE_CALL_GHOST_REC_P()   sub_080240D0()
extern int _08024B18(void);        // SaveRet1_24B18 (src/runtime_state_dispatch.c)

void *SaveGhostRecPair_BF0(void) { return SAVE_CALL_GHOST_REC_P(); }
__asm__(".align 2, 0");
void *SaveGhostRecPair_BFC(void) { return SAVE_CALL_GHOST_REC_P(); }
__asm__(".align 2, 0");
void SaveNoOp_24C08(void) { }
__asm__(".align 2, 0");
void SaveNoOp_24C0C(void) { }
__asm__(".align 2, 0");
void SaveNoOp_24C10(void) { }
__asm__(".align 2, 0");
void SaveNoOp_24C14(void) { }
__asm__(".align 2, 0");
void SaveNoOp_24C18(void) { }
__asm__(".align 2, 0");

// Guarded no-op sequence: the guarded bracket around two calls to the
// return-1 stub. The `movs r0, #0/#1` before the two `_08024B18` calls is the
// dead argument described above.
__attribute__((naked)) void SaveGuardedNoOp_24C1C(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {lr}\n"
        "bl sub_0802B234\n"
        "bl sub_0802B190\n"
        "movs r0, #0\n"
        "bl _08024B18\n"
        "movs r0, #1\n"
        "bl _08024B18\n"
        "bl sub_0802B1B8\n"
        "pop {r0}\n"
        "bx r0\n"
        "movs r0, r0\n"
        ".syntax divided\n"
    );
}

void *_08024BF0(void) __attribute__((alias("SaveGhostRecPair_BF0")));
void *sub_08024BF0(void) __attribute__((alias("SaveGhostRecPair_BF0")));
void *_08024BFC(void) __attribute__((alias("SaveGhostRecPair_BFC")));
void *sub_08024BFC(void) __attribute__((alias("SaveGhostRecPair_BFC")));
void _08024C08(void) __attribute__((alias("SaveNoOp_24C08")));
void _08024C0C(void) __attribute__((alias("SaveNoOp_24C0C")));
void _08024C10(void) __attribute__((alias("SaveNoOp_24C10")));
void _08024C14(void) __attribute__((alias("SaveNoOp_24C14")));
void _08024C18(void) __attribute__((alias("SaveNoOp_24C18")));
void _08024C1C(void) __attribute__((alias("SaveGuardedNoOp_24C1C")));
void sub_08024C1C(void) __attribute__((alias("SaveGuardedNoOp_24C1C")));
// A sound-lane spelling of the same 0x08024C0C no-op (rec35_runtime.c).
void Sound_0x08024C0C(void) __attribute__((alias("SaveNoOp_24C0C")));
// Third spellings (`Sub_`) — only the `_`/`sub_` spellings are emitted, so the
// rec35 lane's capital-S declarations need explicit definitions.
void *Sub_08024BF0(void) __attribute__((alias("SaveGhostRecPair_BF0")));
void *Sub_08024BFC(void) __attribute__((alias("SaveGhostRecPair_BFC")));
void Sub_08024C08(void) __attribute__((alias("SaveNoOp_24C08")));
void Sub_08024C0C(void) __attribute__((alias("SaveNoOp_24C0C")));
void Sub_08024C10(void) __attribute__((alias("SaveNoOp_24C10")));
void Sub_08024C14(void) __attribute__((alias("SaveNoOp_24C14")));
void Sub_08024C18(void) __attribute__((alias("SaveNoOp_24C18")));
void Sub_08024C1C(void) __attribute__((alias("SaveGuardedNoOp_24C1C")));

void SaveGhostBulkSave(void) {
    // One callee-saved base (r4) carries both ghost slots: the ROM loads
    // 0x03003574 once and advances it with `adds r4, #32` for the second
    // save, so the second operand must be `p + 32` and not a second literal
    // (two literals would need two pool words and drop the r4 induction).
    const u8 *p;
    sub_0802417C();
    p = (const u8 *)0x03003574u;
    SAVE_CALL_SLOT_SAVE(0, p);
    SAVE_CALL_SLOT_SAVE(0, p + 32);
}
void _0802471C(void) __attribute__((alias("SaveGhostBulkSave")));

// Scene/reset hook — substantiated from asm/saveblock.s and asm gap 0x2446C (objdump)
// Preserves halfword/byte widths and indirect calls; no guessed struct.
__attribute__((naked)) void SaveHook_0802446C(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push\t{r4, r5, r6, lr}\n"
        "ldr r4, _080024548\n"
        "ldr r0, _08002454C\n"
        "adds\tr2, r4, r0\n"
        "movs\tr1, #0\n"
        "movs\tr0, #8\n"
        "strh\tr0, [r2, #0]\n"
        "ldr r2, _080024550\n"
        "adds\tr0, r4, r2\n"
        "movs\tr2, #1\n"
        "strb\tr2, [r0, #0]\n"
        "movs\tr3, #188\n"
        "lsls\tr3, r3, #3\n"
        "adds\tr0, r4, r3\n"
        "strh\tr2, [r0, #0]\n"
        "movs\tr2, #255\n"
        "lsls\tr2, r2, #4\n"
        "adds\tr0, r4, r2\n"
        "strh\tr1, [r0, #0]\n"
        "ldr r3, _080024554\n"
        "adds\tr0, r4, r3\n"
        "strh\tr1, [r0, #0]\n"
        "adds\tr2, #6\n"
        "adds\tr0, r4, r2\n"
        "strh\tr1, [r0, #0]\n"
        "adds\tr3, #72\n"
        "adds\tr0, r4, r3\n"
        "strh\tr1, [r0, #0]\n"
        "ldr r0, _080024558\n"
        "adds\tr2, r4, r0\n"
        "movs\tr0, #16\n"
        "strh\tr0, [r2, #0]\n"
        "ldr r2, _08002455C\n"
        "adds\tr0, r4, r2\n"
        "strh\tr1, [r0, #0]\n"
        "adds\tr3, #32\n"
        "adds\tr0, r4, r3\n"
        "strh\tr1, [r0, #0]\n"
        "adds\tr2, #30\n"
        "adds\tr0, r4, r2\n"
        "strh\tr1, [r0, #0]\n"
        "adds\tr3, #2\n"
        "adds\tr0, r4, r3\n"
        "strh\tr1, [r0, #0]\n"
        "movs\tr0, #0\n"
        "bl 0x0800274C\n"
        "movs\tr1, #0\n"
        "movs\tr0, #9\n"
        "ldr r2, _080024560\n"
        "adds\tr4, r4, r2\n"
        ".Lsave2446c_0800244D2:\n"
        "strh\tr1, [r4, #0]\n"
        "subs\tr4, #2\n"
        "subs\tr0, #1\n"
        "cmp\tr0, #0\n"
        "bge .Lsave2446c_0800244D2\n"
        "movs\tr1, #0\n"
        "ldr r0, _080024548\n"
        "movs\tr3, #0\n"
        "movs\tr2, #253\n"
        "lsls\tr2, r2, #4\n"
        "adds\tr4, r0, r2\n"
        ".Lsave2446c_0800244E8:\n"
        "lsls\tr0, r1, #3\n"
        "adds\tr2, r1, #1\n"
        "movs\tr1, #3\n"
        "adds\tr0, r0, r4\n"
        "adds\tr0, #6\n"
        ".Lsave2446c_0800244F2:\n"
        "strh\tr3, [r0, #0]\n"
        "subs\tr0, #2\n"
        "subs\tr1, #1\n"
        "cmp\tr1, #0\n"
        "bge .Lsave2446c_0800244F2\n"
        "adds\tr1, r2, #0\n"
        "cmp\tr1, #1\n"
        "ble .Lsave2446c_0800244E8\n"
        "ldr r5, _080024564\n"
        "movs\tr4, #97\n"
        ".Lsave2446c_080024506:\n"
        "adds\tr0, r5, #0\n"
        "bl 0x08024F34\n"
        "adds\tr5, #12\n"
        "subs\tr4, #1\n"
        "cmp\tr4, #0\n"
        "bge .Lsave2446c_080024506\n"
        "movs\tr5, #0\n"
        ".Lsave2446c_080024516:\n"
        "movs\tr4, #0\n"
        "adds\tr6, r5, #1\n"
        ".Lsave2446c_08002451A:\n"
        "adds\tr0, r5, #0\n"
        "adds\tr1, r4, #0\n"
        "movs\tr2, #0\n"
        "bl 0x08002714\n"
        "adds\tr0, r5, #0\n"
        "adds\tr1, r4, #0\n"
        "movs\tr2, #0\n"
        "bl 0x080026DC\n"
        "adds\tr4, #1\n"
        "cmp\tr4, #3\n"
        "ble .Lsave2446c_08002451A\n"
        "adds\tr5, r6, #0\n"
        "cmp\tr5, #1\n"
        "ble .Lsave2446c_080024516\n"
        "bl 0x0802417C\n"
        "bl 0x08024338\n"
        "pop\t{r4, r5, r6}\n"
        "pop\t{r0}\n"
        "bx\tr0\n"
        "_080024548: .word 0x03001780\n"
        "_08002454C: .word 0x00000574\n"
        "_080024550: .word 0x000010BF\n"
        "_080024554: .word 0x00000FF2\n"
        "_080024558: .word 0x00000576\n"
        "_08002455C: .word 0x00001058\n"
        "_080024560: .word 0x00001072\n"
        "_080024564: .word 0x030017B0\n"
        ".syntax divided\n"
    );
}
void sub_0802446C(void) __attribute__((alias("SaveHook_0802446C")));
void _0802446C(void) __attribute__((alias("SaveHook_0802446C")));

// Zone-grid updater — substantiated §10.1 (asm 0x241C8)
// Loops k∈{0,1} c∈0..3: v=_08025D64(k,c) → grids, plus _0800A9A0/_0800A9E0 side effects.
__attribute__((naked)) void SaveUpdate_080241C8(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push\t{r4, r5, r6, r7, lr}\n"
        "mov\tr7, sl\n"
        "mov\tr6, r9\n"
        "mov\tr5, r8\n"
        "push\t{r5, r6, r7}\n"
        "sub\tsp, #8\n"
        "movs\tr7, #0\n"
        "ldr r0, _080024240\n"
        "mov\tr8, r0\n"
        "movs\tr1, #255\n"
        "lsls\tr1, r1, #4\n"
        "add\tr1, r8\n"
        "mov\tsl, r1\n"
        ".Lsave241c8_0800241E2:\n"
        "movs\tr5, #0\n"
        "lsls\tr2, r7, #3\n"
        "mov\tr9, r2\n"
        "adds\tr0, r7, #1\n"
        "str\tr0, [sp, #0]\n"
        ".Lsave241c8_0800241EC:\n"
        "adds\tr0, r7, #0\n"
        "adds\tr1, r5, #0\n"
        "bl 0x08025D64\n"
        "lsls\tr2, r5, #1\n"
        "mov\tr1, r9\n"
        "adds\tr3, r2, r1\n"
        "ldr r6, _080024244\n"
        "add\tr6, r8\n"
        "adds\tr1, r3, r6\n"
        "strh\tr0, [r1, #0]\n"
        "lsls\tr0, r0, #16\n"
        "asrs\tr0, r0, #16\n"
        "str\tr2, [sp, #4]\n"
        "cmp\tr0, #11\n"
        "bne .Lsave241c8_0800242A4\n"
        "ldr r0, _080024248\n"
        "add\tr0, r8\n"
        "adds\tr0, r3, r0\n"
        "movs\tr3, #1\n"
        "strh\tr3, [r0, #0]\n"
        "adds\tr0, r5, #1\n"
        "ldr r1, _08002424C\n"
        "add\tr1, r8\n"
        "strh\tr0, [r1, #0]\n"
        "adds\tr4, r0, #0\n"
        "cmp\tr7, #0\n"
        "bne .Lsave241c8_080024274\n"
        "mov\tr2, sl\n"
        "movs\tr0, #0\n"
        "ldrsh\tr2, [r2, r0]\n"
        "cmp\tr2, #0\n"
        "bne .Lsave241c8_080024254\n"
        "lsls\tr0, r4, #16\n"
        "asrs\tr0, r0, #16\n"
        "ldr r2, _080024250\n"
        "adds\tr6, r6, r2\n"
        "cmp\tr0, #3\n"
        "ble .Lsave241c8_0800242B4\n"
        "movs\tr0, #3\n"
        "strh\tr0, [r1, #0]\n"
        "b .Lsave241c8_0800242B4\n"
        "_080024240: .word 0x03001780\n"
        "_080024244: .word 0x00000FFA\n"
        "_080024248: .word 0x0000101A\n"
        "_08002424C: .word 0x00000FF6\n"
        "_080024250: .word 0xFFFFF006\n"
        ".Lsave241c8_080024254:\n"
        "lsls\tr0, r4, #16\n"
        "asrs\tr0, r0, #16\n"
        "ldr r2, _08002426C\n"
        "adds\tr6, r6, r2\n"
        "cmp\tr0, #3\n"
        "ble .Lsave241c8_0800242B4\n"
        "strh\tr7, [r1, #0]\n"
        "ldr r0, _080024270\n"
        "add\tr0, r8\n"
        "strh\tr3, [r0, #0]\n"
        "b .Lsave241c8_0800242B4\n"
        "movs\tr0, r0\n"
        "_08002426C: .word 0xFFFFF006\n"
        "_080024270: .word 0x00000FF2\n"
        ".Lsave241c8_080024274:\n"
        "mov\tr0, sl\n"
        "ldrh\tr0, [r0, #0]\n"
        "cmp\tr0, #1\n"
        "bne .Lsave241c8_080024288\n"
        "lsls\tr0, r4, #16\n"
        "asrs\tr0, r0, #16\n"
        "cmp\tr0, #3\n"
        "ble .Lsave241c8_080024288\n"
        "movs\tr0, #3\n"
        "strh\tr0, [r1, #0]\n"
        ".Lsave241c8_080024288:\n"
        "movs\tr1, #1\n"
        "mov\tr2, sl\n"
        "strh\tr1, [r2, #0]\n"
        "ldr r0, _08002429C\n"
        "add\tr0, r8\n"
        "strh\tr1, [r0, #0]\n"
        "ldr r1, _0800242A0\n"
        "adds\tr6, r0, r1\n"
        "b .Lsave241c8_0800242B4\n"
        "movs\tr0, r0\n"
        "_08002429C: .word 0x00000FF2\n"
        "_0800242A0: .word 0xFFFFF00E\n"
        ".Lsave241c8_0800242A4:\n"
        "ldr r0, _0800242D4\n"
        "add\tr0, r8\n"
        "adds\tr0, r3, r0\n"
        "movs\tr2, #0\n"
        "strh\tr2, [r0, #0]\n"
        "ldr r0, _0800242D8\n"
        "adds\tr6, r6, r0\n"
        "adds\tr4, r5, #1\n"
        ".Lsave241c8_0800242B4:\n"
        "ldr\tr2, [sp, #4]\n"
        "add\tr2, r9\n"
        "ldr r1, _0800242DC\n"
        "adds\tr0, r6, r1\n"
        "adds\tr0, r2, r0\n"
        "movs\tr1, #0\n"
        "ldrsh\tr0, [r0, r1]\n"
        "cmp\tr0, #2\n"
        "ble .Lsave241c8_0800242E4\n"
        "ldr r1, _0800242E0\n"
        "adds\tr0, r6, r1\n"
        "adds\tr0, r2, r0\n"
        "movs\tr1, #1\n"
        "strh\tr1, [r0, #0]\n"
        "b .Lsave241c8_0800242EE\n"
        "movs\tr0, r0\n"
        "_0800242D4: .word 0x0000101A\n"
        "_0800242D8: .word 0xFFFFF006\n"
        "_0800242DC: .word 0x00000FFA\n"
        "_0800242E0: .word 0x0000102A\n"
        ".Lsave241c8_0800242E4:\n"
        "ldr r1, _080024334\n"
        "adds\tr0, r6, r1\n"
        "adds\tr0, r2, r0\n"
        "movs\tr2, #0\n"
        "strh\tr2, [r0, #0]\n"
        ".Lsave241c8_0800242EE:\n"
        "adds\tr0, r7, #0\n"
        "adds\tr1, r5, #0\n"
        "bl 0x0800A9A0\n"
        "adds\tr2, r0, #0\n"
        "adds\tr0, r7, #0\n"
        "adds\tr1, r5, #0\n"
        "bl 0x08002714\n"
        "adds\tr0, r7, #0\n"
        "adds\tr1, r5, #0\n"
        "bl 0x0800A9E0\n"
        "adds\tr2, r0, #0\n"
        "adds\tr0, r7, #0\n"
        "adds\tr1, r5, #0\n"
        "bl 0x080026DC\n"
        "adds\tr5, r4, #0\n"
        "cmp\tr5, #3\n"
        "bgt .Lsave241c8_08002431A\n"
        "b .Lsave241c8_0800241EC\n"
        ".Lsave241c8_08002431A:\n"
        "ldr\tr7, [sp, #0]\n"
        "cmp\tr7, #1\n"
        "bgt .Lsave241c8_080024322\n"
        "b .Lsave241c8_0800241E2\n"
        ".Lsave241c8_080024322:\n"
        "add\tsp, #8\n"
        "pop\t{r3, r4, r5}\n"
        "mov\tr8, r3\n"
        "mov\tr9, r4\n"
        "mov\tsl, r5\n"
        "pop\t{r4, r5, r6, r7}\n"
        "pop\t{r0}\n"
        "bx\tr0\n"
        "movs\tr0, r0\n"
        "_080024334: .word 0x0000102A\n"
        ".syntax divided\n"
    );
}
void _080241C8(void) __attribute__((alias("SaveUpdate_080241C8")));
void Sub_080241C8(void) __attribute__((alias("SaveUpdate_080241C8")));
void sub_080241C8(void) __attribute__((alias("SaveUpdate_080241C8")));

// Full zone/event rebuild — substantiated §10.2 (asm 0x24338, sole caller @0x2453E)
// Sequence: _08025B3C, _08025B88, zero 2×4×11 via _08025C84, _08025D90, _08025D64→0x103A, _080241C8, event clears, 32×_08025DBC, etc.
void SaveRebuild_08024338(void) {
    // Callees must use the closure spellings the promotion gate resolves
    // (_080025B3C / _080025B88 are the aliases defined in src/code_25930.c; the
    // 8-digit `_08025B3C` form is undefined and reads as a foreign symbol).
    extern void _080025B3C(void); extern void _080025B88(void);
    extern void _08025C84(int,int,int,int); extern int _08025D90(int); extern int _08025D64(int,int);
    extern void _08025F78(int); extern void _08025DBC(int,int); extern void _08025E70(int,int,int,int);
    extern void _08025FF0(int);
    _080025B3C(); _080025B88();
    // ROM: the t = 0/1 pass is UNROLLED inside the (r, col) nest -- two
    // `bl 0x08025C84` per iteration, `movs r0,#0` then `movs r0,#1`, with r1 = r
    // (0..3) and r2 = col (0..10). The three-level `t` loop cannot produce that
    // shape: it makes t the outer counter instead of an unrolled first argument.
    for(int r=0;r<=3;r++)
        for(int col=0;col<=10;col++){
            _08025C84(0,r,col,0);
            _08025C84(1,r,col,0);
        }
    // s16 B[FF2] → _08025D90 → B[FF6]; then _08025D64(B[FF2],B[FF6])→B[103A] — width s16
    // NON-volatile: the ROM's three reads are the indexed `movs rN,#0 /
    // ldrsh rM,[rBase,rN]` form, which agbcc only emits for a plain `*(s16 *)`;
    // the volatile spelling lowers to `ldrh + lsls + asrs` and adds 6 bytes.
    // B[FF2] is read twice, before and after the _08025D90 call, and the call
    // is enough of a memory barrier for agbcc to reload it -- same as the ROM.
    s16 *Bs = (s16 *)GARAGE_BASE_ADDR;
    s16 ff2 = Bs[0xFF2/2]; int ff6 = _08025D90(ff2); Bs[0xFF6/2]=(s16)ff6;
    int v = _08025D64(Bs[0xFF2/2], Bs[0xFF6/2]); Bs[0x103A/2]=(s16)v;
    sub_080241C8();
    // ROM: 20 separate `movs r0,#N / bl 0x08025F78` pairs, not a table walk. A
    // local `int evs[] = {...}` initializer instead puts the list in .rodata,
    // which is a relocation with no ROM VMA -- the probe reports it as
    // UNRESOLVED_RELOCATION and the body also carries a loop the ROM lacks.
    _08025F78(1);  _08025F78(3);  _08025F78(6);  _08025F78(7);
    _08025F78(8);  _08025F78(10); _08025F78(15); _08025F78(16);
    _08025F78(24); _08025F78(25); _08025F78(32); _08025F78(34);
    _08025F78(35); _08025F78(41); _08025F78(57); _08025F78(63);
    _08025F78(65); _08025F78(67); _08025F78(68); _08025F78(72);
    for(int i=0;i<32;i++) _08025DBC(i,0);
    for(int k=0;k<=1;k++) for(int g=0;g<=3;g++) for(int j=0;j<=2;j++) _08025E70(k,g,j,0);
    _08025FF0(0);
}
void _08024338(void) __attribute__((alias("SaveRebuild_08024338")));
void sub_08024338(void) __attribute__((alias("SaveRebuild_08024338")));

// Additional veneers — pure bx lr stubs (no effect). One body per ROM address:
// the splice binds one C body to one entry, so the three trailing stubs at
// 0x08024710/14/18 each need their own definition. The file-scope
// `.align 2, 0` pads the body's own section with `00 00`, as the ROM does,
// instead of gas's `c0 46` nop (trap 6).
static void SaveStubBxLr(void) {}
__asm__(".align 2, 0");
static void SaveStubBxLr_10(void) {}
__asm__(".align 2, 0");
static void SaveStubBxLr_14(void) {}
__asm__(".align 2, 0");
static void SaveStubBxLr_18(void) {}
__asm__(".align 2, 0");
void _08024668(void) __attribute__((alias("SaveStubBxLr")));
void _08024710(void) __attribute__((alias("SaveStubBxLr_10")));
void _08024714(void) __attribute__((alias("SaveStubBxLr_14")));
void _08024718(void) __attribute__((alias("SaveStubBxLr_18")));
void * sub_0800572C(u32 s) __attribute__((alias("_0800572C")));
void * Sub_0800572C(u32 s) __attribute__((alias("_0800572C")));

// ----------------------------------------------------------------------------
// 0x08002A0C — 36B IRQ-state restore ((consolidated/elsewhere); asm/agbmain.s §"0x080029D8"
// documents the pair: sub_080029D8 suspends, this body restores):
//   CpuSet(dst=0x0203F170 table, src=0x030000F8 slot, 28 words); then
//   IE  (0x04000200) = u16[0x030000F8+58]; DISPSTAT (0x04000004) = u16[+56].
// (pool words 0x030000F8 / 0x0203F170 / 0x04000004 / 0x04000200). save-side
// checksum path calls it after handler-table surgery (asm/save_checksum.s:61).
void IrqRestore_2A0C(void) {
    extern void sub_0802D974(const void *src, void *dst, u32 ctrl);
    // One callee-saved base pointer (r4) is reloaded AFTER the copy for both
    // register restores, so the saves must read through `st` *after* the call
    // and the count must be the bare immediate 28 (the ROM's `movs r2,#28`,
    // not a pool word). Order is IE (offset 58 -> 0x04000004) then
    // DISPSTAT (offset 56 -> 0x04000200), which is what the ROM stores.
    volatile u16 *st = (volatile u16 *)0x030000F8u;
    sub_0802D974((const void *)st, (void *)0x0203F170u, 28);
    *(volatile u16 *)0x04000004u = st[29];
    *(volatile u16 *)0x04000200u = st[28];
}
void _08002A0C(void) __attribute__((alias("IrqRestore_2A0C")));
void sub_08002A0C(void) __attribute__((alias("IrqRestore_2A0C")));
