// ============================================================================
// sprite_obj_263e0.c — C lift of the sprite/object-placement cluster
//   asm/code_263e0.s (VMA 0x080263E0–0x08026F30, 26 functions)
//
// Every function is transcribed instruction-for-instruction from the cited
// asm listing (labels = bare VMAs; control flow, widths and call ABI
// preserved; the byte-exact asm build remains the ground truth and `make -B`
// still gates byte-identity). No speculative behavior beyond the asm.
//
// The cluster is the shared menu/race sprite-object API:
//   - VRAM tile/palette upload helpers (CpuFastSet _0802D970)
//   - course-record lookups over tables 0x087999D8/0x0879A304/0x083D7BE8
//     via the proven _08007498/_0800748C (Course_Seek/Course_ArrBase) pair
//   - OAM record emitters over the 16-byte object pool (_08002BFC) and the
//     slot lists rooted at 0x03000140 (_08002C34) / 0x03000134 banks
//   - the 0x03001674 menu-record family (Sprite_Init / Sprite_MenuEvent /
//     Sprite_TickRecords)

#include "gtadv/menus.h"
#include "gba/types.h"

// ----------------------------------------------------------------------------
// Extern callees. On the macOS host a standalone TU needs linkable bodies, so
// weak no-ops are provided under __APPLE__ (overridden by strong test mocks,
// exactly the override model the trampoline pipeline uses on ARM).
// ----------------------------------------------------------------------------
#ifdef __APPLE__
__attribute__((weak)) void _0802D970(const void *a, void *b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void _0802D974(const void *a, void *b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) int _0802D978(int a, int b) { (void)b; return a; }
__attribute__((weak)) int _0802D97C(int a, int b) { (void)b; return 0; }
__attribute__((weak)) void _0802D988(const void *a, void *b) { (void)a; (void)b; }
__attribute__((weak)) void *_08007498(void *X, int i) { (void)X; (void)i; return (void *)0; }
__attribute__((weak)) void *_0800748C(void *X) { (void)X; return (void *)0; }
__attribute__((weak)) void *_08002BFC(int a) { (void)a; return (void *)0; }
__attribute__((weak)) void _08002C34(int a, void *b) { (void)a; (void)b; }
__attribute__((weak)) void _08002C60(int a, u16 b, u16 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void _08002C74(int a, u16 b) { (void)a; (void)b; }
__attribute__((weak)) void _08002C84(int a) { (void)a; }
__attribute__((weak)) void *_08004E0C(void *a) { (void)a; return (void *)0; }
__attribute__((weak)) int _08005758(u32 a) { (void)a; return 0; }
__attribute__((weak)) int _08005790(u32 a) { (void)a; return 0; }
__attribute__((weak)) void _080057A4(u32 a, u32 b) { (void)a; (void)b; }
__attribute__((weak)) void _08007538(void *a, int b, void *c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void _080075E8(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) s16 _080261B0(int a) { (void)a; return 0; }
__attribute__((weak)) void _080261D4(int a) { (void)a; }
__attribute__((weak)) void _080261E8(int a, int b) { (void)a; (void)b; }
__attribute__((weak)) s16 _080261F8(int a) { (void)a; return 0; }
__attribute__((weak)) void _0802620C(void *a, void *b) { (void)a; (void)b; }
__attribute__((weak)) void *_08026230(int a, void *b) { (void)a; (void)b; return (void *)0; }
__attribute__((weak)) void sub_080262A4(void *a) { (void)a; }
__attribute__((weak)) void sub_08026220(void) { }        // Code261D4_CopySlot
// `track_car_helpers.c` exports sub_080262A4/sub_08026220 under
// `#ifndef __APPLE__` only, so the host needs its own definition of the spelling
// its CALL site uses. Same for the two below: _08026948 and _080269AC now call
// the CLOSURE spellings, which `arm-none-eabi-nm -n build-code/code.o` binds as
// `sub_080057A4` and `sub_08026230` — neither `_` form is a closure symbol.
// Both now have C definitions (src/save.c:287, src/runtime_record_helpers.c:444).
__attribute__((weak)) void sub_080057A4(u32 a, u32 b) { (void)a; (void)b; }
__attribute__((weak)) void *sub_08026230(int a, void *b) { (void)a; (void)b; return (void *)0; }
#else
extern void _0802D970(const void *a, void *b, u32 c);   // swi 0x0C CpuFastSet (bios_wrappers.c)
extern void _0802D974(const void *a, void *b, u32 c);   // swi 0x0B CpuSet (bios_wrappers.c)
extern int _0802D978(int a, int b);                     // swi 0x06 Div (bios_wrappers.c)
extern int _0802D97C(int a, int b);                     // swi 0x06 DivRem (bios_wrappers.c)
extern void _0802D988(const void *a, void *b);          // swi 0x11 LZ77UnCompWram (bios_wrappers.c)
extern void *_08007498(void *X, int i);                 // Course_Seek (course_records.c)
extern void *_0800748C(void *X);                        // Course_ArrBase (course_records.c)
extern void *_08002BFC(int a);                          // 16-byte object alloc (foundation_runtime.c)
extern void _08026220(void);                          // Code261D4_CopySlot (track_car_helpers.c)
                                                    // NOT _080026220: that name has a spurious digit and
                                                    // assembles at 0x08026220, drift -2013265920.
extern void _08002C34(int a, void *b);                  // slot-list insert (foundation_runtime.c)
extern void _08002C60(int a, u16 b, u16 c);             // store +6/+30 (foundation_runtime.c)
extern void _08002C74(int a, u16 b);                    // store +22 (foundation_runtime.c)
extern void _08002C84(int a);                           // negate +6 (foundation_runtime.c)
extern void *_08004E0C(void *a);                        // record node alloc+link (foundation_subsys.c)
extern int _08005758(u32 a);                            // save.c bump-free-low
extern int _08005790(u32 a);                            // save.c bump-free-high (returns cursor)
extern void _080057A4(u32 a, u32 b);                    // save.c DMA3 descriptor
extern void sub_080057A4(u32 a, u32 b);                 // 0x080057A4 closure spelling (save.c)
extern void _08007538(void *a, int b, void *c);         // Course_EmitLane_07538 (course_resource.c)
extern void _080075E8(void *a, int b, int c);           // Course_EmitLane_075E8 (course_resource.c)
/* Already inside this file's `#ifdef __APPLE__` host block (line 36), so no
   further guard: the host needs the spelling the CALL site uses, which is the
   `sub_` one, because track_car_helpers.c only exports that alias under
   `#ifndef __APPLE__`. Guarding this again inverted the host path and
   apple_decls caught the resulting undeclared call. */
extern s16 _080261B0(int a);                            // Ai_AwardLeafGet (ai_award_leaves.c)
extern void _080261D4(int a);                           // 0x080261D4 s16 +2 (asm;.set alias)
extern void _080261E8(int a, int b);                    // 0x080261E8 u16 store +4 (asm;.set alias)
extern s16 _080261F8(int a);                            // 0x080261F8 s16 +4 (asm;.set alias)
extern void _0802620C(void *a, void *b);                // 0x0802620C record base store (asm;.set alias)
extern void *_08026230(int a, void *b);                 // 0x08026230 resource wrap A (asm;.set alias)
extern void *sub_08026230(int a, void *b);               // 0x08026230 closure spelling (the `_`
                                                            // form is not bound by code.o)
extern void sub_080262A4(void *a);                // 0x080262A4 resource wrap B (asm;.set alias)
#endif

// Course-record table bases (from the literal pools of this cluster).
#define CREC_TABLE0  ((void *)(uintptr_t)0x087999D8)
#define CREC_TABLE1  ((void *)(uintptr_t)0x0879A304)
#define CREC_TABLE2  ((void *)(uintptr_t)0x083D7BE8)
#define WA_10BC      (*(volatile u8 *)((uintptr_t)0x03001780 + 0x10BC))
#define SPR_MENU_REC (*(volatile void **)(uintptr_t)0x03001674)

// Keypad block at IWRAM 0x030035C0; the field read by Sprite_SetXYRel is the
// u16 at +12. The struct spelling keeps the address as base+member-offset:
// agbcc then emits `ldr rX,[pc]` for 0x030035C0 and `ldrh rX,[rX,#12]`,
// matching the ROM pool (same lever as menu_ccec.c / menu_records.c).
typedef struct { u8 spr_kp_pad[12]; u16 spr_kp_w12; } SpriteKp;
#define SPR_KP_W12 (((volatile SpriteKp *)(uintptr_t)0x030035C0)->spr_kp_w12)

// Thumb-1 `asrs reg` semantics: shift amount = low byte of the word; for
// amounts >= 32 the result is sign fill (matches the ARMv4 behavior).
//
// This has to be a MACRO, not a function. The ROM holds `asrs rD, rM` — a
// register-specified shift inline in the body, not a call. agbcc emits the
// three-operand `asr rD, rN, rM` for `signed >> unsigned` and the Thumb back
// end expands it to that single 2-byte instruction, so the shift must be
// spelled at the use site: behind a C function agbcc emits `bl asr_low8` to a
// local `t` symbol, which is unresolved at probe time.
#define asr_low8(v, p) ((v) >> (unsigned)*(volatile u32 *)(p))

// ============================================================================
// sub_0800263E0 (0x080263E0, 0xC8 B) — 12 CpuFastSet tile uploads from a
// source buffer into the 0x06010120 VRAM tile series, bank = arg1<<5.
//   for i in 0..11: CpuFastSet(dst + i*192, 0x06010120 + i*0x100 + bank*32, 48)
// ============================================================================
void Sprite_BlitTiles(u8 *dst, int bank) {
    static const u32 vram[12] = {
        0x06010120u, 0x06010220u, 0x06010320u, 0x06010420u,
        0x06010520u, 0x06010620u, 0x06010920u, 0x06010A20u,
        0x06010B20u, 0x06010C20u, 0x06010D20u, 0x06010E20u,
    };
    u32 base = (u32)(bank << 5);
    for (int i = 0; i < 12; i++) {
        _0802D970(dst + i * 192, (void *)(uintptr_t)(vram[i] + base), 48);
    }
}
#ifndef __APPLE__
void _0800263E0(u8 *a, int b) __attribute__((alias("Sprite_BlitTiles")));
void sub_0800263E0(u8 *a, int b) __attribute__((alias("Sprite_BlitTiles")));
void Course_0x080263E0(u8 *a, int b) __attribute__((alias("Sprite_BlitTiles"))); /* trampoline elimination: friendly-name spelling used by race_setup.c */
#endif

// ============================================================================
// sub_0800264D8 (0x080264D8, 0x48 B) — course byte lookup:
//   record = Seek(0x087999D8, [rec+0]);  arr = ArrBase(record)
//   off = (arg1 & 0xFFF) >> (word[arr] & 0xFF)     (arithmetic, low-byte shift)
//   b = u8[ArrBase(Seek(record, 0)) + off] & 127
//   return (void*)([rec+4] + b*2304)
// ============================================================================
void *Sprite_CourseByte(void *rec, u32 off) {
    void *record = _08007498(CREC_TABLE0, (int)*(volatile u32 *)((u8 *)rec + 0));
    void *arr = _0800748C(record);
    int r4 = (int)(off & 0xFFFu);
    r4 = asr_low8(r4, arr);
    void *arr2 = _0800748C(_08007498(record, 0));
    u8 *p = (u8 *)arr2 + r4;
    u32 mk = 127u;
    u32 b = mk & (u32)*(volatile u8 *)p;
    // Register pin (measured : the ROM materialises `b * 2304` in
    // r2 (lsls/adds/lsls) and then loads the [rec+4] base into r0, the return
    // register.  Left to itself agbcc allocates the chain to r0 (it is the
    // return register, so the natural accumulator) and then reuses the now-dead
    // r1 mask for the base load -- same mnemonics, different register numbers,
    // 5 halfwords off.  Pinning the chain result frees r0 for the base load.
    // CONTROL : dropping this pin scores 66/72, i.e. the pin is
    // load-bearing and the operand-order fix below is not the whole answer.
    register u32 m __asm__("r2") = b * 2304u;
    // Operand order: the ROM's `adds r0, r0, r2` loads [rec+4] into r0 FIRST
    // and adds the r2 chain second. Writing the sum the other way round makes
    // agbcc emit the same add with its operands exchanged (`adds r0, r2, r0`),
    // which is one byte off. Both operands are u32, so the wraparound is
    // identical -- this is a commutation, not a rewrite of the arithmetic.
    return (void *)(uintptr_t)(*(volatile u32 *)((u8 *)rec + 4) + m);
}
#ifndef __APPLE__
void *_0800264D8(void *a, u32 b) __attribute__((alias("Sprite_CourseByte")));
void *sub_0800264D8(void *a, u32 b) __attribute__((alias("Sprite_CourseByte")));
#endif

// ============================================================================
// sub_080026520 (0x08026520, 0x34 B) — course surface tile lookup:
//   entry = u32[0x080CC1E4 + arg0*4]
//   arr = ArrBase(Seek(Seek(0x083D7BE8, entry), 2))
//   return arr + arg1*32
// ============================================================================
void *Sprite_CourseSurface(int idx, int n) {
    void *tbl;
    u32 *base;
    u32 entry;
    void *arr;
    tbl = CREC_TABLE2;
    base = (u32 *)(uintptr_t)0x080CC1E4u;
    // Load-order pin (measured): the ROM holds `ldr r2,=0x083D7BE8 /
    // ldr r1,=0x080CC1E4` back-to-back BEFORE `lsls r0,#2`, whereas agbcc
    // keeps the entry-table literal live only around its use and emits
    // `ldr r2 / lsls / ldr r1` (first difference +0x4, pool order flipped).
    // Splitting both addresses into locals is not enough -- the scheduler
    // still sinks the second load past the shift. The empty volatile barrier
    // forces both literal loads to materialise before the shift with no
    // emitted instructions (only a `.code 16` mode directive).
    __asm__ volatile ("" : : : "memory");
    entry = *(volatile u32 *)((u8 *)base + (idx << 2));
    arr = _0800748C(_08007498(_08007498(tbl, (int)entry), 2));
    return (void *)((u8 *)arr + (n << 5));
}
#ifndef __APPLE__
void *_080026520(int a, int b) __attribute__((alias("Sprite_CourseSurface")));
void *sub_080026520(int a, int b) __attribute__((alias("Sprite_CourseSurface")));
void *Course_0x08026520(int a, int b) __attribute__((alias("Sprite_CourseSurface"))); /* trampoline elimination: friendly-name spelling used by race_cluster.c */
#endif

// ============================================================================
// sub_080026554 (0x08026554, 0x120 B, unlabeled) — two-record OAM emitter.
//   (rec, x, spr): reads the sprite template at spr:
//     sl=[spr+44], r9=u8[spr+4], r6=u16[spr+24], sp0=(u16)(u32[spr+24]+64)
//   flag = WA_10BC ? 2048 : 1024;  same course-byte probe as 0x264D8 with
//   arg1=x; if the probe byte has bit7 set: _08002C84(u32[spr+20]) and swap
//   r6/sp0 (sp0 gets the original u16, r6 the +64 value).
//   Emits two objects into the pool (_08002BFC), linked via _08002C34 with
//   idx = u32[spr+8].  attr2 = (u32[spr+0] - u16[sl+0/2]) & 0x1FF | 0xC000 |
//   (u32[spr+20] << 9); attr4 = r6/sp0 | flag | u16[spr+28].
// ============================================================================
void Sprite_PlacePair(void *rec, int x, void *spr) {
    u8 *r7 = (u8 *)spr;
    u32 sl = *(volatile u32 *)(r7 + 44);
    u8 r9 = *(volatile u8 *)(r7 + 4);
    u16 r6 = *(volatile u16 *)(r7 + 24);
    u16 sp0 = (u16)(*(volatile u32 *)(r7 + 24) + 64);
    void *record = _08007498(CREC_TABLE0, (int)*(volatile u32 *)(rec));
    void *arr = _0800748C(record);
    int r5 = (int)(x & 0xFFFu);
    r5 = asr_low8(r5, arr);
    void *arr2 = _0800748C(_08007498(record, 0));
    int flag = WA_10BC ? 2048 : 1024;
    if ((*(volatile u8 *)((u8 *)arr2 + r5) & 128u) != 0) {
        _08002C84((int)*(volatile u32 *)(r7 + 20));
        sp0 = *(volatile u16 *)(r7 + 24);
        r6 = (u16)(*(volatile u32 *)(r7 + 24) + 64);
    }
    u8 *obj = (u8 *)_08002BFC(0);
    *(volatile u16 *)(obj + 0) = (u16)(r9 | *(volatile u16 *)(sl + 8));
    *(volatile u16 *)(obj + 2) = (u16)(((u32)(*(volatile u32 *)(r7 + 0) - *(volatile u16 *)(sl + 0)) & 0x1FFu)
                                       | ((u32)(*(volatile u32 *)(r7 + 20)) << 9) | 0xC000u);
    *(volatile u16 *)(obj + 4) = (u16)(r6 | flag | *(volatile u16 *)(r7 + 28));
    *(volatile u16 *)(obj + 12) = 0;
    _08002C34((int)*(volatile u32 *)(r7 + 8), obj);
    u8 *obj2 = (u8 *)_08002BFC(0);
    *(volatile u16 *)(obj2 + 0) = (u16)(r9 | *(volatile u16 *)(sl + 8));
    *(volatile u16 *)(obj2 + 2) = (u16)(((u32)(*(volatile u32 *)(r7 + 0) - *(volatile u16 *)(sl + 2)) & 0x1FFu)
                                        | ((u32)(*(volatile u32 *)(r7 + 20)) << 9) | 0xC000u);
    *(volatile u16 *)(obj2 + 4) = (u16)(flag | sp0 | *(volatile u16 *)(r7 + 28));
    *(volatile u16 *)(obj2 + 12) = 0;
    _08002C34((int)*(volatile u32 *)(r7 + 8), obj2);
}
#ifndef __APPLE__
void _080026554(void *a, int b, void *c) __attribute__((alias("Sprite_PlacePair")));
void sub_080026554(void *a, int b, void *c) __attribute__((alias("Sprite_PlacePair")));
#endif

// 0x08026550 shares the same pair-emitter body, with one ignored r3 input.
void Sprite_PlacePairEntry_26550(void *rec, int x, void *spr, int unused)
{
    (void)unused;
    Sprite_PlacePair(rec, x, spr);
}
#ifndef __APPLE__
void _08026550(void *rec, int x, void *spr, int unused)
    __attribute__((alias("Sprite_PlacePairEntry_26550")));
#endif

// ============================================================================
// sub_080026674 (0x08026674, 0x188 B) — 9-arg emitter pair (race HUD lanes).
//   (rec, off, z, y, a4, a5, a6, a7, a8):
//     r7 = (u16)a4; sl = (u16)(a4+64); flag = WA_10BC ? 2048 : 1024
//     off >>= (word[arr] & 0xFF);  if probe byte bit7: _08002C84(a8) and swap
//     yv = s16((a7 + (a7<0?15:0)) << 12) truncated, zeroed when |s16| <= 4
//     if (yv != 0): _08002C74(a8, yv); pack = (u16)(-yv/10) and (u16)(yv/10 +
//       (yv/5)%2) -> drives both attrs
//     obj1: attr0 = (y - (s16)pack8) & 0xFF | 0x100
//           attr2 = (z - 56) & 0x1FF | 0xC000 | a8<<9
//           attr4 = r7 | a5 | flag
//     obj2: attr0 = (y - (s16)pack16) & 0xFF | 0x100
//           attr2 = (z - 8) & 0x1FF | 0xC000 | a8<<9
//           attr4 = sl | a5 | flag
//   both linked with idx = a6.
// ============================================================================
void Sprite_PlaceEmitters(void *rec, int off, int z, int y,
                          int a4, int a5, int a6, int a7, int a8) {
    u16 r7 = (u16)a4;
    u16 sl = (u16)(a4 + 64);
    u16 r5 = sl;
    void *record = _08007498(CREC_TABLE0, (int)*(volatile u32 *)(rec));
    void *arr = _0800748C(record);
    int r6 = (int)((u32)off & 0xFFFu);
    r6 = asr_low8(r6, arr);
    void *arr2 = _0800748C(_08007498(record, 0));
    int flag = WA_10BC ? 2048 : 1024;
    if ((*(volatile u8 *)((u8 *)arr2 + r6) & 128u) != 0) {
        _08002C84(a8);
        sl = r7;
        r7 = r5;
    }
    int t = a7;
    if (t < 0) t += 15;
    u32 shifted = (u32)t << 12;
    u16 r4 = (u16)(shifted >> 16);
    int h = (int)shifted >> 16;             // asr 16
    if (!(h < -4 || h > 4)) r4 = 0;
    s16 yv = (s16)r4;
    u16 pack8 = 0, pack16 = 0;
    if (yv != 0) {
        _08002C74(a8, (u16)yv);
        pack8 = (u16)_0802D978(-(int)yv, 10);
        pack16 = (u16)(_0802D978((int)yv, 10) + _0802D97C(_0802D978((int)yv, 5), 2));
    }
    u8 *obj = (u8 *)_08002BFC(0);
    *(volatile u16 *)(obj + 0) = (u16)(((y - (s16)pack8) & 0xFF) | 0x100);
    *(volatile u16 *)(obj + 2) = (u16)(((u32)(z - 56) & 0x1FFu) | 0xC000u | ((u32)a8 << 9));
    *(volatile u16 *)(obj + 4) = (u16)(r7 | (u16)a5 | flag);
    *(volatile u16 *)(obj + 12) = 0;
    _08002C34(a6, obj);
    u8 *obj2 = (u8 *)_08002BFC(0);
    *(volatile u16 *)(obj2 + 0) = (u16)(((y - (s16)pack16) & 0xFF) | 0x100);
    *(volatile u16 *)(obj2 + 2) = (u16)(((u32)(z - 8) & 0x1FFu) | 0xC000u | ((u32)a8 << 9));
    *(volatile u16 *)(obj2 + 4) = (u16)(sl | (u16)a5 | flag);
    *(volatile u16 *)(obj2 + 12) = 0;
    _08002C34(a6, obj2);
}
#ifndef __APPLE__
void _080026674(void *a, int b, int c, int d, int e, int f, int g, int h, int i)
    __attribute__((alias("Sprite_PlaceEmitters")));
void sub_080026674(void *a, int b, int c, int d, int e, int f, int g, int h, int i)
    __attribute__((alias("Sprite_PlaceEmitters")));
#endif

// ============================================================================
// sub_0800267FC (0x080267FC, 0x6C B = 108 B) — course byte lookup variant.
//
// ROM CHARACTERISATION (read off asm/code_263e0.s,. In one
// sentence: look up a signed byte in the table at 0x080CDA74 indexed by arg0,
// bail out with 0 if it is -1, resolve two course records into array bases,
// use arg1 as a 12-bit offset shifted right by a per-course shift byte to
// probe a tile byte, and return the TABLE-1 array base advanced by that tile
// index scaled by 1024.
//
//   s8   b  = s8[0x080CDA74 + arg0];      if (b == -1) return 0;
//   void *t1 = ArrBase(Seek(0x0879A304, b));
//   void *rec6 = Seek(0x087999D8, 6);
//   off  = (arg1 & 0xFFF) >> (low byte of word[ArrBase(rec6)]);
//   tile = u8[ArrBase(Seek(rec6, 0)) + off] & 127;
//   return t1 + tile * 1024;
//
// Register roles the ROM holds (this is the part that is hard to guess):
//   r4 = rec6          -- lives across three calls
//   r5 = t1            -- the return accumulator, lives to the end
//   r6 = arg1          -- copied in by the FIRST post-prologue instruction
//                         `adds r6, r1, #0` and then mutated IN PLACE by
//                         `ands r6, #0xFFF` and `asrs r6, r0`, finally
//                         consumed by `adds r0, r0, r6`
//   r1 = pure scratch, reused four times: the 0x080CDA74 literal, then the
//         zero index of an INDEXED `ldrsb r1,[r2,r1]`, then the literal 6,
//         then the 0x0FFF mask, then the literal 127
//   r2 = the address base of the first table load
// Prologue is `push {r4,r5,r6,lr}` (three callee-saved, no r7) and the -1
// early-out `movs r0,#0` sits INSIDE the span, after the literal pool, with
// both epilogue `pop`s shared by the two exits.
//
// SHAPE DECISION AT THE FIRST DIFFERENCE (+0x6, 35/108): the ROM materialises
// the address into r2 (`adds r2, r0, r1`) so r0 stays free to die two
// instructions later at `movs r0,#1`, whereas agbcc folds the same address
// destructively into the incoming argument (`add r0, r0, r1`). The reason is
// the load shape that follows: the ROM's INDEXED
// `movs r1,#0 / ldrsb r1,[r2,r1]` needs a dedicated base register, and agbcc
// emits register-DIRECT `ldrb r0,[r0]` plus an explicit widening
// `lsls #24 / asrs #24` instead (+2 bytes, paid for by the unpinned version
// carrying one extra pushed register).
// ============================================================================
void *Sprite_CourseByte2(int a, u32 off) {
    // Register pin (measured : the ROM copies arg1 straight into
    // r6 as its first post-prologue instruction (`adds r6, r1, #0`) and keeps
    // that ONE register for the rest of the body -- `ands r6, #0xFFF` and
    // `asrs r6, r0` both mutate it in place, and it is finally consumed by
    // `adds r0, r0, r6`. Left unpinned agbcc allocates arg1 to r7, which costs
    // the prologue one extra push (`push {r4,r5,r6,r7,lr}` vs the ROM's
    // `push {r4,r5,r6,lr}`) and pushes the &0xFFF literal pool load out of the
    // caller-saved r1 into a callee-saved r4. Pinning the OFFSET -- the value
    // that has to survive every call -- is the same lever as _0800264D8: pin
    // the value that outlives the calls, not the result.
    register u32 o6 __asm__("r6") = off;
    // MEASURED NEGATIVE : pinning the address base to r2 and a
    // zero index to r1 does NOT produce the ROM's indexed
    // `movs r1,#0 / ldrsb r1,[r2,r1]`. agbcc constant-folds both pins away
    // before local allocation and still emits `add r0,r0,r1 / ldrb r0,[r0]`
    // plus the `lsls #24 / asrs #24` widening -- byte-identical to no pin at
    // all (35/108 either way, first difference still +0x6). The +2 bytes that
    // widening costs are NOT recoverable by pinning.
    s8 b = *(volatile s8 *)(uintptr_t)(0x080CDA74u + (u32)a);
    if (b == -1) return (void *)0;
    void *r5 = _0800748C(_08007498(CREC_TABLE1, (int)b));
    void *rec6 = _08007498(CREC_TABLE0, 6);
    void *arr = _0800748C(rec6);
    o6 &= 0xFFFu;
    o6 = asr_low8(o6, arr);
    void *arr2 = _0800748C(_08007498(rec6, 0));
    u32 tile = 127u & (u32)*(volatile u8 *)((u8 *)arr2 + o6);
    return (void *)((u8 *)r5 + (tile << 10));
}
#ifndef __APPLE__
void *_0800267FC(int a, u32 b) __attribute__((alias("Sprite_CourseByte2")));
void *sub_0800267FC(int a, u32 b) __attribute__((alias("Sprite_CourseByte2")));
void *Course_0x080267FC(int a, u32 b) __attribute__((alias("Sprite_CourseByte2"))); /* trampoline elimination: friendly-name spelling used by race_setup.c */
#endif

// ============================================================================
// sub_080026868 (0x08026868, 0x18 B) — palette upload:
//   CpuFastSet(dst, 0x06010000 + bank*32, 256)
// ============================================================================
void Sprite_CopyPal(u8 *dst, int bank) {
    _0802D970(dst, (void *)(uintptr_t)(0x06010000u + ((u32)bank << 5)), 256);
}
#ifndef __APPLE__
void _080026868(u8 *a, int b) __attribute__((alias("Sprite_CopyPal")));
void sub_080026868(u8 *a, int b) __attribute__((alias("Sprite_CopyPal")));
#endif

// ============================================================================
// sub_080026880 (0x08026880, 0x18 B, unlabeled) — ArrBase(Seek(0x0879A304, 3)).
// ============================================================================
void *Sprite_CourseRecord3(void) {
    return _0800748C(_08007498(CREC_TABLE1, 3));
}
#ifndef __APPLE__
void *_080026880(void) __attribute__((alias("Sprite_CourseRecord3")));
void *sub_080026880(void) __attribute__((alias("Sprite_CourseRecord3")));
#endif

// ============================================================================
// sub_080026898 (0x08026898, 0x9C B) — 9-arg emitter pair (r0/r1 unused by
//   the callee; caller 0x08026A78 passes [rec+92], s16[rec+8]).
//   (unused0, unused1, x, y, a4, a5, a6, a7, a8):
//     flag = (a7 | -a7) >> 31 & 0x400           (a7 != 0 ? 0x400 : 0)
//     obj1: attr0 = (y & 0xFF) | flag;  attr2 = ((x-56) & 0x1FF) | 0xC000
//           attr4 = a4 | (a5<<12) | (a8<<10)
//     obj2: attr0 same;  attr2 = ((x-8) & 0x1FF) | 0xC000
//           attr4 = (a4+64) | (a5<<12) | (a8<<10)
//   both linked with idx = a6.
// ============================================================================
void Sprite_EmitPair(void *unused0, int unused1, int x, int y,
                     u32 a4, u32 a5, u32 a6, u32 a7, u32 a8) {
    (void)unused0; (void)unused1;
    int r5 = x;
    u32 r8 = y;
    u32 flag = (u32)(((a7 | (u32)(-(s32)a7)) >> 31) & 0x400u);
    u8 *obj = (u8 *)_08002BFC(0);
    *(volatile u16 *)(obj + 0) = (u16)((r8 & 0xFFu) | flag);
    *(volatile u16 *)(obj + 2) = (u16)(((u32)(r5 - 56) & 0x1FFu) | 0xC000u);
    *(volatile u16 *)(obj + 4) = (u16)((a4 & 0xFFFFu) | (a5 << 12) | (a8 << 10));
    *(volatile u16 *)(obj + 12) = 0;
    _08002C34((int)a6, obj);
    u8 *obj2 = (u8 *)_08002BFC(0);
    *(volatile u16 *)(obj2 + 0) = (u16)((r8 & 0xFFu) | flag);
    *(volatile u16 *)(obj2 + 2) = (u16)(((u32)(r5 - 8) & 0x1FFu) | 0xC000u);
    *(volatile u16 *)(obj2 + 4) = (u16)(((a4 + 64) & 0xFFFFu) | (a5 << 12) | (a8 << 10));
    *(volatile u16 *)(obj2 + 12) = 0;
    _08002C34((int)a6, obj2);
}
#ifndef __APPLE__
void _080026898(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i)
    __attribute__((alias("Sprite_EmitPair")));
void sub_080026898(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i)
    __attribute__((alias("Sprite_EmitPair")));
#endif

// ============================================================================
// sub_080026938 (0x08026938, 0x10 B) + entry 0x0802693C — resource unlink:
//   0x08026938(mgr, slot): entry = u32[ [mgr+8] + 92 ]; _080262A4(entry, slot)
//   0x0802693C(entry, slot): _080262A4([entry+92], slot)
// ============================================================================
void Sprite_UnlinkSlot(void *mgr) {
    void *x = *(void **)((u8 *)mgr + 8);
    sub_080262A4(*(void **)((u8 *)x + 92));
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08026938(void *a) __attribute__((alias("Sprite_UnlinkSlot")));
void sub_08026938(void *a) __attribute__((alias("Sprite_UnlinkSlot")));
void _080026938(void *a) __attribute__((alias("Sprite_UnlinkSlot")));
void sub_080026938(void *a) __attribute__((alias("Sprite_UnlinkSlot")));
#endif

// ============================================================================
// Sprite record factory. The ROM carries this sequence INLINE in each of the
// two factories -- there is no shared ROM body to call, so a C helper forces
// a `bl` to a local `t` symbol that cannot resolve at probe time:
//   rec = [node + 8];  [rec+18] = 0;  [rec+0] = (u16)alloc(160)
//   _080057A4(u16[rec+0], 160);  [rec+2] = (u16)freeHigh(1)
//   [rec+10] = 0; [rec+16] = 1; [rec+17] = 0
//   _0802620C(rec+20, 0x0202AC00); _08026220
//   [rec+92] = _08026230(0, 0)
// The two spellings differ where the ROM does: 0x08026948 zeroes [rec+10]
// from the r5 slot it already holds and never writes [rec+8], while
// 0x080269AC masks arg0 to a halfword up front (lsls/lsrs #16 into the
// callee-saved r5) and appends the [rec+8] / [rec+18] stores AFTER the
// shared prologue rather than inside it.
// ============================================================================

// ============================================================================
// sub_080026948 (0x08026948, 0x64 B = 100 B) — sprite factory #1.
//
// ROM CHARACTERISATION (read off asm/code_263e0.s,. In one
// sentence: allocate a sprite record node, initialise its 160-byte payload
// block and flag bytes, bind a DMA3 descriptor and a resource, and return the
// node.
//   node = _08004E0C(0x080CDAD8);  rec = [node + 8]
//   [rec+18] = 0
//   [rec+0]  = (u16)_08005758(160);  _080057A4(u16[rec+0], 160)
//   [rec+2]  = (u16)_08005790(1)
//   [rec+10] = 0;  [rec+16] = 1;  [rec+17] = 0
//   _0802620C(rec + 20, 0x0202AC00);  _08026220
//   [rec+92] = _08026230(0, 0);  return node
//
// Register roles the ROM holds (the part that is hard to guess):
//   r4 = rec
//   r8 = node -- held for the WHOLE body, so the prologue pays
//        `mov r6, r8 / push {r6}` and the epilogue `pop {r3} / mov r8,r3`.
//        That is 8 bytes the unpinned lift simply did not have (92 -> 100).
//   r5 = the zero used at BOTH [rec+18] and [rec+10], i.e. a zero constant
//        hoisted across three calls (`movs r5,#0` / `strb r5,[r4,#18]`...
//        `strh r5,[r4,#10]`).
//   r6 = a SECOND, distinct zero, materialised by `movs r6,#0` immediately
//        after the _08005758 call and used once at [rec+17]. The ROM does not
//        share r5's zero there; agbcc originally shared one caller-saved
//        register for both, which is the whole 8-byte deficit.
// Prologue `push {r4,r5,r6,lr}` + separate r8 push; epilogue restores r8
// first, then pops {r4,r5,r6}, then `pop {r1} / bx r1`.
void *Sprite_Create1(void) {
    void *node = _08004E0C((void *)(uintptr_t)0x080CDAD8);
    u8 *rec = (u8 *)(uintptr_t)*(volatile u32 *)((u8 *)node + 8);
    u16 z18;
    register u8 z17 __asm__("r6");
    z18 = 0;
    *(volatile u8 *)(rec + 18) = z18;
    {
        u32 a0 = (u32)_08005758(160);
        z17 = 0;
        *(volatile u16 *)(rec + 0) = (u16)a0;
    }
    sub_080057A4((u32)*(volatile u16 *)(rec + 0), 160);
    *(volatile u16 *)(rec + 2) = (u16)_08005790(1);
    *(volatile u16 *)(rec + 10) = z18;
    *(volatile u8 *)(rec + 16) = 1;
    *(volatile u8 *)(rec + 17) = z17;
    _0802620C(rec + 20, (void *)(uintptr_t)0x0202AC00);
#ifndef __APPLE__
    _08026220();
#else
    sub_08026220();      // host stub spelling
#endif
    *(volatile u32 *)(rec + 92) = (u32)(uintptr_t)sub_08026230(0, 0);
    return node;
}
#ifndef __APPLE__
void *_08026948(void) __attribute__((alias("Sprite_Create1")));
void *sub_08026948(void) __attribute__((alias("Sprite_Create1")));
void *_080026948(void) __attribute__((alias("Sprite_Create1")));
void *sub_080026948(void) __attribute__((alias("Sprite_Create1")));
#endif

// ============================================================================
// sub_0800269AC (0x080269AC, 0x74 B = 116 B, unlabeled) — sprite factory #2.
//
// ROM CHARACTERISATION (read off asm/code_263e0.s,. Same record
// initialisation as 0x08026948, plus an extra halfword field: take arg0,
// truncate it to a halfword, store it at [rec+8], and set [rec+18] to 1
// AFTER the allocation calls instead of leaving it 0.
//   extra = (u16)arg0                      -- masked UP FRONT, see registers
//   node  = _08004E0C(0x080CDAD8);  rec = [node + 8]
//   [rec+18] = 0;  [rec+0] = (u16)_08005758(160);
//   _080057A4(u16[rec+0], 160);  [rec+2] = (u16)_08005790(1)
//   [rec+10] = 0;  [rec+16] = 1;  [rec+17] = 0
//   [rec+8] = extra;  [rec+18] = 1
//   _0802620C(rec + 20, 0x0202AC00);  _08026220
//   [rec+92] = _08026230(0, 0);  return node
//
// Register roles the ROM holds:
//   r4 = rec
//   r9 = node -- held for the whole body, so the prologue pays
//        `mov r6,r9 / mov r5,r8 / push {r5,r6}` and the epilogue
//        `pop {r3,r4} / mov r8,r3 / mov r9,r4` (two saved registers, 12
//        bytes the unpinned lift did not have: 100 -> 116).
//   r5 = extra, ALREADY halfword-masked at the top of the body
//        (`adds r5,r0,#0 / lsls r5,#16 / lsrs r5,#16`). The mask is dead for
//        the `strh` that consumes it, so agbcc drops it and saves 4 bytes;
//        keeping it costs one live callee-saved register that r9/r8 then
//        need, which is the only reason the ROM has them.
//   r6 = the zero used at BOTH [rec+18] and [rec+10]
//   r8 = the SECOND zero, used once at [rec+17]. Thumb-1 cannot `movs r8,#0`
//        (the immediate form only reaches r0-r7), so the ROM materialises the
//        constant in r1 and copies it in: `movs r1,#0 / mov r8,r1` before
//        `strh r0,[r4,#0]`, then fetches it back out at `mov r1,r8` just
//        before `strb r1,[r4,#17]`. That copy-out is what distinguishes this
//        body from 0x08026948, where the same zero needs no copy because it
//        already lives in r6.
//   r0 = the literal 1 for [rec+16] stays live until `strb r0,[r4,#18]`,
//        i.e. the trailing [rec+18]=1 reuses the earlier 1 rather than
//        materialising a third constant.
void *Sprite_Create2(u32 v) {
    u16 extra = (u16)v;
    void *node = _08004E0C((void *)(uintptr_t)0x080CDAD8);
    register void *n9 __asm__("r9") = node;
    u8 *rec = (u8 *)(uintptr_t)*(volatile u32 *)((u8 *)node + 8);
    u16 z18;
    register u8 z17 __asm__("r8");
    z18 = 0;
    *(volatile u8 *)(rec + 18) = z18;
    {
        u32 a0 = (u32)_08005758(160);
        z17 = 0;
        *(volatile u16 *)(rec + 0) = (u16)a0;
    }
    sub_080057A4((u32)*(volatile u16 *)(rec + 0), 160);
    *(volatile u16 *)(rec + 2) = (u16)_08005790(1);
    *(volatile u16 *)(rec + 10) = z18;
    *(volatile u8 *)(rec + 16) = 1;
    *(volatile u8 *)(rec + 17) = z17;
    *(volatile u16 *)(rec + 8) = extra;
    *(volatile u8 *)(rec + 18) = 1;
    _0802620C(rec + 20, (void *)(uintptr_t)0x0202AC00);
#ifndef __APPLE__
    _08026220();
#else
    sub_08026220();      // host stub spelling
#endif
    *(volatile u32 *)(rec + 92) = (u32)(uintptr_t)sub_08026230(0, 0);
    return n9;
}
#ifndef __APPLE__
void *_080269AC(u32 a) __attribute__((alias("Sprite_Create2")));
void *sub_080269AC(u32 a) __attribute__((alias("Sprite_Create2")));
void *_0800269AC(u32 a) __attribute__((alias("Sprite_Create2")));
#endif

// ============================================================================
// sub_080026A20 (0x08026A20, 0xC B, unlabeled) — [mgr+8]+4 = x; +6 = y.
// ============================================================================
// Same shape as the promoted 0x08026A4C twin below, only the two store
// offsets differ: two independent volatile loads of [mgr+8] (a shared `rec`
// local lets agbcc keep the pointer in one register and drops the second
// `ldr`), and word-width params (u16 params cost an `lsls/lsrs` pair).
void Sprite_SetXY(void *mgr, u32 x, u32 y) {
    *(volatile u16 *)((uintptr_t)*(volatile u32 *)((u8 *)mgr + 8) + 4) = (u16)x;
    *(volatile u16 *)((uintptr_t)*(volatile u32 *)((u8 *)mgr + 8) + 6) = (u16)y;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08026A20(void *a, u32 b, u32 c) __attribute__((alias("Sprite_SetXY")));
void sub_08026A20(void *a, u32 b, u32 c) __attribute__((alias("Sprite_SetXY")));
// The closure spells 0x080026A20 with the 9-digit form; the 8-digit alias
// above denotes the same address but is a different symbol, so a promoted
// caller using the closure spelling stayed undefined and got a ROM trampoline.
void _080026A20(void *a, u32 b, u32 c) __attribute__((alias("Sprite_SetXY")));
#endif

// ============================================================================
// sub_080026A2C (0x08026A2C, 0x1C B) — [mgr+8]+4 = x - (s16)u16[0x030035C0+12]>>3;
// [mgr+8]+6 = y.
// ============================================================================
// Word-width params: the ROM stores r1/r2 untouched (`subs r1,r1,r3` /
// `strh r2`), so any narrower spelling adds an `lsls/lsrs` pair. Two
// independent volatile loads of [mgr+8]: a shared `rec` local lets agbcc keep
// the pointer and drops the ROM's second `ldr r0,[r0,#8]` (same shape as the
// Sprite_SetXY / Sprite_SetTenTwelve twins). The keypad word stays
// base+displacement via SPR_KP_W12 so the pool holds 0x030035C0 with
// `ldrh r3,[r3,#12]`, and the >>3 is the fused `lsls #16 / asrs #19` pair.
void Sprite_SetXYRel(void *mgr, u32 x, u32 y) {
    register u8 *rec __asm__("r4") = (u8 *)(uintptr_t)*(volatile u32 *)((u8 *)mgr + 8);
    register u32 k __asm__("r3") = SPR_KP_W12;
    k = (u32)((s32)(k << 16) >> 19);
    x -= k;
    *(volatile u16 *)(rec + 4) = (u16)x;
    *(volatile u16 *)((uintptr_t)*(volatile u32 *)((u8 *)mgr + 8) + 6) = (u16)y;
}
#ifndef __APPLE__
void _080026A2C(void *a, u32 b, u32 c) __attribute__((alias("Sprite_SetXYRel")));
void sub_080026A2C(void *a, u32 b, u32 c) __attribute__((alias("Sprite_SetXYRel")));
#endif

// ============================================================================
// sub_080026A4C (0x08026A4C, 0xC B, unlabeled) — [mgr+8]+10 = a; +12 = b.
// ============================================================================
// The ROM re-reads [mgr+8] for the second store, so these are two independent
// volatile loads and not a shared `rec` local: a common local would let agbcc
// keep the pointer in one register. Params are declared at word width because
// a narrow parameter costs a `lsls/lsrs` zero-extension pair (16 -> 12 B) that
// the ROM does not have.
void Sprite_SetTenTwelve(void *mgr, u32 a, u32 b) {
    *(volatile u16 *)((uintptr_t)*(volatile u32 *)((u8 *)mgr + 8) + 10) = (u16)a;
    *(volatile u16 *)((uintptr_t)*(volatile u32 *)((u8 *)mgr + 8) + 12) = (u16)b;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08026A4C(void *a, u32 b, u32 c) __attribute__((alias("Sprite_SetTenTwelve")));
void sub_08026A4C(void *a, u32 b, u32 c) __attribute__((alias("Sprite_SetTenTwelve")));
#endif

void Sprite_SetFlag16(void *mgr, u32 v) {
    *(volatile u8 *)((uintptr_t)*(volatile u32 *)((u8 *)mgr + 8) + 16) = (u8)v;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08026A58(void *a, u32 b) __attribute__((alias("Sprite_SetFlag16")));
void sub_08026A58(void *a, u32 b) __attribute__((alias("Sprite_SetFlag16")));
// The closure spells 0x080026A58 with the 9-digit form, and asm/code_263e0.s
// defines that label on this span; without it the splice emits the label with
// no C definition. Same reason as the 9-digit alias on Sprite_SetXY above.
void _080026A58(void *a, u32 b) __attribute__((alias("Sprite_SetFlag16")));
#endif

void Sprite_SetFlag17(void *mgr, u32 v) {
    *(volatile u8 *)((uintptr_t)*(volatile u32 *)((u8 *)mgr + 8) + 17) = (u8)v;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08026A60(void *a, u32 b) __attribute__((alias("Sprite_SetFlag17")));
void sub_08026A60(void *a, u32 b) __attribute__((alias("Sprite_SetFlag17")));
// 9-digit closure spelling of 0x080026A60; see the note on Sprite_SetFlag16.
void _080026A60(void *a, u32 b) __attribute__((alias("Sprite_SetFlag17")));
#endif

// ============================================================================
// sub_080026A68 (0x08026A68, 0x10 B, unlabeled) — if (u8[rec+18] == 0)
// u16[rec+8] += 64.
// ============================================================================
void Sprite_Tick(void *rec) {
    if (*(volatile u8 *)((u8 *)rec + 18) == 0) {
        *(volatile u16 *)((u8 *)rec + 8) = (u16)(*(volatile u16 *)((u8 *)rec + 8) + 64);
    }
}
#ifndef __APPLE__
void _08026A68(void *a) __attribute__((alias("Sprite_Tick")));
void sub_08026A68(void *a) __attribute__((alias("Sprite_Tick")));
// 9-digit closure spelling of 0x08026A68: asm/code_263e0.s defines only
// `_080026A68` on this span (no `sub_` twin), and Sprite_Event below must call
// the closure spelling so the spliced link resolves. Same one-hop pattern as
// the 9-digit aliases on Sprite_SetFlag16/17.
void _080026A68(void *a) __attribute__((alias("Sprite_Tick")));
#endif

// ============================================================================
// sub_080026A78 (0x08026A78, 0x3C B) — if (u8[rec+16] != 0):
//   EmitPair(u32[rec+92], s16[rec+8], s16[rec+4], s16[rec+6],
//            u16[rec+0], u16[rec+2], 2, u8[rec+17], 1)
// ============================================================================
void Sprite_Draw(void *rec) {
    if (*(volatile u8 *)((u8 *)rec + 16) != 0) {
        sub_080026898((void *)(uintptr_t)*(volatile u32 *)((u8 *)rec + 92),
                        (int)((s16 *)rec)[4],
                        (int)((s16 *)rec)[2],
                        (int)((s16 *)rec)[3],
                        *(volatile u16 *)((u8 *)rec + 0),
                        *(volatile u16 *)((u8 *)rec + 2),
                        2, *(volatile u8 *)((u8 *)rec + 17), 1);
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08026A78(void *a) __attribute__((alias("Sprite_Draw")));
void sub_08026A78(void *a) __attribute__((alias("Sprite_Draw")));
// 9-digit closure spelling of 0x08026A78. asm/code_263e0.s:806 spells this
// VMA's only labels `sub_080026A78` / `_080026A78` and the switch's `bl` at
// line 896 calls the 9-digit `sub_` twin, so Sprite_Event below must bind that
// one. Before this alias existed the call was an implicit declaration: with
// agbcc the assembled body was byte-identical anyway (int and pointer are both
// 32-bit here), but the C89-equivalence gate compiles with modern
// arm-none-eabi-gcc, where an implicit declaration is a hard error --
// `make matching-ready` failed 151/152 there and nowhere else.
void _080026A78(void *a) __attribute__((alias("Sprite_Draw")));
void sub_080026A78(void *a) __attribute__((alias("Sprite_Draw")));
#endif

// ============================================================================
// sub_080026AB4 (0x08026AB4, 0x48 B) — paint: course tile + palette upload.
//   r1 = ((s16[rec+8] < 0 ? s16+255 : s16) >> 8) << 7
//   base = CourseByte(u32[rec+92], r1); tile = CourseSurface(s16[rec+10],
//   s16[rec+12]); BlitTiles(base, u16[rec+0]);
//   CpuFastSet(tile, 0x05000200 + u16[rec+2]*32, 8)
// ============================================================================
void Sprite_Paint(void *rec) {
    // Sprite records are ordinary RAM. Signed s16 lvalues let agbcc use the
    // ROM's ldrsh instructions; volatile signed halfword reads expand instead.
    int v = *(s16 *)((u8 *)rec + 8);
    if (v < 0) v += 255;
    int r1 = v >> 8;
    r1 <<= 7;
#ifndef __APPLE__
    // The ROM keeps the two call results in r5 and r4 across the next calls.
    register void *base __asm__("r5") =
        Sprite_CourseByte((void *)(uintptr_t)*(volatile u32 *)((u8 *)rec + 92), (u32)r1);
    register void *tile __asm__("r4") =
        Sprite_CourseSurface(*(s16 *)((u8 *)rec + 10),
                             *(s16 *)((u8 *)rec + 12));
#else
    void *base = Sprite_CourseByte((void *)(uintptr_t)*(volatile u32 *)((u8 *)rec + 92), (u32)r1);
    void *tile = Sprite_CourseSurface(*(s16 *)((u8 *)rec + 10),
                                      *(s16 *)((u8 *)rec + 12));
#endif
    sub_0800263E0((u8 *)base, (int)*(volatile u16 *)((u8 *)rec + 0));
#ifndef __APPLE__
    // ROM loads the final index into r6, then shifts into r1. The empty asm
    // keeps agbcc from folding the load directly into the shifted r1 value.
    register u32 rawIndex __asm__("r6") = *(volatile u16 *)((u8 *)rec + 2);
    __asm__("" : "+r" (rawIndex));
    register u32 p __asm__("r1") = rawIndex << 5;
#else
    u32 p = (u32)*(volatile u16 *)((u8 *)rec + 2) << 5;
#endif
    _0802D970(tile, (void *)(uintptr_t)(0x05000200u + p), 8);
}
#ifndef __APPLE__
void _08026AB4(void *a) __attribute__((alias("Sprite_Paint")));
void sub_08026AB4(void *a) __attribute__((alias("Sprite_Paint")));
// 9-digit closure spelling of 0x08026AB4 — same drift and same reason as the
// Sprite_Draw pair above: asm/code_263e0.s:839-841 defines `sub_080026AB4` /
// `_080026AB4`, its `bl` at line 900 calls the 9-digit `sub_` twin, and
// Sprite_Event below binds it. agbcc took the implicit declaration silently;
// modern arm-none-eabi-gcc (the C89-equivalence gate) rejects it.
void _080026AB4(void *a) __attribute__((alias("Sprite_Paint")));
void sub_080026AB4(void *a) __attribute__((alias("Sprite_Paint")));
#endif

void Sprite_Event(int ev, int a, int b, void *rec) {
    (void)a; (void)b;
    switch ((u32)ev) {
    case 5:
#ifndef __APPLE__
        // Closure spellings: the spliced link holds asm/code.s plus this one
        // body and no C object, so the friendly names have nothing to bind
        // to. The host arms call the real bodies, preserving host behavior.
        _080026A68(rec);
#else
        Sprite_Tick(rec);
#endif
        break;
    case 7:
#ifndef __APPLE__
        sub_080026A78(rec);
#else
        Sprite_Draw(rec);
#endif
        break;
    case 8:
#ifndef __APPLE__
        sub_080026AB4(rec);
#else
        Sprite_Paint(rec);
#endif
        break;
    }
}
#ifndef __APPLE__
void _08026B00(int a, int b, int c, void *d) __attribute__((alias("Sprite_Event")));
void sub_08026B00(int a, int b, int c, void *d) __attribute__((alias("Sprite_Event")));
#endif

// ============================================================================
// sub_080026B30 (0x08026B30, 0x50 B) — menu-record init:
//   [0x03001674] = rec;  CpuSet-fill rec[0..60) = 0 (ctrl 0x0500000F)
//   [rec+4] = 0x080261B0(12); [rec+8] = 0x080261F8(12)
//   [rec+6] = 0x080261B0(13); [rec+10] = 0x080261F8(13)
// ============================================================================
void Sprite_Init(void *rec) {
    SPR_MENU_REC = rec;
    u32 zero = 0;
    _0802D974(&zero, rec, 0x0500000Fu);
    // The ROM reloads the shared record slot after each helper call.
    *(volatile u16 *)((u8 *)(uintptr_t)SPR_MENU_REC + 4) = (u16)_080261B0(12);
    *(volatile u16 *)((u8 *)(uintptr_t)SPR_MENU_REC + 8) = (u16)_080261F8(12);
    *(volatile u16 *)((u8 *)(uintptr_t)SPR_MENU_REC + 6) = (u16)_080261B0(13);
    *(volatile u16 *)((u8 *)(uintptr_t)SPR_MENU_REC + 10) = (u16)_080261F8(13);
}
#ifndef __APPLE__
void _08026B30(void *a) __attribute__((alias("Sprite_Init")));
void sub_08026B30(void *a) __attribute__((alias("Sprite_Init")));
#endif

// ============================================================================
// sub_080026B80 (0x08026B80, 0x104 B, unlabeled) — menu-record event handler.
//   rec = [0x03001674];  entries are 20-byte slots: rec + cur*20 with fields
//   +12 (timer), +18 (event id), +20 (x), +22 (y), +30 (armed), +50 (armed2);
//   rec+4 / rec+8 hold 2-byte-per-entry x/y templates.
//   ev 8: [rec+2]=1, clear +30/+12/+50/+32 and return.
//   else: [rec+2]=0; if s16[entry+30] != 0: [entry+12]=12 and flip cur;
//   entry = rec + cur*20; [entry+30]=1; [entry+12]=120; [entry+18]=ev;
//   [entry+20] = u16[rec+4+cur*2]; [entry+22] = u16[rec+8+cur*2];
//   _08007538(0x083A2100, table1[ev], s16[entry+20]);
//   _080075E8(0x083A2100, table2[ev], s16[entry+22]).
// ============================================================================
void Sprite_MenuEvent(int ev) {
    u8 *rec = (u8 *)(uintptr_t)SPR_MENU_REC;
    if (ev == 8) {
        *(volatile u16 *)(rec + 2) = 1;
        *(volatile u16 *)(rec + 30) = 0;
        *(volatile u16 *)(rec + 12) = 0;
        *(volatile u16 *)(rec + 50) = 0;
        *(volatile u16 *)(rec + 32) = 0;
        return;
    }
    *(volatile u16 *)(rec + 2) = 0;
    int cur = (s16)*(volatile u16 *)(rec + 0);
    u8 *entry = rec + cur * 20;
    if ((s16)*(volatile u16 *)(entry + 30) != 0) {
        *(volatile u16 *)(entry + 12) = 12;
        *(volatile u16 *)(rec + 0) = (u16)(cur == 0 ? 1 : 0);
    }
    cur = (s16)*(volatile u16 *)(rec + 0);
    entry = rec + cur * 20;
    *(volatile u16 *)(entry + 30) = 1;
    *(volatile u16 *)(entry + 12) = 120;
    *(volatile u16 *)(entry + 18) = (u16)ev;
    *(volatile u16 *)(entry + 20) = *(volatile u16 *)(rec + 4 + cur * 2);
    *(volatile u16 *)(entry + 22) = *(volatile u16 *)(rec + 8 + cur * 2);
    u32 t1 = *(volatile u32 *)(uintptr_t)(0x080CDAE0u + (u32)ev * 4);
    _08007538((void *)(uintptr_t)0x083A2100, (int)t1,
              (void *)(uintptr_t)(s16)*(volatile u16 *)(entry + 20));
    u32 t2 = *(volatile u32 *)(uintptr_t)(0x080CDB2Cu + (u32)ev * 4);
    _080075E8((void *)(uintptr_t)0x083A2100, (int)t2,
              (int)(s16)*(volatile u16 *)(entry + 22));
}
#ifndef __APPLE__
void _08026B80(int a) __attribute__((alias("Sprite_MenuEvent")));
void sub_08026B80(int a) __attribute__((alias("Sprite_MenuEvent")));
#endif

// ============================================================================
// sub_080026C84 (0x08026C84, 0x28 B) — single OAM emitter via the 8-arg
// RecordSetter_03004: (r0, r1, (s16)r2, (s16)r3, 0, 2, 256, a4).
// ============================================================================
void Sprite_EmitRecord(int a, int b, int c, int d, int a4) {
    extern void _08003004(int, int, int, int, int, int, int, int);
    _08003004(a, b, (s16)c, (s16)d, 0, 2, 256, a4);
}
#ifndef __APPLE__
void _08026C84(int a, int b, int c, int d, int e) __attribute__((alias("Sprite_EmitRecord")));
// The closure spells this address `sub_080026C84` (nine digits) -- see
// asm/code_263e0.s:1047 and `arm-none-eabi-nm build-code/code.o`. The
// eight-digit `sub_08026C84` below is a DISTINCT gas symbol, so aliasing only
// that one leaves the spliced body unable to define the name the link needs.
void sub_080026C84(int a, int b, int c, int d, int e) __attribute__((alias("Sprite_EmitRecord")));
void sub_08026C84(int a, int b, int c, int d, int e) __attribute__((alias("Sprite_EmitRecord")));
#endif

// ============================================================================
// sub_080026CAC (0x08026CAC, 0x9C B) — three Helper_02DB8 emitters:
//   _08007538(0x083A2100, 23, (s16)x); _080075E8(0x083A2100, 9, (s16)y)
//   _08002DB8(base, id, (s16)x, (s16)y, 0, 6, 1)
//   _08002DB8(base+32, id, x+8, y, 0, 6, 1)
//   _08002DB8(base+64, id, x+16, y, 0, 8, 1)
// ============================================================================
void Sprite_EmitTriple(void *base, int id, int x, int y) {
    extern void _08002DB8(void *, int, int, int, int, int, int);
    int sx = (s16)x;
    _08007538((void *)(uintptr_t)0x083A2100, 23, (void *)(uintptr_t)sx);
    int sy = (s16)y;
    _080075E8((void *)(uintptr_t)0x083A2100, 9, sy);
    _08002DB8(base, id, sx, sy, 0, 6, 1);
    _08002DB8((u8 *)base + 32, id, sx + 8, sy, 0, 6, 1);
    _08002DB8((u8 *)base + 64, id, sx + 16, sy, 0, 8, 1);
}
#ifndef __APPLE__
void _08026CAC(void *a, int b, int c, int d) __attribute__((alias("Sprite_EmitTriple")));
void sub_08026CAC(void *a, int b, int c, int d) __attribute__((alias("Sprite_EmitTriple")));
#endif

// ============================================================================
// sub_080026D48 (0x08026D48, 0x1E8 B, unlabeled) — menu-record per-frame tick.
//   rec = [0x03001674] (arg0 unused).
//   If s16[rec+2] != 0: EmitTriple(84, 28, s16[rec+4], s16[rec+8]); [rec+2]=0.
//   Else if s16[rec+30] != 0 && s16[rec+50] != 0:
//     entry = rec + cur*20; if s16[entry+12] > 0:
//       u16[entry+12] -= 2 (two identical decrements in the asm);
//       r5 = s16[entry+12] + 12
//       EmitRecord(104, r5, s16[entry+20], s16[entry+22],
//                  ((12 - s16[entry+12]) << 4) + 256)
//       EmitRecord(104, s16[entry+12] + 24, s16[entry+20], s16[entry+22],
//                  (s16[entry+12] << 4) + 256)
//     if s16[entry+12] == 0: [entry+30] = 0
//   Else (idle branch):
//     if s16[entry+12] > 0:
//       u16[entry+12] -= 1
//       if s16[entry+12] <= 80 || (s16[entry+12]/10)%2 != 0:
//         EmitRecord(104, 24, s16[entry+20], s16[entry+22], 256)
//     if s16[entry+12] == 0: [entry+30] = 0
// ============================================================================
void Sprite_TickRecords(int car) {
    (void)car;
    u8 *rec = (u8 *)(uintptr_t)SPR_MENU_REC;
    if ((s16)*(volatile u16 *)(rec + 2) != 0) {
        Sprite_EmitTriple((void *)(uintptr_t)84, 28,
                          (int)(s16)*(volatile u16 *)(rec + 4),
                          (int)(s16)*(volatile u16 *)(rec + 8));
        *(volatile u16 *)(rec + 2) = 0;
        return;
    }
    if ((s16)*(volatile u16 *)(rec + 30) == 0 || (s16)*(volatile u16 *)(rec + 50) == 0) {
        int cur = (s16)*(volatile u16 *)(rec + 0);
        u8 *entry = rec + cur * 20;
        if ((s16)*(volatile u16 *)(entry + 12) > 0) {
            *(volatile u16 *)(entry + 12) = (u16)(*(volatile u16 *)(entry + 12) - 1);
            if ((s16)*(volatile u16 *)(entry + 12) <= 80 ||
                _0802D97C(_0802D978((int)(s16)*(volatile u16 *)(entry + 12), 10), 2) != 0) {
                Sprite_EmitRecord(104, 24,
                                  (int)(s16)*(volatile u16 *)(entry + 20),
                                  (int)(s16)*(volatile u16 *)(entry + 22), 256);
            }
        }
        if ((s16)*(volatile u16 *)(entry + 12) == 0) {
            *(volatile u16 *)(entry + 30) = 0;
        }
        return;
    }
    int cur = (s16)*(volatile u16 *)(rec + 0);
    u8 *entry = rec + cur * 20;
    if ((s16)*(volatile u16 *)(entry + 12) > 0) {
        *(volatile u16 *)(entry + 12) = (u16)(*(volatile u16 *)(entry + 12) - 1);
        *(volatile u16 *)(entry + 12) = (u16)(*(volatile u16 *)(entry + 12) - 1); // asm decrements twice
        int e12 = (s16)*(volatile u16 *)(entry + 12);
        int x20 = (int)(s16)*(volatile u16 *)(entry + 20);
        int y22 = (int)(s16)*(volatile u16 *)(entry + 22);
        Sprite_EmitRecord(104, e12 + 12, x20, y22, ((12 - e12) << 4) + 256);
        Sprite_EmitRecord(104, e12 + 24, x20, y22, (e12 << 4) + 256);
    }
    if ((s16)*(volatile u16 *)(entry + 12) == 0) {   // shared tail _080026E72
        *(volatile u16 *)(entry + 30) = 0;
    }
}
#ifndef __APPLE__
void _08026D48(int a) __attribute__((alias("Sprite_TickRecords")));
void sub_08026D48(int a) __attribute__((alias("Sprite_TickRecords")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void Sub_080026A2C(void *mgr, u32 x, u32 y) __attribute__((alias("Sprite_SetXYRel")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void Sub_080026A4C(void *mgr, u32 a, u32 b) __attribute__((alias("Sprite_SetTenTwelve")));
void _080026A4C(void *mgr, u32 a, u32 b) __attribute__((alias("Sprite_SetTenTwelve")));
#endif
