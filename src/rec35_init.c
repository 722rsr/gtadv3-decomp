#include "gba/types.h"
#include "gtadv/memory.h"

// Reference: asm/rec35_init.s 0x08015CB4-0x08015E28
// Leaf _08015CB4 with templates 0x082FCF04/0x082B7410 and course helpers 0x08007770 etc.
// Evidence pools: 0x082FCF04,0x082B7410,0x082A798C, WA offsets 88/100/142 etc. Preserve widths.

extern u32 Course_0x08007770(int ctx, void *X, int r2in, int r3in, u32 s0, u32 s1);
extern void Course_0x0800DAB8(void *p);
extern void Course_0x08007614(void *a,int b,int c,int d);
extern void Course_0x0800798C(void *a,void *b);
extern void Course_0x08007A58(void *p);
extern void Course_0x080075E8(void *a,int b,int c);
extern void Course_0x08025BC8(void *p,int v);
extern void Course_0x08007ABC(void *a,void *b,void *c);
extern void Course_0x0800D77C(void *a,int b,int c);

void Rec35_Init(void *ctx) {
    volatile u8 *c = (volatile u8 *)ctx;
    // bl 0x0802B234 omitted (sound)
    // asm/rec35_init.s:17-25: (0, 0x082FCF04, 1, 0, 4, 2); :26-34: (1, 0x082B7410, 1, 0, 0, 3).
    Course_0x08007770(0, (void *)0x082FCF04, 1, 0, 4, 2);
    Course_0x08007770(1, (void *)0x082B7410, 1, 0, 0, 3);
    *(volatile u32 *)(c+88)=6;
    // check *0x08004B68 result etc. — preserve ldrsh at c+? but simplified
    // Templates at c+142 etc.
    *(volatile u32 *)(c+112)=1;
    Course_0x0800DAB8((void*)(c+16));
    Course_0x08007614((void*)0x082A798C,1,0,4);
    Course_0x08007614((void*)0x082A798C,1,1,5);
    Course_0x0800798C((void*)0x082FCF04, (void*)c);
    Course_0x08007A58((void*)c);
    Course_0x080075E8((void*)0x082FCF04,0,3);
    Course_0x0800798C((void*)0x082B7410,(void*)(c+8));
    Course_0x080075E8((void*)0x082B7410,0,6);
    // etc. preserve alias
}

#ifndef __APPLE__
void _08015CB4(void *c) __attribute__((alias("Rec35_Init")));
#endif
