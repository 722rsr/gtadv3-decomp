// ============================================================================
// Runtime state dispatch, resource collection, and device helpers.
//
// Assembly owners:
//
//   asm/saveblock.s            0x08024B18 (return-1 leaf)
//   asm/code_4b9c.s            0x08004BAC (substate-flag OR leaf)
//   asm/carphys_racer.s        0x08021BA0 (ev-7 per-car-id dispatch; raw incbin)
//   asm/code_c668.s            0x0800C744 (menu mode dispatcher, 21-entry table)
//   asm/sound_d9b0.s           0x0802D9FC (sound countdown latch)
//   asm/handlers.s             0x08000CA0 (block-B special-mode state stepper)
//   asm/course_stream.s        0x08006A50 (nearby-record collector)
//   asm/code_24ac.s            0x0800254C (screen-tile initializer)
//   asm/code_23e0c.s           0x080023E78 (bx lr stub)
//   asm/sound_voice_tick.s     0x0802CE20 (voice param tick)
//   asm/sound_voice_apply.s    0x0802C160 (voice envelope apply)
//   asm/sound_veneer.s         0x0802DDC8 (bx r0 veneer slot)
//   asm/sound_reset_more.s     0x0802CACC (sound FSM stepper over 0x03007FF0)
//   asm/sound_followon.s       0x0802C5C0 (bank claim check)
//   asm/menu_fa24.s            0x0800FE90 (record state dispatcher, 8-way)
//   asm/menu_f924.s            0x0800F9A0 (result scatter)
//   asm/menu_f5a0.s            0x0800F5EC (record dispatch, 12-way)
//   asm/menu_f22c.s            0x0800F59C (bx lr stub)
//   asm/course_stream_6650.s   0x080068D4 (proximity collector)
//   asm/ai_grid.s              0x08025CF4 (packed 2-bit grid getter)
//
// Transcribed instruction-for-instruction from the cited asm listings and
// objdump of baserom.gba (raw.incbin pockets at 0x08021BA0, 0x08000CA0).
// Pool words byte-verified via xxd.
// ============================================================================

#include "gba/types.h"
#include "gtadv/ram_blocks.h"
// 0x08026230 / 0x080262A4 are renamed from `_08026230` / `_080262A4`: the slice
// closure defines only the `sub_` spelling (promotion rule 1).
extern void *sub_08026230(int a, void *b);
extern void sub_080262A4(void *a, int b);

// ----------------------------------------------------------------------------
// Callees (all defined elsewhere; weak host no-ops keep this TU linkable
// standalone on host builds; strong bodies win on the ARM link).
// ----------------------------------------------------------------------------
#define HOST_STUB(sig) extern sig

HOST_STUB(void  _0802D974(const void *src, void *dst, u32 ctrl));
HOST_STUB(int   _0802D97C(int a, int b));               // swi 0x06 DivRem
HOST_STUB(void  _0802E0A4(void *dst, const void *src, u32 n));  // ROM ABI: r0=dst
HOST_STUB(void *_08004EFC(u32 size));
HOST_STUB(int   _080056C4(int slot));                   // char-block byte<<14
HOST_STUB(int   _080056DC(int slot));                   // screen-block byte<<11
HOST_STUB(int   _08005B5C(int v));
HOST_STUB(int   _08005988(u32 idx, void *dst));
HOST_STUB(void  _08024A98(void));
HOST_STUB(int   _08002178(int i));
HOST_STUB(void  _0800F7C0(void *rec));
HOST_STUB(u32   _0800F700(int a, int b));
extern void sub_0800F7C0(void *rec);
extern u32 sub_0800F700(int a, int b);
HOST_STUB(void  _0800E650(void *rec, u32 arg));
HOST_STUB(void  _0800D854(void *rec));
HOST_STUB(void  _0800D8E4(void *rec));
HOST_STUB(void  _0800EF54(void *rec));
HOST_STUB(void  _0800F22C(void *rec));
HOST_STUB(void  _0800F5A0(void *rec));
HOST_STUB(void  _0800EBD8(void *rec, u32 a, u32 b));
HOST_STUB(void  _0800ECAC(void *rec, u32 a, u32 b));
HOST_STUB(void  _0800E7CC(void *rec));
HOST_STUB(void  _0800E7A0(void *rec));
HOST_STUB(void  _0800EE00(void *rec, u32 a, u32 b));
HOST_STUB(void  _08007770(int a, void *b, int c, int d, u32 e, u32 f));
HOST_STUB(void  sub_0800C4D0(void *rec));
HOST_STUB(void  _0800C604(void *rec));
HOST_STUB(void  _0800C640(void *rec));
HOST_STUB(void  _0800C658(void *rec));
HOST_STUB(void  _0800C668(int rec, int value));
HOST_STUB(void  _0800C6CC(int rec, int value));
HOST_STUB(void  _0800C730(void *rec, u16 value));
HOST_STUB(void  _080218F8(void *inst));
HOST_STUB(void  _080219D0(void *inst));
HOST_STUB(void  _0800218F8(void *inst));   // closure spelling of 0x080218F8
HOST_STUB(void  _0800219D0(void *inst));   // closure spelling of 0x080219D0
HOST_STUB(void  _08021AFC(void *inst));
HOST_STUB(void  _08021B38(void *inst));
HOST_STUB(void  _0800DBE8(void *rec));
HOST_STUB(void  _0800D97C(void *rec, int reload));      // runtime: menu_stage
HOST_STUB(void  _0802CC34(void *row, void *arg));
HOST_STUB(void  _0802C488(void *rec));
HOST_STUB(void  _0802C11C(void *chan, void *unused));   // voice-list cleanup

static inline int div8000_trunc(int v) {
    // ROM pattern for /0x8000 (2^15) via two shifts with trunc-toward-zero
    // bias: q = v >> 11; if (q < 0) q += 15; return q >> 4.
    // (asm/course_stream.s 0x08006A50, asm/course_stream_6650.s 0x080068D4:
    //  asrs r0,#11; cmp #0; bge +0; adds #15; asrs #4.)
    int q = v >> 11;
    if (q < 0) q += 15;
    return q >> 4;
}

// ============================================================================
// saveblock.s 0x08024B18 — `movs r0,#1; bx lr`
// ============================================================================
int SaveRet1_24B18(void) { return 1; }
int _08024B18(void) __attribute__((alias("SaveRet1_24B18")));
int sub_08024B18(void) __attribute__((alias("SaveRet1_24B18")));
int SaveRet1_24B1C(void) { return 1; }
int _08024B1C(void) __attribute__((alias("SaveRet1_24B1C")));
int sub_08024B1C(void) __attribute__((alias("SaveRet1_24B1C")));
int SaveRet1_24B20(void) { return 1; }
int _08024B20(void) __attribute__((alias("SaveRet1_24B20")));
int sub_08024B20(void) __attribute__((alias("SaveRet1_24B20")));

// ============================================================================
// code_4b9c.s 0x08004BAC — scene-manager substate-flag OR leaf
// (r0 = scene-manager record at *(0x03000198); sets bit0 of the u16 at +4)
// ============================================================================
void SubstateOr1_04BAC(void *ctx) {
    volatile u16 *f = (volatile u16 *)((u8 *)ctx + 4);
    u16 v = 1;
    v |= *f;
    *f = v;
}
__asm__(".align 2, 0");
void _08004BAC(void *c) __attribute__((alias("SubstateOr1_04BAC")));
void sub_08004BAC(void *c) __attribute__((alias("SubstateOr1_04BAC")));

// ============================================================================
// carphys_racer.s 0x08021BA0 — ev-7 per-car-id dispatch (raw.incbin pocket;
// transcribed from objdump of baserom.gba @0x21BA0..0x21BF8).
// ============================================================================
void CarEv7Dispatch_21BA0(void *inst) {
    s16 id;
    _0800D97C((u8 *)inst + 176, 15);
    id = *(s16 *)((u8 *)inst + 192);
    switch (id) {
    case 0: _0800218F8(inst); break;
    case 1: _0800219D0(inst); break;
    case 2: _08021AFC(inst); break;
    case 3: _08021B38(inst); break;
    default: break;
    }
    _0800DBE8((u8 *)inst + 56);
}
void _08021BA0(void *i) __attribute__((alias("CarEv7Dispatch_21BA0")));
void sub_08021BA0(void *i) __attribute__((alias("CarEv7Dispatch_21BA0")));

// ============================================================================
// code_c668.s 0x0800C744 — selector in r0 indexes a 21-entry `mov pc` table
// @0x0800C75C after subtracting one. r1 is preserved in r2 as the forwarded
// value; r3 is the record passed unchanged to the selected helper. Cases:
// 1→C4D0, 3→C604, 4→C640, 8→C658, 13→C668(r3,r1), 14→C6CC(r3,r1),
// 21→C730(r3,(u16)r1); default→shared pop tail 0x0800C7EE.
// ============================================================================
void MenuC744(int mode, int value, int unused, void *rec) {
    switch (mode) {
    case 1:  sub_0800C4D0(rec); break;
    case 3:  _0800C604(rec); break;
    case 4:  _0800C640(rec); break;
    case 8:  _0800C658(rec); break;
    case 13: _0800C668((int)(uintptr_t)rec, value); break;
    case 14: _0800C6CC((int)(uintptr_t)rec, value); break;
    case 21: _0800C730(rec, (u16)value); break;
    default: break;
    }
    (void)unused;
}
void _0800C744(int mode, int value, int unused, void *rec) __attribute__((alias("MenuC744")));
void sub_0800C744(int mode, int value, int unused, void *rec) __attribute__((alias("MenuC744")));
// The ROM span ends on a word boundary. Keep its 0x0000 filler after this
// body's .size so the byte-level candidate carries the same final halfword.
__asm__(".align 2, 0");

// 1. OPERAND ORDER (the 31/36). agbcc materialised the constant first and the
//    address second, emitting `movs r1,#1; ldr r0,=flag; strb r1,[r0]` — the
//    exact mirror image of the ROM's `ldr r1,=flag; movs r0,#1; strb r0,[r1]`.
//    Dropping the pointer variable and writing the store against a bare address
//    expression flips the allocation to the ROM's (movs r0,#1; strb r0,[r1]),
//    but scores the same 31/36: see (2). Both halves are needed.
//
// 2. ADDRESS FOLD (31/36 -> 36/36, +0x14 -> none). With the bare literal,
//    agbcc's ARM reload pass folded the flag address into `adds r1,r2,#2` off
//    the countdown base (candidate shrank to 32 bytes, first diff +0x15). A
//    bare integer literal that happens to be `base + offset` of another
//    constant is exactly what that pass merges. Routing the SAME address through
//    an assembler-defined SYMBOL_REF (`B16FlagVMA`, below) gives the pass nothing
//    to merge and restores the pool load `ldr r1,[pc,#8]`. This is the repo's
//    documented lever, src/runtime_state_dispatch.c:638 SoundC5C0_BankCheck, which notes
//    `simplify_rtx` folds an integer literal into the `plus` but not a
//    SYMBOL_REF, and a symbol_ref is a real insn the reload pass cannot delete.
//    CONTROL (non-vacuity): perturbing only the asm literal to 0x0300176D leaves
//    the ENTIRE instruction stream byte-identical and moves the sole diff to
//    +0x20, the pool word. So the symbol reference is what buys the `ldr`, and
//    the literal is what supplies the pool entry; neither is decoration.
extern const u8 B16FlagVMA[];
void SoundD9FC_Latch(void) {
    volatile u16 *cnt = (volatile u16 *)0x0300176A;
    int t = *cnt;
    if (t != 0) {
        t = *cnt - 1;
        *cnt = (u16)t;
        __asm__(".globl B16FlagVMA\nB16FlagVMA = 0x0300176C\n");
        if ((t << 16) == 0) *(volatile u8 *)(uintptr_t)B16FlagVMA = 1;
    }
}
void _0802D9FC(void) __attribute__((alias("SoundD9FC_Latch")));
void sub_0802D9FC(void) __attribute__((alias("SoundD9FC_Latch")));

// ============================================================================
// handlers.s 0x08000CA0 (480 B) -- SIO multiboot session stepper.
// Record 0x0203F150: +0 mode, +1 state (jump table over 0..4, inline at +0x24),
// +2 tick count, +3 done flag, +4 u32 *step table, +8 s32 level, +12 s32 acc,
// +16 s32 sum, +20 s32 index.  The lone argument is state 2's level-out pointer.
// Every arm falls into the shared `count++ / return 0` tail; states 3 and 4
// share the `return 1` exit.
//
// Load-bearing spellings (each measured; the rest is plain C):
//  * the record is a pinned r5 reached through a pinned-r0 base load, so the
//    entry reads state through r0 and only then copies it to r5;
//  * state 1 reads `mode` twice -- once as the scratch test and once into r4,
//    which is also what `IME = mode` stores;
//  * REG_IE in state 1 is SIOCNT + 216 and the two SIODATA32 words in state 3
//    are REG_IE - 224; spelling either as its own constant adds a pool word;
//  * the +16/+20 field pair must be volatile: the ROM reloads the index once
//    per iteration and stores it back through the base copy in r3, while the
//    back-edge test keeps the incremented register (`subs r2,#224` and the
//    `do` form below are what stop agbcc rolling the loop into one register);
//  * the two zeroed words need one pinned zero each (r0, then r1).
// ============================================================================
int SessionStep_CA0(u32 *levelOut) {
    register u8 *base __asm__("r0") = (u8 *)0x0203F150;
    register u8 *rec __asm__("r5");
    register s32 v __asm__("r6");
    register s32 cl __asm__("r4");
    s32 i;
    register s32 mode __asm__("r4");
    u8 state;

    state = base[1];
    rec = base;
    switch (state) {
    case 0:
        if ((*(u32 *)rec & 0xFF00FFu) != 0)
            rec[1] = 1;
        break;
    case 1:
        if (rec[0] == 1) {
            if (rec[2] <= 5)
                break;
        } else {
            *(volatile u16 *)0x04000128u = 0x1000;
        }
        *(volatile u32 *)0x04000120u = 0;
        *(volatile u16 *)0x04000202u = 0xC0;
        mode = rec[0];
        if (mode == 1) {
            *(volatile u16 *)0x04000128u |= 0x80;
            *(volatile u32 *)0x0400010Cu = 0x00C0F318u;
            *(volatile u16 *)0x04000208u = 0;
            *(volatile u16 *)((u8 *)0x04000128u + 216) |= 0x40;
            *(volatile u16 *)0x04000208u = mode;
        } else {
            *(volatile u16 *)0x04000128u |= 0x4080;
            *(volatile u16 *)0x04000208u = 0;
            *(volatile u16 *)((u8 *)0x04000128u + 216) |= 0x80;
            *(volatile u16 *)0x04000208u = 1;
        }
        rec[2] = 0;
        rec[1] = 2;
        break;
    case 2:
        v = *(s32 *)(rec + 8);
        cl = v;
        if (v > 0x2000)
            cl = 0x2000;
        else if (v < 0)
            cl = 0;
        if (levelOut != 0)
            *levelOut = cl;
        if (rec[0] != 1) {
        {
            register s32 pre __asm__("r0") = *(volatile s32 *)(rec + 20);
            if (pre < cl) {
                register u8 *rc __asm__("r3") = rec;
                s32 *tbl = *(s32 **)(rec + 4);
                do {
                    i = *(volatile s32 *)(rc + 20);
                    *(volatile s32 *)(rc + 16) += tbl[i];
                    i = i + 1;
                    *(volatile s32 *)(rc + 20) = i;
                } while (i < cl);
            }
        }
            if (v > 0x2000) {
                if ((*(s32 *)(rec + 12) += *(s32 *)(rec + 16)) == -1)
                    rec[3] = 1;
            }
        }
        if (v > 0x2000 || rec[2] == 0x8Cu)
            rec[1] = 3;
        break;
    case 3:
        *(volatile u16 *)0x04000208u = 0;
        {
            u8 *ie = (u8 *)0x04000200u;
            register s32 z __asm__("r0");
            *(volatile u16 *)ie &= 0xFF3F;
            *(volatile u16 *)0x04000208u = 1;
            *(volatile u16 *)0x04000128u = 0x1000;
            z = 0x2000;
            *(volatile u32 *)0x04000128u = z;
            z += 3;
            *(volatile u32 *)0x04000128u = z;
            volatile u32 *buf = (volatile u32 *)(ie - 224);
            register u32 b0 __asm__("r0") = 0;
            __asm__ volatile("" : "+r"(b0));
            register u32 b1 __asm__("r1") = 0;
            __asm__ volatile("" : "+r"(b1));
            buf[0] = b0;
            buf[1] = b1;
            if (rec[0] != 0)
                *(volatile u32 *)0x0400010Cu = 0;
            *(volatile u16 *)0x04000202u = 0xC0;
        }
        if (rec[0] == 0)
            return 1;
        rec[2] = 0;
        rec[1] = 4;
        break;
    case 4:
        if (rec[2] > 2)
            return 1;
        break;
    }
    rec[2]++;
    return 0;
}
int _08000CA0(u32 a) __attribute__((alias("SessionStep_CA0")));
int SubCA0(u32 a) __attribute__((alias("SessionStep_CA0")));
int sub_08000CA0(u32 a) __attribute__((alias("SessionStep_CA0")));

// 0x08006A50 (156 B) — nearby-record collector: clear the count/cursor cells,
// scale both tile coordinates, then walk the course root's record array.
// Plain C, no instruction splices. Two measured spellings (2026-10-26):
//  * The tile coordinates are the *parameters*: the ROM's entry copies
//    (`adds r7,r0,#0` / `adds r6,r1,#0`) are the allocator's homes for values
//    live across the loop's calls. A local `tileX = camX` leaves the pre-shift
//    value a short-lived local pseudo and it lands in r3/r4 instead.
//  * The loop-tail `movs r2,#46 / ldrsh r0,[r0,r2]` scratch is a liveness pin:
//    a dummy in r3, output-only before the load and input-only after, moves
//    the scratch off r3 onto the ROM's r2.
void CollectNearby_06A50(int tileX, int tileY) {
    extern volatile u32 NearbyCountCell_06A50[];
    extern volatile u16 NearbyCursorCell_06A50[];
    extern volatile u32 CourseRootCell_06A50[];
    extern volatile u32 NearbyRecordArray_06A50[];
    register s32 index __asm__("r5");
    register volatile u8 *record __asm__("r4");
    register volatile u8 *table __asm__("r0");
    register volatile u32 *countCell __asm__("r2");
    register volatile u16 *cursorCell __asm__("r0");
    register volatile u32 *activeCount __asm__("r8");
    register volatile u32 *countTemp __asm__("r3");
    register volatile u32 *nearby __asm__("r9");
    register volatile u32 *rootCell __asm__("r1");
    register s32 loopBound __asm__("r0");
    register s32 recordY __asm__("r0");
    register u32 zero __asm__("r1");
    register u32 offset __asm__("r1");
    s32 count;
    s32 dx;
    s32 dy;
    s32 shifted;
    s32 slot;

    __asm__(".globl NearbyCountCell_06A50\nNearbyCountCell_06A50 = 0x0203F8A8\n");
    __asm__(".globl NearbyCursorCell_06A50\nNearbyCursorCell_06A50 = 0x0203F874\n");
    __asm__(".globl CourseRootCell_06A50\nCourseRootCell_06A50 = 0x0203F760\n");
    __asm__(".globl NearbyRecordArray_06A50\nNearbyRecordArray_06A50 = 0x0203F880\n");

    countCell = NearbyCountCell_06A50;
    zero = 0;
    *countCell = zero;
    cursorCell = (volatile u16 *)NearbyCursorCell_06A50;
    *cursorCell = (u16)zero;

    tileX = (tileX >> 11) / 16;
    {
        register s32 tileShiftedY __asm__("r0") = tileY >> 11;
        if (tileShiftedY < 0) tileShiftedY += 15;
        tileY = tileShiftedY >> 4;
    }

    index = 0;
    rootCell = CourseRootCell_06A50;
    count = *(s16 *)(*(volatile u32 *)rootCell + 46);
    if (index < count) {
        nearby = NearbyRecordArray_06A50;
        activeCount = countCell;
        do {
            u32 root = *rootCell;
            offset = index << 3;
            table = (volatile u8 *)(*(volatile u32 *)(root + 16));
            __asm__ volatile ("" : "+r"(table));
            record = table + offset;
            dx = tileX - *(s16 *)(record + 0);
            if (_08005B5C(dx) <= 1) {
                recordY = *(s16 *)(record + 2);
                dy = tileY - recordY;
                if (_08005B5C(dy) <= 1) {
                    countTemp = activeCount;
                    slot = *countTemp;
                    nearby[slot] = (u32)(uintptr_t)record;
                    *countTemp = (u32)(slot + 1);
                }
            }
            index++;
            rootCell = CourseRootCell_06A50;
            loopBound = *rootCell;
            {
                register u32 hold3 __asm__("r3");
                __asm__ volatile("" : "=r"(hold3));
                loopBound = *(s16 *)((u8 *)(uintptr_t)loopBound + 46);
                __asm__ volatile("" : : "r"(hold3));
            }
            count = loopBound;
        } while (index < count);
    }
}
void _08006A50(int a, int b) __attribute__((alias("CollectNearby_06A50")));
void sub_08006A50(int a, int b) __attribute__((alias("CollectNearby_06A50")));

// ============================================================================
// code_24ac.s 0x0800254C — screen-tile initializer.
// 0x0802D974 = CpuSet swi 0x0B wrapper (r0=src, r1=dst, r2=ctrl);
// 0x08004EFC = bump alloc (returns 1KB arena, warns on overflow);
// 0x080056C4/DC = BG slot char/screen-block bases;
// fills: 0x01000010, tile patterns 0x1111/0x2222, palette copy ctrl
// 0x04000018, header pair 0x6739/0x7BDE, palette dest 0x04000008,
// char-block rows ctrl 0x04000010; 19 rows × 30 cols checker = ((r+c)&1)+1.
// ============================================================================
void ScreenInit_0254C(void) {
    u16 scratch[4];
    volatile u8 *r6 = (volatile u8 *)(uintptr_t)_08004EFC(1024);

    scratch[0] = 0;
    _0802D974((const void *)&scratch[0], (void *)r6, 0x01000010);
    u16 *fill1 = &scratch[1];
    *fill1 = 0x1111;
    _0802D974((const void *)fill1, (void *)(r6 + 32), 0x01000010);
    u16 *fill2 = &scratch[2];
    *fill2 = 0x2222;
    _0802D974((const void *)fill2, (void *)(r6 + 64), 0x01000010);
    _0802D974((const void *)r6,
              (void *)(uintptr_t)_080056C4(0), 0x04000018);

    *(volatile u16 *)(r6 + 0) = 0;
    *(volatile u16 *)(r6 + 2) = 0x6739;
    *(volatile u16 *)(r6 + 4) = 0x7BDE;
    _0802D974((const void *)r6, (void *)(uintptr_t)0x05000000, 0x04000008);

    int row = 0;
    for (;;) {
        int col = 0;
        int rowOffset = row * 64;
        int nextRow = row + 1;
        u8 *p = (u8 *)r6;
        for (; col <= 29; col++) {
            u16 v = (u16)(((row + col) & 1) + 1);
            *(volatile u16 *)(uintptr_t)p = v;
            p += 2;
        }
        _0802D974((const void *)r6,
                  (void *)(uintptr_t)(_080056DC(0) + rowOffset), 0x04000010);
        row = nextRow;
        if (row > 19) break;
    }
}
void _0800254C(void) __attribute__((alias("ScreenInit_0254C")));
void sub_0800254C(void) __attribute__((alias("ScreenInit_0254C")));

// ============================================================================
// code_23e0c.s 0x080023E78 — bx lr stub
// ============================================================================
void Stub_23E78(void *rec) { (void)rec; }
__asm__(".align 2, 0");
void _080023E78(void *a) __attribute__((alias("Stub_23E78")));
void sub_080023E78(void *a) __attribute__((alias("Stub_23E78")));

// ============================================================================
// sound_voice_tick.s 0x0802CE20 — voice param tick (per-channel struct)
//   [+0] flags, [+8] lo, [+9] hi, [+10..13] step s8, [+14..15] env,
//   [+16] L out, [+17] R out, [+18] bal, [+19] vol, [+20] pan L, [+21] pan R,
//   [+22] pan, [+24] mode.
// Bit0: amplitude/pan → out bytes; bit4: env → [+8]/[+9]; flags ← 0xFA.
// The channel is the second source argument (r1) in this call convention.
// Signed RAM fields use `s8 *` lvalues so old_agbcc selects the ROM's indexed
// LDRSB form; volatile signed-byte lvalues lower to a longer LDRB/extend pair.
// ============================================================================
void VoiceParamTick_2CE20(void *ch, void *unused)
{
    register volatile u8 *c __asm__("r2") = (volatile u8 *)unused;
    register u32 test __asm__("r0") = 1;
    register u32 initialFlags __asm__("r1") = c[0];
    test &= initialFlags;
    if (test != 0) {
        register u32 level __asm__("r3") = c[19];
        register u32 factor __asm__("r1") = c[18];
        register u32 product __asm__("r0") = level;
        __asm__ volatile ("" : "+r"(level));
        __asm__ volatile ("" : "+r"(product));
        product *= factor;
        __asm__ volatile ("" : "+r"(product));
        level = product >> 5;
        register s32 mode __asm__("r4") = c[24];
        if (mode == 1) {
            register s32 volumePan __asm__("r0") = ((s8 *)c)[22];
            volumePan += 128;
            volumePan *= (s32)level;
            level = (u32)volumePan >> 7;
        }
        {
            register s32 left __asm__("r0") = ((s8 *)c)[20];
            register s32 pan __asm__("r1");
            left <<= 1;
            pan = ((s8 *)c)[21];
            pan = left + pan;
            if (mode == 2) {
                register s32 adjust __asm__("r0") = ((s8 *)c)[22];
                pan += adjust;
            }
            if (pan < -128) {
                pan = -128;
                goto pan_ready;
            }
            if (pan > 127) pan = 127;
pan_ready:
            {
                register u32 out __asm__("r0") = (u32)(pan + 128);
                out *= level;
                out >>= 8;
                *(volatile u8 *)(c + 16) = (u8)out;
            }
            {
                register u32 out __asm__("r0") = (u32)(127 - pan);
                out *= level;
                out >>= 8;
                *(volatile u8 *)(c + 17) = (u8)out;
            }
        }
    }
    {
        register u32 envFlags __asm__("r1") = c[0];
        register u32 testEnv __asm__("r0") = 4;
        register u32 savedFlags __asm__("r3");
        testEnv &= envFlags;
        savedFlags = envFlags;
        if (testEnv != 0) {
            register s32 product __asm__("r0") = ((s8 *)c)[14];
            register u32 factor __asm__("r1") = c[15];
            product *= factor;
            register s32 env __asm__("r1") = ((s8 *)c)[12];
            env += product;
            env <<= 2;
            {
                register s32 part __asm__("r0") = ((s8 *)c)[10];
                part <<= 8;
                env += part;
            }
            {
                register s32 part __asm__("r0") = ((s8 *)c)[11];
                part <<= 8;
                env += part;
            }
            env += c[13];
            if (c[24] == 0) {
                register s32 part __asm__("r0") = ((s8 *)c)[22];
                part <<= 4;
                env += part;
            }
            *(volatile u8 *)(c + 8) = (u8)(env >> 8);
            *(volatile u8 *)(c + 9) = (u8)env;
        }
        {
            register u32 clear __asm__("r0") = 250;
            clear &= savedFlags;
            *(volatile u8 *)c = (u8)clear;
        }
    }
    (void)ch;
}
void _0802CE20(void *a, void *b) __attribute__((alias("VoiceParamTick_2CE20")));
void sub_0802CE20(void *a, void *b) __attribute__((alias("VoiceParamTick_2CE20")));

// ============================================================================
// sound_voice_apply.s 0x0802C160 — voice envelope apply (moved to
// src/sound_interp_ced4.c so its file-scope fixed r4/r5 register declarations
// sit at the top of a TU with no other r4/r5-saving functions).
// ============================================================================

// ============================================================================
// sound_veneer.s 0x0802DDC8 — bx r0 veneer slot (4 bytes: bx r0 + nop pad).
// Weak stubs here and in agbmain_session.c coexist (both no-ops).
//
// CONFIRMED NON-ENTRY  — do not lift, do not promote.
// 0x0802DDC8..0x0802DE03 is the armcc register-branch veneer POOL, not code:
// fifteen 4-byte slots `bx r0`, `bx r1`, `bx r2`,... `bx lr`, each followed
// by the halfword pad `46 c0` (`mov r8,r8`). Read straight out of
// build/era-corpus/code.lst, which lists VMAs directly:
//   802ddc8: 4700  bx r0        802ddcc: 4708  bx r1... 802de00: 4770  bx lr
//   802ddca: 46c0  mov r8,r8    802ddce: 46c0  mov r8,r8... 802de02: 46c0  mov r8,r8
// They are reached as `ldr rN, <ram_addr>; bl veneer[rN]` to run code relocated
// to RAM (src/foundation_subsys.c; asm/sound_veneer.s carries the caller
// census). C returns through `bx lr`, so each ARM body spells its indirect
// branch directly and retains the pool's four-byte slot with the pad halfword.
// ==========================================================================
#define DEFINE_SOUND_VENEER(name, reg) \
    __attribute__((naked)) void name(void) { \
        __asm__ volatile ("bx " reg "\n\t.hword 0x46c0"); \
    }
DEFINE_SOUND_VENEER(Veneer_2DDC8, "r0")
DEFINE_SOUND_VENEER(Veneer_2DDCC, "r1")
DEFINE_SOUND_VENEER(Veneer_2DDD0, "r2")
DEFINE_SOUND_VENEER(Veneer_2DDD4, "r3")
DEFINE_SOUND_VENEER(Veneer_2DDD8, "r4")
DEFINE_SOUND_VENEER(Veneer_2DDDC, "r5")
DEFINE_SOUND_VENEER(Veneer_2DDE0, "r6")
DEFINE_SOUND_VENEER(Veneer_2DDE4, "r7")
DEFINE_SOUND_VENEER(Veneer_2DDE8, "r8")
DEFINE_SOUND_VENEER(Veneer_2DDEC, "r9")
DEFINE_SOUND_VENEER(Veneer_2DDF0, "r10")
DEFINE_SOUND_VENEER(Veneer_2DDF4, "r11")
DEFINE_SOUND_VENEER(Veneer_2DDF8, "r12")
DEFINE_SOUND_VENEER(Veneer_2DDFC, "r13")
DEFINE_SOUND_VENEER(Veneer_2DE00, "r14")
#undef DEFINE_SOUND_VENEER
void _0802DDC8(void) __attribute__((alias("Veneer_2DDC8")));
void sub_0802DDC8(void) __attribute__((alias("Veneer_2DDC8")));
void _0802DDCC(void) __attribute__((alias("Veneer_2DDCC")));
void _0802DDD0(void) __attribute__((alias("Veneer_2DDD0")));
void _0802DDD4(void) __attribute__((alias("Veneer_2DDD4")));
void _0802DDD8(void) __attribute__((alias("Veneer_2DDD8")));
void _0802DDDC(void) __attribute__((alias("Veneer_2DDDC")));
void _0802DDE0(void) __attribute__((alias("Veneer_2DDE0")));
void _0802DDE4(void) __attribute__((alias("Veneer_2DDE4")));
void _0802DDE8(void) __attribute__((alias("Veneer_2DDE8")));
void _0802DDEC(void) __attribute__((alias("Veneer_2DDEC")));
void _0802DDF0(void) __attribute__((alias("Veneer_2DDF0")));
void _0802DDF4(void) __attribute__((alias("Veneer_2DDF4")));
void _0802DDF8(void) __attribute__((alias("Veneer_2DDF8")));
void _0802DDFC(void) __attribute__((alias("Veneer_2DDFC")));
void _0802DE00(void) __attribute__((alias("Veneer_2DE00")));

// ============================================================================
// sound_reset_more.s 0x0802CACC — sound FSM stepper over
// root = *(u32 *)0x03007FF0. State word root[0] == "Smsh" (0x68736D53);
// when hit: increment,
// zero 12 stride-64 channel cells at +80, re-arm 4 voice cells [+28] via
// 0x0802DDCC (r0=idx 1..4, r1=[+44]), reset state to "Smsh".
// ============================================================================
void SoundCACC_Step(void) {
    register volatile u32 * volatile *cell __asm__("r0") =
        (volatile u32 * volatile *)0x03007FF0u;
    register volatile u32 *root __asm__("r6") = *cell;
    register u32 state __asm__("r1") = root[0];
    if (state != 0x68736D53u) return;
    register u32 bumped __asm__("r0") = state + 1;
    root[0] = bumped;
    register int count __asm__("r5") = 12;
    register volatile u8 *cursor __asm__("r4") = (volatile u8 *)root + 80;
    register int zero __asm__("r0") = 0;
    do {
        *cursor = (u8)zero;
        --count;
        cursor += 64;
    } while (count > 0);
    register u32 voice __asm__("r4") = root[7];
    u32 voicePresent = voice;
    if (voicePresent != 0) {
        register int index __asm__("r5") = 1;
        int voiceZero = 0;
        do {
            register u32 arg __asm__("r0") = (u8)index;
            __asm__ volatile("" : "+r" (arg));
            ((void (*)(u32))(uintptr_t)root[11])(arg);
            *(volatile u8 *)(uintptr_t)voice = (u8)voiceZero;
            ++index;
            voice += 64u;
        } while (index <= 4);
    }
    root[0] = 0x68736D53u;
}
void _0802CACC(void) __attribute__((alias("SoundCACC_Step")));
void sub_0802CACC(void) __attribute__((alias("SoundCACC_Step")));

// 1. THREE SOURCE BUGS, not just shape (43 → 47 matched):
//    * `(arg << 16) >> 13` on an `int` is an ARITHMETIC shift. The ROM's
//      0x0B40 is `lsrs r0,r0,#13`. The value is non-negative either way, so
//      `(u32)arg` is the honest spelling and matches the bytes.
//    * `& ~1` is gone. The ROM has no mask, and the old candidate emitted none
//      either — the term was already dead, so dropping it is byte-neutral.
//    * The two call sites had the wrong operands. ROM +0x3E is
//      `adds r1,r3,#0` with r3 = row[0] = the claim, so the `state == 0` arm
//      passes `held`, not `state`; and the +0x606 test is `cmp r2,#0` with
//      r2 = row[1] = `state`, not `held`.
//    * The `u16 != 0` test is a direct HALFWORD LOAD (ROM +0x36
//      `ldrh r0,[r1,#4]`), so it reads memory, not a register copy of it
//      (`(u16)state` emitted `lsls r0,r2,#16` instead).
//    * `row[0]` is read before `rec[0]`: ROM +0x18 `ldr r3,[r1,#0]` precedes
//      +0x1A `ldr r2,[r0,#0]`.
//
// 2. THE TWO POOL LOADS (47 → 58 matched, prefix 4 → 5). ROM issues BOTH
//    `ldr r2,=0x08061F74` and `ldr r1,=0x08061FA4` at +0x04/+0x06, before the
//    shift. With folded integer literals the reload pass sinks the first one
//    past the shift. This is the repo's documented lever, src/course_cal.c:8-22:
//    the base must be a `symbol_ref` leaf, because `simplify_rtx` folds an
//    integer literal into the `plus` but not a `SYMBOL_REF`, and a symbol_ref is
//    a real insn the reload pass cannot delete. Control: both bases as
//    symbol_refs without the statement split is still 47/84 prefix 4.
//
// 3. STATEMENT SPLIT + REGISTER PIN (58 → 63, prefix 5 → 58). `hi = arg<<16`
//    must be its own initialised declaration so its `lsls` lands at +0x02,
//    before both pool loads, and `rowtbl` is PINNED to r2 because the ROM's
//    row-table constant lives in r2 while the claim index lands in r3. Controls
//    on this step, all measured:
//      no pin, same split (re-measured on the FINAL body) 79/84 prefix 5
//      pin, `hi` assigned after the bases instead of at its
//        declaration                                53/84 prefix 2
//      pin r2 AND pin r1 (two pins, even in separate statements) 25/84,
//        and the span shrinks to 80 — the two-pin failure, so only one is used.
//
// 4. ARM ORDER (63 → 84 EXACT). The ROM branches OVER the `_0802CC34` arm:
// `cmp r0,#0 / bne +0x606`, with the `state < 0` test out of line at +0x606.
// Spelled `if (halfword != 0) {...} else {...}` agbcc inlines the OTHER arm
// (`beq` to the `_0802CC34` block) and the shape is 63/84 at prefix 58.
// Inverting the outer test to `== 0` is what selects the ROM's fall-through.
// Two `goto` spellings of the same CFG are also exact (both measured 84/84),
// so this is the block-layout choice and not a fragile coincidence.
// ============================================================================
extern const u8 C5C0RowTbl[];
extern const u8 C5C0RecBase[];
void SoundC5C0_BankCheck(int arg) {
    volatile u8 *rec;
    volatile u32 *row;
    register u32 rowtbl __asm__("r2");
    u32 hi = (u32)arg << 16;
    __asm__(".globl C5C0RowTbl\nC5C0RowTbl = 0x08061F74\n.globl C5C0RecBase\nC5C0RecBase = 0x08061FA4\n");
    rowtbl = (u32)(uintptr_t)C5C0RowTbl;
    u32 recbase = (u32)(uintptr_t)C5C0RecBase;
    rec = (volatile u8 *)(uintptr_t)(recbase + (hi >> 13));
    row = *(volatile u32 **)(rowtbl + (*(volatile u16 *)(rec + 4)) * 12);
    u32 claim = row[0];
    u32 held = *(volatile u32 *)(rec + 0);
    if (claim != held) {
        _0802CC34((void *)row, (void *)(uintptr_t)held);
        return;
    }
    u32 state = row[1];
    if (*(volatile u16 *)((volatile u8 *)row + 4) == 0) {
        _0802CC34((void *)row, (void *)(uintptr_t)held);
    } else {
        if ((s32)state < 0) _0802C488((void *)row);
    }
}
void _0802C5C0(int a) __attribute__((alias("SoundC5C0_BankCheck")));
void sub_0802C5C0(int a) __attribute__((alias("SoundC5C0_BankCheck")));

// 0x0802C574 — replace the selected stream when its source or state requires it.
void SoundC574_Replace(int arg) {
    extern const u8 C574RowTbl[];
    extern const u8 C574RecBase[];
    volatile u8 *rec;
    volatile u32 *row;
    register u32 rowtbl __asm__("r2");
    u32 hi = (u32)arg << 16;
    __asm__(".globl C574RowTbl\nC574RowTbl = 0x08061F74\n.globl C574RecBase\nC574RecBase = 0x08061FA4\n");
    rowtbl = (u32)(uintptr_t)C574RowTbl;
    u32 recbase = (u32)(uintptr_t)C574RecBase;
    rec = (volatile u8 *)(uintptr_t)(recbase + (hi >> 13));
    row = *(volatile u32 **)(rowtbl + (*(volatile u16 *)(rec + 4)) * 12);
    u32 claim = row[0];
    u32 held = *(volatile u32 *)(rec + 0);
    if (claim != held) {
        _0802CC34((void *)row, (void *)(uintptr_t)held);
        return;
    }
    u32 state = row[1];
    if (*(volatile u16 *)((volatile u8 *)row + 4) != 0 && (s32)state >= 0)
        return;
    _0802CC34((void *)row, (void *)(uintptr_t)claim);
}
void _0802C574(int a) __attribute__((alias("SoundC574_Replace")));
void sub_0802C574(int a) __attribute__((alias("SoundC574_Replace")));

static inline int WA16s(int off) { return (int)*(volatile s16 *)(0x03001780 + off); }
static inline volatile u16 *WA16p(int off) { return (volatile u16 *)(0x03001780 + off); }

// 0x0800FE90 — s16[WA+0xFBC] state table: writes rec+214 and passes the state
// word as the third argument of _08007770.
// Two spellings the allocator needs: the dispatch load is written off the
// symbol with the offset in a local *before* `wa` is bound, which is what makes
// agbcc load the base into r1 and the offset into r4 (then copy the base down
// to r4 after the indexed load, as the ROM does); binding `wa` first gives the
// two pool loads in the opposite registers and drops the copy. The state word
// is pinned to r2 because agbcc otherwise homes it in r4.
void MenuFE90(void *rec) {
    u8 *r = (u8 *)rec;
    register s16 n __asm__("r2");
    volatile u8 *wa;
    s32 sel;
    u32 off;

    RAM_DEF_03001780;
    off = 0xFBC;
    sel = *(s16 *)(RAM_SYM_03001780 + off);
    wa = RAM_SYM_03001780;
    switch (sel) {
    case 0:
        n = 4;
        if (*(s16 *)(wa + 0x103A) <= 2) *(volatile u16 *)(r + 214) = 1;
        else *(volatile u16 *)(r + 214) = 0;
        break;
    case 5: n = 3; *(volatile u16 *)(r + 214) = 5; break;
    case 1: n = 4; *(volatile u16 *)(r + 214) = 2; break;
    case 2: n = 4; *(volatile u16 *)(r + 214) = 3; break;
    case 3: n = 4; *(volatile u16 *)(r + 214) = n; break;
    case 7:
        switch (*(s16 *)(wa + 0x1078)) {
        case 1: n = 3; *(volatile u16 *)(r + 214) = 6; break;
        case 2: n = 4; *(volatile u16 *)(r + 214) = 7; break;
        }
        break;
    default: break;
    }
    _08007770(0, (void *)0x082D9EF8, n, 0, 4, 1);
}
void _0800FE90(void *a) __attribute__((alias("MenuFE90")));
void sub_0800FE90(void *a) __attribute__((alias("MenuFE90")));

// ============================================================================
// menu_f924.s 0x0800F9A0 — result scatter (rec +0xAC, +0xC0, F700 distance,
// +0xB0, +0xC8, +0xC6 → WA +0x576, +0xFE4, +0xFE6, +0xFEC, +0xFE8,
// +0xFEA; u16[rec+0xDC]==1 → _0800F7C0(rec)).
// ============================================================================
void MenuF9A0_Scatter(void *rec) {
    u8 *r = (u8 *)rec;
    /* Pin the WA base so C pointer additions keep the ROM's operand order. */
    register volatile u8 *wa __asm__("r4") = (volatile u8 *)0x03001780;
    register u32 offset __asm__("r2");
    {
        register u16 value __asm__("r1") = *(volatile u16 *)(r + 172);
        register volatile u16 *dst __asm__("r0");
        offset = 0x576;
        dst = (volatile u16 *)(wa + offset);
        *dst = value;
    }
    {
        register u16 value __asm__("r1") = *(volatile u16 *)(r + 192);
        register volatile u16 *dst __asm__("r0");
        offset = 0xFE4;
        dst = (volatile u16 *)(wa + offset);
        *dst = value;
    }
    {
        /* Pinned zero indices preserve both register-offset signed loads. */
        register u8 *xBase __asm__("r0") = r + 188;
        register s32 xZero __asm__("r1") = 0;
        register s32 x __asm__("r0");
        x = *(s16 *)(xBase + xZero);
        register u8 *yBase __asm__("r1") = r + 190;
        register s32 yZero __asm__("r2") = 0;
        register s32 y __asm__("r1");
        y = *(s16 *)(yBase + yZero);
        register u32 dist __asm__("r0") = sub_0800F700(x, y);
        {
            register volatile u16 *dst __asm__("r1");
            offset = 0xFE6;
            dst = (volatile u16 *)(wa + offset);
            *dst = (u16)dist;
        }
    }
    {
        register u16 value __asm__("r1") = *(volatile u16 *)(r + 176);
        register volatile u16 *dst __asm__("r0");
        offset += 6;
        dst = (volatile u16 *)(wa + offset);
        *dst = value;
    }
    {
        register u16 value __asm__("r1") = *(volatile u16 *)(r + 200);
        register volatile u16 *dst __asm__("r0");
        offset -= 4;
        dst = (volatile u16 *)(wa + offset);
        *dst = value;
    }
    {
        register u16 value __asm__("r0") = *(volatile u16 *)(r + 198);
        register u32 offset __asm__("r1") = 0xFEA;
        wa += offset;
        *(volatile u16 *)wa = value;
    }
    if (*(volatile u16 *)(r + 220) == 1) sub_0800F7C0(rec);
}
void _0800F9A0(void *a) __attribute__((alias("MenuF9A0_Scatter")));
void sub_0800F9A0(void *a) __attribute__((alias("MenuF9A0_Scatter")));

// ============================================================================
// menu_f5a0.s 0x0800F5EC — record dispatch (12-way on arg−1; jump table
// @0x0800F608; case0 → E650; case4 → D854(rec+0x28)/D8E4(rec+0x90)/EF54;
// case5 → F5A0 + u16[rec+0x2C]-gated s16[rec+0xE4] variant switch
// (0→EBD8(a,r2,r3), 2→ECAC, 1→EE00); case11 → E7CC tail target of
// table slot 12 → F22C; cases 2,3,7,8,9,10 → tail).
// ============================================================================
void MenuF5EC(int ev, u32 a, u32 b, void *rec) {
    switch (ev) {
    case 2: _0800E650(rec, a); break;
    case 5:
        _0800D854((u8 *)rec + 40);
        _0800D8E4((u8 *)rec + 144);
        _0800EF54(rec);
        break;
    case 7: _0800F22C(rec); break;
    case 6: {
        _0800F5A0(rec);
        if (*(volatile u16 *)((u8 *)rec + 44) == 0) break;
        {
            int v = (int)*(s16 *)((u8 *)rec + 228);
            switch (v) {
            case 0: _0800EBD8(rec, (u16)a, (u16)b); break;
            case 2: _0800ECAC(rec, (u16)a, (u16)b); break;
            case 1: _0800EE00(rec, (u16)a, (u16)b); break;
            default: break;
            }
        }
        break;
    }
    case 1: _0800E7CC(rec); break;
    case 12: _0800E7A0(rec); break;
    default: break;
    }
}
__asm__(".align 2, 0");
void _0800F5EC(int ev, u32 a, u32 b, void *rec) __attribute__((alias("MenuF5EC")));
void sub_0800F5EC(int ev, u32 a, u32 b, void *rec) __attribute__((alias("MenuF5EC")));

// ============================================================================
// menu_f22c.s 0x0800F59C — bx lr stub
// ============================================================================
void Stub_0F59C(void *rec) { (void)rec; }
__asm__(".align 2, 0");
void _0800F59C(void *a) __attribute__((alias("Stub_0F59C")));
void sub_0800F59C(void *a) __attribute__((alias("Stub_0F59C")));

__attribute__((naked)) void CollectProximity_068D4(void) {
    __asm__ volatile (
        ".syntax unified\n"
        ".L068d4_080068D4:\n"
        "  push {r4, r5, r6, r7, lr}\n"
        "  mov r7, sl\n"
        "  mov r6, r9\n"
        "  mov r5, r8\n"
        "  push {r5, r6, r7}\n"
        "  ldr r1, .L068d4_08006948\n"
        "  movs r2, #192\n"
        "  lsls r2, r2, #4\n"
        "  adds r0, r2, #0\n"
        "  ldrh r1, [r1]\n"
        "  subs r0, r0, r1\n"
        "  ldr r3, .L068d4_0800694C\n"
        "  adds r1, r3, #0\n"
        "  ands r0, r1\n"
        "  adds r0, #64\n"
        "  asrs r4, r0, #7\n"
        "  movs r0, #31\n"
        "  ands r4, r0\n"
        "  ldr r2, .L068d4_08006950\n"
        "  ldr r1, .L068d4_08006954\n"
        "  ldr r7, [r1]\n"
        "  mov ip, r7\n"
        "  mov r3, ip\n"
        "  asrs r0, r3, #11\n"
        "  cmp r0, #0\n"
        "  bge .L068d4_0800690A\n"
        "  adds r0, #15\n"
        ".L068d4_0800690A:\n"
        "  asrs r0, r0, #4\n"
        "  str r0, [r2]\n"
        "  ldr r7, [r1, #4]\n"
        "  asrs r0, r7, #11\n"
        "  cmp r0, #0\n"
        "  bge .L068d4_08006918\n"
        "  adds r0, #15\n"
        ".L068d4_08006918:\n"
        "  asrs r0, r0, #4\n"
        "  str r0, [r2, #4]\n"
        "  ldr r2, .L068d4_08006958\n"
        "  lsls r4, r4, #16\n"
        "  asrs r1, r4, #13\n"
        "  adds r0, r1, r2\n"
        "  ldr r3, [r0]\n"
        "  mov r0, ip\n"
        "  subs r3, r0, r3\n"
        "  adds r5, r3, #0\n"
        "  adds r2, #4\n"
        "  adds r1, r1, r2\n"
        "  ldr r0, [r1]\n"
        "  subs r6, r7, r0\n"
        "  cmp r3, #0\n"
        "  bge .L068d4_0800695C\n"
        "  asrs r0, r5, #11\n"
        "  cmp r0, #0\n"
        "  bge .L068d4_08006940\n"
        "  adds r0, #15\n"
        ".L068d4_08006940:\n"
        "  asrs r0, r0, #4\n"
        "  subs r5, r0, #1\n"
        "  b .L068d4_08006966\n"
        "  .align 2, 0\n"
        ".L068d4_08006948: .word 0x0203F728\n"
        ".L068d4_0800694C: .word 0x00000FFF\n"
        ".L068d4_08006950: .word 0x0203F8E0\n"
        ".L068d4_08006954: .word 0x0203F6B0\n"
        ".L068d4_08006958: .word 0x0805DBF4\n"
        ".L068d4_0800695C:\n"
        "  asrs r0, r5, #11\n"
        "  cmp r0, #0\n"
        "  bge .L068d4_08006964\n"
        "  adds r0, #15\n"
        ".L068d4_08006964:\n"
        "  asrs r5, r0, #4\n"
        ".L068d4_08006966:\n"
        "  cmp r6, #0\n"
        "  bge .L068d4_08006978\n"
        "  asrs r0, r6, #11\n"
        "  cmp r0, #0\n"
        "  bge .L068d4_08006972\n"
        "  adds r0, #15\n"
        ".L068d4_08006972:\n"
        "  asrs r0, r0, #4\n"
        "  subs r6, r0, #1\n"
        "  b .L068d4_08006982\n"
        ".L068d4_08006978:\n"
        "  asrs r0, r6, #11\n"
        "  cmp r0, #0\n"
        "  bge .L068d4_08006980\n"
        "  adds r0, #15\n"
        ".L068d4_08006980:\n"
        "  asrs r6, r0, #4\n"
        ".L068d4_08006982:\n"
        "  ldr r0, .L068d4_08006A38\n"
        "  str r5, [r0]\n"
        "  str r6, [r0, #4]\n"
        "  asrs r1, r4, #16\n"
        "  movs r0, #200\n"
        "  muls r1, r0\n"
        "  ldr r0, .L068d4_08006A3C\n"
        "  adds r1, r1, r0\n"
        "  mov ip, r1\n"
        "  ldr r2, .L068d4_08006A40\n"
        "  movs r1, #0\n"
        "  strh r1, [r2]\n"
        "  ldr r0, .L068d4_08006A44\n"
        "  strh r1, [r0]\n"
        "  mov r1, ip\n"
        "  movs r0, #0\n"
        "  ldrsb r0, [r1, r0]\n"
        "  cmp r0, #0\n"
        "  blt .L068d4_08006A2A\n"
        "  movs r3, #7\n"
        "  mov r8, r3\n"
        "  ldr r7, .L068d4_08006A48\n"
        "  mov r9, r7\n"
        "  movs r0, #4\n"
        "  add r0, r9\n"
        "  mov sl, r0\n"
        "  mov r4, ip\n"
        ".L068d4_080069B8:\n"
        "  movs r0, #0\n"
        "  ldrsb r0, [r4, r0]\n"
        "  adds r2, r0, r5\n"
        "  movs r0, #1\n"
        "  ldrsb r0, [r4, r0]\n"
        "  adds r3, r0, r6\n"
        "  adds r1, r3, #0\n"
        "  mov r7, r8\n"
        "  ands r1, r7\n"
        "  lsls r1, r1, #3\n"
        "  adds r0, r2, #0\n"
        "  ands r0, r7\n"
        "  orrs r1, r0\n"
        "  lsls r1, r1, #2\n"
        "  ldr r0, .L068d4_08006A4C\n"
        "  adds r1, r1, r0\n"
        "  movs r0, #0\n"
        "  ldrsb r0, [r1, r0]\n"
        "  cmp r0, r2\n"
        "  bne .L068d4_080069E8\n"
        "  movs r0, #1\n"
        "  ldrsb r0, [r1, r0]\n"
        "  cmp r0, r3\n"
        "  beq .L068d4_08006A18\n"
        ".L068d4_080069E8:\n"
        "  strb r2, [r1]\n"
        "  strb r3, [r1, #1]\n"
        "  ldr r7, .L068d4_08006A40\n"
        "  movs r0, #0\n"
        "  ldrsh r1, [r7, r0]\n"
        "  lsls r1, r1, #3\n"
        "  add r1, r9\n"
        "  lsls r0, r2, #24\n"
        "  asrs r0, r0, #24\n"
        "  str r0, [r1]\n"
        "  movs r2, #0\n"
        "  ldrsh r1, [r7, r2]\n"
        "  lsls r1, r1, #3\n"
        "  add r1, sl\n"
        "  lsls r0, r3, #24\n"
        "  asrs r0, r0, #24\n"
        "  str r0, [r1]\n"
        "  ldrh r0, [r7]\n"
        "  adds r0, #1\n"
        "  strh r0, [r7]\n"
        "  lsls r0, r0, #16\n"
        "  asrs r0, r0, #16\n"
        "  cmp r0, #3\n"
        "  bgt .L068d4_08006A2A\n"
        ".L068d4_08006A18:\n"
        "  adds r4, #4\n"
        "  mov r0, ip\n"
        "  adds r0, #196\n"
        "  cmp r4, r0\n"
        "  bgt .L068d4_08006A2A\n"
        "  movs r0, #0\n"
        "  ldrsb r0, [r4, r0]\n"
        "  cmp r0, #0\n"
        "  bge .L068d4_080069B8\n"
        ".L068d4_08006A2A:\n"
        "  pop {r3, r4, r5}\n"
        "  mov r8, r3\n"
        "  mov r9, r4\n"
        "  mov sl, r5\n"
        "  pop {r4, r5, r6, r7}\n"
        "  pop {r0}\n"
        "  bx r0\n"
        "  .align 2, 0\n"
        ".L068d4_08006A38: .word 0x0203F8B0\n"
        ".L068d4_08006A3C: .word 0x0805DCF4\n"
        ".L068d4_08006A40: .word 0x0203F870\n"
        ".L068d4_08006A44: .word 0x0203F8A4\n"
        ".L068d4_08006A48: .word 0x0203F8C0\n"
        ".L068d4_08006A4C: .word 0x0203F770\n"
        ".syntax divided\n"
    );
}

void _080068D4(void) __attribute__((alias("CollectProximity_068D4")));
void sub_080068D4(void) __attribute__((alias("CollectProximity_068D4")));

// ============================================================================
// resource_wrap.s 0x080263C4 — resource wrapper: r = Wrap26230(a, c);
// Wrap262A4(r, b); return r. Per asm/resource_wrap.s: r1<-c BEFORE the
// first call (so the composite's second arg feeds Seek's index), r4 =
// result, r1<-b for the second call, returns r4. (WrapA/WrapB bodies
// lifted in runtime_record_helpers.c.)
// ============================================================================
HOST_STUB(void *_08026230(int idx, void *ptr));
HOST_STUB(void  _080262A4(void *rec, int sel));

void *ResourceWrapper_263C4(int a, void *b, int c) {
    void *r = sub_08026230(a, (void *)(uintptr_t)c);
    sub_080262A4(r, (int)(uintptr_t)b);
    return r;
}
__asm__(".align 2, 0");
void *_080263C4(int a, void *b, int c) __attribute__((alias("ResourceWrapper_263C4")));
void *sub_080263C4(int a, void *b, int c) __attribute__((alias("ResourceWrapper_263C4")));
void *Course_0x080263C4(int a, void *b, int c) __attribute__((alias("ResourceWrapper_263C4"))); /* trampoline elimination: friendly-name spelling used by race_cluster.c */

// ============================================================================
// sound_stop.s 0x0802CB84 — stop/clear current sequence state (root at
// *(0x03007FF0), "Smsh" magic; FIFO 0x040000C6 = 0xB600; byte[+4] = 0;
// state -= 10).
// ============================================================================
void SoundStop_2CB84(void *unused) {
    (void)unused;
    volatile u32 *rootp = (volatile u32 *)0x03007FF0;
    volatile u32 *root = (volatile u32 *)(uintptr_t)*rootp;
    u32 state = root[0];
    if (state == 0x68736D53u) return;
    *(volatile u16 *)0x040000C6 = 0xB600;
    {
        volatile u8 *flag = (volatile u8 *)root + 4;
        u8 v = *flag;
        v = 0;
        *flag = v;
    }
    root[0] = state - 10;
}
void _0802CB84(void *a) __attribute__((alias("SoundStop_2CB84")));
void sub_0802CB84(void *a) __attribute__((alias("SoundStop_2CB84")));

// ============================================================================
// sound_start.s 0x0802CC34 — stream start/configuration helper
// (rec = r0, cfg = r1; magic "Smsh" gate at rec+52; channel setup loop over
// stride-80 records at rec+44 with _0802C11C cleanup; tail resets magic).
// ============================================================================
HOST_STUB(void _0802CA34(void *s));

// The state gate, setup writes, two channel walks, and byte/table loads are C.
// Empty register barriers preserve old_agbcc's register choices; whole-span
// and isolated-slice matches gate the body.
void SoundStart_2CC34(void *rec, void *cfg) {
    register volatile u8 *state __asm__("r5") = (volatile u8 *)rec;
    volatile u8 *config = (volatile u8 *)cfg;
    register s32 i __asm__("r6");
    register volatile u8 *channel __asm__("r4");
    register volatile u8 *head __asm__("r1");
    register u32 channelIndex __asm__("r8");
    register u32 startMode __asm__("r0");
    register u32 count __asm__("r2");
    register s32 initialLimit __asm__("r1");
    register u32 flagTest __asm__("r0");
    register s32 stateLimit __asm__("r1");

    if (*(volatile u32 *)(state + 52) != 0x68736D53u)
        return;
    startMode = state[11];
    count = config[2];

    if (startMode != 0) {
        if (*(volatile u32 *)state != 0) {
            head = *(volatile u8 * volatile *)(state + 44);
            flagTest = 64;
            flagTest &= *head;
            if (flagTest != 0)
                goto check_limit;
        }
        {
            u32 whole = *(volatile u32 *)(state + 4);
            u16 low = *(volatile u16 *)(state + 4);
            if (low == 0 || (s32)whole < 0)
                goto start;
        }
check_limit:
        {
            register u32 reloadedCount __asm__("r0") = config[2];
            __asm__ volatile ("" : "+r"(reloadedCount));
            count = reloadedCount;
            __asm__ volatile ("" : "+r"(count));
        }
        if (state[9] > count)
            return;
    }

start:
    *(volatile u32 *)(state + 52) += 1;
    *(volatile u32 *)(state + 4) = 0;
    *(volatile u8 * volatile *)state = (volatile u8 *)config;
    *(volatile u32 *)(state + 48) = *(volatile u32 *)(config + 4);
    state[9] = (u8)count;
    *(volatile u32 *)(state + 12) = 0;
    {
        u32 volume = 150;
        *(volatile u16 *)(state + 28) = (u16)volume;
        *(volatile u16 *)(state + 32) = (u16)volume;
        volume += 106;
        *(volatile u16 *)(state + 30) = (u16)volume;
    }
    *(volatile u16 *)(state + 34) = 0;
    *(volatile u16 *)(state + 36) = 0;

    i = 0;
    channel = *(volatile u8 * volatile *)(state + 44);
    initialLimit = config[0];
    __asm__ volatile ("" : "+r"(initialLimit));
    if (i >= initialLimit)
        goto data_tail;
    if (i >= state[8])
        goto tail_done;
    channelIndex = (u32)i;
    __asm__ volatile ("" : "+r"(channelIndex));
data_loop:
        _0802C11C((void *)state, (void *)channel);
        channel[0] = 192;
        {
            register u32 storedIndex __asm__("r1") = channelIndex;
            __asm__ volatile ("" : "+r"(storedIndex));
            *(volatile u32 *)(channel + 32) = storedIndex;
        }
        {
            register u32 lookupOffset __asm__("r1") = (u32)i << 2;
            register volatile u8 *lookupBase __asm__("r0") = config;
            __asm__ volatile ("" : "+r"(lookupOffset));
            __asm__ volatile ("" : "+r"(lookupBase));
            lookupBase += 8;
            lookupBase += lookupOffset;
            startMode = *(volatile u32 *)lookupBase;
        }
        *(volatile u32 *)(channel + 64) = startMode;
        i++;
        channel += 80;
        startMode = config[0];
        if (i >= (s32)startMode)
            goto data_tail;
        stateLimit = state[8];
        __asm__ volatile ("" : "+r"(stateLimit));
        if (i < stateLimit)
            goto data_loop;

data_tail:
    if (i >= state[8])
        goto tail_done;
    {
        register u32 zeroIndex __asm__("r1") = 0;
        __asm__ volatile ("" : "+r"(zeroIndex));
        channelIndex = zeroIndex;
    }
    __asm__ volatile ("" : "+r"(channelIndex));
    do {
        _0802C11C((void *)state, (void *)channel);
        channel[0] = (u8)channelIndex;
        i++;
        channel += 80;
        stateLimit = state[8];
        __asm__ volatile ("" : "+r"(stateLimit));
    } while (i < stateLimit);

tail_done:
    flagTest = 128;
    flagTest &= config[3];
    if (flagTest != 0) {
        startMode = config[3];
        _0802CA34((void *)(uintptr_t)startMode);
    }
    *(volatile u32 *)(state + 52) = 0x68736D53u;
}
void _0802CC34(void *a, void *b) __attribute__((alias("SoundStart_2CC34")));
void sub_0802CC34(void *a, void *b) __attribute__((alias("SoundStart_2CC34")));

// ============================================================================
// sound_stop_all.s 0x0802CD18 — stop all active streams (magic gate;
// state |= 0x80000000; per-channel _0802C11C cleanup loop; magic restored).
// ============================================================================
void SoundStopAll_2CD18(void *rec) {
    u8 *s = (u8 *)rec;
    // ROM: load the magic once, compare, store magic+1, and re-materialize the
    // constant for the final restore. The loop count is a plain int so the
    // decrement stays `subs r4,#1` (`bgt`), not the `u8` narrowing sequence,
    // and `rec` is passed through unchanged (`adds r0,r6,#0`).
    u32 m = *(volatile u32 *)(s + 52);
    if (m != 0x68736D53u) return;
    *(volatile u32 *)(s + 52) = m + 1;
    *(volatile u32 *)(s + 4) |= 0x80000000u;
    int n = s[8];
    u8 *ch = (u8 *)(uintptr_t)*(volatile u32 *)(s + 44);
    while (n > 0) {
        _0802C11C(s, ch);
        n--;
        ch += 80;
    }
    *(volatile u32 *)(s + 52) = 0x68736D53u;
}
void _0802CD18(void *a) __attribute__((alias("SoundStopAll_2CD18")));
void sub_0802CD18(void *a) __attribute__((alias("SoundStopAll_2CD18")));

// ============================================================================
// sound_tick.s 0x0802C738 — per-voice flag scan (flags&0xC0 = active+64;
// _0802C8B0 VCounter tick; ROM stores [+0]=0x80, [+15]=2, [+19]=64,
// [+25]=22, and [+36]=1 after the call).
// ============================================================================
// Arity is deliberate: the ROM materialises r0 before the call (asm/sound_tick.s:26
// `adds r0, r4, #0`, asm/sound_alloc.s:28 `adds r0, r7, #0`), and C cannot express an
// incoming-but-unread r0 — agbcc only preserves an incoming r0 by declaring it arg0.
// _0802C8B0 never reads it, so no caller can observe a behaviour change.
HOST_STUB(void _0802C8B0(void *self));

void SoundTick_2C738(void *root) {
    volatile u8 *rootp = (volatile u8 *)root;
    register int n asm("r5") = rootp[8];
    register volatile u8 *r4 asm("r4") =
        (volatile u8 *)(uintptr_t)*(volatile u32 *)(rootp + 44);
    if (n <= 0) return;
    int active = 0x80;
    do {
        register int f asm("r1") = r4[0];
        register int gate asm("r0") = active;
        gate &= f;
        if (gate == 0) goto next_voice;
        register int tick asm("r6") = 0x40;
        gate = tick;
        gate &= f;
        if (gate == 0) goto next_voice;
        _0802C8B0((void *)(uintptr_t)r4);
        r4[0] = active;
        r4[15] = 2;
        r4[19] = tick;
        r4[25] = 22;
        r4[36] = 1;
    next_voice:
        n--;
        r4 += 80;
    } while (n > 0);
}
void _0802C738(void *a) __attribute__((alias("SoundTick_2C738")));
void sub_0802C738(void *a) __attribute__((alias("SoundTick_2C738")));

// ============================================================================
// sound_uaeabi_uidiv.s 0x0802DF6C — __aeabi_uidiv (bit-by-bit restoring
// division; zero divisor routes through the _0802DE98 no-op leaf → 0).
// NOT PROMOTABLE: ARMCC runtime (`mov pc, lr` / `pop {pc}` returns, lazy
// single-register frame) like the rest of the EABI division block
// (build/scan_armcc.py); byte-exact via asm/sound_uaeabi_uidiv.s.
// ============================================================================
HOST_STUB(void _0802DE98(void));

__attribute__((naked)) u32 UDiv_2DF6C(u32 n, u32 d) {
    __asm__ volatile (
        ".syntax unified\n"
        ".Ludiv__0802DF6C:\n"
        "cmp r1, #0\n"
        "beq.n .Ludiv__0802DFDA\n"
        "movs r3, #1\n"
        "movs r2, #0\n"
        "push {r4}\n"
        "cmp r0, r1\n"
        "bcc.n .Ludiv__0802DFD4\n"
        "movs r4, #1\n"
        "lsls r4, r4, #28\n"
        ".Ludiv__0802DF7E:\n"
        "cmp r1, r4\n"
        "bcs.n .Ludiv__0802DF8C\n"
        "cmp r1, r0\n"
        "bcs.n .Ludiv__0802DF8C\n"
        "lsls r1, r1, #4\n"
        "lsls r3, r3, #4\n"
        "b.n .Ludiv__0802DF7E\n"
        ".Ludiv__0802DF8C:\n"
        "lsls r4, r4, #3\n"
        ".Ludiv__0802DF8E:\n"
        "cmp r1, r4\n"
        "bcs.n .Ludiv__0802DF9C\n"
        "cmp r1, r0\n"
        "bcs.n .Ludiv__0802DF9C\n"
        "lsls r1, r1, #1\n"
        "lsls r3, r3, #1\n"
        "b.n .Ludiv__0802DF8E\n"
        ".Ludiv__0802DF9C:\n"
        "cmp r0, r1\n"
        "bcc.n .Ludiv__0802DFA4\n"
        "subs r0, r0, r1\n"
        "orrs r2, r3\n"
        ".Ludiv__0802DFA4:\n"
        "lsrs r4, r1, #1\n"
        "cmp r0, r4\n"
        "bcc.n .Ludiv__0802DFB0\n"
        "subs r0, r0, r4\n"
        "lsrs r4, r3, #1\n"
        "orrs r2, r4\n"
        ".Ludiv__0802DFB0:\n"
        "lsrs r4, r1, #2\n"
        "cmp r0, r4\n"
        "bcc.n .Ludiv__0802DFBC\n"
        "subs r0, r0, r4\n"
        "lsrs r4, r3, #2\n"
        "orrs r2, r4\n"
        ".Ludiv__0802DFBC:\n"
        "lsrs r4, r1, #3\n"
        "cmp r0, r4\n"
        "bcc.n .Ludiv__0802DFC8\n"
        "subs r0, r0, r4\n"
        "lsrs r4, r3, #3\n"
        "orrs r2, r4\n"
        ".Ludiv__0802DFC8:\n"
        "cmp r0, #0\n"
        "beq.n .Ludiv__0802DFD4\n"
        "lsrs r3, r3, #4\n"
        "beq.n .Ludiv__0802DFD4\n"
        "lsrs r1, r1, #4\n"
        "b.n .Ludiv__0802DF9C\n"
        ".Ludiv__0802DFD4:\n"
        "adds r0, r2, #0\n"
        "pop {r4}\n"
        "mov pc, lr\n"
        ".Ludiv__0802DFDA:\n"
        "push {lr}\n"
        "bl 0x0802DE98\n"
        "movs r0, #0\n"
        "pop {pc}\n"
        ".syntax divided\n"
    );
}
u32 _0802DF6C(u32 a, u32 b) __attribute__((alias("UDiv_2DF6C")));
u32 sub_0802DF6C(u32 a, u32 b) __attribute__((alias("UDiv_2DF6C")));

int AiGridGet_25CF4(int type, int row, int col) {
    u8 tmp[4];
    _0802E0A4((void *)tmp, (const void *)0x08060D48, 4);
    if (col > 10) return 0;
    if (col < 0) return 0;
    int idx = 44 * type + 11 * row + col;
    int rem = _0802D97C(idx, 4);
    int sh = _0802D97C(idx, 4);
    const u8 *base;
    // Absolute symbol declared INSIDE the body: the slice link splices only this
    // body's own section, so a file-scope definition would not travel with it
    // (the WA_SPLIT_DECL rule of src/race_scene_b1.c:141-156).
    extern u8 B16GridBase[] __asm__("B16GridBase");
    __asm__(".globl B16GridBase\nB16GridBase = 0x03001780\n");
    base = (const u8 *)(uintptr_t)B16GridBase;
    int q = (idx < 0) ? (idx + 3) : idx;
    // 0x03001780 is IWRAM WORK AREA — the packed collection grid at the very
    // start of the work area, not a hardware register — and the ROM reads it
    // exactly once with a bare `ldrb`, with no intervening store. `volatile`
    // would assert a hardware contract this access does not have; it is
    // byte-neutral here (EXACT measured both with and without) and is dropped.
    // The sibling SETTER, Code25930_GridSet in src/code_25930.c, does keep
    // `volatile` — it writes through the same base and its reads/writes must not
    // be folded. That qualifier is untouched.
    const u8 *bp = base + (q >> 2);
    u8 v = *bp;
    u8 m = tmp[rem];
    u8 out = (u8)(v & m);
    return (int)(u8)(out >> (sh * 2));
}
int _08025CF4(int a, int b, int c) __attribute__((alias("AiGridGet_25CF4")));
int Sub_08025CF4(int a, int b, int c) __attribute__((alias("AiGridGet_25CF4")));
int sub_08025CF4(int a, int b, int c) __attribute__((alias("AiGridGet_25CF4")));

// ============================================================================
// menu_dispatch.s 0x0800C168 — record-class ctor A `{0x0800C169, 0x1DE8}`
// (src/foundation_subsys.c), a 14-entry tail-call dispatch on the incoming r0.
// Registers at entry: r0 = id, r1 = arg, r2 = unused, r3 = rec. The ROM opens
// `adds r2, r1, #0` (arg parked in the callee-saved-slot register used by the
// single two-argument arm) then `subs r0, #1`, so slot = id − 1 and the case
// values are 1/3/4/8/14 — that min/max is what keeps agbcc emitting
// `subs r0, #1` / `cmp r0, #13` / the 14-word table. Every arm forwards r3 as
// the callee's first argument; the 14 arm also passes the parked arg as r1:
// 1→C010(rec), 3→C0F0, 4→C110, 8→C128, 14→C138(rec, arg).
// ============================================================================
HOST_STUB(void _0800C010(void *rec));
HOST_STUB(void _0800C0F0(void *rec));
HOST_STUB(void _0800C110(void *rec));
HOST_STUB(void _0800C128(void *rec));
HOST_STUB(void _0800C138(void *rec, u32 sel));

void MenuDispatch_C168(int id, u32 arg, int unused_slot, void *rec) {
    switch (id) {
    case 1:  _0800C010(rec); break;
    case 3:  _0800C0F0(rec); break;
    case 4:  _0800C110(rec); break;
    case 8:  _0800C128(rec); break;
    case 14: _0800C138(rec, arg); break;
    default: break;
    }
}
void _0800C168(int a, u32 b, int c, void *d) __attribute__((alias("MenuDispatch_C168")));
void sub_0800C168(int a, u32 b, int c, void *d) __attribute__((alias("MenuDispatch_C168")));

// ============================================================================
// menu_cfe4.s 0x0800CFE4 — menu record initializer (scene mgr getter, cursor
// store, tree-loader resource attach, obj create + attr/enable/center chain).
// ============================================================================
HOST_STUB(void *_08004B68(void));
HOST_STUB(int   _08004CA8(int v));
HOST_STUB(void  _08007664(void *a, void *b, int c));
HOST_STUB(void *_08026948(void));
HOST_STUB(void  _08026A4C(void *a, int b, int c));
HOST_STUB(void  _08026A58(void *a, u8 b));
HOST_STUB(void  _08026A60(void *a, u8 b));
HOST_STUB(void  _08026938(void *a, int b));
#define MCI_CALLEE(friendly, closure) closure
extern void *sub_08004B68(void);
extern void *_080026948(void);
extern void _080026938(void *obj, int slot);

void MenuInit_CFE4(void *rec) {
    register u8 *r4 __asm__("r4") = (u8 *)rec;
    MCI_CALLEE(_08004B68, sub_08004B68)();
    *(volatile u16 *)(r4 + 0) = (u16)_08004CA8(0);
    _08007664((void *)0, (void *)(uintptr_t)0x08292B40u, 6);
    {
        register u32 waBase __asm__("r0") = 0x03001780u;
        register u32 waOffset __asm__("r1") = 0x574u;
        __asm__ volatile("" : "+r"(waBase), "+r"(waOffset));
        waBase += waOffset;
        *(volatile u16 *)(r4 + 6) = *(volatile u16 *)(uintptr_t)waBase;
    }
    void *obj = MCI_CALLEE(_08026948, _080026948)();
    *(volatile u32 *)(r4 + 28) = (u32)(uintptr_t)obj;
    {
        // Plain indexed reads: r4 (the record) stays live for the later
        // field accesses, which is what makes agbcc take the index form
        // (`movs rX,#imm / ldrsh rY,[r4,rX]`, oracle `_08001A74`) instead
        // of folding the offset into the address.
        register int arg1 __asm__("r1") = *(s16 *)((u8 *)r4 + 6);
        register int arg2 __asm__("r2") = *(s16 *)((u8 *)r4 + 8);
        _08026A4C(obj, arg1, arg2);
    }
    _08026A58((void *)(uintptr_t)*(volatile u32 *)(r4 + 28), 1);
    _08026A60((void *)(uintptr_t)*(volatile u32 *)(r4 + 28), 0);
    {
        register void *objAt28 __asm__("r0") =
            (void *)(uintptr_t)*(volatile u32 *)(r4 + 28);
        register int arg1 __asm__("r1") = *(s16 *)((u8 *)r4 + 6);
    MCI_CALLEE(_08026938, _080026938)(objAt28, arg1);
    }
}
void _0800CFE4(void *a) __attribute__((alias("MenuInit_CFE4")));
void sub_0800CFE4(void *a) __attribute__((alias("MenuInit_CFE4")));

// ============================================================================
// code_2730.s 0x08002730 — s16 grid reader (base 0x03001780+0x109C,
// offset = r1*2 + r0*8; returns s16[base + offset]); 0x0800274C — gate
// writer (sel 0 → 0x1056=0, else 0x1056=1; then _0800279C(0));
// 0x08002784 — sign test ((−v | v) >> 31).
// ============================================================================
HOST_STUB(void _0800279C(int v));

s16 GridS16_2730(int a, int b) {
    extern u8 GridBase[] __asm__("GridBase");
    __asm__(".globl GridBase\nGridBase = 0x03001780\n");
    u32 base = (u32)(uintptr_t)GridBase;
    int off = (b << 1) + (a << 3);
    u32 disp = 0x109Cu;
    u32 idx = 0;
    u32 addr = (u32)off + (base + disp);
    return ((s16 *)(addr + idx))[idx];
}
s16 _08002730(int a, int b) __attribute__((alias("GridS16_2730")));
s16 sub_08002730(int a, int b) __attribute__((alias("GridS16_2730")));

void GridGateWrite_274C(int sel) {
    // Keep sel in r1 and the branch-local base/offset adds separate: the ROM
    // has duplicate base/offset pool words and one shared halfword store.
    register u32 storeValue __asm__("r1") = (u32)sel;
    register uintptr_t cell __asm__("r0");
    if (storeValue == 0u) {
        register uintptr_t off __asm__("r2");
        cell = 0x03001780u;
        __asm__("" : "+r" (cell));
        off = 0x1056u;
        __asm__("" : "+r" (off));
        cell += off;
        __asm__("" : "+r" (cell));
    } else {
        cell = 0x03001780u;
        __asm__("" : "+r" (cell));
        storeValue = 0x1056u;
        __asm__("" : "+r" (storeValue));
        cell += storeValue;
        __asm__("" : "+r" (cell));
        storeValue = 1;
    }
    *(volatile u16 *)cell = (u16)storeValue;
    _0800279C(0);
}
void _0800274C(int a) __attribute__((alias("GridGateWrite_274C")));
void Sub_0800274C(int a) __attribute__((alias("GridGateWrite_274C")));
void sub_0800274C(int a) __attribute__((alias("GridGateWrite_274C")));

int GridSign_2784(void) {
    s16 v = *(volatile s16 *)(0x03001780 + 0x1056);
    return (int)((u32)(-(s32)v | (u32)v) >> 31);
}
int _08002784(void) __attribute__((alias("GridSign_2784")));
int sub_08002784(void) __attribute__((alias("GridSign_2784")));

// THREE levers, each with a removal control:
//
// SCENE-ID LOAD: the ROM's `movs r1,#2; ldrsh r0,[r0,r1]` is an s16
// SUBSCRIPT of index 1 — `mb[1]` on an `s16 *` — not a byte offset. agbcc
// pre-scales the constant index into the index register (`movs r1,#2`),
// producing the register-offset `ldrsh` in 4 bytes. Writing it as a byte
// offset instead forces agbcc to normalise the signed load through
// `ldrh; lsls #16; asrs #16` (6 bytes), or — with `volatile` — through an
// explicit `adds` plus a zero index register. Every byte-offset spelling
// measured 4 bytes OVER.
HOST_STUB(void _080056F4(void *s, int idx, int val));

void CarRecEventReset_22CB4(void *a, void *b) {
    u8 *r4 = (u8 *)b;
    u8 *r5 = (u8 *)a;

    u8 *gp = (u8 *)b + 88;
    u16 zero = 0;
    *(volatile u8 *)gp = (u8)zero;
    *(volatile u16 *)(r5 + 144) = zero;
    // Gate address as TWO pool words + a runtime `adds`. Each half is an
    // assembler SYMBOL_REF, so `simplify_rtx` cannot fold `0x03001780 + 0x10C3`
    // into the single literal 0x03002843 (it folds an integer literal `plus`,
    // but a symbol_ref is a real insn the reload pass cannot merge).
    extern u8 B16GateBase[] __asm__("B16GateBase");
    extern u8 B16GateOff[]  __asm__("B16GateOff");
    __asm__(".globl B16GateBase\nB16GateBase = 0x03001780\n"
            ".globl B16GateOff\nB16GateOff = 0x10C3\n");
    const u8 *gate = B16GateBase + (uintptr_t)B16GateOff;
    if (*gate == 0) {
        _080056F4(r4, 1, 1);
        *(volatile u16 *)(r5 + 144) = 1;
    }
    s16 *mb = (s16 *)(uintptr_t)_08004B68();
    int scene = (int)mb[1];
    switch (scene) {
    case 15:
    case 35: *(volatile u16 *)(r4 + 84) = 5; break;
    case 40: *(volatile u16 *)(r4 + 84) = 1; break;
    default: *(volatile u16 *)(r4 + 84) = 6; break;
    }
}
void _08022CB4(void *a, void *b) __attribute__((alias("CarRecEventReset_22CB4")));
void sub_08022CB4(void *a, void *b) __attribute__((alias("CarRecEventReset_22CB4")));
// The closure (asm/code_22cb4.s) spells this address with NINE hex digits --
// `sub_080022CB4` / `_080022CB4` -- and those are the names the manifest entry
// exports, so a promoted caller binds to those symbols. The 8-digit twins above
// are a different symbol at the same address and do not satisfy it.
void _080022CB4(void *a, void *b) __attribute__((alias("CarRecEventReset_22CB4")));
void sub_080022CB4(void *a, void *b) __attribute__((alias("CarRecEventReset_22CB4")));

// ROM entry alias.
u32 _08002DF6C(u32 n, u32 d) __attribute__((alias("UDiv_2DF6C")));

// ROM entry alias.
void _08023E78(void *rec) __attribute__((alias("Stub_23E78")));
