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

void Rec35_HelperA(void *ctx){
    s16 phase = *(volatile s16 *)((u8*)ctx + 0x94);
    if (phase==1){ EventPost((void *)((u8*)ctx+0xA0),9); EventBind(*(void**)((u8*)ctx+12), (int)(uintptr_t)*(void**)((u8*)ctx+0xA4), (int)(uintptr_t)((u8*)ctx+0xA0)); }
    else if (phase>=2 && phase<=3){ EventPost((void *)((u8*)ctx+0xA0),10); EventBind(*(void**)((u8*)ctx+12), (int)(uintptr_t)*(void**)((u8*)ctx+0xA4), (int)(uintptr_t)((u8*)ctx+0xA0)); }
}
void Rec35_HelperB(void *ctx){
    s16 phase = *(volatile s16 *)((u8*)ctx + 0x96);
    if (phase==1){ EventPost((void *)((u8*)ctx+0xA0),17); EventBind(*(void**)((u8*)ctx+12), (int)(uintptr_t)*(void**)((u8*)ctx+0xA4), (int)(uintptr_t)((u8*)ctx+0xA0)); }
    else if (phase>=2 && phase<=3){ EventPost((void *)((u8*)ctx+0xA0),18); EventBind(*(void**)((u8*)ctx+12), (int)(uintptr_t)*(void**)((u8*)ctx+0xA4), (int)(uintptr_t)((u8*)ctx+0xA0)); }
}

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
