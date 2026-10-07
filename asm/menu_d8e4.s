@ GT Advance 3 - menu stage/state helpers
@ Region: file offset 0x00D8E4-0x00D9A4 (VMA 0x0800D8E4-0x0800D9A4).
@ Exact pure-Thumb transcription; this family has no literal pool.

.thumb

.type sub_0800D8E4, %function
sub_0800D8E4:
_0800D8E4:
        adds r2, r0, #0
        movs r0, #4
        ldrsh r3, [r2, r0]
        cmp r3, #2
        beq.n _0800D910
        cmp r3, #2
        bgt.n _0800D8F8
        cmp r3, #1
        beq.n _0800D902
        b.n _0800D95A
_0800D8F8:
        cmp r3, #3
        beq.n _0800D95A
        cmp r3, #4
        beq.n _0800D946
        b.n _0800D95A
_0800D902:
        movs r0, #2
        strh r0, [r2, #2]
        movs r1, #3
        strh r1, [r2, #0]
        ldr r1, [r2, #8]
        str r3, [r1, #0]
        b.n _0800D958
_0800D910:
        ldrh r0, [r2, #0]
        subs r0, #1
        strh r0, [r2, #0]
        lsls r0, r0, #16
        cmp r0, #0
        bgt.n _0800D95A
        ldrh r0, [r2, #2]
        subs r0, #1
        strh r0, [r2, #2]
        lsls r0, r0, #16
        cmp r0, #0
        bgt.n _0800D92C
        movs r0, #4
        strh r0, [r2, #4]
_0800D92C:
        ldr r1, [r2, #8]
        ldr r0, [r1, #0]
        cmp r0, #0
        beq.n _0800D93C
        cmp r0, #1
        bne.n _0800D940
        movs r0, #0
        b.n _0800D93E
_0800D93C:
        movs r0, #1
_0800D93E:
        str r0, [r1, #0]
_0800D940:
        movs r0, #3
        strh r0, [r2, #0]
        b.n _0800D95A
_0800D946:
        ldr r0, [r2, #12]
        movs r1, #10
        str r1, [r0, #0]
        str r1, [r0, #32]
        movs r1, #1
        str r1, [r0, #28]
        ldr r0, [r2, #8]
        str r1, [r0, #0]
        movs r0, #3
_0800D958:
        strh r0, [r2, #4]
_0800D95A:
        bx lr

.type sub_0800D95C, %function
sub_0800D95C:
_0800D95C:
        adds r3, r0, #0
        cmp r2, #0
        ble.n _0800D970
_0800D962:
        ldrb r0, [r1, #0]
        strb r0, [r3, #0]
        adds r3, #1
        adds r1, #1
        subs r2, #1
        cmp r2, #0
        bne.n _0800D962
_0800D970:
        bx lr
        .hword 0x0000

.type sub_0800D974, %function
sub_0800D974:
_0800D974:
        movs r1, #0
        strh r1, [r0, #0]
        bx lr
        .hword 0x0000

.type sub_0800D97C, %function
sub_0800D97C:
_0800D97C:
        adds r2, r0, #0
        ldrh r0, [r2, #0]
        subs r0, #1
        strh r0, [r2, #0]
        lsls r0, r0, #16
        cmp r0, #0
        bge.n _0800D9A0
        strh r1, [r2, #0]
        movs r1, #2
        ldrsh r0, [r2, r1]
        cmp r0, #0
        bne.n _0800D998
        movs r0, #1
        b.n _0800D99E
_0800D998:
        cmp r0, #1
        bne.n _0800D9A0
        movs r0, #0
_0800D99E:
        strh r0, [r2, #2]
_0800D9A0:
        bx lr
        .hword 0x0000
@ Region end (VMA 0x0800D9A4). Emits no bytes; gives the promotion screen an
@ end marker for the last function in this region instead of guessing.
menu_d8e4_end:
