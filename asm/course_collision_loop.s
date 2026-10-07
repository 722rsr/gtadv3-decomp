@ GT Advance 3 - course proximity iterator
@ Region: file offset 0x007110-0x007210 (VMA 0x08007110-0x08007210).
@ Exact ARMCC high-register Thumb transcription with one private threshold
@ pool at 0x720C.

.thumb
.type sub_08007110, %function
sub_08007110:
_08007110:
        push {r4, r5, r6, r7, lr}
        mov r7, sl
        mov r6, r9
        mov r5, r8
        push {r5, r6, r7}
        sub sp, #24
        adds r4, r1, #0
        adds r3, r2, #0
        movs r1, #0
        str r3, [sp, #20]
        bl 0x08006C10
        mov r9, r0
        ldr r1, [r4, #0]
        ldr r0, [r0, #12]
        subs r6, r1, r0
        ldr r1, [r4, #4]
        mov r2, r9
        ldr r0, [r2, #16]
        subs r7, r1, r0
        movs r4, #40
        add r4, r9
        mov r8, r4
        movs r5, #0
        mov sl, r5
        movs r0, #1
        str r0, [sp, #12]
        ldr r3, [sp, #20]
_08007148:
        mov r0, r9
        adds r0, #44
        add r0, sl
        ldr r2, [r0, #0]
        str r2, [sp, #0]
        mov r0, r9
        adds r0, #40
        add r0, sl
        ldr r1, [r0, #0]
        negs r1, r1
        str r1, [sp, #4]
        adds r0, r2, #0
        muls r0, r6
        muls r1, r7
        adds r0, r0, r1
        str r0, [sp, #8]
        adds r0, r3, #0
        mov r1, r8
        mov r2, sp
        str r3, [sp, #20]
        bl 0x08005E14
        ldr r3, [sp, #20]
        ldr r1, [r3, #0]
        asrs r5, r1, #8
        mov r2, r8
        movs r4, #12
        ldrsh r0, [r2, r4]
        cmp r5, r0
        blt.n _080071EA
        movs r4, #16
        ldrsh r0, [r2, r4]
        cmp r5, r0
        bgt.n _080071EA
        ldr r0, [r3, #4]
        asrs r4, r0, #8
        movs r5, #14
        ldrsh r0, [r2, r5]
        cmp r4, r0
        blt.n _080071EA
        movs r5, #18
        ldrsh r0, [r2, r5]
        cmp r4, r0
        bgt.n _080071EA
        subs r5, r1, r6
        adds r0, r5, #0
        bl 0x08005B5C
        movs r1, #128
        lsls r1, r1, #3
        ldr r3, [sp, #20]
        cmp r0, r1
        bgt.n _080071EA
        ldr r0, [r3, #4]
        subs r4, r0, r7
        adds r0, r4, #0
        str r1, [sp, #16]
        bl 0x08005B5C
        ldr r1, [sp, #16]
        ldr r3, [sp, #20]
        cmp r0, r1
        bgt.n _080071EA
        adds r0, r5, #0
        muls r0, r5
        adds r1, r4, #0
        muls r1, r4
        adds r0, r0, r1
        ldr r1, _0800720C
        cmp r0, r1
        bgt.n _080071E0
        ldr r0, [r3, #0]
        mov r2, r9
        ldr r1, [r2, #12]
        adds r0, r0, r1
        str r0, [r3, #0]
_080071E0:
        ldr r0, [r3, #4]
        mov r4, r9
        ldr r1, [r4, #16]
        adds r0, r0, r1
        str r0, [r3, #4]
_080071EA:
        movs r5, #28
        add r8, r5
        add sl, r5
        ldr r0, [sp, #12]
        subs r0, #1
        str r0, [sp, #12]
        cmp r0, #0
        bge.n _08007148
        movs r0, #0
        add sp, #24
        pop {r3, r4, r5}
        mov r8, r3
        mov r9, r4
        mov sl, r5
        pop {r4, r5, r6, r7}
        pop {r1}
        bx r1
_0800720C: .word 0x001FFFFF
