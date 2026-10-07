@ menu setup helpers.
@ VMA 0x0800C454-0x0800C4D0 (124 bytes), pure Thumb with private pools.

	.thumb
	.type sub_0800C454, %function
sub_0800C454:
	push {r4, r5, r6, lr}
	sub sp, #8
	ldr r1, .L_c4a8
	movs r0, #0
	str r0, [sp]
	str r0, [sp, #4]
	movs r2, #8
	movs r3, #0
	bl 0x08007770
	adds r4, r0, #0
	movs r0, #0
	bl sub_080056DC
	adds r5, r0, #0
	movs r0, #0
	bl sub_080056C4
	movs r0, #0
	movs r1, #0x80
	lsls r1, #4
	adds r6, r5, r1
.L_c480:
	adds r3, r0, #1
	lsls r0, #6
	adds r1, r0, r6
	adds r0, r0, r5
	movs r2, #31
.L_c48a:
	strh r4, [r0]
	strh r4, [r1]
	adds r1, #2
	adds r0, #2
	subs r2, #1
	cmp r2, #0
	bge .L_c48a
	adds r0, r3, #0
	cmp r0, #31
	ble .L_c480
	add sp, #8
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.short 0
.L_c4a8:
	.word 0x08292B40

	.type sub_0800C4AC, %function
sub_0800C4AC:
	push {lr}
	sub sp, #8
	ldr r1, .L_c4c8
	movs r0, #0
	str r0, [sp]
	movs r0, #1
	str r0, [sp, #4]
	movs r2, #12
	movs r3, #0
	bl 0x08007770
	add sp, #8
	pop {r0}
	bx r0
.L_c4c8:
	.word 0x08292B40
@ 0x0800C4CC is a real 2-byte leaf (`bx lr`), not alignment filler: filler is
@ `46 c0`/`00 00`, and this halfword returns r0 unchanged before the 4-byte
@ `00 00` pad that aligns `sub_0800C4D0` (asm/menu_c4d0.s). No label and no
@ inbound `bl`, so `sub_0800C4AC`'s 32-byte body was measured against 36.
	.type _0800C4CC, %function
_0800C4CC:
	bx lr
	.short 0

@ Zero-byte end anchor: 0x0800C4CC's 4-byte span ends where asm/menu_c4d0.s
@ begins, and the owning file cannot spell the next file's label. Same idiom as
@ `menu_dispatch_end:` (see docs/matching_workflow.md).
menu_c454_end:
