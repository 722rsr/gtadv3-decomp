@ GT Advance 3 - MTO sound driver: voice initializer and raw wrappers
@ Region: file offset 0x02C780-0x02C8C4 (VMA 0x0802C780-0x0802C8C4).
@ Pure Thumb; literal pools and the BIOS wrapper are kept at original offsets.

.thumb
.type sub_0802C780, %function
sub_0802C780:
_0802C780:
	push {r4, r5, r6, lr}
	sub sp, #4
	adds r5, r0, #0
	ldr r1, _0802C848
	movs r0, #143
	strh r0, [r1, #0]
	ldr r3, _0802C84C
	movs r2, #0
	strh r2, [r3, #0]
	ldr r0, _0802C850
	movs r1, #8
	strb r1, [r0, #0]
	adds r0, #6
	strb r1, [r0, #0]
	adds r0, #16
	strb r1, [r0, #0]
	subs r0, #20
	movs r1, #128
	strb r1, [r0, #0]
	adds r0, #8
	strb r1, [r0, #0]
	adds r0, #16
	strb r1, [r0, #0]
	subs r0, #13
	strb r2, [r0, #0]
	movs r0, #119
	strb r0, [r3, #0]
	ldr r0, _0802C854
	ldr r4, [r0, #0]
	ldr r6, [r4, #0]
	ldr r0, _0802C858
	cmp r6, r0
	bne.n _0802C840
	adds r0, r6, #1
	str r0, [r4, #0]
	ldr r1, _0802C85C
	ldr r0, _0802C860
	str r0, [r1, #32]
	ldr r0, _0802C864
	str r0, [r1, #68]
	ldr r0, _0802C868
	str r0, [r1, #76]
	ldr r0, _0802C86C
	str r0, [r1, #112]
	ldr r0, _0802C870
	str r0, [r1, #116]
	ldr r0, _0802C874
	str r0, [r1, #120]
	ldr r0, _0802C878
	str r0, [r1, #124]
	adds r2, r1, #0
	adds r2, #128
	ldr r0, _0802C87C
	str r0, [r2, #0]
	adds r1, #132
	ldr r0, _0802C880
	str r0, [r1, #0]
	str r5, [r4, #28]
	ldr r0, _0802C884
	str r0, [r4, #40]
	ldr r0, _0802C888
	str r0, [r4, #44]
	ldr r0, _0802C88C
	str r0, [r4, #48]
	ldr r0, _0802C890
	movs r1, #0
	strb r0, [r4, #12]
	str r1, [sp, #0]
	ldr r2, _0802C894
	mov r0, sp
	adds r1, r5, #0
	bl 0x0802D974
	movs r0, #1
	strb r0, [r5, #1]
	movs r0, #17
	strb r0, [r5, #28]
	adds r1, r5, #0
	adds r1, #65
	movs r0, #2
	strb r0, [r1, #0]
	adds r1, #27
	movs r0, #34
	strb r0, [r1, #0]
	adds r1, #37
	movs r0, #3
	strb r0, [r1, #0]
	adds r1, #27
	movs r0, #68
	strb r0, [r1, #0]
	adds r1, #36
	movs r0, #4
	strb r0, [r1, #1]
	movs r0, #136
	strb r0, [r1, #28]
	str r6, [r4, #0]
_0802C840:
	add sp, #4
	pop {r4, r5, r6}
	pop {r0}
	bx r0
_0802C848: .word 0x04000084
_0802C84C: .word 0x04000080
_0802C850: .word 0x04000063
_0802C854: .word 0x03007FF0
_0802C858: .word 0x68736D53
_0802C85C: .word 0x0203EBB0
_0802C860: .word 0x0802D6F5
_0802C864: .word 0x0802C3F9
_0802C868: .word 0x0802C40D
_0802C86C: .word 0x0802D84D
_0802C870: .word 0x0802C391
_0802C874: .word 0x0802C991
_0802C878: .word 0x0802C11D
_0802C87C: .word 0x0802CD59
_0802C880: .word 0x0802CE21
_0802C884: .word 0x0802D035
_0802C888: .word 0x0802CF7D
_0802C88C: .word 0x0802CED5
_0802C890: .word 0x00000000
_0802C894: .word 0x05000040

.type sub_0802C898, %function
sub_0802C898:
_0802C898:
	svc #42
	bx lr

.type sub_0802C89C, %function
sub_0802C89C:
_0802C89C:
	push {lr}
	ldr r1, _0802C8AC
	ldr r1, [r1, #0]
	bl 0x0802DDCC
	pop {r0}
	bx r0
	.short 0x0000
_0802C8AC: .word 0x0203EC38

.type sub_0802C8B0, %function
sub_0802C8B0:
_0802C8B0:
	push {lr}
	ldr r1, _0802C8C0
	ldr r1, [r1, #0]
	bl 0x0802DDCC
	pop {r0}
	bx r0
	.short 0x0000
_0802C8C0: .word 0x0203EC3C

@ Region end. The last body (_0802C8B0) runs to exactly here, and this file is
@ self-terminated with no.include, so this anchor is its precise end marker.
sound_init_end:
