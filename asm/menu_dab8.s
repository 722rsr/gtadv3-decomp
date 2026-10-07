@ GT Advance 3 - menu object initializer
@ Region: file offset 0x00DAB8-0x00DBE8 (VMA 0x0800DAB8-0x0800DBE8).
@ Pure Thumb, ARMCC; jump table at 0x0800DB24 (10 entries), private pools DB10/DB14/DB18/DB1C/DB20/DB78/DB7C/DBE0/DBE4.
@ First function of raw pocket post-menu_d9a4; next raw starts at 0x0800DBE8.

.thumb
.type sub_0800DAB8, %function
sub_0800DAB8:
_0800DAB8:
        push {r4, r5, r6, lr}
        adds r6, r0, #0
        ldr r5, _0800DB10
        adds r4, r6, #0
        adds r4, #0x24
        adds r0, r5, #0
        adds r1, r4, #0
        bl 0x0800798C
        adds r0, r4, #0
        bl 0x08007A58
        adds r0, r5, #0
        movs r1, #4
        movs r2, #1
        bl 0x080075E8
        adds r0, r5, #0
        movs r1, #3
        movs r2, #2
        bl 0x080075E8
        ldr r0, _0800DB14
        adds r1, r6, #0
        adds r1, #0x34
        bl 0x0800798C
        movs r0, #0x10
        bl 0x0800572C
        str r0, [r6, #0x44]
        ldr r0, _0800DB18
        ldr r1, _0800DB1C
        adds r0, r0, r1
        movs r1, #0
        ldrsh r0, [r0, r1]
        cmp r0, #9
        bhi _0800DB98
        lsls r0, r0, #2
        ldr r1, _0800DB20
        adds r0, r0, r1
        ldr r0, [r0]
        mov pc, r0
        .short 0x0000
_0800DB10: .word 0x082A798C
_0800DB14: .word 0x082A95E4
_0800DB18: .word 0x03001780
_0800DB1C: .word 0x00000FBC
_0800DB20: .word 0x0800DB24
        .word 0x0800DB4C
        .word 0x0800DB54
        .word 0x0800DB5C
        .word 0x0800DB88
        .word 0x0800DB90
        .word 0x0800DB50
        .word 0x0800DB58
        .word 0x0800DB60
        .word 0x0800DB8C
        .word 0x0800DB94
_0800DB4C:
        movs r0, #7
        b _0800DB96
_0800DB50:
        movs r0, #6
        b _0800DB96
_0800DB54:
        movs r0, #8
        b _0800DB96
_0800DB58:
        movs r0, #5
        b _0800DB96
_0800DB5C:
        movs r0, #1
        b _0800DB96
_0800DB60:
        ldr r0, _0800DB78
        ldr r1, _0800DB7C
        adds r0, r0, r1
        movs r1, #0
        ldrsh r0, [r0, r1]
        cmp r0, #1
        beq _0800DB80
        cmp r0, #2
        beq _0800DB84
        movs r0, #2
        b _0800DB96
        .short 0x0000
_0800DB78: .word 0x03001780
_0800DB7C: .word 0x00001078
_0800DB80:
        movs r0, #0xB
        b _0800DB96
_0800DB84:
        movs r0, #0xA
        b _0800DB96
_0800DB88:
        movs r0, #3
        b _0800DB96
_0800DB8C:
        movs r0, #0
        b _0800DB96
_0800DB90:
        movs r0, #4
        b _0800DB96
_0800DB94:
        movs r0, #9
_0800DB96:
        str r0, [r6, #0x48]
_0800DB98:
        ldr r0, [r6, #0x38]
        ldr r1, [r6, #0x48]
        ldr r2, [r6, #0x44]
        bl 0x08007ABC
        ldr r0, _0800DBE0
        adds r1, r6, #0
        adds r1, #0x3C
        bl 0x0800798C
        movs r0, #0x26
        bl 0x0800572C
        adds r2, r0, #0
        str r2, [r6, #0x50]
        ldr r0, [r6, #0x40]
        ldr r1, [r6, #0x54]
        bl 0x08007ABC
        ldr r0, _0800DBE4
        adds r1, r6, #0
        adds r1, #0x2C
        bl 0x0800798C
        movs r0, #0x3C
        bl 0x0800572C
        adds r2, r0, #0
        str r2, [r6, #0x5C]
        ldr r0, [r6, #0x30]
        ldr r1, [r6, #0x60]
        bl 0x08007ABC
        pop {r4, r5, r6}
        pop {r0}
        bx r0

        .balign 4, 0
_0800DBE0: .word 0x082AB030
_0800DBE4: .word 0x082B283C
