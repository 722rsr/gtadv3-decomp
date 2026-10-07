@ GT Advance 3 - bonus collection award helpers
@ Region: file offset 0x025FAC-0x026068 (VMA 0x08025FAC-0x08026068).
@ Exact Thumb transcription with private literal pools.

.thumb
.type sub_08025FAC, %function
sub_08025FAC:
_08025FAC:
        adds r3, r0, #0
        cmp r3, #0
        bge.n _08025FB4
        movs r3, #0
_08025FB4:
        cmp r3, #97
        ble.n _08025FBA
        movs r3, #98
_08025FBA:
        ldr r1, _08025FE8
        adds r0, r3, #0
        cmp r3, #0
        bge.n _08025FC4
        adds r0, r3, #7
_08025FC4:
        asrs r0, r0, #3
        adds r1, #32
        adds r0, r0, r1
        movs r1, #0
        ldrsb r1, [r0, r1]
        ldr r2, _08025FEC
        movs r0, #7
        ands r0, r3
        lsls r0, r0, #1
        adds r0, r0, r2
        movs r2, #0
        ldrsh r0, [r0, r2]
        ands r1, r0
        negs r0, r1
        orrs r0, r1
        lsrs r0, r0, #31
        bx lr
        .short 0x0000
_08025FE8: .word 0x03001780
_08025FEC: .word 0x080CD9D4

.type sub_08025FF0, %function
sub_08025FF0:
_08025FF0:
        ldr r1, _08026000
        movs r2, #175
        lsls r2, r2, #3
        adds r1, r1, r2
        adds r0, r0, r1
        movs r1, #1
        strb r1, [r0, #0]
        bx lr
_08026000: .word 0x03001780

.type sub_08026004, %function
sub_08026004:
_08026004:
        movs r2, #0
        ldr r1, _0802601C
        movs r3, #175
        lsls r3, r3, #3
        adds r1, r1, r3
        adds r0, r0, r1
        ldrb r0, [r0, #0]
        cmp r0, #1
        bne.n _08026018
        movs r2, #1
_08026018:
        adds r0, r2, #0
        bx lr
_0802601C: .word 0x03001780

.type sub_08026020, %function
sub_08026020:
_08026020:
        push {r4, lr}
        sub sp, #8
        adds r4, r0, #0
        ldr r1, _0802605C
        mov r0, sp
        movs r2, #6
        bl 0x0802E0A4
        cmp r4, #0
        bgt.n _08026036
        movs r4, #0
_08026036:
        cmp r4, #1
        ble.n _0802603C
        movs r4, #2
_0802603C:
        ldr r0, _08026060
        ldr r1, _08026064
        adds r0, r0, r1
        adds r0, r4, r0
        movs r1, #1
        strb r1, [r0, #0]
        lsls r0, r4, #1
        add r0, sp
        movs r1, #0
        ldrsh r0, [r0, r1]
        bl 0x08025F78
        add sp, #8
        pop {r4}
        pop {r0}
        bx r0
_0802605C: .word 0x08060D4C
_08026060: .word 0x03001780
_08026064: .word 0x0000057C
@ End-of-region anchor for the splicer: this file declares exactly one
@ `@ Region:` (0x025FAC-0x026068) and no `.include`, and the body at
@ 0x08026020 is 72 bytes, so it ends on 0x08026068 -- exactly that boundary.
@ Without it promotion_screen refuses the body with "no end marker in
@ ai_award_tail.s", even though the body is byte-exact at 72/72.
ai_award_tail_end:
