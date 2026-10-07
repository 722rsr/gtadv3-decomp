@ GT Advance 3 - race/menu command dispatcher
@ Region: file offset 0x00B190-0x00B4A8 (VMA 0x0800B190-0x0800B4A8).
@ Exact Thumb function with an embedded eight-way command jump table and
@ private literal pools.  The packet built here is consumed by the shared
@ UI/race dispatcher at 0x080188B0; the callers are the race FSM and scene
@ handlers.  Pools are deliberately kept at their original ROM offsets.

.thumb

.type sub_0800B190, %function
sub_0800B190:
	push {r4, r5, r6, lr}
	sub sp, #60
	movs r0, #0
	str r0, [sp, #56]
	add r0, sp, #56
	ldr r2, _0800B1BC
	mov r1, sp
	bl 0x0802D974
	ldr r0, _0800B1C0
	ldr r1, _0800B1C4
	adds r0, r0, r1
	movs r2, #0
	ldrsh r0, [r0, r2]
	cmp r0, #7
	bls _0800B1B2
	b _0800B2F8
_0800B1B2:
	lsls r0, r0, #2
	ldr r1, _0800B1C8
	adds r0, r0, r1
	ldr r0, [r0]
	mov pc, r0

	.align 2, 0
_0800B1BC:
	.word 0x0500000E
_0800B1C0:
	.word 0x03001780
_0800B1C4:
	.word 0x00000FBC
_0800B1C8:
	.word _0800B1CC
_0800B1CC:
	.word _0800B1F4
	.word _0800B254
	.word _0800B25C
	.word _0800B2AC
	.word _0800B2F8
	.word _0800B23C
	.word _0800B1EC
	.word _0800B264

_0800B1EC:
	mov r1, sp
	movs r0, #0
	strh r0, [r1, #0]
	b _0800B2F8

_0800B1F4:
	ldr r2, _0800B20C
	ldr r4, _0800B210
	adds r3, r2, r4
	movs r5, #0
	ldrsh r0, [r3, r5]
	cmp r0, #2
	bgt _0800B214
	mov r1, sp
	movs r0, #2
	strh r0, [r1, #0]
	b _0800B2F8

	.align 2, 0
_0800B20C:
	.word 0x03001780
_0800B210:
	.word 0x0000103A

_0800B214:
	mov r1, sp
	movs r0, #1
	strh r0, [r1, #0]
	ldr r1, _0800B234
	adds r0, r2, r1
	movs r4, #0
	ldrsh r0, [r0, r4]
	ldr r5, _0800B238
	adds r1, r2, r5
	movs r2, #0
	ldrsh r1, [r1, r2]
	movs r4, #0
	ldrsh r2, [r3, r4]
	bl 0x08025790
	b _0800B29A

	.align 2, 0
_0800B234:
	.word 0x00000FF2
_0800B238:
	.word 0x00000FF6

_0800B23C:
	mov r1, sp
	movs r0, #4
	strh r0, [r1, #0]
	ldr r0, _0800B24C
	ldr r5, _0800B250
	adds r0, r0, r5
	b _0800B292

	.align 2, 0
_0800B24C:
	.word 0x03001780
_0800B250:
	.word 0x00000576

_0800B254:
	mov r1, sp
	movs r0, #3
	strh r0, [r1, #0]
	b _0800B2F8

_0800B25C:
	mov r1, sp
	movs r0, #5
	strh r0, [r1, #0]
	b _0800B2F8

_0800B264:
	ldr r2, _0800B280
	ldr r3, _0800B284
	adds r0, r2, r3
	movs r4, #0
	ldrsh r0, [r0, r4]
	cmp r0, #1
	beq _0800B288
	cmp r0, #2
	bne _0800B2A4
	mov r1, sp
	movs r0, #6
	strh r0, [r1, #0]
	b _0800B2F8

	.align 2, 0
_0800B280:
	.word 0x03001780
_0800B284:
	.word 0x00001078

_0800B288:
	mov r1, sp
	movs r0, #7
	strh r0, [r1, #0]
	ldr r5, _0800B2A0
	adds r0, r2, r5
_0800B292:
	movs r1, #0
	ldrsh r0, [r0, r1]
	bl 0x0802591C
_0800B29A:
	mov r1, sp
	strh r0, [r1, #6]
	b _0800B2F8

	.align 2, 0
_0800B2A0:
	.word 0x00000576

_0800B2A4:
	mov r1, sp
	movs r0, #7
	strh r0, [r1, #0]
	b _0800B2F8

_0800B2AC:
	mov r1, sp
	movs r0, #8
	strh r0, [r1, #0]
	ldr r2, _0800B344
	ldr r3, _0800B348
	adds r0, r2, r3
	ldrh r0, [r0]
	strh r0, [r1, #40]
	mov r4, sp
	ldr r5, _0800B34C
	adds r3, r2, r5
	movs r0, #0
	ldrsh r1, [r3, r0]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2
	adds r0, r0, r2
	adds r0, #48
	ldrb r0, [r0]
	lsls r0, r0, #24
	asrs r0, r0, #24
	strh r0, [r4, #42]
	movs r4, #0
	ldrsh r1, [r3, r4]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2
	adds r0, r0, r2
	add r1, sp, #44
	adds r0, #48
	ldmia r0!, {r3, r4, r5}
	stmia r1!, {r3, r4, r5}
	ldr r5, _0800B350
	adds r2, r2, r5
	ldrh r1, [r2]
	mov r0, sp
	adds r0, #45
	strb r1, [r0]

_0800B2F8:
	mov r1, sp
	ldr r4, _0800B344
	ldr r2, _0800B354
	adds r0, r4, r2
	ldrh r0, [r0]
	strb r0, [r1, #21]
	ldr r3, _0800B358
	adds r0, r4, r3
	ldrh r0, [r0]
	strh r0, [r1, #2]
	ldr r0, _0800B35C
	adds r5, r4, r0
	ldrh r0, [r5]
	strh r0, [r1, #4]
	ldr r1, _0800B360
	adds r0, r4, r1
	movs r2, #0
	ldrsh r0, [r0, r2]
	cmp r0, #0
	bne _0800B364
	movs r3, #0
	ldrsh r0, [r5, r3]
	adds r1, #126
	adds r4, r4, r1
	movs r2, #0
	ldrsh r1, [r4, r2]
	bl 0x080256F4
	mov r1, sp
	strh r0, [r1, #8]
	movs r3, #0
	ldrsh r0, [r5, r3]
	movs r5, #0
	ldrsh r1, [r4, r5]
	bl 0x08025710
	mov r1, sp
	b _0800B374

	.align 2, 0
_0800B344:
	.word 0x03001780
_0800B348:
	.word 0x00000FC4
_0800B34C:
	.word 0x00000574
_0800B350:
	.word 0x00000FC6
_0800B354:
	.word 0x00000FEE
_0800B358:
	.word 0x00000576
_0800B35C:
	.word 0x00000FF6
_0800B360:
	.word 0x00000FBC

_0800B364:
	mov r1, sp
	ldr r2, _0800B3A0
	adds r0, r4, r2
	ldrh r0, [r0]
	strh r0, [r1, #8]
	ldr r3, _0800B3A4
	adds r0, r4, r3
	ldrh r0, [r0]
_0800B374:
	strh r0, [r1, #10]
	mov r0, sp
	ldrh r0, [r0]
	cmp r0, #2
	bne _0800B3B8
	ldr r2, _0800B3A8
	ldr r4, _0800B3AC
	adds r0, r2, r4
	movs r5, #0
	ldrsh r0, [r0, r5]
	ldr r3, _0800B3B0
	adds r1, r2, r3
	movs r4, #0
	ldrsh r1, [r1, r4]
	ldr r5, _0800B3B4
	adds r2, r2, r5
	movs r3, #0
	ldrsh r2, [r2, r3]
	bl 0x08025750
	b _0800B3D0

	.align 2, 0
_0800B3A0:
	.word 0x00000FE6
_0800B3A4:
	.word 0x00000FE4
_0800B3A8:
	.word 0x03001780
_0800B3AC:
	.word 0x00000FF2
_0800B3B0:
	.word 0x00000FF6
_0800B3B4:
	.word 0x0000103A

_0800B3B8:
	ldr r2, _0800B3E8
	ldr r4, _0800B3EC
	adds r0, r2, r4
	movs r5, #0
	ldrsh r1, [r0, r5]
	lsls r0, r1, #3
	adds r0, r0, r1
	lsls r0, r0, #3
	ldr r1, _0800B3F0
	adds r2, r2, r1
	adds r0, r0, r2
	ldr r0, [r0]
_0800B3D0:
	str r0, [sp, #12]
	mov r0, sp
	ldrh r1, [r0]
	subs r0, r1, #6
	lsls r0, r0, #16
	lsrs r0, r0, #16
	cmp r0, #1
	bhi _0800B3F8
	mov r1, sp
	ldr r3, _0800B3E8
	ldr r4, _0800B3F4
	b _0800B446

	.align 2, 0
_0800B3E8:
	.word 0x03001780
_0800B3EC:
	.word 0x00000576
_0800B3F0:
	.word 0x000005E4
_0800B3F4:
	.word 0x00000FC2

_0800B3F8:
	lsls r0, r1, #16
	asrs r0, r0, #16
	cmp r0, #2
	bne _0800B440
	ldr r4, _0800B434
	ldr r0, _0800B438
	adds r5, r4, r0
	movs r1, #0
	ldrsh r0, [r5, r1]
	ldr r2, _0800B43C
	adds r4, r4, r2
	movs r3, #0
	ldrsh r1, [r4, r3]
	bl 0x080257D4
	mov r1, sp
	movs r6, #0
	strh r0, [r1, #24]
	movs r1, #0
	ldrsh r0, [r5, r1]
	movs r2, #0
	ldrsh r1, [r4, r2]
	bl 0x080257F0
	mov r1, sp
	strb r0, [r1, #29]
	mov r0, sp
	strh r6, [r0, #26]
	b _0800B478

	.align 2, 0
_0800B434:
	.word 0x03001780
_0800B438:
	.word 0x00000FF6
_0800B43C:
	.word 0x0000103A

_0800B440:
	mov r1, sp
	ldr r3, _0800B49C
	ldr r4, _0800B4A0
_0800B446:
	adds r2, r3, r4
	ldrh r0, [r2]
	strh r0, [r1, #24]
	mov r4, sp
	movs r5, #0
	ldrsh r1, [r2, r5]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2
	adds r0, r0, r3
	adds r0, #48
	ldrb r0, [r0]
	lsls r0, r0, #24
	asrs r0, r0, #24
	strh r0, [r4, #26]
	movs r0, #0
	ldrsh r1, [r2, r0]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2
	adds r0, r0, r3
	add r1, sp, #28
	adds r0, #48
	ldmia r0!, {r2, r3, r4}
	stmia r1!, {r2, r3, r4}
_0800B478:
	mov r2, sp
	ldr r1, _0800B49C
	ldr r5, _0800B4A4
	adds r0, r1, r5
	ldrh r0, [r0]
	strb r0, [r2, #22]
	movs r0, #188
	lsls r0, r0, #3
	adds r1, r1, r0
	ldrh r0, [r1]
	strb r0, [r2, #20]
	mov r0, sp
	bl 0x080188B0
	add sp, #60
	pop {r4, r5, r6}
	pop {r0}
	bx r0

	.align 2, 0
_0800B49C:
	.word 0x03001780
_0800B4A0:
	.word 0x00000574
_0800B4A4:
	.word 0x00000FF2
