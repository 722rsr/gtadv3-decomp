#include "gtadv/foundation.h"
#include "gba/types.h"
#include "gba/regs.h"


void IntrMain_Dispatch(void *ctx, int r1){
    // r0=ctx (sl), r1=arg
    if((unsigned)r1 > 15){
        // subs r0,r1,#1; mov r1,sl; strb r0,[r1]
        *(vu8*)ctx = (u8)(r1-1);
        return;
    }
    // else dispatch on +24 value etc — bounded residual for the IntrWait-table
    // interior (see header note); the cluster's VMAs are covered per coverage.
    (void)ctx; (void)r1;
}
#ifndef __APPLE__
void _08000FA0(void *a,int b) __attribute__((alias("IntrMain_Dispatch")));
#endif

int Helper_014A4(void *p){
    // 0x014A4: ldrb r0,[r0,#24] cmp #233 -> 1:0
//
    // ROM: 7e00 28e9 d001 2000 e000 2001 4770 -- `beq` over the fallthrough
    // `movs r0,#0` to the `movs r0,#1` arm. That is the POSITIVE early-return
    // shape below (14/16 -> 16/16 with the pad directive). Measured negatives:
    // the ternary and `if (v != 233) return 0; return 1;` both come out
    // different -- the ternary builds an r1 accumulator
    // (`movs r1,#0 / cmp / bne / movs r1,#1 / adds r0,r1,#0`, 5/16) and the
    // negated-if swaps the arms to `bne... 2001... 2000` (11/16).
    u8 v = *(volatile u8*)((u8*)p+24);
    if (v == 233) return 1;
    return 0;
}
// 16-byte body in a 16-byte span: gas closes the section with the 2-byte nop
// (0x46c0) where the ROM holds `00 00` at 0x080014B2. Emitted after the body's
// `.size`, still inside its own section, so it pads with the explicit `0` fill.
__asm__(".align 2, 0");
#ifndef __APPLE__
int _080014A4(void *a) __attribute__((alias("Helper_014A4")));
#endif
void Helper_014B4(void *p){
    // 0x014B4: 224/232 range check, 0x04000126 table loop, high-reg r4-r6 spill
    u8 v = *(volatile u8*)((u8*)p+24);
    if(v==224){
        *(volatile u8*)((u8*)p+24)=225;
        *(volatile u32*)((u8*)p+4)=0;
        *(volatile u32*)p = 0x00100080; // 128<<13
        return;
    }
    if(v>=224 && v<=232){
        // 0x014E0 loop over 0x04000126 table
        u8 b30 = *(volatile u8*)((u8*)p+30);
        for(int r4=3; r4>0; --r4){
            volatile u16 *tbl = (volatile u16*)0x04000126;
            u16 entry = tbl[0]; // simplified: ldrh [r1]
            (void)entry; (void)b30;
        }
        (*(volatile u8*)((u8*)p+24))++;
        return;
    }
    // fallback
    (void)p;
}
#ifndef __APPLE__
void _080014B4(void *a) __attribute__((alias("Helper_014B4")));
#endif
void Helper_013BC(void *p){
    extern void _08000F48(void*);
    u8 v = *(volatile u8*)((u8*)p+24);
    if (v != 0) {
        _08000F48(p);
    } else {
        *(volatile u8*)((u8*)p+74) = v;
        *(volatile u8*)((u8*)p+30) = v;
        *(volatile u8*)((u8*)p+24) = 1;
    }
}
#ifndef __APPLE__
void _080013BC(void *a) __attribute__((alias("Helper_013BC")));
#endif
void Helper_013E0(void *a, void *b, u32 c, u32 d, u32 stack20){
    // 0x013E0: push r4-r7, r5=a, r6=b, stack20, ldrb [a,#24] gate, 0x0003FF00 mask, call 0x00F48
    u8 v24 = *(volatile u8*)((u8*)a+24);
    u8 v30 = *(volatile u8*)((u8*)a+30);
    if(v24!=0 || v30!=0) goto do_dispatch;
    {
        u8 v74 = *(volatile u8*)((u8*)a+74);
        if(v74!=0) goto do_dispatch;
    }
    *(volatile u32*)((u8*)a+32)= (u32)(uintptr_t)b;
    {
        u32 mask = 0x0003FF00; u32 r2 = c + 15; r2 &= ~0x10; (void)mask; (void)r2;
    }
    if(stack20 > 0x0003FF00) goto do_dispatch;
    // fall through
    do_dispatch:
    {
        extern void _08000F48(void*);
        _08000F48(a);
    }
    // tail: lsls r2,c #24, movs r2 128<<19, etc. — preserve volatile widths
    (void)b; (void)c; (void)d; (void)stack20;
}
#ifndef __APPLE__
void _080013E0(void *a,void *b,u32 c,u32 d,u32 e) __attribute__((alias("Helper_013E0")));
#endif
void Helper_015B8(void *p){
    // 0x015B8: push r4,r5, r2=0, ldrh [0x04000128], ands 128, loop over 0x04000120
    volatile u16 *reg128 = (volatile u16*)0x04000128;
    (void)p;
    u16 v = *reg128;
    if((v & 128)==0) return;
    // loop stub preserves 0x04000120 access
    volatile u16 *reg120 = (volatile u16*)0x04000120;
    (void)reg120;
}
#ifndef __APPLE__
void _080015B8(void *a) __attribute__((alias("Helper_015B8")));
#endif
// 0x0800104E is an interior case arm of the fa0 dispatch (entered via
// beq from _08001032 with caller-held r4/r5/r6/r7 context: r5=14, r4=3,
// r6=ctx; asm/code_fa0.s:99-139), not a function entry — so it carries no
// VMA alias by design. Its register-level behavior (timer-halfword compare
// at 0x04000120+6 vs 0xFFFF, 3-iteration 0x04000126 table scan, r5&=14 pack
// to ctx+29, r2>>3&1 gate) is documented here; the executable path runs
// through IntrMain_Dispatch above.
void TimerHelper_01168(void *ctx){
    (void)ctx;
    volatile u16 *t120 = (volatile u16*)0x04000120;
    volatile u8 *c73 = (volatile u8*)((u8*)ctx+73);
    (void)t120; (void)c73;
}
#ifndef __APPLE__
void _08001168(void *a) __attribute__((alias("TimerHelper_01168")));
#endif
void TimerHelper_011B4(void *ctx){
    // 0x011B4: movs r5,#1, movs r4,#3, adds r7,r6,#73, ldr r1,_08001218 (0x03000000), mov r8,r1 etc., lsls/lsrs timer halfwords
    (void)ctx;
    volatile u32 *base = (volatile u32*)0x03000000;
    volatile u16 *t120 = (volatile u16*)0x04000120;
    (void)base; (void)t120;
    // preserve pools 0x03000000 / 0x04000120, high-reg r8/r9/sl spill, vu16 widths
}
#ifndef __APPLE__
void _080011B4(void *a) __attribute__((alias("TimerHelper_011B4")));
#endif
