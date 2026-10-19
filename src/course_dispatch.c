#include "gba/types.h"
#include "gba/bios.h"

// Course dispatch — VMA 0x08008AAC–0x080095F0
// Mechanical translation with opaque volatile byte pointers, exact widths, explicit stack args, table strides, direct helper ABIs.

extern int _08002BE8(void); // u32 random, s32

extern void _08002C48(int value, u16 course);
__asm__(".set AAC_TBLLO, 0x080CB110");
extern u16 AAC_TBLLO[];
__asm__(".set AAC_TBLHI, 0x080CB132");
extern u16 AAC_TBLHI[];
__asm__(".set AAC_ROOT, 0x030003E0");
extern u8 AAC_ROOT[];

void *Course_Dispatch_08AAC(void *out_) {
    __asm__(".globl AAC_TBLLO\nAAC_TBLLO = 0x080CB110\n"
            ".globl AAC_TBLHI\nAAC_TBLHI = 0x080CB132\n"
            ".globl AAC_ROOT\nAAC_ROOT = 0x030003E0\n");
    register u8 *cursor __asm__("r4") = (u8 *)out_;
    register int rnd1 __asm__("r5");
    register int rnd2 __asm__("r6");
    u8 **root;
    u16 *tbl_lo;
    u32 temp7;
    register u32 t134 __asm__("r3");
    u32 c134;
    __asm__ volatile ("" : "+r" (cursor) : : "memory");
    {
        int t = _08002BE8();
        rnd1 = (s16)t;
    }
    __asm__ volatile ("" : "+r" (rnd1) : : "memory");
    {
        int t = _08002BE8();
        rnd2 = (s16)t;
    }
    __asm__ volatile ("" : "+r" (rnd2) : : "memory");
    tbl_lo = AAC_TBLLO;
    root = (u8 **)AAC_ROOT;
    __asm__ volatile ("" : "+r" (root));
    {
        u8 *rec = *root;
        int idx = *(s16 *)(rec + 22);
        u16 lo = *(u16 *)(tbl_lo + idx);
        _08002C48(rnd1, lo);
        {
            u16 *tbl_hi = AAC_TBLHI;
            u8 *rec2 = *root;
            // Hold r2 across the second table index so the ROM's
            // `movs r3,#22; ldrsh r0,[r0,r3]` allocation is reproduced.
            register u32 hold2 __asm__("r2");
            __asm__ volatile ("" : "=r" (hold2));
            int idx2 = *(s16 *)(rec2 + 22);
            u16 hi = *(u16 *)(tbl_hi + idx2);
            __asm__ volatile ("" : "+r" (hold2) : : "memory");
            _08002C48(rnd2, hi);
        }
        {
            u8 *rc = *root;
            int mode = *(s16 *)(rc + 16);
            if (mode > 1) {
                u32 v0 = *(volatile u32 *)(rc + 84);
                v0 -= 16;
                v0 &= 255u;
                t134 = 134u;
                __asm__ volatile ("" : "+r" (t134));
                t134 <<= 7;
                __asm__ volatile ("" : "+r" (t134));
                c134 = t134;
                __asm__ volatile ("" : "+r" (c134));
                __asm__ volatile ("" : "+r" (t134));
                v0 |= c134;
                *(volatile u16 *)(cursor + 0) = (u16)v0;
                {
                    u32 w = *(volatile u32 *)(rc + 80);
                    w -= 20;
                    t134 = 0x1FFu;
                    __asm__ volatile ("" : "+r" (t134));
                    c134 = t134;
                    __asm__ volatile ("" : "+r" (c134));
                    w &= c134;
                    t134 = 0xFFFFC000u;
                    __asm__ volatile ("" : "+r" (t134));
                    c134 = t134;
                    __asm__ volatile ("" : "+r" (c134));
                    __asm__ volatile ("" : "+r" (t134));
                    w |= c134;
                    {
                        register u32 sh1 __asm__("r1") = ((u32)rnd2 << 9);
                        __asm__ volatile ("" : "+r" (sh1));
                        w |= sh1;
                    }
                    *(volatile u16 *)(cursor + 2) = (u16)w;
                }
                {
                    register u32 x58 __asm__("r1") = *(volatile u16 *)(rc + 58);
                    register u32 p0 __asm__("r0");
                    register u32 x60 __asm__("r3");
                    __asm__ volatile ("" : "+r" (x58));
                    p0 = x58 << 12;
                    __asm__ volatile ("" : "+r" (p0));
                    x60 = *(volatile u16 *)(rc + 60);
                    __asm__ volatile ("" : "+r" (x60));
                    p0 |= x60;
                    *(volatile u16 *)(cursor + 4) = (u16)p0;
                }
                cursor += 8;
                {
                    // Hold r3 across the mode re-read so its index stays in
                    // r1 (`movs r1,#16; ldrsh r0,[r2,r1]`) as in the ROM.
                    register u32 hold3 __asm__("r3");
                    __asm__ volatile ("" : "=r" (hold3));
                    int m2 = *(s16 *)(rc + 16);
                    __asm__ volatile ("" : "+r" (hold3) : : "memory");
                    register u32 sh3 __asm__("r3") = ((u32)rnd1 << 9);
                    __asm__ volatile ("" : "+r" (sh3));
                    if (m2 > 9) {
                        register u32 m0 __asm__("r0");
                        register u32 cp1 __asm__("r1");
                        *(volatile u16 *)(cursor + 0) = 0x33Au;
                        {
                            register u32 w2 __asm__("r0");
                            m0 = 0xFFFF8084u;
                            __asm__ volatile ("" : "+r" (m0));
                            cp1 = m0;
                            __asm__ volatile ("" : "+r" (cp1));
                            w2 = sh3;
                            __asm__ volatile ("" : "+r" (w2));
                            w2 |= cp1;
                            *(volatile u16 *)(cursor + 2) = (u16)w2;
                        }
                        {
                            register u32 x56 __asm__("r0") = *(volatile u16 *)(rc + 56);
                            __asm__ volatile ("" : "+r" (x56));
                            x56 += 16;
                            __asm__ volatile ("" : "+r" (x56));
                            rc = (u8 *)(uintptr_t)*(volatile u16 *)(rc + 58);
                            __asm__ volatile ("" : "+r" (rc));
                            __asm__ volatile ("" : "+r" (cp1));
                            {
                                register u32 sh __asm__("r1") = (u32)(uintptr_t)rc << 12;
                                __asm__ volatile ("" : "+r" (sh));
                                x56 |= sh;
                            }
                            *(volatile u16 *)(cursor + 4) = (u16)x56;
                        }
                        cursor += 8;
                    }
                    *(volatile u16 *)(cursor + 0) = 0x33Au;
                    __asm__ volatile ("" : "+r" (rc));
                    {
                            register u32 w3 __asm__("r3");
                            register u32 mbase __asm__("r2") = 0xFFFF8098u;
                            register u32 mcp __asm__("r0");
                            __asm__ volatile ("" : "+r" (mbase));
                            mcp = mbase;
                            __asm__ volatile ("" : "+r" (mcp));
                            w3 = sh3 | mcp;
                        *(volatile u16 *)(cursor + 2) = (u16)w3;
                    }
                    {
                        register u8 *rr __asm__("r1") = *root;
                        register u32 x58 __asm__("r3") = *(volatile u16 *)(rr + 58);
                        register u32 p0 __asm__("r0");
                        __asm__ volatile ("" : "+r" (x58));
                        p0 = x58 << 12;
                        __asm__ volatile ("" : "+r" (p0));
                        rr = (u8 *)(uintptr_t)*(volatile u16 *)(rr + 56);
                        __asm__ volatile ("" : "+r" (rr));
                        p0 |= (u32)(uintptr_t)rr;
                        *(volatile u16 *)(cursor + 4) = (u16)p0;
                    }
                    cursor += 8;
                    return (void *)cursor;
                }
            } else {
                register u32 c0 __asm__("r1") = 0x433Au;
                register u32 v0 __asm__("r0");
                __asm__ volatile ("" : "+r" (c0));
                v0 = c0;
                __asm__ volatile ("" : "+r" (v0));
                *(volatile u16 *)(cursor + 0) = (u16)v0;
                {
                    register u32 sh0 __asm__("r0") = ((u32)rnd1 << 9);
                    __asm__ volatile ("" : "+r" (sh0));
                    t134 = 0xFFFFC088u;
                    __asm__ volatile ("" : "+r" (t134));
                    c134 = t134;
                    __asm__ volatile ("" : "+r" (c134));
                    __asm__ volatile ("" : "+r" (t134));
                    sh0 |= c134;
                    *(volatile u16 *)(cursor + 2) = (u16)sh0;
                }
                {
                    register u32 x58 __asm__("r1") = *(volatile u16 *)(rc + 58);
                    register u32 p0 __asm__("r0");
                    register u32 lo __asm__("r2");
                    __asm__ volatile ("" : "+r" (x58));
                    p0 = x58 << 12;
                    __asm__ volatile ("" : "+r" (p0));
                    lo = *(volatile u16 *)(rc + 56);
                    __asm__ volatile ("" : "+r" (lo));
                    p0 |= lo;
                    *(volatile u16 *)(cursor + 4) = (u16)p0;
                }
                cursor += 8;
                return (void *)cursor;
            }
        }
    }
}
void _08008AAC(void *a) __attribute__((alias("Course_Dispatch_08AAC")));
void sub_08008AAC(void *a) __attribute__((alias("Course_Dispatch_08AAC")));

void *Course_Dispatch_08BB8(void *out) {
    register volatile u16 *cursor __asm__("r3") = (volatile u16 *)out;
    register volatile u8 * volatile *root __asm__("ip") =
        (volatile u8 * volatile *)0x030003e0u;
    register volatile u8 *rec __asm__("r2") = *root;
    register u32 reg1 __asm__("r1");
    register s32 value __asm__("r0");
    register u32 word4 __asm__("r4");
    register u32 word5 __asm__("r5");
    register u32 mask __asm__("r6");
    // word7/temp7 both encode the ROM's r7 scratch use: word7 (pinned) carries
    // the shifted/loaded temps whose copies to r1 are real instructions, and
    // temp7 spans two blocks so global-alloc must place it in r7, whose
    // mark_home_live also restores the ROM's r7 prologue save.
    register u32 word7 __asm__("r7");
    register u32 temp7;
    register u32 mask8 __asm__("r8");

    value = *(s16 *)(rec + 16);
    if (value > 1) {
        word5 = *(volatile u32 *)(rec + 84);
        word4 = 255;
        __asm__ volatile ("" : "+r"(word4));
        mask8 = word4;
        __asm__ volatile ("" : "+r"(mask8));
        value = word5;
        value &= word4;
        word7 = 128;
        word7 <<= 7;
        __asm__ volatile ("" : "+r"(word7));
        reg1 = word7;
        __asm__ volatile ("" : "+r"(reg1));
        value |= reg1;
        cursor[0] = (u16)value;

        word4 = *(volatile u32 *)(rec + 80);
        value = word4;
        value += 12;
        reg1 = 0x1ff;
        __asm__ volatile ("" : "+r"(reg1));
        mask = reg1;
        __asm__ volatile ("" : "+r"(mask));
        value &= mask;
        temp7 = 0xffffc000u;
        __asm__ volatile ("" : "+r"(reg1));
        __asm__ volatile ("" : "+r"(temp7));
        reg1 = temp7;
        __asm__ volatile ("" : "+r"(temp7));
        __asm__ volatile ("" : "+r"(reg1));
        value |= reg1;
        cursor[1] = (u16)value;

        reg1 = *(volatile u16 *)(rec + 58);
        __asm__ volatile ("" : "+r"(reg1));
        value = reg1 << 12;
        word7 = *(u16 *)(rec + 60);
        __asm__ volatile ("" : "+r"(word7));
        value |= word7;
        cursor[2] = (u16)value;
        cursor = (volatile u16 *)((volatile u8 *)cursor + 8);

        value = *(s16 *)(rec + 16);
        if (value > 9) {
            value = word5 - 6;
            word7 = mask8;
            __asm__ volatile ("" : "+r"(word7));
            value &= word7;
            cursor[0] = (u16)value;

            value = word4 - 20;
            value &= mask;
            word4 = 0xffff8000u;
            __asm__ volatile ("" : "+r"(word4));
            reg1 = word4;
            __asm__ volatile ("" : "+r"(reg1));
            value |= reg1;
            cursor[1] = (u16)value;

            value = *(volatile u16 *)(rec + 56);
            value += 16;
            rec = (volatile u8 *)(uintptr_t)*(volatile u16 *)(rec + 58);
            __asm__ volatile ("" : "+r"(rec) : : "memory");
            reg1 = (u32)(uintptr_t)rec << 12;
            value |= reg1;
            cursor[2] = (u16)value;
            cursor = (volatile u16 *)((volatile u8 *)cursor + 8);
        }

        word7 = (u32)(uintptr_t)root;
        __asm__ volatile ("" : "+r"(word7));
        rec = *(volatile u8 **)(uintptr_t)word7;
        value = *(volatile u32 *)(rec + 84);
        value -= 6;
        reg1 = mask8;
        __asm__ volatile ("" : "+r"(reg1));
        value &= reg1;
        cursor[0] = (u16)value;
        value = *(volatile u32 *)(rec + 80);
        value &= mask;
        word4 = 0xffff8000u;
        __asm__ volatile ("" : "+r"(word4));
        reg1 = word4;
        __asm__ volatile ("" : "+r"(reg1));
        value |= reg1;
        cursor[1] = (u16)value;
        word7 = *(u16 *)(rec + 58);
        value = word7 << 12;
        rec = (volatile u8 *)(uintptr_t)*(u16 *)(rec + 56);
        value |= (u32)(uintptr_t)rec;
    } else {
        value = *(volatile u32 *)(rec + 84);
        value -= 6;
        reg1 = 255;
        __asm__ volatile ("" : "+r"(reg1));
        value &= reg1;
        word4 = 128;
        word4 <<= 7;
        __asm__ volatile ("" : "+r"(word4));
        reg1 = word4;
        __asm__ volatile ("" : "+r"(reg1));
        value |= reg1;
        cursor[0] = (u16)value;

        value = *(volatile u32 *)(rec + 80);
        reg1 = 0x1ff;
        __asm__ volatile ("" : "+r"(reg1));
        value &= reg1;
        temp7 = 0xffffc000u;
        __asm__ volatile ("" : "+r"(temp7));
        reg1 = temp7;
        __asm__ volatile ("" : "+r"(temp7));
        __asm__ volatile ("" : "+r"(reg1));
        value |= reg1;
        cursor[1] = (u16)value;

        reg1 = *(volatile u16 *)(rec + 58);
        __asm__ volatile ("" : "+r"(reg1));
        value = reg1 << 12;
        rec = (volatile u8 *)(uintptr_t)*(u16 *)(rec + 56);
        value |= (u32)(uintptr_t)rec;
    }

    cursor[2] = (u16)value;
    cursor = (volatile u16 *)((volatile u8 *)cursor + 8);
    return (void *)cursor;
}
void *_08008BB8(void *a) __attribute__((alias("Course_Dispatch_08BB8")));
void *sub_08008BB8(void *a) __attribute__((alias("Course_Dispatch_08BB8")));

// The 0x08008CA0 phase machine (2384 bytes) is owned by
// src/scene_record_dispatch.c as CoursePhaseMachine_08CA0.

// ROM entry alias.
void RomEmit8AAC(void *rec) __attribute__((alias("Course_Dispatch_08AAC")));
void *RomEmit8BB8(void *out) __attribute__((alias("Course_Dispatch_08BB8")));
