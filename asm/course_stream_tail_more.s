@ GT Advance 3 - course streaming predicates and movement helpers
@ Region: file offset 0x006D68-0x006E70 (VMA 0x08006D68-0x08006E70).
@ Exact Thumb transcription; no literal pools in this range.

.thumb
.type sub_08006D68, %function
sub_08006D68:
_08006D68:
        movs r1, #0
        ldr r0, [r0, #24]
        cmp r0, #2
        bne.n _08006D72
        movs r1, #1
_08006D72:
        adds r0, r1, #0
        bx lr
        .short 0x0000

.type sub_08006D78, %function
sub_08006D78:
_08006D78:
        movs r1, #0
        ldr r0, [r0, #24]
        cmp r0, #4
        bne.n _08006D82
        movs r1, #1
_08006D82:
        adds r0, r1, #0
        bx lr
        .short 0x0000

.type sub_08006D88, %function
sub_08006D88:
_08006D88:
        movs r2, #0
        ldr r1, [r0, #24]
        movs r0, #128
        lsls r0, r0, #19
        cmp r1, r0
        bne.n _08006D96
        movs r2, #1
_08006D96:
        adds r0, r2, #0
        bx lr
        .short 0x0000

.type sub_08006D9C, %function
sub_08006D9C:
_08006D9C:
        push {r4, r5, r6, r7, lr}
        mov r7, r9
        mov r6, r8
        push {r6, r7}
        mov r8, r0
        mov r9, r1
        movs r1, #0
        bl 0x08006C10
        adds r6, r0, #0
        cmp r6, #0
        beq.n _08006E0E
        movs r1, #1
        negs r1, r1
        mov r0, r8
        bl 0x08006C10
        adds r7, r0, #0
        cmp r7, #0
        beq.n _08006DF2
        mov r0, r8
        movs r1, #0
        bl 0x08006C10
        movs r2, #28
        ldrsh r1, [r0, r2]
        ldr r2, [r0, #12]
        adds r2, r2, r1
        movs r1, #28
        ldrsh r3, [r7, r1]
        ldr r1, [r7, #12]
        adds r1, r1, r3
        subs r4, r2, r1
        movs r1, #30
        ldrsh r2, [r0, r1]
        ldr r1, [r0, #16]
        adds r1, r1, r2
        movs r0, #30
        ldrsh r2, [r7, r0]
        ldr r0, [r7, #16]
        adds r0, r0, r2
        subs r5, r1, r0
        b.n _08006DFA
_08006DF2:
        movs r1, #24
        ldrsh r4, [r6, r1]
        movs r2, #26
        ldrsh r5, [r6, r2]
_08006DFA:
        mov r1, r9
        ldr r0, [r1, #0]
        muls r0, r4
        ldr r1, [r1, #4]
        muls r1, r5
        adds r0, r0, r1
        cmp r0, #0
        bge.n _08006E0E
        movs r0, #1
        b.n _08006E10
_08006E0E:
        movs r0, #0
_08006E10:
        pop {r3, r4}
        mov r8, r3
        mov r9, r4
        pop {r4, r5, r6, r7}
        pop {r1}
        bx r1

.type sub_08006E1C, %function
sub_08006E1C:
_08006E1C:
        push {r4, r5, r6, lr}
        adds r4, r0, #0
        adds r6, r1, #0
        movs r1, #0
        bl 0x08006C10
        adds r5, r0, #0
        cmp r5, #0
        bne.n _08006E32
        movs r0, #0
        b.n _08006E68
_08006E32:
        adds r0, r5, #0
        adds r1, r6, #0
        bl 0x08006C94
        cmp r0, #0
        blt.n _08006E5C
        movs r1, #1
        negs r1, r1
        adds r0, r4, #0
        bl 0x08006C10
        adds r4, r0, #0
        cmp r4, #0
        beq.n _08006E66
        adds r1, r6, #0
        bl 0x08006C94
        cmp r0, #0
        blt.n _08006E66
        adds r0, r4, #0
        b.n _08006E68
_08006E5C:
        adds r0, r4, #0
        movs r1, #1
        bl 0x08006C10
        b.n _08006E68
_08006E66:
        adds r0, r5, #0
_08006E68:
        pop {r4, r5, r6}
        pop {r1}
        bx r1
        .short 0x0000
@ End-of-region anchor for the splicer. One `@ Region:` (0x08006D68-0x08006E70),
@ no `.include`, ends at 0x08006E70 -- exactly where sub_08006E1C's span ends.
@ Zero bytes.
course_stream_tail_more_end:
