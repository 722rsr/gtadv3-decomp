@ GT Advance 3 - course collision candidate helper
@ Region: file offset 0x006FD4-0x007110 (VMA 0x08006FD4-0x08007110).
@ Exact ARMCC high-register Thumb transcription; pool-free.

.thumb
.type sub_08006FD4, %function
sub_08006FD4:
_08006FD4:
        push {r4, r5, r6, r7, lr}
        mov r7, sl
        mov r6, r9
        mov r5, r8
        push {r5, r6, r7}
        sub sp, #20
        adds r7, r0, #0
        mov r9, r1
        mov r8, r2
        adds r5, r3, #0
        movs r0, #0
        mov sl, r0
        adds r0, r7, #0
        movs r1, #0
        bl 0x08006C10
        adds r6, r0, #0
        cmp r6, #0
        beq.n _080070E2
        mov r1, r8
        ldr r1, [r1, #0]
        mov ip, r1
        ldr r2, [r6, #12]
        subs r2, r1, r2
        str r2, [sp, #4]
        mov r4, r8
        ldr r3, [r4, #4]
        ldr r1, [r6, #16]
        subs r1, r3, r1
        str r1, [sp, #8]
        movs r4, #26
        ldrsh r0, [r6, r4]
        mov r8, r0
        mov r0, r8
        muls r0, r2
        mov r8, r0
        movs r2, #24
        ldrsh r0, [r6, r2]
        muls r0, r1
        mov r4, r8
        subs r2, r4, r0
        str r2, [sp, #12]
        movs r0, #0
        mov r8, r0
        mov r4, ip
        cmp r2, #0
        bgt.n _08007040
        movs r1, #1
        negs r1, r1
        mov r8, r1
        cmp r2, #0
        bge.n _08007040
        movs r2, #1
        mov r8, r2
_08007040:
        str r4, [sp, #4]
        str r3, [sp, #8]
        ldr r0, [r7, #0]
        ldrb r0, [r0, #0]
        cmp r0, #0
        bne.n _080070B2
        movs r1, #1
        negs r1, r1
        adds r0, r7, #0
        bl 0x08006C10
        cmp r0, #0
        bne.n _08007080
        ldr r0, [r7, #0]
        adds r0, #4
        str r0, [sp, #0]
        adds r0, r6, #0
        mov r1, r9
        add r2, sp, #4
        adds r3, r5, #0
        bl 0x08006E70
        lsls r0, r0, #24
        cmp r0, #0
        ble.n _08007080
        ldr r0, [r7, #0]
        adds r0, #12
        mov sl, r0
        ldr r0, [r5, #0]
        str r0, [sp, #4]
        ldr r0, [r5, #4]
        str r0, [sp, #8]
_08007080:
        adds r0, r7, #0
        movs r1, #1
        bl 0x08006C10
        cmp r0, #0
        bne.n _080070B2
        ldr r0, [r7, #0]
        adds r0, #32
        str r0, [sp, #0]
        adds r0, r6, #0
        mov r1, r9
        add r2, sp, #4
        adds r3, r5, #0
        bl 0x08006E70
        lsls r0, r0, #24
        cmp r0, #0
        ble.n _080070B2
        ldr r0, [r7, #0]
        adds r0, #40
        mov sl, r0
        ldr r0, [r5, #0]
        str r0, [sp, #4]
        ldr r0, [r5, #4]
        str r0, [sp, #8]
_080070B2:
        mov r4, r8
        cmp r4, #0
        blt.n _080070E2
        lsls r0, r4, #3
        subs r0, r0, r4
        lsls r0, r0, #2
        adds r0, #32
        adds r4, r6, r0
        str r4, [sp, #0]
        adds r0, r6, #0
        mov r1, r9
        add r2, sp, #4
        adds r3, r5, #0
        bl 0x08006E70
        lsls r0, r0, #24
        cmp r0, #0
        ble.n _080070E2
        adds r4, #8
        mov sl, r4
        ldr r0, [r5, #0]
        str r0, [sp, #4]
        ldr r0, [r5, #4]
        str r0, [sp, #8]
_080070E2:
        mov r0, sl
        cmp r0, #0
        beq.n _080070FE
        ldr r0, [sp, #4]
        str r0, [r5, #0]
        ldr r0, [sp, #8]
        str r0, [r5, #4]
        mov r1, sl
        ldr r0, [r1, #0]
        str r0, [r5, #8]
        ldr r0, [r1, #4]
        str r0, [r5, #12]
        movs r0, #1
        b.n _08007100
_080070FE:
        movs r0, #0
_08007100:
        add sp, #20
        pop {r3, r4, r5}
        mov r8, r3
        mov r9, r4
        mov sl, r5
        pop {r4, r5, r6, r7}
        pop {r1}
        bx r1
