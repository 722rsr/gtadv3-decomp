@ GT Advance 3 - course resource accessors
@ Region: file offset 0x00748C-0x007664 (VMA 0x0800748C-0x08007664).
@ Exact ARMCC Thumb transcription, including private literal pools.

.thumb
.type sub_0800748C, %function
sub_0800748C:
_0800748C:
        adds r1, r0, #0
        adds r0, #12
        ldr r1, [r1, #8]
        lsls r1, r1, #2
        adds r0, r0, r1
        bx lr

.type sub_08007498, %function
sub_08007498:
_08007498:
        lsls r1, r1, #2
        adds r2, r0, #0
        adds r2, #12
        adds r2, r2, r1
        ldr r1, [r2, #0]
        adds r0, r0, r1
        bx lr

        .short 0

.type sub_080074A8, %function
sub_080074A8:
_080074A8:
        ldr r0, [r0, #4]
        bx lr

.type sub_080074AC, %function
sub_080074AC:
_080074AC:
        push {r4, r5, lr}
        sub sp, #4
        adds r5, r0, #0
        adds r4, r1, #0
        movs r0, #0
        str r0, [sp, #0]
        ldr r2, _080074D0
        mov r0, sp
        bl 0x0802D974
        adds r0, r5, #0
        bl 0x08007484
        str r0, [r4, #4]
        add sp, #4
        pop {r4, r5}
        pop {r0}
        bx r0
_080074D0:
        .word 0x05000002

.type sub_080074D4, %function
sub_080074D4:
_080074D4:
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
        bge.n _0800752E
_080074F8:
        adds r0, r7, #0
        adds r1, r5, #0
        bl 0x08007498
        adds r4, r0, #0
        ldr r0, [r4, #0]
        cmp r0, #1
        bne.n _08007528
        adds r0, r7, #0
        adds r1, r5, #0
        adds r2, r6, #0
        bl 0x08007538
        adds r0, r4, #0
        bl 0x0800748C
        ldrb r0, [r0, #7]
        bl 0x0800572C
        adds r0, r4, #0
        bl 0x0800748C
        ldrb r0, [r0, #7]
        adds r6, r0, r6
_08007528:
        adds r5, #1
        cmp r5, r8
        blt.n _080074F8
_0800752E:
        pop {r3}
        mov r8, r3
        pop {r4, r5, r6, r7}
        pop {r0}
        bx r0

.type sub_08007538, %function
sub_08007538:
_08007538:
        push {r4, r5, r6, lr}
        adds r4, r2, #0
        bl 0x08007498
        movs r1, #0
        bl 0x08007498
        adds r5, r0, #0
        bl 0x0800748C
        adds r6, r0, #0
        lsls r4, r4, #5
        ldr r0, _0800756C
        adds r4, r4, r0
        adds r0, r5, #0
        bl 0x080074A8
        adds r2, r0, #0
        adds r0, r6, #0
        adds r1, r4, #0
        bl 0x08005614
        pop {r4, r5, r6}
        pop {r0}
        bx r0
        .short 0
_0800756C:
        .word 0x06010000

.type sub_08007570, %function
sub_08007570:
_08007570:
        push {r4, r5, r6, lr}
        adds r4, r2, #0
        adds r5, r3, #0
        ldr r6, [sp, #16]
        bl 0x08007498
        movs r1, #0
        bl 0x08007498
        bl 0x0800748C
        lsls r5, r5, #5
        adds r0, r0, r5
        lsls r4, r4, #5
        ldr r1, _080075A0
        adds r4, r4, r1
        lsls r6, r6, #5
        adds r1, r4, #0
        adds r2, r6, #0
        bl 0x08005614
        pop {r4, r5, r6}
        pop {r0}
        bx r0
_080075A0:
        .word 0x06010000

.type sub_080075A4, %function
sub_080075A4:
_080075A4:
        push {r4, r5, r6, r7, lr}
        mov r7, r8
        push {r7}
        sub sp, #12
        adds r4, r0, #0
        adds r6, r2, #0
        mov r8, r3
        ldr r7, [sp, #36]
        ldr r5, [sp, #40]
        ldr r0, [r4, #4]
        bl 0x08007498
        bl 0x0800748C
        ldrb r1, [r0, #6]
        ldrh r4, [r4, #0]
        adds r2, r1, r4
        str r5, [sp, #0]
        ldrb r1, [r0, #0]
        str r1, [sp, #4]
        ldrb r0, [r0, #1]
        str r0, [sp, #8]
        adds r0, r6, #0
        mov r1, r8
        adds r3, r7, #0
        bl 0x08002DB8
        add sp, #12
        pop {r3}
        mov r8, r3
        pop {r4, r5, r6, r7}
        pop {r0}
        bx r0
        .short 0

.type sub_080075E8, %function
sub_080075E8:
_080075E8:
        push {r4, lr}
        adds r4, r2, #0
        bl 0x08007498
        movs r1, #0
        bl 0x08007498
        bl 0x0800748C
        lsls r4, r4, #5
        ldr r1, _08007610
        adds r4, r4, r1
        adds r1, r4, #0
        movs r2, #32
        bl 0x08005614
        pop {r4}
        pop {r0}
        bx r0
        .short 0
_08007610:
        .word 0x05000200

.type sub_08007614, %function
sub_08007614:
_08007614:
        push {r4, r5, lr}
        adds r5, r2, #0
        adds r4, r3, #0
        bl 0x08007498
        movs r1, #0
        bl 0x08007498
        bl 0x0800748C
        lsls r5, r5, #5
        adds r0, r0, r5
        lsls r4, r4, #5
        ldr r1, _08007640
        adds r4, r4, r1
        adds r1, r4, #0
        movs r2, #32
        bl 0x08005614
        pop {r4, r5}
        pop {r0}
        bx r0
_08007640:
        .word 0x05000200

.type sub_08007644, %function
sub_08007644:
_08007644:
        push {lr}
        bl 0x08007498
        movs r1, #0
        bl 0x08007498
        bl 0x0800748C
        pop {r1}
        bx r1

.type sub_08007658, %function
sub_08007658:
_08007658:
        push {lr}
        bl 0x080074A8
        lsrs r0, r0, #5
        pop {r1}
        bx r1
course_resource_access_end:
