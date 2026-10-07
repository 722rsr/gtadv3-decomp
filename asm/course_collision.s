@ GT Advance 3 - course collision/proximity helper
@ Region: file offset 0x006E70-0x006FD4 (VMA 0x08006E70-0x08006FD4).
@ Exact ARMCC high-register Thumb transcription; pool-free.

.thumb
.type sub_08006E70, %function
sub_08006E70:
_08006E70:
        push {r4, r5, r6, r7, lr}
        mov r7, sl
        mov r6, r9
        mov r5, r8
        push {r5, r6, r7}
        sub sp, #36
        mov sl, r0
        mov r9, r1
        mov ip, r2
        ldr r0, [sp, #68]
        mov r8, r0
        ldr r2, [r2, #4]
        ldr r0, [r1, #4]
        subs r2, r2, r0
        asrs r2, r2, #4
        str r2, [sp, #28]
        str r2, [sp, #0]
        ldr r2, [r1, #0]
        mov r0, ip
        ldr r0, [r0, #0]
        subs r1, r2, r0
        asrs r0, r1, #4
        str r0, [sp, #4]
        ldr r1, [sp, #28]
        cmp r1, #0
        bne.n _08006EAA
        cmp r0, #0
        bne.n _08006EAA
        b.n _08006FC0
_08006EAA:
        mov r2, r9
        ldr r0, [r2, #0]
        mov r1, sl
        ldr r1, [r1, #12]
        subs r0, r0, r1
        asrs r6, r0, #4
        ldr r0, [r2, #4]
        mov r1, sl
        ldr r1, [r1, #16]
        subs r2, r0, r1
        asrs r7, r2, #4
        mov r2, ip
        ldr r2, [r2, #0]
        mov r0, sl
        ldr r0, [r0, #12]
        subs r2, r2, r0
        str r2, [sp, #32]
        asrs r1, r2, #4
        str r1, [sp, #12]
        mov r2, ip
        ldr r0, [r2, #4]
        mov r1, sl
        ldr r1, [r1, #16]
        subs r0, r0, r1
        asrs r0, r0, #4
        str r0, [sp, #16]
        ldr r0, [sp, #0]
        muls r0, r6
        ldr r1, [sp, #4]
        muls r1, r7
        adds r0, r0, r1
        str r0, [sp, #8]
        mov r2, r8
        adds r2, #8
        str r2, [sp, #20]
        adds r0, r3, #0
        adds r1, r2, #0
        mov r2, sp
        str r3, [sp, #24]
        bl 0x08005E14
        lsls r0, r0, #24
        ldr r3, [sp, #24]
        cmp r0, #0
        beq.n _08006F9A
        mov r1, r8
        ldr r0, [r1, #0]
        ldr r2, [sp, #12]
        subs r4, r2, r0
        ldr r0, [r1, #4]
        ldr r1, [sp, #16]
        subs r5, r1, r0
        mov r0, r8
        ldr r2, [r0, #12]
        adds r0, r2, #0
        muls r0, r5
        mov r1, r8
        ldr r6, [r1, #8]
        adds r1, r6, #0
        muls r1, r4
        adds r0, r0, r1
        cmp r0, #0
        ble.n _08006FC0
        mov r4, r8
        movs r5, #20
        ldrsh r0, [r4, r5]
        subs r0, #16
        ldr r1, [r3, #0]
        cmp r1, r0
        blt.n _08006FC0
        mov r4, r8
        movs r5, #24
        ldrsh r0, [r4, r5]
        adds r0, #16
        cmp r1, r0
        bgt.n _08006FC0
        mov r1, r8
        movs r4, #22
        ldrsh r0, [r1, r4]
        subs r0, #16
        ldr r1, [r3, #4]
        cmp r1, r0
        blt.n _08006FC0
        mov r5, r8
        movs r4, #26
        ldrsh r0, [r5, r4]
        adds r0, #16
        cmp r1, r0
        bgt.n _08006FC0
        str r2, [sp, #0]
        negs r1, r6
        str r1, [sp, #4]
        ldr r5, [sp, #12]
        adds r0, r2, #0
        muls r0, r5
        ldr r2, [sp, #16]
        muls r1, r2
        adds r0, r0, r1
        str r0, [sp, #8]
        adds r0, r3, #0
        ldr r1, [sp, #20]
        mov r2, sp
        str r3, [sp, #24]
        bl 0x08005E14
        lsls r0, r0, #24
        ldr r3, [sp, #24]
        cmp r0, #0
        beq.n _08006FC0
        ldr r0, [r3, #0]
        lsls r0, r0, #4
        mov r4, sl
        ldr r1, [r4, #12]
        adds r0, r0, r1
        str r0, [r3, #0]
        ldr r0, [r3, #4]
        lsls r0, r0, #4
        ldr r1, [r4, #16]
        adds r0, r0, r1
        b.n _08006FBA
_08006F9A:
        mov r1, r8
        ldr r0, [r1, #0]
        subs r4, r6, r0
        ldr r0, [r1, #4]
        subs r5, r7, r0
        ldr r0, [r1, #12]
        muls r0, r5
        ldr r1, [r1, #8]
        muls r1, r4
        adds r0, r0, r1
        cmp r0, #0
        ble.n _08006FC0
        mov r2, r9
        ldr r0, [r2, #0]
        str r0, [r3, #0]
        ldr r0, [r2, #4]
_08006FBA:
        str r0, [r3, #4]
        movs r0, #1
        b.n _08006FC2
_08006FC0:
        movs r0, #0
_08006FC2:
        add sp, #36
        pop {r3, r4, r5}
        mov r8, r3
        mov r9, r4
        mov sl, r5
        pop {r4, r5, r6, r7}
        pop {r1}
        bx r1
        .short 0x0000
