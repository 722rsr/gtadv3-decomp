#include "gtadv/foundation.h"
#include "gtadv/memory.h"
#include "gba/regs.h"
#include "gba/bios.h"

void *RuntimeMemcpy(void *dst, const void *src, u32 n) {
    register u8 *original asm("r5") = (u8 *)dst;
    register u8 *d asm("r4") = original;
    register const u8 *s asm("r3") = (const u8 *)src;
    register u32 count asm("r2") = n;
    register u8 *word_dst asm("r1");
    register u32 scratch asm("r0");
    if (count > 15) {
        scratch = (u32)s;
        __asm__ volatile("" : "+r"(scratch));
        scratch |= (u32)original;
        scratch &= 3u;
        if (scratch == 0) {
            word_dst = original;
            do {
                __asm__ volatile(
                    ".code 16\n\t"
                    "ldmia %0!, {r0}\n\t"
                    "stmia %1!, {r0}\n\t"
                    "ldmia %0!, {r0}\n\t"
                    "stmia %1!, {r0}\n\t"
                    "ldmia %0!, {r0}\n\t"
                    "stmia %1!, {r0}\n\t"
                    "ldmia %0!, {r0}\n\t"
                    "stmia %1!, {r0}\n"
                    : "+r"(s), "+r"(word_dst) : : "r0", "memory");
                count -= 16;
            } while (count > 15);
            if (count > 3) {
                do {
                    __asm__ volatile(
                        ".code 16\n\t"
                        "ldmia %0!, {r0}\n\t"
                        "stmia %1!, {r0}\n"
                        : "+r"(s), "+r"(word_dst) : : "r0", "memory");
                    count -= 4;
                } while (count > 3);
            }
            d = word_dst;
        }
    }
    count--;
    scratch = 1;
    scratch = -scratch;
    if (count != scratch) {
        word_dst = (u8 *)scratch;
        do {
            *d++ = *s++;
            count--;
        } while (count != (u32)word_dst);
    }
    return original;
}

__asm__(".align 2, 0");

void *RuntimeMemset(void *dst, int c, u32 n) {
    // The ROM returns dst in r0 and fills aligned words with a four-store
    // loop for n>=16, then a one-store loop above three. Two ORs build the
    // repeated byte (`v | v<<8`, then `|= <<16`).
    //
    // The byte loop compares the pre-decrement count. `while (n-- > 0)` keeps
    // that sequence; a bare `while (n--)` compares the wrapped post-value.
    // Declaring v before d and deriving the word cursor from dst preserve the
    // ROM's r5/r4/r3 entry copies and its r5-based alignment test.
    u32 v = (u32)c;
    u8 *d = (u8 *)dst;
    if (n > 3 && (((u32)(uintptr_t)dst & 3u) == 0)) {
        u32 *w = (u32 *)(uintptr_t)dst;
        v &= 0xFFu;
        u32 fill = v | (v << 8);
        fill |= fill << 16;
        while (n > 15) {
            *w++ = fill;
            *w++ = fill;
            *w++ = fill;
            *w++ = fill;
            n -= 16;
        }
        while (n > 3) {
            *w++ = fill;
            n -= 4;
        }
        d = (u8 *)w;
    }
    while (n-- > 0)
        *d++ = (u8)v;
    return dst;
}

void *_08002E0A4(void *a, const void *b, u32 c) __attribute__((alias("RuntimeMemcpy")));
void *Sub_08002E0A4(void *a, const void *b, u32 c) __attribute__((alias("RuntimeMemcpy")));
void *_0802E0A4(void *a, const void *b, u32 c) __attribute__((alias("RuntimeMemcpy")));
void *sub_0802E0A4(void *a, const void *b, u32 c) __attribute__((alias("RuntimeMemcpy")));
void *_08002E104(void *a, int b, u32 c) __attribute__((alias("RuntimeMemset")));
void *sub_0802E104(void *a, int b, u32 c) __attribute__((alias("RuntimeMemset")));
void *_0802E104(void *a, int b, u32 c) __attribute__((alias("RuntimeMemset")));
// This TU uses the ROM's non-interworking return sequence. Its body is 82
// bytes; the span ends with a zero halfword rather than gas's Thumb nop fill.
__asm__(".align 2, 0");
