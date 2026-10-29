// ============================================================================
// menu_ff78_c.c — reconstructed C for asm/menu_ff78.s no-op `bx lr` stubs +
// screen-setup cursor handlers.
//
//   sub_080012574 (0x080012574) — bx lr no-op (4B)
//   sub_080012850 (0x080012850) — bx lr no-op (4B)
//   sub_080012F14 (0x080012F14) — bx lr no-op (4B)
//   sub_0800133E0 (0x0800133E0) — bx lr no-op (4B)
//   sub_080013E60 (0x080013E60) — bx lr no-op (4B)
//   sub_080014074 (0x080014074) — bx lr no-op (4B; tail of sub_080014072)
//   sub_0800141A8 (0x0800141A8) — bx lr no-op (4B)
//   sub_080014BCC (0x080014BCC) — bx lr no-op (4B)
//   sub_080014C84 (0x080014C84) — bx lr no-op (4B)
//   sub_080015CB0 (0x080015CB0) — bx lr no-op (4B; rec35_driver.s `bl 0x08015CB0`)
//
//   sub_080010F3C (0x080010F3C) — cursor-move handler w/ s16[rec+0xC2]=0
//                                  (mirrors 0x11214 but uses _08025A8C/0x258B8)
// ============================================================================

#include "gba/types.h"

#define HOST_STUB(sig) extern sig

// ---- no-op stubs ----------------------------------------------------------
void _080012574(volatile void *rec) { (void)rec; }
__asm__(".align 2, 0");
void _080012850(void) { }
__asm__(".align 2, 0");
void _080012F14(void) { }
__asm__(".align 2, 0");
void _0800133E0(void) { }
__asm__(".align 2, 0");
void _080013E60(void) { }
__asm__(".align 2, 0");
void _080014074(void) { }
__asm__(".align 2, 0");
void _0800141A8(void) { }
__asm__(".align 2, 0");
void _080014BCC(void) { }
__asm__(".align 2, 0");
void _080014C84(void) { }
__asm__(".align 2, 0");
void _080015CB0(void) { }
__asm__(".align 2, 0");

void sub_080012574(volatile void *rec) __attribute__((alias("_080012574")));
void sub_080012850(void) __attribute__((alias("_080012850")));
void sub_080012F14(void) __attribute__((alias("_080012F14")));
void sub_0800133E0(void) __attribute__((alias("_0800133E0")));
void sub_080013E60(void) __attribute__((alias("_080013E60")));
void sub_080014074(void) __attribute__((alias("_080014074")));
void sub_0800141A8(void) __attribute__((alias("_0800141A8")));
void sub_080014BCC(void) __attribute__((alias("_080014BCC")));
void sub_080014C84(void) __attribute__((alias("_080014C84")));
void sub_080015CB0(void) __attribute__((alias("_080015CB0")));
void Sub_080012574(volatile void *rec) __attribute__((alias("_080012574")));
void Sub_080012850(void) __attribute__((alias("_080012850")));
void Sub_080012F14(void) __attribute__((alias("_080012F14")));
void Sub_0800133E0(void) __attribute__((alias("_0800133E0")));
void Sub_080013E60(void) __attribute__((alias("_080013E60")));
void Sub_080014074(void) __attribute__((alias("_080014074")));
void Sub_0800141A8(void) __attribute__((alias("_0800141A8")));
void Sub_080014BCC(void) __attribute__((alias("_080014BCC")));
void Sub_080014C84(void) __attribute__((alias("_080014C84")));
void Sub_080015CB0(void) __attribute__((alias("_080015CB0")));

// ---- sub_080010F3C: cursor-move handler ----------------------------------
extern int  _08025A8C(int i);            // code_25930.c GetEntry
extern s16  _080258B8(int idx);          // course_cal.c Course_GetCup
extern s16  _0800F744(s32 c);            // menus.c
extern int  _08025E1C(int id);           // ai_grid_leaves.c Ai_GridGetPacked
extern void _0800F810(void *rec);        // menus.c record field refresh
extern s16  _0800258A8(int idx);         // numeric_leaves.c Code258A8
extern void _08007ABC(void *rec, int id, int lane); // resource bind
extern void _0802B368(u16 v);            // sound.c SoundDeferredVol
extern void _080026F50(u32 a, u32 b, u32 c, void *d); // garage.c Garage_PlaceRecord
extern s16  _0800F7EC(s32 a, s32 b);     // menus.c

#define CB55C 0x080CB55Cu

static inline u16 RD16(volatile u8 *rec, int off)  { return *(volatile u16 *)(rec + off); }
static inline void WR16(volatile u8 *rec, int off, u16 v) { *(volatile u16 *)(rec + off) = v; }
static inline u32  RD32(volatile u8 *rec, int off) { return *(volatile u32 *)(rec + off); }
static inline void WR32(volatile u8 *rec, int off, u32 v) { *(volatile u32 *)(rec + off) = v; }
static inline s16  RDS16(volatile u8 *rec, int off){ return *(volatile s16 *)(rec + off); }
static inline u16  tbl16(int off, s16 idx) {
    return *(volatile u16 *)(uintptr_t)(CB55C + off + (int)idx * 2);
}

// ============================================================================
// sub_080010F3C — cursor-move handler with s16[rec+0xC2]=0 after apply.
// r0 = rec, r1 = keymask (16/32 = up/down). Same epilogue shape as 0x11214
// but uses _08025A8C (vs 0x11214's _08025A8C, identical) and adds a
// s16[rec+0xC2]=0 write + the 0xE8/0xF4 rec placement sequence.
// ============================================================================
// sub_080010F3C — cursor-move handler: key-gated cursor step with a delay
// counter, then the shared apply/placement tail (EXACT 356/356, verified).
//
//  * Plain (non-volatile) field accesses are load-bearing: the file's
//    RD16/RDS16 helpers are volatile, and a volatile s16 read compiles to
//    `ldrh/lsls/asrs` where the ROM has `movs rX,#0; ldrsh rY,[rZ,rX]`.
//  * The delay decrement goes through an `int` local (`d`): with the value
//    stored straight from the u16 read the compiler materialises the `zero`
//    constant one slot early, and an s16/u16 local adds an `lsls/lsrs`
//    truncation the ROM does not have.
//  * `zero` is declared inside the block and assigned as its first statement,
//    which is what puts `movs rX,#0` between the `subs` and the `strh`.
void MenuFF78_10F3C(void *rec_, int a1, int a2) {
    extern const u16 MenuFF78Table10F3C[];
    __asm__(".globl MenuFF78Table10F3C\nMenuFF78Table10F3C = 0x080CB55C\n");
    u8 *rec = (u8 *)rec_;
    u16 keys = (u16)a1;
    u16 old = *(u16 *)(rec + 180);
    int d;
    (void)a2;

    if (keys & 32) {
        if (*(s16 *)(rec + 186) <= 0) {
            *(u16 *)(rec + 180) = (u16)(old - 1);
            *(u16 *)(rec + 186) = 10;
        }
    }
    if (keys & 16) {
        if (*(s16 *)(rec + 186) <= 0) {
            *(u16 *)(rec + 180) = (u16)(*(u16 *)(rec + 180) + 1);
            *(u16 *)(rec + 186) = 10;
        }
    }
    d = *(u16 *)(rec + 186) - 1;
    {
        u16 zero;
        s16 cur;
        u16 limit;

        zero = 0;
        *(u16 *)(rec + 186) = (u16)d;
        if (*(s16 *)(rec + 186) <= 0)
            *(u16 *)(rec + 186) = zero;
        cur = *(s16 *)(rec + 180);
        limit = *(u16 *)(rec + 184);
        if (cur >= *(s16 *)(rec + 184))
            *(u16 *)(rec + 180) = limit;
        if (*(s16 *)(rec + 180) < 0)
            *(u16 *)(rec + 180) = zero;
    }
    if ((s16)old == *(s16 *)(rec + 180))
        return;
    if (*(s16 *)(rec + 180) == 99)
        return;
    _0802B368(3);
    *(u16 *)(rec + 172) = (u16)_08025A8C(*(s16 *)(rec + 180));
    *(u16 *)(rec + 188) = (u16)_080258B8(*(s16 *)(rec + 172));
    *(u16 *)(rec + 190) = (u16)_0800F744(*(s16 *)(rec + 188));
    *(u16 *)(rec + 194) = (u16)_08025E1C(*(s16 *)(rec + 180));
    _0800F810(rec);
    {
        register u32 *p244 __asm__("r0");
        s16 v;
        s16 r = (s16)_0800258A8(*(s16 *)(rec + 172));

        *(u32 *)(rec + 232) = (u32)r;
        _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 28), (int)r, *(u32 *)(rec + 228));
        p244 = (u32 *)(rec + 244);
        v = *(s16 *)(rec + 172);
        *p244 = (u32)v;
        ((void (*)(u32, u32, u32))_080026F50)(p244[-1], (u32)v, 7);
        {
            u32 *dst340 = (u32 *)(rec + 340);
            register u32 tableBase __asm__("r1") =
                (u32)(uintptr_t)MenuFF78Table10F3C;
            register u32 tblOff __asm__("r0") =
                (u32)(s16)*(s16 *)(rec + 188);
            u16 sel;
            tblOff <<= 1;
            sel = *(u16 *)(uintptr_t)(tblOff + tableBase);
            *dst340 = (u32)sel;
            _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 36), (int)sel, *(u32 *)(rec + 336));
        }
        {
            s16 tail = (s16)_0800F7EC(*(s16 *)(rec + 188), *(s16 *)(rec + 190));
            *(u32 *)(rec + 352) = (u32)tail;
            _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 36), (int)tail, *(u32 *)(rec + 348));
        }
    }
}

void _080010F3C(void *a, int b, int c) __attribute__((alias("MenuFF78_10F3C")));
void sub_080010F3C(void *a, int b, int c) __attribute__((alias("MenuFF78_10F3C")));

// ============================================================================
// sub_0800110A0 — cursor-move handler w/ _0802584C (vs 0x10F3C's _08025A8C)
// and the _0800F8B4 selector rebind step. Same epilogue shape.
// r0 = rec, r1 = keymask (16/32 = up/down).
// ============================================================================
extern s16  _0802584C(int a);             // ai_line_more.c Ai_LineLookupA
extern void _0800F8B4(void *rec, u32 sel); // menus.c selector rebind
void MenuFF78_110A0(void *rec_, int a1, int a2) {
    extern const u16 MenuFF78Table110A0[];
    __asm__(".globl MenuFF78Table110A0\nMenuFF78Table110A0 = 0x080CB55C\n");
    u8 *rec = (u8 *)rec_;
    u16 keys = (u16)a1;
    u16 old = *(u16 *)(rec + 180);
    int d;
    u16 zero;
    (void)a2;

    if (keys & 32) {
        if (*(s16 *)(rec + 186) <= 0) {
            *(u16 *)(rec + 180) = (u16)(old - 1);
            *(u16 *)(rec + 186) = 10;
        }
    }
    if (keys & 16) {
        if (*(s16 *)(rec + 186) <= 0) {
            *(u16 *)(rec + 180) = (u16)(*(u16 *)(rec + 180) + 1);
            *(u16 *)(rec + 186) = 10;
        }
    }
    d = *(u16 *)(rec + 186) - 1;
    {
        u16 z;
        s16 cur;
        u16 limit;

        z = 0;
        zero = z;
        *(u16 *)(rec + 186) = (u16)d;
        if (*(s16 *)(rec + 186) <= 0)
            *(u16 *)(rec + 186) = zero;
        cur = *(s16 *)(rec + 180);
        limit = *(u16 *)(rec + 184);
        if (cur > *(s16 *)(rec + 184))
            *(u16 *)(rec + 180) = limit;
        if (*(s16 *)(rec + 180) < 0)
            *(u16 *)(rec + 180) = zero;
    }
    if ((s16)old == *(s16 *)(rec + 180))
        return;
    if (*(s16 *)(rec + 180) == 99)
        return;
    _0802B368(3);
    *(u16 *)(rec + 172) = (u16)_0802584C(*(s16 *)(rec + 180));
    *(u16 *)(rec + 188) = (u16)_080258B8(*(s16 *)(rec + 172));
    *(u16 *)(rec + 190) = (u16)_0800F744(*(s16 *)(rec + 188));
    *(u16 *)(rec + 194) = zero;
    _0800F810(rec);
    {
        register u32 *p244 __asm__("r0");
        s16 v;
        s16 r = (s16)_0800258A8(*(s16 *)(rec + 172));

        *(u32 *)(rec + 232) = (u32)r;
        _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 28), (int)r, *(u32 *)(rec + 228));
        _0800F8B4(rec, (s16)(*(u16 *)(rec + 194) - 1));
        p244 = (u32 *)(rec + 244);
        v = *(s16 *)(rec + 172);
        *p244 = (u32)v;
        ((void (*)(u32, u32, u32))_080026F50)(p244[-1], (u32)v, 7);
        {
            u32 *dst340 = (u32 *)(rec + 340);
            register u32 tableBase __asm__("r1") =
                (u32)(uintptr_t)MenuFF78Table110A0;
            register u32 tblOff __asm__("r0") =
                (u32)(s16)*(s16 *)(rec + 188);
            u16 sel;
            tblOff <<= 1;
            sel = *(u16 *)(uintptr_t)(tblOff + tableBase);
            *dst340 = (u32)sel;
            _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 36), (int)sel, *(u32 *)(rec + 336));
        }
        {
            s16 tail = (s16)_0800F7EC(*(s16 *)(rec + 188), *(s16 *)(rec + 190));
            *(u32 *)(rec + 352) = (u32)tail;
            _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 36), (int)tail, *(u32 *)(rec + 348));
        }
    }
}

void _0800110A0(void *a, int b, int c) __attribute__((alias("MenuFF78_110A0")));
void sub_0800110A0(void *a, int b, int c) __attribute__((alias("MenuFF78_110A0")));

// ============================================================================
// sub_080011484 — cursor-move handler w/ _0802581C (Ai_LineFindCourse) for
// s16[rec+0xC2]; otherwise similar epilogue (F8B4 rebind + 0xE8/0xF4 placement
// + 0x154/0x160/0x168 from 0x080CB55C, etc.).
// ============================================================================
extern int  _0802581C(int a);             // ai_line_more.c Ai_LineFindCourse
// C lift (EXACT 348/348). Same cursor/delay clamp as 0x10F3C, then re-derives
// the row (0x172 <- _08025A8C, 0x188 <- _080258B8, 0x194 <- _08025E1C(0x2581C))
// and refreshes the 232/340/244 lanes. Plain (non-volatile) field accesses are
// load-bearing, as in 0x10F3C: a volatile s16 read compiles to ldrh/lsls/asrs
// where the ROM has movs #0 / ldrsh.
void MenuFF78_11484(void *rec_, int a1, int a2) {
    extern const u16 MenuFF78Table11484[];
    __asm__(".globl MenuFF78Table11484\nMenuFF78Table11484 = 0x080CB55C\n");
    u8 *rec = (u8 *)rec_;
    u16 keys = (u16)a1;
    u16 old = *(u16 *)(rec + 180);
    int d;
    (void)a2;

    if (keys & 32) {
        if (*(s16 *)(rec + 186) <= 0) {
            *(u16 *)(rec + 180) = (u16)(old - 1);
            *(u16 *)(rec + 186) = 10;
        }
    }
    if (keys & 16) {
        if (*(s16 *)(rec + 186) <= 0) {
            *(u16 *)(rec + 180) = (u16)(*(u16 *)(rec + 180) + 1);
            *(u16 *)(rec + 186) = 10;
        }
    }
    d = *(u16 *)(rec + 186) - 1;
    {
        u16 zero;
        s16 cur;
        u16 limit;

        zero = 0;
        *(u16 *)(rec + 186) = (u16)d;
        if (*(s16 *)(rec + 186) <= 0)
            *(u16 *)(rec + 186) = zero;
        cur = *(s16 *)(rec + 180);
        limit = *(u16 *)(rec + 184);
        if (cur >= *(s16 *)(rec + 184))
            *(u16 *)(rec + 180) = limit;
        if (*(s16 *)(rec + 180) < 0)
            *(u16 *)(rec + 180) = zero;
    }
    *(u16 *)(rec + 172) = (u16)_08025A8C(*(s16 *)(rec + 180));
    *(u16 *)(rec + 188) = (u16)_080258B8(*(s16 *)(rec + 172));
    *(u16 *)(rec + 194) =
        (u16)_08025E1C((s16)_0802581C(*(s16 *)(rec + 172)));
    if ((s16)old != *(s16 *)(rec + 180) && *(s16 *)(rec + 180) != 99) {
        _0802B368(3);
        _0800F810(rec);
    }
    _0800F8B4(rec, (s16)(*(u16 *)(rec + 194) - 1));
    {
        s16 r = (s16)_0800258A8(*(s16 *)(rec + 172));
        register u32 tableBase __asm__("r1");
        register u32 tblOff __asm__("r0");
        u16 sel;
        u32 *p340;

        *(u32 *)(rec + 232) = (u32)r;
        /* Statement order is load-bearing: the call comes first, then the +340
           address takes r1 (movs #170 / lsls / adds), and only then may the
           table base load r1. A folded `Table[call()]` lets the base load
           hoist ahead of the address and shifts the whole lane. */
        tblOff = (u32)(s16)_080258B8(*(s16 *)(rec + 172));
        p340 = (u32 *)(rec + 340);
        tableBase = (u32)(uintptr_t)MenuFF78Table11484;
        /* The scale is fused with the sign extension: asrs #15, not
           lsls #16 / asrs #16 / lsls #1. */
        tblOff = ((s32)tblOff << 16) >> 15;
        sel = *(u16 *)(uintptr_t)(tblOff + tableBase);
        *p340 = (u32)sel;
        *(u32 *)(rec + 244) = (u32)*(s16 *)(rec + 172);
        _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 28),
                   *(u32 *)(rec + 232), *(u32 *)(rec + 228));
        _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 36),
                   *(u32 *)(rec + 340), *(u32 *)(rec + 336));
        ((void (*)(u32, u32, u32))_080026F50)(*(u32 *)(rec + 240),
                                               *(u32 *)(rec + 244), 7);
    }
}
void _080011484(void *a, int b, int c) __attribute__((alias("MenuFF78_11484")));
void sub_080011484(void *a, int b, int c) __attribute__((alias("MenuFF78_11484")));

// ============================================================================
// sub_0800115E0 — list-cursor handler w/ u16[rec+190] cursor over the record
// list. Uses _0800F744/F700/F6D0 for clamp + select logic. Same epilogue shape
// (s16[rec+212] = selected idx).
// ============================================================================
extern u32 _0800F6D0(s32 idx);
extern u32 _0800F700(s32 a, s32 b);
void MenuFF78_115E0(void *rec_, int a1, int a2) {
    /* The ROM helper returns a sign-extended word in r0. */
    extern int MenuF744_WordReturn(int) __asm__("_0800F744");
    register u8 *rec __asm__("r6") = rec_;
    register u16 *cursor __asm__("r4");
    register u16 *first __asm__("r5");
    s16 *group;
    u16 input = (u16)a2;
    register u16 old __asm__("r8");
    __asm__ volatile("" : "+r"(input));
    first = (u16 *)(rec + 190);
    old = *first;
    (void)a1;
    if (input == 32) *first -= 1;
    if (input == 16) *first += 1;
    group = (s16 *)(rec + 188);
    if (*first == 99) {
        int g = *group;
        *first = _0800F700(g, MenuF744_WordReturn(0));
        old = *(volatile u16 *)first;
    }
    if (*(s16 *)first >= (int)_0800F6D0(*group) && *first != 99)
        *first = _0800F6D0(*group) - 1;
    {
        register u8 *p __asm__("r0") = rec;
        register u32 zero __asm__("r2") = 0;
        register s32 cv __asm__("r1");
        p += 190;
        cv = *(s16 *)(p + zero);
        cursor = (u16 *)p;
        if (cv < 0) *cursor = 0;
    }
    {
        register u16 v __asm__("r0") = *cursor;
        register u8 *dest __asm__("r1") = rec;
        dest += 198;
        *(u16 *)dest = v;
    }
    {
        register s32 prev __asm__("r0") = (s16)old;
        register u32 zero __asm__("r2") = 0;
        register s32 cur __asm__("r1") = *(s16 *)((u8 *)cursor + zero);
        if (prev != cur && cur != 99) {
            int v;
            _0802B368(3);
            v = _0800F7EC(*group, *(s16 *)cursor);
            *(s32 *)(rec + 352) = (s16)v;
            _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 36),
                       (s16)v, *(u32 *)(rec + 348));
            _0800F810(rec);
        }
    }
    {
        int v;
        register u8 *gp __asm__("r0") = (u8 *)group;
        register u16 gv __asm__("r1") = *(u16 *)gp;
        /* Bracket the reads to reserve the ROM's ldrsh scratch registers. */
        if (gv == 1) {
            register u32 hold __asm__("r1");
            register s32 arg0;
            __asm__ volatile("" : "=r"(hold));
            arg0 = *(s16 *)gp;
            __asm__ volatile("" : : "r"(hold));
            {
                register s32 arg1 __asm__("r1");
                register u32 hold2 __asm__("r2");
                __asm__ volatile("" : "=r"(hold2));
                arg1 = *(s16 *)cursor;
                __asm__ volatile("" : : "r"(hold2));
                v = _0800F700(arg0, arg1) - 1;
            }
        } else {
            register s32 arg0 __asm__("r0");
            register s32 arg1 __asm__("r1");
            {
                register u32 hold3 __asm__("r3");
                __asm__ volatile("" : "=r"(hold3));
                arg0 = *(s16 *)group;
                __asm__ volatile("" : : "r"(hold3));
            }
            arg1 = *(s16 *)cursor;
            v = _0800F700(arg0, arg1);
        }
        {
            register u8 *dest __asm__("r1") = rec;
            dest += 212;
            *(u16 *)dest = v;
        }
    }
}

void _0800115E0(void *a, int b, int c) __attribute__((alias("MenuFF78_115E0")));
void sub_0800115E0(void *a, int b, int c) __attribute__((alias("MenuFF78_115E0")));

// ============================================================================
// sub_0800116DC — small cursor handler w/ u16[rec+192] (clamped 0..2, 99→0),
// mirrors to u16[rec+200], and the same epilogue 0x15C bind + s16[rec+212].
// ============================================================================
void MenuFF78_116DC(void *rec_, int unused, int a2) {
    (void)unused;
    __asm__(".globl MenuFF78Table116DC\nMenuFF78Table116DC = 0x080CB55C\n");
    extern const u16 MenuFF78Table116DC[];
    register u8 *r4 __asm__("r4") = (u8 *)rec_;
    register u32 mode __asm__("r2") = (u32)a2;
    mode = (u16)mode;
    register volatile u8 *r1 __asm__("r1") = r4;
    r1 += 192;
    register u16 old __asm__("r3") = *(volatile u16 *)r1;

    if (mode == 32) {
        register u16 dec __asm__("r0") = (u16)(old - 1);
        *(volatile u16 *)r1 = dec;
    }
    if (mode == 16) {
        register u16 inc __asm__("r0") = *(volatile u16 *)r1;
        inc = (u16)(inc + 1);
        *(volatile u16 *)r1 = inc;
    }
    if (*(volatile u16 *)r1 == 99) {
        *(volatile u16 *)r1 = 0;
        old = 0;
    }
    {
        register u32 zeroClamp __asm__("r2") = 0;
        register s32 clampValue __asm__("r0");
        clampValue = *(s16 *)((u8 *)r1 + zeroClamp);
        if (clampValue > 2 && clampValue != 99)
            *(volatile u16 *)r1 = 2;
    }
    {
        register u8 *p0 __asm__("r0") = r4;
        p0 += 192;
        register u32 zeroCursor __asm__("r2") = 0;
        register s32 signedValue __asm__("r1");
        signedValue = *(s16 *)(p0 + zeroCursor);
        register u8 *r5 __asm__("r5") = p0;
        if (signedValue < 0)
            *(volatile u16 *)r5 = 0;
        register u16 mirror __asm__("r1") = *(volatile u16 *)r5;
        register u8 *p200 __asm__("r0") = r4;
        p200 += 200;
        *(volatile u16 *)p200 = mirror;

        register s32 previous __asm__("r0") = (s16)old;
        register u32 zeroCompare __asm__("r3") = 0;
        register s32 currentValue __asm__("r1");
        currentValue = *(s16 *)(r5 + zeroCompare);
        if (previous != currentValue && currentValue != 99) {
            _0802B368(3);
            register u32 destOffset __asm__("r0") = 182;
            __asm__ volatile ("" : "+r"(destOffset));
            destOffset <<= 1;
            register u32 dest __asm__("r2") =
                (u32)(uintptr_t)r4 + destOffset;
            __asm__ volatile ("" : "+r"(dest));

            register u32 tableBase __asm__("r1") =
                (u32)(uintptr_t)MenuFF78Table116DC;
            __asm__ volatile ("" : "+r"(tableBase));
            register u32 tableZero __asm__("r3") = 0;
            register s32 row __asm__("r0");
            row = *(s16 *)(r5 + tableZero);
            row <<= 1;
            tableBase += 20;
            row += tableBase;
            register u32 selected __asm__("r1") =
                *(volatile u16 *)(uintptr_t)row;
            *(volatile u32 *)(uintptr_t)dest = selected;

            register u32 firstArg __asm__("r0") =
                *(volatile u32 *)(r4 + 36);
            register u32 laneOffset __asm__("r3") = 180;
            __asm__ volatile ("" : "+r"(laneOffset));
            laneOffset <<= 1;
            register u32 lane __asm__("r2") =
                (u32)(uintptr_t)r4 + laneOffset;
            __asm__ volatile ("" : "+r"(lane));
            lane = *(volatile u32 *)(uintptr_t)lane;
            _08007ABC((void *)(uintptr_t)firstArg, (int)selected, (int)lane);
            _0800F810(r4);
        }
        register u16 finalValue __asm__("r1") = *(volatile u16 *)r5;
        register u8 *endDest __asm__("r0") = r4;
        endDest += 212;
        *(volatile u16 *)endDest = finalValue;
    }
}
void _0800116DC(void *a, int b, int c) __attribute__((alias("MenuFF78_116DC")));
void sub_0800116DC(void *a, int b, int c) __attribute__((alias("MenuFF78_116DC")));
