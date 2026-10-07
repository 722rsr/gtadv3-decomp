@ GT Advance 3 - course resource record emitter
@ Region: file offset 0x007B18-0x007B8C (VMA 0x08007B18-0x08007B8C).
@ Exact ARMCC high-register Thumb transcription; pool-free.

.thumb
.type sub_08007B18, %function
sub_08007B18:
_08007B18:
        push {r4, r5, r6, r7, lr}
        mov r7, sl
        mov r6, r9
        mov r5, r8
        push {r5, r6, r7}
        sub sp, #24
        adds r4, r0, #0
        mov sl, r2
        mov r9, r3
        ldr r0, [r4, #4]
        bl 0x08007498
        bl 0x0800748C
        adds r6, r0, #0
        ldrh r0, [r6, #2]
        ldrh r4, [r4, #0]
        adds r0, r0, r4
        mov r8, r0
        movs r5, #0
        ldrb r3, [r6, #7]
        cmp r5, r3
        bge.n _08007B7C
        movs r7, #1
        adds r4, r6, #0
        adds r4, #8
_08007B4C:
        ldrb r0, [r4, #2]
        add r0, sl
        ldrb r1, [r4, #3]
        add r1, r9
        ldrb r2, [r4, #0]
        add r2, r8
        ldr r3, [sp, #60]
        str r3, [sp, #0]
        ldrb r3, [r4, #1]
        str r3, [sp, #4]
        str r7, [sp, #8]
        ldr r3, [sp, #64]
        str r3, [sp, #12]
        ldr r3, [sp, #68]
        str r3, [sp, #16]
        str r7, [sp, #20]
        ldr r3, [sp, #56]
        bl 0x08002ED0
        adds r4, #4
        adds r5, #1
        ldrb r0, [r6, #7]
        cmp r5, r0
        blt.n _08007B4C
_08007B7C:
        add sp, #24
        pop {r3, r4, r5}
        mov r8, r3
        mov r9, r4
        mov sl, r5
        pop {r4, r5, r6, r7}
        pop {r0}
        bx r0
