@ GT Advance 3 - award record setter
@ Region: file offset 0x026150-0x026180 (VMA 0x08026150-0x08026180).
@ Exact pure-Thumb helper with its private IWRAM table pool.

.thumb
.type sub_08026150, %function
sub_08026150:
_08026150:
        push {r4, r5, lr}
        adds r4, r0, #0
        adds r0, r1, #0
        lsls r4, r4, #16
        lsrs r4, r4, #16
        lsls r0, r0, #16
        lsrs r5, r0, #16
        asrs r0, r0, #16
        bl 0x0800572C
        ldr r1, _0802617C
        lsls r4, r4, #16
        asrs r4, r4, #13
        adds r4, r4, r1
        strh r0, [r4, #0]
        strh r5, [r4, #2]
        movs r1, #0
        ldrsh r0, [r4, r1]
        pop {r4, r5}
        pop {r1}
        bx r1
        .short 0x0000
_0802617C: .word 0x030015F0

@ End of region 0x08026150-0x08026180. The last body here ends exactly at
@ 0x08026180, where asm/ai_award_mid.s's prologue begins, so this anchor is
@ the precise end marker -- not a bounded-region guess. Without it
@ _08026150 has no end marker to fall back on and the promotion screen
@ refuses it. Emits no bytes; the following file supplies the address.
ai_award_leaf_end:
