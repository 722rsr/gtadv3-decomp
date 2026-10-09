#include "gtadv/ai_race.h"
#include "gba/types.h"

// ---- ring helpers ----
void Ai_RingReset(void){
    volatile u32 *base=(volatile u32*)(uintptr_t)0x030005B0u;
    base[0]=0;
    base[1]=0xFFFFFFFFu; // -1
    base[2]=0;
}
#ifndef __APPLE__
void _08023FE4(void) __attribute__((alias("Ai_RingReset")));
#endif

void Ai_RingPush(int ev, int arg) {
    register int i __asm__("r4") = 0;
    register int *base __asm__("r0");
    register int *b __asm__("r3");
    register int first __asm__("r1");
    register int neg1 __asm__("r2");
    base = (int *)(uintptr_t)0x030005B0u;
    __asm__("" : "+r"(base));
    first = base[1];
    neg1 = -1;
    b = base;
    if (first != neg1) {
        const int *p = b + 1;
        do {
            p += 2;
            i++;
            if (i > 9)
                break;
        } while (*p != neg1);
    }
    {
        int off = i * 8;
        int *base_ev = b + 1;
        *(int *)((uintptr_t)off + (uintptr_t)base_ev) = ev;
        int *base_arg = b + 2;
        *(int *)((uintptr_t)off + (uintptr_t)base_arg) = arg;
        register int next_off __asm__("r0") = (i + 1) * 8;
        *(int *)((uintptr_t)next_off + (uintptr_t)base_ev) = -1;
        *(int *)((uintptr_t)next_off + (uintptr_t)base_arg) = 0;
    }
}
#ifndef __APPLE__
void _08023FF8(int a,int b) __attribute__((alias("Ai_RingPush")));
void Scene_PostEvent(int a,int b) __attribute__((alias("Ai_RingPush")));
#endif

// _08023958 — 121B phase pump, 6 BL sites, tier gate wa+0x10C3
void _08023958(void *ctx, int a, int b) {
    register u8 *c __asm__("r5") = (u8 *)ctx;
    u16 rb = (u16)b;
    u16 *p158 = (u16 *)(c + 158);
    u16 *p164;
    register u16 *p162 __asm__("r2");
    extern void _08002158(int, int);
    extern void _0802B368(int);
    extern u8 AiWaBase[];
    extern u8 AiWaOff[];
    __asm__(".globl AiWaBase\nAiWaBase = 0x03001780\n");
    __asm__(".globl AiWaOff\nAiWaOff = 0x10C3\n");

    if ((u16)(*p158 - 5) > 3) {
        _08002158(4, *p158);
    }
    p164 = (u16 *)(c + 164);
    if (*p158 == 1) {
        if (rb == 2) {
            _0802B368(4);
            {
                s16 *p166 = (s16 *)(c + 166);
                if (*p166 == 0)
                    *p166 = rb;
                *p164 = rb;
            }
        }
        if (rb == 1) {
            register s16 *r0_p __asm__("r0") = (s16 *)(c + 166);
            register int val __asm__("r1");
            register s16 *p166 __asm__("r4");
            register int r2_zero __asm__("r2");
            __asm__("" : "+r"(r0_p));
            __asm__("movs %3, #0\n\tldrsh %0, [%1, %3]\n\tmov %2, %1"
                    : "=r"(val), "+r"(r0_p), "=r"(p166), "=r"(r2_zero));
            if (val == 0)
                *p166 = 2;
            {
                register int v __asm__("r0") = *(u16 *)p166;
                if (v == 1)
                    _0802B368(1);
                else
                    _0802B368(4);
            }
            *p164 = *(u16 *)p166;
        }
    }
    {
        s16 v158 = *(s16 *)(c + 158);
        if (v158 == 3 || v158 == 10 || ((p162 = (u16 *)(c + 162)), v158 == 11)) {
            u16 r = (u16)(rb - 1);
            __asm__("" : "+r"(c));
            p162 = (u16 *)(c + 162);
            if (r <= 1) {
                register u8 *base __asm__("r0") = AiWaBase;
                register u32 off __asm__("r1") = (uintptr_t)AiWaOff;
                if (*(base + off) == 1)
                    *p162 = 1;
            }
        }
    }
    if (rb == 32)
        *(s16 *)(c + 166) = 2;
    if (rb == 16)
        *(s16 *)(c + 166) = 1;
    _08002158(6, *p162);
    _08002158(5, *p164);
    __asm__("" :: "r"(c));
}

// _08023ED0 — 12-way record-42 handler, table at 0x08023EF0
void _08023ED0(int ev, int a, int b, void *ctx) {
    switch (ev) {
        case 2: {
            extern void _08022CB4(void *, int);
            _08022CB4(ctx, a);
            break;
        }
        case 5: {
            extern void _0800D854(void *);
            extern void _0800D8E4(void *);
            extern void _080235F4(void *);
            _0800D854((u8 *)ctx + 16);
            _0800D8E4((u8 *)ctx + 120);
            _080235F4(ctx);
            break;
        }
        case 7: {
            s16 phase = *(s16 *)((u8 *)ctx + 142);
            switch (phase) {
                case 0: { extern void _08023BD4(void *); _08023BD4(ctx); break; }
                case 1: { extern void _08023D4C(void *); _08023D4C(ctx); break; }
                case 2: { extern void _08023E0C(void *); _08023E0C(ctx); break; }
            }
            break;
        }
        case 6: {
            extern void _08023E7C(void *);
            _08023E7C(ctx);
            if (*(u16 *)((u8 *)ctx + 20) == 0)
                break;
            {
                s16 phase = *(s16 *)((u8 *)ctx + 142);
                switch (phase) {
                    case 0: { extern void _0802381C(void *, int, int); _0802381C(ctx, (u16)a, (u16)b); break; }
                    case 1: { _08023958(ctx, (u16)a, (u16)b); break; }
                    case 2: { extern void _08023A34(void *, int, int); _08023A34(ctx, (u16)a, (u16)b); break; }
                }
            }
            break;
        }
        case 1: {
            extern void _08023628(void *);
            _08023628(ctx);
            break;
        }
        case 12: {
            extern void _08022D20(void *);
            _08022D20(ctx);
            break;
        }
        default:
            break;
    }
}
__asm__(".align 2, 0");

// _0800AA40 — race FSM, 8-way at +0xFBC — BLOCKED: placeholder 43-event logic incomplete
// Exact asm requires slot field at +0x100A+ b*2+a*8 via ldrsh, tier gate at +0x10E5 (ldrb s8),
// place-change predicates: [sp+8] (slot) vs r8/r9 stacks, wa+0x1074 gate, grid deltas via
// _0800A9A0/_0800A9E0, and 8-way table _0800AAFC (cases 0/1/2/3/5/7 + tails 4/6). Current C
// simplified case 0 fires unconditional _08023FF8(43,0) without wa+0x1074/ slot==10 / r5 vs sl
// checks — not instruction-backed. Leave asm untouched (asm/ai_racefsm.s), no T claim.
// Concrete blocker: need ramwatch of wa+0x1074 s16, wa+0xFF0 one-shot, and stack slot at
// wa+0x28xx before claiming 38 vs 43 vs 49 paths. No weak alias retained.

// _0800B4A8 — collection manager: triples via _08024F4C/_08024F74/_08024F9C, grants via _08024FC4/_08024FE8, grid scans
void _0800B4A8(void){
    volatile u8 *wa = (volatile u8*)(uintptr_t)0x03001780u;
    s16 a = *(volatile s16*)(wa+0xFF2);
    s16 b = *(volatile s16*)(wa+0xFF8);
    // selector C from record array +0x100A
    s16 c = *(volatile s16*)(wa+0x100A + (b*2 + a*8));
    extern s16 _08024F4C(s16,s16,s16);
    extern s16 _08024F74(s16,s16,s16);
    extern s16 _08024F9C(s16,s16,s16);
    extern s16 _08024FC4(s16,s16,s16);
    extern s16 _08024FE8(s16,s16,s16);
    extern int _08025FAC(int);
    extern void _08025F78(int);
    extern void _08026020(int);
    extern int _08025F20(int);
    extern void _08025EC0(int,int);
    extern int _08025CF4(int,int,int);
    extern void _08023FF8(int,int);
    extern int _08024C90(int);
    s16 id0 = _08024F4C(a,b,c);
    s16 id1 = _08024F74(a,b,c);
    s16 id2 = _08024F9C(a,b,c);
    s16 limit = *(volatile s16*)(wa+0x1054);
    s8 tier = *(volatile s8*)(wa+0x10E5);
    if(id0!=-1 && tier>=id2 && limit < id2){
        if(id0<=2){
            *(volatile s16*)(wa+0x1046)=id0;
            int cnt = _08025F20(id0)+1;
            if(cnt>3) cnt=3;
            *(volatile s16*)(wa+0x1048)=cnt;
            if(cnt<=3){
                *(volatile s16*)(wa+0x105A)=1;
                _08025EC0(id0,cnt);
                _08023FF8(27,0);
            }
        } else {
            if(_08025F20(id0) < id1){
                *(volatile s16*)(wa+0x1046)=id0;
                *(volatile s16*)(wa+0x1048)=id1;
                *(volatile s16*)(wa+0x105A)=1;
                _08025EC0(id0,id1);
                _08023FF8(27,0);
            }
        }
    }
    s16 gid = _08024FC4(a,b,c);
    s16 gtag = _08024FE8(a,b,c);
    if(gid!=-1 && !_08025FAC(gid) && tier>=gtag){
        *(volatile s16*)(wa+0x1058)=1;
        int idx = _08024C90(gid);
        *(volatile s16*)(wa+0x1060 + idx*2)=1;
        s16 cnt = *(volatile s16*)(wa+0x103C);
        *(volatile u8*)(wa + 0x103E + cnt)= (u8)gid;
        _08025F78(gid);
        _08023FF8(28,0);
        *(volatile s16*)(wa+0x103C)= cnt+1;
    }
    // grid award scans (4×11) — faithful two-call per cell as per objdump: first lsls24 cmp 0 (beq skip), second lsls24/lsrs24 cmp 3 (bhi skip)
    int c0=0,c1=0;
    for(int rr=0;rr<4;++rr) for(int cc=0;cc<11;++cc){
        int a0 = _08025CF4(0,rr,cc); // first call: lsls r0,#24; cmp #0; beq skip
        if((a0<<24)!=0){
            int b0 = _08025CF4(0,rr,cc); // second call: lsls #24; lsrs #24; cmp #3
            if(((b0<<24)>>24)<=3) c0++;
        }
        int a1 = _08025CF4(1,rr,cc);
        if((a1<<24)!=0){
            int b1 = _08025CF4(1,rr,cc);
            if(((b1<<24)>>24)<=3) c1++;
        }
    }
    if(c0>43 && !_08025FAC(94)){ _08026020(0); *(volatile s16*)(wa+0x1076)=1; {s16 cnt=*(volatile s16*)(wa+0x103C); *(volatile u8*)(wa + 0x103E + cnt)=94; _08025F78(94); _08023FF8(28,0); *(volatile s16*)(wa+0x103C)=cnt+1;}}
    if(c1>43 && !_08025FAC(95)){ _08026020(1); *(volatile s16*)(wa+0x1076)=1; {s16 cnt=*(volatile s16*)(wa+0x103C); *(volatile u8*)(wa + 0x103E + cnt)=95; _08025F78(95); _08023FF8(28,0); *(volatile s16*)(wa+0x103C)=cnt+1;}}
    int c1e=0; for(int rr=0;rr<4;++rr) for(int cc=0;cc<11;++cc) if((( _08025CF4(1,rr,cc)<<24)>>24)==3) c1e++;
    if(c1e>43 && !_08025FAC(96)){ _08026020(2); *(volatile s16*)(wa+0x1076)=1; {s16 cnt=*(volatile s16*)(wa+0x103C); *(volatile u8*)(wa + 0x103E + cnt)=96; _08025F78(96); _08023FF8(28,0); *(volatile s16*)(wa+0x103C)=cnt+1;}}
    int c0e=0; for(int rr=0;rr<4;++rr) for(int cc=0;cc<11;++cc) if((( _08025CF4(0,rr,cc)<<24)>>24)==3) c0e++;
    if(c0e>43 && *(volatile s16*)(wa+0xFF0)==0){ *(volatile s16*)(wa+0xFF0)=1; _08023FF8(29,0); }
}

// Leaf proved for wa+0x1054 s16 adjacency to _0800B4A8 — exact asm plus ramwatch
// Asm: ldr r3,=0x00001054; adds r0, r7, r3; movs r3,#0; ldrsh r2,[r0,r3] (s16, offset 0) then cmp r2,r1 (id2 s16 via lsls16/asrs16) with bge (signed)
// Ramwatch headless 2500 frames (interval 100, baserom.gba inputs.csv → /tmp/caps1054b): IWRAM 0x030027D4 (wa+0x1054) stable 0x0000 s16 0 / tier 0, width s16 via ldrsh not ldrh, no racectx +0x490/+0x4E0 invented
s16 Ai_B4A8_GetLimit(void){
    volatile u8 *wa = (volatile u8*)(uintptr_t)0x03001780u;
    return *(volatile s16*)(wa+0x1054);
}

// ROM entry alias.
#ifndef __APPLE__
void _080023FE4(void) __attribute__((alias("Ai_RingReset")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void _080023FF8(int ev,int arg) __attribute__((alias("Ai_RingPush")));
#endif
