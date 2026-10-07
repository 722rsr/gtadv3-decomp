#include "gtadv/ai_award.h"
#include "gba/types.h"

// ---- helpers: external ROM/IWRAM helpers not in lane ----
extern void CpuSet(const void *src, void *dst, u32 mode);
extern void _0802E0A4(void *dst, const void *src, u32 n);
static inline int do_packed_shift(int idx,int bits){
    extern int _0802D97C(int,int);
    return _0802D97C(idx,bits);
}
__attribute__((unused)) static int _use_packed_shift = 0;

// _08025F78 — set collection bit for car id
void Ai_AwardSetBit(int id){
    int r1 = id;
    int r0;
    volatile u8 *base = (volatile u8*)(uintptr_t)AI_COLLECTION_BM_BASE;
    // floor div for negatives as asm: if r1<0 r0=r1+7 else r0=r1; r0>>=3
    if(r1 < 0) r0 = r1 + 7; else r0 = r1;
    r0 >>= 3; // asrs #3
    volatile u8 *bytePtr = base + 32 + (u32)r0; // 0x030017A0 base
    const u8 *maskTbl = (const u8*)(uintptr_t)AI_MASK_TABLE;
    int tmp = r0 << 3;
    tmp = r1 - tmp; // id %8 with floor correction
    tmp &= 7;
    tmp <<= 1;
    u8 mask = maskTbl[tmp]; // ldrb [maskTbl+tmp] — low byte of hword entry
    u8 cur = *bytePtr;
    cur |= mask;
    *bytePtr = cur;
}
#ifndef __APPLE__
void _08025F78(int id) __attribute__((alias("Ai_AwardSetBit")));
void sub_08025F78(int id) __attribute__((alias("Ai_AwardSetBit")));
void Sub_08025F78(int id) __attribute__((alias("Ai_AwardSetBit")));
#endif

// _08025FAC — owned? test
//
// ROM shape (68 B at 0x08025FAC, file offset 0x25FAC; pools 0x03001780 and
extern u8 AIColBM[] __asm__("AIColBM");
extern u8 AIMaskTbl[] __asm__("AIMaskTbl");
int Ai_OwnedTest(int id){
    register int r3 __asm__("r3") = id;
    register u8 *base __asm__("r1");
    int dv;
    u8 *blk;
    int ve;
    u8 *maskTbl;
    int bit;
    s16 mask;
    int anded;
    int neg;
    int orv;
    unsigned out;
    __asm__(".globl AIColBM\nAIColBM = 0x03001780\n");
    __asm__(".globl AIMaskTbl\nAIMaskTbl = 0x080CD9D4\n");
    if(r3 < 0) r3 = 0;
    if(r3 > 97) r3 = 98;
    base = (u8*)AIColBM;
    if(r3 < 0) dv = r3 + 7; else dv = r3;
    dv >>= 3;
    blk = base + 32;
    dv += (int)(uintptr_t)blk;
    ve = *(s8*)(u8*)(uintptr_t)dv;
    maskTbl = (u8*)AIMaskTbl;
    bit = r3 & 7;
    bit <<= 1;
    mask = *(s16*)(maskTbl + bit);
    anded = ve & (int)mask;
    neg = -anded;
    orv = neg | anded;
    out = (unsigned)orv >> 31;
    return (int)out;
}
#ifndef __APPLE__
int _08025FAC(int id) __attribute__((alias("Ai_OwnedTest")));
int sub_08025FAC(int id) __attribute__((alias("Ai_OwnedTest")));
#endif

// Friendly-name wrappers used by car_award.c (u8 car id).
// VMA sub_08025FAC (presence test) and sub_08025F78 (grant = set bit).
int Ai_CarPresence(u8 id) { return Ai_OwnedTest((int)id); }
void Ai_GrantEvent(u8 id) { Ai_AwardSetBit((int)id); }

// _08025FF0(slot) — set owned flag at 0x03001780+1400+slot
//
// The ROM keeps 1400 as `movs r2,#175; lsls r2,r2,#3` and adds it to the
// POINTER rather than folding it into the literal: `ldr r1,=0x03001780 /
// movs r2,#175 / lsls r2,r2,#3 / adds r1,r1,r2 / adds r0,r0,r1`. A
// CONST_INT base is folded by simplify_rtx at expand time and the whole shift
// pair disappears (measured candidate: a single pool word 0x03001CF8 and a
// 4-byte-short body). Naming the base as an absolute SYMBOL_REF is the lever
// src/save.c's SaveBumpBase already uses: a SYMBOL_REF is not folded.
extern u8 AICollectionBM[] __asm__("AICollectionBM");
void Ai_SetOwnedFlag(int slot){
    volatile u8 *base = (volatile u8*)AICollectionBM;
    volatile u8 *p;
    u32 off = 175;
    __asm__(".globl AICollectionBM\nAICollectionBM = 0x03001780\n");
    off = off << 3;
    base += off;
    p = base + slot;
    *p = 1;
}
#ifndef __APPLE__
void _08025FF0(int a) __attribute__((alias("Ai_SetOwnedFlag")));
void sub_08025FF0(int a) __attribute__((alias("Ai_SetOwnedFlag")));
void Sub_08025FF0(int a) __attribute__((alias("Ai_SetOwnedFlag")));
#endif

// _08026004 — get flag
//
// ROM 0x08026004: `movs r2,#0 / ldr r1,=0x03000178 / movs r3,#175 /
// lsls r3,r3,#3 / adds r1,r1,r3 / adds r0,r0,r1 / ldrb r0,[r0,#0] /
// cmp r0,#1 / bne / movs r2,#1 / adds r0,r2,#0 / bx lr`. Three levers, each
// load-bearing:
//   * the base is a SYMBOL_REF, not a CONST_INT, so 0x03000178 stays a pool
//     `ldr` instead of folding into the pointer arithmetic (see Ai_SetOwnedFlag).
//   * `off` is pinned to r3: the ROM keeps 1400 in r3, not r2 where agbcc's
//     local_alloc would put it (r1 is live as the base).
//   * `res` is pinned to r2 AND initialised at the TOP: the ROM's `movs r2,#0`
//     is the first instruction, ahead of the pool load. The u8-typed flag and
//     the int result are distinct registers, so the store never reuses r2.
// NOTE the base here is the SAME 0x03001780 that AI_COLLECTION_BM_BASE names:
// the pool word at 0x0802601C is 0x03001780 (verified byte-for-byte).
extern u8 AIFlagBase[] __asm__("AIFlagBase");
int Ai_GetOwnedFlag(int slot){
    register int res asm("r2");
    register u32 off asm("r3");
    volatile u8 *base;
    u8 v;
    __asm__(".globl AIFlagBase\nAIFlagBase = 0x03001780\n");
    res = 0;
    base = AIFlagBase;
    off = 175;
    off = off << 3;
    base += off;
    v = *(volatile u8 *)(uintptr_t)((uintptr_t)slot + (uintptr_t)base);
    if (v == 1) res = 1;
    return res;
}
#ifndef __APPLE__
int _08026004(int a) __attribute__((alias("Ai_GetOwnedFlag")));
int sub_08026004(int a) __attribute__((alias("Ai_GetOwnedFlag")));
#endif

// _08026020 — clamp slot 0..2, set its byte, then hand the packed table word
// to _08025F78. 72 B, pools 0x08060D4C (source table), 0x03001780 and 0x57C
// (flag base + offset, kept as two literals so the adds pair matches the ROM).
//
// `u8 buf[8] = {0};` was the recorded "blocked on memset" blocker, and it is
// not a ROM call at all: the ROM's frame is a bare `sub sp,#8` and nothing
// zeroes it — only the 6 bytes written by 0x0802E0A4 are ever read. agbcc
// lowers the zero-init to `bl memset`, a compiler intrinsic with no ROM
// counterpart, which is why the body read as blocked on it. Leaving `buf`
// uninitialised removes the intrinsic and matches the ROM.
// Call-site split. The closure binds `_08025F78`/`sub_08025F78`; the friendly
// name `Ai_AwardSetBit` is not a closure symbol and `sub_08025F78` is not yet a
// promoted export, so it resolves by neither of promotion_screen's two routes
// and the screen blocks the body with "closure defines _08025F78/sub_08025F78
// (rename)". Same shape as RM_CALLEE in src/runtime_hud.c and CO_CALLEE in
// src/course_orch.c. Note the discriminator is promoted-vs-unpromoted, not
// friendly-vs-closure: a promoted export resolves even when absent from nm.
#ifndef __APPLE__
#define AA_CALLEE(friendly, closure) closure
extern void sub_08025F78(int id);
#else
#define AA_CALLEE(friendly, closure) friendly
#endif
void Ai_ClaimSlotAndSetBit(int slot){
    u8 buf[8];
    _0802E0A4(buf, (const void *)(uintptr_t)0x08060D4Cu, 6u);
    // clamp 0..2
    int r4 = slot;
    if(r4 <= 0) r4 = 0;
    if(r4 > 1) r4 = 2;
    uintptr_t base_v, slot_off;
#ifndef __APPLE__
    { extern u8 AiColFlagBase[], AiColFlagOff[];
      __asm__(".globl AiColFlagBase\nAiColFlagBase = 0x03001780\n");
      __asm__(".globl AiColFlagOff\nAiColFlagOff = 0x57C\n");
      base_v = (uintptr_t)AiColFlagBase;
      slot_off = (uintptr_t)AiColFlagOff; }
#else
    base_v = (uintptr_t)AI_COLLECTION_BM_BASE;
    slot_off = 0x57C;
#endif
    // r4 on the LEFT of the final add: the ROM emits `adds r0, r4, r0`, and C's
    // left-associative `+` would give `adds r0, r0, r4` (0x1900 vs 0x1820).
    volatile u8 *flag = (volatile u8 *)((uintptr_t)r4 + (base_v + slot_off));
    *flag = 1;
    // plain s16, not volatile: a volatile read lowers to ldrh + lsls/asrs
    // where the ROM has one ldrsh.
    int half = *(s16*)(buf + (r4<<1));
    AA_CALLEE(Ai_AwardSetBit, sub_08025F78)(half);
}
#ifndef __APPLE__
void _08026020(int a) __attribute__((alias("Ai_ClaimSlotAndSetBit")));
// The closure binds `sub_08026020` (lowercase). A capital-`Sub_` spelling is a
// different symbol to a case-sensitive linker and matched no closure label
// here, so it could never have bound; the manifest's export list needs the
// lowercase twin, and an exported `sub_` twin must itself be defined in C.
void sub_08026020(int a) __attribute__((alias("Ai_ClaimSlotAndSetBit")));
#endif


// _08026150(idx, a, b) — resolve leaf via 0x0800572C, store the pair, read
// back. r0=idx, r1=a, r2=b(unused); returns the stored halfword sign-extended.
// The 8-byte stride is `lsls #16 / asrs #13`, i.e. (idx<<3) with idx taken as a
// signed 16-bit value, so the index locals are built in the ROM's order:
// r4 = (u16)idx, then the (u16)a / (s16)a pair off one shared `lsls #16`, then
// the call, and only then the base add.
s16 Ai_AwardLeafSetRet(int idx, int a, int b){
    //. The closure spells this callee `sub_0800572C` (asm/save_alloc.s
    // `.type sub_0800572C, %function`), and src/code_25930.c / course_resource_more.c
    // already declare that spelling. The `_0800572C` spelling here left the
    // promotion screen reporting
    // `_0800572C: closure defines sub_0800572C at 0x0800572c (rename)`.
    // Use the closure's own name; no C body is being changed here, only the
    // callee spelling, so the scored bytes are unaffected.
    extern int sub_0800572C(int);
    u32 ia = (u32)idx << 16;
    u32 r4 = ia >> 16;
    u32 ta = (u32)a << 16;
    u32 r5 = ta >> 16;
    s32 r0 = (s32)ta >> 16;
    s32 ret;
    uintptr_t base;
    u32 off;
    s32 iw;
    volatile u16 *p;
    ret = sub_0800572C(r0);
#ifndef __APPLE__
    { extern u8 AiAwardRecBase[];
      __asm__(".globl AiAwardRecBase\nAiAwardRecBase = 0x030015F0\n");
      base = (uintptr_t)AiAwardRecBase; }
#else
    base = (uintptr_t)AI_AWARD_REC_BASE;
#endif
    iw = (s32)r4;
    off = (u32)((iw << 16) >> 13);
    p = (volatile u16 *)(uintptr_t)(off + base);
    p[0] = (u16)ret;
    p[1] = (u16)r5;
    return *(s16 *)p;
}
#ifndef __APPLE__
s16 _08026150(int a, int b, int c) __attribute__((alias("Ai_AwardLeafSetRet")));
s16 sub_08026150(int a, int b, int c) __attribute__((alias("Ai_AwardLeafSetRet")));
#endif
// The header's historical spelling returns void; the ROM returns r0 (the
// sign-extended halfword it just stored) and 10 asm call sites consume it, so
// the void wrapper keeps the declared prototype honest for existing C callers.
void Ai_AwardLeafSet(int idx, int a, int b){
    (void)Ai_AwardLeafSetRet(idx, a, b);
}

// _080261B0(idx) -> s16 at +0. Leaf: no frame, `movs r1,#0` + `ldrsh` (not
// `ldrh` + sign-extend pair), so the load is read through a plain `s16 *`.
// Dropping `volatile` here is the load-form choice the workflow records; a
// `volatile s16 *` produces `ldrh` + `lsls/asrs` instead of one `ldrsh`.
s16 Ai_AwardLeafGet(int idx){
#ifndef __APPLE__
    extern u8 AiAwardRecBase[];
    __asm__(".globl AiAwardRecBase\nAiAwardRecBase = 0x030015F0\n");
    uintptr_t base = (uintptr_t)AiAwardRecBase;
#else
    uintptr_t base = (uintptr_t)AI_AWARD_REC_BASE;
#endif
    int off = (idx << 16) >> 13;
    return *(s16 *)(uintptr_t)(off + base);
}
#ifndef __APPLE__
s16 _080261B0(int a) __attribute__((alias("Ai_AwardLeafGet")));
s16 sub_080261B0(int a) __attribute__((alias("Ai_AwardLeafGet")));
#endif

// _080261C4(idx, val) — strh to +0. Same leaf shape as 261B0 with a store:
// `off + base` in that order keeps the pool load first, matching the ROM.
void Ai_AwardLeafPut(int idx, int v){
#ifndef __APPLE__
    extern u8 AiAwardRecBase[];
    __asm__(".globl AiAwardRecBase\nAiAwardRecBase = 0x030015F0\n");
    uintptr_t base = (uintptr_t)AiAwardRecBase;
#else
    uintptr_t base = (uintptr_t)AI_AWARD_REC_BASE;
#endif
    int off = (idx << 16) >> 13;
    volatile u16 *p = (volatile u16 *)(uintptr_t)(off + base);
    p[0] = (u16)v;
}
#ifndef __APPLE__
void _080261C4(int a, int b) __attribute__((alias("Ai_AwardLeafPut")));
void sub_080261C4(int a, int b) __attribute__((alias("Ai_AwardLeafPut")));
#endif
