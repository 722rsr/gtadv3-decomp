@ GT Advance 3 - menu/results detail renderer
@ Region: file offset 0x00CAE4-0x00CC38 (VMA 0x0800CAE4-0x0800CC38).
@ Pure Thumb; byte-exact transcription including the private template pool.

.thumb
.type sub_0800CAE4, %function
sub_0800CAE4:
_0800CAE4:
	push {r4, r5, r6, r7, lr}
	sub sp, #56
	adds r7, r0, #0
	add r5, sp, #28
	adds r1, r5, #0
	ldr r0, _0800CC08
	ldmia r0!, {r2, r3, r4}
	stmia r1!, {r2, r3, r4}
	ldmia r0!, {r2, r3, r4}
	stmia r1!, {r2, r3, r4}
	ldr r0, [r0, #0]
	str r0, [r1, #0]
	ldr r1, _0800CC0C
	movs r0, #4
	bl 0x08003940
	ldr r4, _0800CC10
	bl 0x08002044
	adds r3, r0, #0
	movs r0, #20
	movs r1, #20
	adds r2, r4, #0
	bl 0x08003F18
	movs r6, #0
_0800CB18:
	movs r0, #50
	adds r4, r6, #0
	muls r4, r0
	adds r4, #50
	adds r0, r6, #0
	bl 0x0800206C
	adds r2, r0, #0
	lsls r2, r2, #16
	lsrs r2, r2, #16
	adds r0, r4, #0
	movs r1, #30
	bl 0x08003978
	adds r0, r6, #0
	movs r1, #0
	bl 0x08001E5C
	adds r2, r0, #0
	lsls r2, r2, #16
	lsrs r2, r2, #16
	adds r0, r4, #0
	movs r1, #40
	bl 0x08003978
	adds r0, r6, #0
	movs r1, #0
	bl 0x08002178
	adds r2, r0, #0
	lsls r2, r2, #16
	lsrs r2, r2, #16
	adds r0, r4, #0
	movs r1, #50
	bl 0x08003978
	adds r6, #1
	cmp r6, #3
	ble.n _0800CB18
	movs r4, #70
	movs r6, #6
_0800CB6A:
	ldmia r5!, {r2}
	movs r0, #100
	adds r1, r4, #0
	bl 0x08003838
	adds r4, #10
	subs r6, #1
	cmp r6, #0
	bge.n _0800CB6A
	movs r1, #0
	ldrsh r0, [r7, r1]
	lsls r1, r0, #2
	adds r1, r1, r0
	lsls r1, r1, #1
	adds r1, #70
	ldr r2, _0800CC14
	movs r0, #90
	bl 0x08003838
	ldr r1, _0800CC18
	ldr r4, _0800CC1C
	ldr r2, _0800CC20
	adds r0, r4, r2
	movs r3, #0
	ldrsh r0, [r0, r3]
	lsls r0, r0, #2
	adds r0, r0, r1
	ldr r2, [r0, #0]
	movs r0, #144
	movs r1, #70
	bl 0x08003838
	ldr r1, _0800CC24
	adds r0, r4, r1
	movs r3, #0
	ldrsh r2, [r0, r3]
	movs r0, #160
	movs r1, #80
	bl 0x08003954
	ldr r1, _0800CC28
	ldr r2, _0800CC2C
	adds r0, r4, r2
	movs r3, #0
	ldrsh r0, [r0, r3]
	lsls r0, r0, #2
	adds r0, r0, r1
	ldr r2, [r0, #0]
	movs r0, #184
	movs r1, #70
	bl 0x08003838
	ldr r1, _0800CC30
	ldr r0, _0800CC34
	adds r4, r4, r0
	movs r2, #0
	ldrsh r0, [r4, r2]
	lsls r0, r0, #2
	adds r0, r0, r1
	ldr r2, [r0, #0]
	movs r0, #184
	movs r1, #80
	bl 0x08003838
	movs r2, #29
	ldrsb r2, [r7, r2]
	movs r0, #160
	movs r1, #130
	bl 0x08003954
	ldrh r1, [r7, #0]
	movs r0, #0
	bl 0x08002158
	add sp, #56
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.short 0
_0800CC08: .word 0x0805F850
_0800CC0C: .word 0x0805F86C
_0800CC10: .word 0x0805F87C
_0800CC14: .word 0x0805F888
_0800CC18: .word 0x080CB2C0
_0800CC1C: .word 0x03001780
_0800CC20: .word 0x00000576
_0800CC24: .word 0x00000574
_0800CC28: .word 0x080CB3B8
_0800CC2C: .word 0x0000113C
_0800CC30: .word 0x080CB3C4
_0800CC34: .word 0x0000113E
