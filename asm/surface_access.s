@ GT Advance 3 - course/state resource accessors
@ Region: file offset 0x005F8C-0x006050 (VMA 0x08005F8C-0x08006050).
@ Pure Thumb; eight ARMCC leaves and their private literal pools.

.thumb
.type sub_08005F8C, %function
sub_08005F8C:
_08005F8C:
	ldr r2, [r2, #0]
	ldr r1, [r1, #0]
	subs r2, r2, r1
	str r2, [r0, #0]
	str r2, [r0, #4]
	bx lr

.type sub_08005F98, %function
sub_08005F98:
_08005F98:
	ldr r0, _08005FA0
	ldr r0, [r0, #0]
	bx lr
	.short 0
_08005FA0: .word 0x0203F760

.type sub_08005FA4, %function
sub_08005FA4:
_08005FA4:
	push {r4, r5, lr}
	adds r5, r0, #0
	adds r4, r1, #0
	bl 0x08005F98
	ldr r0, [r0, #0]
	asrs r4, r4, #11
	ldrh r0, [r0, #8]
	muls r4, r0
	asrs r5, r5, #11
	adds r4, r4, r5
	movs r0, #128
	lsls r0, r0, #18
	adds r4, r4, r0
	bl 0x08005F98
	ldr r0, [r0, #36]
	ldrb r4, [r4, #0]
	adds r0, r4, r0
	ldrb r0, [r0, #0]
	pop {r4, r5}
	pop {r1}
	bx r1
	.short 0

.type sub_08005FD4, %function
sub_08005FD4:
_08005FD4:
	push {lr}
	bl 0x08005F98
	ldr r0, [r0, #0]
	pop {r1}
	bx r1

.type sub_08005FE0, %function
sub_08005FE0:
_08005FE0:
	push {r4, lr}
	adds r4, r0, #0
	bl 0x08005F98
	lsls r1, r4, #2
	adds r1, r1, r4
	lsls r1, r1, #2
	ldr r0, [r0, #4]
	adds r0, r0, r1
	pop {r4}
	pop {r1}
	bx r1

.type sub_08005FF8, %function
sub_08005FF8:
_08005FF8:
	movs r0, #0
	bx lr

.type sub_08005FFC, %function
sub_08005FFC:
_08005FFC:
	push {r4, lr}
	lsls r4, r2, #16
	lsrs r4, r4, #16
	bl 0x08005FF8
	lsls r4, r4, #16
	asrs r4, r4, #16
	subs r0, r0, r4
	lsls r0, r0, #16
	asrs r0, r0, #16
	pop {r4}
	pop {r1}
	bx r1
	.short 0

.type sub_08006018, %function
sub_08006018:
_08006018:
	push {r4, r5, r6, lr}
	bl 0x08005F98
	ldr r4, _08006044
	ldr r1, [r0, #0]
	ldrh r5, [r1, #8]
	ldr r2, _08006048
	ldr r6, _0800604C
	ldr r1, [r0, #28]
	movs r3, #127
_0800602C:
	str r1, [r2, #0]
	str r4, [r2, #4]
	str r6, [r2, #8]
	ldr r0, [r2, #8]
	adds r4, #128
	adds r1, r1, r5
	subs r3, #1
	cmp r3, #0
	bge.n _0800602C
	pop {r4, r5, r6}
	pop {r0}
	bx r0
_08006044: .word 0x06004000
_08006048: .word 0x040000D4
_0800604C: .word 0x80000040
