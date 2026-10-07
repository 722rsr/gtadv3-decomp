@ GT Advance 3 - course resource helpers (DMA3 VRAM/palette setup)
@ Region: file offset 0x007770-0x00798C (VMA 0x08007770-0x0800798C).
@ Exact ARMCC Thumb transcription, four DMA3 descriptor pools at 0x7858/0x78E8/0x7920/0x7974 = 0x040000D4.

.thumb
.type sub_08007770, %function
sub_08007770:
_08007770:
        push {r4, r5, r6, r7, lr}
        mov r7, sl
        mov r6, r9
        mov r5, r8
        push {r5, r6, r7}
        sub sp, #28
        mov sl, r0
        adds r0, r1, #0
        adds r1, r2, #0
        str r3, [sp, #16]
        bl 0x08007498
        adds r4, r0, #0
        movs r1, #0
        bl 0x08007498
        adds r5, r0, #0
        adds r0, r4, #0
        movs r1, #1
        bl 0x08007498
        adds r6, r0, #0
        adds r0, r4, #0
        movs r1, #2
        bl 0x08007498
        adds r7, r0, #0
        adds r0, r4, #0
        bl 0x0800748C
        mov r8, r0
        adds r0, r5, #0
        bl 0x08007658
        adds r1, r0, #0
        mov r0, sl
        bl 0x080050D0
        mov r9, r0
        adds r0, r5, #0
        bl 0x0800748C
        adds r4, r0, #0
        adds r0, r5, #0
        bl 0x080074A8
        adds r3, r0, #0
        mov r0, sl
        adds r1, r4, #0
        mov r2, r9
        bl 0x08005260
        adds r0, r7, #0
        bl 0x0800748C
        adds r1, r0, #0
        mov r2, r8
        ldrh r2, [r2, #0]
        lsrs r0, r2, #3
        str r0, [sp, #0]
        mov r3, r8
        ldrh r3, [r3, #2]
        lsrs r0, r3, #3
        str r0, [sp, #4]
        mov r0, r9
        str r0, [sp, #8]
        ldr r2, [sp, #64]
        str r2, [sp, #12]
        mov r0, sl
        ldr r2, [sp, #16]
        ldr r3, [sp, #60]
        bl 0x080053DC
        ldr r4, _08007858
        adds r0, r6, #0
        bl 0x0800748C
        str r0, [r4, #0]
        ldr r3, [sp, #64]
        lsls r3, r3, #5
        str r3, [sp, #20]
        movs r0, #160
        lsls r0, r0, #19
        adds r0, r3, r0
        str r0, [r4, #4]
        adds r0, r6, #0
        bl 0x080074A8
        lsrs r0, r0, #2
        movs r1, #132
        lsls r1, r1, #24
        orrs r0, r1
        str r0, [r4, #8]
        ldr r0, [r4, #8]
        mov r1, r8
        ldrh r1, [r1, #0]
        lsrs r3, r1, #3
        mov r2, r8
        ldrh r2, [r2, #2]
        lsrs r0, r2, #3
        str r0, [sp, #0]
        mov r0, sl
        ldr r1, [sp, #16]
        ldr r2, [sp, #60]
        bl 0x080054A4
        mov r0, r9
        add sp, #28
        pop {r3, r4, r5}
        mov r8, r3
        mov r9, r4
        mov sl, r5
        pop {r4, r5, r6, r7}
        pop {r1}
        bx r1
        .short 0
_08007858: .word 0x040000D4

.type sub_0800785C, %function
sub_0800785C:
_0800785C:
        push {r4, r5, r6, lr}
        mov r6, sl
        mov r5, r9
        mov r4, r8
        push {r4, r5, r6}
        mov sl, r0
        adds r0, r1, #0
        adds r1, r2, #0
        adds r6, r3, #0
        bl 0x08007498
        adds r4, r0, #0
        movs r1, #0
        bl 0x08007498
        adds r5, r0, #0
        adds r0, r4, #0
        movs r1, #1
        bl 0x08007498
        mov r8, r0
        adds r0, r5, #0
        bl 0x08007658
        adds r1, r0, #0
        mov r0, sl
        bl 0x080050D0
        mov r9, r0
        adds r0, r5, #0
        bl 0x0800748C
        adds r4, r0, #0
        adds r0, r5, #0
        bl 0x080074A8
        adds r3, r0, #0
        mov r0, sl
        adds r1, r4, #0
        mov r2, r9
        bl 0x08005260
        ldr r4, _080078E8
        mov r0, r8
        bl 0x0800748C
        str r0, [r4, #0]
        lsls r6, r6, #5
        movs r0, #160
        lsls r0, r0, #19
        adds r6, r6, r0
        str r6, [r4, #4]
        mov r0, r8
        bl 0x080074A8
        lsrs r0, r0, #2
        movs r1, #132
        lsls r1, r1, #24
        orrs r0, r1
        str r0, [r4, #8]
        ldr r0, [r4, #8]
        mov r0, r9
        pop {r3, r4, r5}
        mov r8, r3
        mov r9, r4
        mov sl, r5
        pop {r4, r5, r6}
        pop {r1}
        bx r1
        .short 0
_080078E8: .word 0x040000D4

.type sub_080078EC, %function
sub_080078EC:
_080078EC:
        push {r4, r5, r6, lr}
        adds r6, r2, #0
        bl 0x08007498
        movs r1, #0
        bl 0x08007498
        adds r5, r0, #0
        ldr r4, _08007920
        bl 0x0800748C
        str r0, [r4, #0]
        str r6, [r4, #4]
        adds r0, r5, #0
        bl 0x080074A8
        lsrs r0, r0, #2
        movs r1, #132
        lsls r1, r1, #24
        orrs r0, r1
        str r0, [r4, #8]
        ldr r0, [r4, #8]
        pop {r4, r5, r6}
        pop {r0}
        bx r0
        .short 0
_08007920: .word 0x040000D4

.type sub_08007924, %function
sub_08007924:
_08007924:
        push {lr}
        bl 0x08007498
        movs r1, #2
        bl 0x08007498
        bl 0x0800748C
        pop {r1}
        bx r1

.type sub_08007938, %function
sub_08007938:
_08007938:
        push {r4, r5, r6, lr}
        adds r4, r2, #0
        bl 0x08007498
        movs r1, #1
        bl 0x08007498
        adds r6, r0, #0
        ldr r5, _08007974
        bl 0x0800748C
        str r0, [r5, #0]
        lsls r4, r4, #5
        movs r0, #160
        lsls r0, r0, #19
        adds r4, r4, r0
        str r4, [r5, #4]
        adds r0, r6, #0
        bl 0x080074A8
        lsrs r0, r0, #2
        movs r1, #132
        lsls r1, r1, #24
        orrs r0, r1
        str r0, [r5, #8]
        ldr r0, [r5, #8]
        pop {r4, r5, r6}
        pop {r0}
        bx r0
        .short 0
_08007974: .word 0x040000D4

.type sub_08007978, %function
sub_08007978:
_08007978:
        push {lr}
        bl 0x08007498
        movs r1, #0
        bl 0x08007498
        bl 0x0800748C
        pop {r1}
        bx r1

@ Unique end marker so the body owned by this file can be C-owned.
@ Emits no bytes.
course_resource_helpers_end:
