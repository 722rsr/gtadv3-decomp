@ GT Advance 3 - course resource record emitter continuation
@ Region: file offset 0x007B8C-0x007BFC (VMA 0x08007B8C-0x08007BFC).
@ Exact ARMCC high-register Thumb transcription; pool-free.

.thumb
.type sub_08007B8C, %function
sub_08007B8C:
_08007B8C:
        push {r4, r5, r6, r7, lr}
        mov r7, r9
        mov r6, r8
        push {r6, r7}
        sub sp, #24
        adds r4, r0, #0
        mov r9, r2
        mov r8, r3
        ldr r0, [r4, #4]
        bl 0x08007498
        bl 0x0800748C
        adds r6, r0, #0
        ldrh r0, [r6, #2]
        ldrh r4, [r4, #0]
        adds r7, r0, r4
        movs r5, #0
        ldrb r3, [r6, #7]
        cmp r5, r3
        bge.n _08007BEE
_08007BB6:
        adds r4, r6, #0
        adds r4, #8
_08007BBA:
        ldrb r0, [r4, #2]
        add r0, r9
        ldrb r1, [r4, #3]
        add r1, r8
        ldrb r3, [r4, #0]
        adds r2, r3, r7
        ldr r3, [sp, #56]
        str r3, [sp, #0]
        ldrb r3, [r4, #1]
        str r3, [sp, #4]
        movs r3, #1
        str r3, [sp, #8]
        ldr r3, [sp, #60]
        str r3, [sp, #12]
        ldr r3, [sp, #64]
        str r3, [sp, #16]
        movs r3, #0
        str r3, [sp, #20]
        ldr r3, [sp, #52]
        bl 0x08002ED0
        adds r4, #4
        adds r5, #1
        ldrb r0, [r6, #7]
        cmp r5, r0
        blt.n _08007BBA
_08007BEE:
        add sp, #24
        pop {r3, r4}
        mov r8, r3
        mov r9, r4
        pop {r4, r5, r6, r7}
        pop {r0}
        bx r0
course_resource_leaf_more2_end:
