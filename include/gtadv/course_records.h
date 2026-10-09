#ifndef GTADV_COURSE_RECORDS_H
#define GTADV_COURSE_RECORDS_H
#include "gba/types.h"

// Records family 0x08007BFC-0x08009B60 — 6 iterators now proven via instruction widths, 4B stride, 6 sp args, table bases
// Iterators share: push high-reg, sub sp #24/#16, ldr [r0+4]→07498→0748C, ldrb cnt [+7] u8, loop 4B rows [+8] via ldrb u8[4], bl 02ED0 with 6 sp args (u8 via ldrb at +0..3, s8 via ldrsb at +6, s32 via lsls #10, u32 via ldr)
void Course_Iter_07BFC(void *X, int a1, int a2, int a3, u32 s0, u32 s1, u32 s2, u32 s3, u32 s4); // _08007BFC: 9-arg (r0-r3 + sp+52/56/60/64/68); r1->r9, r2->seek idx, r3->r8
void Course_Iter_07C68(void *X, u32 r1_add, u32 kind, u32 r3_add, u32 s0_yadd, u32 s1_p3, u32 s2_s0, u32 s3, u32 s4); // _08007C68: 9 args (r1=z_add, r2=seek kind, r3=x_add, s0=y_add, s1=02ED0 r3, s2=02ED0 s0, s3=02ED0 s3, s4=02ED0 s4)
void Course_Iter_07CD0(void *X, u8 sl, u8 r9, u32 s56, u32 s60, u32 s64); // _08007CD0: sp+56 u32, sl via r1, r9 via r3, 240-ldrb+asrs s32
void Course_Iter_07D4C(void *X, u8 sl, u8 r9, u32 s44, u32 s48, u32 s52, u32 s56); // _08007D4C: ldrh +2 u16, r8 via r9, 4B rows, bl 03004
void Course_Iter_07DB4(void *X, u8 sl, u8 r7, u32 s40, u32 s44, u32 s48, u32 s52, u32 s56); // _08007DB4: s 0..cnt, ldrb +2 u8 +r7
void Course_Iter_07E14(void *X, u8 sl, u8 r9, u32 s48, u32 s52, u32 s56, u32 s60); // _08007E14: ldrb +7 u8 cnt, loop +4 stride, bl 02F68/02DB8 via r12
void Course_RecordsIterA(void *X); // legacy stub kept for hosts
int  Course_DistanceSqrtUdiv(int a,int b); // legacy stub for _08007F08 family

// Math leaves 0x07EC4-0x07FC0 — table shapes 0x080C9064/0x080CA064/0x080CB064 proven via ROM dump
u32  Course_Math_SumU16(void *rec, int i); // _08007EC4: (rec,i) via _08007498/_0800748C, ldrh pair, wide adds (caller strh)
int  Course_Math_AbsRoundAvg(int a, int b); // _08007EE0: abs(a)+abs(b) - ((5*min)>>3), s32
int  Course_Math_DistanceHeading(int x0,int y0,int x1,int y1); // _08007F08: s32 diff, muls, sqrt swi 8, udiv swi 6, table 0x080C9064/0x080CA064 s16
int Course_Math_HeadingInterp(int p0, int p1, int p2, void *out); // _08007FC0: s16 clamp, table 0x080CB064 s16, muls s32>>12, strh, returns clamp flag

// State leaves 0x08014-0x08284 — block behind 0x030003E0, widths proven via strh/strb/lsls
u32 Course_08014(void); // _08008014: ldmia 32 B template 0x0805F604, Vu32 at 0x03002858/60 + IO 0x04000100, divide 23; returns the accumulated counter (r0)
void Course_StateTick(void *blk); // legacy stub
void Course_State_Store0(u16 v); // _08008074: strh [r1+0] u16
void Course_State_Store14(u16 v); // _08008080: strh [r1+14] u16
void Course_State_Store6(u16 v); // _0800808C: strh [r1+6] u16
void Course_State_Store10(u16 v); // _08008098: strh [r1+10] u16
void Course_State_ClampStore2(int v); // _080080A4: s16 clamp 0..0x3E7 then strh [+2] u16, void leaf, lsls/lsrs/asrs
void Course_State_Store64(u32 v); // _080080CC: str [r1+64] u32
void Course_State_Store68(u32 v); // _080080D8: str [r1+68] u32
void Course_State_Store72_104(u32 v); // _080080E4: str [r1+72] u32, strh 104 at [+4] u16
void Course_State_Store72_150(u32 v); // _080080F4: str [r1+72] u32, strh 150 at [+34] u16
void Course_State_Flag78(void); // _08008104: adds #78, strb 1 at [+78] u8
void *Course_State_Store12Flag77(int v); // _08008114: u16[rec+12] cmp, strb 1 at [+77] u8 if !=, strh [+12] u16, returns rec
void Course_State_Store8Flag76(u16 v); // _08008134: ldrh [+10] s16 clamp, strb at [+76] u8, strh [+8] u16
int  Course_State_CourseId(int id); // _08008164: strh [+16] s16, div-by-10 tables 0x080CB17C/0x080CB074 via swi 6, call 07538
void Course_State_Store18_48(u32 v); // _080081F8: strh [+18] u16, strh 48 at [+26] u16
void Course_State_Store22_24(void); // _08008208: strh 16 at [+22] u16, strh 1 at [+24] u16
int  Course_State_ResetTimed(void); // _0800821C: sdiv 5, strh [+30] s16, ldrsh table 0x080CB074, bl 07ABC gate
void Course_State_Store32(u32 v); // _08008284: strh [+32] u16

// Builders 0x08290-0x08AAC
void Course_RecordInit(void *rec); // _08008290 filler
void Course_Build32Row(int sel); // _08008300 32-row
void Course_Build16Row(int sel); // _080083B8 16-row
void Course_BuildDispatch(int mode,int sel); // _08008480 dispatch via 0x03001780+0x10FC
void Course_Emit8A(void *out,int w); // _0800861C
void Course_Emit8B(void *out,int w); // _08008768
void Course_Emit8C(void *out,int w); // _080088B0

// Dispatch 0x8AAC-0x95F0
void Course_Dispatch2Rec(void *rec); // _08008AAC 2/3 rec emit
void Course_DispatchB(void *rec); // _08008BB8
void Course_PhaseDispatcher(void *ctx); // _08008CA0 21-entry table

// Leaves 0x95F0-0x9B60 — 095F0/0961C flag-gated writers proven via ldrh/ldrsh, 09674/09680 u32, 0968C s16
void Course_LeavesWrite(void *rec); // legacy stub for 0x095F0 cluster
void Course_Leaves_095F0(void); // _080095F0: ldrh [r1+50] u16, ldrsh [r1+12] s16, lsls #2, bl 07570
void Course_Leaves_0961C(void); // _0800961C: ldrh [r1+48] u16, ldrsh [r1+8] s16
u32 Course_GetU32_030003E4_0(void); // _08009674: ldr [0x030003E4] u32
void Course_SetU32_030003E4_0(u32 v); // _08009680: str [0x030003E4] u32
s16 Course_Fetch_S16_20Shift(void *a); // _0800968C: lsls #16, asrs #15, adds #20, ldrsh s16
void Course_HudTemplate(void *rec); // _08009900 CpuSet via _08002E0A4

#endif
