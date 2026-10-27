// ============================================================================
// menu_ff78_b.c — reconstructed C for asm/menu_ff78.s (3 functions):
//
//   sub_080010CB8 (0x08010CB8) — car-select input handler (mode machine
//                                0..3 + cursor-apply mode machine 0/1/2)
//   sub_080011214 (0x08011214) — cursor-move handler (keymask bit gate)
//   sub_0800113A0 (0x080113A0) — apply-current-selection handler
//                                (0x109D4 tail dispatch, r1/r2 dead)
//
// Common epilogue shape: s16[rec+0xBC] = _080258B8(s16[rec+0xAC]);
// s16[rec+0xBE] = _0800F744(s16[rec+0xBC]); record placement via
// _080026F50(u32[rec+0xF0], s16[rec+0xAC], 7); u16[0x080CB55C + 2*s16[0xBC]]
// -> u32[rec+0x154]; _0800F7EC(s16[0xBC], s16[0xBE]) -> u32[rec+0x160];
// resource binds via _08007ABC; final u16[rec+0xB6] = u16[rec+0xAC].
//
// sub_080010CB8 is C (see its comment); the other two bodies are still
// transcribed instruction-for-instruction from asm/menu_ff78.s.
// ============================================================================

#include "gba/types.h"

#define HOST_STUB(sig) extern sig

HOST_STUB(void _08007ABC(void *rec, int id, int lane));     // 0x08007ABC resource bind

extern void _0800108D4(void *rec);            // menus.c Menu108D4
extern void _0800108F0(void *rec);            // menus.c Menu108F0
extern void _0802B368(u16 v);                 // sound.c SoundDeferredVol
extern int  _0800F744(s32 c);                 // menus.c
extern u32  _0800F6D0(s32 c);                 // menus.c
extern u32  _0800F700(s32 a, s32 b);          // menus.c
extern s16  _0800F7EC(s32 a, s32 b);          // menus.c
extern void _0800F810(void *rec);             // menus.c record field refresh
extern s16  _080258B8(int idx);               // course_cal.c Course_GetCup
extern int  _08025A8C(int i);                 // code_25930.c GetEntry
extern int  _08025E1C(int id);                // ai_grid_leaves.c Ai_GridGetPacked
extern s16  _0800258A8(int idx);              // numeric_leaves.c 0x080CD7D0 lookup
extern u16  _08002178(int i);                 // block_b.c BlockB_DispatchIfLess
extern void _080026F50(u32 a, u32 b, u32 c, void *d); // garage.c Garage_PlaceRecord

// Record layout for the two field-to-field copies below. The ROM materialises
// a store's destination address only when the store itself is emitted, which is
// what a struct-field assignment produces; a raw `*(u16 *)(rec + N)` store
// expands the address before the right-hand side, two instructions early.
struct MenuFF78Rec10CB8 {
    u8  pad0[0xBE];
    s16 fBe;                     // 0xBE
    s16 fC0;                     // 0xC0
};

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
// sub_080010CB8 — car-select input handler: mode machine 0..3, then a
// cursor-apply mode machine 0/1/2 with the 12-byte resource-row loop.
// ----------------------------------------------------------------------------
// EXACT 644/644 bytes, no instruction-bearing asm. The body was steered from
// the `--diff` listing; each spelling below was measured on its own.
//
//  * Both machines are `switch`es. The first one keeps four *separate* case
//    labels (case 1 and case 2 carry identical bodies), which is what makes
//    agbcc build the ROM's four-node tree (`cmp #1 / beq / cmp #1 / bgt / ...`);
//    `case 1: case 2:` instead folds into a range node and pivots at 2.
//  * `key` and `old` are `int` locals holding the u16 truncations. As `u16`
//    agbcc keeps a second, zero-extended copy of the key for case 3
//    (`adds r6,r5,#0`) and parks `old` in r8; as `int` they land in r5/r6.
//  * The two `*dst = *src` halfword copies go through `struct
//    MenuFF78Rec10CB8`. A struct-field assignment defers the destination
//    address to the store, so the ROM's load-then-address order falls out and
//    the following `== 99` test picks up the `lsls/asrs` pair; a raw
//    `*(u16 *)(rec + N)` store expands the address first and drops it.
//  * The row loop recomputes `off = i * 12` in the body, which loop.c turns
//    into the ROM's `adds r4,#12` biv, and keeps each record base in a
//    statement variable (`pa = rec + 376; pb = pa - 4;`). Through a plain
//    `rec + 376 + off` expression agbcc instead strength-reduces the address
//    into a pointer the loop walks by 12 (`[r4,#4]`-style stores).
//    `tb` (0x080CB566 = the +10 table column) must be a variable too, or the
//    pool load stays inside the loop instead of the ROM's `mov sl`.
//  * `lt = _0800F700(...)` is its own statement: the call must precede the
//    store address, while the row address must still precede the table load.
//    The store itself is the chained `*pa = v = ...`, which keeps the address
//    first and leaves the value in r1 for the `_08007ABC` argument.
//  * `port = *(u32 *)(rec + 36)` is loaded before `pb = pa - 4`, which puts the
//    resource-port load in the ROM's slot between store and lane pointer.
//  * Case 2's table read needs the ROM's `dest-address, pool base, index,
//    lsls, base+20, add, ldrh` sequence; that comes from the statement order
//    `out, tb, off2, row`, then `*out = v = *(u16 *)(off2 + row)`.
//  * `_0800F744` is declared returning `int` in this TU: the ROM feeds its
//    result straight to `_0800F700` without the `lsls/asrs` an `s16`
//    declaration would emit.
// ============================================================================
extern const u16 MenuFF78Tbl10CB8[];
void MenuFF78_10CB8(void *rec_, int a1, int a2) {
    __asm__(".globl MenuFF78Tbl10CB8\nMenuFF78Tbl10CB8 = 0x080CB55C\n");
    u8 *rec = (u8 *)rec_;
    int key = (u16)a2;
    int old = *(u16 *)(rec + 168);
    (void)a1;

    switch (*(s16 *)(rec + 168)) {
    case 0:
        if (key == 2) _0800108D4(rec);
        if (key == 1) {
            *(u16 *)(rec + 168) = *(u16 *)(rec + 168) + 1;
            _0802B368(1);
            if (*(u16 *)(rec + 172) != *(u16 *)(rec + 176)) {
                *(u16 *)(rec + 198) = 99;
                *(u16 *)(rec + 200) = 99;
            }
            *(u16 *)(rec + 176) = *(u16 *)(rec + 172);
        }
        break;
    case 1:
        if (key == 2) {
            *(u16 *)(rec + 168) = old - 1;
            _0802B368(4);
        }
        if (key == 1) {
            *(u16 *)(rec + 168) = *(u16 *)(rec + 168) + 1;
            _0802B368(1);
        }
        break;
    case 2:
        if (key == 2) {
            *(u16 *)(rec + 168) = old - 1;
            _0802B368(4);
        }
        if (key == 1) {
            *(u16 *)(rec + 168) = *(u16 *)(rec + 168) + 1;
            _0802B368(1);
        }
        break;
    case 3:
        if (key == 2) {
            *(u16 *)(rec + 168) = old - 1;
            _0802B368(4);
        }
        if (key == 1) _0800108F0(rec);
        break;
    }
    if (*(s16 *)(rec + 168) > 4) *(u16 *)(rec + 168) = 4;
    if (*(u16 *)(rec + 168) == old) return;
    switch (*(s16 *)(rec + 168)) {
    case 0:
        *(u16 *)(rec + 190) = 99;
        *(u16 *)(rec + 192) = 99;
        _0800F810(rec);
        break;
    case 1:
        if (*(u16 *)(rec + 172) == *(u16 *)(rec + 176)) {
            ((struct MenuFF78Rec10CB8 *)rec)->fBe = *(u16 *)(rec + 198);
            if (((struct MenuFF78Rec10CB8 *)rec)->fBe == 99)
                *(u16 *)(rec + 190) = (u16)_0800F700(*(s16 *)(rec + 188),
                                                     _0800F744(0));
        } else {
            *(u16 *)(rec + 190) = (u16)_0800F700(*(s16 *)(rec + 188),
                                                 _0800F744(0));
        }
        {
            int i = 0;
            s16 *p188;
            u32 tb;
            u32 off;
            u16 v;
            u32 lt;
            u8 *pa;
            u8 *pb;
            p188 = (s16 *)(rec + 188);
            tb = 0x080CB566u;
            for (; i < (s32)_0800F6D0(*p188); i++) {
                u32 port;
                off = i * 12;
                lt = _0800F700(*p188, i);
                pa = rec + 376;
                *(u32 *)(pa + off) = v = *(u16 *)(uintptr_t)(lt * 2 + tb);
                port = *(u32 *)(rec + 36);
                pb = pa - 4;
                _08007ABC((void *)(uintptr_t)port, v, *(u32 *)(pb + off));
            }
        }
        if (*(u16 *)(rec + 190) != 99) {
            s16 tail = _0800F7EC(*(s16 *)(rec + 188), *(s16 *)(rec + 190));
            *(u32 *)(rec + 352) = (u32)tail;
            _08007ABC(*(void **)(rec + 36), tail, *(u32 *)(rec + 348));
        }
        _0800F810(rec);
        break;
    case 2:
        if (*(u16 *)(rec + 172) == *(u16 *)(rec + 176)) {
            ((struct MenuFF78Rec10CB8 *)rec)->fC0 = *(u16 *)(rec + 200);
            if (((struct MenuFF78Rec10CB8 *)rec)->fC0 == 99)
                ((struct MenuFF78Rec10CB8 *)rec)->fC0 = 0;
        } else {
            ((struct MenuFF78Rec10CB8 *)rec)->fC0 = 0;
        }
        {
            const u16 *tb;
            u16 *row;
            u32 *out;
            u16 v;
            s32 off2;
            out = (u32 *)(rec + 364);
            tb = MenuFF78Tbl10CB8;
            off2 = *(s16 *)(rec + 192) * 2;
            row = (u16 *)(uintptr_t)(tb + 10);
            *out = v = *(u16 *)(uintptr_t)((u32)off2 + (u32)(uintptr_t)row);
            _08007ABC(*(void **)(rec + 36), v, *(u32 *)(rec + 360));
            *(u32 *)(rec + 376) = tb[10];
            *(u32 *)(rec + 388) = tb[11];
            *(u32 *)(rec + 400) = tb[12];
        }
        _08007ABC(*(void **)(rec + 36), *(u32 *)(rec + 376),
                  *(u32 *)(rec + 372));
        _08007ABC(*(void **)(rec + 36), *(u32 *)(rec + 388),
                  *(u32 *)(rec + 384));
        _08007ABC(*(void **)(rec + 36), *(u32 *)(rec + 400),
                  *(u32 *)(rec + 396));
        _0800F810(rec);
        break;
    }
}

void _080010CB8(void *a, int b, int c) __attribute__((alias("MenuFF78_10CB8")));
void sub_080010CB8(void *a, int b, int c) __attribute__((alias("MenuFF78_10CB8")));

// ============================================================================
// sub_080011214 — cursor-move handler (keymask 0x20 = down, 0x10 = up).
// ============================================================================
__attribute__((naked)) void MenuFF78_11214(void *rec_, int a1, int a2) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush\t{r4, r5, r6, r7, lr}\n"
        "\tmov\tr7, r9\n"
        "\tmov\tr6, r8\n"
        "\tpush\t{r6, r7}\n"
        "\tadds\tr5, r0, #0\n"
        "\tlsls\tr1, r1, #16\n"
        "\tlsrs\tr3, r1, #16\n"
        "\tmovs\tr0, #32\n"
        "\tands\tr0, r3\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .Lff11214_080011244\n"
        "\tadds\tr2, r5, #0\n"
        "\tadds\tr2, #186\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r2, r1]\n"
        "\tcmp\tr0, #0\n"
        "\tbgt .Lff11214_080011244\n"
        "\tadds\tr0, r5, #0\n"
        "\tadds\tr0, #180\n"
        "\tldrh\tr1, [r0, #0]\n"
        "\tsubs\tr1, #1\n"
        "\tstrh\tr1, [r0, #0]\n"
        "\tmovs\tr0, #10\n"
        "\tstrh\tr0, [r2, #0]\n"
        ".Lff11214_080011244:\n"
        "\tmovs\tr0, #16\n"
        "\tands\tr0, r3\n"
        "\tadds\tr1, r5, #0\n"
        "\tadds\tr1, #186\n"
        "\tadds\tr6, r5, #0\n"
        "\tadds\tr6, #180\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .Lff11214_080011266\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r1, r2]\n"
        "\tcmp\tr0, #0\n"
        "\tbgt .Lff11214_080011266\n"
        "\tldrh\tr0, [r6, #0]\n"
        "\tadds\tr0, #1\n"
        "\tstrh\tr0, [r6, #0]\n"
        "\tmovs\tr0, #10\n"
        "\tstrh\tr0, [r1, #0]\n"
        ".Lff11214_080011266:\n"
        "\tldrh\tr0, [r1, #0]\n"
        "\tsubs\tr0, #1\n"
        "\tmovs\tr3, #0\n"
        "\tmov\tr8, r3\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tlsls\tr0, r0, #16\n"
        "\tcmp\tr0, #0\n"
        "\tbgt .Lff11214_08001127a\n"
        "\tmov\tr4, r8\n"
        "\tstrh\tr4, [r1, #0]\n"
        ".Lff11214_08001127a:\n"
        "\tadds\tr2, r6, #0\n"
        "\tadds\tr0, r5, #0\n"
        "\tadds\tr0, #184\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr1, [r2, r3]\n"
        "\tldrh\tr3, [r0, #0]\n"
        "\tmovs\tr4, #0\n"
        "\tldrsh\tr0, [r0, r4]\n"
        "\tcmp\tr1, r0\n"
        "\tblt .Lff11214_080011290\n"
        "\tstrh\tr3, [r2, #0]\n"
        ".Lff11214_080011290:\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r2, r1]\n"
        "\tcmp\tr0, #0\n"
        "\tbge .Lff11214_08001129c\n"
        "\tmov\tr3, r8\n"
        "\tstrh\tr3, [r2, #0]\n"
        ".Lff11214_08001129c:\n"
        "\tmovs\tr4, #0\n"
        "\tldrsh\tr0, [r6, r4]\n"
        "\tbl sub_08025A8C\n"
        "\tadds\tr1, r5, #0\n"
        "\tadds\tr1, #178\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr0, #0\n"
        "\tmovs\tr1, #2\n"
        "\tbl 0x08002178\n"
        "\tadds\tr4, r0, #0\n"
        "\tmovs\tr0, #1\n"
        "\tmovs\tr1, #2\n"
        "\tbl 0x08002178\n"
        "\tlsls\tr4, r4, #16\n"
        "\tlsls\tr0, r0, #16\n"
        "\tcmp\tr4, r0\n"
        "\tbne .Lff11214_08001138e\n"
        "\tmovs\tr0, #182\n"
        "\tadds\tr0, r0, r5\n"
        "\tmov\tr9, r0\n"
        "\tadds\tr7, r5, #0\n"
        "\tadds\tr7, #172\n"
        "\tldrh\tr1, [r0, #0]\n"
        "\tldrh\tr2, [r7, #0]\n"
        "\tcmp\tr1, r2\n"
        "\tbeq .Lff11214_080011388\n"
        "\tmovs\tr0, #3\n"
        "\tbl 0x0802B368\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r6, r3]\n"
        "\tbl sub_08025A8C\n"
        "\tstrh\tr0, [r7, #0]\n"
        "\tmovs\tr4, #0\n"
        "\tldrsh\tr0, [r7, r4]\n"
        "\tbl 0x080258B8\n"
        "\tadds\tr4, r5, #0\n"
        "\tadds\tr4, #188\n"
        "\tstrh\tr0, [r4, #0]\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r4, r1]\n"
        "\tbl sub_0800F744\n"
        "\tadds\tr6, r5, #0\n"
        "\tadds\tr6, #190\n"
        "\tstrh\tr0, [r6, #0]\n"
        "\tadds\tr0, r5, #0\n"
        "\tadds\tr0, #194\n"
        "\tmov\tr2, r8\n"
        "\tstrh\tr2, [r0, #0]\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r7, r3]\n"
        "\tbl 0x080258A8\n"
        "\tadds\tr1, r0, #0\n"
        "\tadds\tr0, r5, #0\n"
        "\tadds\tr0, #232\n"
        "\tlsls\tr1, r1, #16\n"
        "\tasrs\tr1, r1, #16\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tldr\tr0, [r5, #28]\n"
        "\tadds\tr2, r5, #0\n"
        "\tadds\tr2, #228\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tadds\tr0, r5, #0\n"
        "\tadds\tr0, #244\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr1, [r7, r2]\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tsubs\tr0, #4\n"
        "\tldr\tr0, [r0, #0]\n"
        "\tmovs\tr2, #7\n"
        "\tbl 0x08026F50\n"
        "\tmovs\tr3, #170\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr2, r5, r3\n"
        "\tldr r1, .Lff11214_08001139c\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r4, r3]\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr0, r0, r1\n"
        "\tldrh\tr1, [r0, #0]\n"
        "\tstr\tr1, [r2, #0]\n"
        "\tldr\tr0, [r5, #36]\n"
        "\tmovs\tr3, #168\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr2, r5, r3\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r4, r1]\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr1, [r6, r2]\n"
        "\tbl sub_0800F7EC\n"
        "\tadds\tr1, r0, #0\n"
        "\tmovs\tr3, #176\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr0, r5, r3\n"
        "\tlsls\tr1, r1, #16\n"
        "\tasrs\tr1, r1, #16\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tldr\tr0, [r5, #36]\n"
        "\tmovs\tr4, #174\n"
        "\tlsls\tr4, r4, #1\n"
        "\tadds\tr2, r5, r4\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        ".Lff11214_080011388:\n"
        "\tldrh\tr0, [r7, #0]\n"
        "\tmov\tr1, r9\n"
        "\tstrh\tr0, [r1, #0]\n"
        ".Lff11214_08001138e:\n"
        "\tpop\t{r3, r4}\n"
        "\tmov\tr8, r3\n"
        "\tmov\tr9, r4\n"
        "\tpop\t{r4, r5, r6, r7}\n"
        "\tpop\t{r0}\n"
        "\tbx\tr0\n"
        "\tmovs\tr0, r0\n"
        ".Lff11214_08001139c: .word 0x080CB55C\n"
        ".syntax divided\n"
    );
}

void _080011214(void *a, int b, int c) __attribute__((alias("MenuFF78_11214")));
void sub_080011214(void *a, int b, int c) __attribute__((alias("MenuFF78_11214")));

// ============================================================================
// sub_0800113A0 — apply-current-selection handler (0x109D4 tail; r1/r2 dead).
// ============================================================================
void MenuFF78_113A0(void *rec_, int a1, int a2) {
    u8 *rec = (u8 *)rec_;
    u8 *p172 = rec + 172;
    u8 *p188;
    u16 t;

    extern const u16 MenuFF78Table113A0[];
    __asm__(".globl MenuFF78Table113A0\nMenuFF78Table113A0 = 0x080CB55C\n");
    (void)a1;
    (void)a2;

    t = (u16)_080258B8(*(s16 *)p172);
    p188 = rec + 188;
    *(u16 *)p188 = t;
    t = (u16)_0800F744(*(s16 *)p188);
    *(u16 *)(rec + 190) = t;
    // r1 is dead in the callee, so the high-nibble comparison is the u16 one.
    if ((u16)((u16 (*)(int, int))_08002178)(0, 2) !=
        (u16)((u16 (*)(int, int))_08002178)(1, 2))
        return;
    if (*(u16 *)(rec + 182) != *(u16 *)p172) {
        // The ROM writes +244 and then reads back +240 through the same
        // address register (`subs r0,#4`); keeping the destination in r0 and
        // indexing it is what reproduces that instead of a second add.
        register u32 *p244 __asm__("r0");
        s16 v;
        s16 r = (s16)_0800258A8(*(s16 *)p172);

        *(u32 *)(rec + 232) = (u32)r;
        _08007ABC(*(void **)(rec + 28), (int)r, *(u32 *)(rec + 228));
        p244 = (u32 *)(rec + 244);
        v = *(s16 *)p172;
        *p244 = (u32)v;
        ((void (*)(u32, u32, u32))_080026F50)(p244[-1], (u32)v, 7);
        {
            u32 *dst340 = (u32 *)(rec + 340);
            // The ROM loads the table base into r1 *before* the index and
            // takes r3 for the zero index; a pinned base plus a pinned r0
            // offset is the pair that reproduces both registers.
            register u32 tableBase __asm__("r1") =
                (u32)(uintptr_t)MenuFF78Table113A0;
            register u32 tblOff __asm__("r0") =
                (u32)(s16)*(s16 *)p188;
            u16 sel;
            tblOff <<= 1;
            sel = *(u16 *)(uintptr_t)(tblOff + tableBase);
            *dst340 = (u32)sel;
            _08007ABC(*(void **)(rec + 36), (int)sel, *(u32 *)(rec + 336));
        }
        {
            s16 tail = (s16)_0800F7EC(*(s16 *)p188, *(s16 *)(rec + 190));
            *(u32 *)(rec + 352) = (u32)tail;
            _08007ABC(*(void **)(rec + 36), (int)tail, *(u32 *)(rec + 348));
        }
    }
    *(u16 *)(rec + 182) = *(u16 *)p172;
}

void _0800113A0(void *a, int b, int c) __attribute__((alias("MenuFF78_113A0")));
void sub_0800113A0(void *a, int b, int c) __attribute__((alias("MenuFF78_113A0")));
