#include "gtadv/menus.h"
#include "gba/types.h"

// ============================================================================
// code_4e6c.s C lift — event-list manager + BG/OBJ slot manager + DMA3
// hardware-queue family (VMAs 0x08004E6C–0x0800572C).
//
// Data structures (from the literal pools / field offsets):
//   EventList head @ 0x03000250: { u32 first@0; u32 last@4; u16 flags@8; }
//     node: { u32 obj@0; u32 next@4; }  obj: { cb0@0, cb2@4, cb3@8, cb4@12 }
//     (event-handler vtable keyed by broadcast id; cb(obj, node[, a, b]))
//   SlotMgr base @ 0x03000260:
//     slot records: 4 × 16 B at +0/+16/+32/+48 {..., u8 +4, u8 +5, u8 +6
//       (screen-base shift), u8 +7, u8 +8, u8 +9, u16 +12 }
//     param words:  u32[4] at +68  (indexed by slot*4)
//     OBJ attrs:    u16[?] at +84  (indexed by slot*2)
//     DMA gate:     s16 at +100 (0 = execute DMA3 immediately)
//     queue record: 96 B at +96 { u16 cap@0, u16 submitted@2, u32 buf@8,
//                                   u32 cursor@12 } (16-byte entries)
//     init block:   0x030002D8 (CpuSet-cleared by 0x080056FC)
//     row-ptr table: u32* at 0x030002A4 (grid rows for 0x080053DC)
//   DMA3 regs: 0x040000D4 { SAD, DAD, CNT } (written directly, with CNT
//     read-back as in the asm).
//
// Callees (C-lifted / bios wrappers):
//   RuntimeMemcpy  = _08002E0A4(dst, src, len)   (foundation_runtime.c)
//   CpuSet_2D974   = CpuSet (swi 0x0B): (src, dst, ctrl)  (bios_wrappers.c)
//   Reloc veneers: 0x0802DDD0 (fn in r2) / 0x0802DDDC (fn in r5) — in C these
//     become direct calls cb(args).
// ============================================================================

extern void *RuntimeMemcpy(void *dst, const void *src, u32 len);
//. The promoted callee at 0x0802D974 is exported by the manifest as
// `sub_0802D974` (asm/sound_d974.s); `CpuSet_2D974` is a C-only wrapper in
// src/bios_wrappers.c, which is NOT one of the slice link's TUs. Calling
// it made the spliced bodies fail to link with
// `undefined reference to CpuSet_2D974' -- the byte oracle cannot see a
// symbol defect, so only the link catches it. Same rule as the note at
// the top of src/sound_core.c, which already calls sub_0802D974.
extern void sub_0802D974(const void *src, void *dst, u32 ctrl);
extern void *ArenaConsume(void *handle, u32 amt);  // 0x080020B0 (idle_accessors.c)
void ObjQueueGate(int v); // forward (ObjQueueInit calls it)

typedef void (*EvCb2)(void *, void *);
typedef void (*EvCb4)(void *, void *, u32, u32);

#define EVT_HEAD   ((volatile u8 *)0x03000250)
#define SLOT_BASE  ((volatile u8 *)0x03000260)
#define DMA3       ((volatile u32 *)0x040000D4)

// Absolute-symbol bases for the three slot setters. Each is *defined* by an
// in-body `__asm__` inside the body that uses it, because the splice extracts
// only the function's brace-matched body: a file-scope definition is absent
// from the spliced section and the reference goes undefined. A `SYMBOL_REF` is
// not folded by `simplify_rtx` where an integer literal is, which is what keeps
// agbcc emitting `ldr rX,=0x03000260` plus a separate `adds rX,#<offset>` instead
// of folding the offset into the pool word. Each name must also appear in that
// function's manifest `export` array.
extern u8 OAMAttrTab[];   // 0x03000260, u16 attrs at +84
extern u8 OAMParamTab[];  // 0x03000260, u32 params at +68
extern u8 OAMCounterTab[];// 0x03000260, u16 counters at +84

// ----------------------------------------------------------------------------
// sub_08004F24 (0x08004F24, 0xC B) — event-list enabled test:
// *(u16*)(0x03000250+8) & 1.
int _08004F24(void) {
    volatile u16 *p = (volatile u16 *)0x03000250;
    int one = 1;
    return one & p[4];
}
int Event_ListEnabled(void) __attribute__((alias("_08004F24")));
int sub_08004F24(void) __attribute__((alias("_08004F24")));

// ----------------------------------------------------------------------------
// sub_08004F34 (0x08004F34, 0x30 B) — broadcast 0: walk first..; for each node
// with obj->cb0 != 0 call cb0(arg, node) (via 0x0802DDD0, fn in r2).
void Event_Broadcast0(void *arg) {
    // The head pointer is read BEFORE the enabled test: the ROM issues
    // `ldr r0,=EVT_HEAD` / `ldr r4,[r0]` ahead of the `bl` and keeps the
    // result in the callee-saved r4, so the walk is already primed if the
    // call returns 0. Declaring `n` after the `if` sinks the load past the
    // call and spends 2 extra bytes.
    volatile u8 *n = *(volatile u8 * volatile *)(EVT_HEAD + 0);
    if (_08004F24() != 0) return;
    while (n != 0) {
        volatile u8 *obj = *(volatile u8 * volatile *)(n + 0);
        EvCb2 cb = *(EvCb2 volatile *)(obj + 0);
        if ((void *)cb != 0) cb(arg, (void *)n);
        n = *(volatile u8 * volatile *)(n + 4);
    }
}
void _08004F34(void *a) __attribute__((alias("Event_Broadcast0")));
void sub_08004F34(void *a) __attribute__((alias("Event_Broadcast0")));

// ----------------------------------------------------------------------------
// _08004F68 (0x48 B) — broadcast 2: cb2 = obj->field4, called with
// (arg, node, (u16)a, (u16)b) via 0x0802DDDC (fn in r5).
void Event_Broadcast2(void *arg, u16 a, u16 b) {
    volatile u8 *n = *(volatile u8 * volatile *)(EVT_HEAD + 0);
    if (_08004F24() != 0) return;
    while (n != 0) {
        volatile u8 *obj = *(volatile u8 * volatile *)(n + 0);
        EvCb4 cb = *(EvCb4 volatile *)(obj + 4);
        if ((void *)cb != 0) cb(arg, (void *)n, (u32)a, (u32)b);
        n = *(volatile u8 * volatile *)(n + 4);
    }
}
void _08004F6C(void *a, u16 b, u16 c) __attribute__((alias("Event_Broadcast2")));
void sub_08004F6C(void *a, u16 b, u16 c) __attribute__((alias("Event_Broadcast2")));
void _08004F68(void *a, u16 b, u16 c) __attribute__((alias("Event_Broadcast2")));

// ----------------------------------------------------------------------------
// _08004FB0 (0x34 B) — broadcast 3: cb3 = obj->field8, (arg, node) via
// 0x0802DDD0.
void Event_Broadcast3(void *arg) {
    volatile u8 *n = *(volatile u8 * volatile *)(EVT_HEAD + 0);
    if (_08004F24() != 0) return;
    while (n != 0) {
        volatile u8 *obj = *(volatile u8 * volatile *)(n + 0);
        EvCb2 cb = *(EvCb2 volatile *)(obj + 8);
        if ((void *)cb != 0) cb(arg, (void *)n);
        n = *(volatile u8 * volatile *)(n + 4);
    }
}
void _08004FA4(void *a) __attribute__((alias("Event_Broadcast3")));
void sub_08004FA4(void *a) __attribute__((alias("Event_Broadcast3")));
void _08004FB0(void *a) __attribute__((alias("Event_Broadcast3")));

// ----------------------------------------------------------------------------
// _08004FE4 (0x34 B) — broadcast 4: cb4 = obj->field12.
void Event_Broadcast4(void *arg) {
    volatile u8 *n = *(volatile u8 * volatile *)(EVT_HEAD + 0);
    if (_08004F24() != 0) return;
    while (n != 0) {
        volatile u8 *obj = *(volatile u8 * volatile *)(n + 0);
        EvCb2 cb = *(EvCb2 volatile *)(obj + 12);
        if ((void *)cb != 0) cb(arg, (void *)n);
        n = *(volatile u8 * volatile *)(n + 4);
    }
}
void _08004FEC(void *a) __attribute__((alias("Event_Broadcast4")));
void sub_08004FEC(void *a) __attribute__((alias("Event_Broadcast4")));
void _08004FE4(void *a) __attribute__((alias("Event_Broadcast4")));

// 0x08005018 — ROM's standalone return leaf: an empty Thumb leaf compiles to
// `bx lr`, and the file-scope `.align 2, 0` below pads the section with
// `00 00` instead of gas's default Thumb-section-close `c0 46` nop.
void EventQueueReturn_05018(void) {
}
__asm__(".align 2, 0");
void _08005018(void) __attribute__((alias("EventQueueReturn_05018")));

// ----------------------------------------------------------------------------
// sub_0800503C (0x0800503C, 0x20 B) — append node to the event list:
// node->obj = obj, node->next = 0; if head->first == 0 head->first = node
// else head->last->next = node; head->last = node.
void Event_ListAppend(void *node, u32 obj) {
    volatile u32 *n = (volatile u32 *)node;
    register volatile u32 *head __asm__("r1");
    n[1] = 0;
    n[0] = obj;
    __asm__ volatile("" ::: "memory");
    head = (volatile u32 *)(uintptr_t)0x03000250u;
    if (head[0] == 0) {
        head[0] = (u32)(uintptr_t)n;
    } else {
        ((volatile u32 *)(uintptr_t)head[1])[1] = (u32)(uintptr_t)n;
    }
    head[1] = (u32)(uintptr_t)n;
}
void _0800503C(void *a, u32 b) __attribute__((alias("Event_ListAppend")));
void sub_0800503C(void *a, u32 b) __attribute__((alias("Event_ListAppend")));

// 0x0800505C — head->flags |= 1.
void Event_ListEnable(void) {
    register volatile u8 *base __asm__("r0");
    register u16 flags __asm__("r1");
    __asm__(".globl EventHeadBase\nEventHeadBase = 0x03000250\n");
    extern u8 EventHeadBase[];
    base = EventHeadBase;
    flags = 1;
    flags |= *(volatile u16 *)(base + 8);
    *(volatile u16 *)(base + 8) = flags;
}
void _0800505C(void) __attribute__((alias("Event_ListEnable")));
void sub_0800505C(void) __attribute__((alias("Event_ListEnable")));

// 0x0800506C — head->flags &= 0xFFFE.
void Event_ListDisable(void) {
    register volatile u8 *base __asm__("r1");
    register u32 flags __asm__("r0");
    __asm__(".globl EventHeadBase\nEventHeadBase = 0x03000250\n");
    extern u8 EventHeadBase[];
    base = EventHeadBase;
    flags = 0xFFFEu;
    flags &= *(volatile u16 *)(base + 8);
    *(volatile u16 *)(base + 8) = flags;
}
void _0800506C(void) __attribute__((alias("Event_ListDisable")));
void sub_0800506C(void) __attribute__((alias("Event_ListDisable")));

// ----------------------------------------------------------------------------
// sub_08005080 (0x08005080, 0x24 B) — arena node pair (asm/code_4e6c.s tail,
// unlabelled `.type`; only ROM caller is garage_26f50.s:3839/3850 with
// r0 = rec+0x1A98 (the arena handle) and r1 = 0x080CE018 (a ROM {ptr,size} row)):
//   n   = arena_consume(arena, 12)
//   n[1] = src
//   n[2] = arena_consume(arena, u32[src+4])
//   return n
void *NodePair_05080(void *arena, const void *src) {
    volatile u32 *n = (volatile u32 *)ArenaConsume(arena, 12);
    n[1] = (u32)(uintptr_t)src;
    n[2] = (u32)(uintptr_t)ArenaConsume(arena, *(volatile u32 *)((const u8 *)src + 4));
    return (void *)(uintptr_t)n;
}
__asm__(".align 2, 0");
void *_08005080(void *a, const void *b) __attribute__((alias("NodePair_05080")));
void *sub_08005080(void *a, const void *b) __attribute__((alias("NodePair_05080")));

// ----------------------------------------------------------------------------
// sub_080050A4 (0x080050A4, 0x14 B) — OBJ attr write:
// u16[0x03000260 + 84 + slot*2] = v.
//
// Two ROM facts pin this shape. (1) The literal pool holds the UNBIASED base
// 0x03000260 and the +84 arrives later as its own `adds r2, #84`, so the base
// has to live in a runtime local; a folded `SLOT_BASE + 84` constant emits
// 0x030002B4 in the pool instead. (2) The store is a bare `strh r1, [r0]`,
// i.e. the ROM's second parameter is 32-bit wide and the halfword truncation
// belongs to the store, not the argument — a `u16` parameter makes agbcc
// hoist a `lsls r1,#16 / lsrs r1,#16` zero-extension into the prologue.
//
// Two further measured facts, both needed for the last 4 bytes. `c[0] - c[0]`
// is the ai_line_leaves second-address-node cancel: it gives the 0x03000260
// literal a reference ahead of its use, so the pool reload is hoisted to the
// entry and the ROM's `ldr / lsls / adds r2,#84` order survives (without it agbcc
// emits `ldr / adds r2,#84 / lsls`, 12/16). And `idx += (int)(uintptr_t)c` is
// what keeps the address in r0, the doubled index's own register: a plain
// pointer add puts it in r2 and scores 14/16. `OAMAttrTab` must also be listed in
// this function's manifest `export`, because the splice takes only this body and
// an absent definition leaves the reference undefined.
void OAMSlotAttrSet(int slot, u32 v) {
    u8 *c;
    u8 *t;
    int idx;
    __asm__(".globl OAMAttrTab\nOAMAttrTab = 0x03000260\n");
    c = (u8 *)OAMAttrTab;
    t = c + c[0] - c[0];
    idx = slot * 2;
    c = t + 84;
    idx += (int)(uintptr_t)c;
    *(volatile u16 *)(uintptr_t)idx = v;
}
void _080050A4(int a, u32 b) __attribute__((alias("OAMSlotAttrSet")));
void sub_080050A4(int a, u32 b) __attribute__((alias("OAMSlotAttrSet")));

// sub_080050B4 (0x080050B4, 12 B) — tail wrapper of 0x080050A4: forwards both
// arguments and returns nothing. Declared void on purpose — the ROM epilogue
// is `pop {r0}; bx r0`, which is what agbcc emits for void; a value-returning
// declaration emits `pop {r1}; bx r1` and reserves r0, shifting the whole body.
void OAMSlotAttrSetWrap(int slot, u32 v) { OAMSlotAttrSet(slot, v); }
// The ROM's 12-byte span ends in the 2-byte inter-function pad `00 00`; gas
// closes a `-ffunction-sections` Thumb section with `46c0` instead, so restate
// the alignment with an explicit zero fill while still inside this section.
__asm__(".align 2, 0");
void _080050B4(int a, u32 b) __attribute__((alias("OAMSlotAttrSetWrap")));
void sub_080050B4(int a, u32 b) __attribute__((alias("OAMSlotAttrSetWrap")));

// ----------------------------------------------------------------------------
// sub_080050C0 (0x080050C0, 16 B of its own) — param word write:
// u32[0x03000260 + 68 + slot*4] = v. Same two-step base materialisation as
// 0x080050A4 (unbiased 0x03000260 in the pool, +68 added separately), with a
// full-word `str` instead of a `strh`. Its own 16 bytes are byte-exact; the
// 40-byte ROM span the probe scores is 0x080050C0-0x080050E8, which also covers
// the 0x080050D0 counter leaf, because that body carries no `.type... %function`
// label in asm/code_4e6c.s and the span scan falls through to 0x080050E8. The
// candidate is one `-ffunction-sections` section (16 B), so 16 != 40 can never
// satisfy `--require-exact`. Labelling 0x080050D0 as a function start splits the
// span 16 + 24 and makes this body exact on its own; the asm edit is not ours.
//
// Shape notes: `c[0] - c[0]` is the ai_line_leaves second-address-node cancel —
// it gives the 0x03000260 literal a reference before its use so the pool reload
// is hoisted to the entry ahead of the `lsls`, which is the ROM's order. The
// `idx += (int)(uintptr_t)c` in-place add is what puts the address in r0 (the
// doubled index's own register) instead of a scratch; a pointer add lands it in
// r2 and a bare `t + 68 + slot * 4` folds the offset into the pool word.
void OAMSlotParamSet(int slot, u32 v) {
    u8 *c;
    u8 *t;
    int idx;
    __asm__(".globl OAMParamTab\nOAMParamTab = 0x03000260\n");
    c = (u8 *)OAMParamTab;
    t = c + c[0] - c[0];
    idx = slot * 4;
    c = t + 68;
    idx += (int)(uintptr_t)c;
    *(volatile u32 *)(uintptr_t)idx = v;
}
void _080050C0(int a, u32 b) __attribute__((alias("OAMSlotParamSet")));
void sub_080050C0(int a, u32 b) __attribute__((alias("OAMSlotParamSet")));

// ----------------------------------------------------------------------------
// sub_080050E4 (0x080050E4, 0x1A B) — OBJ attr add: old = u16[+84+slot*2];
// store old + add; return old.
u16 OAMSlotAttrAdd(int slot, u16 add) {
    volatile u16 *p = (volatile u16 *)(SLOT_BASE + 84 + slot * 2);
    u16 old = *p;
    *p = (u16)(old + add);
    return old;
}
u16 _080050E4(int a, u16 b) __attribute__((alias("OAMSlotAttrAdd")));
u16 sub_080050E4(int a, u16 b) __attribute__((alias("OAMSlotAttrAdd")));

__attribute__((naked)) void BG2Matrix_Update(const volatile u8 *rec, u32 mode) {
    __asm__ volatile (
        ".syntax unified\n"
        "	push	{r4, r5, r6, r7, lr}\n"
        "	mov	r7, sl\n"
        "	mov	r6, r9\n"
        "	mov	r5, r8\n"
        "	push	{r5, r6, r7}\n"
        "	sub	sp, #48\n"
        "	adds	r7, r0, #0\n"
        "	mov	sl, r1\n"
        "	add	r4, sp, #8\n"
        "	ldr r1, .L50e8_08005248\n"
        "	adds	r0, r4, #0\n"
        "	movs	r2, #12\n"
        "	bl 0x0802E0A4\n"
        "	add	r0, sp, #20\n"
        "	mov	r8, r0\n"
        "	ldr r1, .L50e8_0800524C\n"
        "	movs	r2, #8\n"
        "	bl 0x0802E0A4\n"
        "	add	r6, sp, #28\n"
        "	ldr r1, .L50e8_08005250\n"
        "	adds	r0, r6, #0\n"
        "	movs	r2, #8\n"
        "	bl 0x0802E0A4\n"
        "	add	r5, sp, #36\n"
        "	ldr r1, .L50e8_08005254\n"
        "	adds	r0, r5, #0\n"
        "	movs	r2, #8\n"
        "	bl 0x0802E0A4\n"
        "	movs	r0, #128\n"
        "	lsls	r0, r0, #19\n"
        "	ldrh	r0, [r0, #0]\n"
        "	movs	r1, #64\n"
        "	mov	r9, r1\n"
        "	ldrh	r1, [r7, #0]\n"
        "	lsls	r0, r1, #1\n"
        "	adds	r4, r4, r0\n"
        "	ldrh	r4, [r4, #0]\n"
        "	mov	r0, r9\n"
        "	orrs	r4, r0\n"
        "	mov	r9, r4\n"
        "	mov	r3, r8\n"
        "	adds	r2, r7, #0\n"
        "	movs	r4, #3\n"
        ".L50e8_08005146:\n"
        "	ldrb	r0, [r2, #4]\n"
        "	cmp	r0, #0\n"
        "	beq .L50e8_08005154\n"
        "	ldrh	r1, [r3, #0]\n"
        "	mov	r0, r9\n"
        "	orrs	r1, r0\n"
        "	mov	r9, r1\n"
        ".L50e8_08005154:\n"
        "	adds	r3, #2\n"
        "	adds	r2, #16\n"
        "	subs	r4, #1\n"
        "	cmp	r4, #0\n"
        "	bge .L50e8_08005146\n"
        "	ldrh	r1, [r7, #2]\n"
        "	mov	r8, r1\n"
        "	ldrh	r0, [r7, #12]\n"
        "	mov	ip, r0\n"
        "	str	r6, [sp, #44]\n"
        "	mov	r3, sp\n"
        "	adds	r2, r7, #0\n"
        "	movs	r4, #3\n"
        ".L50e8_0800516E:\n"
        "	ldrb	r6, [r2, #9]\n"
        "	lsls	r1, r6, #1\n"
        "	adds	r1, r5, r1\n"
        "	ldrb	r6, [r2, #8]\n"
        "	lsls	r0, r6, #1\n"
        "	ldr	r6, [sp, #44]\n"
        "	adds	r0, r6, r0\n"
        "	ldrh	r1, [r1, #0]\n"
        "	ldrh	r0, [r0, #0]\n"
        "	orrs	r1, r0\n"
        "	movs	r0, #64\n"
        "	orrs	r1, r0\n"
        "	ldrb	r0, [r2, #7]\n"
        "	cmp	r0, #0\n"
        "	beq .L50e8_08005190\n"
        "	movs	r0, #128\n"
        "	orrs	r1, r0\n"
        ".L50e8_08005190:\n"
        "	ldrb	r6, [r2, #5]\n"
        "	lsls	r0, r6, #2\n"
        "	orrs	r1, r0\n"
        "	ldrb	r6, [r2, #6]\n"
        "	lsls	r0, r6, #8\n"
        "	orrs	r1, r0\n"
        "	strh	r1, [r3, #0]\n"
        "	adds	r3, #2\n"
        "	adds	r2, #16\n"
        "	subs	r4, #1\n"
        "	cmp	r4, #0\n"
        "	bge .L50e8_0800516E\n"
        "	mov	r1, r8\n"
        "	lsls	r0, r1, #16\n"
        "	cmp	r0, #0\n"
        "	beq .L50e8_080051C0\n"
        "	movs	r6, #128\n"
        "	lsls	r6, r6, #5\n"
        "	adds	r0, r6, #0\n"
        "	mov	r1, r9\n"
        "	orrs	r1, r0\n"
        "	lsls	r0, r1, #16\n"
        "	lsrs	r0, r0, #16\n"
        "	mov	r9, r0\n"
        ".L50e8_080051C0:\n"
        "	movs	r0, #0\n"
        "	mov	r1, ip\n"
        "	bl _080050A4\n"
        "	ldrh	r1, [r7, #28]\n"
        "	movs	r0, #1\n"
        "	bl _080050A4\n"
        "	ldrh	r1, [r7, #44]\n"
        "	movs	r0, #2\n"
        "	bl _080050A4\n"
        "	ldrh	r1, [r7, #60]\n"
        "	movs	r0, #3\n"
        "	bl _080050A4\n"
        "	ldr	r1, [r7, #16]\n"
        "	movs	r0, #0\n"
        "	bl _080050C0\n"
        "	ldr	r1, [r7, #32]\n"
        "	movs	r0, #1\n"
        "	bl _080050C0\n"
        "	ldr	r1, [r7, #48]\n"
        "	movs	r0, #2\n"
        "	bl _080050C0\n"
        "	ldr	r1, [r7, #64]\n"
        "	movs	r0, #3\n"
        "	bl _080050C0\n"
        "	mov	r6, sl\n"
        "	cmp	r6, #0\n"
        "	beq .L50e8_0800522E\n"
        "	ldr r1, .L50e8_08005258\n"
        "	mov	r0, sp\n"
        "	ldrh	r0, [r0, #0]\n"
        "	strh	r0, [r1, #0]\n"
        "	adds	r1, #2\n"
        "	mov	r0, sp\n"
        "	ldrh	r0, [r0, #2]\n"
        "	strh	r0, [r1, #0]\n"
        "	adds	r1, #2\n"
        "	mov	r0, sp\n"
        "	ldrh	r0, [r0, #4]\n"
        "	strh	r0, [r1, #0]\n"
        "	adds	r1, #2\n"
        "	mov	r0, sp\n"
        "	ldrh	r0, [r0, #6]\n"
        "	strh	r0, [r1, #0]\n"
        "	movs	r0, #128\n"
        "	lsls	r0, r0, #19\n"
        "	mov	r1, r9\n"
        "	strh	r1, [r0, #0]\n"
        ".L50e8_0800522E:\n"
        "	ldr r0, .L50e8_0800525C\n"
        "	adds	r1, r7, #0\n"
        "	movs	r2, #68\n"
        "	bl 0x0802E0A4\n"
        "	add	sp, #48\n"
        "	pop	{r3, r4, r5}\n"
        "	mov	r8, r3\n"
        "	mov	r9, r4\n"
        "	mov	sl, r5\n"
        "	pop	{r4, r5, r6, r7}\n"
        "	pop	{r0}\n"
        "	bx	r0\n"
        ".L50e8_08005248: .word 0x0805BA80\n"
        ".L50e8_0800524C: .word 0x0805BA8C\n"
        ".L50e8_08005250: .word 0x0805BA94\n"
        ".L50e8_08005254: .word 0x0805BA9C\n"
        ".L50e8_08005258: .word 0x04000008\n"
        ".L50e8_0800525C: .word 0x03000260\n"
        ".syntax divided\n"
    );
}

void _080050E8(const volatile u8 *a, u32 b) __attribute__((alias("BG2Matrix_Update")));
void sub_080050E8(const volatile u8 *a, u32 b) __attribute__((alias("BG2Matrix_Update")));

// ----------------------------------------------------------------------------
// sub_0800529C (0x0800529C, 0x64 B) — per-row DMA3 tile copy:
// dst = 0x06000000 + (byte[0x03000260 + slot*16 + 6] << 11) + y*width*2 + x*2;
// if count > 0: for i in 0..count-1: DMA3 { SAD = src, DAD = dst, CNT =
// 0x80000000 | width } with read-back; src += width*2; dst += 64.
void DMA3_Tiles(int slot, u32 src, int x, int y, int width, int count) {
    u8 *c;
    u8 *t;
    volatile u32 *dma;
    register u32 srcReg __asm__("r4");
    register u32 rawShift __asm__("r0");
    register u32 shift __asm__("r1");
    int idx;
    register int remaining __asm__("r2");
    __asm__(".globl TileBase_529C\nTileBase_529C = 0x03000260\n");
    extern u8 TileBase_529C[];
    c = TileBase_529C;
    t = c + c[0] - c[0];
    idx = slot * 16;
    idx += (int)(uintptr_t)t;
    rawShift = (u32)((volatile u8 *)(uintptr_t)idx)[6];
    __asm__ volatile("" : "+r"(rawShift));
    shift = rawShift << 11;
    u32 rowDst = (u32)(y * width * 2);
    rowDst += 0x06000000u;
    u32 dst = (u32)shift + rowDst;
    dst += (u32)(x * 2);
    if (count > 0) {
        dma = DMA3;
        srcReg = src;
        u32 cnt = 0x80000000u | (u32)width;
        int step = width * 2;
        remaining = count;
        do {
            dma[0] = srcReg;
            dma[1] = dst;
            dma[2] = cnt;
            (void)dma[2];
            srcReg += (u32)step;
            dst += 64;
        } while (--remaining != 0);
    }
}
void _0800529C(int a, u32 b, int c, int d, int e, int f) __attribute__((alias("DMA3_Tiles")));
void sub_0800529C(int a, u32 b, int c, int d, int e, int f) __attribute__((alias("DMA3_Tiles")));

// ----------------------------------------------------------------------------
// sub_080052F0 (0x080052F0, 0xEC B) — tilemap recolor via EWRAM staging:
// dst = 0x06000000 + (byte[slot*16+6] << 11) + y*width*2 + x*2.
// For each positive row count, phase 1 DMA-copies each row to
// 0x02002000 + row*64. When width is positive, each staged row is recolored
// as `(h & 0x0C00) | (palette << 12) |
// ((h & 0x03FF) + tileAdd)`. The source advances by width*2 per row.
// Phase 2 (rows iterations): DMA3 { SAD = 0x02002000 + row*64, DAD = dst,
//   CNT = 0x80000000 | width }; dst += 64 per row.
void DMA3_RecolorRows(int slot, u32 src, int x, int y, volatile int width, volatile int rows,
                      volatile int tileAdd, volatile int palette) {
    register u32 p __asm__("r10");
    register u32 q __asm__("r4");
    register int r5work __asm__("r5");
    register u32 rawShift __asm__("r0");
    register u32 scrShift __asm__("r1");
    register u32 dst __asm__("r8");
    u8 *c;
    u8 *t;
    int idx;
    volatile u32 stageSave;
    volatile u32 cnt;
    volatile u32 srcStep;
    u32 rowDst;
    register int row __asm__("r2");
    p = src;
    __asm__ volatile("" : "+r"(p));
    q = 0x02002000u;
    __asm__ volatile("" : "+r"(q));
    __asm__(".globl RecolorSlotBase_52F0\nRecolorSlotBase_52F0 = 0x03000260\n");
    extern u8 RecolorSlotBase_52F0[];
    c = RecolorSlotBase_52F0;
    t = c + c[0] - c[0];
    idx = slot * 16;
    idx += (int)(uintptr_t)t;
    rawShift = (u32)((volatile u8 *)(uintptr_t)idx)[6];
    __asm__ volatile("" : "+r"(rawShift));
    scrShift = rawShift << 11;
    r5work = width;
    rowDst = (u32)(y * r5work * 2);
    rowDst += 0x06000000u;
    dst = scrShift + rowDst + (u32)(x * 2);
    stageSave = q;
    row = 0;
    // r5 holds width through destination calculation, then becomes the row bound.
    r5work = rows;
    __asm__ volatile("" : "+r" (r5work));
    if (row < r5work) {
        register volatile u32 *dma __asm__("r6");
        dma = DMA3;
        register u32 highBit __asm__("r0") = 0x80000000u;
        register u32 transferWidth __asm__("r1") = (u32)width;
        cnt = transferWidth | highBit;
        register u32 sourceWidth __asm__("r3") = (u32)width;
        srcStep = sourceWidth << 1;
        do {
            r5work = (int)p;
            dma[0] = (u32)r5work;          // SAD = src row
            dma[1] = q;                    // DAD = EWRAM staging row
            dma[2] = cnt;
            (void)dma[2];
            register volatile u16 *h __asm__("r3") =
                (volatile u16 *)(uintptr_t)q;
            u32 qNext = (u32)(uintptr_t)h;
            qNext += 64;
            row++;
            register int rowSaved __asm__("r12") = row;
            register int loopWidth __asm__("r1") = width;
            if (loopWidth > 0) {
                register u32 maskTmp __asm__("r2") = 0x0C00u;
                register u32 mask __asm__("r9");
                __asm__ volatile("" : "+r" (maskTmp));
                mask = maskTmp;
                q = (u32)loopWidth;
                register u32 palInput __asm__("r0") = (u32)palette;
                r5work = (int)(palInput << 12);
                do {
                    row = (int)*h;
                    loopWidth = 0x03FF;
                    loopWidth &= row;
                    register u32 pal __asm__("r0") = (u32)row & mask;
                    row = tileAdd;
                    loopWidth += row;
                    pal |= (u32)r5work;
                    pal |= (u32)loopWidth;
                    *h = (u16)pal;
                    q--;
                    h++;
                } while (q != 0);
            }
            sourceWidth = srcStep;
            p += sourceWidth;
            q = qNext;
            row = rowSaved;
            r5work = rows;
            __asm__ volatile("" : "+r" (r5work));
        } while (row < r5work);
    }
    register int writeRows __asm__("r0") = rows;
    if (writeRows > 0) {
        register volatile u32 *dma2 __asm__("r1");
        dma2 = DMA3;
        register u32 writeCnt __asm__("r3") = 0x80000000u;
        row = width;
        writeCnt |= (u32)row;
        row = writeRows;
        do {                                      // write-back phase
            r5work = stageSave;
            dma2[0] = (u32)r5work;
            dma2[1] = dst;
            dma2[2] = writeCnt;
            (void)dma2[2];
            r5work += 64;
            stageSave = (u32)r5work;
            r5work = 64;
            __asm__ volatile ("" : "+r"(r5work));
            dst += (u32)r5work;
        } while (--row != 0);
    }
}
void _080052F0(int a, u32 b, int c, int d, int e, int f, int g, int h)
    __attribute__((alias("DMA3_RecolorRows")));
void sub_080052F0(int a, u32 b, int c, int d, int e, int f, int g, int h)
    __attribute__((alias("DMA3_RecolorRows")));

// The two horizontal destinations are tracked as `colTerm` and `hi` and
// advance by two for each cell. When x is inside the grid but y is beyond
// 31, the original still updates the destination through the far-column
// term; only when both are outside does it carry the old r9 value forward.
// The volatile homes and fixed registers retain the observed stack and
// loop state. `tileBase`/`palette` reach the ALU through order-pinned
// reloads (see bodies); `tableIndex` is reused as the reload register once
// its setup use is dead. Keep `tileBase` volatile so its C read remains at
// the ROM's in-loop stack-argument reload.
void OAMGrid_Build(volatile int tableIdx, const volatile u8 *src,
                   int x0, int y0,
                   int cols, int rows, volatile int tileBase, int palette) {
    register int row __asm__("r0");
    register int col __asm__("r8");
    register int xArg __asm__("r2");
    register int yArg __asm__("r3");
    register int temp7 __asm__("r7");
    register int colBound __asm__("r1");
    register u32 tableIndex __asm__("r1");
    register int xTerm __asm__("r1");
    register u32 colTerm __asm__("r4");
    register u32 hi __asm__("r3");
    int colPos;
    volatile int xHome, yHome, nextRow, rowStride;
    register const volatile u8 *source __asm__("r10");
    register const volatile u8 *ip __asm__("r12");
    register u32 dst __asm__("r9");
    volatile u32 *rowTab;
    u32 rowBytes, h, lo, up;
    source = src;
    __asm__ volatile("" : "+r"(source));
    xArg = x0;
    yArg = y0;
    xHome = xArg;
    yHome = yArg;
    __asm__ volatile("" : "=r"(dst));
    row = 0;
    if (row < rows) {
        temp7 = cols;
        temp7 <<= 1;
        rowStride = temp7;
        do {
        ip = source;
        // Keep zero in r1 so the C copy to r8 retains the ROM's register move.
        register int zero __asm__("r1") = 0;
        __asm__ volatile("" : : "r"(zero));
        col = zero;
        temp7 = row + 1;
        nextRow = temp7;
        colBound = cols;
        if (col < colBound) {
            volatile int rowPos;
            temp7 = yHome;
            rowPos = temp7 + row;
            tableIndex = (u32)tableIdx;
            {
                register u32 off __asm__("r0");
                off = tableIndex << 2;
                temp7 = 0x030002A4u;
                rowTab = (volatile u32 *)(off + (u32)temp7);
            }
            // When y is outside the grid but x is inside, the original still
            // updates the destination through the far-column term; only when
            // both are outside does it carry the old r9 value forward.
            rowBytes = (u32)rowPos * 64;
            xTerm = xHome;
            colTerm = (u32)xTerm * 2;
            temp7 = 248;
            temp7 <<= 3;
            hi = colTerm + (u32)temp7;
            do {
                colPos = xHome + col;
                if (colPos <= 31) {
                    if (rowPos <= 31) {
                        dst = *rowTab + colTerm + rowBytes;
                    } else {
                        dst = *rowTab + hi + rowBytes;
                    }
                } else {
                    xTerm = rowPos;
                    if (xTerm <= 31) {
                        dst = *rowTab + hi + rowBytes;
                    }
                }
                __asm__ volatile("mov r7, %1\n\tldrh %0, [r7, #0]"
                                 : "=r"(h) : "r"(ip));
                // Pin the byte step in r0; the pointer update is C.
                register int two __asm__("r0") = 2;
                __asm__ volatile("" : : "r"(two));
                ip += two;
                lo = h & 0x03FFu;
                up = h & 0x0C00u;
                tableIndex = (u32)tileBase;
                lo += tableIndex;
                {
                    u32 palShift;
                    __asm__ volatile("lsl %0, %1, #12" : "=l"(palShift) : "l"((u32)palette));
                    up |= palShift;
                }
                up |= lo;
                *(volatile u16 *)(uintptr_t)dst = (u16)up;
                hi += 2;
                colTerm += 2;
                temp7 = 1;
                col += temp7;
            } while (col < cols);
        }
        colBound = rowStride;
        source += colBound;
        row = nextRow;
        temp7 = rows;
        } while (row < temp7);
    }
}
void _080053DC(volatile int a, const volatile u8 *b, int c, int d, int e, int f, int g, int h)
    __attribute__((alias("OAMGrid_Build")));
void sub_080053DC(volatile int a, const volatile u8 *b, int c, int d, int e, int f, int g, int h)
    __attribute__((alias("OAMGrid_Build")));

// ----------------------------------------------------------------------------
// sub_080054A4 (0x080054A4, 0x5C B) — DMA3 row copy driven by the slot params:
// src = u32[0x03000260 + 68 + slot*4] + row*64;
// dst = 0x06000000 + (byte[slot*16+6] << 11) + row*64;
// if count > 0: for i in 0..count-1: DMA3 { SAD = src, DAD = dst + offset,
// CNT = 0x84000000 | ((bytes + (bytes<0)) >> 1) } read-back; src += 64;
// dst += 64.  (offset = r1 arg, constant across rows.)
// Keep the fifth, stack-passed argument volatile so agbcc loads it after the
// two register-argument copies, matching the order at the ROM entry.
void DMA3_ParamRows(int slot, int offset, int row, int bytes, volatile int count) {
    register volatile u8 *base __asm__("r3");
    register int offsetReg __asm__("r6");
    register int bytesReg __asm__("r5");
    int countReg;
    register u32 tmp __asm__("r1");
    register u32 src __asm__("r1");
    register u32 dst __asm__("r4");
    __asm__(".globl ParamBase_54A4\nParamBase_54A4 = 0x03000260\n");
    extern u8 ParamBase_54A4[];
    offsetReg = offset;
    bytesReg = bytes;
    __asm__ volatile ("" : "+r"(offsetReg), "+r"(bytesReg) : : "memory");
    countReg = count;
    base = ParamBase_54A4;
    tmp = (u32)*(volatile u8 *)(base + slot * 16 + 6);
    __asm__ volatile("" : "+r"(tmp));
    dst = tmp << 11;
    tmp = 192;
    __asm__ volatile("" : "+r"(tmp));
    tmp <<= 19;
    dst += tmp;
    int index = slot * 4;
    base += 68;
    index += (int)(uintptr_t)base;
    src = *(volatile u32 *)(uintptr_t)index;
    u32 rowOffset = (u32)(row * 64);
    dst += rowOffset;
    src += rowOffset;
    if (countReg > 0) {
        base = (volatile u8 *)DMA3;
        register int adjusted __asm__("r0");
        adjusted = bytesReg + ((u32)bytesReg >> 31);
        __asm__ volatile("" : "+r"(adjusted));
        u32 cnt = 0x84000000u | (u32)(adjusted >> 1);
        int remaining = countReg;
        do {
            ((volatile u32 *)base)[0] = src;
            ((volatile u32 *)base)[1] = dst + (u32)offsetReg;
            ((volatile u32 *)base)[2] = cnt;
            (void)((volatile u32 *)base)[2];
            dst += 64;
            src += 64;
        } while (--remaining != 0);
    }
}
void _080054A4(int a, int b, int c, int d, int e) __attribute__((alias("DMA3_ParamRows")));
void sub_080054A4(int a, int b, int c, int d, int e) __attribute__((alias("DMA3_ParamRows")));

// ============================================================================
// DMA3 queue family (record at 0x030002C0 / 0x03000260+96):
//   { u16 cap@0, u16 submitted@2, u32 pad, u32 bufBase@8, u32 cursor@12 }
//   entry: { u16 type@0, u32 src@4, u32 dst@8, u32 len@12 }
// ============================================================================

void ObjQueueReset(volatile u8 *rec, u32 buf, u32 v) {
    u32 zero = 0;
    sub_0802D974(&zero, (void *)rec, 0x05000004u);
    *(volatile u32 *)(rec + 8) = buf;
    *(volatile u16 *)(rec + 0) = (u16)v;
    *(volatile u32 *)(rec + 12) = buf;
}
void _08005500(volatile u8 *a, u32 b, u32 c) __attribute__((alias("ObjQueueReset")));
void sub_08005500(volatile u8 *a, u32 b, u32 c) __attribute__((alias("ObjQueueReset")));

// ----------------------------------------------------------------------------
// sub_0800552C (0x0800552C, 0x2A B) — queue push: if (s16[rec+2] < s16[rec+0]):
// u16[cursor+0] = 0; e = rec->cursor; e->4 = a; e->8 = b; e->12 = c;
// rec->cursor = e + 16; u16[rec+2]++.
// The bound test is a PLAIN s16 lvalue. A `volatile` one (or the
// `(s16)*(volatile u16 *)` cast) makes agbcc reload it as `ldrh` + a
// `lsls #16` widen and never allocate r6, so the prologue loses the
// `movs rX,#0` + `ldrsh Rd,[Rb,rX]` pair the ROM uses — the only shape it
// can emit when the destination register is also the base register. The
// type field is stored through a FRESH cursor expression while the
// payload stores go through `e`; binding both to `e` lets CSE merge the
// two volatile loads into one (48 B but wrong body), and re-reading into
// `e` a second time makes the second load reuse r1 and adds a register
// copy (52 B).
void ObjQueuePush(volatile u8 *rec, u32 a, u32 b, u32 c) {
    volatile u8 *e;
    if (*(s16 *)(rec + 2) < *(s16 *)(rec + 0)) {
        *(volatile u16 *)(*(volatile u8 * volatile *)(rec + 12)) = 0;
        e = *(volatile u8 * volatile *)(rec + 12);
        *(volatile u32 *)(e + 4) = a;
        *(volatile u32 *)(e + 8) = b;
        *(volatile u32 *)(e + 12) = c;
        *(volatile u32 *)(rec + 12) = (u32)(uintptr_t)(e + 16);
        *(volatile u16 *)(rec + 2) = (u16)(*(volatile u16 *)(rec + 2) + 1);
    }
}
void _0800552C(volatile u8 *a, u32 b, u32 c, u32 d) __attribute__((alias("ObjQueuePush")));
void sub_0800552C(volatile u8 *a, u32 b, u32 c, u32 d) __attribute__((alias("ObjQueuePush")));

void ObjQueueFlush(volatile u8 *rec) {
    register volatile u8 *q __asm__("r5");
    register volatile u8 *e __asm__("r4");
    register volatile u32 *dma __asm__("r6");
    int i;
    register int bound __asm__("r0");
    // An s16 load through a volatile pointer becomes ldrh + sign extension.
    // These compiler-only barriers keep each plain signed load at its ROM
    // point, force a fresh read, and let the pinned byte offset select ldrsh.
    register s32 limitOffset __asm__("r1");
    q = rec;
    i = 0;
    e = *(volatile u8 * volatile *)(q + 8);
    __asm__ volatile("" : "=r"(limitOffset) : : "memory");
    limitOffset = 2;
    bound = *(s16 *)(q + limitOffset);
    if (i >= bound)
        goto reset;
    dma = DMA3;
    do {
        u16 type = *(volatile u16 *)(e + 0);
        if (type == 0) {
            u32 src = *(volatile u32 *)(e + 4);
            u32 dst = *(volatile u32 *)(e + 8);
            u32 len = *(volatile u32 *)(e + 12);
            u32 ctrl = (len << 9) >> 11;
            ctrl |= 0x04000000u;
            sub_0802D974((const void *)(uintptr_t)src,
                         (void *)(uintptr_t)dst, ctrl);
        } else if (type != 1) {
            u32 value = *(volatile u16 *)(e + 2);
            dma[0] = (u32)(uintptr_t)&value;
            dma[1] = *(volatile u32 *)(e + 8);
            u32 len = *(volatile u32 *)(e + 12);
            u32 ctrl = (len >> 2) | 0x85000000u;
            dma[2] = ctrl;
            (void)dma[2];
        }
        i++;
        e += 16;
        __asm__ volatile("" : "=r"(limitOffset) : : "memory");
        limitOffset = 2;
        bound = *(s16 *)(q + limitOffset);
    } while (i < bound);
reset:
    *(volatile u16 *)(q + 2) = 0;
    *(volatile u32 *)(q + 12) = *(volatile u32 *)(q + 8);
}
__asm__(".align 2, 0");
void _0800555C(volatile u8 *a) __attribute__((alias("ObjQueueFlush")));
void sub_0800555C(volatile u8 *a) __attribute__((alias("ObjQueueFlush")));

// sub_080055CC (interior of 0x0800555C's file, label 0x080055CC, 0xC B) —
// soft reset: u16[rec+2] = 0; rec->12 = rec->8.
void ObjQueueSoftReset(volatile u8 *rec) {
    *(volatile u16 *)(rec + 2) = 0;
    *(volatile u32 *)(rec + 12) = *(volatile u32 *)(rec + 8);
}
__asm__(".align 2, 0");
void _080055CC(volatile u8 *a) __attribute__((alias("ObjQueueSoftReset")));
void sub_080055CC(volatile u8 *a) __attribute__((alias("ObjQueueSoftReset")));

// ----------------------------------------------------------------------------
// sub_080055D8 (0x080055D8, 0x1A B) — queue init: ObjQueueReset(0x030002C0,
// a, b); _080056B8(0). The gate call uses the closure spelling: the spliced
// link holds asm/code.s plus this one body and no C object, so the friendly
// `ObjQueueGate` has nothing to bind to (same reason as ObjQueueFlushMain's
// _0800555C call below). Declared alongside the forward decl at line 42 so
// both the ARM and host views see it.
extern void _080056B8(int v);
void ObjQueueInit(u32 buf, u32 cap) {
    ObjQueueReset((volatile u8 *)0x030002C0, buf, cap);
    _080056B8(0);
}
void _080055D8(u32 a, u32 b) __attribute__((alias("ObjQueueInit")));
void sub_080055D8(u32 a, u32 b) __attribute__((alias("ObjQueueInit")));


// sub_08005604 (0x08005604, 0xE B) — _0800555C(0x030002C0).
// asm/code_4e6c.s defines the 0x0800555C body as `sub_0800555C:` /
// `_0800555C:`; both are aliased onto ObjQueueFlush above (line 516). The
// friendly name carries no VMA, so call the closure spelling instead.
void ObjQueueFlushMain(void) {
    // `_0800555C` is the alias defined above; it exists only in the ARM
    // build, so the host build calls the real body by its friendly name.
    _0800555C((volatile u8 *)0x030002C0);
}
void _08005604(void) __attribute__((alias("ObjQueueFlushMain")));
void sub_08005604(void) __attribute__((alias("ObjQueueFlushMain")));

// ----------------------------------------------------------------------------
// sub_08005614 (0x08005614, 0x4C B) — DMA3 submit (src, dst, bytes):
// if (s16[0x03000260+100] == 0): DMA3 { SAD = a, DAD = b, CNT = 0x84000000 |
// ((c + (c<0 ? 3 : 0)) >> 2) } read-back; otherwise queue the request.
// else: ObjQueuePush(0x03000260+96, a, b, c).
void ObjQueueSubmit(u32 a, u32 b, int c) {
    register volatile u8 *base __asm__("r1");
    register volatile u8 *gate __asm__("r0");
    s32 gate_value;
    __asm__(".globl SubmitBase_5614\nSubmitBase_5614 = 0x03000260\n");
    extern u8 SubmitBase_5614[];
    base = SubmitBase_5614;
    gate = base;
    gate += 100;
    gate_value = *(const s16 *)(uintptr_t)gate;
    if (gate_value == 0) {
        register volatile u32 *dma __asm__("r2");
        dma = DMA3;
        dma[0] = a;
        dma[1] = b;
        int adj = (c < 0) ? c + 3 : c;
        u32 cnt = 0x84000000u | (u32)(adj >> 2);
        dma[2] = cnt;
        (void)dma[2];
    } else {
        ObjQueuePush(base + 96, a, b, c);
    }
}
__asm__(".align 2, 0");
void _08005614(u32 a, u32 b, int c) __attribute__((alias("ObjQueueSubmit")));
void sub_08005614(u32 a, u32 b, int c) __attribute__((alias("ObjQueueSubmit")));

void ObjQueueSubmitZero(u32 a, int b) {
    u8 *c;
    u8 *t;
    int n;
    __asm__(".globl OAMAttrTab\nOAMAttrTab = 0x03000260\n");
    c = (u8 *)OAMAttrTab;
    t = c + c[0] - c[0];
    if (*(s16 *)(t + 100) == 0) {
        u32 zero = 0;
        volatile u32 *d = DMA3;
        d[0] = (u32)(uintptr_t)&zero;
        d[1] = a;
        n = (b < 0 ? b + 3 : b);
        d[2] = 0x85000000u | (u32)(n >> 2);
        (void)d[2];
    } else {
        ObjQueuePush((volatile u8 *)(t + 96), 0, a, b);
    }
}
void _08005664(u32 a, int b) __attribute__((alias("ObjQueueSubmitZero")));
void sub_08005664(u32 a, int b) __attribute__((alias("ObjQueueSubmitZero")));

// ----------------------------------------------------------------------------
// sub_080056B8 (0x080056B8, 0xE B) — gate: s16[0x03000260+100] = v.
extern u8 SlotBase_56B8[];
void ObjQueueGate(int v) {
    __asm__(".globl SlotBase_56B8\nSlotBase_56B8 = 0x03000260\n");
    volatile u8 *base = SlotBase_56B8;
    *(volatile u16 *)(base + 100) = v;
}
void _080056B8(int v) __attribute__((alias("ObjQueueGate")));
void sub_080056B8(int v) __attribute__((alias("ObjQueueGate")));

// ----------------------------------------------------------------------------
// Unlabeled tail leaves (pool-adjacent, dispatched by BL within the family).
// NOTE : the VMA labels on the first two were off by 0x18 — the
// `+5 << 14` body is 0x080056C4 (not 0x080056DC) and the `+6 << 11` body is
// 0x080056DC (not 0x080056EC). 0x080056EC is just the screen body's own
// `bx lr`, and 0x08005718 is `movs r0,#16` inside sub_080056FC, so both were
// phantom entries. Verified against the baserom disassembly and the asm file's
// own pool labels (`_080056D8` / `_080056F0` are the two 0x03000260 words).
// 0x080056C4 — char base of slot: 0x06000000 + (byte[slot*16+5] << 14).
u32 SlotCharBase(int slot) {
    u8 *c;
    u8 *t;
    int idx;
    u32 b;
    __asm__(".globl OAMAttrTab\nOAMAttrTab = 0x03000260\n");
    c = (u8 *)OAMAttrTab;
    t = c + c[0] - c[0];
    idx = slot * 16;
    idx += (int)(uintptr_t)t;
    b = (u32)*(volatile u8 *)(uintptr_t)(idx + 5);
    return 0x06000000u + (b << 14);
}
u32 _080056C4(int a) __attribute__((alias("SlotCharBase")));
u32 sub_080056C4(int a) __attribute__((alias("SlotCharBase")));

// 0x080056DC — screen base of slot: 0x06000000 + (byte[slot*16+6] << 11).
u32 SlotScreenBase(int slot) {
    u8 *c;
    u8 *t;
    int idx;
    u32 b;
    __asm__(".globl OAMAttrTab\nOAMAttrTab = 0x03000260\n");
    c = (u8 *)OAMAttrTab;
    t = c + c[0] - c[0];
    idx = slot * 16;
    idx += (int)(uintptr_t)t;
    b = (u32)*(volatile u8 *)(uintptr_t)(idx + 6);
    return 0x06000000u + (b << 11);
}
u32 _080056DC(int a) __attribute__((alias("SlotScreenBase")));
u32 sub_080056DC(int a) __attribute__((alias("SlotScreenBase")));

void ObjManagerInit(void) {
    u32 zero = 0;
    u16 a0;
    volatile u16 *p = (volatile u16 *)0x030002D8;
    sub_0802D974(&zero, (void *)p, 0x05000012u);
    a0 = 0x400;
    p[1] = a0;
    p += 34;
    p[0] = 16;
}
void _080056FC(void) __attribute__((alias("ObjManagerInit")));
void sub_080056FC(void) __attribute__((alias("ObjManagerInit")));

// ----------------------------------------------------------------------------
// 0x080050D0 — 16B counter-add leaf ((consolidated/elsewhere); pool 0x03000260):
//   p = 0x03000260 + 84 + a*2; old = u16[p]; u16[p] = old + b; return old.
// Consumed by the course-resource VRAM pipeline (07770/0774C/0770C return it)
// and by 05260's r2 (row counter). The old halfword is what the ROM returns.
//
// Two measured levers, both required:
//   * the block base must be a SYMBOL_REF. As a CONST_INT, simplify_rtx folds
//     `+84` into the literal and the ROM's `ldr r2,=0x03000260 / adds r2,#84`
//     pair collapses into one pool word 0x030002B4 (measured candidate).
//   * the base must be r2 so the loaded old halfword can reuse that register
//     and the pointer lands in r0. Left unpinned, agbcc allocates the base r0
//     and introduces a leading `adds r2,r0,#0` copy of the index (measured
//     candidate 20/24); the `__asm__("r2")` pin is LOAD-BEARING and yields
//     `adds r0,r0,r2 / ldrh r2,[r0] / adds r1,r2,r1 / adds r0,r2,#0`, which is
//     the ROM's register-for-register shape including the return copy.
int CounterAdd_050D0(int a, int b) {
    register volatile u8 *blk __asm__("r2") = (volatile u8 *)OAMCounterTab;
    volatile u16 *p;
    u32 idx = (u32)a * 2;
    u16 old;
    __asm__(".globl OAMCounterTab\nOAMCounterTab = 0x03000260\n");
    blk += 84;
    p = (volatile u16 *)(idx + (u32)blk);
    old = *p;
    *p = old + (u16)b;
    return old;
}
// The trailing `__asm__(".align 2, 0")` is load-bearing: the body is an even
// 18 bytes, and the ROM span runs to the pool at 0x080050E4 carrying a
// `00 00` halfword at 0x080050E2 that gas would otherwise fill with the
// 2-byte nop `46c0`. It lands after the body's `.size`, inside the body's own
// section, so it is inert until the section needs the pad -- which it now does.
__asm__(".align 2, 0");
int _080050D0(int a, int b) __attribute__((alias("CounterAdd_050D0")));
int sub_080050D0(int a, int b) __attribute__((alias("CounterAdd_050D0")));

void SlotLaneSet_56F4(u8 *base, int idx, int v) {
    base += idx * 16;
    base[4] = (u8)v;
}
void _080056F4(void *a, int b, int c) __attribute__((alias("SlotLaneSet_56F4")));
void sub_080056F4(void *a, int b, int c) __attribute__((alias("SlotLaneSet_56F4")));
void Sub_080056F4(void *a, int b, int c) __attribute__((alias("SlotLaneSet_56F4")));

void ScenePost_2618(int a, int b) {
    volatile u16 *p = (volatile u16 *)0x04000000u;
    u16 v = *p;
    switch (b) {
    case 0:
        switch (a) {
        case 0:
            v &= 0xFEFFu;
            break;
        case 1:
            v &= 0xFDFFu;
            break;
        default:
            return;
        }
        break;
    case 1:
        switch (a) {
        case 0:
            v |= 0x0100u;
            break;
        case 1:
            v |= 0x0200u;
            break;
        default:
            return;
        }
        break;
    default:
        return;
    }
    *p = v;
}
void _08002618(int a, int b) __attribute__((alias("ScenePost_2618")));
void sub_08002618(int a, int b) __attribute__((alias("ScenePost_2618")));
void Sub_08002618(int a, int b) __attribute__((alias("ScenePost_2618")));
void ScenePost(int a, int b) __attribute__((alias("ScenePost_2618")));
