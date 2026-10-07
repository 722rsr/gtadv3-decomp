#include "gtadv/car_physics_lane.h"
#include "gba/types.h"

// Lane: carphys_tick 0x0800A668–0x0800AA40
// Sources: asm/carphys_tick.s (A668 tick table, A9A0 grid helpers, AA20 shared tick)
// Substantiated per asm header comments and asm/carphys_racer.s.

extern int sub_08004CF0(void); // pop LIFO
extern void sub_08004CD4(int); // push
extern void sub_08004CC4(void); // clear
extern int _0800AA20(void); // shared racer tick (returns step id)
extern int _08009BF8(void *); extern int _08009F58(void *);
extern int _0800A62C(void *); extern int _08009F5C(void *);
extern int _0800A668(void *); // 0x0800A668 per-record tick (alias defined below)
extern int _0800A94C(void *); // 0x0800A94C secondary dispatch (src/car_tick_helpers.c)
extern int _0800A074(void *); extern int _0800A150(void *);
extern int _0800A1B0(void *); extern int _0800A1BC(void *);
extern int _0800A120(void *); extern int _0800A124(void *); extern int _0800A1C8(void *);
extern int _0800A11C(void *); extern int _08009FB8(void *);
extern int _0800A028(void *); extern int _0800A06C(void *); extern int _0800A070(void *);
extern int _0800A1D4(void *); extern int _0800A1E0(void *); extern int _0800A1EC(void *); extern int _0800A1F8(void *);
extern int _0800A204(void *); extern int _0800A21C(void *); extern int _0800A228(void *);
extern int _0800A23C(void *); extern int _0800A248(void *);
extern int _0800AF84(void *); extern int _08009BCC(void);
// Jump-table targets read straight off the 52 words at 0x0800A688. `_08009BCC`
// is the only one the ROM calls with r0 still holding the selector, so its
// prototype is (void); the other two 2-instruction leaves are given a ctx
// argument because the ROM sets r0 = ctx before every other call.
extern int _08009FFC(void *);
// 0x0800A020 and 0x0800A024 are real 4-byte ROM leaves -- `movs r0,#44; bx lr`
// and `movs r0,#15; bx lr`. They were bare `extern`s, so the case 41/44 calls
// had no C owner in the slice link. Defined here so the calls reach C. Read straight off baserom.gba at file offsets 0xa020/0xa024.
int _0800A020(void *a) { (void)a; return 44; }
int _0800A024(void *a) { (void)a; return 15; }
extern int _0800A1A4(void *);
extern int _0800A254(void *); extern int _0800A344(void *);
extern int _0800A44C(void *); extern int _0800A518(void *);
extern void _0800D778(void);
extern int sub_08025CF4(int,int,int);

// Grid aggregate helpers — substantiated: min over 1..3 qualifying cells, seed 3
// ROM 0x0800A9A0 / 0x0800A9E0: `lsls r0,#24 / lsrs r1,r0,#24` materialises the
// call result as a u8 (not `& 0xFF` on an int); the cell test is
// `subs r0,r1,#1 / cmp r0,#2 / bhi` = UNSIGNED <=2, and the running min uses
// `cmp r5,r6 / bgt` = `last <= best` (assign on ties, not `last < best`).
int CarTick_GridA(int type,int row){
    int best=3, last=0;
    for(int col=0; col<=2; col++){
        u8 v = sub_08025CF4(type,row,col);
        // `(unsigned)`, NOT `(u8)`: `v-1` promotes to int under C's usual
        // arithmetic conversions, and the signedness is load-bearing -- it is
        // what makes agbcc emit `bhi` (0xd8, the ROM's form) rather than `bge`
        // (0xda). The values agree at runtime, but an 8-bit cast hands agbcc a
        // different type and it REALLOCATES: (u8) scored 33/64 where this
        // scores 62/64. Do not "simplify" this to (u8) or to a bare 2.
        if ((unsigned)(v - 1) <= 2u) last=v;
        if (last <= best) best=last;
    }
    return best;
}
__asm__(".align 2, 0");
int CarTick_GridB(int type,int row){
    int best=3, last=0;
    for(int col=3; col<=10; col++){
        u8 v = sub_08025CF4(type,row,col);
        // `(unsigned)`, NOT `(u8)`: `v-1` promotes to int under C's usual
        // arithmetic conversions, and the signedness is load-bearing -- it is
        // what makes agbcc emit `bhi` (0xd8, the ROM's form) rather than `bge`
        // (0xda). The values agree at runtime, but an 8-bit cast hands agbcc a
        // different type and it REALLOCATES: (u8) scored 33/64 where this
        // scores 62/64. Do not "simplify" this to (u8) or to a bare 2.
        if ((unsigned)(v - 1) <= 2u) last=v;
        if (last <= best) best=last;
    }
    return best;
}
// Same filler case as `CarTick_GridA` above, for `_0800A9E0`. 62/64 -> EXACT.
__asm__(".align 2, 0");
#if defined(__linux__) || defined(__ELF__)
#ifndef __APPLE__
int _0800A9A0(int t,int r) __attribute__((alias("CarTick_GridA")));
int sub_0800A9A0(int t,int r) __attribute__((alias("CarTick_GridA")));
int _0800A9E0(int t,int r) __attribute__((alias("CarTick_GridB")));
int sub_0800A9E0(int t,int r) __attribute__((alias("CarTick_GridB")));
#endif
#endif

// The s16 reads are deliberately NOT volatile: a volatile s16 read makes agbcc
// fold the load to `ldrh r0,[r5] / lsls #16 / asrs #16`, where the ROM has
// `movs r1,#0 / ldrsh r0,[r5,r1]`. All three s16 reads in the ROM (selector
// 0x0800A670, phase 0x0800A882, requeue 0x0800A8EC) are the indexed form.
int CarTick_Dispatcher(void *ctx){
    int requeue = 0;
    int step;
    switch (*(s16 *)ctx){
        case 0: step = _08009BF8(ctx); break;
        case 2: step = _08009F58(ctx); break;
        case 49: step = _0800AF84(ctx); break;
        case 51: step = _08009BCC(); break;
        case 45: step = 46; break;
        case 46: step = 12; break;
        case 12: step = 48; break;
        case 48: step = 13; break;
        case 13: step = _0800A62C(ctx); break;
        case 14: step = _08009F5C(ctx); break;
        case 23: step = _08009FB8(ctx); break;
        case 42: step = _08009FFC(ctx); break;
        case 41: step = _0800A020(ctx); break;
        case 44: step = _0800A024(ctx); break;
        case 31: step = _0800A028(ctx); break;
        case 32: step = _0800A06C(ctx); break;
        case 33: step = _0800A070(ctx); break;
        case 15: step = _0800A074(ctx); break;
        case 22: step = _0800A11C(ctx); break;
        case 19: step = _0800A120(ctx); break;
        case 20: step = _0800A124(ctx); break;
        case 16: step = _0800A150(ctx); break;
        case 40: step = _0800A1A4(ctx); break;
        case 17: step = _0800A1B0(ctx); break;
        case 18: step = _0800A1BC(ctx); break;
        case 21: step = _0800A1C8(ctx); break;
        case 27: step = _0800A1D4(ctx); break;
        case 28: step = _0800A1E0(ctx); break;
        case 29: step = _0800A1EC(ctx); break;
        case 30: step = _0800A1F8(ctx); break;
        case 38: step = _0800A204(ctx); break;
        case 36: step = _0800A21C(ctx); break;
        case 37: step = _0800A228(ctx); break;
        case 24: step = _0800A23C(ctx); break;
        case 25: step = _0800A248(ctx); break;
        case 35: {
            s16 phase = *(s16 *)(0x03001780u + 0x0FBCu);
            switch (phase){
                case 0: step = _0800A344(ctx); break;
                case 7: step = _0800A518(ctx); break;
                case 3: step = _0800A44C(ctx); break;
                default: step = _0800A254(ctx); break;
            }
            break;
        }
        case 6: step = 9; break;
        case 9: step = 7; break;
        case 7:
            requeue = 1;
            step = (*(s32 *)((u8 *)ctx + 0x2C) != 0) ? 10 : 8;
            break;
        case 10: _0800D778(); step = 11; break;
        default: step = sub_08004CF0(); break;
    }
    if (requeue) sub_08004CD4(*(s16 *)ctx);
    return step;
}
#if defined(__linux__) || defined(__ELF__)
#ifndef __APPLE__
int _0800A668(void *c) __attribute__((alias("CarTick_Dispatcher")));
#endif
#endif

// Shared racer tick body 0xAA20 — bounded, pools 0x030005B0 queue, 0x03000610 manager
// Exact: push lr, sub sp#8, mov r0,sp, bl sub_08024048, ldr r0,[sp], ldr r1,[sp+4], cmp r0,#-1, bne, bl sub_08004CF0
int CarTick_Shared(void){
    long long pair;
    register int a __asm__("r0");
    extern void sub_08024048(void*);
    sub_08024048(&pair);
    a = (int)*(volatile long long *)&pair;
    register int neg __asm__("r2");
    neg = -1;
    if (a == neg) sub_08004CF0();
    return a;
}
#if defined(__linux__) || defined(__ELF__)
#ifndef __APPLE__
int _0800AA20(void) __attribute__((alias("CarTick_Shared")));
int sub_0800AA20(void) __attribute__((alias("CarTick_Shared")));
#endif
#endif

// Simple record-tick wrappers 0xA1D4/0xA1E0/0xA1EC/0xA1F8 and 0xA204/0xA21C/0xA23C/0xA248 — bounded, no pool, push lr / bl _0800AA20
int CarTick_Rec_0A1D4(void *ctx){ return _0800AA20(); (void)ctx; }
#ifndef __APPLE__
int _0800A1D4(void *c) __attribute__((alias("CarTick_Rec_0A1D4")));
#endif
int CarTick_Rec_0A1E0(void *ctx){ return _0800AA20(); (void)ctx; }
#ifndef __APPLE__
int _0800A1E0(void *c) __attribute__((alias("CarTick_Rec_0A1E0")));
#endif
int CarTick_Rec_0A1EC(void *ctx){ return _0800AA20(); (void)ctx; }
#ifndef __APPLE__
int _0800A1EC(void *c) __attribute__((alias("CarTick_Rec_0A1EC")));
#endif
// 0x0800A980(ctx) — top-level substate dispatch (asm/carphys_tick.s:1880-1900):
int _0800A980(void *ctx)
{
    int r;
    switch (*(u16 *)((u8 *)ctx + 6)) {
    case 0: r = _0800A668(ctx); break;
    case 2: r = 47; break;
    default: r = _0800A94C(ctx); break;
    }
    return r;
}
// 30 bytes of code in a 4-aligned section: same filler case as the other
// leaves in this file (ROM holds `00 00`, gas closes with `nop`). 30/32 -> EXACT.
__asm__(".align 2, 0");

int CarTick_Rec_0A1F8(void *ctx){ return _0800AA20(); (void)ctx; }
#ifndef __APPLE__
int _0800A1F8(void *c) __attribute__((alias("CarTick_Rec_0A1F8")));
#endif
int CarTick_Rec_0A204(void *ctx){ return _0800AA20(); (void)ctx; }
// 10 bytes of code in a 4-aligned section: the ROM holds `00 00` at
// 0x0800A20E where gas closes the section with `nop`. Same file-scope `.align`
// idiom as the neighbouring leaves.
__asm__(".align 2, 0");
#ifndef __APPLE__
int _0800A204(void *c) __attribute__((alias("CarTick_Rec_0A204")));
#endif
// 0x0800A210 — unreferenced twin (asm/carphys_tick.s:852-858): the same
// `push {lr}; bl 0x0800AA20; pop {r1}; bx r1` + `00 00` pad as its neighbours,
// with no BL/literal xref anywhere in the closure. Typed in asm, so it is a
// known start with no C candidate until this body exists.
int CarTick_Rec_0A210(void *ctx){ return _0800AA20(); (void)ctx; }
__asm__(".align 2, 0");
#ifndef __APPLE__
int _0800A210(void *c) __attribute__((alias("CarTick_Rec_0A210")));
#endif
int CarTick_Rec_0A21C(void *ctx){ return _0800AA20(); (void)ctx; }
#ifndef __APPLE__
int _0800A21C(void *c) __attribute__((alias("CarTick_Rec_0A21C")));
#endif
int CarTick_Rec_0A23C(void *ctx){ return _0800AA20(); (void)ctx; }
#ifndef __APPLE__
int _0800A23C(void *c) __attribute__((alias("CarTick_Rec_0A23C")));
#endif
int CarTick_Rec_0A248(void *ctx){ return _0800AA20(); (void)ctx; }
#ifndef __APPLE__
int _0800A248(void *c) __attribute__((alias("CarTick_Rec_0A248")));
#endif
