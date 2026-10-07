@ GT Advance 3 - course resource setup helper
@ Region: file offset 0x007664-0x007770 (VMA 0x08007664-0x08007770).
@ Exact ARMCC Thumb transcription with the DMA3 descriptor pool at 0x776C.

.thumb
.type sub_08007664, %function
sub_08007664:
_08007664:
        push {r4, r5, r6, r7, lr}
        mov r7, sl
        mov r6, r9
        mov r5, r8
        push {r5, r6, r7}
        sub sp, #20
        mov r8, r0
        adds r0, r1, #0
        adds r1, r2, #0
        bl 0x08007498
        adds r4, r0, #0
        movs r1, #0
        bl 0x08007498
        adds r5, r0, #0
        adds r0, r4, #0
        movs r1, #1
        bl 0x08007498
        mov sl, r0
        adds r0, r4, #0
        movs r1, #2
        bl 0x08007498
        mov r9, r0
        adds r0, r4, #0
        bl 0x0800748C
        adds r7, r0, #0
        adds r0, r5, #0
        bl 0x08007658
        adds r1, r0, #0
        mov r0, r8
        bl 0x080050D0
        str r0, [sp, #16]
        ldrh r6, [r7, #6]
        cmp r6, #0
        beq.n _080076FC
        adds r0, r5, #0
        bl 0x0800748C
        adds r4, r0, #0
        mov r0, r8
        bl sub_080056C4
        adds r1, r0, #0
        adds r0, r4, #0
        bl 0x0802D984
        mov r0, r9
        bl 0x0800748C
        movs r4, #128
        lsls r4, r4, #18
        adds r1, r4, #0
        bl 0x0802D988
        ldrh r1, [r7, #0]
        lsrs r0, r1, #3
        str r0, [sp, #0]
        ldrh r7, [r7, #2]
        lsrs r0, r7, #3
        str r0, [sp, #4]
        movs r0, #0
        str r0, [sp, #8]
        str r0, [sp, #12]
        mov r0, r8
        adds r1, r4, #0
        movs r2, #0
        movs r3, #0
        bl 0x080052F0
        b.n _08007738
_080076FC:
        adds r0, r5, #0
        bl 0x0800748C
        adds r4, r0, #0
        adds r0, r5, #0
        bl 0x080074A8
        adds r3, r0, #0
        mov r0, r8
        adds r1, r4, #0
        movs r2, #0
        bl 0x08005260
        mov r0, r9
        bl 0x0800748C
        adds r1, r0, #0
        ldrh r2, [r7, #0]
        lsrs r0, r2, #3
        str r0, [sp, #0]
        ldrh r7, [r7, #2]
        lsrs r0, r7, #3
        str r0, [sp, #4]
        str r6, [sp, #8]
        str r6, [sp, #12]
        mov r0, r8
        movs r2, #0
        movs r3, #0
        bl 0x080052F0
_08007738:
        ldr r4, _0800776C
        mov r0, sl
        bl 0x0800748C
        str r0, [r4, #0]
        movs r0, #160
        lsls r0, r0, #19
        str r0, [r4, #4]
        mov r0, sl
        bl 0x080074A8
        lsrs r0, r0, #2
        movs r1, #132
        lsls r1, r1, #24
        orrs r0, r1
        str r0, [r4, #8]
        ldr r0, [r4, #8]
        ldr r0, [sp, #16]
        add sp, #20
        pop {r3, r4, r5}
        mov r8, r3
        mov r9, r4
        mov sl, r5
        pop {r4, r5, r6, r7}
        pop {r1}
        bx r1
_0800776C: .word 0x040000D4
