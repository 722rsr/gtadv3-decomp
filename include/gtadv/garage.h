#ifndef GTADV_GARAGE_H
#define GTADV_GARAGE_H

#include "gba/types.h"

// Garage / course-record placement cluster — asm/garage_26f50.s
// VMA 0x08026F50–0x0802B04C, 97 funcs, pure Thumb ARMCC.
// Wraps sub_08007538 / 75E8 / 02ED0-family helpers over tables 0x080CDB78 / 0x083A4374.
// Related implementations: asm/carphys_tick.s, asm/saveblock.s, course pipeline docs.

// Work-area / garage base anchors
#define GARAGE_WA_BASE   0x03001780u
#define GARAGE_PTR_SLOT  0x03001678u
#define GARAGE_EXTRA_SLOT 0x0300167Cu
#define GARAGE_CURSOR_OFF 0x574u  // s16 idx at 0x03001780+0x574

// C API — aliases preserve original BL targets. Leaves first, then dispatchers.

// Primary placement veneer (old-style alias preserved)
void Garage_PlaceRecord(u32 a, u32 b, u32 c, void *dst); // sub_08026F50 / _080026F50

// Internal helpers (numeric BL targets in other menu/course files)
void Garage_Helper_26F80(void *a, u32 b, void *c, u32 d, u32 e, bool flag);
void Garage_Helper_26FC4(void *a, u32 b, u32 c, void *d, u32 e, u32 f);
void Garage_InitRecord(void *rec, u32 a1, u32 count, u32 flagByte); // sub_080026FF4 (4-arg, flag byte via r3 MSB, high-reg r8 spill)
void Garage_RangeHelper(u32 a, u32 b, u32 c);
void Garage_MatrixHelper(void *base, u32 row);

// Course-placement leaves (heavy math, tables 0x0805CAF0 etc.)
void Garage_Leaf_27230(void *a, u32 b, u32 c, u32 d, u32 e);
void Garage_Leaf_27234(void *a, u32 b, u32 c);
void Garage_Leaf_275B8(u32 a, u32 b, void *c, u32 d, u32 e);
void Garage_Leaf_279D8(void *a, u32 b, void *c, u32 d, u32 e);
void Garage_Leaf_27B94(void *a);
void Garage_Leaf_27D30(void *a);
void Garage_Leaf_27D84(u32 mode);

// Additional garage leaves covering 0x027000–0x02B04C span — 0x26FF4 family
void Garage_ByteStore_27104(void *idx, uint32_t val); // _080027104 (u8 at [*(0x03001678)+idx*8+24])
void Garage_ByteStore_27110(void *base, uint32_t val); // _080027110 (u8 at [*(0x03001678)+17])

void Garage_Leaf_27000(void);
void Garage_Leaf_27460(void);
void Garage_Leaf_276F0(void);
void Garage_Leaf_27CE0(void);
void Garage_Leaf_27D08(void);
void Garage_Leaf_27F60(void);
void Garage_28734(void); // sub_080028734 — 8B push {lr} bl _08001818 pop bx (direct wrapper, no pools)
void Garage_28744(void); // sub_080028744 — 52B push {r4,lr} ldr 02030000 bl 07978/2D984/07924 etc, pools 02030000/06004000 (bounded, no car-record stride)
void Garage_28828(void); // sub_080028828 — 8B push {lr} bl 028784 pop bx (wrapper over dispatcher, no pools)
void Garage_289C0(void *rec); // sub_0800289C0 — 16B push {lr} ldrh [r0,#16] movs r0,#0 bl 01E48 pop bx (field +16 u16 via ldrh)
void Garage_28B40(void); // sub_080028B40 — 20B push {lr} ldr 138B bl 016D0 bl 01818 pop bx (pools 0000138B, no car-record stride)
void Garage_29218(void); // sub_080029218 — 12B push {lr} bl 04BD8 pop bx (direct wrapper, no pools, no car-record stride)
void Garage_29224(void); // sub_080029224 — 12B push {lr} movs #1 bl 02060 pop bx (direct wrapper, r0=1, no pools)
void Garage_29208(void *rec); // sub_080029208 — 12B push {lr} movs #213 lsls #5 adds r0 pop bx (r0+0x1AA0 bl 04EF0, no pools, no car-record stride)
void Garage_28B74(void); // sub_080028B74 — 20B push {lr} ldr 14D movs #0 bl 01E48 pop bx (pool 0000014D, r0=0 r1=14D)
void Garage_28888(int a, int b, int flag); // sub_080028888 — 24B push {lr} lsls r2 #16 lsrs #16 movs #1 ands cmp beq bl 04EC0 pop bx (r2 u16 &1, no pools)
void Garage_28B54(void *rec); // sub_080028B54 — 32B push {r4,r5,lr} movs r5 #0 loop 4× ldrh movs r0=r5 bl 01E5C strh [r4,#0] (r4+2, r5+1, cmp #3 ble, no pools)

#endif // GTADV_GARAGE_H
