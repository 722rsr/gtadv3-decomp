@ GT Advance 3 - course proximity search
@ Region: file offset 0x007210-0x007368 (VMA 0x08007210-0x08007368).
@ Exact ARMCC high-register Thumb transcription; pool-free.

.thumb
.type sub_08007210, %function
sub_08007210:
_08007210:
        push {r4, r5, r6, r7, lr}
        mov r7, sl
        mov r6, r9
        mov r5, r8
        push {r5, r6, r7}
        sub sp, #72
        adds r7, r1, #0
        movs r1, #0
        bl 0x08006C10
        mov r8, r0
        ldr r0, [r7, #8]
        cmp r0, #0
        bne.n _08007234
        ldr r0, [r7, #12]
        cmp r0, #0
        bne.n _08007234
        b.n _08007356
_08007234:
        add r4, sp, #24
        ldr r0, [r7, #0]
        mov r2, r8
        ldr r1, [r2, #12]
        subs r0, r0, r1
        str r0, [sp, #24]
        ldr r0, [r7, #4]
        ldr r1, [r2, #16]
        subs r0, r0, r1
        str r0, [r4, #4]
        ldr r0, [r7, #8]
        str r0, [r4, #8]
        ldr r0, [r7, #12]
        str r0, [r4, #12]
        mov r0, sp
        adds r1, r4, #0
        bl 0x08005DA4
        ldr r2, [sp, #24]
        ldr r0, [r4, #8]
        adds r0, r2, r0
        str r0, [sp, #52]
        adds r5, r0, #0
        ldr r1, [r4, #4]
        ldr r0, [r4, #12]
        adds r0, r1, r0
        str r0, [sp, #56]
        adds r6, r0, #0
        cmp r5, r2
        ble.n _08007272
        adds r5, r2, #0
_08007272:
        cmp r6, r1
        ble.n _08007278
        adds r6, r1, #0
_08007278:
        ldr r3, [sp, #52]
        cmp r3, r2
        bge.n _08007280
        str r2, [sp, #52]
_08007280:
        ldr r0, [sp, #56]
        cmp r0, r1
        bge.n _08007288
        str r1, [sp, #56]
_08007288:
        movs r1, #0
        str r1, [sp, #48]
_0800728C:
        mov r2, sp
        adds r2, #40
        str r2, [sp, #60]
        mov r4, r8
        adds r4, #40
        mov r9, r1
_08007298:
        ldr r0, [sp, #60]
        mov r1, sp
        adds r2, r4, #0
        bl 0x08005E14
        lsls r0, r0, #24
        cmp r0, #0
        beq.n _08007346
        mov r0, r8
        adds r0, #32
        add r0, r9
        ldr r1, [sp, #24]
        ldr r0, [r0, #0]
        subs r1, r1, r0
        str r1, [r7, #0]
        mov r0, r8
        adds r0, #36
        add r0, r9
        ldr r2, [sp, #28]
        ldr r0, [r0, #0]
        subs r2, r2, r0
        str r2, [r7, #4]
        mov r0, r8
        adds r0, #40
        add r0, r9
        mov sl, r0
        ldr r0, [r0, #0]
        muls r1, r0
        mov r0, r8
        adds r0, #44
        add r0, r9
        mov ip, r0
        ldr r0, [r0, #0]
        muls r0, r2
        adds r1, r1, r0
        ldr r3, [sp, #40]
        cmp r1, #0
        bge.n _08007324
        cmp r5, r3
        bgt.n _08007346
        ldr r0, [sp, #60]
        ldr r1, [r0, #4]
        cmp r6, r1
        bgt.n _08007346
        ldr r2, [sp, #52]
        cmp r2, r3
        blt.n _08007346
        ldr r0, [sp, #56]
        cmp r0, r1
        blt.n _08007346
        movs r2, #12
        ldrsh r0, [r4, r2]
        asrs r2, r3, #8
        str r2, [sp, #68]
        cmp r0, r2
        bgt.n _08007346
        movs r2, #14
        ldrsh r0, [r4, r2]
        asrs r1, r1, #8
        cmp r0, r1
        bgt.n _08007346
        movs r2, #16
        ldrsh r0, [r4, r2]
        ldr r2, [sp, #68]
        cmp r0, r2
        blt.n _08007346
        movs r2, #18
        ldrsh r0, [r4, r2]
        cmp r0, r1
        blt.n _08007346
_08007324:
        mov r1, r8
        ldr r0, [r1, #12]
        adds r0, r3, r0
        str r0, [r7, #0]
        ldr r2, [sp, #60]
        ldr r0, [r2, #4]
        ldr r1, [r1, #16]
        adds r0, r0, r1
        str r0, [r7, #4]
        mov r3, sl
        ldr r0, [r3, #0]
        str r0, [r7, #8]
        mov r1, ip
        ldr r0, [r1, #0]
        str r0, [r7, #12]
        movs r0, #1
        b.n _08007358
_08007346:
        adds r4, #28
        movs r2, #28
        add r9, r2
        ldr r3, [sp, #48]
        adds r3, #1
        str r3, [sp, #48]
        cmp r3, #1
        ble.n _08007298
_08007356:
        movs r0, #0
_08007358:
        add sp, #72
        pop {r3, r4, r5}
        mov r8, r3
        mov r9, r4
        mov sl, r5
        pop {r4, r5, r6, r7}
        pop {r1}
        bx r1
