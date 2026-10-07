@ GT Advance 3 - course resource helper continuation
@ Region: file offset 0x00798C-0x007ABC (VMA 0x0800798C-0x08007ABC).
@ Exact ARMCC Thumb transcription, including the private literal pool.

.thumb
.type sub_0800798C, %function
sub_0800798C:
_0800798C:
        push {r4, r5, lr}
        sub sp, #4
        adds r5, r0, #0
        adds r4, r1, #0
        movs r0, #0
        str r0, [sp, #0]
        ldr r2, _080079B0
        mov r0, sp
        bl 0x0802D974
        adds r0, r5, #0
        bl 0x08007484
        str r0, [r4, #4]
        add sp, #4
        pop {r4, r5}
        pop {r0}
        bx r0
_080079B0:
        .word 0x05000002

.type sub_080079B4, %function
sub_080079B4:
_080079B4:
        push {r4, r5, r6, r7, lr}
        mov r7, r8
        push {r7}
        adds r4, r1, #0
        bl 0x08007484
        adds r7, r0, #0
        bl 0x08007488
        mov r8, r0
        adds r6, r4, #0
        movs r5, #0
        cmp r5, r8
        bge.n _080079FA
_080079D0:
        adds r0, r7, #0
        adds r1, r5, #0
        bl 0x08007498
        adds r4, r0, #0
        ldr r0, [r4, #0]
        cmp r0, #4
        bne.n _080079F4
        adds r0, r7, #0
        adds r1, r5, #0
        adds r2, r6, #0
        bl 0x08007ABC
        adds r0, r4, #0
        bl 0x0800748C
        ldrh r0, [r0, #0]
        adds r6, r0, r6
_080079F4:
        adds r5, #1
        cmp r5, r8
        blt.n _080079D0
_080079FA:
        pop {r3}
        mov r8, r3
        pop {r4, r5, r6, r7}
        pop {r0}
        bx r0

.type sub_08007A04, %function
sub_08007A04:
_08007A04:
        push {r4, r5, r6, r7, lr}
        mov r7, r8
        push {r7}
        adds r5, r0, #0
        adds r4, r1, #0
        ldr r7, [r5, #4]
        adds r0, r7, #0
        bl 0x08007488
        mov r8, r0
        strh r4, [r5, #0]
        lsls r4, r4, #16
        lsrs r6, r4, #16
        movs r4, #0
        cmp r4, r8
        bge.n _08007A4E
_08007A24:
        adds r0, r7, #0
        adds r1, r4, #0
        bl 0x08007498
        adds r5, r0, #0
        ldr r0, [r5, #0]
        cmp r0, #4
        bne.n _08007A48
        adds r0, r7, #0
        adds r1, r4, #0
        adds r2, r6, #0
        bl 0x08007ABC
        adds r0, r5, #0
        bl 0x0800748C
        ldrh r0, [r0, #0]
        adds r6, r0, r6
_08007A48:
        adds r4, #1
        cmp r4, r8
        blt.n _08007A24
_08007A4E:
        pop {r3}
        mov r8, r3
        pop {r4, r5, r6, r7}
        pop {r0}
        bx r0

.type sub_08007A58, %function
sub_08007A58:
_08007A58:
        push {r4, r5, r6, r7, lr}
        mov r7, r8
        push {r7}
        adds r4, r0, #0
        ldr r7, [r4, #4]
        adds r0, r7, #0
        bl 0x08007488
        mov r8, r0
        movs r0, #0
        bl 0x0800572C
        strh r0, [r4, #0]
        lsls r0, r0, #16
        lsrs r6, r0, #16
        movs r5, #0
        cmp r5, r8
        bge.n _08007AB2
_08007A7C:
        adds r0, r7, #0
        adds r1, r5, #0
        bl 0x08007498
        adds r4, r0, #0
        ldr r0, [r4, #0]
        cmp r0, #4
        bne.n _08007AAC
        adds r0, r7, #0
        adds r1, r5, #0
        adds r2, r6, #0
        bl 0x08007ABC
        adds r0, r4, #0
        bl 0x0800748C
        ldrh r0, [r0, #0]
        bl 0x0800572C
        adds r0, r4, #0
        bl 0x0800748C
        ldrh r0, [r0, #0]
        adds r6, r0, r6
_08007AAC:
        adds r5, #1
        cmp r5, r8
        blt.n _08007A7C
_08007AB2:
        pop {r3}
        mov r8, r3
        pop {r4, r5, r6, r7}
        pop {r0}
        bx r0
course_resource_more_end:
