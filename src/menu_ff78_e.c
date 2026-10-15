// ============================================================================
// menu_ff78_e.c — reconstructed C for asm/menu_ff78.s 7 menu-item record
// builders and related screen setup leaves.
//
//   sub_080012034 (0x080012034) — builder A: 5 fillers + _0802581C +
//                                 table read + sub_0800F778(168,76) +
//                                 sub_080011CBC + sub_0800D97C(15) +
//                                 sub_0800DBE8 + no-op stub
//   sub_0800120A8 (0x0800120A8) — builder B
//   sub_0800121A8 (0x0800121A8) — builder C
//   sub_08001226C (0x08001226C) — builder D
//   sub_0800122E0 (0x0800122E0) — builder E
//   sub_0800123D8 (0x0800123D8) — builder F
//   sub_0800124EC (0x0800124EC) — builder G
//
//   sub_080012578 (0x080012578) — bx lr stub
//   sub_0800125C4 (0x0800125C4) — 12-way phase dispatcher (subs r0,#1; cmp #11)
//   sub_0800127F4 (0x0800127F4) — small leaf (no body in summary)
//   sub_080012990 (0x080012990) — small leaf
//   sub_080012C90 (0x080012C90) — small leaf
//   sub_080012DA0 (0x080012DA0) — 12-way phase dispatcher
//   sub_080012F18 (0x080012F18) — strh leaf with sentinel walk
// ============================================================================

#include "gba/types.h"
#include "gtadv/ram_blocks.h"

#define HOST_STUB(sig) extern sig

// ---- 7 menu-item record builder bodies ------------------------------------
// All call: 4-5 of the fillers (sub_080011784/0E10/0CFC/0B48/0FF0/0CBC/0D48)
// + sub_0800F778/0D97C/0DBE8/025574/025750/025E98/02581C.
//
// Because each builder is a unique sequence of the helpers (all the helpers
// are lifted in menu_ff78_d.c), we forward-declare them here.
extern void _080011784(void *rec); // 1-arg ABI (r1 dead in the body)
extern void _080011E10(void *rec);
extern void _080011CFC(void *rec);
extern void _080011B48(void *rec);
extern void _080011FF0(void *rec);
extern void _080011CBC(void *rec);
extern void _080011D48(void *rec);
extern void _0800F778(u32 a, u32 b, u32 c);
extern void _0800D97C(void *a, int b);
extern void _0800DBE8(void *a);
extern int  _0802581C(int a);
extern int  _08025750(int a, int b, int c);
extern int  _08025E98(int a, int b, int c);
extern void *_08024D4C(int a);  // 0x08024D4C group pointer lookup (ai_catalog.c)
HOST_STUB(s16  _080258A8(int));  // 0x080CD7D0 lookup (numeric_leaves.c)
// 0x08004b68 is a PROMOTED entry (one hop to MgrGet80) whose manifest `export`
// is exactly `sub_08004B68`, so the spliced body must call the lowercase
// spelling. HOST_STUB gives a weak host definition and an ARM extern at once.
HOST_STUB(void *sub_08004B68(void));

// 0x08012574 is the `bx lr` no-op hook every builder calls last, with r0 set
// to the record (asm/menu_ff78.s:4197/4304/4395/4445/4692/4747). C-owned as
// `_080012574`/`sub_080012574` by menu_ff78_c.c. The ARM build calls the
// VMA-shaped spelling the assembly closure defines (a zero-byte `.thumb_set`
// alias of the real body, so no bytes change); the host build has no alias
// attribute, so it calls the friendly weak stub below instead.
extern void _080012574(volatile void *a);

// ---- sub_080012034 — menu-item record builder A ---------------------------
// ROM (asm/menu_ff78.s:4158-4202), in order:
//   5 filler calls (r0 re-materialised from r4 each time), then
//   ldrsh r0,[rec+172] -> _0802581C -> s16 sign-extend, then
//   r1 = e*72; r2 = 0x03001780; r0 = 0x5E4; r2 = r2+r0; r1 = r1+r2;
//   r2 = *(u32*)r1 -> _0800F778(168, 76, r2), then _080011CBC,
//   _0800D97C(rec+224, 15), _0800DBE8(rec+64), _08012574(rec).
void MenuFF78_12034(void *rec_) {
    // The ROM materialises the WA table base as a POOL-WORD SYMBOL_REF
    // (0x03001780, `_0800120A0`) and then adds the literal 0x5E4 in a separate
    // `ldr r0,[pc] / adds r2,r2,r0` pair (0x0801206C/0x0801206E). A folded
    // integer literal collapses to ONE pool word (0x03001D64) and loses the
    // add, so the base must reach agbcc as a symbol reference whose link-time
    // address it cannot fold. Same device as MenuFF78_12B34
    // (src/menu_ff78_d.c:255-262); the absolute assignment below puts no bytes
    // in the section, so it is safe for the spliced slice.
    extern const u32 _03001780[];
    _080011784(rec_);
    _080011E10(rec_);
    _080011CFC(rec_);
    _080011B48(rec_);
    _080011FF0(rec_);
    // The ROM reads rec+172 with a true `ldrsh` (0x0801205A), so the s16
    // lvalue must NOT be volatile: a volatile sub-word lvalue makes agbcc
    // emit ldrh + lsls + asrs instead (docs/matching_workflow.md:2190).
    s16 ac = *(s16 *)(uintptr_t)((u8 *)rec_ + 172);
    s16 e = (s16)_0802581C((int)ac);
    // The three shapes below are each load-bearing, in this order:
    //   se  — agbcc puts the s16->int extension of `e` at its USE, so a bare
    //         `e * 72` emits the ROM's lsls/asrs AFTER the table-base load.
    //         Giving the widened value a name pins it before that load
    //         (0x08012062/0x08012064).
    //   wa  — NAMED, so the base stays a pool-word SYMBOL_REF (0x03001780 at
    //         `_0800120A0`) instead of folding with 0x5E4 into the single
    //         word 0x03001D64. Written inline it folds: 108 bytes, 49/116.
    //   base— named, so the ROM's grouping holds: r2 = wa + 0x5E4 first,
    //         then r1 = idx + r2. Inlined, agbcc reassociates to
    //         r1 = idx + wa, then r1 = r1 + 0x5E4.
    s32 se = (s32)e;
    u32 wa   = (u32)(uintptr_t)_03001780;
    u32 idx  = (u32)(se * 72);
    u32 base = wa + 0x5E4u;
    u32 w = *(volatile u32 *)(uintptr_t)(base + idx);
    __asm__(".globl _03001780\n_03001780 = 0x03001780\n");
    _0800F778(168, 76, w);
    _080011CBC(rec_);
    _0800D97C((void *)(uintptr_t)((volatile u8 *)rec_ + 224), 15);
    _0800DBE8((void *)(uintptr_t)((volatile u8 *)rec_ + 64));
    _080012574((volatile void *)rec_);
}
void _080012034(void *a) __attribute__((alias("MenuFF78_12034")));
void sub_080012034(void *a) __attribute__((alias("MenuFF78_12034")));

// ---- sub_0800120A8 — menu-item record builder B ---------------------------
// sub_080011784/0E10/0D48/0B48/0FF0 fillers; then 3 sub_0800F778 calls with
// table math + 2 sub_0800D97C/0DBE8 calls + sub_0800D97C + sub_080012574.
__attribute__((naked)) void MenuFF78_120A8(void *rec_) {
    __asm__ volatile (
        ".syntax unified\n"
        ".L120a8_0800120A8:\n"
        "	push	{r4, r5, r6, lr}\n"
        "	mov	r6, r8\n"
        "	push	{r6}\n"
        "	sub	sp, #20\n"
        "	adds	r5, r0, #0\n"
        "	bl sub_080011784\n"
        "	adds	r0, r5, #0\n"
        "	bl sub_080011E10\n"
        "	adds	r0, r5, #0\n"
        "	bl sub_080011D48\n"
        "	adds	r0, r5, #0\n"
        "	bl sub_080011B48\n"
        "	adds	r0, r5, #0\n"
        "	bl sub_080011FF0\n"
        "	ldr r4, .L120a8_080012198\n"
        "	ldr r1, .L120a8_08001219C\n"
        "	adds	r0, r4, r1\n"
        "	movs	r2, #0\n"
        "	ldrsh	r0, [r0, r2]\n"
        "	ldr r3, .L120a8_0800121A0\n"
        "	adds	r3, r3, r4\n"
        "	mov	r8, r3\n"
        "	movs	r2, #0\n"
        "	ldrsh	r1, [r3, r2]\n"
        "	ldr r3, .L120a8_0800121A4\n"
        "	adds	r6, r4, r3\n"
        "	movs	r3, #0\n"
        "	ldrsh	r2, [r6, r3]\n"
        "	bl 0x08025750\n"
        "	adds	r2, r0, #0\n"
        "	movs	r0, #168\n"
        "	movs	r1, #52\n"
        "	bl sub_0800F778\n"
        "	movs	r0, #255\n"
        "	lsls	r0, r0, #4\n"
        "	adds	r4, r4, r0\n"
        "	movs	r1, #0\n"
        "	ldrsh	r0, [r4, r1]\n"
        "	mov	r2, r8\n"
        "	movs	r3, #0\n"
        "	ldrsh	r1, [r2, r3]\n"
        "	movs	r3, #0\n"
        "	ldrsh	r2, [r6, r3]\n"
        "	bl 0x08025E98\n"
        "	adds	r2, r0, #0\n"
        "	movs	r0, #168\n"
        "	movs	r1, #76\n"
        "	bl sub_0800F778\n"
        "	adds	r0, r5, #0\n"
        "	adds	r0, #40\n"
        "	movs	r2, #150\n"
        "	lsls	r2, r2, #1\n"
        "	adds	r1, r5, r2\n"
        "	ldr	r1, [r1, #0]\n"
        "	movs	r3, #152\n"
        "	lsls	r3, r3, #1\n"
        "	adds	r2, r5, r3\n"
        "	ldr	r2, [r2, #0]\n"
        "	movs	r3, #44\n"
        "	str	r3, [sp, #0]\n"
        "	movs	r3, #3\n"
        "	str	r3, [sp, #4]\n"
        "	movs	r4, #1\n"
        "	str	r4, [sp, #8]\n"
        "	str	r4, [sp, #12]\n"
        "	movs	r6, #0\n"
        "	str	r6, [sp, #16]\n"
        "	movs	r3, #8\n"
        "	bl sub_08007BFC\n"
        "	adds	r0, r5, #0\n"
        "	adds	r0, #56\n"
        "	movs	r2, #156\n"
        "	lsls	r2, r2, #1\n"
        "	adds	r1, r5, r2\n"
        "	ldr	r1, [r1, #0]\n"
        "	movs	r3, #158\n"
        "	lsls	r3, r3, #1\n"
        "	adds	r2, r5, r3\n"
        "	ldr	r2, [r2, #0]\n"
        "	movs	r3, #56\n"
        "	str	r3, [sp, #0]\n"
        "	movs	r3, #12\n"
        "	str	r3, [sp, #4]\n"
        "	str	r4, [sp, #8]\n"
        "	str	r4, [sp, #12]\n"
        "	str	r6, [sp, #16]\n"
        "	movs	r3, #11\n"
        "	bl sub_08007BFC\n"
        "	adds	r0, r5, #0\n"
        "	bl sub_080011CBC\n"
        "	adds	r0, r5, #0\n"
        "	adds	r0, #224\n"
        "	movs	r1, #15\n"
        "	bl sub_0800D97C\n"
        "	adds	r0, r5, #0\n"
        "	adds	r0, #64\n"
        "	bl sub_0800DBE8\n"
        "	adds	r0, r5, #0\n"
        "	bl sub_080012574\n"
        "	add	sp, #20\n"
        "	pop	{r3}\n"
        "	mov	r8, r3\n"
        "	pop	{r4, r5, r6}\n"
        "	pop	{r0}\n"
        "	bx	r0\n"
        ".L120a8_080012198: .word 0x03001780\n"
        ".L120a8_08001219C: .word 0x00000FF2\n"
        ".L120a8_0800121A0: .word 0x00000FF6\n"
        ".L120a8_0800121A4: .word 0x0000103A\n"
        ".syntax divided\n"
    );
}

void _0800120A8(void *a) __attribute__((alias("MenuFF78_120A8")));
void sub_0800120A8(void *a) __attribute__((alias("MenuFF78_120A8")));

// ============================================================================
// Phase dispatchers (12-entry jump tables; `subs r0,#1; cmp #11`).
// ============================================================================

// ============================================================================
// sub_080012578 — countdown tick leaf.
// if u16[WA+0xFBC] == 3:
//   if _08002140 == 2: u32[rec+0x1CC] = 0
//   else: u32[rec+0x1CC]++; if > 180: emit scene event 21 via sub_08004D4C.
// ============================================================================
extern int  _08002140(void);
extern void _08004D4C(int a, int b, int c);
extern u8 FF78EWA[];
void MenuFF78_12578(volatile u8 *rec) {
    int n;
    u8 *wa = (u8 *)(uintptr_t)FF78EWA;
    u32 off = 0xFBC;
    __asm__(".globl FF78EWA\nFF78EWA = 0x03001780\n");
    if (*(volatile u16 *)(uintptr_t)(wa + off) != 3)
        return;
    if (_08002140() != 2) {
        n = (int)*(volatile u32 *)(rec + 0x1CC) + 1;
        *(volatile u32 *)(rec + 0x1CC) = (u32)n;
        if (n > 180)
            _08004D4C(21, 0, 0);
    } else {
        *(volatile u32 *)(rec + 0x1CC) = 0;
    }
}
void _080012578(volatile u8 *a) __attribute__((alias("MenuFF78_12578")));
void sub_080012578(volatile u8 *a) __attribute__((alias("MenuFF78_12578")));

// sub_0800125C4 — 12-way phase dispatcher (offsets 0x80 to 0x8B0 in table).
// We model it as a no-op since the cases are leaf stubs that don't run in
// our menu path. (The real 12 targets are lifted elsewhere or are no-ops.)
__attribute__((naked)) void MenuFF78_125C4(void *rec_, int a1, int a2, int a3) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush\t{r4, r5, r6, r7, lr}\n"
        "\tadds\tr5, r1, #0\n"
        "\tadds\tr6, r2, #0\n"
        "\tadds\tr4, r3, #0\n"
        "\tsubs\tr0, #1\n"
        "\tcmp\tr0, #11\n"
        "\tbls .Lff125c4__0800125D4\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__0800125D4:\n"
        "\tlsls\tr0, r0, #2\n"
        "\tldr r1, .Lff125c4__0800125E0\n"
        "\tadds\tr0, r0, r1\n"
        "\tldr\tr0, [r0, #0]\n"
        "\tmov\tpc, r0\n"
        "\tmovs\tr0, r0\n"
        ".Lff125c4__0800125E0: .word 0x080125E4\n"
        ".Lff125c4__0800125E4: .word 0x080127DE\n"
        ".Lff125c4__0800125E8: .word 0x08012614\n"
        ".Lff125c4__0800125EC: .word 0x080127EC\n"
        ".Lff125c4__0800125F0: .word 0x080127EC\n"
        ".Lff125c4__0800125F4: .word 0x0801261E\n"
        ".Lff125c4__0800125F8: .word 0x08012694\n"
        ".Lff125c4__0800125FC: .word 0x08012628\n"
        ".Lff125c4__080012600: .word 0x080127EC\n"
        ".Lff125c4__080012604: .word 0x080127EC\n"
        ".Lff125c4__080012608: .word 0x080127EC\n"
        ".Lff125c4__08001260C: .word 0x080127EC\n"
        ".Lff125c4__080012610: .word 0x080127E6\n"
        ".Lff125c4__080012614:\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr1, r5, #0\n"
        "\tbl sub_0800F924\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__08001261E:\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr0, #64\n"
        "\tbl sub_0800D854\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__080012628:\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr0, #214\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tcmp\tr0, #7\n"
        "\tbls .Lff125c4__080012634\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__080012634:\n"
        "\tlsls\tr0, r0, #2\n"
        "\tldr r1, .Lff125c4__080012640\n"
        "\tadds\tr0, r0, r1\n"
        "\tldr\tr0, [r0, #0]\n"
        "\tmov\tpc, r0\n"
        "\tmovs\tr0, r0\n"
        ".Lff125c4__080012640: .word 0x08012644\n"
        ".Lff125c4__080012644: .word 0x08012664\n"
        ".Lff125c4__080012648: .word 0x0801266C\n"
        ".Lff125c4__08001264C: .word 0x0801268C\n"
        ".Lff125c4__080012650: .word 0x08012674\n"
        ".Lff125c4__080012654: .word 0x0801267C\n"
        ".Lff125c4__080012658: .word 0x08012684\n"
        ".Lff125c4__08001265C: .word 0x08012684\n"
        ".Lff125c4__080012660: .word 0x0801268C\n"
        ".Lff125c4__080012664:\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl sub_080012034\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__08001266C:\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl sub_0800120A8\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__080012674:\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl sub_08001226C\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__08001267C:\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl sub_0800123D8\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__080012684:\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl sub_0800124EC\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__08001268C:\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl sub_0800121A8\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__080012694:\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl sub_080012578\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr0, #68\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tcmp\tr0, #0\n"
        "\tbne .Lff125c4__0800126A6\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__0800126A6:\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr0, #214\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tcmp\tr0, #7\n"
        "\tbls .Lff125c4__0800126B2\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__0800126B2:\n"
        "\tlsls\tr0, r0, #2\n"
        "\tldr r1, .Lff125c4__0800126BC\n"
        "\tadds\tr0, r0, r1\n"
        "\tldr\tr0, [r0, #0]\n"
        "\tmov\tpc, r0\n"
        ".Lff125c4__0800126BC: .word 0x080126C0\n"
        ".Lff125c4__0800126C0: .word 0x080127BE\n"
        ".Lff125c4__0800126C4: .word 0x080127CE\n"
        ".Lff125c4__0800126C8: .word 0x08012794\n"
        ".Lff125c4__0800126CC: .word 0x080126E0\n"
        ".Lff125c4__0800126D0: .word 0x0801270A\n"
        ".Lff125c4__0800126D4: .word 0x0801271A\n"
        ".Lff125c4__0800126D8: .word 0x08012742\n"
        ".Lff125c4__0800126DC: .word 0x08012794\n"
        ".Lff125c4__0800126E0:\n"
        "\tlsls\tr0, r5, #16\n"
        "\tlsrs\tr7, r0, #16\n"
        "\tlsls\tr0, r6, #16\n"
        "\tlsrs\tr5, r0, #16\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr2, r5, #0\n"
        "\tbl sub_08001096C\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr0, #168\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r0, r1]\n"
        "\tcmp\tr0, #0\n"
        "\tbne .Lff125c4__0800127EC\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr2, r5, #0\n"
        "\tbl sub_0800110A0\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__08001270A:\n"
        "\tlsls\tr1, r5, #16\n"
        "\tlsrs\tr1, r1, #16\n"
        "\tlsls\tr2, r6, #16\n"
        "\tlsrs\tr2, r2, #16\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl sub_0800109D4\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__08001271A:\n"
        "\tlsls\tr0, r5, #16\n"
        "\tlsrs\tr5, r0, #16\n"
        "\tlsls\tr0, r6, #16\n"
        "\tlsrs\tr6, r0, #16\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr1, r5, #0\n"
        "\tadds\tr2, r6, #0\n"
        "\tbl sub_080010CB8\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr0, #168\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r0, r1]\n"
        "\tcmp\tr0, #1\n"
        "\tbeq .Lff125c4__08001277C\n"
        "\tcmp\tr0, #1\n"
        "\tble .Lff125c4__080012764\n"
        "\tcmp\tr0, #2\n"
        "\tbne .Lff125c4__0800127EC\n"
        "\tb .Lff125c4__080012788\n"
        ".Lff125c4__080012742:\n"
        "\tlsls\tr0, r5, #16\n"
        "\tlsrs\tr5, r0, #16\n"
        "\tlsls\tr0, r6, #16\n"
        "\tlsrs\tr6, r0, #16\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr1, r5, #0\n"
        "\tadds\tr2, r6, #0\n"
        "\tbl sub_080010CB8\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr0, #168\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r0, r1]\n"
        "\tcmp\tr0, #1\n"
        "\tbeq .Lff125c4__08001277C\n"
        "\tcmp\tr0, #1\n"
        "\tbgt .Lff125c4__08001276A\n"
        ".Lff125c4__080012764:\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .Lff125c4__080012770\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__08001276A:\n"
        "\tcmp\tr0, #2\n"
        "\tbeq .Lff125c4__080012788\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__080012770:\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr1, r5, #0\n"
        "\tadds\tr2, r6, #0\n"
        "\tbl sub_080011484\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__08001277C:\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr1, r5, #0\n"
        "\tadds\tr2, r6, #0\n"
        "\tbl sub_0800115E0\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__080012788:\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr1, r5, #0\n"
        "\tadds\tr2, r6, #0\n"
        "\tbl sub_0800116DC\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__080012794:\n"
        "\tlsls\tr0, r5, #16\n"
        "\tlsrs\tr7, r0, #16\n"
        "\tlsls\tr0, r6, #16\n"
        "\tlsrs\tr5, r0, #16\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr2, r5, #0\n"
        "\tbl sub_08001096C\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr0, #168\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r0, r1]\n"
        "\tcmp\tr0, #0\n"
        "\tbne .Lff125c4__0800127EC\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr2, r5, #0\n"
        "\tbl sub_080010F3C\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__0800127BE:\n"
        "\tlsls\tr1, r5, #16\n"
        "\tlsrs\tr1, r1, #16\n"
        "\tlsls\tr2, r6, #16\n"
        "\tlsrs\tr2, r2, #16\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl sub_080010914\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__0800127CE:\n"
        "\tlsls\tr1, r5, #16\n"
        "\tlsrs\tr1, r1, #16\n"
        "\tlsls\tr2, r6, #16\n"
        "\tlsrs\tr2, r2, #16\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl sub_080010940\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__0800127DE:\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl sub_0800FF78\n"
        "\tb .Lff125c4__0800127EC\n"
        ".Lff125c4__0800127E6:\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl sub_0800F9A0\n"
        ".Lff125c4__0800127EC:\n"
        "\tpop\t{r4, r5, r6, r7}\n"
        "\tpop\t{r0}\n"
        "\tbx\tr0\n"
        "\tmovs\tr0, r0\n"
        ".syntax divided\n"
    );
}
void _0800125C4(void *a, int b, int c, int d) __attribute__((alias("MenuFF78_125C4")));
void sub_0800125C4(void *a, int b, int c, int d) __attribute__((alias("MenuFF78_125C4")));

// sub_080012DA0 — same pattern: 12-way phase dispatcher.
extern void sub_0800127F4(int a, void *b);
extern void sub_0800D854(void *p);
extern void sub_0800D8E4(void *p);
extern void sub_080012C7C(void *rec);
extern void sub_080012C90(void *rec);
extern void sub_080012D54(void *rec);
extern void sub_080012990(void *rec, u16 a1, u16 a2);
extern void sub_080012854(void *rec);
extern void sub_080012850(void *rec);

void MenuFF78_12DA0(int ev, int a1, int a2, void *rec_) {
    u8 *r4 = (u8 *)rec_;
    switch (ev) {
    case 2:
        sub_0800127F4((int)(uintptr_t)r4, (void *)(uintptr_t)a1);
        break;
    case 5: {
        extern u8 Wa12DA0[];
        extern u8 Fbc12DA0[];
        __asm__(".globl Wa12DA0\nWa12DA0 = 0x03001780\n"
                ".globl Fbc12DA0\nFbc12DA0 = 0x00000FBC\n");
        sub_0800D854(r4 + 8);
        sub_0800D8E4(r4 + 112);
        {
            register u8 *wa __asm__("r0") = Wa12DA0;
            register u32 off __asm__("r1") = (u32)(uintptr_t)Fbc12DA0;
            __asm__ volatile ("" : "+r"(wa), "+r"(off));
            if (*(volatile u16 *)(wa + off) == 3)
                sub_080012C7C(r4);
        }
        break;
    }
    case 7:
        sub_080012C90(r4);
        break;
    case 6:
        sub_080012D54(r4);
        if (*(volatile u16 *)(r4 + 12) != 0)
            sub_080012990(r4, (u16)a1, (u16)a2);
        break;
    case 1:
        sub_080012854(r4);
        break;
    case 12:
        sub_080012850(r4);
        break;
    }
}
void _080012DA0(int a, int b, int c, void *d) __attribute__((alias("MenuFF78_12DA0")));
void sub_080012DA0(int a, int b, int c, void *d) __attribute__((alias("MenuFF78_12DA0")));

// ============================================================================
// sub_080012F18 — strh leaf with sentinel walk.
// r4 = pointer to u32 array; r5 = index. u32[r4] == 0xFFFFFFFF -> stop.
// if _08025FAC(u32[r4]) != 0, write u32[r4] -> u32[base + 188 + 4*index]
// then r4 += 4, r5++. After loop, write u16(r5-1) -> u16[base+184]
// and u32[base + 188 + 4*r5] = 0xFFFFFFFF.
// ============================================================================
extern int _08025FAC(u32 v);
// EXACT at 84/84 bytes (probe: _080012F18 --require-exact).
//
// FINDING (the anti-giv pins): the ROM recomputes the store address every
// iteration -- `lsls r0,r5,#2 / adds r1,r6,#0 / adds r1,#188 / adds r1,r1,r0`
// -- instead of strength-reducing `rec+188+n*4` into an auto-increment
// pointer.  Left alone agbcc's loop pass forms a DEST_ADDR giv over the count
// (mult 4 == SImode) and, because ARM defines AUTO_INC_DEC, the giv gets the
// auto-increment bonus in the benefit test (loop.c:4560-4584) so it is never
// dropped.  It then emits `stmia r7!,{r0}` and needs a fourth callee-saved
// register: `push {r4,r5,r6,r7,lr}`, which shifts the whole body by one and
// costs the first 17 bytes.  Pinning the scaled index to r0 (initialised
// first) and keeping the base a plain local defeats the recognizer (the
// address source is an asm operand), reproduces the per-iteration
// recomputation AND the ROM's instruction order.  An asm `+r` barrier on the
// count works too but the pin alone is enough, so it is omitted.
void MenuFF78_12F18(void *rec_) {
    u8 *rec = (u8 *)rec_;
    // Non-volatile s16/u32 reads so the halfword load lowers to `ldrsh` and
    // the array read stays a plain `ldr`; the ROM reloads `[r4]` after the
    // call instead of keeping the value live across it.
    u32 *r4 = (u32 *)_08024D4C(*(s16 *)(rec + 176));
    int r5 = 0;
    while (*r4 != 0xFFFFFFFFu) {
        if (_08025FAC(*r4) != 0) {
            register u32 idx_ __asm__("r0") = r5 * 4;
            u8 *b_ = rec + 188;
            *(u32 *)(uintptr_t)(b_ + idx_) = *r4;
            r5++;
        }
        r4++;
    }
    // Tail: the ROM computes `r5-1` into r1 BEFORE materialising `rec+184`
    // into r0, then reuses r0 for `rec+188` (`adds r0,#4`) and adds the scaled
    // index (`lsls r1,r5,#2 / adds r0,r0,r1`).  Declaring the value pin first
    // and chaining `e_ += 4` after the index pin reproduces both the order and
    // the register reuse.
    {
        register s32 n_ __asm__("r1") = (s32)r5 - 1;
        register u8 *e_ __asm__("r0") = rec + 184;
        *(s16 *)e_ = (s16)n_;
        n_ = (s32)r5 * 4;
        e_ += 4;
        *(u32 *)(uintptr_t)(e_ + n_) = 0xFFFFFFFFu;
    }
}
// The ROM pads the body's last two bytes with 0x0000 after `bx r0`; agbcc
// emits a one-instruction `nop` (0xC046) there. The explicit `0` fill selects
// the ROM's bytes (same device as MenuFF78_12E9C/12ED0 above).
__asm__(".align 2, 0");
void _080012F18(void *a) __attribute__((alias("MenuFF78_12F18")));
void Sub_080012F18(void *a) __attribute__((alias("MenuFF78_12F18")));
void sub_080012F18(void *a) __attribute__((alias("MenuFF78_12F18")));

// ============================================================================
// sub_0800127F4 — flag setter based on car id + WA gate.
// if u16[WA+0xFBC] == 3, strb 0 -> [r1+88] (countdown flag clear).
// then check sub_08004B68->u16[+2] (car id):
//   20, 35 -> s16[r1+84] = 5
//   39     -> s16[r1+84] = 1
//   else   -> s16[r1+84] = 6
// ============================================================================
extern void *_08004B68(void);
void MenuFF78_127F4(int unused_, void *rec_) {
    (void)unused_;
    extern u8 MenuFF78E_Wa[] __asm__("MenuFF78E_Wa");
    uintptr_t base = (uintptr_t)MenuFF78E_Wa;
    if (*(volatile u16 *)(uintptr_t)(base + 0xFBCu) == 3) {
        *(volatile u8 *)(uintptr_t)((volatile u8 *)rec_ + 88) = 0;
    }
    // The ROM reads the car id with a REGISTER index holding a BYTE offset of
    // 2: `movs r1,#2; ldrsh r0,[r0,r1] @ 0x08012810..12`. An s16 subscript of
    // 1 gives that byte offset; subscript 2 scales to `#4`. A non-volatile
    // s16 lvalue is what yields `ldrsh` rather than `ldrh` + shifts.
    s16 car_id = ((s16 *)(uintptr_t)sub_08004B68())[1];
    // The ROM's ladder jumps into THREE shared store blocks, not four:
    // `beq -> 0x08012832` serves BOTH car 35 (0x08012816) and car 20
    // (0x0801281E), `beq -> 0x0801283A` serves 39 (0x0801282E), and both
    // defaults `b -> 0x08012842`. Only a goto-style tail merge emits three
    // blocks; a plain if/else ladder duplicates the second `== 5` store and
    // grows the body by 4 bytes.
//
    // `cmp r0,#35; bgt @ 0x08012818` targets 0x0801282C, which sits AFTER the
    // literal pool — so the `> 35` arm is the out-of-line one and the
    // `<= 35` ladder is the fall-through. Stating the `<= 35` arm first is
    // what makes agbcc lay the blocks out in the ROM's order.
    if (car_id == 35) goto set5;
    if (car_id <= 35) {
        if (car_id == 20) goto set5;
        goto set6;
    }
    if (car_id == 39) goto set1;
    goto set6;
set5:
    *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 84) = 5;
    goto store;
set1:
    *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 84) = 1;
    goto store;
set6:
    *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 84) = 6;
store:
    return;
}
void _0800127F4(int a, void *b) __attribute__((alias("MenuFF78_127F4")));
void sub_0800127F4(int a, void *b) __attribute__((alias("MenuFF78_127F4")));

// ============================================================================
// sub_080012E9C — small leaf: if u32[rec+144]==0, sub_0800D77C(rec+156,0,88);
// if u32[rec+144]==1, sub_0800D77C(rec+156,0,-32); then u32[rec+144]=2.
//
// FINDING : the `rec` local was costing an instruction the ROM does
// not have. The ROM's first two body instructions are `adds r4, r0, #0 @
// 0x08012E9E` then `adds r4, #144 @ 0x08012EA0` — one copy of the pointer,
// biased in place. With the named `rec` local, agbcc also keeps a second,
// unbiased copy live across the branch and emits `adds r2, r0, #0` at
// 0x08012E9E, so the body desynchronises at +2. Inlining the parameter
// removes it: prefix 2 -> 8, candidate 52 -> 48 bytes. The byte COUNT falls
// (26 -> 13) while the prefix rises, which is the expected signature of
// deleting the misaligning instruction; prefix is the sound signal here.
extern void _0800D77C(void *a, int b, int c);
void MenuFF78_12E9C(void *rec_) {
    s32 v = *(volatile s32 *)(uintptr_t)((volatile u8 *)rec_ + 144);
    if (v != 1) {
        if (v < 2) {
            if (v == 0) {
                _0800D77C((void *)(uintptr_t)((volatile u8 *)rec_ + 156), 0, 88);
                *(volatile u32 *)(uintptr_t)((volatile u8 *)rec_ + 144) = 2;
            }
        }
    } else {
        _0800D77C((void *)(uintptr_t)((volatile u8 *)rec_ + 156), 0, -32);
        *(volatile u32 *)(uintptr_t)((volatile u8 *)rec_ + 144) = 2;
    }
}
__asm__(".align 2, 0");
void _080012E9C(void *a) __attribute__((alias("MenuFF78_12E9C")));
void Sub_080012E9C(void *a) __attribute__((alias("MenuFF78_12E9C")));
void sub_080012E9C(void *a) __attribute__((alias("MenuFF78_12E9C")));

// ============================================================================
// sub_080012ED0 — car-id -> flag mapper (two-way: 35 -> 5, else -> 6).
//
// FINDING : the WA read is a TWO-POOL-WORD add, not a folded
// constant — `ldr r0,=0x03001780; ldr r1,=0x0FBC; adds r0,r0,r1;
// ldrh r0,[r0]` (0x08012ED4..0x08012EDA). A folded `0x03001780u+0xFBCu`
// emits a single 16-bit pool word and loses both ldrs and the adds, which
// is exactly the +2 first difference. Same non-foldable symbol recipe as
// MenuFF78_12F6C below.
// ============================================================================
void MenuFF78_12ED0(int unused_, void *rec_) {
    (void)unused_;
    extern u8 MenuFF78E_Wa[] __asm__("MenuFF78E_Wa");
    __asm__(".globl MenuFF78E_Wa\nMenuFF78E_Wa = 0x03001780\n");
    uintptr_t base = (uintptr_t)MenuFF78E_Wa;
    if (*(volatile u16 *)(uintptr_t)(base + 0xFBCu) == 3) {
        *(volatile u8 *)(uintptr_t)((volatile u8 *)rec_ + 88) = 0;
    }
    u16 car_id = *(volatile u16 *)(uintptr_t)((volatile u8 *)sub_08004B68() + 2);
    if (car_id == 35) {
        *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 84) = 5;
    } else {
        *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 84) = 6;
    }
}
// The ROM pads the body's last two bytes with 0x0000 (0x08012F12..0x08012F13)
// after `bx r0`; agbcc emits a one-instruction `nop` (0xC046) there. The
// explicit `0` fill selects the ROM's bytes.
__asm__(".align 2, 0");
void _080012ED0(int a, void *b) __attribute__((alias("MenuFF78_12ED0")));
void Sub_080012ED0(int a, void *b) __attribute__((alias("MenuFF78_12ED0")));
void sub_080012ED0(int a, void *b) __attribute__((alias("MenuFF78_12ED0")));

// ============================================================================
// sub_080012F6C — small leaf: u32[rec+188] = s16[WA+0xFC2]; u32[rec+192] = -1;
// u16[rec+184] = 0; bx lr.
// ============================================================================
void MenuFF78_12F6C(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u32 *q;
    extern u8 MenuFF78E_Wa[] __asm__("MenuFF78E_Wa");
    __asm__(".globl MenuFF78E_Wa\nMenuFF78E_Wa = 0x03001780\n");
    uintptr_t base = (uintptr_t)MenuFF78E_Wa;
    q = (u32 *)(uintptr_t)(rec + 188);
    register u32 idx __asm__("r3") = 0;
    s16 v = ((s16 *)(uintptr_t)(base + 0xFC2u))[idx];
    *(volatile u32 *)q = (u32)(s32)v;
    {
        u32 *q2 = (u32 *)(uintptr_t)((u8 *)q + 4);
        *(volatile u32 *)q2 = 0xFFFFFFFFu;
        *(volatile u16 *)(uintptr_t)((u8 *)q2 - 8) = 0;
    }
}
void _080012F6C(void *a) __attribute__((alias("MenuFF78_12F6C")));
void sub_080012F6C(void *a) __attribute__((alias("MenuFF78_12F6C")));

// ============================================================================
// sub_080012F98 — screen init: sub_080012F6C + u16[rec+174]=u16[WA+0xFC2];
// u16[rec+176]=10; u16[rec+136]=0; then _080022E4(s16[rec+174])->u16[rec+180];
// read byte at WA + (s16*3*4+49) -> s16[rec+172]; s8 at +48 -> s16[rec+178];
// u32[rec+144]=2; u16[rec+186]=0; sub_0800D77C(rec+156,0,-32).
//
// EXACT at 140/140 bytes, including the literal pool. The work-area base uses
// the SYMBOL form (RAM_DEF/RAM_SYM_03001780), the [rec+174] field is re-read
// at each site, and the WA+0xFC2 value goes through a local so its store address
// stays after the load. Pinning the shared zero to r5 and preserving the two
// byte-load sequences also keeps their original order around the stores.
// ============================================================================
extern int _080022E4(int a);
void MenuFF78_12F98(void *rec_) {
    // The ROM keeps the work-area base live in r4 (`ldr r4,=0x03001780`) and
    // folds nothing: the +0xFC2 read is `ldr r1,=0xFC2 / adds r0,r4,r1`. Two
    // spellings failed before this one: the numeric `0x03001780u + 0xFC2u`
    // folds to a single 0x03002742 pool word (no add at all), and a plain
    // `volatile u8 *wa = (0x03001780u)` local still lets agbcc rematerialise
    // the constant at each of the three uses (`ldr r0,[pc,..]` x3, +8 bytes)
    // instead of keeping it live. The SYMBOL form (RAM_DEF/RAM_SYM_03001780,
    // the same lever src/save.c documents) is not a foldable constant, so the
    // base stays in r4 across the whole body.
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *wa;
    volatile u16 *slot174;
    u16 h;
    register u32 zero __asm__("r5");
    RAM_DEF_03001780;
    MenuFF78_12F6C(rec_);
    wa = RAM_SYM_03001780;
    // The work-area read goes into a local first: inline, agbcc hoists the
    // store's address computation (rec+174) above the load.
    h = *(volatile u16 *)(uintptr_t)(wa + 0xFC2u);
    slot174 = (volatile u16 *)(uintptr_t)(rec + 174);
    zero = 0;
    // Keep `movs r5,#0` at the ROM point, before the first store.
    __asm__ volatile("" : "+r"(zero));
    // The field is volatile, so each of the three later uses re-reads it rather
    // than reusing a local (`ldrsh r0,[r7,r1]` with the address kept in r7).
    *slot174 = h;
    *(volatile u16 *)(uintptr_t)(rec + 176) = 10;
    *(volatile u16 *)(uintptr_t)(rec + 136) = (u16)zero;
    *(volatile u16 *)(uintptr_t)(rec + 180) =
        (u16)_080022E4((int)*(s16 *)(uintptr_t)(rec + 174));
    {
        // Keep the zero index in r1; the signed C read emits the ROM's
        // `ldrsb r1,[r0,r1]` register-offset load.
        u8 *base = (u8 *)wa;
        u8 *p1 = base + ((int)*(s16 *)(uintptr_t)(rec + 174) * 3) * 4 + 49;
        register s32 b1 __asm__("r1") = 0;
        b1 = *(s8 *)(p1 + b1);
        *(volatile s16 *)(uintptr_t)(rec + 172) = (s16)b1;
        // Keep the +48 signed-byte read before the rec+178 address; agbcc
        // otherwise hoists the destination calculation above the read.
        register s32 b2 __asm__("r0") = (s32)(uintptr_t)(
            base + ((int)*(s16 *)(uintptr_t)(rec + 174) * 3) * 4 + 48);
        b2 = *(s8 *)(uintptr_t)b2;
        *(volatile s16 *)(uintptr_t)(rec + 178) = (s16)b2;
    }
    *(volatile u32 *)(uintptr_t)(rec + 144) = 2;
    *(volatile u16 *)(uintptr_t)(rec + 186) = (u16)zero;
    _0800D77C((void *)(uintptr_t)(rec + 156), 0, -32);
}
void _080012F98(void *a) __attribute__((alias("MenuFF78_12F98")));
void Sub_080012F98(void *a) __attribute__((alias("MenuFF78_12F98")));
void sub_080012F98(void *a) __attribute__((alias("MenuFF78_12F98")));
