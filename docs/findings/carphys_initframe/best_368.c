#include "gtadv/car_physics_lane.h"
#include "gba/types.h"
#include "gba/bios.h"

// Lane core: carphys_racer, go_start, grant_delay
// Sources: asm/carphys_racer.s, asm/go_start.s, asm/grant_delay.s, asm/carphys_racer.s

// Forward decls of external helpers (provided by other lanes or ROM)
extern void *sub_08004B68(void);
extern void *_08004B68(void);
extern void sub_0802B1E4(int);
extern void sub_0800DAB8(void *p);
extern void sub_080075E8(void *base,int idx,int n);
extern void sub_0800798C(void *base, void *dst);
extern void sub_08007A58(void *p);
extern void *sub_0800572C(u32 s);  // save.c:813 alias of _0800572C: `void *(u32)`, not void/int
extern int  _0800572C(int);
extern int  _08005758(int);
extern void sub_08007ABC(u32 a, u32 b, u32 c);
extern void sub_0800279C(int);
extern void sub_0800D77C(void *a,int b,int c);
extern int  sub_08007570(void *base,int v,int n,int m,int k); // 5-arg ROM ABI; decl-only here
extern int  sub_08025CF4(int,int,int);
extern int  _080025CF4(int,int,int);    // _08007770 true arity 6 (2 stack words); the two car-id sites pass {4, r9=1}.
    extern void sub_08007770(int,void*,int,int,int,int);
extern int  sub_08024C3C(int);
extern int  sub_08024D5C(int);
extern int  sub_08024C58(int);
extern int  sub_08025F78(int);
extern int  sub_08025FAC(int);
extern int  sub_08024C90(int);
extern void _08023FF8(int,int);
extern int  sub_08026004(int);
extern int  sub_08025FF0(int);
extern int  sub_080022E4(int);
extern void sub_0800D9A4(void *obj, int idx, int arg);
extern int  sub_080258A8(int idx);
extern int  sub_08026F30(int idx, int flags);
extern void sub_08026F50(int a, int b, int c);
extern void _0802B368(u16 v);   // closure spelling; was sub_08002B368, a
                               // 9-digit form the screen cannot match

// IWRAM base
#define WA 0x03001780u

// ROM data addresses used by CarPhysRacer_InitFrame (pools at 0x080215BC-0x08021824).
// ROM_ASSET_MAIN is a descriptor {u32 pad[3]; s32 offs[]}: entry i lives at base+12+i*4
// and is an offset relative to the base (see sub_08007498/sub_08007770).
#define ROM_PALETTE_A   0x082A798Cu
#define ROM_PALETTE_B   0x082C4228u
#define ROM_RES_24      0x082C4458u
#define ROM_RES_32      0x082C5040u
#define ROM_RES_40      0x082D0DC0u
#define ROM_RES_8       0x0836EF7Cu
#define ROM_RES_16      0x08342B60u
#define ROM_ASSET_MAIN  0x08368B0Cu
#define ROM_EC_IDX      0x080CBFACu  // s16 table: tbl[s16[pBE]*2 + s16[pBC]*8]
#define ROM_ASSET_F0    0x08349754u  // base for sub_08007570/(id0) sub_080075E8
#define ROM_IDX_LOW     0x080CBFECu  // s16 index table (id0 path)
#define ROM_IDX_HIGH    0x080CC02Cu  // s16 index table (id0 path)
#define ROM_RES_FBDC    0x082DFBDCu
#define EWRAM_STATE     0x0203F990u  // EWRAM car-id0 state {+0: u16, +2: u16}

// ---- grant_delay : _0800B82C ----
void GrantDelay_Tick(int carId, int delay)
{
    extern u8 GrantDelayWa[];
    register int id __asm__("r5") = carId;
    register volatile u8 *wa __asm__("r6") = (volatile u8 *)GrantDelayWa;
    register u32 offset __asm__("r2") = 0x10F8u;
    register uintptr_t slot __asm__("r4");
    register u32 index __asm__("r0");
    register volatile u8 *store_addr __asm__("r1");

    __asm__(".globl GrantDelayWa\nGrantDelayWa = 0x03001780\n");
    if (*(volatile u32 *)(wa + offset) < (u32)delay)
        return;
    if (((u32)sub_08025FAC(id) << 16) != 0)
        return;

    offset = 0x1058u;
    {
        volatile u16 *flag = (volatile u16 *)(wa + offset);
        __asm__ volatile ("" : : : "memory");
        slot = 1;
        *flag = (u16)slot;
    }
    index = (u32)sub_08024C90(id);
    index <<= 1;
    offset = 131;
    offset <<= 5;
    store_addr = wa + offset;
    index += (u32)(uintptr_t)store_addr;
    *(volatile u16 *)(uintptr_t)index = (u16)slot;

    slot = (uintptr_t)(wa + 0x103Cu);
    {
        register u32 work __asm__("r1") = 0;
        register s32 count __asm__("r0");
        // Keep this non-volatile signed-halfword read into an int accumulator:
        // agbcc then emits the ROM's `movs r1,#0; ldrsh r0,[r4,r1]` directly.
        count = *(s16 *)(uintptr_t)(slot + work);
        offset -= 34;
        work = (u32)(wa + offset);
        count += work;
        *(volatile u8 *)(uintptr_t)count = (u8)id;
    }

    sub_08025F78(id);
    _08023FF8(28, 0);
    *(volatile u16 *)slot = *(volatile u16 *)slot + 1;
}

void _0800B82C(int a0,int a1) __attribute__((alias("GrantDelay_Tick")));
void sub_0800B82C(int a0,int a1) __attribute__((alias("GrantDelay_Tick")));

// ---- go_start : _0800B89C ----
void GoStart_Tick(void) {
    register volatile u8 *wa __asm__("r5") = (volatile u8 *)(uintptr_t)WA;
    volatile u16 *stage = (volatile u16 *)(wa + 0x576);

    {
        register u16 stageWord __asm__("r1");
        stageWord = *stage;
        if (stageWord == 1) {
            sub_0800B82C(86, 10);
            sub_0800B82C(56, 30);
        }
    }
    {
        register u16 stageWord __asm__("r2");
        stageWord = *stage;
        if (stageWord == 2) {
            sub_0800B82C(55, 10);
            sub_0800B82C(31, 30);
        }
    }
    {
        register u16 stageWord __asm__("r0");
        stageWord = *stage;
        if (stageWord == 3) {
            sub_0800B82C(14, 10);
            sub_0800B82C(98, 30);
        }
    }

    {
        register u16 stageWord __asm__("r4");
        stageWord = *stage;
        if (stageWord == 1 && *(volatile u32 *)(wa + 0x10F8) > 19u &&
            (u16)sub_08026004(1) == 0) {
            sub_08025FF0(1);
            {
                register u32 selOff __asm__("r2") = 0x104Au;
                volatile s16 *selection = (volatile s16 *)(wa + selOff);
                s16 index = *(s16 *)(uintptr_t)selection;
                s32 scaledIndex = (s32)index << 1;
                __asm__ volatile ("" : "+r"(scaledIndex));
                selOff += 2u;
                __asm__ volatile ("" : "+r"(selOff));
                {
                    register volatile u8 *scriptBase __asm__("r1");
                    scriptBase = wa + selOff;
                    *(volatile u16 *)(uintptr_t)(scaledIndex + (s32)(uintptr_t)scriptBase) = 2;
                    __asm__ volatile ("" : "+r"(wa));
                    _08023FF8(30, 0);
                    *selection = (u16)(*selection + 1);
                }
            }
        }
    }

    wa = (volatile u8 *)(uintptr_t)WA;
    {
        register u32 stageOffset __asm__("r1") = 0x576u;
        if (*(volatile u16 *)(wa + stageOffset) == 2) {
            __asm__ volatile ("" : : : "r2");
            {
                register u32 timerOffset __asm__("r2") = 0x10F8u;
                u32 timerValue;
                __asm__ volatile ("" : "+r"(timerOffset));
                timerValue = *(volatile u32 *)(wa + timerOffset);
                __asm__ volatile ("" : "+r"(timerOffset));
                if (timerValue > 19u && (u16)sub_08026004(2) == 0) {
                    sub_08025FF0(2);
                    {
                        volatile s16 *selection = (volatile s16 *)(wa + 0x104A);
                        s16 index = *(s16 *)(uintptr_t)selection;
                        s32 scaledIndex = (s32)index << 1;
                        __asm__ volatile ("" : "+r"(scaledIndex));
                        {
                            register u32 scriptOffset __asm__("r2") = 0x104Cu;
                            register volatile u8 *targetBase __asm__("r1");
                            __asm__ volatile ("" : "+r"(scriptOffset));
                            targetBase = wa + scriptOffset;
                            *(volatile u16 *)(uintptr_t)(scaledIndex + (s32)(uintptr_t)targetBase) = 3;
                        }
                        __asm__ volatile ("" : "+r"(wa));
                        _08023FF8(30, 0);
                        *selection = (u16)(*selection + 1);
                    }
                }
            }
        }
    }
}
void _0800B89C(void) __attribute__((alias("GoStart_Tick")));
void sub_0800B89C(void) __attribute__((alias("GoStart_Tick")));

void CarPhysRacer_MarkScene(void *ctx, void *rec){
    (void)ctx;

    (void)sub_08004B68();
    *(volatile u16 *)((uintptr_t)rec + 0x54) = 1;
}
void _0802135C(void *a, void *b) __attribute__((alias("CarPhysRacer_MarkScene")));

void CarPhysRacer_Nop(void){}
__asm__(".align 2, 0");
void _08021370(void) __attribute__((alias("CarPhysRacer_Nop")));

void CarPhysRacer_RaceStartLatch(void *ctx,int b,int c){
    // _08021828: r2=c, t = (u16)c -1; if <=1 then latch.
    // The 16-bit truncation is written in the ROM's own shape, not as an
    // arithmetic add of 0xFFFF: `((c<<16) + 0xFFFF0000) >> 16`. Written as
    // `(u16)((u16)c + 0xFFFF)` agbcc instead reassociates to
    // `ldr r0,=0xFFFF0000 / adds r2,r2,r0 / lsls r2,#16 / lsrs r2,#16` -- the
    // same four operations with the constant load hoisted ABOVE the shift
    // (47/56, first diff +0x04). Spelling the shift first reproduces the ROM's
    // `lsls r2,#16 / ldr r0,=0xFFFF0000 / adds r2,r2,r0 / lsrs r2,#16` exactly,
    // and still takes the unsigned `cmp r2,#1; bhi`, which only an unsigned
    // `t` produces. 56/56.
    u16 t = (u16)(((((u32)c) << 16) + 0xFFFF0000u) >> 16);
    if (t > 1) return;
    _0802B368(1);
    *(volatile u32 *)((uintptr_t)ctx + 0xC8) = 1;
    *(volatile u16 *)((uintptr_t)ctx + 0x3C) = 0;
    *(volatile u16 *)((uintptr_t)ctx + 0xA4) = 1;
    *(volatile u16 *)((uintptr_t)ctx + 0x40) = 1;
    (void)b;
}
void _08021828(void *a,int b,int c) __attribute__((alias("CarPhysRacer_RaceStartLatch")));

void CarPhysRacer_InitFrame(void *ctx) {
    extern u8 GrantDelayWa[];            // defined at 0x03001780 (see GrantDelay_Tick)
    u8 *c = (u8 *)ctx;
    register s16 *cell __asm__("sl");    // ctx+0xC0: registry record id cell
    s16 *p;                              // scratch pointer reused by every id arm
    s16 *pBC;                            // ctx+0xBC (WA mirror)
    s16 *pBE;                            // ctx+0xBE (WA mirror)
    u32 *pDC;                            // ctx+0xDC (slot ptr for sub_08007ABC)
    u32 *pE0;                            // ctx+0xE0
    u32 *pF4;                            // ctx+0xF4: bound-object handle slot
    u32 *pF8;                            // ctx+0xF8
    u32 *p100;                           // ctx+0x100
    u32 *p104;                           // ctx+0x104
    u8 *sub38;                           // ctx+0x38: sub-object address
    u8 *p16;                             // ctx+0x10
    u32 h;
    s16 v;
    u8 *asset;                           // 0x08368B0C asset descriptor

    v = *(s16 *)sub_08004B68();
    if (v == 28)
        goto arm28;
    if (v < 29) {
        if (v == 27)
            goto arm27;
        p = (s16 *)(c + 0xC0);
        cell = p;
        goto have_cell;
    }
    if (v == 29)
        goto arm29;
    if (v == 30)
        goto arm30;
    cell = (s16 *)(c + 0xC0);
    goto have_cell;

arm27:
    sub_0802B1E4(66);
    p = (s16 *)(c + 0xC0);
    *p = 0;
    cell = p;
    goto have_cell;
arm28:
    sub_0802B1E4(66);
    p = (s16 *)(c + 0xC0);
    *p = 1;
    cell = p;
    goto have_cell;
arm29:
    sub_0802B1E4(66);
    p = (s16 *)(c + 0xC0);
    *p = 2;
    cell = p;
    goto have_cell;
arm30:
    sub_0802B1E4(66);
    p = (s16 *)(c + 0xC0);
    *p = 3;
    cell = p;
have_cell:

    {
        u8 *wa = GrantDelayWa;
        pBC = (s16 *)(c + 0xBC);
        *pBC = *(u16 *)(wa + 0x1046);
        pBE = (s16 *)(c + 0xBE);
        *pBE = *(u16 *)(wa + 0x1048);
    }

    {
        u32 *p = (u32 *)(c + 0x80);
        *p = 6;
        p += 3;
        *p = 16;
        p += 3;
        *p = 0;
    }

    sub38 = c + 0x38;
    sub_0800DAB8(sub38);

    sub_080075E8((void *)ROM_PALETTE_A, 2, 5);
    sub_080075E8((void *)ROM_PALETTE_B, 0, 6);
    sub_0800798C((void *)ROM_RES_24, c + 24);

    h = (u32)(uintptr_t)sub_0800572C(7);
    pDC = (u32 *)(c + 0xDC);
    *pDC = h;
    pE0 = (u32 *)(c + 0xE0);
    *pE0 = 2;
    sub_08007ABC(*(u32 *)(c + 28), 2, *pDC);

    sub_0800798C((void *)ROM_RES_32, c + 32);

    h = (u32)(uintptr_t)sub_0800572C(14);
    pF4 = (u32 *)(c + 0xF4);
    *pF4 = h;
    pF8 = (u32 *)(c + 0xF8);
    *pF8 = 5;
    sub_08007ABC(*(u32 *)(c + 36), 5, *pF4);

    sub_0800798C((void *)ROM_RES_40, c + 40);

    h = (u32)(uintptr_t)sub_0800572C(7);
    p100 = (u32 *)(c + 0x100);
    *p100 = h;
    p104 = (u32 *)(c + 0x104);
    *p104 = 50;
    sub_08007ABC(*(u32 *)(c + 44), 50, *p100);

    asset = (u8 *)ROM_ASSET_MAIN;
    sub_0800798C(asset, c);
    sub_08007A58(c);
    sub_080075E8(asset, 0, 3);
    sub_080075E8(asset, 1, 4);
    sub_080075E8(asset, 2, 7);
    sub_080075E8(asset, 3, 10);
    sub_0800798C((void *)ROM_RES_8, c + 8);

    h = (u32)(uintptr_t)sub_0800572C(65);
    *(u32 *)(c + 0xE8) = h;
    {
        s32 idx = (s32)*pBE * 2 + (s32)*pBC * 8;
        v = *(s16 *)(ROM_EC_IDX + idx);
        *(s32 *)(c + 0xEC) = v;
        sub_08007ABC(*(u32 *)(c + 12), (u32)(s32)v, *(u32 *)(c + 0xE8));
    }

    p16 = c + 16;
    sub_0800798C((void *)ROM_RES_16, p16);
    sub_08007A58(p16);
    sub_080075E8((void *)ROM_RES_16, 0, 9);
    *(u32 *)(c + 0xA8) = (u32)(uintptr_t)(p16 + 184);
    *(u32 *)(c + 0xAC) = (u32)(uintptr_t)sub38;
    *(u32 *)(c + 0x38) = 0;
    *(u16 *)(c + 0x3C) = 1;

    sub_0800D77C(c + 76, 0, -32);
    sub_0800D77C(c + 68, 0, 160);
    {
        u16 *q = (u16 *)(c + 64);
        q[0] = 6;
        q[1] = 5;
    }
    *(u32 *)(p16 + 184) = 2;
    sub_0800279C(1);

    switch (*cell) {
    case 0:
        sub_08007770(0, asset, 5, 0, 4, sub38);
        *(u16 *)(EWRAM_STATE + 2) = (u16)sub_08005758(16);
        *(u16 *)EWRAM_STATE = 0;
        v = *pBE;
        if (v == 0)
            return;
        if (v < 0)
            return;
        if (v > 3)
            return;
        sub_08007570((void *)ROM_ASSET_F0,
                     *(s16 *)(ROM_IDX_LOW + (s32)v * 2 + (s32)*pBC * 8),
                     *(s16 *)(EWRAM_STATE + 2), 16, 16);
        sub_080075E8((void *)ROM_ASSET_F0,
                     *(s16 *)(ROM_IDX_HIGH + (s32)*pBE * 2 + (s32)*pBC * 8), 8);
        break;
    case 1:
        sub_08007770(0, asset, 4, 0, 4, sub38);
        {
            u8 *wa = GrantDelayWa;
            u16 *cnt = (u16 *)(wa + 0x103C);
            *cnt = (u16)(*cnt - 1);
            if ((s16)*cnt <= 0)
                *cnt = 0;
            v = ((s8 *)wa)[0x103E + *(s16 *)(wa + 0x103C)];
        }
        *(u16 *)(c + 0xB6) = (u16)v;
        *(u16 *)(c + 0xB8) = (u16)sub_080022E4(*(s16 *)(c + 0xB6));
        *(u16 *)(c + 0xB4) = 0;
        sub_0800D9A4(c + 0xCC, *(s16 *)(c + 0xB6), 0);
        h = (u32)sub_08024C3C(*(s16 *)(c + 0xB6));
        *pF8 = h;
        sub_08007ABC(*(u32 *)(c + 36), h, *pF4);
        h = (u32)sub_08024D5C(*(s16 *)(c + 0xB6));
        *pE0 = h;
        sub_08007ABC(*(u32 *)(c + 28), h, *pDC);
        h = (u32)sub_08024C58(*(s16 *)(c + 0xB6));
        *p104 = h;
        sub_08007ABC(*(u32 *)(c + 44), h, *p100);
        break;
    case 2:
        sub_08007770(0, asset, 7, 0, 4, sub38);
        break;
    case 3:
        {
            u8 *wa = GrantDelayWa;
            u16 *cnt = (u16 *)(wa + 0x104A);
            *cnt = (u16)(*cnt - 1);
            if ((s16)*cnt <= 0)
                *cnt = 0;
            *(u16 *)(c + 0xC2) = *(u16 *)(wa + 0x104C + *(s16 *)(wa + 0x104A) * 2);
        }
        sub_08007770(0, asset, 6, 0, 4, sub38);
        sub_0800798C((void *)ROM_RES_FBDC, c + 48);
        sub_080075E8((void *)ROM_RES_FBDC, 0, 3);
        h = (u32)(uintptr_t)sub_0800572C(12);
        *(u32 *)(c + 0x10C) = h;
        v = (s16)sub_080258A8(*(s16 *)(c + 0xC2));
        *(s32 *)(c + 0x110) = (s32)v;
        sub_08007ABC(*(u32 *)(c + 52), (u32)(s32)v, h);
        *(s32 *)(c + 0x11C) = (s32)*(s16 *)(c + 0xC2);
        {
            int r = sub_08026F30(*(s32 *)(c + 0x11C), 4);
            *(u32 *)(c + 0x118) = (u32)r;
            sub_08026F50(r, *(u32 *)(c + 0x11C), 4);
        }
        break;
    default:
        break;
    }
}

void _08021374(void *ctx) __attribute__((alias("CarPhysRacer_InitFrame")));


// Substantiated gap leaf sub_080021C94: ctx = r1, slot = _08004B68->+2
// if *(s16*)(slot+2)==15||20 → *(u16*)(ctx+84)=5 else 6 (asm/carphys_racer_tail.s:7-21)
// The ROM epilogue is `pop {r4}; pop {r0}; bx r0` (B70 at 0x08021CB8), which
// DESTROYS r0 — so the routine is void in the ROM and no caller can observe the
// 5/6 it just stored. Returning int makes agbcc pick a different pop scratch
// (`pop {r1}; bx r1`), which is the only remaining byte difference.
void Gap_021C94_Select(void *a, void *b){
    volatile u16 *dst;
    int out;
    s16 v;
    (void)a;
    v = *(const s16 *)((uintptr_t)_08004B68() + 2);
    if (v == 15 || v == 20) {
        dst = (volatile u16 *)((uintptr_t)b + 84);
        out = 5;
    } else {
        dst = (volatile u16 *)((uintptr_t)b + 84);
        out = 6;
    }
    *dst = (u16)out;
}
__asm__(".align 2, 0");
void _080021C94(void *a, void *b) __attribute__((alias("Gap_021C94_Select")));
void sub_080021C94(void *a, void *b) __attribute__((alias("Gap_021C94_Select")));

// Dispatcher _08021BF8 : 12-entry jump table (ev-1)
void CarPhysRacer_Dispatcher(int ev,int b,int c,void *ctx){
    // Mirrors asm table at 0x08021C14; slot = ev-1
    extern void _08021374(void *x);
    extern void NopCtx21370(void *x) __asm__("_08021370");
    unsigned slot = (unsigned)(ev - 1);
    if (slot > 11)
        return;
    switch(slot){
        // ROM block order is 1,4,6,5,0,11 (table at 0x08021C14); source order
        // sets layout order, so list them in ROM order.
        // ROM 0x08021C44: `adds r0,r4; adds r1,r5; bl 0x0802135C` -- the
        // second argument (r1/b) is the record pointer MarkScene stores into.
        case 1: CarPhysRacer_MarkScene(ctx, (void *)(uintptr_t)b); break;
        case 4: { extern void sub_0800D854(void*); extern void sub_0800D8E4(void*); sub_0800D854((u8*)ctx+0x38); sub_0800D8E4((u8*)ctx+0xA0); break; }
        case 6: { extern void _08021BA0(void*); _08021BA0(ctx); break; }
        case 5: { if (*(volatile u16 *)((uintptr_t)ctx+0x3C)!=0) CarPhysRacer_RaceStartLatch(ctx,(int)(u16)b,(int)(u16)c); break; }
        case 0: _08021374(ctx); break;
        case 11: NopCtx21370(ctx); break;
        default: break;
    }
}
__asm__(".align 2, 0");
void _08021BF8(int ev,int b,int c,void *ctx) __attribute__((alias("CarPhysRacer_Dispatcher")));

// Remaining gap helpers (0x21CC4-0x22CB4, ~280 labels) — TODO pending BL target audit.
// Only sub_080021C94 above is substantiated; rest stay as explicit TODO without alias.
