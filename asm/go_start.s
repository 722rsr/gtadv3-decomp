@ GT Advance 3 - race start helper (GO! / opponent-car spawner)
@ Region: file offset 0xB89C-0xB990 (VMA 0x0800B89C-0x0800B990).
@
@ Disassembled via objdump from baserom.gba; byte-exact.
@ Companion: asm/ai_collect.s, asm/carphys_racer.s
@
@ Called every frame by race-FSM case 2 (_0800AC90, phase 2 = GO!/start).
@ u16[wa+0x576] (0x03001CF6) is a start-stage counter advanced at the
@ end of each pass:
@   stage 1: grant cars {86 after 10, 56 after 30}
@   stage 2: grant cars {55 after 10, 31 after 30}   (one-make cup pairs,
@   stage 3: grant cars {14 after 10, 98 after 30}    see ai_opponents §6)
@ Then, for stages 1 and 2 only: when word[wa+0x10F8] > 19 and the
@ corresponding trailing-object slot is free (sub_08026004), spawn it
@ (sub_08025FF0), append command {2 or 3} to the car-3 playback script
@ wa+0x104C[] at index s16[wa+0x104A] (bumped), and fire ring event 30.
@ The script array is consumed by the ev-7 car-3 branch of _08021374.

.thumb

sub_0800B89C:
_0800B89C:
	push {r4, r5, lr}
	ldr r5, _0800B97C @ =0x03001780
	ldr r0, _0800B980 @ =0x00000576
	adds r4, r5, r0       @ &u16[wa+0x0576] start-stage counter
	ldrh r1, [r4]
	cmp r1, #1
	bne _0800B8BA
	movs r0, #86          @ 0x56
	movs r1, #10
	bl sub_0800B82C       @ grant car 86 after 10
	movs r0, #56          @ 0x38
	movs r1, #30
	bl sub_0800B82C       @ grant car 56 after 30
_0800B8BA:
	ldrh r2, [r4]
	cmp r2, #2
	bne _0800B8D0
	movs r0, #55          @ 0x37
	movs r1, #10
	bl sub_0800B82C       @ grant car 55 after 10
	movs r0, #31          @ 0x1f
	movs r1, #30
	bl sub_0800B82C       @ grant car 31 after 30
_0800B8D0:
	ldrh r0, [r4]
	cmp r0, #3
	bne _0800B8E6
	movs r0, #14
	movs r1, #10
	bl sub_0800B82C       @ grant car 14 after 10
	movs r0, #98          @ 0x62
	movs r1, #30
	bl sub_0800B82C       @ grant car 98 after 30
_0800B8E6:
	ldrh r4, [r4]
	cmp r4, #1
	bne _0800B92A
	ldr r1, _0800B984 @ =0x000010F8
	adds r0, r5, r1
	ldr r0, [r0]          @ word[wa+0x10F8]
	cmp r0, #19
	bls _0800B92A
	movs r0, #1
	bl sub_08026004       @ trailing slot 1 free?
	lsls r0, r0, #16
	cmp r0, #0
	bne _0800B92A
	movs r0, #1
	bl sub_08025FF0       @ spawn trailing object 1
	ldr r2, _0800B988 @ =0x0000104A
	adds r4, r5, r2
	movs r1, #0
	ldrsh r0, [r4, r1]    @ script index
	lsls r0, r0, #1
	adds r2, #2           @ -> 0x104C
	adds r1, r5, r2
	adds r0, r0, r1
	movs r1, #2
	strh r1, [r0]         @ script[index] = 2
	movs r0, #30          @ 0x1e
	movs r1, #0
	bl _08023FF8          @ ring event 30
	ldrh r0, [r4]
	adds r0, #1
	strh r0, [r4]         @ script index++
_0800B92A:
	ldr r5, _0800B97C @ =0x03001780
	ldr r1, _0800B980 @ =0x00000576
	adds r0, r5, r1
	ldrh r0, [r0]         @ re-read stage counter
	cmp r0, #2
	bne _0800B974
	ldr r2, _0800B984 @ =0x000010F8
	adds r0, r5, r2
	ldr r0, [r0]
	cmp r0, #19
	bls _0800B974
	movs r0, #2
	bl sub_08026004       @ trailing slot 2 free?
	lsls r0, r0, #16
	cmp r0, #0
	bne _0800B974
	movs r0, #2
	bl sub_08025FF0       @ spawn trailing object 2
	ldr r0, _0800B988 @ =0x0000104A
	adds r4, r5, r0
	movs r1, #0
	ldrsh r0, [r4, r1]
	lsls r0, r0, #1
	ldr r2, _0800B98C @ =0x0000104C
	adds r1, r5, r2
	adds r0, r0, r1
	movs r1, #3
	strh r1, [r0]         @ script[index] = 3
	movs r0, #30          @ 0x1e
	movs r1, #0
	bl _08023FF8          @ ring event 30
	ldrh r0, [r4]
	adds r0, #1
	strh r0, [r4]         @ script index++
_0800B974:
	pop {r4, r5}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_0800B97C: .4byte 0x03001780
_0800B980: .4byte 0x00000576
_0800B984: .4byte 0x000010F8
_0800B988: .4byte 0x0000104A
_0800B98C: .4byte 0x0000104C
