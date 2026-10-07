@ GT Advance 3 - course proximity search continuation
@ Region: file offset 0x007368-0x00748C (VMA 0x08007368-0x0800748C).
@ Exact ARMCC high-register Thumb transcription; includes the adjacent leaf.

.thumb
.type sub_08007368, %function
sub_08007368:
_08007368:
        push {r4, r5, r6, r7, lr}
        mov r7, sl
        mov r6, r9
        mov r5, r8
        push {r5, r6, r7}
        sub sp, #56
        adds r4, r1, #0
        mov r9, r3
        lsls r2, r2, #16
        lsrs r2, r2, #16
        str r2, [sp, #36]
        movs r1, #0
        bl 0x08006C10
        adds r7, r0, #0
        add r2, sp, #12
        ldr r0, [r4, #0]
        ldr r1, [r7, #12]
        subs r0, r0, r1
        str r0, [sp, #12]
        ldr r0, [r4, #4]
        ldr r1, [r7, #16]
        subs r0, r0, r1
        str r0, [r2, #4]
        movs r0, #0
        str r0, [sp, #40]
        mov r1, sp
        adds r1, #20
        str r1, [sp, #52]
        mov r0, sp
        adds r0, #28
        str r0, [sp, #48]
_080073A8:
        ldr r1, [sp, #40]
        cmp r1, #0
        beq.n _080073B4
        movs r0, #32
        negs r0, r0
        b.n _080073B6
_080073B4:
        movs r0, #32
_080073B6:
        str r0, [sp, #20]
        movs r0, #64
        str r0, [sp, #24]
        ldr r0, [sp, #36]
        lsls r1, r0, #16
        ldr r0, [sp, #52]
        asrs r1, r1, #16
        bl 0x08005BA8
        mov r0, sp
        add r1, sp, #12
        bl 0x08005DA4
        movs r1, #0
        str r1, [sp, #44]
        ldr r0, [sp, #48]
        mov r8, r0
        adds r4, r7, #0
        adds r4, #40
        mov sl, r1
_080073DE:
        mov r0, r8
        mov r1, sp
        adds r2, r4, #0
        bl 0x08005E14
        lsls r0, r0, #24
        cmp r0, #0
        beq.n _08007458
        ldr r3, [sp, #28]
        asrs r5, r3, #8
        movs r1, #12
        ldrsh r0, [r4, r1]
        cmp r0, r5
        bgt.n _08007458
        movs r1, #16
        ldrsh r0, [r4, r1]
        cmp r0, r5
        blt.n _08007458
        mov r0, r8
        ldr r2, [r0, #4]
        asrs r6, r2, #8
        movs r1, #14
        ldrsh r0, [r4, r1]
        cmp r0, r6
        bgt.n _08007458
        movs r1, #18
        ldrsh r0, [r4, r1]
        cmp r0, r6
        blt.n _08007458
        ldr r0, [sp, #12]
        subs r5, r3, r0
        ldr r0, [sp, #16]
        subs r6, r2, r0
        adds r0, r5, #0
        muls r0, r5
        adds r1, r6, #0
        muls r1, r6
        adds r0, r0, r1
        cmp r0, #25
        bgt.n _08007458
        ldr r0, [r7, #12]
        adds r0, r3, r0
        mov r1, r9
        str r0, [r1, #0]
        ldr r0, [r7, #16]
        adds r0, r2, r0
        str r0, [r1, #4]
        adds r0, r7, #0
        adds r0, #40
        add r0, sl
        ldr r0, [r0, #0]
        negs r0, r0
        str r0, [r1, #8]
        adds r0, r7, #0
        adds r0, #44
        add r0, sl
        ldr r0, [r0, #0]
        negs r0, r0
        str r0, [r1, #12]
        movs r0, #1
        b.n _08007474
_08007458:
        adds r4, #28
        movs r0, #28
        add sl, r0
        ldr r1, [sp, #44]
        adds r1, #1
        str r1, [sp, #44]
        cmp r1, #1
        ble.n _080073DE
        ldr r0, [sp, #40]
        adds r0, #1
        str r0, [sp, #40]
        cmp r0, #1
        ble.n _080073A8
        movs r0, #0
_08007474:
        add sp, #56
        pop {r3, r4, r5}
        mov r8, r3
        mov r9, r4
        mov sl, r5
        pop {r4, r5, r6, r7}
        pop {r1}
        bx r1

.type sub_08007484, %function
sub_08007484:
_08007484:
        bx lr

        .short 0

.type sub_08007488, %function
sub_08007488:
_08007488:
        ldr r0, [r0, #8]
        bx lr
course_proximity_more_end:
