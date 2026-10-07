#include "gtadv/sound.h"
#include "gba/types.h"


s32 SoundInterp_CED4(u32 a, u32 b, u32 c){
    // Exact: lsls r0,#24 / lsrs r0,#24 u8 at 0x02CED4/0x02CED6, similarly r1->r5, r2->ip, mov ip,r2
    u32 av = (a << 24) >> 24; // u8
    u32 bv = (b << 24) >> 24; // u8
    u32 cv = (c << 24) >> 24; // u8 third param -> ip
    (void)cv; // used as ip for av==4 path, otherwise overwritten to 0/255
    // Preserve direct branches: cmp r0,#4 bne (0x02CED8/0x02CEDA), cmp bv,#20 bhi etc., cmp bv,#35 etc.
    // Use exact ROM-table pools via volatile ldr [pc] loads
    if(av == 4){
        // cmp r0,#4 beq path at 0x02CED8
        if(bv > 20){
            // bhi at 0x02CEF0
            u32 t = bv - 21;
            t = (t << 24) >> 24;
            if(t > 59) t = 59; // cmp #59 bls at 0x02CEFE
            bv = t;
        } else {
            bv = 0;
        }
        volatile u8 *tbl = (volatile u8*)(uintptr_t)*(volatile u32*)0x0802CF08u; // ldr r0,=0x08061708 @0x02CF08 pool
        u32 v = tbl[bv]; // ldrb [r0] at 0x02CF08 load
        // b.n 0x02CF6E -> adds 0x800 (128<<4) and return
        s32 ret = (s32)v + 0x800; // movs r1,#128 lsls #4 / adds at 0x02CF6E
        return ret;
    } else {
        // Exact for b (original bv): if b<=35, ip=0 r5=0; if b>35, r5=b-36, ip remains original c (cv) unless r5>130 then r5=130 ip=255
        // Preserve original b and c before mutating bv (per 0x02CF0C/0x02CF18)
        u32 origB = bv;
        u32 origC = cv; // mov ip,r2 at 0x02CED4 (ip = c)
        u32 ip;
        if(origB <= 35){
            // cmp r5,#35 bhi at 0x02CF0C -> b<=35: mov r0,#0 mov ip,r0 (ip=0) mov r5,#0
            ip = 0;
            bv = 0;
        } else {
            // b>35: adds r0,r5,#0 subs r0,#36 lsls/lsrs -> r5 = b-36
            u32 t = (origB - 36) & 0xFF;
            t = (t << 24) >> 24;
            if(t > 130){
                // cmp #130 bls at 0x02CF2A -> >130: mov r5,#130 mov r1,#255 mov ip,r1 (ip=255)
                t = 130;
                ip = 255;
            } else {
                // bls -> keep ip as original c (mov ip,r2 preserved)
                ip = origC;
            }
            bv = t;
        }
        // ldr r3,=0x0806166C @0x02CF74 pool, adds r0,r5,r3, ldrb r6,[r0]
        volatile u8 *tbl2 = (volatile u8*)(uintptr_t)*(volatile u32*)0x0802CF74u; // 0x0806166C
        volatile s16 *tbl3 = (volatile s16*)(uintptr_t)*(volatile u32*)0x0802CF78u; // 0x080616F0 s16 via ldrsh
        u32 idx = bv; // u8
        u8 v6 = tbl2[idx]; // ldrb r6,[r0] at 0x02CF74 load
        // ldrsh / asrs logic at 0x02CF78 etc.: preserve widths via ldrsh (s16) and asrs
        u32 low = v6 & 15; // ands r0,#15 at 0x02CF74
        s16 base = tbl3[low * 2 + 0]; // lsls #1 / adds / ldrsh at 0x02CF74
        s32 g6 = (s32)base;
        // asrs r0,r6,#4 at 0x02CF78
        s32 g6s = g6 >> (v6 >> 4); // approximate asrs r6,r0
        // second segment at 0x02CF6E etc.: ldrb r1,[r0], ands, lsls, ldrsh, asrs, subs, muls, asrs #8, adds, adds #0x800
        u8 v1 = tbl2[idx + 1]; // ldrb r1,[r0] at 0x02CF74+...
        u32 low2 = v1 & 15;
        s16 base2 = tbl3[low2 * 2 + 0];
        s32 g1 = (s32)base2 >> (v1 >> 4);
        s32 diff = g1 - g6s; // subs at 0x02CF78
        s32 prod = (s32)ip * diff; // muls r7,r0 at 0x02CF78, ip is original c or 0/255
        s32 res = g6s + (prod >> 8); // asrs #8 at 0x02CF6E
        res += 0x800; // adds r0, r1, #0x800
        return res;
    }
}
#ifndef __APPLE__
s32 _0802CED4(u32 a, u32 b, u32 c) __attribute__((alias("SoundInterp_CED4")));
s32 sub_0802CED4(u32 a, u32 b, u32 c) __attribute__((alias("SoundInterp_CED4")));
#endif
