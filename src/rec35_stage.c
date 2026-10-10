#include "gtadv/rec35.h"
#include "gtadv/memory.h"
#include "gba/types.h"

// Reference: asm/rec35_stage.s 0x08015ED0-0x08016002 + asm/rec35_helper.s

extern int Sound_Cmd3(void); // 0x0802B3B8 returns 0 when complete (low byte 0)
extern void GuardedSaveOnly(void); // 0x08024BC0
extern void GuardedFullSave(void); // 0x08024B70
extern void ScenePost(int a,int b); // 0x08002618
extern void EventPost(void *list, int id); // 0x08025BF0 (ROM shape (P,S): [list+4] = s16[0x080CD830 + id*20])
extern void EventBind(void *a, int b, int c); // 0x08007ABC (ROM shape: 3 reg args (void*,int,int),)
#ifndef __APPLE__
extern void sub_08025BF0(void *list, int id);
extern void sub_08007ABC(void *a, int b, int c);
#define REC35_POST sub_08025BF0
#define REC35_BIND sub_08007ABC
#else
#define REC35_POST EventPost
#define REC35_BIND EventBind
#endif

// The phase is an ordinary IWRAM halfword (ldrsh). EventBind receives the
// posted node stored at ctx+0xA0, not the address of that pointer cell.
// A switch preserves the case-1 pivot followed by signed bounds checks;
// an if/range expression instead introduces an unsigned subtract-and-compare.
// The section alignment restores the ROM's trailing zero halfword.
void Rec35_HelperA(void *ctx){
    s16 phase = *(s16 *)((u8 *)ctx + 0x94);
    switch (phase) {
    case 1:
        REC35_POST((u8 *)ctx + 0xA0, 9);
        REC35_BIND(*(void **)((u8 *)ctx + 12),
                   (int)(uintptr_t)*(void **)((u8 *)ctx + 0xA4),
                   (int)*(u32 *)((u8 *)ctx + 0xA0));
        break;
    case 2:
    case 3:
        REC35_POST((u8 *)ctx + 0xA0, 10);
        REC35_BIND(*(void **)((u8 *)ctx + 12),
                   (int)(uintptr_t)*(void **)((u8 *)ctx + 0xA4),
                   (int)*(u32 *)((u8 *)ctx + 0xA0));
        break;
    }
}
__asm__(".align 2, 0");
void Rec35_HelperB(void *ctx){
    s16 phase = *(s16 *)((u8 *)ctx + 0x96);
    switch (phase) {
    case 1:
        REC35_POST((u8 *)ctx + 0xA0, 17);
        REC35_BIND(*(void **)((u8 *)ctx + 12),
                   (int)(uintptr_t)*(void **)((u8 *)ctx + 0xA4),
                   (int)*(u32 *)((u8 *)ctx + 0xA0));
        break;
    case 2:
    case 3:
        REC35_POST((u8 *)ctx + 0xA0, 18);
        REC35_BIND(*(void **)((u8 *)ctx + 12),
                   (int)(uintptr_t)*(void **)((u8 *)ctx + 0xA4),
                   (int)*(u32 *)((u8 *)ctx + 0xA0));
        break;
    }
}
__asm__(".align 2, 0");

void Rec35_StageA(void *ctx){
    s16 ph = *(volatile s16 *)((u8*)ctx + 0x94);
    if (ph>4) return;
    switch(ph){
        case 0: *(volatile s16 *)((u8*)ctx+0x94)=4; break;
        case 1: *(volatile s16 *)((u8*)ctx+0x90)=1; Rec35_HelperA(ctx); ScenePost(1,1); *(volatile s16 *)((u8*)ctx+0x94)=2; break;
        case 2: *(volatile s16 *)((u8*)ctx+0x90)=1; if (Sound_Cmd3()==0){ *(volatile s16 *)((u8*)ctx+20)=1; GuardedSaveOnly(); Rec35_HelperA(ctx); ScenePost(1,1); *(volatile s16 *)((u8*)ctx+0x94)=3; } else *(volatile s16 *)((u8*)ctx+20)=0; break;
        default: break;
    }
}
void Rec35_StageB(void *ctx){
    s16 ph = *(volatile s16 *)((u8*)ctx + 0x96);
    if (ph>4) return;
    switch(ph){
        case 0: *(volatile s16 *)((u8*)ctx+0x96)=4; break;
        case 1: *(volatile s16 *)((u8*)ctx+0x90)=1; Rec35_HelperB(ctx); ScenePost(1,1); *(volatile s16 *)((u8*)ctx+0x96)=2; break;
        case 2: *(volatile s16 *)((u8*)ctx+0x90)=1; if (Sound_Cmd3()==0){ *(volatile s16 *)((u8*)ctx+20)=1; GuardedFullSave(); GuardedSaveOnly(); Rec35_HelperB(ctx); ScenePost(1,1); *(volatile s16 *)((u8*)ctx+0x96)=3; } else *(volatile s16 *)((u8*)ctx+20)=0; break;
        default: break;
    }
}

#ifndef __APPLE__
void _08015ED0(void *c) __attribute__((alias("Rec35_StageA")));
void _08015F68(void *c) __attribute__((alias("Rec35_StageB")));
void _08015E28(void *c) __attribute__((alias("Rec35_HelperA")));
void _08015E7C(void *c) __attribute__((alias("Rec35_HelperB")));
#endif
