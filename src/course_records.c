#include "gtadv/course_records.h"
#include "gba/bios.h"

// Reference: asm/course_records_7bfc.s, course_builders_8290.s, course_dispatch_8aac.s, course_leaves_95f0.s

extern void *_08007498(void *X,int i);
extern void *_0800748C(void *X);
extern void _08003004(int a,int b,int c,int d,int e,int f,int g,int h);
extern void _08002ED0(void *a, int b, int c, int d, int e, int f, int g, int h, int i, int j);
extern void _08002E0A4(void *a,void *b,int c);
extern int _08002BE8(void);
extern int _08002C48(int a, int b);
extern u32 sub_0802DF6C(u32 numerator, u32 denominator);

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
    // _08008290: DMA setup, 32 halfwords of 0x2A8 + table bytes, then 12 pairs of CpuSet copies.
    extern void sub_080078EC(void *,void *,int);
    extern void *sub_08007924(void *,int);
    extern void sub_0802D974(const void*,void*,u32);
    u16 frame[32];
    void *base = (void *)0x0828FF08u;
    register u8 *src asm("r2");
    register u16 *dst asm("r1");
    register u32 value asm("r3");
    register int count asm("r4");
    register u8 *dst_a asm("r6");
    register u8 *dst_b asm("r5");
    register u32 sum asm("r0");
    (void)rec;
    sub_080078EC(base,(void *)(uintptr_t)9,0x0600D500);
    src = (u8 *)sub_08007924(base,9);
    __asm__ volatile(
        ".code 16\n\t"
        ".short 0x20aa\n\t"
        ".short 0x0080\n\t"
        ".short 0x1c03\n"
        : "=r"(value) : : "r0");
    dst = frame;
    count = 31;
    do {
        __asm__ volatile(
            ".code 16\n\t"
            ".short 0x7815\n\t"
            ".short 0x1958\n"
            : "=r"(sum) : "r"(src), "r"(value) : "r5");
        *dst++ = (u16)sum;
    } while (--count >= 0);
    dst_a = (u8 *)0x0600FA00u;
    dst_b = (u8 *)0x0600F200u;
    count = 11;
    do {
        sub_0802D974(frame,dst_b,32);
        sub_0802D974(frame,dst_a,32);
        dst_a += 64;
        dst_b += 64;
    } while (--count >= 0);
}
__attribute__((naked)) void Course_Build32Row(int sel){
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, sl\n"
        "mov r6, r9\n"
        "mov r5, r8\n"
        "push {r5, r6, r7}\n"
        "sub sp, #68\n"
        "lsls r0, r0, #24\n"
        "movs r5, #8\n"
        "cmp r0, #0\n"
        "beq .L8300_316\n"
        "movs r5, #7\n"
        ".L8300_316:\n"
        "ldr r4, .L8300_3a4\n"
        "ldr r2, .L8300_3a8\n"
        "adds r0, r4, #0\n"
        "adds r1, r5, #0\n"
        "bl 0x080078EC\n"
        "adds r0, r4, #0\n"
        "adds r1, r5, #0\n"
        "movs r2, #13\n"
        "bl 0x08007938\n"
        "adds r0, r4, #0\n"
        "adds r1, r5, #0\n"
        "bl 0x08007924\n"
        "str r0, [sp, #64]\n"
        "movs r4, #0\n"
        "ldr r0, .L8300_3ac\n"
        "mov sl, r0\n"
        "movs r1, #192\n"
        "lsls r1, r1, #4\n"
        "mov r9, r1\n"
        "ldr r0, .L8300_3b0\n"
        "mov r8, r0\n"
        "movs r1, #208\n"
        "lsls r1, r1, #8\n"
        "adds r7, r1, #0\n"
        ".L8300_34c:\n"
        "movs r0, #22\n"
        "muls r0, r4\n"
        "ldr r1, [sp, #64]\n"
        "adds r0, r1, r0\n"
        "mov r1, sp\n"
        "movs r2, #11\n"
        "bl 0x0802D974\n"
        "mov r3, sp\n"
        "adds r5, r4, #0\n"
        "adds r5, #14\n"
        "adds r6, r4, #1\n"
        "movs r4, #31\n"
        ".L8300_366:\n"
        "ldrh r2, [r3, #0]\n"
        "mov r1, sl\n"
        "ands r1, r2\n"
        "mov r0, r9\n"
        "ands r0, r2\n"
        "add r1, r8\n"
        "orrs r0, r7\n"
        "orrs r1, r0\n"
        "strh r1, [r3, #0]\n"
        "subs r4, #1\n"
        "adds r3, #2\n"
        "cmp r4, #0\n"
        "bge .L8300_366\n"
        "lsls r1, r5, #6\n"
        "ldr r0, .L8300_3b4\n"
        "adds r1, r1, r0\n"
        "mov r0, sp\n"
        "movs r2, #11\n"
        "bl 0x0802D974\n"
        "adds r4, r6, #0\n"
        "cmp r4, #5\n"
        "ble .L8300_34c\n"
        "add sp, #68\n"
        "pop {r3, r4, r5}\n"
        "mov r8, r3\n"
        "mov r9, r4\n"
        "mov sl, r5\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        ".L8300_3a4: .4byte 0x0828FF08\n"
        ".L8300_3a8: .4byte 0x0600D520\n"
        ".L8300_3ac: .4byte 0x000003FF\n"
        ".L8300_3b0: .4byte 0x000002A9\n"
        ".L8300_3b4: .4byte 0x0600F026\n"
        ".syntax divided\n"
    );
}
void Course_Build16Row(int sel){
    typedef struct { u32 w[4]; } Row16Template;
    typedef struct {
        u16 pixels[32];
        u32 selectors[4];
        void *base;
    } Row16Frame;
    extern void sub_080078EC(void*,void*,int);
    extern void *sub_08007938(void*,int,int);
    extern void *sub_08007924(void*,int);
    extern void sub_0802D974(const void*,void*,u32);
    Row16Frame frame;
    u32 *selectors = frame.selectors;
    *(Row16Template *)selectors =
        *(const Row16Template *)(uintptr_t)0x0805F624u;
    u32 *selector = selectors;
    selector += sel;
    int r5 = (int)*selector;
    void *template_base = (void *)0x0828FF08u;
    sub_080078EC(template_base, (void *)(uintptr_t)r5, 0x0600DB60);
    sub_08007938(template_base, r5, 12);
    frame.base = sub_08007924(template_base, r5);
    u32 r7_mask;
    __asm__ volatile(
        ".code 16\n\t"
        "movs r4, #0\n\t"
        "ldr r0, =0x000003FF\n\t"
        "mov sl, r0\n\t"
        "movs r1, #192\n\t"
        "lsl r1, r1, #4\n\t"
        "mov r9, r1\n\t"
        "ldr r5, =0x000002DB\n\t"
        "mov r8, r5\n\t"
        "movs r6, #192\n\t"
        "lsl r6, r6, #8\n\t"
        "add r7, r6, #0\n"
        "1:\n\t"
        "lsl r0, r4, #4\n\t"
        "sub r0, r0, r4\n\t"
        "lsl r0, r0, #1\n\t"
        "ldr r1, [sp, #80]\n\t"
        "add r0, r1, r0\n\t"
        "mov r1, sp\n\t"
        "movs r2, #15\n\t"
        "bl sub_0802D974\n\t"
        "mov r3, sp\n\t"
        "add r5, r4, #0\n\t"
        "add r5, #18\n\t"
        "add r6, r4, #1\n\t"
        "movs r4, #31\n"
        "2:\n\t"
        "ldrh r2, [r3, #0]\n\t"
        "mov r1, sl\n\t"
        "and r1, r2\n\t"
        "mov r0, r9\n\t"
        "and r0, r2\n\t"
        "add r1, r8\n\t"
        "orr r0, r7\n\t"
        "orr r1, r0\n\t"
        "strh r1, [r3, #0]\n\t"
        "sub r4, #1\n\t"
        "add r3, #2\n\t"
        "cmp r4, #0\n\t"
        "bge 2b\n\t"
        "lsl r1, r5, #6\n\t"
        "ldr r4, =0x0600F000\n\t"
        "add r1, r1, r4\n\t"
        "mov r0, sp\n\t"
        "movs r2, #15\n\t"
        "bl sub_0802D974\n\t"
        "add r4, r6, #0\n\t"
        "cmp r4, #1\n\t"
        "ble 1b\n"
        : "=r"(r7_mask)
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6",
          "r8", "r9", "r10", "r11", "r12", "lr", "cc", "memory");
    __asm__ volatile("" : : "r"(r7_mask));
}
__attribute__((naked)) void Course_BuildDispatch(int mode,int sel) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, lr}\n"
        "sub sp, #4\n"
        "adds r3, r0, #0\n"
        "lsls r4, r1, #24\n"
        "lsrs r4, r4, #24\n"
        "ldr r0, .Lcr08480_080084ac\n"
        "str r3, [r0, #0]\n"
        "movs r0, #0\n"
        "str r0, [sp, #0]\n"
        "ldr r2, .Lcr08480_080084b0\n"
        "mov r0, sp\n"
        "adds r1, r3, #0\n"
        "bl sub_0802D974\n"
        "lsls r4, r4, #24\n"
        "asrs r4, r4, #24\n"
        "cmp r4, #0\n"
        "beq .Lcr08480_080084b4\n"
        "cmp r4, #1\n"
        "beq .Lcr08480_08008502\n"
        "b .Lcr08480_0800850e\n"
        ".align 2, 0\n"
        ".Lcr08480_080084ac: .word 0x030003E0\n"
        ".Lcr08480_080084b0: .word 0x0500001A\n"
        ".Lcr08480_080084b4:\n"
        "movs r0, #1\n"
        "bl sub_08008300\n"
        "ldr r0, .Lcr08480_080084d4\n"
        "ldr r1, .Lcr08480_080084d8\n"
        "adds r0, r0, r1\n"
        "movs r1, #0\n"
        "ldrsh r0, [r0, r1]\n"
        "cmp r0, #2\n"
        "beq .Lcr08480_080084ea\n"
        "cmp r0, #2\n"
        "bgt .Lcr08480_080084dc\n"
        "cmp r0, #0\n"
        "beq .Lcr08480_080084e2\n"
        "b .Lcr08480_080084fa\n"
        ".align 2, 0\n"
        ".Lcr08480_080084d4: .word 0x03001780\n"
        ".Lcr08480_080084d8: .word 0x000010FC\n"
        ".Lcr08480_080084dc:\n"
        "cmp r0, #5\n"
        "beq .Lcr08480_080084f2\n"
        "b .Lcr08480_080084fa\n"
        ".Lcr08480_080084e2:\n"
        "movs r0, #0\n"
        "bl sub_080083B8\n"
        "b .Lcr08480_0800850e\n"
        ".Lcr08480_080084ea:\n"
        "movs r0, #2\n"
        "bl sub_080083B8\n"
        "b .Lcr08480_0800850e\n"
        ".Lcr08480_080084f2:\n"
        "movs r0, #3\n"
        "bl sub_080083B8\n"
        "b .Lcr08480_0800850e\n"
        ".Lcr08480_080084fa:\n"
        "movs r0, #1\n"
        "bl sub_080083B8\n"
        "b .Lcr08480_0800850e\n"
        ".Lcr08480_08008502:\n"
        "movs r0, #0\n"
        "bl sub_08008300\n"
        "movs r0, #0\n"
        "bl sub_080083B8\n"
        ".Lcr08480_0800850e:\n"
        "movs r0, #9\n"
        "bl sub_080261B0\n"
        "ldr r4, .Lcr08480_08008608\n"
        "ldr r1, [r4, #0]\n"
        "strh r0, [r1, #36]\n"
        "movs r0, #9\n"
        "bl 0x080261F8\n"
        "ldr r1, [r4, #0]\n"
        "strh r0, [r1, #38]\n"
        "movs r0, #9\n"
        "bl sub_080261B0\n"
        "ldr r1, [r4, #0]\n"
        "adds r0, #43\n"
        "strh r0, [r1, #48]\n"
        "movs r0, #9\n"
        "bl sub_080261B0\n"
        "ldr r1, [r4, #0]\n"
        "adds r0, #47\n"
        "strh r0, [r1, #50]\n"
        "movs r0, #15\n"
        "bl sub_080261B0\n"
        "ldr r1, [r4, #0]\n"
        "strh r0, [r1, #52]\n"
        "movs r0, #15\n"
        "bl 0x080261F8\n"
        "ldr r1, [r4, #0]\n"
        "strh r0, [r1, #54]\n"
        "ldr r5, .Lcr08480_0800860c\n"
        "adds r1, #88\n"
        "adds r0, r5, #0\n"
        "bl sub_0800798C\n"
        "ldr r1, [r4, #0]\n"
        "adds r0, r1, #0\n"
        "adds r0, #88\n"
        "ldrh r1, [r1, #36]\n"
        "bl sub_08007A04\n"
        "ldr r0, [r4, #0]\n"
        "ldrh r2, [r0, #38]\n"
        "adds r0, r5, #0\n"
        "movs r1, #2\n"
        "bl sub_080075E8\n"
        "ldr r0, [r4, #0]\n"
        "adds r0, #77\n"
        "movs r1, #1\n"
        "strb r1, [r0, #0]\n"
        "ldr r0, [r4, #0]\n"
        "adds r0, #76\n"
        "strb r1, [r0, #0]\n"
        "ldr r0, [r4, #0]\n"
        "adds r0, #88\n"
        "movs r1, #10\n"
        "bl sub_08007EC4\n"
        "adds r1, r0, #0\n"
        "ldr r0, [r4, #0]\n"
        "strh r1, [r0, #44]\n"
        "adds r0, #88\n"
        "movs r1, #11\n"
        "bl sub_08007EC4\n"
        "ldr r1, [r4, #0]\n"
        "strh r0, [r1, #46]\n"
        "ldr r0, .Lcr08480_08008610\n"
        "ldr r1, .Lcr08480_08008614\n"
        "adds r0, r0, r1\n"
        "ldrh r0, [r0, #0]\n"
        "cmp r0, #5\n"
        "bne .Lcr08480_08008600\n"
        "movs r0, #3\n"
        "bl sub_080261B0\n"
        "ldr r2, [r4, #0]\n"
        "strh r0, [r2, #56]\n"
        "adds r1, r0, #0\n"
        "adds r1, #32\n"
        "strh r1, [r2, #60]\n"
        "adds r1, #32\n"
        "strh r1, [r2, #40]\n"
        "adds r0, #104\n"
        "strh r0, [r2, #42]\n"
        "movs r0, #2\n"
        "bl sub_080261B0\n"
        "ldr r1, [r4, #0]\n"
        "strh r0, [r1, #62]\n"
        "movs r0, #3\n"
        "bl 0x080261F8\n"
        "ldr r1, [r4, #0]\n"
        "strh r0, [r1, #58]\n"
        "ldr r0, .Lcr08480_08008618\n"
        "adds r1, #96\n"
        "bl sub_0800798C\n"
        "ldr r1, [r4, #0]\n"
        "ldr r0, [r1, #100]\n"
        "ldrh r2, [r1, #60]\n"
        "movs r1, #10\n"
        "bl sub_08007ABC\n"
        "ldr r0, [r4, #0]\n"
        "ldrh r2, [r0, #40]\n"
        "adds r0, r5, #0\n"
        "movs r1, #0\n"
        "bl sub_08007538\n"
        "ldr r0, [r4, #0]\n"
        "ldrh r2, [r0, #42]\n"
        "adds r0, r5, #0\n"
        "movs r1, #1\n"
        "bl sub_08007538\n"
        ".Lcr08480_08008600:\n"
        "add sp, #4\n"
        "pop {r4, r5}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        ".Lcr08480_08008608: .word 0x030003E0\n"
        ".Lcr08480_0800860c: .word 0x0828FF08\n"
        ".Lcr08480_08008610: .word 0x03001780\n"
        ".Lcr08480_08008614: .word 0x000010FC\n"
        ".Lcr08480_08008618: .word 0x0828C420\n"
        ".syntax divided\n"
    );
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
int Course_Math_DistanceHeading(int x0,int y0,int x1,int y1) { // _08007F08: signed deltas, unsigned normalized division, signed table result
    // Keep the ROM's argument and result lifetimes in r7/r5/r6; the barriers
    // preserve its entry copies and the one-value scratch register.
    register int y0_input __asm__("r7") = y0;
    register int x1_input __asm__("r5") = x1;
    register int y1_input __asm__("r6") = y1;
    __asm__ volatile ("" : "+r" (y0_input), "+r" (x1_input), "+r" (y1_input));

    register int dx __asm__("r5") = x1_input - x0;
    int sx;
    if (dx < 0) { dx = -dx; sx = 0; }
    else {
        register int one __asm__("r1") = 1;
        __asm__ volatile ("" : "+r" (one));
        sx = one;
    }

    register int dy __asm__("r6") = y1_input - y0_input;
    int sy;
    if (dy < 0) { dy = -dy; sy = 0; }
    else { sy = 1; }

    if (dx == 0 && dy == 0) return 0;

    int sq = dx * dx + dy * dy;
    u16 root = (u16)Sqrt((u32)sq);
    int rt = (int)root;
    register int rx __asm__("r5") =
        (int)sub_0802DF6C((u32)dx << 11, (u32)rt);
    register int ry __asm__("r6") =
        (int)sub_0802DF6C((u32)dy << 11, (u32)rt);

    const s16 *tbl;
    register u32 index2 __asm__("r0");
    if (rx <= ry) {
        tbl = (const s16 *)0x080C9064;
        index2 = (u32)rx << 1;
    } else {
        tbl = (const s16 *)0x080CA064;
        index2 = (u32)ry << 1;
    }
    register int v __asm__("r5") =
        *(s16 *)(uintptr_t)(index2 + (uintptr_t)tbl);

    if (sx == 0) {
        if (sy) v += 1024;
        else v = 1024 - v;
    } else {
        if (sy) v = 3072 - v;
        else v += 3072;
    }
    return v;
}
// ROM has a zero-filled alignment halfword after the return instruction.
__asm__(".align 2, 0");
int Course_Math_HeadingInterp(int p0, int p1, int p2, void *out) {
    register void *o __asm__("r5") = out;
    register int a4 __asm__("r4");
    register int clamped __asm__("r6");
    register u32 c __asm__("r1");
    u32 p1s;
    u32 p0w = (u32)p0 << 16;
    p1s = (u16)p1;
    c = (u16)p2;
    clamped = 0;
    a4 = (int)(p0w >> 16);
    int p0s = (int)p0w >> 16;
    if (p0s > 7) { a4 = 7; clamped = 1; }
    c = (u32)((s16)c - (int)(p1s = (u32)(s32)(s16)p1s));
    {
        s16 *table = (s16 *)0x080CB064u;
        // Materialize the literal base before the signed index scaling, in r2.
        __asm__ volatile ("" : "+r" (table));
        int tv = table[(s16)a4];
        // Keep the selected index live through the ldrsh address calculation.
        __asm__ volatile ("" : "+r" (a4));
        int res = tv * (int)c;
        if (res < 0) res += 0xFFF;
        res >>= 12;
        // Thumb's three-register ADD must encode the ROM's r3+r0 operand order.
        register int accum __asm__("r0") = res;
        register int start __asm__("r3") = (int)p1s;
        __asm__ volatile ("add %0, %1, %0"
                          : "+r" (accum) : "r" (start) : "cc");
        res = accum;
        *(volatile s16 *)o = (s16)res;
    }
    return clamped;
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
    // _08008014: VMA 0x08008014 96 B pure Thumb straight-line leaf.
    // The 8-word template copy is a struct assignment (3+3+2 ldmia/stmia
    // with writeback, no call); a word loop emits single-word ldr/str.
    // pA comes first as base+(135<<5) (constructed 4320) and pB as
    // base+pool 0x10D8 -- not as two absolute pool addresses. The slot
    // address is computed before r4 is loaded (`ldr r4,[r5]` follows the
    // `add r0,sp`).
    typedef struct { u32 w[8]; } Tpl8;
    u32 sp_buf[8];
    *(Tpl8 *)sp_buf = *(const Tpl8 *)(uintptr_t)0x0805F604u;
    // The ROM builds pA as base+(135<<5) (constructed 4320) and pB as
    // base+pool 0x10D8. Both offsets must survive constant-folding (else
    // the compiler pool-loads the absolute addresses), so each passes
    // through an empty asm barrier after its construction.
    // base/offA/pA are pinned to r0/r4/r5 (ROM `ldr r0,=base /
    // movs r4,#135 / lsls r4,#5 / adds r5,r0,r4`). The pins also defeat
    // constant-folding of the absolute addresses; the barrier keeps offA's
    // shift materialized instead of folded.
    register u32 base __asm__("r0") = 0x03001780u;
    register u32 offA __asm__("r4") = 135u;
    offA <<= 5;
    __asm__("" : "+r"(offA));
    register u32 *pA __asm__("r5") = (u32 *)(base + offA);   // 0x03002860
    u32 offB = 0x10D8u;
    __asm__("" : "+r"(offB));
    u32 *pB = (u32 *)(base + offB);   // 0x03002858
    u32 r6 = *pB;
    u32 idx = (r6 & 7u) << 2;
    u32 *slot = (u32 *)((u8 *)sp_buf + idx);
    u32 r4 = *pA;
    u32 t = *slot;
    r4 += t;
    u32 timer = *(volatile u32 *)0x04000100u;
    r4 += timer;
    *pA = r4;
    extern u32 sub_0802DF6C(u32, u32);
    u32 q = sub_0802DF6C(r4, 23u);
    r4 += q;
    r4 += r6;
    *pA = r4;
    // ROM tail: `adds r0, r4, #0; add sp,#32; pop {r4,r5,r6}; pop {r1}; bx r1`
    // — the accumulated counter IS returned in r0 (consumed by MenuPkt_08009B60,
    // which floors it and packs pkt+2).
    return r4;
}
u32 _08008014(void) __attribute__((alias("Course_08014")));
u32 sub_08008014(void) __attribute__((alias("Course_08014")));

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
    void *seekbase = *(void **)((u8 *)X + 4);
    void *arr = _0800748C(_08007498(seekbase, a2));
    u8 *a = (u8 *)arr;
    extern void sub_08002ED0(void *, int, int, int, int, int, int, int, int, int);
    int i = 0;
    u8 *ent;
    u32 one;
    if (i >= (int)a[7]) goto done;
    one = 1;
    ent = a + 8;
    do {
        u32 x = (u32)ent[2] + (u32)a3;
        u32 y = (u32)ent[3] + s0;
        u32 z = (u32)ent[0] + (u32)a1;
        sub_08002ED0((void *)(uintptr_t)x, (int)y, (int)z, (int)s1, (int)s2,
                     (int)ent[1], (int)one, (int)s3, (int)s4, (int)one);
        ent += 4;
        i++;
    } while (i < (int)a[7]);
done:;
}
__asm__(".align 2, 0");
extern void *_08002BFC(int a);
extern void _08002C34(int idx, void *node);
extern void sub_08002ED0(void *a, int b, int c, int d, int e,
                         int f, int g, int h, int i, int j);

// Host-only fallback for the _08002ED0 placement writer. The ROM build calls
// the closure symbol directly so the row loop retains its original call edge.
// ROM _08002ED0 consumes TEN args (r0-r3 + 6 stack words).
//   a0=r7 seed, a1=ip value (low byte), a2=r6 OR-mask, a3=dst<<12,
//   s0=2C34 idx, s1=table idx, s2=loop count, s3=+4 mask<<10,
//   s4=+0 mask<<10, s5=u16[place+12].

// _08007C68 (asm/course_records_7bfc.s:89-142) — 9 machine args (4 reg +
// The ROM reloads its final stack argument inside each row iteration; keeping
// this by-value parameter volatile preserves that repeated load.
void Course_Iter_07C68(void *X, u32 r1_add, u32 kind, u32 r3_add,
                       u32 s0_yadd, u32 s1_p3, u32 s2_s0, u32 s3,
                       volatile u32 s4) {
    register u32 dx __asm__("r7") = r3_add;
    u32 dx_copy;
    int call_z;
    void *arr = _0800748C(_08007498(*(void *volatile *)((volatile u8 *)X + 4), (int)kind));
    volatile u8 *a = (volatile u8 *)arr;
    u32 i = 0;
    if ((int)i < (int)a[7]) {
        volatile u8 *row = a + 8;
        dx_copy = dx;
        do {
        // Keep each row byte in the ROM's scratch register until its add;
        // without the barriers agbcc folds both loads into r0 and reorders
        // the coordinate instructions.
        register u32 xsrc __asm__("r1") = row[2];
        __asm__ volatile ("" : "+r" (xsrc));
        u32 x = xsrc + dx_copy;
        register u32 ysrc __asm__("r2") = row[3];
        __asm__ volatile ("" : "+r" (ysrc));
        u32 y = ysrc + s0_yadd;
        u32 z = (u32)row[0] + r1_add;
        call_z = (int)z;
        sub_08002ED0((void *)x, (int)y, call_z, (int)s1_p3, (int)s2_s0,
                     (int)row[1], 1, (int)s3, (int)s4, 0);
            row += 4;
            i++;
        } while ((int)i < (int)a[7]);
    }
}
// _08007CD0 — 124 bytes. Same row-loop family as Course_Iter_07BFC/_07DB4, but
// the per-row z seed is `(240 - (int)arr[4]) / 2` (the ROM's
// `movs r0,#240 / ldrb r1,[r6,#4] / subs / lsrs #31 / adds / asrs #1`) and the
// emit call is sub_08002ED0 with arg8 hard-wired to 0:
//   sub_08002ED0(ent[2]+h, ent[3]+a3, ent[0]+a1, a4, a5, ent[1], 1, 0, a6, 1).
// Seven machine args: a4..a6 at [sp,#56..#64].
void Course_Iter_07CD0(void *X, int a1, int a2, int a3, u32 a4, u32 a5, u32 a6) {
    void *seekbase = *(void *volatile *)((volatile u8 *)X + 4);
    void *arr = _0800748C(_08007498(seekbase, a2));
    u8 *a = (u8 *)arr;
    int h = (240 - (int)a[4]) / 2;
    int i = 0;
    u8 *ent;
    u32 one;
    if (i >= (int)a[7]) goto done;
    one = 1;
    ent = a + 8;
    do {
        u32 x = (u32)ent[2] + (u32)h;
        u32 y = (u32)ent[3] + (u32)a3;
        u32 z = (u32)ent[0] + (u32)a1;
        _08002ED0((void *)(uintptr_t)x, (int)y, (int)z, (int)a4, (int)a5,
                  (int)ent[1], (int)one, 0, (int)a6, (int)one);
        ent += 4;
        i++;
    } while (i < (int)a[7]);
done:;
}
// The 124-byte ROM span ends `47 00 00 00`; gas would fill the tail with the
// 2-byte Thumb NOP, so select the zero fill.
__asm__(".align 2, 0");
// _08007D4C — 104 bytes. Same family: 8 machine args (a0..a3 in registers,
// a4..a7 at [sp,#44..#56]). Two differences from _08007DB4: the seek kind is
// a1 (748C gets a1, not a2), and the row's z seed is precomputed BEFORE the
// loop as `(u16)arr[2] + (u16)X[0]` (the ROM's two `ldrh` at 0x1c/0x1e).
// Call shape: _08003004(ent[2]+a2, ent[3]+a3, ent[0]+seed, a6, a7, ent[1],
// a4, a5).
void Course_Iter_07D4C(void *X, int a1, int a2, int a3, u32 a4, u32 a5, u32 a6,
                       u32 a7) {
    void *seekbase = *(void *volatile *)((volatile u8 *)X + 4);
    void *arr = _0800748C(_08007498(seekbase, a1));
    u8 *a = (u8 *)arr;
    // The ROM's two `ldrh` are in the opposite order to the source statement
    // order here: agbcc schedules the plain-C loads in reverse (writing v2 then
    // x0 emits `ldrh r4,[r4,#0]` then `ldrh r0,[r6,#2]`, the reverse of the
    // ROM). Swapping the two statements -- not the sum -- reproduces the ROM's
    // `ldrh r0,[r6,#2] / ldrh r4,[r4,#0] / adds r7,r0,r4` while keeping the
    // allocation that puts the loop's top count load in r3 (inline-asm loads
    // fixed the order but moved that load to r0).
    u16 x0 = *(u16 *)((u8 *)X + 0);
    u16 v2 = *(u16 *)(a + 2);
    int seed = (int)v2 + (int)x0;
    int i = 0;
    u8 *ent;
    if (i >= (int)a[7]) goto done;
    ent = a + 8;
    do {
        u32 x = (u32)ent[2] + (u32)a2;
        u32 y = (u32)ent[3] + (u32)a3;
        u32 z = (u32)ent[0] + (u32)seed;
        _08003004((int)x, (int)y, (int)z, (int)a6, (int)a7,
                  (int)ent[1], (int)a4, (int)a5);
        ent += 4;
        i++;
    } while (i < (int)a[7]);
done:;
}
// _08007DB4 — 96 bytes. Same family as Course_Iter_07BFC above (exact there),
// but it calls _08003004 (RecordSetter_03004) instead of sub_08002ED0 and has
// NINE machine args: a0..a3 in r0-r3 and a4..a8 at [sp,#40..#56]. The old stub
// read the record pointer straight from X and passed `0` as the seek kind, and
// it dropped the loop and the 9th parameter entirely.
//
// ROM: r8 = a1 (sl), r1 = a2 (seek kind), r7 = a3; arr = 748C(7498(*(X+4),a2));
// loop rows at a+8, calling _08003004(ent[2]+a3, ent[3]+a4, ent[0]+a1,
// a7, a8, ent[1], a5, a6). The row count a[7] is re-read at the bottom of the
// loop (the ROM reloads `ldrb r0,[r6,#7]`), which the goto/do-while shape of
// Course_Iter_07BFC reproduces.
void Course_Iter_07DB4(void *X, int a1, int a2, int a3, u32 a4, u32 a5, u32 a6,
                       u32 a7, u32 a8) {
    void *seekbase = *(void *volatile *)((volatile u8 *)X + 4);
    void *arr = _0800748C(_08007498(seekbase, a2));
    u8 *a = (u8 *)arr;
    int i = 0;
    u8 *ent;
    if (i >= (int)a[7]) goto done;
    ent = a + 8;
    do {
        u32 x = (u32)ent[2] + (u32)a3;
        u32 y = (u32)ent[3] + a4;
        u32 z = (u32)ent[0] + (u32)a1;
        _08003004((int)x, (int)y, (int)z, (int)a7, (int)a8,
                  (int)ent[1], (int)a5, (int)a6);
        ent += 4;
        i++;
    } while (i < (int)a[7]);
done:;
}
// _08007E14 — 176 bytes. Same row-loop family, plus a running 8-bit
// heading/delta accumulator:
//   prev = 0; acc = 0; i = 0;
//   while (i < arr[7]) {
//       cur = ent[0];
//       acc += (cur < prev) ? cur + 256 - prev : cur - prev;   // wrap at 256
//       prev = cur;
//       if (a7) _08002F68(ent[2]+a3, ent[3]+a4, a1+acc, a5, a6, ent[1], 1);
//       else    _08002DB8(ent[2]+a3, ent[3]+a4, a1+acc, a5, a6, ent[1], 1);
//       ent += 4; i++;
//   }
// Eight machine args: a4..a7 at [sp,#48..#60]. The 8th arg is a plain `u8`,
// not `volatile`: agbcc widens the incoming stack word with `lsls/lsrs #24`
// into an SImode pseudo, which then spills to a FOUR-byte home and is
// re-loaded with `ldr` per iteration (a volatile u8 would instead force a
// byte home with strb/ldrb, and reserving the register for it pushes a1/a3
// out of sl/r9).
//
// Three shape constraints, all measured (see build/cases_e14.py):
//  * `delta` must be computed by two statements (`w = cur + 256; delta =
//    w - prev;`). A single `cur + 256 - prev` is reassociated to
//    `cur - (prev - 256)` and the 256 becomes a pool word (0xFFFFFF00) with
//    a 2-byte-longer body;
//  * the branches must be written `if (cur >= prev) delta = cur - prev; else
//    ... wrap ...` so agbcc emits `blt` into the wrap clause like the ROM
//    (the natural `cur < prev` order emits `bge`);
//  * the call arguments must be spelled inside each branch: hoisting them into
//    shared locals puts the argument setup before the `a7` test, and hoisting
//    the literal `1` costs a callee-saved register.
extern void _08002F68(int x, int id, int y, int z, int idx, int row, int count);
extern void _08002DB8(void *a, int b, int c, int d, int e, int f, int g);
void Course_Iter_07E14(void *X, int a1, int a2, int a3, u32 a4, u32 a5, u32 a6,
                       u8 a7) {
    void *seekbase = *(void *volatile *)((volatile u8 *)X + 4);
    void *arr = _0800748C(_08007498(seekbase, a2));
    u8 *a = (u8 *)arr;
    int prev = 0;
    int acc = 0;
    int i = 0;
    u8 *ent;
    if (i >= (int)a[7]) goto done;
    ent = a + 8;
    do {
        int cur = (int)ent[0];
        int delta;
        if (cur >= prev) {
            delta = cur - prev;
        } else {
            int w = cur + 256;
            delta = w - prev;
        }
        prev = cur;
        acc += delta;
        if (a7 != 0)
            _08002F68((int)ent[2] + a3, (int)ent[3] + a4, a1 + acc,
                      (int)a5, (int)a6, (int)ent[1], 1);
        else
            _08002DB8((void *)(uintptr_t)((u32)ent[2] + (u32)a3),
                      (int)ent[3] + a4, a1 + acc,
                      (int)a5, (int)a6, (int)ent[1], 1);
        ent += 4;
        i++;
    } while (i < (int)a[7]);
done:;
}

// Aliases: only for substantiated leaves + iterators + builders proven via 4B stride, u8[4] rows, 6 sp args, pools, mGBA EWRAM watchpoint, u16 masks
void _08008290(void *a) __attribute__((alias("Course_RecordInit")));
void sub_08008290(void *a) __attribute__((alias("Course_RecordInit")));
void _08008300(int a) __attribute__((alias("Course_Build32Row")));
void _080083B8(int a) __attribute__((alias("Course_Build16Row")));
void _08008480(int a,int b) __attribute__((alias("Course_BuildDispatch")));
u32 _08007EC4(void *a, int b) __attribute__((alias("Course_Math_SumU16")));
u32 sub_08007EC4(void *a, int b) __attribute__((alias("Course_Math_SumU16")));
int _08007EE0(int a,int b) __attribute__((alias("Course_Math_AbsRoundAvg")));
int _08007F08(int a,int b,int c,int d) __attribute__((alias("Course_Math_DistanceHeading")));
int _08007FC0(int a, int b, int c, void *d) __attribute__((alias("Course_Math_HeadingInterp")));
int sub_08007FC0(int a, int b, int c, void *d) __attribute__((alias("Course_Math_HeadingInterp")));
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
void _08007CD0(void *a, int b, int c, int d, u32 e, u32 f, u32 g) __attribute__((alias("Course_Iter_07CD0")));
void sub_08007CD0(void *a, int b, int c, int d, u32 e, u32 f, u32 g) __attribute__((alias("Course_Iter_07CD0")));
void _08007D4C(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h) __attribute__((alias("Course_Iter_07D4C")));
void sub_08007D4C(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h) __attribute__((alias("Course_Iter_07D4C")));
void _08007DB4(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h,
               u32 i) __attribute__((alias("Course_Iter_07DB4")));
void sub_08007DB4(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h,
                  u32 i) __attribute__((alias("Course_Iter_07DB4")));
void _08007E14(void *a, int b, int c, int d, u32 e, u32 f, u32 g,
               volatile u8 h) __attribute__((alias("Course_Iter_07E14")));
void sub_08007E14(void *a, int b, int c, int d, u32 e, u32 f, u32 g,
                  volatile u8 h) __attribute__((alias("Course_Iter_07E14")));
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

// ROM entry alias.
void Sub_08007BFC(void *X, int a1, int a2, int a3,
                       u32 s0, u32 s1, u32 s2, u32 s3, u32 s4) __attribute__((alias("Course_Iter_07BFC")));
void sub_08007BFC(void *X, int a1, int a2, int a3,
                       u32 s0, u32 s1, u32 s2, u32 s3, u32 s4) __attribute__((alias("Course_Iter_07BFC")));
