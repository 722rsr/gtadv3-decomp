@ GT Advance 3 - EEPROM save-slot API
@ Region: file offset 0x0059F0-0x005A58 (VMA 0x080059F0-0x08005A58).
@ Pure Thumb; byte-exact transcription.

.thumb
.type sub_080059F0, %function
sub_080059F0:
_080059F0:
	push {r4, r5, r6, lr}
	sub sp, #4
	adds r4, r1, #0
	lsls r0, r0, #24
	lsrs r0, r0, #21
	ldr r1, _08005A4C
	adds r5, r0, r1
	ldr r1, [r5, #4]
	adds r0, r4, #0
	bl 0x08005860
	str r0, [sp, #0]
	movs r6, #128
	lsls r6, r6, #18
	mov r0, sp
	adds r1, r6, #0
	movs r2, #2
	bl 0x0802D974
	ldr r1, _08005A50
	ldr r2, [r5, #4]
	lsrs r0, r2, #31
	adds r2, r2, r0
	lsls r2, r2, #10
	lsrs r2, r2, #11
	adds r0, r4, #0
	bl 0x0802D974
	ldr r0, _08005A54
	ldr r0, [r0, #0]
	ldr r0, [r0, #4]
	cmp r0, #2
	bgt.n _08005A42
	cmp r0, #1
	blt.n _08005A42
	ldr r1, [r5, #0]
	ldr r2, [r5, #4]
	adds r2, #4
	adds r0, r6, #0
	bl 0x080058D0
_08005A42:
	movs r0, #1
	add sp, #4
	pop {r4, r5, r6}
	pop {r1}
	bx r1
_08005A4C: .word 0x0300032C
_08005A50: .word 0x02000004
_08005A54: .word 0x030003AC
save_slot_api_end:
