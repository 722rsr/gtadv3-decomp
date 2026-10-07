#include "gtadv/foundation.h"
// car_racer_constants.c — byte-exact lift of pure Thumb constant-return
// racer step functions (asm evidence per xxd at file offset == VMA).
// All are:  push {r0,lr} / movs r0,#N / pop {r1} / bx r1
//          or:  movs r0,#N / bx lr  (4 B form, no push/pop)
// C ABI:  each takes (void *ctx) and returns the step id constant.

// 0x08009F58 (4 B) = movs r0,#3; bx lr; case 2 -> 3
int CarRacer_Return3(void *ctx) { (void)ctx; return 3; }
#ifndef __APPLE__
int _08009F58(void *) __attribute__((alias("CarRacer_Return3")));
int sub_08009F58(void *) __attribute__((alias("CarRacer_Return3")));
#endif

// 0x0800A06C (4 B) = movs r0,#15; bx lr; case 32 -> 15
int CarRacer_Return15(void *ctx) { (void)ctx; return 15; }
#ifndef __APPLE__
int _0800A06C(void *) __attribute__((alias("CarRacer_Return15")));
int sub_0800A06C(void *) __attribute__((alias("CarRacer_Return15")));
#endif

// 0x0800A070 (4 B) = movs r0,#17; bx lr; case 33 -> 17
int CarRacer_Return17(void *ctx) { (void)ctx; return 17; }
#ifndef __APPLE__
int _0800A070(void *) __attribute__((alias("CarRacer_Return17")));
int sub_0800A070(void *) __attribute__((alias("CarRacer_Return17")));
#endif

// 0x0800A11C (4 B) = movs r0,#26; bx lr; case 22 -> 26
int CarRacer_Return26(void *ctx) { (void)ctx; return 26; }
#ifndef __APPLE__
int _0800A11C(void *) __attribute__((alias("CarRacer_Return26")));
int sub_0800A11C(void *) __attribute__((alias("CarRacer_Return26")));
#endif

// 0x0800A120 (4 B) = movs r0,#20; bx lr; case 19 -> 20
int CarRacer_Return20(void *ctx) { (void)ctx; return 20; }
#ifndef __APPLE__
int _0800A120(void *) __attribute__((alias("CarRacer_Return20")));
int sub_0800A120(void *) __attribute__((alias("CarRacer_Return20")));
#endif

// 0x0800A1B0 (12 B) = push {lr}; bl 0x0800B190; movs r0,#0x31; pop {r1}; bx r1
//                      returns 0x31 (49); case 17 -> 49.
// The `bl` is the engine-command dispatcher and was MISSING: the C dropped the
// call and kept only the constant, so the body compiled to 4 bytes
// (`movs r0,#49; bx lr`) and never matched. `sub_0800B190` is the closure's
// spelling at 0x0800b190 — `asm/race_dispatch.s` defines no `_` twin, so the
// declaration has to be the `sub_` one for the spliced link to resolve it.
extern void sub_0800B190(void);
int CarRacer_Return49(void *ctx) { sub_0800B190(); return 49; }
#ifndef __APPLE__
int _0800A1B0(void *) __attribute__((alias("CarRacer_Return49")));
int sub_0800A1B0(void *) __attribute__((alias("CarRacer_Return49")));
#endif

// 0x0800A1BC (12 B) = push {lr}; bl 0x0800B190; movs r0,#0x31; pop {r1}; bx r1
//                      returns 0x31 (49); case 18 -> 49.
// Byte-for-byte the same body as 0x0800A1B0 and 0x0800A1A4 (the last is
// already promoted as CarTick_Rec_0A1A4); the same missing `bl` was the reason
// this one compiled to 4 bytes too.
int CarRacer_Return49b(void *ctx) { sub_0800B190(); return 49; }
#ifndef __APPLE__
int _0800A1BC(void *) __attribute__((alias("CarRacer_Return49b")));
int sub_0800A1BC(void *) __attribute__((alias("CarRacer_Return49b")));
#endif

// 0x08022D20 (4 B) = bx lr;.hword 0x0000  — no-op stub called from
//                      race phase 12 by Code22C0C_Dispatch; safe to lift.
void CarRacer_NoOp22D20(void *ctx) { (void)ctx; }
#ifndef __APPLE__
void _08022D20(void *) __attribute__((alias("CarRacer_NoOp22D20")));
void sub_08022D20(void *) __attribute__((alias("CarRacer_NoOp22D20")));
#endif

// 0x08023144 (4 B) = bx lr;.hword 0x0000  — no-op stub called from
//                      Code23114_Dispatch.
void CarRacer_NoOp23144(void *ctx) { (void)ctx; }
#ifndef __APPLE__
void _08023144(void *) __attribute__((alias("CarRacer_NoOp23144")));
void sub_08023144(void *) __attribute__((alias("CarRacer_NoOp23144")));
#endif
