@ GT Advance 3 - MTO sound driver: post-follow-on pure-Thumb helpers
@ Region: file offset 0x02C614-0x02C738 (VMA 0x0802C614-0x0802C738).
@ Eight small pure-Thumb functions with local pools; byte-exact.

.thumb

.type sub_0802C614, %function
sub_0802C614:
_0802C614:
	push {lr}
	lsls r0, r0, #16
	ldr r2, _0802C640
	ldr r1, _0802C644
	lsrs r0, r0, #13
	adds r0, r0, r1
	ldrh r3, [r0, #4]
	lsls r1, r3, #1
	adds r1, r1, r3
	lsls r1, r1, #2
	adds r1, r1, r2
	ldr r2, [r1, #0]
	ldr r1, [r2, #0]
	ldr r0, [r0, #0]
	cmp r1, r0
	bne.n _0802C63A
	adds r0, r2, #0
	bl 0x0802CD18
_0802C63A:
	pop {r0}
	bx r0
	.short 0x0000
_0802C640: .word 0x08061F74
_0802C644: .word 0x08061FA4

.type sub_0802C648, %function
sub_0802C648:
_0802C648:
	push {lr}
	lsls r0, r0, #16
	ldr r2, _0802C674
	ldr r1, _0802C678
	lsrs r0, r0, #13
	adds r0, r0, r1
	ldrh r3, [r0, #4]
	lsls r1, r3, #1
	adds r1, r1, r3
	lsls r1, r1, #2
	adds r1, r1, r2
	ldr r2, [r1, #0]
	ldr r1, [r2, #0]
	ldr r0, [r0, #0]
	cmp r1, r0
	bne.n _0802C66E
	adds r0, r2, #0
	bl 0x0802C488
_0802C66E:
	pop {r0}
	bx r0
	.short 0x0000
_0802C674: .word 0x08061F74
_0802C678: .word 0x08061FA4

.type sub_0802C67C, %function
sub_0802C67C:
_0802C67C:
	push {r4, r5, lr}
	ldr r0, _0802C6A0
	lsls r0, r0, #16
	lsrs r0, r0, #16
	cmp r0, #0
	beq.n _0802C69A
	ldr r5, _0802C6A4
	adds r4, r0, #0
_0802C68C:
	ldr r0, [r5, #0]
	bl 0x0802CD18
	adds r5, #12
	subs r4, #1
	cmp r4, #0
	bne.n _0802C68C
_0802C69A:
	pop {r4, r5}
	pop {r0}
	bx r0
_0802C6A0: .word 0x00000004
_0802C6A4: .word 0x08061F74

.type sub_0802C6A8, %function
sub_0802C6A8:
_0802C6A8:
	push {lr}
	bl 0x0802C488
	pop {r0}
	bx r0
	.short 0x0000

.type sub_0802C6B4, %function
sub_0802C6B4:
_0802C6B4:
	push {r4, r5, lr}
	ldr r0, _0802C6D8
	lsls r0, r0, #16
	lsrs r0, r0, #16
	cmp r0, #0
	beq.n _0802C6D2
	ldr r5, _0802C6DC
	adds r4, r0, #0
_0802C6C4:
	ldr r0, [r5, #0]
	bl 0x0802C488
	adds r5, #12
	subs r4, #1
	cmp r4, #0
	bne.n _0802C6C4
_0802C6D2:
	pop {r4, r5}
	pop {r0}
	bx r0
_0802C6D8: .word 0x00000004
_0802C6DC: .word 0x08061F74

.type sub_0802C6E0, %function
sub_0802C6E0:
_0802C6E0:
	push {lr}
	lsls r1, r1, #16
	lsrs r1, r1, #16
	bl 0x0802C4A4
	pop {r0}
	bx r0
	.short 0x0000

.type sub_0802C6F0, %function
sub_0802C6F0:
_0802C6F0:
	adds r2, r0, #0
	lsls r1, r1, #16
	lsrs r1, r1, #16
	ldr r3, [r2, #52]
	ldr r0, _0802C708
	cmp r3, r0
	bne.n _0802C706
	strh r1, [r2, #38]
	strh r1, [r2, #36]
	ldr r0, _0802C70C
	strh r0, [r2, #40]
_0802C706:
	bx lr
_0802C708: .word 0x68736D53
_0802C70C: .word 0x00000101

.type sub_0802C710, %function
sub_0802C710:
_0802C710:
	adds r2, r0, #0
	lsls r1, r1, #16
	lsrs r1, r1, #16
	ldr r3, [r2, #52]
	ldr r0, _0802C730
	cmp r3, r0
	bne.n _0802C72E
	strh r1, [r2, #38]
	strh r1, [r2, #36]
	movs r0, #2
	strh r0, [r2, #40]
	ldr r0, [r2, #4]
	ldr r1, _0802C734
	ands r0, r1
	str r0, [r2, #4]
_0802C72E:
	bx lr
_0802C730: .word 0x68736D53
_0802C734: .word 0x7FFFFFFF
sound_more_end:
