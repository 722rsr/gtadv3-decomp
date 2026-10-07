@ GT Advance 3 - menu scene helper pocket
@ Region: file offset 0x00C0F0-0x00C168 (VMA 0x0800C0F0-0x0800C168).
@ Four pure-Thumb helpers with one private literal pool; byte-exact.

.thumb
.type sub_0800C0F0, %function
sub_0800C0F0:
_0800C0F0:
	push {r4, lr}
	adds r4, r0, #0
	adds r0, #8
	movs r1, #1
	bl 0x080050E8
	movs r0, #1
	bl 0x080056B8
	adds r4, #92
	ldrh r0, [r4, #0]
	bl 0x08004E1C
	pop {r4}
	pop {r0}
	bx r0

.type sub_0800C110, %function
sub_0800C110:
_0800C110:
	push {lr}
	bl 0x080055F4
	bl 0x08002B50
	bl 0x08002BB4
	bl 0x08002B44
	pop {r0}
	bx r0
	.short 0

.type sub_0800C128, %function
sub_0800C128:
_0800C128:
	push {lr}
	bl 0x08005604
	bl 0x08002C98
	pop {r0}
	bx r0
	.short 0

.type sub_0800C138, %function
sub_0800C138:
_0800C138:
	push {lr}
	cmp r1, #1
	blt.n _0800C15E
	cmp r1, #2
	bgt.n _0800C150
	movs r1, #239
	lsls r1, r1, #5
	adds r0, r0, r1
	ldr r0, [r0, #0]
	bl 0x08004EF0
	b.n _0800C15E
_0800C150:
	cmp r1, #4
	bgt.n _0800C15E
	ldr r1, _0800C164
	adds r0, r0, r1
	ldr r0, [r0, #0]
	bl 0x08004EF0
_0800C15E:
	pop {r0}
	bx r0
	.short 0
_0800C164: .word 0x00001DE4
menu_c0f0_end:
