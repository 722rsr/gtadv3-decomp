@ GT Advance 3 - menu record helper pocket
@ Region: file offset 0x00C2C4-0x00C340 (VMA 0x0800C2C4-0x0800C340).
@ Four pure-Thumb helpers with private offset pools; byte-exact.

.thumb
.type sub_0800C2C4, %function
sub_0800C2C4:
_0800C2C4:
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

.type sub_0800C2E4, %function
sub_0800C2E4:
_0800C2E4:
	push {lr}
	bl 0x080055F4
	bl 0x08002B50
	bl 0x08002BB4
	bl 0x08002B44
	pop {r0}
	bx r0
	.short 0

.type sub_0800C2FC, %function
sub_0800C2FC:
_0800C2FC:
	push {lr}
	bl 0x08005604
	bl 0x08002C98
	pop {r0}
	bx r0
	.short 0

.type sub_0800C30C, %function
sub_0800C30C:
_0800C30C:
	push {lr}
	cmp r1, #1
	blt.n _0800C336
	cmp r1, #2
	bgt.n _0800C328
	ldr r1, _0800C324
	adds r0, r0, r1
	ldr r0, [r0, #0]
	bl 0x08004EF0
	b.n _0800C336
	.short 0
_0800C324: .word 0x00001DE4
_0800C328:
	cmp r1, #4
	bgt.n _0800C336
	ldr r1, _0800C33C
	adds r0, r0, r1
	ldr r0, [r0, #0]
	bl 0x08004EF0
_0800C336:
	pop {r0}
	bx r0
	.short 0
_0800C33C: .word 0x00001DE8

menu_c2c4_end:
