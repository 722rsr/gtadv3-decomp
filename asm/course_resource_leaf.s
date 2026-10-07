@ GT Advance 3 - course resource update helper
@ Region: file offset 0x007ABC-0x007B18 (VMA 0x08007ABC-0x08007B18).
@ Exact ARMCC Thumb transcription, including both private pools.

.thumb
.type sub_08007ABC, %function
sub_08007ABC:
_08007ABC:
        push {r4, r5, r6, lr}
        adds r5, r2, #0
        bl 0x08007498
        adds r4, r0, #0
        movs r1, #0
        bl 0x08007498
        adds r6, r0, #0
        adds r0, r4, #0
        bl 0x0800748C
        ldrb r0, [r0, #6]
        cmp r0, #0
        beq.n _08007AF0
        adds r0, r6, #0
        bl 0x0800748C
        lsls r1, r5, #5
        ldr r2, _08007AEC
        adds r1, r1, r2
        bl 0x0802D984
        b.n _08007B0E
_08007AEC:
        .word 0x06010000
_08007AF0:
        adds r0, r6, #0
        bl 0x0800748C
        adds r4, r0, #0
        lsls r5, r5, #5
        ldr r0, _08007B14
        adds r5, r5, r0
        adds r0, r6, #0
        bl 0x080074A8
        adds r2, r0, #0
        adds r0, r4, #0
        adds r1, r5, #0
        bl 0x08005614
_08007B0E:
        pop {r4, r5, r6}
        pop {r0}
        bx r0
_08007B14:
        .word 0x06010000
course_resource_leaf_end:
