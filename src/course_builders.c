#include "gba/types.h"
#include "gba/bios.h"

// Course builders — VMA 0x08008290–0x08008AAC
// Mechanical translation with opaque volatile byte pointers, exact u8/u16/s16/u32 widths,
// explicit stack args/temporaries, table strides, direct helper ABIs.
// Uses course trace 0x0203F760 -> 0x030039D0 root where it proves r4=out/w args.

extern int sub_0802D97C(int a, int b); // u32 div, s32
extern int sub_0802D978(int a, int b); // s32 div, s16

void *Course_Build_Emit8A(void *out, int w)
{
    register volatile u8 *dst __asm__("r4") = (volatile u8 *)out;
    register s32 width __asm__("r5") = (s32)w;
    register s32 pitch __asm__("r8");
    register s32 mid __asm__("r9");
    register s32 tall __asm__("sl");
    register u32 tableCell __asm__("r6");
    register u32 result __asm__("r0");
    register u32 base __asm__("r1");
    register u32 rowBase2 __asm__("r2");
    register u32 label __asm__("r1");

    pitch = 150;
    pitch <<= 1;
    result = (u32)sub_0802D97C(width, pitch);
    result = (u32)sub_0802D978((s32)result, 3);
    tall = (s32)result;

    tableCell = 0x4650;
    result = (u32)sub_0802D97C(width, (s32)tableCell);
    result = (u32)sub_0802D978((s32)result, pitch);
    mid = (s32)result;
    result = (u32)sub_0802D978(width, (s32)tableCell);
    pitch = (s32)result;

    label = 0x803C;
    __asm__ volatile ("" : "+r" (label));
    width = (s32)label;
    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 90;
    result = (u32)sub_0802D978(pitch, 10);
    tableCell = 0x030003E0;
    base = *(volatile u32 *)(uintptr_t)tableCell;
    result <<= 1;
    {
        register u32 low __asm__("r2");
        low = *(volatile u16 *)(uintptr_t)(base + 52);
        result += low;
    }
    base = *(volatile u16 *)(uintptr_t)(base + 54);
    base <<= 12;
    result |= base;
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 98;
    result = (u32)sub_0802D97C(pitch, 10);
    {
        register u32 part __asm__("r3");
        register u32 high __asm__("r1");
        rowBase2 = *(volatile u32 *)(uintptr_t)tableCell;
        __asm__ volatile ("" : "+r" (rowBase2));
        result <<= 1;
        part = *(volatile u16 *)(uintptr_t)(rowBase2 + 52);
        __asm__ volatile ("" : "+r" (part));
        result = part + result;
        part = *(volatile u16 *)(uintptr_t)(rowBase2 + 54);
        __asm__ volatile ("" : "+r" (part));
        high = part << 12;
        result |= high;
    }
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 106;
    {
        result = *(volatile u16 *)(uintptr_t)(rowBase2 + 52);
        result += 20;
        rowBase2 = *(volatile u16 *)(uintptr_t)(rowBase2 + 54);
        __asm__ volatile ("" : "+r" (rowBase2));
        {
            register u32 high __asm__("r1");
            high = rowBase2 << 12;
            result |= high;
        }
    }
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 114;
    result = (u32)sub_0802D978(mid, 10);
    base = *(volatile u32 *)(uintptr_t)tableCell;
    result <<= 1;
    {
        register u32 low __asm__("r2");
        low = *(volatile u16 *)(uintptr_t)(base + 52);
        result += low;
    }
    base = *(volatile u16 *)(uintptr_t)(base + 54);
    base <<= 12;
    result |= base;
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 122;
    result = (u32)sub_0802D97C(mid, 10);
    {
        register u32 part __asm__("r3");
        register u32 high __asm__("r1");
        rowBase2 = *(volatile u32 *)(uintptr_t)tableCell;
        __asm__ volatile ("" : "+r" (rowBase2));
        result <<= 1;
        part = *(volatile u16 *)(uintptr_t)(rowBase2 + 52);
        __asm__ volatile ("" : "+r" (part));
        result = part + result;
        part = *(volatile u16 *)(uintptr_t)(rowBase2 + 54);
        __asm__ volatile ("" : "+r" (part));
        high = part << 12;
        result |= high;
    }
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 130;
    {
        result = *(volatile u16 *)(uintptr_t)(rowBase2 + 52);
        result += 22;
        rowBase2 = *(volatile u16 *)(uintptr_t)(rowBase2 + 54);
        __asm__ volatile ("" : "+r" (rowBase2));
        {
            register u32 high __asm__("r1");
            high = rowBase2 << 12;
            result |= high;
        }
    }
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 138;
    result = (u32)sub_0802D978(tall, 10);
    base = *(volatile u32 *)(uintptr_t)tableCell;
    result <<= 1;
    {
        register u32 low __asm__("r2");
        low = *(volatile u16 *)(uintptr_t)(base + 52);
        result += low;
    }
    base = *(volatile u16 *)(uintptr_t)(base + 54);
    base <<= 12;
    result |= base;
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 146;
    result = (u32)sub_0802D97C(tall, 10);
    base = *(volatile u32 *)(uintptr_t)tableCell;
    result <<= 1;
    {
        register u32 part __asm__("r3");
        part = *(volatile u16 *)(uintptr_t)(base + 52);
        __asm__ volatile ("" : "+r" (part));
        result = part + result;
        base = *(volatile u16 *)(uintptr_t)(base + 54);
        __asm__ volatile ("" : "+r" (base));
        base <<= 12;
        result |= base;
    }
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;
    return (void *)dst;
}
void *_0800861C(void *a, int b) __attribute__((alias("Course_Build_Emit8A")));
void *sub_0800861C(void *a, int b) __attribute__((alias("Course_Build_Emit8A")));

void *Course_Build_Emit8B(void *out, int w)
{
    register volatile u8 *dst __asm__("r4") = (volatile u8 *)out;
    register s32 width __asm__("r5") = (s32)w;
    register s32 pitch __asm__("r8");
    register s32 mid __asm__("r9");
    register s32 tall __asm__("sl");
    register u32 tableCell __asm__("r6");
    register u32 result __asm__("r0");
    register u32 base __asm__("r1");
    register u32 rowBase2 __asm__("r2");
    register u32 label __asm__("r1");

    pitch = 150;
    pitch <<= 1;
    result = (u32)sub_0802D97C(width, pitch);
    result = (u32)sub_0802D978((s32)result, 3);
    tall = (s32)result;

    tableCell = 0x4650;
    result = (u32)sub_0802D97C(width, (s32)tableCell);
    result = (u32)sub_0802D978((s32)result, pitch);
    mid = (s32)result;
    result = (u32)sub_0802D978(width, (s32)tableCell);
    pitch = (s32)result;

    label = 128;
    label <<= 8;
    __asm__ volatile ("" : "+r" (label));
    width = (s32)label;
    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 56;
    result = (u32)sub_0802D978(pitch, 10);
    tableCell = 0x030003E0;
    base = *(volatile u32 *)(uintptr_t)tableCell;
    result <<= 1;
    {
        register u32 low __asm__("r2");
        low = *(volatile u16 *)(uintptr_t)(base + 52);
        result += low;
    }
    base = *(volatile u16 *)(uintptr_t)(base + 54);
    base <<= 12;
    result |= base;
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 64;
    result = (u32)sub_0802D97C(pitch, 10);
    {
        register u32 part __asm__("r3");
        register u32 high __asm__("r1");
        rowBase2 = *(volatile u32 *)(uintptr_t)tableCell;
        __asm__ volatile ("" : "+r" (rowBase2));
        result <<= 1;
        part = *(volatile u16 *)(uintptr_t)(rowBase2 + 52);
        __asm__ volatile ("" : "+r" (part));
        result = part + result;
        part = *(volatile u16 *)(uintptr_t)(rowBase2 + 54);
        __asm__ volatile ("" : "+r" (part));
        high = part << 12;
        result |= high;
    }
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 70;
    {
        result = *(volatile u16 *)(uintptr_t)(rowBase2 + 52);
        result += 20;
        rowBase2 = *(volatile u16 *)(uintptr_t)(rowBase2 + 54);
        __asm__ volatile ("" : "+r" (rowBase2));
        {
            register u32 high __asm__("r1");
            high = rowBase2 << 12;
            result |= high;
        }
    }
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 77;
    result = (u32)sub_0802D978(mid, 10);
    base = *(volatile u32 *)(uintptr_t)tableCell;
    result <<= 1;
    {
        register u32 low __asm__("r2");
        low = *(volatile u16 *)(uintptr_t)(base + 52);
        result += low;
    }
    base = *(volatile u16 *)(uintptr_t)(base + 54);
    base <<= 12;
    result |= base;
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 85;
    result = (u32)sub_0802D97C(mid, 10);
    {
        register u32 part __asm__("r3");
        register u32 high __asm__("r1");
        rowBase2 = *(volatile u32 *)(uintptr_t)tableCell;
        __asm__ volatile ("" : "+r" (rowBase2));
        result <<= 1;
        part = *(volatile u16 *)(uintptr_t)(rowBase2 + 52);
        __asm__ volatile ("" : "+r" (part));
        result = part + result;
        part = *(volatile u16 *)(uintptr_t)(rowBase2 + 54);
        __asm__ volatile ("" : "+r" (part));
        high = part << 12;
        result |= high;
    }
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 91;
    {
        result = *(volatile u16 *)(uintptr_t)(rowBase2 + 52);
        result += 22;
        rowBase2 = *(volatile u16 *)(uintptr_t)(rowBase2 + 54);
        __asm__ volatile ("" : "+r" (rowBase2));
        {
            register u32 high __asm__("r1");
            high = rowBase2 << 12;
            result |= high;
        }
    }
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 98;
    result = (u32)sub_0802D978(tall, 10);
    base = *(volatile u32 *)(uintptr_t)tableCell;
    result <<= 1;
    {
        register u32 low __asm__("r2");
        low = *(volatile u16 *)(uintptr_t)(base + 52);
        result += low;
    }
    base = *(volatile u16 *)(uintptr_t)(base + 54);
    base <<= 12;
    result |= base;
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 106;
    result = (u32)sub_0802D97C(tall, 10);
    base = *(volatile u32 *)(uintptr_t)tableCell;
    result <<= 1;
    {
        register u32 part __asm__("r3");
        part = *(volatile u16 *)(uintptr_t)(base + 52);
        __asm__ volatile ("" : "+r" (part));
        result = part + result;
        base = *(volatile u16 *)(uintptr_t)(base + 54);
        __asm__ volatile ("" : "+r" (base));
        base <<= 12;
        result |= base;
    }
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;
    return (void *)dst;
}
void *_08008768(void *a, int b) __attribute__((alias("Course_Build_Emit8B")));
void *sub_08008768(void *a, int b) __attribute__((alias("Course_Build_Emit8B")));

void *Course_Build_Emit8C(void *out, int w)
{
    register volatile u8 *dst __asm__("r4") = (volatile u8 *)out;
    register s32 width __asm__("r5") = (s32)w;
    register s32 pitch __asm__("r8");
    register s32 mid __asm__("r9");
    register s32 tall __asm__("sl");
    register u32 tableCell __asm__("r6");
    register u32 result __asm__("r0");
    register u32 base __asm__("r1");
    register u32 rowBase2 __asm__("r2");
    register u32 label __asm__("r1");

    pitch = 150;
    pitch <<= 1;
    result = (u32)sub_0802D97C(width, pitch);
    result = (u32)sub_0802D978((s32)result, 3);
    tall = (s32)result;

    tableCell = 0x4650;
    result = (u32)sub_0802D97C(width, (s32)tableCell);
    result = (u32)sub_0802D978((s32)result, pitch);
    mid = (s32)result;
    result = (u32)sub_0802D978(width, (s32)tableCell);
    pitch = (s32)result;

    label = 0x808F;
    __asm__ volatile ("" : "+r" (label));
    width = (s32)label;
    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 56;
    result = (u32)sub_0802D978(pitch, 10);
    tableCell = 0x030003E0;
    base = *(volatile u32 *)(uintptr_t)tableCell;
    result <<= 1;
    {
        register u32 low __asm__("r2");
        low = *(volatile u16 *)(uintptr_t)(base + 52);
        result += low;
    }
    base = *(volatile u16 *)(uintptr_t)(base + 54);
    base <<= 12;
    result |= base;
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 64;
    result = (u32)sub_0802D97C(pitch, 10);
    {
        register u32 part __asm__("r3");
        register u32 high __asm__("r1");
        rowBase2 = *(volatile u32 *)(uintptr_t)tableCell;
        __asm__ volatile ("" : "+r" (rowBase2));
        result <<= 1;
        part = *(volatile u16 *)(uintptr_t)(rowBase2 + 52);
        __asm__ volatile ("" : "+r" (part));
        result = part + result;
        part = *(volatile u16 *)(uintptr_t)(rowBase2 + 54);
        __asm__ volatile ("" : "+r" (part));
        high = part << 12;
        result |= high;
    }
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 70;
    {
        result = *(volatile u16 *)(uintptr_t)(rowBase2 + 52);
        result += 20;
        rowBase2 = *(volatile u16 *)(uintptr_t)(rowBase2 + 54);
        __asm__ volatile ("" : "+r" (rowBase2));
        {
            register u32 high __asm__("r1");
            high = rowBase2 << 12;
            result |= high;
        }
    }
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 77;
    result = (u32)sub_0802D978(mid, 10);
    base = *(volatile u32 *)(uintptr_t)tableCell;
    result <<= 1;
    {
        register u32 low __asm__("r2");
        low = *(volatile u16 *)(uintptr_t)(base + 52);
        result += low;
    }
    base = *(volatile u16 *)(uintptr_t)(base + 54);
    base <<= 12;
    result |= base;
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 85;
    result = (u32)sub_0802D97C(mid, 10);
    {
        register u32 part __asm__("r3");
        register u32 high __asm__("r1");
        rowBase2 = *(volatile u32 *)(uintptr_t)tableCell;
        __asm__ volatile ("" : "+r" (rowBase2));
        result <<= 1;
        part = *(volatile u16 *)(uintptr_t)(rowBase2 + 52);
        __asm__ volatile ("" : "+r" (part));
        result = part + result;
        part = *(volatile u16 *)(uintptr_t)(rowBase2 + 54);
        __asm__ volatile ("" : "+r" (part));
        high = part << 12;
        result |= high;
    }
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 91;
    {
        result = *(volatile u16 *)(uintptr_t)(rowBase2 + 52);
        result += 22;
        rowBase2 = *(volatile u16 *)(uintptr_t)(rowBase2 + 54);
        __asm__ volatile ("" : "+r" (rowBase2));
        {
            register u32 high __asm__("r1");
            high = rowBase2 << 12;
            result |= high;
        }
    }
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 98;
    result = (u32)sub_0802D978(tall, 10);
    base = *(volatile u32 *)(uintptr_t)tableCell;
    result <<= 1;
    {
        register u32 low __asm__("r2");
        low = *(volatile u16 *)(uintptr_t)(base + 52);
        result += low;
    }
    base = *(volatile u16 *)(uintptr_t)(base + 54);
    base <<= 12;
    result |= base;
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;

    *(volatile u16 *)(dst + 0) = (u16)width;
    *(volatile u16 *)(dst + 2) = 106;
    result = (u32)sub_0802D97C(tall, 10);
    base = *(volatile u32 *)(uintptr_t)tableCell;
    result <<= 1;
    {
        register u32 part __asm__("r3");
        part = *(volatile u16 *)(uintptr_t)(base + 52);
        __asm__ volatile ("" : "+r" (part));
        result = part + result;
        base = *(volatile u16 *)(uintptr_t)(base + 54);
        __asm__ volatile ("" : "+r" (base));
        base <<= 12;
        result |= base;
    }
    *(volatile u16 *)(dst + 4) = (u16)result;
    dst += 8;
    return (void *)dst;
}
void *_080088B0(void *a, int b) __attribute__((alias("Course_Build_Emit8C")));
void *sub_080088B0(void *a, int b) __attribute__((alias("Course_Build_Emit8C")));

// Return the end cursor consumed by the next course-emission step.
// The body is C; empty register barriers retain the compiler's ROM-matching
// operand choices around the signed and halfword loads.
void *Course_Build_Tail_089FC(void *out) {
    register volatile u16 *cursor __asm__("r2") = (volatile u16 *)out;
    register volatile u8 * volatile *root __asm__("r4") =
        (volatile u8 * volatile *)0x030003e0u;
    register volatile u8 *rec __asm__("r3") = *root;
    register u32 reg1 __asm__("r1");
    register s32 value __asm__("r0");
    register u32 literal __asm__("r5");

    value = *(s16 *)(uintptr_t)(rec + 16);
    if (value > 1) {
        value = *(volatile u32 *)(rec + 84);
        reg1 = 255;
        __asm__ volatile ("" : "+r"(reg1));
        value &= reg1;
        literal = 128;
        literal <<= 7;
        __asm__ volatile ("" : "+r"(literal));
        reg1 = literal;
        __asm__ volatile ("" : "+r"(reg1));
        value |= reg1;
        cursor[0] = (u16)value;

        value = *(volatile u32 *)(rec + 80);
        value += 12;
        literal = 0x1ff;
        __asm__ volatile ("" : "+r"(literal));
        reg1 = literal;
        __asm__ volatile ("" : "+r"(reg1));
        value &= reg1;
        literal = 0xffffc000u;
        __asm__ volatile ("" : "+r"(literal));
        reg1 = literal;
        __asm__ volatile ("" : "+r"(reg1));
        value |= reg1;
        cursor[1] = (u16)value;

        reg1 = *(volatile u16 *)(rec + 58);
        __asm__ volatile ("" : "+r"(reg1));
        value = reg1 << 12;
        literal = *(volatile u16 *)(rec + 60);
        __asm__ volatile ("" : "+r"(literal));
        value |= literal;
        cursor[2] = (u16)value;
        cursor = (volatile u16 *)((volatile u8 *)cursor + 8);

        value = *(s16 *)(uintptr_t)(rec + 16);
        if (value > 9) {
            cursor[0] = 74;
            literal = 0x8094;
            __asm__ volatile ("" : "+r"(literal));
            value = literal;
            cursor[1] = (u16)value;
            value = *(volatile u16 *)(rec + 56);
            value += 16;
            rec = (volatile u8 *)(uintptr_t)*(volatile u16 *)(rec + 58);
            __asm__ volatile ("" : "+r"(rec));
            reg1 = (u32)(uintptr_t)rec << 12;
            value |= reg1;
            cursor[2] = (u16)value;
            cursor = (volatile u16 *)((volatile u8 *)cursor + 8);
        }

        cursor[0] = 74;
        reg1 = 0x80a8;
        __asm__ volatile ("" : "+r"(reg1));
        value = reg1;
        cursor[1] = (u16)value;
        {
            register volatile u8 *rec2 __asm__("r1") = *root;
            register u16 high __asm__("r3");
            high = *(volatile u16 *)(rec2 + 58);
            value = high << 12;
            value |= ((volatile u16 *)rec2)[28];
        }
    } else {
        literal = 0x404a;
        __asm__ volatile ("" : "+r"(literal));
        value = literal;
        cursor[0] = (u16)value;
        reg1 = 0xc0a8;
        __asm__ volatile ("" : "+r"(reg1));
        value = reg1;
        cursor[1] = (u16)value;
        literal = *(volatile u16 *)(rec + 58);
        __asm__ volatile ("" : "+r"(literal));
        value = literal << 12;
        {
            register u32 hold1 __asm__("r1");
            __asm__ volatile ("" : "=r"(hold1));
            value |= ((volatile u16 *)rec)[28];
            __asm__ volatile ("" : : "r"(hold1));
        }
    }
    cursor[2] = (u16)value;
    cursor = (volatile u16 *)((volatile u8 *)cursor + 8);
    return (void *)cursor;
}
void *_080089FC(void *a) __attribute__((alias("Course_Build_Tail_089FC")));
void *sub_080089FC(void *a) __attribute__((alias("Course_Build_Tail_089FC")));

// ROM entry alias.
void *RomEmit861C(void *out, int w) __attribute__((alias("Course_Build_Emit8A")));
void *RomEmit8768(void *out, int w) __attribute__((alias("Course_Build_Emit8B")));
void *RomEmit88B0(void *out, int w) __attribute__((alias("Course_Build_Emit8C")));
void *RomEmit89FC(void *out) __attribute__((alias("Course_Build_Tail_089FC")));
