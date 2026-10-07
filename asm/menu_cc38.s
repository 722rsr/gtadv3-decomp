@ GT Advance 3 - menu state dispatch helpers
@ Region: file offset 0x00CC38-0x00CCEC (VMA 0x0800CC38-0x0800CCEC).
@ Pure Thumb; byte-exact transcription of four small helpers.

.thumb
.type sub_0800CC38, %function
sub_0800CC38:
_0800CC38:
	bx lr
	.short 0

.type sub_0800CC3C, %function
sub_0800CC3C:
_0800CC3C:
	push {lr}
	cmp r0, #6
	beq.n _0800CC66
	cmp r0, #6
	bhi.n _0800CC4C
	cmp r0, #1
	beq.n _0800CC56
	b.n _0800CC7C
_0800CC4C:
	cmp r0, #7
	beq.n _0800CC5E
	cmp r0, #11
	beq.n _0800CC76
	b.n _0800CC7C
_0800CC56:
	adds r0, r3, #0
	bl 0x0800C7F4
	b.n _0800CC7C
_0800CC5E:
	adds r0, r3, #0
	bl 0x0800CAE4
	b.n _0800CC7C
_0800CC66:
	lsls r1, r1, #16
	lsrs r1, r1, #16
	lsls r2, r2, #16
	lsrs r2, r2, #16
	adds r0, r3, #0
	bl 0x0800C884
	b.n _0800CC7C
_0800CC76:
	adds r0, r3, #0
	bl 0x0800CC38
_0800CC7C:
	pop {r0}
	bx r0

.type sub_0800CC80, %function
sub_0800CC80:
_0800CC80:
	push {r4, r5, lr}
	lsls r2, r2, #16
	lsrs r2, r2, #16
	adds r4, r2, #0
	bl 0x08004B68
	adds r5, r0, #0
	movs r0, #1
	ands r0, r4
	cmp r0, #0
	beq.n _0800CC9C
	adds r0, r5, #0
	bl 0x08004BB8
_0800CC9C:
	movs r0, #2
	ands r0, r4
	cmp r0, #0
	beq.n _0800CCAA
	adds r0, r5, #0
	bl 0x08004BC8
_0800CCAA:
	pop {r4, r5}
	pop {r0}
	bx r0

.type sub_0800CCB0, %function
sub_0800CCB0:
_0800CCB0:
	push {lr}
	cmp r0, #6
	bne.n _0800CCC4
	lsls r1, r1, #16
	lsrs r1, r1, #16
	lsls r2, r2, #16
	lsrs r2, r2, #16
	adds r0, r3, #0
	bl 0x0800CC80
_0800CCC4:
	pop {r0}
	bx r0

.type sub_0800CCC8, %function
sub_0800CCC8:
_0800CCC8:
	push {r4, lr}
	adds r4, r1, #0
	bl 0x08004B68
	ldrh r0, [r0, #2]
	cmp r0, #3
	bne.n _0800CCDE
	adds r1, r4, #0
	adds r1, #84
	movs r0, #6
	b.n _0800CCE4
_0800CCDE:
	adds r1, r4, #0
	adds r1, #84
	movs r0, #1
_0800CCE4:
	strh r0, [r1, #0]
	pop {r4}
	pop {r0}
	bx r0
menu_cc38_end:
