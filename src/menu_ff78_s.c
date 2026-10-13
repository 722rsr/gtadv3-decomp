// ============================================================================
// menu_ff78_s.c — reconstructed C for asm/menu_ff78.s (1 function).
//
// Body transcribed instruction-for-instruction from the asm listing.
// External callees use the spellings retained by the independent C slice.
//
//   sub_080013B68 (0x080013B68) — (rec) long builder + 13950 x3 + loop.
// ============================================================================

#include "gba/types.h"

extern void _08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void _08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i);
extern void _0800D97C(void *a, int b);
extern void _0800DBE8(void *a);
extern void _0800139F0(void *a, u32 b);
extern void _080013AF4(void *a);
extern void _080013AA8(void *a);
extern void sub_080013E60(void *a);
extern void _080013950(void *a, void *b, u32 c, u32 d);
extern void _080026A2C(void *a, int b, int c); // sprite_obj_263e.c Sprite_SetXYRel: ldr r4,[r0,#8]
extern int _08025500(int v);
extern int _08025518(int v);
extern int _08025530(int v);
extern int _08025548(const void *v); // 12-byte record ptr
extern int _080255C4(const void *v); // 12-byte record ptr
extern int _08025640(const void *v); // 12-byte record ptr

// ----------------------------------------------------------------------------
// sub_080013B68 — (rec):
//   26A2C(u32[rec+580],120,32).
//   If (s8)u8[0x080CCEEC + s16[174]*20 + 8]==1:
//     7B18(rec,30,128,80,8,1,1,0).
//   7B18(rec,8,72,64,7,3,1,0); 7B18(rec,20,168,48,3,1,1,0);
//   7B18(rec,20,168,72,3,1,1,0); 7B18(rec,17,0,56,3,1,1,0);
//   7B18(rec,21,0,40,3,1,1,0).
//   If s16[186]==0: 139F0(rec, u16[136]).
//   D97C(164,15); D97C(168,10);
//   7B18(rec,18,u32[156],u32[160],3,1,1,0); 13AF4(rec);
//   7B18(rec,13,56,96,...); 7B18(rec,14,56,112,...);
//   7B18(rec,12,56,128,...) (all {3,1,1,0}).
//   7BFC(rec+8,u32[584],u32[588],88,32,9,3,1,0);
//   7BFC(rec+16,u32[596],u32[600],171,56,5,1,1,0);
//   7BFC(rec+24,u32[608],u32[612],171,80,5,1,1,0).
//   13950(rec,104,104,(s8)(25500(s)+25548(s*12+0x030017B0)));
//   13950(rec,104,120,(s8)(25518(s)+255C4(...)));
//   13950(rec,104,136,(s8)(25530(s)+25640(...))) with s = s16[174].
//   Loop r5=0 while r5 < (s8)u8[0x080CCEEC + s*20]:
//     7B18(rec, s16[0x080CB6AC+r5*2], u32[0x080CB6E0+r5*8],
//       u32[...+4], 4,1,1,0); r5++.
//   13AA8(rec); DBE8(rec+32); 13E60(rec).
void MenuFF78_13B68(void *rec_) {
    u8 *rec = (u8 *)rec_;
    _080026A2C((void *)(uintptr_t)*(u32 *)(uintptr_t)(rec + 580), 120, 32);
    {
    extern u8 J13B68_BASE[];
    __asm__(".globl J13B68_BASE\nJ13B68_BASE = 0x080CCEEC\n");
    u8 *base = (u8 *)(uintptr_t)J13B68_BASE;
    if ((s8)base[(u32)(s32)*(s16 *)(uintptr_t)(rec + 174) * 20 + 8] == 1)
        _08007B18((void *)rec, 30, 128, 80, 8, 1, 1, 0);
    _08007B18((void *)(uintptr_t)(rec + 68), 8, 72, 64, 7, 3, 1, 0);
    _08007B18((void *)rec, 20, 168, 48, 3, 1, 1, 0);
    _08007B18((void *)rec, 20, 168, 72, 3, 1, 1, 0);
    _08007B18((void *)rec, 17, 0, 56, 3, 1, 1, 0);
    _08007B18((void *)rec, 21, 0, 40, 3, 1, 1, 0);
    if (*(s16 *)(uintptr_t)(rec + 186) == 0)
        _0800139F0((void *)rec, *(u16 *)(uintptr_t)(rec + 136));
    _0800D97C((void *)(uintptr_t)(rec + 164), 15);
    _0800D97C((void *)(uintptr_t)(rec + 168), 10);
    _08007B18((void *)rec, 18,
                 (int)*(u32 *)(uintptr_t)(rec + 156),
                 (int)*(u32 *)(uintptr_t)(rec + 160),
                 3, 1, 1, 0);
    _080013AF4((void *)rec);
    _08007B18((void *)rec, 13, 56, 96, 3, 1, 1, 0);
    _08007B18((void *)rec, 14, 56, 112, 3, 1, 1, 0);
    _08007B18((void *)rec, 12, 56, 128, 3, 1, 1, 0);
    _08007BFC((void *)(uintptr_t)(rec + 8),
                 (int)*(u32 *)(uintptr_t)(rec + 584),
                 (int)*(u32 *)(uintptr_t)(rec + 588),
                 88, 32, 9, 3, 1, 0);
    _08007BFC((void *)(uintptr_t)(rec + 16),
                 (int)*(u32 *)(uintptr_t)(rec + 596),
                 (int)*(u32 *)(uintptr_t)(rec + 600),
                 171, 56, 5, 1, 1, 0);
    _08007BFC((void *)(uintptr_t)(rec + 24),
                 (int)*(u32 *)(uintptr_t)(rec + 608),
                 (int)*(u32 *)(uintptr_t)(rec + 612),
                 171, 80, 5, 1, 1, 0);
    {
        int c0 = (int)(s8)(_08025500((int)*(s16 *)(uintptr_t)(rec + 174))
            + _08025548((const void *)(uintptr_t)((u32)(s32)*(s16 *)(uintptr_t)(rec + 174) * 12 + 0x030017B0u)));
        _080013950((void *)rec, (void *)(uintptr_t)104, 104, (u32)c0);
        int c1 = (int)(s8)(_08025518((int)*(s16 *)(uintptr_t)(rec + 174))
            + _080255C4((const void *)(uintptr_t)((u32)(s32)*(s16 *)(uintptr_t)(rec + 174) * 12 + 0x030017B0u)));
        _080013950((void *)rec, (void *)(uintptr_t)104, 120, (u32)c1);
        int c2 = (int)(s8)(_08025530((int)*(s16 *)(uintptr_t)(rec + 174))
            + _08025640((const void *)(uintptr_t)((u32)(s32)*(s16 *)(uintptr_t)(rec + 174) * 12 + 0x030017B0u)));
        _080013950((void *)rec, (void *)(uintptr_t)104, 136, (u32)c2);
    }
    {
        extern u8 J13B68_E[];
        __asm__(".globl J13B68_E\nJ13B68_E = 0x080CB6AC\n");
        u32 i = 0;
        /* Keep the ROM's one-time base copy (`mov r8,r9` at 0x08013DD6)
           before the event-word table load. */
        if ((int)i < (int)((s8 *)(uintptr_t)base)[(u32)(s32)*(s16 *)(uintptr_t)(rec + 174) * 20]) {
            register u8 *lp __asm__("r8") = base;
            __asm__ volatile("" : "+r"(lp));
            for (;;) {
                /* Keep the event-table literal in r1 before index arithmetic. */
                register const s16 *idx_base __asm__("r1") =
                    (const s16 *)(uintptr_t)J13B68_E;
                __asm__ volatile("" : "+r"(idx_base));
                _08007B18((void *)rec,
                             (int)idx_base[i],
                             (int)*(u32 *)(uintptr_t)(0x080CB6E0u + i * 8),
                             (int)*(u32 *)(uintptr_t)(0x080CB6E0u + 4 + i * 8),
                             4, 1, 1, 0);
                i++;
                /* The ROM re-forms rec + 174 on each iteration. Prevent
                   retaining that address across the call, which otherwise
                   displaces the table base and widens the saved-register set. */
                register u8 *loop_rec __asm__("r0") = rec;
                __asm__ volatile("" : "+r"(loop_rec));
                if (!((int)i < (int)((s8 *)(uintptr_t)lp)[(u32)(s32)*(s16 *)(uintptr_t)(loop_rec + 174) * 20]))
                    break;
            }
        }
    }
    _080013AA8((void *)rec);
    _0800DBE8((void *)(uintptr_t)(rec + 32));
    sub_080013E60((void *)rec);
    }
}
void _080013B68(void *a) __attribute__((alias("MenuFF78_13B68")));
void Sub_080013B68(void *a) __attribute__((alias("MenuFF78_13B68")));
void sub_080013B68(void *a) __attribute__((alias("MenuFF78_13B68")));
