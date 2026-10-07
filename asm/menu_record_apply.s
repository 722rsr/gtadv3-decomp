@ GT Advance 3 - menu/results record apply helper
@ Region: file offset 0x00BE20-0x00BE74 (VMA 0x0800BE20-0x0800BE74).
@ Pure Thumb with private IWRAM pools; byte-exact.

.thumb
.type sub_0800BE20, %function
sub_0800BE20:
_0800BE20:
	push {r4, lr}
	sub sp, #12
	ldr r1, _0800BE60
	ldr r2, _0800BE64
	adds r0, r1, r2
	movs r3, #0
	ldrsh r4, [r0, r3]
	adds r4, #31
	ldr r2, _0800BE68
	adds r0, r1, r2
	ldr r0, [r0, #0]
	str r0, [sp, #0]
	mov r2, sp
	ldr r3, _0800BE6C
	adds r0, r1, r3
	ldrh r0, [r0, #0]
	strh r0, [r2, #4]
	add r0, sp, #8
	ldr r2, _0800BE70
	adds r1, r1, r2
	movs r2, #3
	bl 0x0800D95C
	adds r0, r4, #0
	mov r1, sp
	bl 0x0800BB0C
	add sp, #12
	pop {r4}
	pop {r0}
	bx r0
	.short 0
_0800BE60: .word 0x03001780
_0800BE64: .word 0x00000576
_0800BE68: .word 0x000010F8
_0800BE6C: .word 0x00000574
_0800BE70: .word 0x00001088

@ Byte-neutral end label for the promoted _0800BE20 span (0x0800BE20-0x0800BE74).
@ The region header declares exactly that extent and the file ends there, so
@ the splice needs an explicit terminator; a bare label emits no bytes and
@ leaves the executable region identical.
menu_record_apply_end:
