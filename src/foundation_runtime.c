#include "gtadv/foundation.h"
#include "gtadv/memory.h"
#include "gba/regs.h"
#include "gba/bios.h"

// runtime_2aac.s — IRQ/DISPSTAT + placement helpers (summary, behavioral)
// The full 4KB span contains ~100 small placement math leaves; we provide
// the architecturally relevant ones plus stubs for the rest that keep linking.

// 0x08002AAC: enable VBlank/VCount IRQs, and GamePak IRQ only when the
// cartridge fixed byte and game-code word match. This routine does not
// write IME; its final DISPSTAT value is 0x3828, not 0x2838.
void RuntimeIrqConfig(void) {
    register volatile u16 *ie __asm__("r3") = (volatile u16 *)0x04000200u;
    *ie = 5;
    if (*(const u8 *)0x080000B2u == 150) {
        register const u32 *codep __asm__("r0") = (const u32 *)0x080000ACu;
        register const u32 *expectedp __asm__("r1") = (const u32 *)0x0802E1FCu;
        // Materialize both addresses before either word is fetched, as in
        // the ROM's ldr r0 / ldr r1 / ldr r2,[r0] / ldr r0,[r1].
        __asm__("" : : "r"(codep), "r"(expectedp));
        u32 code = *codep;
        u32 expected = *expectedp;
        if (code == expected) {
            register u32 flags __asm__("r0") = *ie;
            *ie = flags | 8192;
        }
    }
    *(volatile u16 *)0x04000004u = 0x3828;
}
#ifndef __APPLE__
void _08002AAC(void) __attribute__((alias("RuntimeIrqConfig")));
// The closure spells 0x08002AAC `sub_08002AAC`, and promoted bodies call it by
// that name. Without this the alias exists in no C TU, the asm copy is a local
// `t` symbol no separate object can resolve, and the call silently becomes a
// ROM veneer -- which the byte oracle cannot see. One hop to the real body.
void sub_08002AAC(void) __attribute__((alias("RuntimeIrqConfig")));
#endif

// hot placement helpers — substantiated from runtime_2aac.s
// _08002AF4(x) = x*4 + 0x0203F170 (IRQ table base) — verified at 0x02AF4: lsls #2 + ldr constant
u32 Place_x4(u32 x) { return x*4 + 0x0203F170; }
#ifndef __APPLE__
u32 _08002AF4(u32 x) __attribute__((alias("Place_x4")));
u32 sub_08002AF4(u32 v) __attribute__((alias("Place_x4")));
#endif
// _08002C48 exact: ldr r2,=0x03000134; ldr r2,[r2]; lsls r1#16/lsrs#16 u16 val; lsls r0#5 *32; strh r1@[r0+6]/[r0+30]; pool _08002C5C=0x03000134
void Store_02C48(int idx, u16 val){
    // preceding lsls r1,#16 / lsrs r1,#16 in asm normalizes to u16
    u16 v = val; // already Vu16 via caller; explicit mask kept for exactness: v & 0xFFFF
    volatile u32 *basePtr = (volatile u32*)(uintptr_t)0x03000134;
    volatile u8 *base = (volatile u8*)(uintptr_t)*basePtr;
    volatile u16 *p6 = (volatile u16*)(((u32)idx<<5) + (uintptr_t)base + 6);
    volatile u16 *p30 = (volatile u16*)(((u32)idx<<5) + (uintptr_t)base + 30);
    *p6 = v;
    *p30 = v;
}
#ifndef __APPLE__
void _08002C48(int a, u16 b) __attribute__((alias("Store_02C48")));
#endif
// neighboring _08002C60 exact: ldr r3,=0x03000134; ldr r3,[r3]; lsls r0#5; adds r0,r3; strh r1@[r0+6]; strh r2@[r0+30]; pool _08002C70=0x03000134
// The two halfword parameters are WORD-typed, and that is a ROM fact: the
// stores are `strh r1,[r3,#6] / strh r2,[r3,#30]` with the incoming
// registers untouched. agbcc's `assign_parms` narrows a sub-word parameter to
// Pmode in the prologue (GCC only deletes that conversion when the parameter
// is entirely unused), so `u16 v1/v2` cost two `lsls/lsrs #16` pairs -- 8
// bytes the ROM does not have, and the body scored 1/36. Widening is
// behaviour-preserving: `strh` drops the high half exactly as the ROM does,
// and every call site passes halfword values (0x08002F34 does).
void Store_02C60(int idx, u32 v1, u32 v2){
    volatile u32 *basePtr = (volatile u32*)(uintptr_t)0x03000134;
    volatile u8 *base = (volatile u8*)(uintptr_t)*basePtr;
    *(volatile u16*)(((u32)idx<<5) + (uintptr_t)base + 6) = v1;
    *(volatile u16*)(((u32)idx<<5) + (uintptr_t)base + 30) = v2;
}
#ifndef __APPLE__
void _08002C60(int a, u32 b, u32 c) __attribute__((alias("Store_02C60")));
#endif

// Substantive tail: _08002ED0 placement writer + IWRAM placement/accessor leaves
// Evidence: asm/runtime_2aac.s 0x02ED0..0x048D8, table at 0x080C4940 (8 B rows).
// _08002ED0 takes TEN args (r0-r3 + 6 caller stack words); the frame is
// push{r4-r7,lr}+push{r5-r7}+sub sp,#4 (36 B), so the loop head reads the
// caller words at [sp+36..56]. Arg map (names below):
//   a=r7 seed ([place+2] &= 0x1FF), b=ip low byte ([place] |= b&255),
//   c=r6 OR-mask ([place+4]), d (<<12 into [place+4]),
//   e (r0 of the _08002C34(e, place) call), f (<<3 table index into 0x080C4940),
//   g (loop count; 0 skips), h (<<10 into [place+4]), i (<<10 into sl for [place]),
//   j (u16 [place+12]). Per-iteration: a += (s8)[tbl+6], c += (s16)[tbl+4].
// The allocator's r0 argument is dead. Its no-argument assembler-name
// declaration below preserves the ROM call without an extra movs r0,#0.
// The table index is word-wide; the ROM does not truncate it to s16.
void EmitPlace_02ED0(void *a, int b, int c, int d, int e, int f, int g, int h, int i, int j);
#ifndef __APPLE__
void _08002ED0(void *a, int b, int c, int d, int e, int f, int g, int h, int i, int j) __attribute__((alias("EmitPlace_02ED0")));
void sub_08002ED0(void *a, int b, int c, int d, int e, int f, int g, int h, int i, int j) __attribute__((alias("EmitPlace_02ED0")));
void Sub_08002ED0(void *a, int b, int c, int d, int e, int f, int g, int h, int i, int j) __attribute__((alias("EmitPlace_02ED0")));
#endif
void EmitPlace_02ED0(void *base,int id,int z,int y,int idx,int row,int count,int bank,int shape,int clear) {
    typedef struct { u16 attr0, attr1, advance; s8 dx; u8 pad; } EmitRow;
    typedef struct { u16 attr0, attr1, attr2, pad; u32 next; u16 clear; } EmitObj;
#ifndef __APPLE__
    // Retain the assembly's interior label at the first stack-argument load.
    __asm__(".global _08002EE4\n.set _08002EE4, EmitPlace_02ED0 + 20");
#endif
    int x = (int)(uintptr_t)base;
    register int incoming __asm__("ip") = id;
    register int tile __asm__("r6") = z;
    volatile int depth = y;
    __asm__("" : : "m"(row), "m"(count), "m"(bank), "m"(shape), "m"(clear) : "memory");
    int rowindex = row;
    __asm__("" : "+r"(rowindex));
    register int remaining __asm__("r5") = count;
    register u32 shapearg __asm__("r2") = shape;
    const EmitRow *tbl = (const EmitRow *)(0x080C4940u + ((u32)rowindex << 3));
    register u32 shapebits __asm__("sl") = shapearg << 10;
    u16 advance = tbl->advance;
    if (remaining == 0) return;
    register u32 ybits __asm__("r9") = (u32)incoming & 255;
    register int step __asm__("r8") = (s16)advance;
    do {
        extern void _08002C34(int,void *);
#ifndef __APPLE__
        extern void *EmitAllocate_02ED0(void) __asm__("_08002BFC");
        EmitObj *obj = (EmitObj *)EmitAllocate_02ED0();
#else
        extern void *_08002BFC(int);
        EmitObj *obj = (EmitObj *)_08002BFC(0);
#endif
        u32 attr0 = shapebits;
        __asm__("" : "+r"(attr0));
        register u32 tableattr __asm__("r3") = tbl->attr0;
        obj->attr0 = attr0 | tableattr | ybits;
        // The ROM loads the mask in r3, copies it to r2, then masks r0.
        register u32 mask __asm__("r3") = 511;
        register u32 maskcopy __asm__("r2") = mask;
        __asm__("" : "+r"(maskcopy) : "r"(mask));
        obj->attr1 = ((u32)x & maskcopy) | tbl->attr1;
        register u32 d __asm__("r3") = depth;
        register u32 bits __asm__("r0") = (d << 12) | (u32)tile;
        __asm__("" : : "r"(d));
        register u32 bankarg __asm__("r3") = bank;
        __asm__("" : : "r"(bankarg));
        register u32 bankfield __asm__("r2") = bankarg << 10;
        bits |= bankfield;
        __asm__("" : "+r"(bits));
        obj->attr2 = bits;
#ifndef __APPLE__
        // Hardware ABI contract: the ROM reads only the low halfword of
        // the tenth caller word at sp+56 after its fixed 36-byte frame.
        // A normal cast emits a word load; &clear emits add sp,#56 instead
        // of the ROM's mov r0,sp / ldrh r0,[r0,#56]. Byte gates cover the
        // frame size and this exact access independently of the host model.
        register const struct { u8 pad[56]; u16 clear; } *frame __asm__("sp");
        __asm__("" : "=r"(frame));
        obj->clear = frame->clear;
#else
        obj->clear = (u16)clear;
#endif
        _08002C34(idx,obj);
        x += tbl->dx;
        tile += step;
    } while (--remaining != 0);
}
void RecordSetter_02F68(int x,int id,int y,int z,int idx,int row,int count);
#ifndef __APPLE__
void _08002F68(int a,int b,int c,int d,int e,int f,int g) __attribute__((alias("RecordSetter_02F68")));
void sub_08002F68(int a,int b,int c,int d,int e,int f,int g) __attribute__((alias("RecordSetter_02F68")));
#endif
void RecordSetter_02F68(int x,int id,int y,int z,int idx,int row,int count){
    // 0x02F68 (asm/runtime_2aac.s:612-703): 7-arg sprite emitter over table
    // 0x080C4940 + (row<<3). Prebuild attr0 = h0 | 0x400 | (id & 0xFF);
    // per iteration: obj = _08002BFC; attr2 = (x & 0x1FF) | h2;
    // attr4 = (z << 12) | y; attr12 = 0; _08002C34(idx, obj);
    // x += (s16)(s8)tbl[6]; y += (s16)u16[tbl+4]; count guarded at entry.
    volatile u8 *tbl=(volatile u8*)(uintptr_t)(0x080C4940u + ((u32)row<<3));
    if(count==0) return;
    u16 h0=*(volatile u16*)(tbl+0);
    u16 h2=*(volatile u16*)(tbl+2);
    u16 attr0=(u16)(h0 | 0x400u | ((u32)id & 0xFFu));
    s16 dx=(s16)*(volatile u16*)(tbl+6);   // s8 zero-extended to u16
    s16 dy=(s16)*(volatile u16*)(tbl+4);
    int rx=x;
    int ry=y;
    while(count-- != 0){
        extern void *_08002BFC(int);
        volatile u8 *place=(volatile u8*)_08002BFC(0);
        *(volatile u16*)(place+0)=attr0;
        *(volatile u16*)(place+2)=(u16)(((u32)rx & 0x1FFu) | h2);
        *(volatile u16*)(place+4)=(u16)(((u32)z<<12) | (u32)ry);
        *(volatile u16*)(place+12)=0;
        extern void _08002C34(int, void*);
        _08002C34(idx, (void*)place);
        rx+=dx;
        ry+=dy;
    }
}
void RecordSetter_03004(int base,int v1,int z,int y,int idx,int row,int a6,int a7);
#ifndef __APPLE__
void _08003004(int a,int b,int c,int d,int e,int f,int g,int h)
    __attribute__((alias("RecordSetter_03004")));
#endif
void RecordSetter_03004(int base,int v1,int z,int y,int idx,int row,int a6,int a7){
    // 0x03004: single-emit OAM record over table 0x080C4940 + (row<<3).
    //   r9 = (s16)_08002BE8(0x080C4940)   (bump counter u16[0x03000158])
    //   _08002C60(r9, (u16)a6, (u16)a7)
    //   obj = _08002BFC;
    //   if (a6 <= 255 || a7 <= 255): base -= s8[6]/2(floor); v1 -= s8[7]/2(floor);
    //     attr0 = (h0 | v1&0xFF) | 0x300
    //   else: attr0 = (h0 | v1&0xFF) | 0x100
    //   attr2 = h2 | base&0x1FF | r9<<9; attr4 = y<<12 | z; attr12 = 0
    //   _08002C34(idx, obj)
    //
    // The ROM's allocation is `mov r8, r0` / `adds r6, r1, #0` / `mov sl, r3`
    // for the three register-homed parameters, i.e. base=r8, v1=r6, y=sl, and
    // r9 holds the counter result -- all four are callee-saved here, so the
    // high-register prologue above spills them. Pinning each of them (the
    // parameter copy is what the pin is honoured for) reproduces that; the
    // table pointer lands in r7 and the allocator result in r2 for free.
    //
    // Entry order (reachable? no): the ROM interleaves the parameter copies as
    // `mov r8,r0 / adds r6,r1,#0 / str r2,[sp,#0] / mov sl,r3 / ldr r1,[sp,#40]`
    // while this body gets `str r2,[sp,#0] / ldr r2,[sp,#40] / mov r8,r0 / ...`
    // -- i.e. agbcc reuses r2 for the row load as soon as z is spilled, where
    // the ROM keeps r2 live and loads row into r1. One basic block, no
    // scheduler, so this is allocation pressure, not statement order.
    // h0=r5 and h2=r4 are the ROM's own allocation too, and both attribute
    // words update IN PLACE (`orrs r5,r6` / `orrs r4,r3`), so h0|v1, h0|mode,
    // h2|mask and h2|r9<<9 are each spelled as their own statement; a single
    // combined expression makes agbcc compute into a temp and copy back.
    //
    // PARKED at 122/200 (200-byte candidate, first diff +0xC). The mode bits
    // are now right: ORing a *constant literal* into the attribute inside each
    // arm (`h0 = (u16)(h0 | 0x300u);`) makes agbcc materialise the constant in
    // a scratch pseudo and copy it into the OR's operand register --
    // `movs r1,#192 / lsls r1,r1,#2 / add r0,r1,#0 / orr r5,r5,r0` -- which is
    // the ROM's sequence. agbcc puts the copy ONCE, at the join, while the ROM
    // has one per arm with a different scratch (r1 then-arm, r3 else-arm), i.e.
    // the two arms' constant pseudos got different registers there. The same
    // lever fixes the 0x1FF mask in a minimal function (single statement,
    // literal mask: `ldr r1,=511 / add r0,r1,#0 / and r2,r2,r0 / orr r4,r4,r2`
    // with base in a low register), but not in this body: here the AND comes
    // out `ldr r0,=511 / mov r1,r8 / and r0,r1`, i.e. the constant is the AND
    // destination (op0=op1) instead of the copied base.
    //
    // What is left is a register-choice family, not a spelling family. The ROM
    // uses r3 as its scratch throughout the tail (base copy at 0x90, sl copy
    // at 0xA0, z load at 0xA4, else-arm mode const at 0x82) where agbcc picks
    // r1/r4, and it uses r1 where agbcc picks r0 (mask const at 0x8C, attr12
    // zero at 0x9C). r3 is never allocated at all in the candidate, which
    // suggests r0/r1 are live differently: the ROM's choices look like
    // "everything shifted one register up" for the r0/r1 pairs. Also still
    // open: the then-arm copy placement, the `adds r0,r1,#0` missing there,
    // the attr12 zero (`movs r1,#0` materialised a statement early and reused,
    // agbcc emits `movs r0,#0` at the store), the rev' argument loads, and the
    // entry reorder (z spill and the row load land above the three parameter
    // copies where the ROM has copies first). Non-exact by construction; keep
    // the improved body for whoever revisits it.
#ifndef __APPLE__
    register int rbase __asm__("r8");
    register int rv1 __asm__("r6");
    register int ry __asm__("sl");
    register int r9 __asm__("r9");
    volatile u8 *tbl;
    u16 orval;
    register u16 h0 __asm__("r5");
    register u16 h2 __asm__("r4");
    register volatile u8 *obj __asm__("r2");
#else
    int rbase, rv1, ry, r9;
    volatile u8 *tbl;
    u16 h0, h2;
    u16 orval;
    volatile u8 *obj;
#endif
    rbase = base;
    rv1 = v1;
    ry = y;
    tbl = (volatile u8*)(uintptr_t)(0x080C4940u + ((u32)row<<3));
    {
        extern int _08002BE8(int);
        r9 = (s16)_08002BE8((int)(uintptr_t)0x080C4940);
    }
    h0 = *(volatile u16*)(tbl+0);
    h2 = *(volatile u16*)(tbl+2);

    {
        extern void _08002C60(int, u32, u32);
        _08002C60(r9, (u16)a6, (u16)a7);
    }
#ifndef __APPLE__
    // The allocator takes no argument in the ROM: a no-parameter
    // declaration keeps `bl _08002BFC` free of an argument setup.
    {
        extern void *EmitAlloc_03004(void) __asm__("_08002BFC");
        obj = (volatile u8*)EmitAlloc_03004();
    }
#else
    {
        extern void *_08002BFC(int);
        obj = (volatile u8*)_08002BFC(0);
    }
#endif
    // Non-volatile s8 loads: a volatile signed byte lvalue lowers to
    // `ldrb` plus an explicit sign extension, while the ROM's plain
    // `ldrsb` is what a non-volatile signed byte read emits. The halving is
    // C `/2` (truncating): the ROM emits the `lsrs #31; adds; asrs #1`
    // idiom, not `asrs #31; adds; asrs #1` for a hand-written floor shift.
    if (a6 <= 255 || a7 <= 255) {
        int b6=*(s8*)(tbl+6);
        rbase-=b6/2;
        int b7=*(s8*)(tbl+7);
        rv1-=b7/2;
        rv1&=0xFF;
        h0=(u16)(h0 | (u16)rv1);
        h0=(u16)(h0 | 0x300u);
    } else {
        rv1&=0xFF;
        h0=(u16)(h0 | (u16)rv1);
        h0=(u16)(h0 | 0x100u);
    }
    *(volatile u16*)(obj+0)=h0;
    // A low-register copy of the base (`register u32 rb2 __asm__("r3");
    // rb2 = rbase; h2 |= rb2 & 0x1FFu;`) does NOT fix this: rb2's single use is
    // copy-propagated back to rbase, the pinned copy disappears, and the AND
    // stays `ldr r0,=511 / mov r1,r8 / and r0,r1`.
    h2=(u16)(h2 | (u16)(rbase & 0x1FF));
    h2=(u16)(h2 | (u16)(r9<<9));
    *(volatile u16*)(obj+2)=h2;
    *(volatile u16*)(obj+4)=(u16)(((u32)ry<<12) | (u32)z);
    *(volatile u16*)(obj+12)=0;
    {
        extern void _08002C34(int, void*);
        _08002C34(idx, (void*)obj);
    }
}
// 0x080030CC: inits u16 buf[4] = {160,0,0,0} on stack (mov r2,sp first),
// then 32 CpuSet calls _08002D974(buf, key, 0x04000002) with key = r5 =
// (224<<19) + i*8 (0x1C00000 base) and r4 = 31 counting down to 0 (bge).
void Helper_030CC(void){
    u16 buf[4];
#ifndef __APPLE__
    register u32 key __asm__("r5");
    register int n __asm__("r4");
#else
    u32 key;
    int n;
#endif
    extern void sub_0802D974(const void *a, void *b, u32 c);
    buf[0] = 160;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0;
    key = (u32)224 << 19;
    n = 31;
    do {
        sub_0802D974(buf, (void *)key, 0x04000002u);
        key += 8;
        n--;
    } while (n >= 0);
}
#ifndef __APPLE__
void _080030CC(void) __attribute__((alias("Helper_030CC")));
#endif

// _08002B00 residual (B of 28; NOT alignment-only — the span ends
void Store_02B00(u32 a,u32 b){ *(vu32*)0x03000134=a; __asm__("" ::: "r2"); *(vu32*)0x0300013C=b; *(vu32*)0x03000138=a; }
#ifndef __APPLE__
void _08002B00(u32 a,u32 b) __attribute__((alias("Store_02B00")));
#endif
void Store_02B1C(u32 a,u32 b){ *(vu32*)0x03000140=a; *(vu32*)0x03000144=b; }
#ifndef __APPLE__
void _08002B1C(u32 a,u32 b) __attribute__((alias("Store_02B1C")));
#endif
void Store_02B30(u32 a,u32 b){ *(vu32*)0x03000148=a; *(vu32*)0x0300014C=b; }
#ifndef __APPLE__
void _08002B30(u32 a,u32 b) __attribute__((alias("Store_02B30")));
#endif
void Store_02B44(void){ *(vu16*)0x03000158=0; }
#ifndef __APPLE__
void _08002B44(void) __attribute__((alias("Store_02B44")));
// `sub_08002B44` is declared further down with `Sub_08002B44`, beside the other
// two spellings of this body.
#endif
// _08002BB4 residual (B of 36; prefix 9, candidate 32 B). The
void Store_02BB4(void){
    *(vu32*)0x03000154 = *(vu32*)0x03000148;
#ifndef __APPLE__
    __asm__ volatile("" ::: "r1");
#endif
    *(vu32*)0x03000150 = *(vu32*)0x0300014C;
}
#ifndef __APPLE__
void _08002BB4(void) __attribute__((alias("Store_02BB4")));
#endif
int LoadSub_02BD8(void){
    const s16 *p = (const s16 *)0x03000158;
    return 32 - *p;
}
#ifndef __APPLE__
int _08002BD8(void) __attribute__((alias("LoadSub_02BD8")));
#endif
int Inc_02BE8(int dummy){
    volatile u16 *p = (volatile u16 *)0x03000158;
    u16 cur = *p;
    *p = cur + 1;
    return (s16)cur;
}
#ifndef __APPLE__
int _08002BE8(int) __attribute__((alias("Inc_02BE8")));
#endif

void Insert_02C34(int idx, void *node){
    // _08002C34: ldr r2,=0x03000140; ldr r2,[r2]; lsls r0,r0,#2; adds r2,r2,r0; ldr r0,[r2]; str r0,[r1,#8]; str r1,[r2]; bx lr
    // pool _08002C44.4byte 0x03000140 at original offset; widths Vu32 ldr/str, *4 table, +8 Vu32 store
    u32 base = *(volatile u32*)(uintptr_t)0x03000140;
    volatile u32 *slot = (volatile u32*)((uintptr_t)base + ((u32)idx<<2));
    u32 old = *slot;
    *(volatile u32*)((u8*)node + 8) = old;
    *slot = (u32)(uintptr_t)node;
}
#ifndef __APPLE__
void _08002C34(int a, void *b) __attribute__((alias("Insert_02C34")));
#endif

// 0x08002DB8: emit count objects from one 8-byte ROM row. The returned
// objects live in the RAM shadow pool, not hardware OAM. The ROM loads the
// advance halfword even when count is zero, then sign-extends it for the loop.
void Helper_02DB8(void *base,int id,int z,int y,int idx,int row,int count) {
    typedef struct { u16 attr0, attr1, advance; s8 dx; u8 pad; } EmitRow;
    typedef struct { u16 attr0, attr1, attr2, pad; u32 next; u16 clear; } EmitObj;
    int x = (int)(uintptr_t)base;
    register int incoming __asm__("ip") = id;
    register int tile __asm__("r6") = z;
    register int depth __asm__("sl") = y;
    // Preserve the incoming-register copies before reading the stack row,
    // and keep count in r5 so the table calculation uses r0/r1, not r4/r0.
    int rowindex = row;
    __asm__("" : "+r"(rowindex));
    register int remaining __asm__("r5") = count;
    const EmitRow *tbl = (const EmitRow *)(0x080C4940u + ((u32)rowindex << 3));
    u16 advance = tbl->advance;
    if (remaining == 0) return;
    register u32 ybits __asm__("r9") = (u32)incoming & 255;
    register int step __asm__("r8") = (s16)advance;
    do {
        // The allocator overwrites r0 without reading it; the ROM call has
        // no argument materialization. Its legacy int parameter is unused.
        extern void _08002C34(int,void *);
#ifndef __APPLE__
        extern void *EmitAllocate_02DB8(void) __asm__("_08002BFC");
        EmitObj *obj = (EmitObj *)EmitAllocate_02DB8();
#else
        extern void *_08002BFC(int);
        EmitObj *obj = (EmitObj *)_08002BFC(0);
#endif
        u32 attr0 = ybits;
        __asm__("" : "+r"(attr0));
        obj->attr0 = attr0 | tbl->attr0;
        obj->attr1 = ((u32)x & 511) | tbl->attr1;
        // Shift depth inside the loop, retaining the unshifted sl value.
        __asm__("" : "+r"(depth));
        obj->attr2 = ((u32)depth << 12) | (u32)tile;
        obj->clear = 0;
        _08002C34(idx,obj);
        x += tbl->dx;
        tile += step;
    } while (--remaining != 0);
}
#ifndef __APPLE__
void _08002DB8(void *a,int b,int c,int d,int e,int f,int g)
    __attribute__((alias("Helper_02DB8")));
void sub_08002DB8(void *a,int b,int c,int d,int e,int f,int g)
    __attribute__((alias("Helper_02DB8")));
#endif
// 0x08002E3C: the eight-argument variant adds bank bits to attr2. The ROM
// normalizes the eighth stack argument to u16 and retains it in sl; depth
// occupies the single stack local and is loaded anew for each object.
void Helper_02E3C(void *base,int id,int z,int y,int idx,int row,int count,u32 bank) {
    typedef struct { u16 attr0, attr1, advance; s8 dx; u8 pad; } EmitRow;
    typedef struct { u16 attr0, attr1, attr2, pad; u32 next; u16 clear; } EmitObj;
    int x = (int)(uintptr_t)base;
    register int incoming __asm__("ip") = id;
    register int tile __asm__("r6") = z;
    volatile int depth = y;
    // Force the incoming copies and depth spill before the three stack
    // loads, without emitting instructions or modifying the arguments.
    __asm__("" : : "m"(row), "m"(count), "m"(bank) : "memory");
    int rowindex = row;
    __asm__("" : "+r"(rowindex));
    register int remaining __asm__("r5") = count;
    register u32 bankarg __asm__("r1") = bank;
    register u32 bankbits __asm__("sl") = (u16)bankarg;
    const EmitRow *tbl = (const EmitRow *)(0x080C4940u + ((u32)rowindex << 3));
    u16 advance = tbl->advance;
    if (remaining == 0) return;
    register u32 ybits __asm__("r9") = (u32)incoming & 255;
    register int step __asm__("r8") = (s16)advance;
    do {
        extern void _08002C34(int,void *);
#ifndef __APPLE__
        extern void *EmitAllocate_02E3C(void) __asm__("_08002BFC");
        EmitObj *obj = (EmitObj *)EmitAllocate_02E3C();
#else
        extern void *_08002BFC(int);
        EmitObj *obj = (EmitObj *)_08002BFC(0);
#endif
        u32 attr0 = ybits;
        __asm__("" : "+r"(attr0));
        obj->attr0 = attr0 | tbl->attr0;
        obj->attr1 = ((u32)x & 511) | tbl->attr1;
        __asm__("" : "+r"(bankbits));
        // The ROM loads depth through r3, shifts into r0, and reuses r3
        // for sl. Keep the final OR/store in r0 rather than destroying r2.
        register u32 d __asm__("r3") = depth;
        register u32 bits __asm__("r0") = (d << 12) | (u32)tile;
        __asm__("" : : "r"(d));
        bits |= bankbits << 10;
        __asm__("" : "+r"(bits));
        obj->attr2 = bits;
        obj->clear = 0;
        _08002C34(idx,obj);
        x += tbl->dx;
        tile += step;
    } while (--remaining != 0);
}
#ifndef __APPLE__
void _08002E3C(void *a,int b,int c,int d,int e,int f,int g,u32 h)
    __attribute__((alias("Helper_02E3C")));
void sub_08002E3C(void *a,int b,int c,int d,int e,int f,int g,u32 h)
    __attribute__((alias("Helper_02E3C")));
#endif

// span-tail init — _080048D8 is scene-tail reset (writes 0x03000198+? per findings §6)
// Implemented as heap cursor reset via 0x03000198+0x88/0x90 alias to _08004A0C
void RuntimeTailInit(void){ extern void HeapReset(void); HeapReset(); }
#ifndef __APPLE__
void _080048D8(void) __attribute__((alias("RuntimeTailInit")));
#endif

// ---- Mechanical translation of remaining bounded runtime leaves (opaque IWRAM) ----
// Each uses exact VMA, pool at original.4byte offset, and volatile u8/u16/s16/u32 widths.
// No manager struct names — opaque *(volatile uX*)(uintptr_t)ADDR+off. Helper ABIs as extern.

// _08002B50: HeapFill — two CpuSet fills + 0x1FF-capped strh loop.
// The first fill's source is the stack zero word itself (`mov r0,sp;
// bl` with `[sp] = 0`), its ctrl is (v144 & 0x1FFFFF) | 160<<19 -- not a
// hardcoded ctrl with &masked as source. The second fill's source is the
// second stack word (`add r0,sp,#4`), dst is reloaded per... no: dst is
// *0x03000134 once, ctrl is pool 0x05000100. The paint loop reloads its
// base from the 0x03000134 cell EVERY iteration (`ldr r1,[r4]` first),
// compares a pool-loaded 0x1FF bound, and stores via base+(i<<1).
void Wrap_02B50(void){
    extern void sub_0802D974(const void*,void*,u32);
    u32 z0 = 0;
    u32 v140 = *(volatile u32*)(uintptr_t)0x03000140u;
    // The ROM pool-loads 0x03000144 separately; an empty `+r` barrier does
    // NOT stop agbcc folding it to `adds r0,#4` here (verified by direct
    // probe: same-register allocation makes the barrier transparent), so
    // the barrier clobbers r0 instead -- with r0's value unknown the
    // compiler cannot derive the address and must pool-load it.
    __asm__("" : : "r"(v140) : "r0");
    u32 v144 = *(volatile u32*)(uintptr_t)0x03000144u;
    u32 masked = v144 & 0x001FFFFFu;
    masked |= (160u << 19);
    sub_0802D974(&z0, (void*)v140, masked);
    u32 z1 = 0;
    // The src pointer materializes before the cell pool-load (ROM `add
    // r0,sp,#4` precedes `ldr r4,=0x03000134`), so it is named here.
    u32 *src1 = &z1;
    volatile u32 *cell = (volatile u32*)(uintptr_t)0x03000134u;
    sub_0802D974(src1, (void*)*cell, 0x05000100u);
    int i = 0;
    u32 bd = 0x1FFu;
    u16 v = 160;
    for (; i <= (int)bd; i += 4) {
        // b is pinned to r1 (ROM `ldr r1,[r4]` first, index into r0);
        // unpinned it takes r0 and the pair swaps. The address sum is
        // written index-first so the result lands in r0 (`adds r0,r0,r1`,
        // not `adds r1,r1,r0`).
        register u32 b __asm__("r1") = *cell;
        *(u16*)((i << 1) + b) = v;
    }
}
#ifndef __APPLE__
void _08002B50(void) __attribute__((alias("Wrap_02B50")));
#endif

// _08002C74 exact: ldr r2,=0x03000134; ldr r2,[r2]; lsls r0,#5; adds r0,r2; strh r1,[r0,#22]; pool _08002C80=0x03000134
void Store_02C74(int idx, int val){
    volatile u32 *basePtr = (volatile u32*)(uintptr_t)0x03000134;
    volatile u8 *base = (volatile u8*)(uintptr_t)*basePtr;
    volatile u16 *p = (volatile u16*)(((u32)idx << 5) + (uintptr_t)base + 22);
    *p = (u16)val;
}
#ifndef __APPLE__
void _08002C74(int a, int b) __attribute__((alias("Store_02C74")));
void sub_08002C74(int a, int b) __attribute__((alias("Store_02C74")));
#endif

// _08002C84 EXACT : 20/20, prefix 20, candidate 20 B. The
// semantics were already right — `ldr r1,=0x03000134/ldr r1,[r1]/lsls r0,#5/
// adds r0,r1` then negate the s16 at `[base + idx*32 + 6]` and store it back.
// The only difference could not remove was one register choice: ROM
// `ldrh r2,[r0,#6] / negs r1,r2 / strh r1,[r0,#6]`, candidate `ldrh r1,[r0,#6]
// / negs r1,r1 / strh r1,[r0,#6]`. diagnosed the cause correctly
// (gcc-2.95 local-alloc's QTY_CMP_PRI is 0 for both operands at n_refs==1, so
// the tie falls to the quantity number and the earlier-created LOAD always
// wins) but had no lever: making the negation win the tie needs it referenced
// twice, and every "reference it twice" shape grows the body.
// The lever is the hard-register pin, which reaches the same decision
// directly. `register s16 v __asm__("r2")` forces the raw load into r2 —
// forced by the ROM's own `ldrh r2,[r0,#6]` at 0x08002C8C, the only
// instruction that names r2 there; with r2 taken, r1 (free once `base` dies at
// `adds r0,r0,r1`) becomes the only register left for the negation, which is
// the ROM's `negs r1,r2` at 0x08002C8E. Both pins are needed and neither
// suffices alone — measured: both 20/20 prefix 20; r2 pin only 18/20 prefix 10
// (load correct, `negs r2,r2`); r1 pin only 18/20 prefix 8, identical to the
// unpinned body. The two must also be SEPARATE declaration statements: two
// `register... __asm__` in one block makes the C89 transform bail with
// "budget exhausted" and the probe then skips the whole TU.
// `register T v __asm__("rN")` is a GNU extension, so this unit is not strict
// C89; the C89 transform still processes it and the probe compiles it.
// _08002C84 exact: ldr r1,=0x03000134; ldr r1,[r1]; lsls r0,#5; adds r0,r1; ldrh r2,[r0,#6]; negs r1,r2; strh r1,[r0,#6]; pool _08002C94=0x03000134
void Wrap_02C84(int idx){
    volatile u32 *basePtr = (volatile u32*)(uintptr_t)0x03000134;
    volatile u8 *base = (volatile u8*)(uintptr_t)*basePtr;
    volatile u16 *p = (volatile u16*)(((u32)idx<<5) + (uintptr_t)base + 6);
    register s16 v __asm__("r2") = *(volatile s16*)p;
    register s16 n __asm__("r1") = -v;
    *(volatile u16*)p = (u16)n;
}
#ifndef __APPLE__
void _08002C84(int a) __attribute__((alias("Wrap_02C84")));
#endif

// _08003130 exact: ldr r0,=0x03000134; ldr r0,[r0]; bx lr; pool _08003138=0x03000134 Vu32
u32 Get_03130(void){
    return *(volatile u32*)(uintptr_t)0x03000134;
}
#ifndef __APPLE__
u32 _08003130(void) __attribute__((alias("Get_03130")));
#endif

// _0800313C exact: ldr r1,=0x03000138; str r0,[r1]; bx lr; pool _08003144=0x03000138 Vu32 (typed entry despite objdump order)
void Store_0313C(u32 val){
    *(volatile u32*)(uintptr_t)0x03000138 = val;
}
#ifndef __APPLE__
void _0800313C(u32 a) __attribute__((alias("Store_0313C")));
#endif

// _08002D3C: block copy 0x03000140->0x03000138 then OAM fill via _0802D970.
// ROM (asm/runtime_2aac.s:329-378): push {r4-r6,lr}; tbl=*0x03000140 (r4,
// ldmia r4!,{r1}); dst=*0x03000138 (r2, +=8); idx r3=0; cntPtr=0x03000144
// (r5, count reloaded each outer iter); oamCell=0x03000134 (r6, src for
// final CpuFastSet to 0x07000000 len 256). Outer: if (idx<count) do {
// entry=*tbl++; idx++; if (entry) do { *(u32*)dst=*entry;
// *(u16*)(dst+4)=*(u16*)(entry+4); dst+=8; entry=*(entry+8); } while(entry);
// } while(idx<count). Tail: CpuFastSet(*oamCell, 224<<19, 128<<1).
void Wrap_02D3C(void){
    // Interleaved pool-load/deref order to match the ROM's
    // ldr r0,=140 / ldr r4,[r0] / ldr r0,=138 / ldr r2,[r0] /
    // movs r3,#0 / ldr r1,=144 / ldr r0,[r1] / ldr r6,=134 sequence.
    // Grouping pool loads lets agbcc keep the table in r5 and the
    // destination in r1 instead of r4/r2, and declaring idx after the
    // 134-cell pin groups both remaining pools before the movs.
    volatile u32 *p140 = (volatile u32 *)(uintptr_t)0x03000140u;
    u32 *tbl = (u32 *)(uintptr_t)*p140;
    // Prevent folding 0x03000138 to `subs r0,#8` from the 0x03000140 pool
    // address above (candidate ldmia path then loses a pool and both data
    // registers swap). Same r0-clobber lever as Wrap_02B50: with r0's value
    // unknown the compiler must pool-load the next IWRAM cell address.
    __asm__("" : : "r"(tbl) : "r0");
    volatile u32 *p138 = (volatile u32 *)(uintptr_t)0x03000138u;
    u8 *dst = (u8 *)(uintptr_t)*p138;
    int idx = 0;
    volatile u32 *cntPtr = (volatile u32 *)(uintptr_t)0x03000144u;
    // Load the count value for the entry check first (ROM ldr r0,[r1];
    // cmp r3,r0; bge skip), then save the pointer itself for the
    // per-iteration reloads (ROM adds r5,r1 after the check). Spelling the
    // entry check directly as `idx < *cntPtr` lets agbcc keep the pointer
    // in r4 from the start and emit a bare `b' to the bottom test instead.
    // idx/cnt are SIGNED: the ROM's entry/exit tests are bge/blt, not
    // bcs/bcc, so u32 here costs exactly those two branch opcodes.
    int cnt = (int)*cntPtr;
    // Pin the OAM cell pointer to r6: the ROM pool-loads 0x03000134 after
    // the count deref and keeps it in r6 for the tail CpuFastSet, which is
    // what forces push {r4,r5,r6,lr}. Declared earlier its pool groups
    // before the movs above.
    register volatile u32 *oamCell __asm__("r6") = (volatile u32 *)(uintptr_t)0x03000134u;
    extern void _0802D970(const void *, void *, u32);
    if (idx < cnt) {
        volatile int *cntSaved = (volatile int *)(uintptr_t)0x03000144u;
        do {
            u8 *entry = *(u8 **)tbl;
            tbl++;
            idx++;
            if (entry != (u8 *)0) {
                do {
                    *(volatile u32 *)dst = *(volatile u32 *)entry;
                    *(volatile u16 *)(dst + 4) = *(volatile u16 *)(entry + 4);
                    dst += 8;
                    entry = *(u8 **)(entry + 8);
                } while (entry != (u8 *)0);
            }
        } while (idx < *cntSaved);
    }
    // Load the OAM source before synthesising the immediates: the ROM does
    // ldr r0,[r6] / movs r1,#224 / lsls / movs r2,#128 / lsls, while passing
    // *oamCell directly lets agbcc emit the two movs/lsls first.
    u32 oamSrc = *oamCell;
    _0802D970((const void *)(uintptr_t)oamSrc,
              (void *)(uintptr_t)((u32)224 << 19), (u32)((u32)128 << 1));
}
#ifndef __APPLE__
void _08002D3C(void) __attribute__((alias("Wrap_02D3C")));
// Both entry spellings are branch/entry labels in the reference span.
void sub_08002D3C(void) __attribute__((alias("Wrap_02D3C")));
#endif

// _08002D98: 4× strh at 0x03000134+r0+6/14/22/30 with *32 stride
void Wrap_02D98(u32 r0, u32 r1, u32 r2, u32 r3, u32 r5){
    volatile u32 *basePtr = (volatile u32*)(uintptr_t)0x03000134;
    volatile u8 *base = (volatile u8*)(uintptr_t)*basePtr;
    // The sum must ACCUMULATE into the pinned register, not be formed beside
    // it. `(u32)base + (r0 << 5)` in one initialiser emits
    // `adds r0, r4, r0` (base as the addend); the ROM's `adds r0, r0, r4`
    // wants the shifted index in r0 first and the base added into it. Two
    // statements do exactly that and close the body: 32/32 EXACT, measured
    //. This is the resolution of the note below that recorded the
    // operand order as an open instruction-order wall -- it was never the
    // scale/load order, only the add's operand order, and `+=` is the lever.
    register u32 addr __asm__("r0") = (r0 << 5);
    addr += (u32)(uintptr_t)base;
    *(volatile u16 *)(addr + 6)  = (u16)r1;
    *(volatile u16 *)(addr + 14) = (u16)r2;
    *(volatile u16 *)(addr + 22) = (u16)r3;
    *(volatile u16 *)(addr + 30) = (u16)r5;
}
#ifndef __APPLE__
void _08002D98(u32 a,u32 b,u32 c,u32 d,u32 e) __attribute__((alias("Wrap_02D98")));
#endif

void Wrap_03248(int a,int b,int c){
    extern void _08007538(void*,int,void*); extern int _08007614(void*,int,int,void*);
    void *tbl = (void*)(uintptr_t)0x087ACF70;
    _08007538(tbl,0,(void*)(uintptr_t)a);
    _08007614(tbl,3,0,(void*)(uintptr_t)b);
    (void)c;
}
#ifndef __APPLE__
void _08003248(int a,int b,int c) __attribute__((alias("Wrap_03248")));
void sub_08003248(int a,int b,int c) __attribute__((alias("Wrap_03248")));
#endif

// _08003270: +1 variant (asm/runtime_2aac.s:1026-1032: r2 = incoming r0)
void Wrap_03270(int a,int b,int c){
    extern void _08007538(void*,int,void*); extern int _08007614(void*,int,int,void*);
    void *tbl=(void*)(uintptr_t)0x087ACF70;
    _08007538(tbl,1,(void*)(uintptr_t)a);
    _08007614(tbl,4,0,(void*)(uintptr_t)b);
    (void)c;
}
#ifndef __APPLE__
void _08003270(int a,int b,int c) __attribute__((alias("Wrap_03270")));
void sub_08003270(int a,int b,int c) __attribute__((alias("Wrap_03270")));
#endif

// _08003298: +2 variant (asm/runtime_2aac.s:1046-1052: r2 = incoming r0)
void Wrap_03298(int a,int b,int c){
    extern void _08007538(void*,int,void*); extern int _08007614(void*,int,int,void*);
    void *tbl=(void*)(uintptr_t)0x087ACF70;
    _08007538(tbl,2,(void*)(uintptr_t)a);
    _08007614(tbl,5,0,(void*)(uintptr_t)b);
    (void)c;
}
#ifndef __APPLE__
void _08003298(int a,int b,int c) __attribute__((alias("Wrap_03298")));
#endif

// _080032E6: dispatch over r5 0..3 via _080032E0/03326/03330/0333A wrappers
void Wrap_032E6(int id, void *buf){
    volatile u32 *p134 = (volatile u32*)(uintptr_t)0x03000160;
    u32 base = *p134;
    for(int i=0;i<=3;i++){
        u8 v = *(volatile u8*)((u8*)buf + i*4);
        if(v==0) continue;
        // indirect via 0x080C4BB0 table not proven — opaque
        (void)v; (void)base; (void)id;
        if(i==0) Wrap_03248(0,0,0);
        else if(i==2) Wrap_03270(0,0,0);
        else if(i==3) Wrap_03298(0,0,0);
    }
}
#ifndef __APPLE__
void _080032E6(int a,void *b) __attribute__((alias("Wrap_032E6")));
#endif

// _08003604: counted string scan then 6-arg emit via _08003350.
// ROM (asm/runtime_2aac.s:1140-1185): push {r4-r7,lr}+sub sp,#8; save
// a/b/c in r5/r6/r7; r4 = *(int*)(0x080C4BA0 + a*4)/2 signed (ldr /
// lsrs #31 / adds / asrs #1, i.e. C `/2` not `>>1`); scan d (r3) with
// r2=d, r1=120: if (*d) do { d++; r1-=r4; } while(*d); then stack
// [sp]=d (original), [sp+4]=1 and bl _08003350(a, r1, b, c, d, 1).
void Wrap_03604(int a,int b,int c,void *d){
    volatile u8 *orig = (volatile u8 *)d;
    // The ROM loads the table base before shifting the index
    // (`ldr r1,=0x080C4BA0` / `lsls r0,r5,#2` / `adds r0,r0,r1`). A plain
    // `base + (a<<2)` lets agbcc scale the index first, which loses the
    // pool load's position and both registers' roles. Two levers close it:
    // the `r1` pin plus `+r` barrier materialize the constant into r1 up
    // front, and the `r0`-pinned `addr` with `addr += base` accumulates the
    // add into r0 (`adds r0,r0,r1`) instead of r1.
    register u32 base __asm__("r1") = 0x080C4BA0u;
    __asm__("" : "+r"(base));
    register u32 addr __asm__("r0") = (u32)a << 2;
    addr += base;
    int v = *(volatile int *)(uintptr_t)addr;
    int step = v / 2;
    volatile u8 *p = orig;
    int x = 120;
    extern void _08003350(int, int, int, int, const void *, int);
    if (*p != 0) {
        do {
            p++;
            x -= step;
        } while (*p != 0);
    }
    _08003350(a, x, b, c, (const void *)orig, 1);
}
#ifndef __APPLE__
void _08003604(int a,int b,int c,void *d) __attribute__((alias("Wrap_03604")));
// runtime_2aac.s branches here by the `sub_08003604` spelling; without this
// alias the independent link would resolve that call with a ROM veneer.
void sub_08003604(int a,int b,int c,void *d) __attribute__((alias("Wrap_03604")));
void Sub_08002B50(void) __attribute__((alias("Wrap_02B50")));
void sub_08002B50(void) __attribute__((alias("Wrap_02B50")));
void Sub_08002B44(void) __attribute__((alias("Store_02B44")));
void sub_08002B44(void) __attribute__((alias("Store_02B44")));
void Sub_08002BB4(void) __attribute__((alias("Store_02BB4")));
void sub_08002BB4(void) __attribute__((alias("Store_02BB4")));
int sub_08002BD8(void) __attribute__((alias("LoadSub_02BD8")));
int sub_08002BE8(int) __attribute__((alias("Inc_02BE8")));
#endif
