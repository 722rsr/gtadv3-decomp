@ GT Advance 3 - award record accessor pair
@ Region: file offset 0x0261B0-0x0261D4 (VMA 0x080261B0-0x080261D4).
@ Two exact pure-Thumb leaves sharing 0x030015F0 table base.
@ Each owns its private literal pool word + 0000 pad; no high-reg prologue.
@ Proven via sliced objdump --adjust-vma and xref.py BL scan
@ (17 callers for 261B0, 1 for 261C4, zero literal-pointer hits into middles).

.thumb
.type sub_080261B0, %function
sub_080261B0:
_080261B0:
        ldr r1, _080261C0
        lsls r0, r0, #16
        asrs r0, r0, #13
        adds r0, r0, r1
        movs r1, #0
        ldrsh r0, [r0, r1]
        bx lr
        .short 0x0000
_080261C0: .word 0x030015F0

.type sub_080261C4, %function
sub_080261C4:
_080261C4:
        ldr r2, _080261D0
        lsls r0, r0, #16
        asrs r0, r0, #13
        adds r0, r0, r2
        strh r1, [r0, #0]
        bx lr
_080261D0: .word 0x030015F0

@ End of region 0x080261D4. The last function here ends exactly at 0x080261D4, where
@ the next region file's prologue begins, so this anchor is the precise
@ end marker -- not a bounded-region guess. Without it the last body in
@ this file has no end marker to fall back on and the promotion screen
@ refuses it. Emits no bytes; the following file supplies the address.
ai_award_mid_end:
