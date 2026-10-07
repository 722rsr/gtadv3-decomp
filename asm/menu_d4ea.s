@ GT Advance 3 - menu event/state handlers
@ Region: file offset 0x00D4EA-0x00D8E4 (VMA 0x0800D4EA-0x0800D8E4).
@ Exact pure-Thumb transcription.  The word tables are runtime event
@ dispatch tables and remain at their original offsets.

.thumb

        .hword 0x0000

.type sub_0800D4EC, %function
sub_0800D4EC:
_0800D4EC:
        push {r4, lr}
        sub sp, #4
        adds r4, r0, #0
        bl 0x0800254C
        ldr r0, _0800D520
        bl 0x08002124
        movs r0, #2
        strh r0, [r4, #2]
        movs r0, #1
        strb r0, [r4, #8]
        movs r0, #0
        bl 0x080022D8
        cmp r0, #0
        bne.n _0800D528
        movs r0, #100
        str r0, [sp, #0]
        adds r1, r4, #0
        adds r1, #12
        ldr r2, _0800D524
        mov r0, sp
        bl 0x0802D974
        b.n _0800D538
_0800D520: .word 0x00001399
_0800D524: .word 0x05000200
_0800D528:
        movs r0, #0
        str r0, [sp, #0]
        adds r1, r4, #0
        adds r1, #12
        ldr r2, _0800D540
        mov r0, sp
        bl 0x0802D974
_0800D538:
        add sp, #4
        pop {r4}
        pop {r0}
        bx r0
_0800D540: .word 0x05000200

.type sub_0800D544, %function
sub_0800D544:
_0800D544:
        push {r4, lr}
        adds r4, r0, #0
        movs r1, #0
        ldrsh r0, [r4, r1]
        cmp r0, #0
        beq.n _0800D556
        cmp r0, #1
        beq.n _0800D578
        b.n _0800D596
_0800D556:
        movs r0, #0
        bl 0x080022D8
        cmp r0, #0
        beq.n _0800D56E
        adds r0, r4, #0
        adds r0, #12
        movs r1, #128
        lsls r1, r1, #4
        bl 0x0800226C
        b.n _0800D590
_0800D56E:
        adds r0, r4, #0
        adds r0, #12
        bl 0x08002298
        b.n _0800D590
_0800D578:
        bl 0x080022C0
        cmp r0, #0
        bne.n _0800D590
        bl 0x080022CC
        adds r2, r0, #0
        movs r0, #70
        movs r1, #70
        bl 0x08003978
        b.n _0800D596
_0800D590:
        ldrh r0, [r4, #0]
        adds r0, #1
        strh r0, [r4, #0]
_0800D596:
        pop {r4}
        pop {r0}
        bx r0

.type sub_0800D59C, %function
sub_0800D59C:
_0800D59C:
        push {lr}
        bl 0x0800D544
        pop {r0}
        bx r0
        .hword 0x0000

.type sub_0800D5A8, %function
sub_0800D5A8:
_0800D5A8:
        push {r4, lr}
        sub sp, #12
        mov r1, sp
        ldr r0, _0800D5D8
        ldmia r0!, {r2, r3, r4}
        stmia r1!, {r2, r3, r4}
        ldr r1, _0800D5DC
        movs r0, #4
        bl 0x08003940
        ldr r4, _0800D5E0
        bl 0x08002140
        adds r3, r0, #0
        movs r0, #50
        movs r1, #30
        adds r2, r4, #0
        bl 0x08003F18
        add sp, #12
        pop {r4}
        pop {r0}
        bx r0
        .hword 0x0000
_0800D5D8: .word 0x0805F920
_0800D5DC: .word 0x0805F92C
_0800D5E0: .word 0x0805F938

.type sub_0800D5E4, %function
sub_0800D5E4:
_0800D5E4:
        push {r4, r5, r6, r7, lr}
        adds r5, r0, #0
        movs r6, #0
        movs r7, #0
        movs r4, #0
        b.n _0800D61E
_0800D5F0:
        adds r0, r4, #0
        movs r1, #1
        bl 0x08002178
        lsls r0, r0, #16
        lsrs r1, r0, #16
        movs r3, #1
        movs r0, #1
        ands r0, r1
        cmp r0, #0
        beq.n _0800D608
        movs r6, #1
_0800D608:
        movs r2, #2
        adds r0, r1, #0
        ands r0, r2
        cmp r0, #0
        beq.n _0800D61C
        cmp r4, #0
        bne.n _0800D618
        movs r7, #1
_0800D618:
        strh r2, [r5, #4]
        strb r3, [r5, #10]
_0800D61C:
        adds r4, #1
_0800D61E:
        bl 0x08002140
        cmp r4, r0
        blt.n _0800D5F0
        cmp r6, #0
        beq.n _0800D638
        movs r0, #1
        bl 0x08004BFC
        movs r0, #1
        bl 0x08004EC0
        b.n _0800D644
_0800D638:
        cmp r7, #0
        beq.n _0800D644
        movs r0, #1
        bl 0x08004EA8
        strb r6, [r5, #8]
_0800D644:
        pop {r4, r5, r6, r7}
        pop {r0}
        bx r0
        .hword 0x0000

.type sub_0800D64C, %function
sub_0800D64C:
_0800D64C:
        bx lr
        .hword 0x0000

.type sub_0800D650, %function
sub_0800D650:
_0800D650:
        push {lr}
        movs r0, #1
        bl 0x08004ED8
        pop {r0}
        bx r0

.type sub_0800D65C, %function
sub_0800D65C:
_0800D65C:
        adds r1, #88
        movs r0, #0
        strb r0, [r1, #0]
        bx lr

.type sub_0800D664, %function
sub_0800D664:
_0800D664:
        push {r4, lr}
        adds r4, r1, #0
        subs r0, #1
        cmp r0, #10
        bhi.n _0800D6D8
        lsls r0, r0, #2
        ldr r1, _0800D678
        adds r0, r0, r1
        ldr r0, [r0, #0]
        mov pc, r0
_0800D678:
        .word 0x0800D67C
        .word 0x0800D6B2
        .word 0x0800D6A8
        .word 0x0800D6D8
        .word 0x0800D6D8
        .word 0x0800D6CA
        .word 0x0800D6BA
        .word 0x0800D6D2
        .word 0x0800D6D8
        .word 0x0800D6D8
        .word 0x0800D6D8
        .word 0x0800D6D8
_0800D6A8:
        adds r0, r3, #0
        adds r1, r4, #0
        bl 0x0800D65C
        b.n _0800D6D8
_0800D6B2:
        adds r0, r3, #0
        bl 0x0800D4EC
        b.n _0800D6D8
_0800D6BA:
        lsls r1, r4, #16
        lsrs r1, r1, #16
        lsls r2, r2, #16
        lsrs r2, r2, #16
        adds r0, r3, #0
        bl 0x0800D64C
        b.n _0800D6D8
_0800D6CA:
        adds r0, r3, #0
        bl 0x0800D59C
        b.n _0800D6D8
_0800D6D2:
        adds r0, r3, #0
        bl 0x0800D5A8
_0800D6D8:
        pop {r4}
        pop {r0}
        bx r0
        .hword 0x0000

.type sub_0800D6E0, %function
sub_0800D6E0:
_0800D6E0:
        push {r4, lr}
        adds r4, r0, #0
        ldr r1, _0800D700
        movs r0, #0
        movs r2, #5
        bl 0x08007664
        movs r0, #150
        lsls r0, r0, #1
        str r0, [r4, #0]
        movs r0, #0
        str r0, [r4, #4]
        pop {r4}
        pop {r0}
        bx r0
        .hword 0x0000
_0800D700: .word 0x08292B40

.type sub_0800D704, %function
sub_0800D704:
_0800D704:
        push {lr}
        adds r1, r0, #0
        ldr r0, [r1, #0]
        cmp r0, #0
        ble.n _0800D714
        subs r0, #1
        str r0, [r1, #0]
        b.n _0800D722
_0800D714:
        ldr r0, [r1, #4]
        cmp r0, #0
        bne.n _0800D722
        movs r0, #1
        str r0, [r1, #4]
        bl 0x08004EC0
_0800D722:
        pop {r0}
        bx r0
        .hword 0x0000

.type sub_0800D728, %function
sub_0800D728:
_0800D728:
        bx lr
        .hword 0x0000

.type sub_0800D72C, %function
sub_0800D72C:
_0800D72C:
        adds r1, #84
        movs r0, #1
        strh r0, [r1, #0]
        bx lr

.type sub_0800D734, %function
sub_0800D734:
_0800D734:
        push {lr}
        cmp r0, #2
        beq.n _0800D74E
        cmp r0, #2
        bhi.n _0800D744
        cmp r0, #1
        beq.n _0800D756
        b.n _0800D774
_0800D744:
        cmp r0, #5
        beq.n _0800D75E
        cmp r0, #6
        beq.n _0800D766
        b.n _0800D774
_0800D74E:
        adds r0, r3, #0
        bl 0x0800D72C
        b.n _0800D774
_0800D756:
        adds r0, r3, #0
        bl 0x0800D6E0
        b.n _0800D774
_0800D75E:
        adds r0, r3, #0
        bl 0x0800D704
        b.n _0800D774
_0800D766:
        lsls r1, r1, #16
        lsrs r1, r1, #16
        lsls r2, r2, #16
        lsrs r2, r2, #16
        adds r0, r3, #0
        bl 0x0800D728
_0800D774:
        pop {r0}
        bx r0

.type sub_0800D778, %function
sub_0800D778:
_0800D778:
        bx lr
        .hword 0x0000

.type sub_0800D77C, %function
sub_0800D77C:
_0800D77C:
        str r1, [r0, #0]
        str r2, [r0, #4]
        bx lr
        .hword 0x0000

.type sub_0800D784, %function
sub_0800D784:
_0800D784:
        movs r1, #0
        strh r1, [r0, #4]
        movs r1, #1
        str r1, [r0, #0]
        bx lr
        .hword 0x0000

.type sub_0800D790, %function
sub_0800D790:
_0800D790:
        adds r2, r0, #0
        ldr r1, [r2, #16]
        subs r1, #2
        str r1, [r2, #16]
        ldr r0, [r2, #24]
        adds r0, #4
        str r0, [r2, #24]
        cmp r1, #144
        ble.n _0800D7A6
        cmp r0, #0
        blt.n _0800D7B2
_0800D7A6:
        movs r0, #0
        str r0, [r2, #24]
        movs r0, #144
        str r0, [r2, #16]
        movs r0, #2
        str r0, [r2, #0]
_0800D7B2:
        bx lr

.type sub_0800D7B4, %function
sub_0800D7B4:
_0800D7B4:
        movs r1, #1
        strh r1, [r0, #4]
        movs r1, #3
        str r1, [r0, #0]
        bx lr
        .hword 0x0000

.type sub_0800D7C0, %function
sub_0800D7C0:
_0800D7C0:
        bx lr
        .hword 0x0000

.type sub_0800D7C4, %function
sub_0800D7C4:
_0800D7C4:
        movs r1, #11
        str r1, [r0, #0]
        bx lr
        .hword 0x0000

.type sub_0800D7CC, %function
sub_0800D7CC:
_0800D7CC:
        push {lr}
        adds r1, r0, #0
        ldr r0, [r1, #32]
        subs r0, #1
        str r0, [r1, #32]
        cmp r0, #0
        bgt.n _0800D7FC
        movs r0, #12
        str r0, [r1, #0]
        ldr r0, [r1, #28]
        cmp r0, #0
        beq.n _0800D7EA
        cmp r0, #1
        beq.n _0800D7F4
        b.n _0800D7FC
_0800D7EA:
        movs r2, #10
        ldrsh r0, [r1, r2]
        bl 0x08004EA8
        b.n _0800D7FC
_0800D7F4:
        movs r2, #8
        ldrsh r0, [r1, r2]
        bl 0x08004EC0
_0800D7FC:
        pop {r0}
        bx r0

.type sub_0800D800, %function
sub_0800D800:
_0800D800:
        adds r2, r0, #0
        ldr r1, [r2, #16]
        adds r1, #2
        str r1, [r2, #16]
        ldr r0, [r2, #24]
        subs r3, r0, #4
        str r3, [r2, #24]
        cmp r1, #159
        bgt.n _0800D81A
        movs r0, #32
        negs r0, r0
        cmp r3, r0
        bgt.n _0800D824
_0800D81A:
        movs r0, #32
        negs r0, r0
        str r0, [r2, #24]
        movs r0, #160
        str r0, [r2, #16]
_0800D824:
        bx lr
        .hword 0x0000

.type sub_0800D828, %function
sub_0800D828:
_0800D828:
        push {r4, lr}
        adds r4, r0, #0
        ldr r0, [r4, #28]
        cmp r0, #0
        beq.n _0800D838
        cmp r0, #1
        beq.n _0800D842
        b.n _0800D84A
_0800D838:
        movs r1, #10
        ldrsh r0, [r4, r1]
        bl 0x08004EA8
        b.n _0800D84A
_0800D842:
        movs r1, #8
        ldrsh r0, [r4, r1]
        bl 0x08004EC0
_0800D84A:
        movs r0, #14
        str r0, [r4, #0]
        pop {r4}
        pop {r0}
        bx r0

.type sub_0800D854, %function
sub_0800D854:
_0800D854:
        push {lr}
        adds r2, r0, #0
        ldr r0, [r2, #0]
        cmp r0, #14
        bhi.n _0800D8DE
        lsls r0, r0, #2
        ldr r1, _0800D868
        adds r0, r0, r1
        ldr r0, [r0, #0]
        mov pc, r0
_0800D868: .word 0x0800D86C
        .word 0x0800D8A8
        .word 0x0800D8B0
        .word 0x0800D8B8
        .word 0x0800D8DE
        .word 0x0800D8DE
        .word 0x0800D8DE
        .word 0x0800D8DE
        .word 0x0800D8DE
        .word 0x0800D8DE
        .word 0x0800D8DE
        .word 0x0800D8C0
        .word 0x0800D8C8
        .word 0x0800D8D0
        .word 0x0800D8D8
        .word 0x0800D8DE
_0800D8A8:
        adds r0, r2, #0
        bl 0x0800D784
        b.n _0800D8DE
_0800D8B0:
        adds r0, r2, #0
        bl 0x0800D790
        b.n _0800D8DE
_0800D8B8:
        adds r0, r2, #0
        bl 0x0800D7B4
        b.n _0800D8DE
_0800D8C0:
        adds r0, r2, #0
        bl 0x0800D7C4
        b.n _0800D8DE
_0800D8C8:
        adds r0, r2, #0
        bl 0x0800D7CC
        b.n _0800D8DE
_0800D8D0:
        adds r0, r2, #0
        bl 0x0800D800
        b.n _0800D8DE
_0800D8D8:
        adds r0, r2, #0
        bl 0x0800D828
_0800D8DE:
        pop {r0}
        bx r0
        .hword 0x0000
menu_d4ea_end:
