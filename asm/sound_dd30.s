@ sound/save continuation: sector compare helper.
@ VMA 0x0802DD30-0x0802DD86 (86 bytes), pure Thumb with private pools.

	.thumb
	.type sub_0802DD30, %function
sub_0802DD30:
	push {r4, r5, lr}
	sub sp, #8
	adds r4, r1, #0
	lsls r0, #16
	lsrs r1, r0, #16
	movs r5, #0
	ldr r0, .L_dd4c
	ldr r0, [r0]
	ldrh r0, [r0, #4]
	cmp r1, r0
	bcc .L_dd54
	ldr r0, .L_dd50
	b .L_dd7e
	.short 0
.L_dd4c:
	.word 0x0203FD54
.L_dd50:
	.word 0x000080FF
.L_dd54:
	adds r0, r1, #0
	mov r1, sp
	bl sub_0802DBA4
	mov r2, sp
	movs r3, #0
	b .L_dd6c
.L_dd62:
	adds r0, r3, #1
	lsls r0, #24
	lsrs r3, r0, #24
	cmp r3, #3
	bhi .L_dd7c
.L_dd6c:
	ldrh r1, [r4]
	ldrh r0, [r2]
	adds r2, #2
	adds r4, #2
	cmp r1, r0
	beq .L_dd62
	movs r5, #128
	lsls r5, #8
.L_dd7c:
	adds r0, r5, #0
.L_dd7e:
	add sp, #8
	pop {r4, r5}
	pop {r1}
	bx r1
sound_dd30_end:
