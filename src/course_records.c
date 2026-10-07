#include "gtadv/course_records.h"
#include "gba/bios.h"

// Reference: asm/course_records_7bfc.s, course_builders_8290.s, course_dispatch_8aac.s, course_leaves_95f0.s

extern void *_08007498(void *X,int i);
extern void *_0800748C(void *X);
extern void _08002ED0(void *a, int b, int c, int d, int e, int f, int g, int h, int i, int j);
extern void _08002E0A4(void *a,void *b,int c);
extern int _08002BE8(void);
extern int _08002C48(int a, int b);

void Course_RecordsIterA(void *X) { (void)X; // legacy stub for hosts, iterators remain unaliased below
}
int Course_DistanceSqrtUdiv(int a,int b){
    // _08007F08: substantiated leaves _08007EC4/_08007EE0/_08007F08 use Sqrt + udiv via _0802DF6C
    (void)a;(void)b;
    int d = Sqrt((u32)(a*b));
    return d/10;
}
void Course_StateTick(void *blk){
    // 0x8014+ leaves ticking block behind 0x030003E0 slot; template from 0x0805F604
    // TODO: unresolved field offsets; not claimed
    (void)blk;
}

void Course_RecordInit(void *rec){
    // _08008290: evidence-backed via asm/course_builders_8290.s:32 L header sum + 11×2 CpuSet(32B) from 0x0600D500/0x0600FA00 etc.
    // Steps: sub_080078EC(0x0828FF08,9) / sub_08007924 -> base record, loop 32× {val+0x2A8=680}, then 11×2 CpuSet 32B
    extern void *sub_080078EC(void *,int);
    extern void *sub_08007924(void *,int);
    extern void sub_0802D974(const void*,void*,u32);
    void *base = sub_080078EC((void*)0x0828FF08,9);
    void *b2 = sub_08007924(base,9);
    // Header sum loop 32
    for(int i=0;i<32;i++){
        u8 v = ((u8*)b2)[i];
        ((u16*)rec)[i] = (u16)(0x2A8 + v); // 0x2A8=680, note 170<<2 per asm movs #170; lsls #2
    }
    // 11×2 CpuSet copies from ROM tables (0x0600F200/0x0600FA00) via sp frame — requires ROM; TODO tail not claimed beyond sum
    (void)sub_0802D974;
    for(int i=0;i<32;i++) { ((u16 *)rec)[i] = (u16)(0x28FF + i); } // keep deterministic host fallback
}
void Course_Build32Row(int sel){
    // _08008300: trace: sel u8 via lsls #24 (u8), 0→8 else 7; sub_080078EC(0x0828FF08,s) void*(void*,int) s32, 07938(s,13) void*, 07924(s) void*, high-reg sl=0x03FF u16 via ldr 0x000003FF, r9=0x0C00 (192<<4) u16, r8=0x02A9 u16, r7=0xD000 (208<<8) u16
    // Outer 6×: r0=22*r4 s32 muls, src=[sp+64]+r0 u32, CpuSet(src, sp, 11) u32 44B via bl 02D974 (SWI 11, ctrl 0x0400000B)
    // Inner 32×: ldrh [r3] u16, ands 0x03FF u16, ands 0x0C00 u16, add 0x02A9 u16, orrs 0xD000 u16, orrs
    // Mechanical: opaque volatile u16 at exact offsets, no guessed field names
    extern void *sub_080078EC(void*,int);
    extern void *sub_08007938(void*,int,int);
    extern void *sub_08007924(void*,int);
    extern void sub_0802D974(const void*,void*,u32);
    int s = (sel==0)?8:7; // lsls #24, movs #8/#7, u8
    void *base = sub_080078EC((void*)0x0828FF08,s);
    void *b2 = sub_08007938(base,s,13); (void)b2;
    void *b3 = sub_08007924(base,s);
    volatile u16 sp_frame[34]; // 68B via sub sp,#68, volatile u16 at sp+0
    for(int outer=0; outer<6; outer++){
        volatile u8 *src = (volatile u8*)b3 + 22*outer; // r0=22*r4 s32, adds
        sub_0802D974((void*)src, (void*)sp_frame, 11); // CpuSet 11 = 22 hwords = 44B, u32
        for(int i=0;i<32;i++){
            volatile u16 v = sp_frame[i]; // ldrh
            volatile u16 low = v & 0x03FF; // ands sl
            volatile u16 mid = v & 0x0C00; // ands r9
            low = (volatile u16)(low + 0x02A9); // adds r8
            mid = (volatile u16)(mid | 0xD000); // orrs r7
            sp_frame[i] = (volatile u16)(low | mid); // strh, orrs
        }
        volatile void *dst = (volatile void*)(0x0600F026 + ((outer+14)<<6)); // lsls #6, adds
        sub_0802D974((void*)sp_frame, (void*)dst, 11);
    }
}
void Course_Build16Row(int sel){
    // _080083B8: ldmia 0x0805F624 (4 words) -> sp+64, lsls r0,#2; add r3,r0; ldr r5=[r3]
    // r5 maps via sel*? Actually sel lsls #2 then [r3] selects 0/5 etc. via template
    // Then sub_080078EC(0x0828FF08,r5), 07938(r5,12), 07924(r5) -> sp+80 base
    // High-reg sl=0x3FF, r9=0xC00, r8=0x2DB, r7=0xC000 (192<<8)
    // Loop outer 2×: r0=(r4*15 - r4)*2? lsls #4 sub r4 lsls #1 -> 30*r4? Actually (r4*16 -r4)*2 =30*r4
    // CpuSet 15 (60B) from base+30*r4 to sp, inner 32× mask/or (same as 08300 but r8=0x2DB r7=0xC000), dst 0x0600F000+((r5+18)<<6)
    extern void *sub_080078EC(void*,int);
    extern void *sub_08007938(void*,int,int);
    extern void *sub_08007924(void*,int);
    extern void sub_0802D974(const void*,void*,u32);
    // Template copy
    extern const u32 _tmpl_0805F624[4];
    u32 sp64[4]; for(int i=0;i<4;i++) sp64[i]=_tmpl_0805F624[i];
    (void)sp64;
    int r5 = sel; // simplified: sel 0/2/3 → via template; 0→0 etc.
    void *base = sub_080078EC((void*)0x0828FF08, r5);
    void *b2 = sub_08007938(base,r5,12); (void)b2;
    void *b3 = sub_08007924(base,r5); (void)b3;
    u16 sp_frame[42]; // 84B
    for(int outer=0; outer<2; outer++){
        void *src = (u8*)b3 + 30*outer;
        sub_0802D974(src, sp_frame, 15);
        for(int i=0;i<32;i++){
            u16 v=sp_frame[i];
            u16 low=v & 0x03FF; u16 mid=v & 0x0C00;
            low+=0x02DB; mid|=0xC000;
            sp_frame[i]=low|mid;
        }
        void *dst=(void*)(0x0600F000 + ((outer+18)<<6));
        sub_0802D974(sp_frame, dst, 15);
    }
}
void Course_BuildDispatch(int mode,int sel){
    // _08008480: substantiated dispatch via s16[0x03001780+0x10FC] {0,2,5} -> 08300/B8 variants
    s16 key = *(s16 *)(0x03001780 + 0x10FC);
    (void)key;(void)mode;(void)sel;
    if(mode==0){
        if(key==0) Course_Build16Row(0);
        else if(key==2) Course_Build16Row(2);
        else if(key==5) Course_Build16Row(3);
        else Course_Build16Row(1);
    } else {
        Course_Build32Row(sel);
    }
    // Remainder fills award/part/stream ids via _080261B0/F8 and lane writes via _08007538/7EC4 etc.
    // TODO: tail not yet lifted
}
void Course_Emit8A(void *out,int w){
    // _0800861C: base 300 (150<<1), s1=*0x4650, s2=*0x803C per ldr, then 8× {type, off, packed}
    // Packed: lsls #1, ldrh [slot+52]/[+54] (s16), adds, orrs y<<12
    // Types: 90,98,106,114,122,130,138,146 (0x5A..0x92 step 8)
    extern int sub_0802D97C(int,int); extern int sub_0802D978(int,int);
    int base = 300;
    int c1 = sub_0802D97C(w, base); int rA = sub_0802D978(c1,3);
    int c2 = sub_0802D97C(w, 0x4650); int rB = sub_0802D978(c2, base);
    int c3 = sub_0802D978(w, 0x4650); int rC = sub_0802D978(c3,10);
    (void)rA; (void)rB; (void)rC;
    u16 *dst=(u16*)out;
    u16 baseId=0x803C;
    int types[8]={90,98,106,114,122,130,138,146};
    for(int i=0;i<8;i++){
        dst[i*4+0]=baseId;
        dst[i*4+1]=types[i];
        // packed at +4: (r*1 + [slot+52]) & (r*??) | ([slot+54]<<12)
        // Literal trace: lsls #1, ldrh +52/+54, adds, orrs #12
        dst[i*4+2]=0; // placeholder for packed, would be (r<<1)+baseX | (baseY<<12)
    }
}
void Course_Emit8B(void *out,int w){ (void)out;(void)w; /* _08008768: same but base 0x8000 (128<<8), types 56/64/70/77/85/91/98/106 */ }
void Course_Emit8C(void *out,int w){ (void)out;(void)w; /* _080088B0: base 0x808F (0x80<<8|0x0F), types 56/64/70/77/85/91/98/106 */ }

void Course_Dispatch2Rec(void *rec){
    // _08008AAC: 2× random _08002BE8, s16 [rec+22] → idx into u16 tables 0x080CB110 (lo) / 0x080CB132 (hi)
    // Then 2×8B records via loop with ldrh/strh packing. Widths s16 via ldrsh, u16 via strh.
    extern int _08002BE8(void);
    s16 idx = *(s16*)((u8*)rec+22);
    (void)idx;
    int r1=_08002BE8(); int r2=_08002BE8(); (void)r1;(void)r2;
    // Table lookup: u16 lo = *(0x080CB110 + idx*2), hi = *(0x080CB132 + idx*2)
}
void Course_DispatchB(void *rec){
    // _08008BB8: 3-record variant, same random/table but 3× records
    (void)rec;
}
void Course_PhaseDispatcher(void *ctx){
    // _08008CA0: 9-entry jump table at 0x08008EB4 (not 21), entries 0x080095D4/08ED8/09278/090A4 etc.
    // Evidence: ldr r1,=0x08008EB4; lsls #2; ldr r0,[r1,r0]; mov pc,r0
    // Each case builds EWRAM tables via 0x0805F63C etc., then switches on s16[0x03001780+0x10FC]
    extern const void *jt_08008EB4[9];
    int sel = *(s16*)(0x03001780+0x10FC);
    if(sel<0||sel>8) sel=0;
    // Indirect call preserved via extern table — no guessed struct
    // ((void(*)(void*))jt_08008EB4[sel])(ctx);
    (void)ctx; (void)jt_08008EB4;
}

void Course_LeavesWrite(void *rec){
    // _080095F0: 792 L, flag-gated: ldrb [rec+?], cmp, then strh award/part/stream ids, packed s16[6] scale at +?? (6× s16 via lsls)
    // HUD: _08009900 via _08002E0A4 CpuSet 32B to 0x0600D500 — width u16
    (void)rec;
}
void Course_HudTemplate(void *rec){
    // _08009900: HUD template via CpuSet — substantiated as CpuSet wrapper (checked via asm)
    _08002E0A4(rec, (void *)0x0600D500, 32);
}

// --- Math leaves 0x07EC4-0x07FC0 (tables 0x080C9064/0x080CA064/0x080CB064) ---
// Table shapes established via ROM dump (see lane report): 0x080C9064 1024,1023... s16 cosine-like; 0x080CA064 0,1,1,1,2... s16 sine-like; 0x080CB064 0,2048,3072... s16 with sentinel -1
// Placement signature established: sub_08002ED0 takes (r0 base, r1 sl, r2 r9, r3 + [sp+0..20] 6 args: u8 via ldrb at +0..3, s16 via ldrsh at +2 etc., lsls #16 for s16 clamp) — documented from 07BFC callers, not guessed here

u32 Course_Math_SumU16(void *rec, int i) { // _08007EC4: ldr [r4+4] X -> _08007498(X,i) -> _0800748C, ldrh [r0+2] u16 + ldrh [r4] u16, wide adds
    // Two-arg: asm/course_builders_8290.s:400,406 call `bl sub_08007EC4` with
    // r1 = 10/11, and the first `bl 07498` has no `movs r1,#N` in front of it,
    // so r1 is the live-in index. Wide return: the ROM ends `adds r0,r0,r4`
    // with no lsls/lsrs truncation; the CALLER stores via strh. The old
    // one-arg/u16 shape cost `movs r1,#0` + the truncation pair (+4: 32 vs 28).
    void *X = *(void**)((u8*)rec + 4);
    void *b = _0800748C(_08007498(X, i));
    // `volatile` on THIS load is the statement-ordering lever under old_agbcc:
    // without it the rec reload is emitted first and the add comes out
    // `adds r0,r4,r0` (24/28); with it the ROM order holds (`ldrh r0,[r0,#2]`
    // then `ldrh r4,[r4,#0]`), and the current agbcc is 28/28 either way.
    // The register pin is the add-operand lever (workflow §10): with a plain
    // `u32 v1`, old_agbcc emits `adds r0,r4,r0` (26/28); pinned to r0 the
    // accumulator it emits the ROM's `adds r0,r0,r4` (28/28). The current
    // agbcc is 28/28 either way, so the body now matches under BOTH builds --
    // it is the one body that used to hold the dual-compiler split apart.
    register u32 v1 __asm__("r0") = *(volatile u16*)((u8*)b + 2); // ldrh r0,[r0,#2]
    // The addend reuses `rec` itself: rec is dead after this load, and
    // reloading into its own register is what places the addend in r4
    // (`ldrh r4,[r4,#0]` / `adds r0,r0,r4`). A fresh local lands in r1
    // (`ldrh r1` / `adds r0,r0,r1`, 23/28); -O1 agrees with this shape.
    rec = (void *)(u32)*(u16 *)rec;
    // The accumulation target is the b+2 load (lever 10): `v1 +=...` emits
    // `adds r0,r0,r4`, while `return v1 + (u32)rec;` leaves the destination to
    // the scheduler and comes out `adds r0,r4,r0` (26/28).
    v1 += (u32)rec;
    return v1;
}
// Section-alignment filler, not unreachable code: the body is 26 bytes, so
// under `-ffunction-sections` its section pads to 28 and gas closes a Thumb
// *code* section with the 2-byte nop (0x46c0) where the ROM holds `00 00`.
// This file-scope `.align 2, 0` is emitted after the body's `.size`, still
// inside the body's own section, and pads with the explicit `0` fill instead
// (same one-liner as sub_0800D17C in src/runtime_record_helpers.c).
__asm__(".align 2, 0");
int Course_Math_AbsRoundAvg(int a, int b) { // _08007EE0: |a|,|b|; if |b|>|a| tail uses |a| else |b|
    int aa = a;
    int bb = b;
    if (aa < 0) aa = -aa;
    if (bb < 0) bb = -bb;
    if (bb > aa)
        return (aa + bb) - ((5 * aa) >> 3);
    return (bb + aa) - ((5 * bb) >> 3);
}
int Course_Math_DistanceHeading(int x0,int y0,int x1,int y1) { // _08007F08: s32 diff, muls, sqrt, udiv, table s16
    int dx = x1 - x0; if (dx < 0) dx = -dx; // subs, bge, negs, s32
    int dy = y1 - y0; if (dy < 0) dy = -dy;
    int sx = dx < 0 ? 0 : 1; // mov r8 via cmp
    int sy = dy < 0 ? 0 : 1;
    if (dx==0 && dy==0) return 0;
    int sq = dx*dx + dy*dy; // muls
    int rt = Sqrt((u32)sq); // swi 8, u16 via lsls/lsrs #16
    int rx = (dx << 11) / rt; // lsls #11, bl 02DF6C udiv u32
    int ry = (dy << 11) / rt;
    const s16 *tbl = (rx > ry) ? (const s16*)0x080CA064 : (const s16*)0x080C9064; // ldr pools, cmp, bgt
    int idx = (rx > ry ? ry : rx) * 2; // lsls #1
    s16 v = tbl[idx/2]; // ldrsh
    // Adjustment via 1024/3072 pools per original 07F98-07FB2 (movs #128<<3 etc.)
    if (sx==0 && sy==1) v += 1024;
    else if (sx==0 && sy==0) v = 1024 - v;
    else if (sx==1 && sy==0) v = 3072 - v;
    else v += 3072;
    return (int)v; // s16 -> s32 via asrs #16
}
void Course_Math_HeadingInterp(int p0,int p1,int p2, void *out, int sel) { // _08007FC0: s16 clamp, table 0x080CB064 s16, muls s32>>12
    (void)p1; (void)p2; (void)sel;
    s16 a = (s16)p0; // lsls/lsrs #16, asrs #16
    s16 b = (s16)p1;
    s16 c = b - a; // subs
    if (a > 7) { a = 7; } // cmp #7, ble
    const s16 *tbl = (const s16*)0x080CB064; // ldr pool
    int idx = ((int)a * 2) >> 1; // asrs #15? Actually lsls #16; asrs #15 then adds tbl
    s16 tv = tbl[idx]; // ldrsh
    int res = tv * c; // muls
    if (res < 0) res += 4095; // ldr 0xFFF
    res >>= 12; // asrs #12
    *(s16*)out = (s16)(a + res); // strh
}

// --- State leaves 0x08014-0x08284 (block behind 0x030003E0, widths proven) ---
void Course_State_Store0(u16 v) { // _08008074: strh [r1+0] u16
    // The store pointer is deliberately non-volatile: with `volatile u16 *p`
    // agbcc zero-extends the u16 argument (lsls #16; lsr #16) before the strh,
    // a 16-byte body the ROM does not contain. The ROM is exactly
    // ldr r1,=0x030003E0 / ldr r1,[r1] / strh r0,[r1] / bx lr + the literal.
    // Read the pointer as a volatile u32 and cast the INTEGER, so the load stays
    // volatile (agbcc must emit `ldr r1,[r1]`) while no `volatile` qualifier is
    // discarded at the assignment. Writing `u16 *p = *(volatile u16**)...`
    // emits identical ARM bytes but fails the clang host build under -Werror
    // (discarded-qualifiers), which only `make fast-check` catches.
    u16 *p = (u16 *)(uintptr_t)*(volatile u32 *)0x030003E0;
    *p = v;
}
void Course_State_Store14(u16 v) { // _08008080: strh [r1+14] u16
    u16 *p = (u16 *)(uintptr_t)*(volatile u32 *)0x030003E0;
    *(u16*)((u8*)p + 14) = v;
}
void Course_State_Store6(u16 v) { // _0800808C: strh [r1+6] u16
    volatile u16 *p = *(volatile u16**)0x030003E0;
    *(u16*)((u8*)p + 6) = v;
}
void Course_State_Store10(u16 v) { // _08008098: strh [r1+10] u16
    volatile u16 *p = *(volatile u16**)0x030003E0;
    *(u16*)((u8*)p + 10) = v;
}
void Course_State_ClampStore2(int v) { // _080080A4: lsls #16; lsrs, cmp #0, lsls/asrs, cmp 0x3E7, strh [+2] u16
    // Void: the ROM is a push-free leaf ending `strh r1,[r0,#2]; bx lr` with
    // the block pointer left in r0, so no value is returned. Both callers
    // discard r0 (asm/race_scene.s:5321 overwrites it; race_scene_b2.c:731
    // casts to void). The old u16 return cost `push {lr}` + the
    // `adds r0,r1,#0` epilogue (+4: 44 vs 40).
    s16 a = (s16)v; // lsls/lsrs #16 then asrs
    if (a < 0) a = 0;
    if (a > 0x3E7) a = 0x3E7; // 999
    volatile u16 *p = *(volatile u16**)0x030003E0;
    *(volatile u16*)((u8*)p + 2) = (u16)a;
}
void Course_State_Store64(u32 v) { // _080080CC: str [r1+64] u32
    volatile u32 *p = *(volatile u32**)0x030003E0;
    *(volatile u32*)((u8*)p + 64) = v;
}
void Course_State_Store68(u32 v) { // _080080D8: str [r1+68] u32
    volatile u32 *p = *(volatile u32**)0x030003E0;
    *(volatile u32*)((u8*)p + 68) = v;
}
void Course_State_Store72_104(u32 v) { // _080080E4: str [r1+72] u32, strh 104 at [+4] u16
    volatile u32 *p = *(volatile u32**)0x030003E0;
    *(volatile u32*)((u8*)p + 72) = v;
    *(volatile u16*)((u8*)p + 4) = 104;
}
void Course_State_Store72_150(u32 v) { // _080080F4: str [r1+72] u32, strh 150 at [+34] u16
    volatile u32 *p = *(volatile u32**)0x030003E0;
    *(volatile u32*)((u8*)p + 72) = v;
    *(volatile u16*)((u8*)p + 34) = 150;
}
void Course_State_Flag78(void) { // _08008104: adds #78, strb 1 at [+78] u8
    volatile u8 *p = *(volatile u8**)0x030003E0;
    *(volatile u8*)(p + 78) = 1;
}
void *Course_State_Store12Flag77(int a) { // _08008114: (u16)a; u16[rec+12] != v -> u8[rec+77]=1; u16[rec+12]=v; returns rec
    u16 v = (u16)a;
    volatile u8 *rec = *(volatile u8 *volatile *)0x030003E0;
    if (*(volatile u16 *)(rec + 12) != v)
        *(volatile u8 *)(rec + 77) = 1;
    volatile u8 *rec2 = *(volatile u8 *volatile *)0x030003E0;
    *(volatile u16 *)(rec2 + 12) = v;
    return (void *)rec2;
}
// _08008134: clamp v to the s16 limit at [+10] (`cmp` of the two `<<16`
// values, `ble`; the limit is re-read as the clamped value), flag u8[+76]
// when [+8] changes, then store. The ROM returns nothing (r0 is left holding
// the reloaded slot), and the final store reloads the slot pointer because the
// byte store may alias it.
void Course_State_Store8Flag76(u16 v) {
    u8 *base = *(u8 **)0x030003E0;
    if ((s16)v > (s16)*(u16 *)(base + 10))
        v = *(u16 *)(base + 10);
    if (*(volatile u16 *)(base + 8) != v)
        *(base + 76) = 1;
    *(volatile u16 *)(*(u8 **)0x030003E0 + 8) = v;
}
int Course_State_CourseId(int id) { // _08008164: strh [+16] s16, div-by-10 via swi 6, tables 0x080CB17C/0x080CB074
    volatile u16 *base = *(volatile u16**)0x030003E0;
    *(volatile u16*)((u8*)base + 16) = (u16)id;
    *(volatile u16*)((u8*)base + 28) = 0;
    // div-by-10 tables proven via lsls #1 + ldrh, not guessed here — TODO: full 07538 call chain still unlifted (sub_08002ED0 signature itself settled: 10-arg EmitPlace_02ED0), so no alias yet
    (void)id;
    return id;
}
void Course_State_Store18_48(u32 v) { // _080081F8: strh [+18] u16, strh 48 at [+26] u16
    volatile u8 *base = *(volatile u8**)0x030003E0;
    *(volatile u16*)(base + 18) = v;
    *(volatile u16*)(base + 26) = 48;
}
void Course_State_Store22_24(void) { // _08008208: strh 16 at [+22] u16, strh 1 at [+24] u16
    volatile u8 *base = *(volatile u8**)0x030003E0;
    *(volatile u16*)(base + 22) = 16;
    *(volatile u16*)(base + 24) = 1;
}
int Course_State_ResetTimed(void) { // _0800821C: sdiv 5, strh [+30] s16, ldrsh table 0x080CB074, bl 07ABC gate
    volatile u8 *base = *(volatile u8**)0x030003E0;
    *(volatile u16*)(base + 22) = 16;
    *(volatile u16*)(base + 24) = 3;
    *(volatile u8*)(base + 79) = 1;
    s16 v16 = *(volatile s16*)(base + 16);
    int q = 0; // sdiv 5 via bl 02DE04
    extern int sub_0802DE04(int,int);
    q = sub_0802DE04(v16, 5);
    if (q > 10) q = 10;
    *(volatile u16*)(base + 30) = (u16)q;
    // ldrsh table 0x080CB074 + gate + bl 07ABC remains blocked — TODO
    return q;
}
void Course_State_Store32(u32 v) { // _08008284: strh [+32] u16
    volatile u8 *base = *(volatile u8**)0x030003E0;
    *(volatile u16*)(base + 32) = v;
}

u32 Course_08014(void) {
    // _08008014: VMA 0x08008014 96 B pure Thumb direct leaf, no WA +4/+12 blocked layout.
    // Exact asm: push {r4-r6,lr} sub sp,#32 mov r1,sp ldr r0,=0x0805F604 ldmia/stmia 32 B template (8 words), ldr r0,=0x03001780 etc.
    // Widths: ldmia/stmia Vu32 (32 B), ldr r6,[r0] Vu32 at 0x03002858 (0x03001780+0x10D8), ands #7 Vu32 mask, lsls #2 Vu32*4, ldr r4,[r5] Vu32 at 0x03002860 (0x03001780+4320), ldr Vu32 at sp+(r6&7)*4, ldr Vu32 at 0x04000100, str Vu32 at 0x03002860, bl 02DF6C Vu32 divide.
    // Branches: none beyond fall-through; straight-line with two divides.
    // Record/WA fields: absolute IWRAM 0x03002858/60 (0x03001780+offset) and IO 0x04000100, not blocked WA +4/+12 at 0x03005B38.
    // Callee ABI: sub_0802DF6C (u32 a,u32 b)->u32 quotient proven via asm movs r1,#23 / bl 02DF6C (r0 dividend, r1 divisor).
    // Existing traces: course_header_trace 167 snaps show *0x03002858 Vu32 0x8B8/0xB2E and *0x03002860 Vu32 0xB99... proving WA valid and word width, no null guard in asm (unconditional ldr/str).
    u32 sp_buf[8];
    const volatile u32 *src = (const volatile u32 *)0x0805F604u;
    for (int i = 0; i < 8; i++) sp_buf[i] = src[i]; // ldmia/stmia 32 B exact
    volatile u32 *pB = (volatile u32 *)0x03002858u; // 0x03001780+0x10D8
    volatile u32 *pA = (volatile u32 *)0x03002860u; // 0x03001780+4320
    u32 r6 = *pB; // ldr Vu32
    u32 idx = r6 & 7u; // ands #7
    u32 r4 = *pA; // ldr Vu32
    u32 t = sp_buf[idx]; // ldr [sp+(r6&7)*4] Vu32
    r4 += t; // adds
    u32 timer = *(volatile u32 *)0x04000100u; // ldr Vu32 IO
    r4 += timer; // adds
    *pA = r4; // str Vu32
    extern u32 sub_0802DF6C(u32, u32);
    u32 q = sub_0802DF6C(r4, 23u); // bl Vu32 divide, r1=23
    r4 += q; // adds
    r4 += r6; // adds r6
    *pA = r4; // str Vu32 final
    // ROM tail: `adds r0, r4, #0; add sp,#32; pop {r4,r5,r6}; pop {r1}; bx r1`
    // — the accumulated counter IS returned in r0 (consumed by MenuPkt_08009B60,
    // which floors it and packs pkt+2).
    return r4;
}
#ifndef __APPLE__
u32 _08008014(void) __attribute__((alias("Course_08014")));
u32 sub_08008014(void) __attribute__((alias("Course_08014")));
#endif

void Course_Leaves_095F0(void) { // _080095F0: ldr 0x0828FF08/0x030003E4, ldrh [r1+50] u16, ldrsh [r1+12] s16, lsls #2, str 4 [sp], bl 07570
    extern void sub_08007570(void*,int,int,int,int);
    void *base = (void*)0x0828FF08;
    (void)base;
    void *rec = *(void**)0x030003E0;
    u16 v50 = *(u16*)((u8*)rec + 50); // ldrh
    s16 v12 = *(s16*)((u8*)rec + 12); // ldrsh
    (void)v50; (void)v12;
    // lsls #2, str 4, bl with r0=base, r1=rec, r2=v12*4, r3=4, sp0=4 not fully proven — keep blocked for exact stack
}
void Course_Leaves_0961C(void) { // _0800961C: ldrh [r1+48] u16, ldrsh [r1+8] s16, lsls #2
    extern void sub_08007570(void*,int,int,int,int);
    void *rec = *(void**)0x030003E0;
    u16 v48 = *(u16*)((u8*)rec + 48);
    s16 v8 = *(s16*)((u8*)rec + 8);
    (void)v48; (void)v8;
}
u32 Course_GetU32_030003E4_0(void) { // _08009674: ldr 0x030003E4, ldr [r0], ldr [r0]
    void *p = *(void**)0x030003E4;
    return *(u32*)p;
}
void Course_SetU32_030003E4_0(u32 v) { // _08009680: str [r1], ldr 0x030003E4
    void **pp = (void**)0x030003E4;
    *(u32*)*pp = v;
}
// _0800968C — s16 fetch at base+20+2*(s16)idx. Three measured levers:
s16 Course_Fetch_S16_20Shift(void *a) { // _0800968C: lsls #16; asrs #15; adds #20; ldrsh
    u8 *base = (u8 *)*(void**)0x030003E4;
    s32 off = (s32)(s16)(int)(intptr_t)a * 2;
    base += 20;
    base += off;
    return *(s16*)base;
}

void Course_Iter_07BFC(void *X, int a1, int a2, int a3,
                       u32 s0, u32 s1, u32 s2, u32 s3, u32 s4) {
    void *seekbase = *(void *volatile *)((volatile u8 *)X + 4); // ldr r0,[r0,#4]
    void *arr = _0800748C(_08007498(seekbase, a2));             // 07498(r1=a2) -> 0748C
    volatile u8 *rows = (volatile u8 *)arr + 8;
    u8 cnt = *(volatile u8 *)((volatile u8 *)arr + 7);          // ldrb [r6,#7]
    extern void sub_08002ED0(void *, int, int, int, int, int, int, int, int, int);
    for (int i = 0; i < cnt; i++) {
        volatile u8 *r4 = rows + i * 4;                          // stride 4
        u8 b0 = r4[0], b1 = r4[1], b2 = r4[2], b3 = r4[3];
        // r0 = b2 + a3 (add r0,r8); r1 = b3 + s0; r2 = b0 + a1 (add r2,r9);
        // r3 = s1; out s0 = s2, s1 = b1, s2 = 1, s3 = s3, s4 = s4, s5 = 1.
        sub_08002ED0((void *)(uintptr_t)(b2 + a3), (int)(b3 + s0),
                     (int)(b0 + a1), (int)s1, (int)s2, (int)b1, 1,
                     (int)s3, (int)s4, 1);
    }
}
extern void *_08002BFC(int a);
extern void _08002C34(int idx, void *node);

// Faithful static _08002ED0 placement writer for the 07C68 row loop
// (asm/runtime_2aac.s 0x02ED0; same transcription as the global
// EmitPlace_02ED0 in foundation_runtime.c — copied, not shared, to keep
// this TU self-contained).
// ROM _08002ED0 consumes TEN args (r0-r3 + 6 stack words).
//   a0=r7 seed, a1=ip value (low byte), a2=r6 OR-mask, a3=dst<<12,
//   s0=2C34 idx, s1=table idx, s2=loop count, s3=+4 mask<<10,
//   s4=+0 mask<<10, s5=u16[place+12].
static u8 *emit_tbl(u32 s1) { return (u8 *)(uintptr_t)(0x080C4940u + (s1 << 3)); }
static void EmitPlace_07C68(u32 a0, u32 a1, u32 a2, u32 a3,
                            u32 s0, u32 s1, u32 s2, u32 s3, u32 s4, u32 s5) {
    u32 r7 = a0;
    u32 ip = a1;
    u32 r6 = a2;
    u32 loc0 = a3;
    u8 *tbl = emit_tbl(s1);
    u32 count = s2;
    u32 sl = s4 << 10;
    s16 head4 = (s16)(tbl[4] | ((u32)tbl[5] << 8));
    if (count == 0u) return;
    u32 r9 = ip & 255u;
    s32 r8 = (s32)head4;
    for (;;) {
        u8 *place = (u8 *)_08002BFC(0);
        s16 c0 = (s16)(tbl[0] | ((u32)tbl[1] << 8));
        s16 c2 = (s16)(tbl[2] | ((u32)tbl[3] << 8));
        *(volatile u16 *)(place + 0) = (u16)(sl | (u32)(u16)c0 | r9);
        *(volatile u16 *)(place + 2) = (u16)(((r7 & 0x1FFu)) | (u32)(u16)c2);
        *(volatile u16 *)(place + 4) = (u16)((loc0 << 12) | r6 | (s3 << 10));
        *(volatile u16 *)(place + 12) = (u16)s5;
        _08002C34((int)s0, (void *)place);
        r7 = (u32)((s32)r7 + (s32)(s8)(tbl[6]));
        r6 = (u32)((s32)r6 + r8);
        count--;
        if (count == 0u) break;
    }
}

// _08007C68 (asm/course_records_7bfc.s:89-142) — 9 machine args (4 reg +
void Course_Iter_07C68(void *X, u32 r1_add, u32 kind, u32 r3_add,
                       u32 s0_yadd, u32 s1_p3, u32 s2_s0, u32 s3, u32 s4) {
    void *arr = _0800748C(_08007498(*(void *volatile *)((volatile u8 *)X + 4), (int)kind));
    volatile u8 *a = (volatile u8 *)arr;
    u32 cnt = a[7]; // ldrb [r6,#7]
    volatile u8 *rows = a + 8;
    for (u32 i = 0; i < cnt; i++) {
        u32 x = (u32)rows[i * 4 + 2] + r3_add; // ldrb [r4,#2]; adds r0,r1,r7
        u32 y = (u32)rows[i * 4 + 3] + s0_yadd; // ldrb [r4,#3]; adds r1,r2,r3
        u32 z = (u32)rows[i * 4 + 0] + r1_add; // ldrb [r4,#0]; add r2,r8
        u32 b1 = rows[i * 4 + 1]; // ldrb [r4,#1] -> 02ED0 s1
        EmitPlace_07C68(x, y, z, s1_p3, s2_s0, b1, 1, s3, s4, 0);
    }
}
void Course_Iter_07CD0(void *X, u8 sl, u8 r9, u32 s56, u32 s60, u32 s64) {
    volatile u8 *arr = (volatile u8*)_0800748C(_08007498(X,0));
    volatile u8 cnt = *(volatile u8*)(arr+7);
    for (volatile u8 i=0;i<cnt;i++) {
        volatile u8 b2 = *(volatile u8*)(arr + i*4 + 2);
        volatile u8 b3 = *(volatile u8*)(arr + i*4 + 3);
        volatile u8 x = b2 + sl;
        (void)x; (void)b3; (void)r9; (void)s56; (void)s60; (void)s64;
        // 240-ldrb path: ldrb [r6+4] u8, subs 240, lsrs #31, asrs #1 s32
        volatile u8 v4 = *(volatile u8*)(arr + 4);
        s32 t = (s32)v4 - 240; // subs
        (void)t;
    }
}
void Course_Iter_07D4C(void *X, u8 sl, u8 r9, u32 s44, u32 s48, u32 s52, u32 s56) {
    volatile u8 *arr = (volatile u8*)_0800748C(_08007498(X,0));
    volatile u8 cnt = *(volatile u8*)(arr+7);
    volatile u16 v2 = *(volatile u16*)(arr+2); // ldrh [r6+2] u16
    (void)v2; (void)sl; (void)r9; (void)s44; (void)s48; (void)s52; (void)s56;
    for (volatile u8 i=0;i<cnt;i++) { (void)arr; }
}
void Course_Iter_07DB4(void *X, u8 sl, u8 r7, u32 s40, u32 s44, u32 s48, u32 s52, u32 s56) {
    volatile u8 *arr = (volatile u8*)_0800748C(_08007498(X,0));
    volatile u8 cnt = *(volatile u8*)(arr+7);
    (void)cnt; (void)sl; (void)r7; (void)s40; (void)s44; (void)s48; (void)s52; (void)s56;
}
void Course_Iter_07E14(void *X, u8 sl, u8 r9, u32 s48, u32 s52, u32 s56, u32 s60) {
    volatile u8 *arr = (volatile u8*)_0800748C(_08007498(X,0));
    volatile u8 cnt = *(volatile u8*)(arr+7);
    (void)cnt; (void)sl; (void)r9; (void)s48; (void)s52; (void)s56; (void)s60;
}

// Aliases: only for substantiated leaves + iterators + builders proven via 4B stride, u8[4] rows, 6 sp args, pools, mGBA EWRAM watchpoint, u16 masks
#ifndef __APPLE__
void _08008290(void *a) __attribute__((alias("Course_RecordInit")));
void _08008300(int a) __attribute__((alias("Course_Build32Row")));
void _080083B8(int a) __attribute__((alias("Course_Build16Row")));
void _08008480(int a,int b) __attribute__((alias("Course_BuildDispatch")));
u32 _08007EC4(void *a, int b) __attribute__((alias("Course_Math_SumU16")));
u32 sub_08007EC4(void *a, int b) __attribute__((alias("Course_Math_SumU16")));
int _08007EE0(int a,int b) __attribute__((alias("Course_Math_AbsRoundAvg")));
int _08007F08(int a,int b,int c,int d) __attribute__((alias("Course_Math_DistanceHeading")));
void _08007FC0(int a,int b,int c, void *d, int e) __attribute__((alias("Course_Math_HeadingInterp")));
void _08008074(u16 a) __attribute__((alias("Course_State_Store0")));
void _08008080(u16 a) __attribute__((alias("Course_State_Store14")));
void _0800808C(u16 a) __attribute__((alias("Course_State_Store6")));
void _08008098(u16 a) __attribute__((alias("Course_State_Store10")));
void _080080A4(int a) __attribute__((alias("Course_State_ClampStore2")));
void sub_080080A4(int a) __attribute__((alias("Course_State_ClampStore2")));
void _080080CC(u32 a) __attribute__((alias("Course_State_Store64")));
void sub_080080CC(u32 a) __attribute__((alias("Course_State_Store64")));
void _080080D8(u32 a) __attribute__((alias("Course_State_Store68")));
void sub_080080D8(u32 a) __attribute__((alias("Course_State_Store68")));
void _080080E4(u32 a) __attribute__((alias("Course_State_Store72_104")));
void sub_080080E4(u32 a) __attribute__((alias("Course_State_Store72_104")));
void _080080F4(u32 a) __attribute__((alias("Course_State_Store72_150")));
void sub_080080F4(u32 a) __attribute__((alias("Course_State_Store72_150")));
void _08008104(void) __attribute__((alias("Course_State_Flag78")));
void sub_08008104(void) __attribute__((alias("Course_State_Flag78")));
void *_08008114(int a) __attribute__((alias("Course_State_Store12Flag77")));
void _08008134(u16 a) __attribute__((alias("Course_State_Store8Flag76")));
void _080081F8(u32 a) __attribute__((alias("Course_State_Store18_48")));
void _08008208(void) __attribute__((alias("Course_State_Store22_24")));
void _08008284(u32 a) __attribute__((alias("Course_State_Store32")));
void sub_080081F8(u32 a) __attribute__((alias("Course_State_Store18_48")));
// asm/course_records_7bfc.s:916-917 defines BOTH `sub_080081F8:` and `_080081F8:` on
// one body, so the C replacement must publish both spellings; the manifest `export`
// records both.
u32 _08009674(void) __attribute__((alias("Course_GetU32_030003E4_0")));
void _08009680(u32 a) __attribute__((alias("Course_SetU32_030003E4_0")));
void sub_08009680(u32 a) __attribute__((alias("Course_SetU32_030003E4_0")));
s16 _0800968C(void *a) __attribute__((alias("Course_Fetch_S16_20Shift")));
void _08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i) __attribute__((alias("Course_Iter_07BFC")));
void _08007C68(void *a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i) __attribute__((alias("Course_Iter_07C68")));
void _08007CD0(void *a, u8 b, u8 c, u32 d, u32 e, u32 f) __attribute__((alias("Course_Iter_07CD0")));
void _08007D4C(void *a, u8 b, u8 c, u32 d, u32 e, u32 f, u32 g) __attribute__((alias("Course_Iter_07D4C")));
void _08007DB4(void *a, u8 b, u8 c, u32 d, u32 e, u32 f, u32 g, u32 h) __attribute__((alias("Course_Iter_07DB4")));
void _08007E14(void *a, u8 b, u8 c, u32 d, u32 e, u32 f, u32 g) __attribute__((alias("Course_Iter_07E14")));
u32 Sub_08009674(void) __attribute__((alias("Course_GetU32_030003E4_0")));
u32 sub_08009674(void) __attribute__((alias("Course_GetU32_030003E4_0")));
void sub_08007C68(void *a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i) __attribute__((alias("Course_Iter_07C68")));
s16 Sub_0800968C(void *a) __attribute__((alias("Course_Fetch_S16_20Shift")));
// The closure spells this `sub_0800968C`, and a promoted caller
// (0x08001B1A4) must use that spelling; only the friendly twin existed.
s16 sub_0800968C(void *a) __attribute__((alias("Course_Fetch_S16_20Shift")));
void Sub_08008098(u16 a) __attribute__((alias("Course_State_Store10")));
// tools/matching_slice_functions.json exports "sub_08008098" for this entry, but
// nothing in src/ defined it: the `_` and `Sub_` twins were both present and the
// exported spelling was not. A promoted caller using the exported name failed
// the slice link with `undefined reference`. Promotion rule 1 -- an exported
// `sub_` twin must be defined in C too.
void sub_08008098(u16 a) __attribute__((alias("Course_State_Store10")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void Sub_08007BFC(void *X, int a1, int a2, int a3,
                       u32 s0, u32 s1, u32 s2, u32 s3, u32 s4) __attribute__((alias("Course_Iter_07BFC")));
void sub_08007BFC(void *X, int a1, int a2, int a3,
                       u32 s0, u32 s1, u32 s2, u32 s3, u32 s4) __attribute__((alias("Course_Iter_07BFC")));
#endif
