#include "gba/types.h"
#include "gtadv/memory.h"

// 0x080163B8 (284 B) — rec35 runtime-registered event driver.
// _080163B8(ev, arg_b, arg_c, ctx): `idx = ev - 1`, ev > 12 falls to the shared
// epilogue. Slots 2/3 (ev 3/4) and 7..10 (ev 8..11) are empty. armcc puts the
// jump-table base literal in word[0] of the table block and the 12 absolute
// slot addresses in words[1..12]; agbcc's thumb.md switch expansion emits the
// same base-literal-then-entries layout, so this is a plain `switch`.
// _080015C9C's own prototype reads (dead, rec) -- at this call site r1 is
// arg_b, so the second parameter is the one that carries a record pointer.
extern void _08015CB4(void *ctx);                      // 0x08015CB4 rec35_init.c
extern void _080015C9C(void *dead, int arg);           // 0x08015C9C menu_ff78_f.c
extern void _0800D854(void *p);                        // 0x0800D854 menu_stage.c
extern void _0800D8E4(void *p);                        // 0x0800D8E4 menu_stage.c
extern void _08015ED0(void *ctx);                      // 0x08015ED0 rec35_stage.c
extern void _08015F68(void *ctx);                      // 0x08015F68 rec35_stage.c
extern void sub_0800160C4(void *ctx, int a, int b);    // 0x080160C4 rec35_16002.c
extern void sub_080016004(void *ctx, int a, int b);    // 0x08016004 rec35_16002.c
extern void sub_080016164(void *ctx, int a, int b);    // 0x08016164 rec35_16002.c
extern void sub_0800161A0(void *ctx, int a, int b);    // 0x080161A0 rec35_16002.c
extern void sub_0800162E0(void *ctx);                  // 0x080162E0 rec35_16002.c
extern void sub_080015CB0(void *ctx);                  // 0x08015CB0 menu_ff78_c.c (callee takes none)

void Rec35_Driver(int ev, int arg_b, int arg_c, void *ctx_)
{
    u8 *ctx = (u8 *)ctx_;

    // Arm order below is the ROM's body order (2, 5, 7, 6, 1, 12): agbcc
    // emits switch arms in source order and the table slots stay index-ordered.
    switch (ev) {
    case 2:
        _080015C9C(ctx, arg_b);
        break;
    case 3:
    case 4:
        break;
    case 5:
        _0800D854(ctx + 16);
        _0800D8E4(ctx + 120);
        switch (*(s16 *)(ctx + 142)) {
        case 0:
            _08015ED0(ctx);
            break;
        case 1:
            _08015F68(ctx);
            break;
        }
        break;
    case 7:
        sub_0800162E0(ctx);
        break;
    case 6:
        if (*(u16 *)(ctx + 20) != 0) {
            if (*(s16 *)(ctx + 144) == 0) {
                switch (*(s16 *)(ctx + 142)) {
                case 0:
                    sub_0800160C4(ctx, (u16)arg_b, (u16)arg_c);
                    break;
                case 1:
                    sub_080016004(ctx, (u16)arg_b, (u16)arg_c);
                    break;
                }
            } else {
                switch (*(s16 *)(ctx + 142)) {
                case 0:
                    sub_080016164(ctx, (u16)arg_b, (u16)arg_c);
                    break;
                case 1:
                    sub_0800161A0(ctx, (u16)arg_b, (u16)arg_c);
                    break;
                }
            }
        }
        break;
    case 8:
    case 9:
    case 10:
    case 11:
        break;
    case 1:
        _08015CB4(ctx);
        break;
    case 12:
        sub_080015CB0(ctx);
        break;
    }
}

// The ROM pads the 282-byte body with two zero bytes; without this the section
// ends 2-mod-4 and `as` pads it with a Thumb `nop` (0xC046) instead. Must sit
// before the alias declarations or agbcc drops it (same as Race_Setup_19200).
__asm__(".pushsection .text.Rec35_Driver,\"ax\",%progbits\n\t.short 0\n\t.popsection");
void _080163B8(int ev, int arg_b, int arg_c, void *ctx) __attribute__((alias("Rec35_Driver")));
