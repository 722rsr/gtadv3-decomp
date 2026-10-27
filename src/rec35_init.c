#include "gba/types.h"
#include "gtadv/memory.h"

// Reference: asm/rec35_init.s 0x08015CB4-0x08015E28
// Leaf _08015CB4 with templates 0x082FCF04/0x082B7410 and course helpers 0x08007770 etc.
// Evidence pools: 0x082FCF04,0x082B7410,0x082A798C, WA offsets 88/100/142 etc. Preserve widths.

extern u32 Course_0x08007770(int ctx, void *X, int r2in, int r3in, u32 s0, u32 s1);
extern void Course_0x0800DAB8(void *p);
// Peer spellings are the manifest's (`tools/matching_slice_functions.json`): the
// independent code slice splices each promoted body into its asm hole and only
// defines the `c_name`/`export` spellings, so a call must use one of those.
extern void _08007614(void *a,int b,int c,int d);
extern void sub_0800798C(void *a,void *b);
extern void sub_08007A58(void *p);
extern void _080075E8(void *a,int b,int c);
extern void _080025BC8(void *p,int v);
extern void _08007ABC(void *a,void *b,void *c);
extern void _0800D77C(void *a,int b,int c);
extern void sub_0802B234(void);
extern void *sub_08004B68(void);
extern void sub_0800279C(int v);

// Straight-line scene initializer. The body is a call/ store sequence around one
// 3-way dispatch on the scene cell (36/37/default) and a second 3-way dispatch
// on the mode halfword it just wrote at +142. Both dispatches are `switch`es:
// the ROM's `cmp #36 / beq`, `cmp #37 / beq`, `b` with the default arm laid out
// FIRST and the two case bodies after it, in source order, is agbcc's switch
// shape (a plain if/else chain emits `bne` and strands the second compare).
// The three template pointers are plain literals, so each pools itself.
void Rec35_Init(void *ctx_) {
    u8 *ctx = (u8 *)ctx_;
    s16 *flag;
    int w;
    s16 v;

    sub_0802B234();
    Course_0x08007770(0, (void *)0x082FCF04, 1, 0, 4, 2);
    Course_0x08007770(1, (void *)0x082B7410, 1, 0, 0, 3);
    *(u32 *)(ctx + 88) = 6;
    v = *(s16 *)((u8 *)sub_08004B68() + 0);
    // Each arm materialises +142 on its own: the ROM keeps the address in a
    // scratch and copies it into the surviving home after the arm's store, so
    // the address cannot be hoisted ahead of the dispatch.
    switch (v) {
    default:
        flag = (s16 *)(ctx + 142);
        break;
    case 36:
        *(u32 *)(ctx + 100) = 14;
        *(u16 *)(ctx + 142) = 0;
        flag = (s16 *)(ctx + 142);
        break;
    case 37:
        *(u32 *)(ctx + 100) = 15;
        *(u16 *)(ctx + 142) = 1;
        flag = (s16 *)(ctx + 142);
        break;
    }
    *(u32 *)(ctx + 112) = 1;
    Course_0x0800DAB8(ctx + 16);
    _08007614((void *)0x082A798C, 1, 0, 4);
    _08007614((void *)0x082A798C, 1, 1, 5);
    sub_0800798C((void *)0x082FCF04, ctx);
    sub_08007A58(ctx);
    _080075E8((void *)0x082FCF04, 0, 3);
    sub_0800798C((void *)0x082B7410, ctx + 8);
    _080075E8((void *)0x082B7410, 0, 6);
    _080025BC8(ctx + 160, 0);
    _08007ABC(*(void **)(ctx + 12), *(void **)(ctx + 164),
                     *(void **)(ctx + 160));
    *(u32 *)(ctx + 128) = (u32)(ctx + 152);
    *(u32 *)(ctx + 132) = (u32)(ctx + 16);
    *(u32 *)(ctx + 16) = 0;
    *(u16 *)(ctx + 20) = 1;
    _0800D77C(ctx + 36, 0, -32);
    _0800D77C(ctx + 28, 0, 160);
    *(u16 *)(ctx + 24) = 6;
    *(u16 *)(ctx + 26) = 5;
    *(u32 *)(ctx + 152) = 2;
    *(u16 *)(ctx + 144) = 0;
    *(u16 *)(ctx + 148) = 0;
    *(u16 *)(ctx + 150) = 0;
    w = *flag;
    switch (w) {
    default:
        break;
    case 0:
        *(u16 *)(ctx + 140) = w;
        sub_0800279C(0);
        break;
    case 1:
        *(u16 *)(ctx + 140) = w;
        break;
    }
}

__asm__(".short 0");
void _08015CB4(void *c) __attribute__((alias("Rec35_Init")));
void sub_08015CB4(void *c) __attribute__((alias("Rec35_Init")));
