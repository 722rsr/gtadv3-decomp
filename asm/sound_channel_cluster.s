@ GT Advance 3 - sound channel stream helpers
@ Region: file offset 0x02BC84-0x02BE78 (VMA 0x0802BC84-0x0802BE78).
@ Exact pure-Thumb transcription. The ROM table and DMA register word are
@ private pools at 0x2BCE4 and 0x2BE74.

.thumb
.type sub_0802BC84, %function
sub_0802BC84:
_0802BC84:
        push {r4, r5, lr}
        adds r5, r1, #0
        ldr r4, [r5, #32]
        cmp r4, #0
        beq.n _0802BCA8
_0802BC8E:
        ldrb r1, [r4, #0]
        movs r0, #199
        tst r0, r1
        beq.n _0802BC9C
        movs r0, #64
        orrs r1, r0
        strb r1, [r4, #0]
_0802BC9C:
        adds r0, r4, #0
        bl 0x0802BC64
        ldr r4, [r4, #52]
        cmp r4, #0
        bne.n _0802BC8E
_0802BCA8:
        movs r0, #0
        strb r0, [r5, #0]
        pop {r4, r5}
        pop {r0}
        bx r0
        .short 0x0000

.type sub_0802BCB4, %function
sub_0802BCB4:
_0802BCB4:
        mov ip, lr
        movs r1, #36
        ldr r2, _0802BCE4
_0802BCBA:
        ldr r3, [r2, #0]
        bl 0x0802BCCE
        stmia r0!, {r3}
        adds r2, #4
        subs r1, #1
        bgt.n _0802BCBA
        bx ip
        .short 0x0000

.type sub_0802BCCC, %function
sub_0802BCCC:
_0802BCCC:
        ldrb r3, [r2, #0]
_0802BCCE:
        push {r0}
        lsrs r0, r2, #25
        bne.n _0802BCE0
        ldr r0, _0802BCE4
        cmp r2, r0
        bcc.n _0802BCDE
        lsrs r0, r2, #14
        beq.n _0802BCE0
_0802BCDE:
        movs r3, #0
_0802BCE0:
        pop {r0}
        bx lr
_0802BCE4: .word 0x080614E0

.type sub_0802BCE8, %function
sub_0802BCE8:
_0802BCE8:
        ldr r2, [r1, #64]
_0802BCEA:
        adds r3, r2, #1
        str r3, [r1, #64]
        ldrb r3, [r2, #0]
        b.n _0802BCCE
        .short 0x0000

.type sub_0802BCF4, %function
sub_0802BCF4:
_0802BCF4:
        push {lr}
_0802BCF6:
        ldr r2, [r1, #64]
        ldrb r0, [r2, #3]
        lsls r0, r0, #8
        ldrb r3, [r2, #2]
        orrs r0, r3
        lsls r0, r0, #8
        ldrb r3, [r2, #1]
        orrs r0, r3
        lsls r0, r0, #8
        bl 0x0802BCCC
        orrs r0, r3
        str r0, [r1, #64]
        pop {r0}
        bx r0

.type sub_0802BD14, %function
sub_0802BD14:
_0802BD14:
        ldrb r2, [r1, #2]
        cmp r2, #3
        bcs.n _0802BD2C
        lsls r2, r2, #2
        adds r3, r1, r2
        ldr r2, [r1, #64]
        adds r2, #4
        str r2, [r3, #68]
        ldrb r2, [r1, #2]
        adds r2, #1
        strb r2, [r1, #2]
        b.n _0802BCF4
_0802BD2C:
        b.n _0802BC84
        .short 0x0000

.type sub_0802BD30, %function
sub_0802BD30:
_0802BD30:
        ldrb r2, [r1, #2]
        cmp r2, #0
        beq.n _0802BD42
        subs r2, #1
        strb r2, [r1, #2]
        lsls r2, r2, #2
        adds r3, r1, r2
        ldr r2, [r3, #68]
        str r2, [r1, #64]
_0802BD42:
        bx lr

.type sub_0802BD44, %function
sub_0802BD44:
_0802BD44:
        push {lr}
        ldr r2, [r1, #64]
        ldrb r3, [r2, #0]
        cmp r3, #0
        bne.n _0802BD54
        adds r2, #1
        str r2, [r1, #64]
        b.n _0802BCF6
_0802BD54:
        ldrb r3, [r1, #3]
        adds r3, #1
        strb r3, [r1, #3]
        mov ip, r3
        bl 0x0802BCE8
        cmp ip, r3
        bcs.n _0802BD66
        b.n _0802BCF6
_0802BD66:
        movs r3, #0
        strb r3, [r1, #3]
        adds r2, #5
        str r2, [r1, #64]
        pop {r0}
        bx r0
        .short 0x0000

.type sub_0802BD74, %function
sub_0802BD74:
_0802BD74:
        mov ip, lr
        bl 0x0802BCE8
        strb r3, [r1, #29]
        bx ip
        .short 0x0000

.type sub_0802BD80, %function
sub_0802BD80:
_0802BD80:
        mov ip, lr
        bl 0x0802BCE8
        lsls r3, r3, #1
        strh r3, [r0, #28]
        ldrh r2, [r0, #30]
        muls r3, r2
        lsrs r3, r3, #8
        strh r3, [r0, #32]
        bx ip

.type sub_0802BD94, %function
sub_0802BD94:
_0802BD94:
        mov ip, lr
        bl 0x0802BCE8
        strb r3, [r1, #10]
        ldrb r3, [r1, #0]
        movs r2, #12
        orrs r3, r2
        strb r3, [r1, #0]
        bx ip
        .short 0x0000

.type sub_0802BDA8, %function
sub_0802BDA8:
_0802BDA8:
        mov ip, lr
        ldr r2, [r1, #64]
        ldrb r3, [r2, #0]
        adds r2, #1
        str r2, [r1, #64]
        lsls r2, r3, #1
        adds r2, r2, r3
        lsls r2, r2, #2
        ldr r3, [r0, #48]
        adds r2, r2, r3
        ldr r3, [r2, #0]
        bl 0x0802BCCE
        str r3, [r1, #36]
        ldr r3, [r2, #4]
        bl 0x0802BCCE
        str r3, [r1, #40]
        ldr r3, [r2, #8]
        bl 0x0802BCCE
        str r3, [r1, #44]
        bx ip
        .short 0x0000

.type sub_0802BDD8, %function
sub_0802BDD8:
_0802BDD8:
        mov ip, lr
        bl 0x0802BCE8
        strb r3, [r1, #18]
        ldrb r3, [r1, #0]
        movs r2, #3
        orrs r3, r2
        strb r3, [r1, #0]
        bx ip
        .short 0x0000

.type sub_0802BDEC, %function
sub_0802BDEC:
_0802BDEC:
        mov ip, lr
        bl 0x0802BCE8
        subs r3, #64
        strb r3, [r1, #20]
        ldrb r3, [r1, #0]
        movs r2, #3
        orrs r3, r2
        strb r3, [r1, #0]
        bx ip

.type sub_0802BE00, %function
sub_0802BE00:
_0802BE00:
        mov ip, lr
        bl 0x0802BCE8
        subs r3, #64
        strb r3, [r1, #14]
        ldrb r3, [r1, #0]
        movs r2, #12
        orrs r3, r2
        strb r3, [r1, #0]
        bx ip

.type sub_0802BE14, %function
sub_0802BE14:
_0802BE14:
        mov ip, lr
        bl 0x0802BCE8
        strb r3, [r1, #15]
        ldrb r3, [r1, #0]
        movs r2, #12
        orrs r3, r2
        strb r3, [r1, #0]
        bx ip
        .short 0x0000

.type sub_0802BE28, %function
sub_0802BE28:
_0802BE28:
        mov ip, lr
        bl 0x0802BCE8
        strb r3, [r1, #27]
        bx ip
        .short 0x0000

.type sub_0802BE34, %function
sub_0802BE34:
_0802BE34:
        mov ip, lr
        bl 0x0802BCE8
        ldrb r0, [r1, #24]
        cmp r0, r3
        beq.n _0802BE4A
        strb r3, [r1, #24]
        ldrb r3, [r1, #0]
        movs r2, #15
        orrs r3, r2
        strb r3, [r1, #0]
_0802BE4A:
        bx ip

.type sub_0802BE4C, %function
sub_0802BE4C:
_0802BE4C:
        mov ip, lr
        bl 0x0802BCE8
        subs r3, #64
        strb r3, [r1, #12]
        ldrb r3, [r1, #0]
        movs r2, #12
        orrs r3, r2
        strb r3, [r1, #0]
        bx ip

.type sub_0802BE60, %function
sub_0802BE60:
_0802BE60:
        mov ip, lr
        ldr r2, [r1, #64]
        ldrb r3, [r2, #0]
        adds r2, #1
        ldr r0, _0802BE74
        adds r0, r0, r3
        bl _0802BCEA
        strb r3, [r0, #0]
        bx ip
_0802BE74: .word 0x04000060

@ Synthetic end anchor: sub_0802BE60's span reaches the region end and no
@ label assembles there, so a manifest entry needs this line to bound the
@ splice (promotion_screen has_end_anchor / match_c_slice.replace_body).
sound_channel_cluster_end:
