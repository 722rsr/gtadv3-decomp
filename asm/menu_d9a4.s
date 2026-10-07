@ GT Advance 3 - menu object setup helpers
@ Region: file offset 0x00D9A4-0x00DAB8 (VMA 0x0800D9A4-0x0800DAB8).
@ Exact pure-Thumb transcription; the following object initializer begins at
@ 0x0800DAB8 and remains in the surrounding raw pocket.

.thumb
.type sub_0800D9A4, %function
sub_0800D9A4:
_0800D9A4:
        push {r4, r5, r6, lr}
        adds r5, r0, #0
        adds r4, r1, #0
        lsls r4, r4, #16
        lsrs r4, r4, #16
        lsls r2, r2, #16
        lsrs r6, r2, #16
        bl 0x08026948
        str r0, [r5, #0]
        lsls r4, r4, #16
        asrs r4, r4, #16
        adds r0, r4, #0
        bl 0x080022E4
        adds r4, r0, #0
        movs r0, #1
        negs r0, r0
        cmp r4, r0
        bne.n _0800D9CE
        movs r4, #23
_0800D9CE:
        ldr r0, [r5, #0]
        lsls r2, r6, #16
        asrs r2, r2, #16
        adds r1, r4, #0
        bl 0x08026A4C
        ldr r0, [r5, #0]
        movs r1, #1
        bl 0x08026A58
        ldr r0, [r5, #0]
        movs r1, #0
        bl 0x08026A60
        ldr r0, [r5, #0]
        adds r1, r4, #0
        bl 0x08026938
        pop {r4, r5, r6}
        pop {r0}
        bx r0

.type sub_0800D9F8, %function
sub_0800D9F8:
_0800D9F8:
        push {r4, r5, r6, lr}
        adds r5, r0, #0
        adds r4, r1, #0
        lsls r4, r4, #16
        lsrs r4, r4, #16
        lsls r2, r2, #16
        lsrs r6, r2, #16
        movs r0, #160
        lsls r0, r0, #5
        bl 0x080269AC
        str r0, [r5, #0]
        lsls r4, r4, #16
        asrs r4, r4, #16
        adds r0, r4, #0
        bl 0x080022E4
        adds r4, r0, #0
        movs r0, #1
        negs r0, r0
        cmp r4, r0
        bne.n _0800DA26
        movs r4, #23
_0800DA26:
        ldr r0, [r5, #0]
        lsls r2, r6, #16
        asrs r2, r2, #16
        adds r1, r4, #0
        bl 0x08026A4C
        ldr r0, [r5, #0]
        movs r1, #1
        bl 0x08026A58
        ldr r0, [r5, #0]
        movs r1, #0
        bl 0x08026A60
        ldr r0, [r5, #0]
        adds r1, r4, #0
        bl 0x08026938
        pop {r4, r5, r6}
        pop {r0}
        bx r0

.type sub_0800DA50, %function
sub_0800DA50:
_0800DA50:
        push {r4, r5, r6, lr}
        adds r6, r0, #0
        adds r0, r1, #0
        lsls r2, r2, #16
        lsrs r5, r2, #16
        lsls r0, r0, #16
        asrs r0, r0, #16
        bl 0x080022E4
        adds r4, r0, #0
        movs r0, #1
        negs r0, r0
        cmp r4, r0
        bne.n _0800DA6E
        movs r4, #23
_0800DA6E:
        ldr r0, [r6, #0]
        lsls r2, r5, #16
        asrs r2, r2, #16
        adds r1, r4, #0
        bl 0x08026A4C
        ldr r0, [r6, #0]
        adds r1, r4, #0
        bl 0x08026938
        pop {r4, r5, r6}
        pop {r0}
        bx r0

.type sub_0800DA88, %function
sub_0800DA88:
_0800DA88:
        push {r4, r5, lr}
        adds r5, r0, #0
        adds r0, r1, #0
        lsls r2, r2, #16
        lsrs r4, r2, #16
        lsls r0, r0, #16
        asrs r0, r0, #16
        bl 0x080022E4
        adds r1, r0, #0
        movs r0, #1
        negs r0, r0
        cmp r1, r0
        bne.n _0800DAA6
        movs r1, #23
_0800DAA6:
        lsls r2, r4, #16
        asrs r2, r2, #16
        adds r0, r5, #0
        bl 0x08026A4C
        pop {r4, r5}
        pop {r0}
        bx r0
        .short 0x0000
menu_d9a4_end:
