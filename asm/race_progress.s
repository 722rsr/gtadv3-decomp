@ GT Advance 3 - race progress / award-grid helpers.
@ Region: file offset 0x00AD84-0x00B0BC (VMA 0x0800AD84-0x0800B0BC).
@
@ This is the exact cluster between the converted race FSM and the existing
@ 0x0800B0BC marker.  The three routines update the 4x11 progress/award
@ grids, commit the current progress cell, and run the per-frame progress
@ tail.  Literal pools are kept at their original offsets.

.thumb

@ ----------------------------------------------------------------------------
@ _0800AD84 - commit the selected grid cell and award progress.
_0800AD84:
	push {r4, r5, r6, r7, lr}
	mov r7, r9
	mov r6, r8
	push {r6, r7}
	ldr r2, _0800AD84_lit_0
	ldr r0, _0800AD84_lit_1
	adds r3, r2, r0
	movs r4, #0
	ldrsh r1, [r3, r4]
	lsls r1, r1, #1
	ldr r6, _0800AD84_lit_2
	adds r5, r2, r6
	movs r7, #0
	ldrsh r0, [r5, r7]
	lsls r0, r0, #3
	adds r1, r1, r0
	ldr r0, _0800AD84_lit_3
	adds r0, r0, r2
	mov r9, r0
	add r1, r9
	adds r6, #72
	adds r4, r2, r6
	ldrh r0, [r4, #0]
	strh r0, [r1, #0]
	movs r7, #0
	ldrsh r1, [r3, r7]
	lsls r1, r1, #1
	movs r6, #0
	ldrsh r0, [r5, r6]
	lsls r0, r0, #3
	adds r1, r1, r0
	ldr r7, _0800AD84_lit_4
	adds r7, r7, r2
	mov r8, r7
	add r1, r8
	ldrh r0, [r4, #0]
	strh r0, [r1, #0]
	ldrh r0, [r3, #0]
	ldr r1, _0800AD84_lit_5
	adds r6, r2, r1
	strh r0, [r6, #0]
	ldrh r1, [r5, #0]
	ldr r3, _0800AD84_lit_6
	adds r0, r2, r3
	strh r1, [r0, #0]
	ldr r0, _0800AD84_lit_7
	adds r7, r2, r0
	ldrb r0, [r7, #0]
	subs r0, #1
	lsls r0, r0, #24
	lsrs r0, r0, #24
	cmp r0, #2
	bhi _0800AD84_done
	movs r1, #0
	ldrsh r0, [r5, r1]
	movs r2, #0
	ldrsh r1, [r6, r2]
	movs r3, #0
	ldrsh r2, [r4, r3]
	bl sub_08025CF4
	lsls r0, r0, #24
	lsrs r0, r0, #24
	movs r3, #0
	ldrsb r3, [r7, r3]
	cmp r0, r3
	bge _0800AD84_store
	movs r7, #0
	ldrsh r0, [r5, r7]
	movs r2, #0
	ldrsh r1, [r6, r2]
	movs r7, #0
	ldrsh r2, [r4, r7]
	bl 0x08025C84
	movs r0, #1
	bl sub_0800279C
_0800AD84_store:
	movs r0, #0
	ldrsh r1, [r6, r0]
	lsls r1, r1, #1
	movs r2, #0
	ldrsh r0, [r5, r2]
	lsls r0, r0, #3
	adds r1, r1, r0
	add r1, r8
	ldrh r0, [r4, #0]
	strh r0, [r1, #0]
	ldrh r2, [r4, #0]
	adds r2, #1
	strh r2, [r4, #0]
	movs r3, #0
	ldrsh r0, [r6, r3]
	lsls r0, r0, #1
	movs r4, #0
	ldrsh r1, [r5, r4]
	lsls r1, r1, #3
	adds r0, r0, r1
	add r0, r9
	strh r2, [r0, #0]
_0800AD84_done:
	pop {r3, r4}
	mov r8, r3
	mov r9, r4
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.align 2, 0
_0800AD84_lit_0:
	.word 0x03001780
_0800AD84_lit_1:
	.word 0x00000FF6
_0800AD84_lit_2:
	.word 0x00000FF2
_0800AD84_lit_3:
	.word 0x00000FFA
_0800AD84_lit_4:
	.word 0x0000100A
_0800AD84_lit_5:
	.word 0x00000FF8
_0800AD84_lit_6:
	.word 0x00000FF4
_0800AD84_lit_7:
	.word 0x000010E5

@ ----------------------------------------------------------------------------
@ _0800AE78 - advance the committed progress/award state.
_0800AE78:
	push {r4, r5, r6, r7, lr}
	ldr r2, _0800AE78_lit_0
	ldr r1, _0800AE78_lit_1
	adds r0, r2, r1
	movs r3, #0
	ldrsh r1, [r0, r3]
	lsls r1, r1, #1
	ldr r5, _0800AE78_lit_2
	adds r4, r2, r5
	movs r7, #0
	ldrsh r0, [r4, r7]
	lsls r0, r0, #3
	adds r1, r1, r0
	ldr r3, _0800AE78_lit_3
	adds r0, r2, r3
	adds r0, r1, r0
	movs r5, #0
	ldrsh r6, [r0, r5]
	ldr r7, _0800AE78_lit_4
	adds r0, r2, r7
	adds r0, r1, r0
	movs r5, #0
	ldrsh r3, [r0, r5]
	mov ip, r3
	adds r5, r2, #0
	cmp r6, #11
	bne _0800AE78_tail
	subs r7, #16
	adds r0, r5, r7
	adds r1, r1, r0
	movs r0, #0
	ldrsh r3, [r1, r0]
	cmp r3, #0
	bne _0800AE78_nonzero
	movs r0, #1
	strh r0, [r1, #0]
	ldr r1, _0800AE78_lit_5
	adds r2, r5, r1
	ldrh r0, [r2, #0]
	adds r0, #1
	strh r0, [r2, #0]
	lsls r0, r0, #16
	asrs r0, r0, #16
	cmp r0, #3
	ble _0800AE78_tail
	subs r7, #42
	adds r0, r5, r7
	movs r7, #0
	ldrsh r1, [r0, r7]
	cmp r1, #1
	bne _0800AE78_commit
	movs r7, #0
	ldrsh r0, [r4, r7]
	cmp r0, #0
	bne _0800AE78_set3
	strh r1, [r4, #0]
	strh r3, [r2, #0]
	b _0800AE78_tail
	.align 2, 0
_0800AE78_lit_0:
	.word 0x03001780
_0800AE78_lit_1:
	.word 0x00000FF8
_0800AE78_lit_2:
	.word 0x00000FF2
_0800AE78_lit_3:
	.word 0x00000FFA
_0800AE78_lit_4:
	.word 0x0000102A
_0800AE78_lit_5:
	.word 0x00000FF6
_0800AE78_set3:
	strh r1, [r4, #0]
	b _0800AE78_set3_done
_0800AE78_commit:
	strh r3, [r4, #0]
_0800AE78_set3_done:
	movs r0, #3
	strh r0, [r2, #0]
	b _0800AE78_tail
_0800AE78_nonzero:
	movs r2, #0
	movs r0, #1
	strh r0, [r1, #0]
	ldr r0, _0800AE78_lit_6
	adds r3, r5, r0
	movs r1, #0
	ldrsh r0, [r3, r1]
	cmp r0, #2
	ble _0800AE78_tail
	movs r7, #255
	lsls r7, r7, #4
	adds r0, r5, r7
	movs r7, #0
	ldrsh r1, [r0, r7]
	cmp r1, #1
	bne _0800AE78_clear
	movs r7, #0
	ldrsh r0, [r4, r7]
	cmp r0, #0
	beq _0800AE78_clear
	strh r1, [r4, #0]
	b _0800AE78_set3_value
	.align 2, 0
_0800AE78_lit_6:
	.word 0x00000FF6
_0800AE78_clear:
	strh r2, [r4, #0]
_0800AE78_set3_value:
	movs r0, #3
	strh r0, [r3, #0]
_0800AE78_tail:
	cmp r6, #3
	bne _0800AE78_done
	mov r0, ip
	cmp r0, #0
	bne _0800AE78_done
	ldr r1, _0800AE78_lit_7
	adds r0, r5, r1
	movs r2, #0
	ldrsh r1, [r0, r2]
	lsls r1, r1, #1
	ldr r3, _0800AE78_lit_8
	adds r0, r5, r3
	movs r7, #0
	ldrsh r0, [r0, r7]
	lsls r0, r0, #3
	adds r1, r1, r0
	ldr r2, _0800AE78_lit_9
	adds r0, r5, r2
	adds r1, r1, r0
	movs r0, #1
	strh r0, [r1, #0]
_0800AE78_done:
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_0800AE78_lit_7:
	.word 0x00000FF8
_0800AE78_lit_8:
	.word 0x00000FF2
_0800AE78_lit_9:
	.word 0x0000102A

@ ----------------------------------------------------------------------------
@ _0800AF84 - per-frame progress state tail.
_0800AF84:
	push {r4, lr}
	ldr r2, _0800AF84_lit_0
	ldr r0, _0800AF84_lit_1
	adds r3, r2, r0
	ldrb r1, [r3, #0]
	cmp r1, #0
	beq _0800AFA4
	movs r0, #0
	strb r0, [r3, #0]
	movs r0, #47
	b _0800B0AE
	.align 2, 0
_0800AF84_lit_0:
	.word 0x03001780
_0800AF84_lit_1:
	.word 0x000010BE
_0800AFA4:
	movs r0, #134
	lsls r0, r0, #5
	adds r3, r2, r0
	ldrb r0, [r3, #0]
	cmp r0, #0
	beq _0800AFBA
	strb r1, [r3, #0]
	bl sub_08004CC4
	movs r0, #13
	b _0800B0AE
_0800AFBA:
	ldr r3, _0800AF84_lit_2
	adds r1, r2, r3
	movs r0, #0
	ldrsb r0, [r1, r0]
	cmp r0, #1
	beq _0800AFDE
	cmp r0, #1
	bgt _0800AFD4
	cmp r0, #0
	beq _0800AFE4
	b _0800AFE6
	.align 2, 0
_0800AF84_lit_2:
	.word 0x000010E5
_0800AFD4:
	cmp r0, #2
	beq _0800AFE4
	cmp r0, #3
	beq _0800AFE2
	b _0800AFE6
_0800AFDE:
	movs r0, #3
	b _0800AFE4
_0800AFE2:
	movs r0, #1
_0800AFE4:
	strb r0, [r1, #0]
_0800AFE6:
	ldr r4, _0800AF84_lit_3
	ldr r1, _0800AF84_lit_4
	adds r0, r4, r1
	movs r1, #0
	strh r1, [r0, #0]
	ldr r2, _0800AF84_lit_5
	adds r0, r4, r2
	strh r1, [r0, #0]
	ldr r3, _0800AF84_lit_6
	adds r0, r4, r3
	strh r1, [r0, #0]
	ldr r1, _0800AF84_lit_7
	adds r0, r4, r1
	movs r2, #0
	ldrsh r0, [r0, r2]
	subs r3, #126
	adds r1, r4, r3
	movs r2, #0
	ldrsh r1, [r1, r2]
	adds r3, #68
	adds r2, r4, r3
	movs r3, #0
	ldrsh r2, [r2, r3]
	bl sub_08025CF4
	lsls r0, r0, #24
	lsrs r0, r0, #24
	ldr r2, _0800AF84_lit_8
	adds r1, r4, r2
	strh r0, [r1, #0]
	ldr r3, _0800AF84_lit_9
	adds r0, r4, r3
	movs r1, #0
	ldrsh r0, [r0, r1]
	cmp r0, #1
	beq _0800B066
	cmp r0, #1
	bgt _0800B054
	cmp r0, #0
	beq _0800B070
	b _0800B088
	.align 2, 0
_0800AF84_lit_3:
	.word 0x03001780
_0800AF84_lit_4:
	.word 0x0000103C
_0800AF84_lit_5:
	.word 0x0000104A
_0800AF84_lit_6:
	.word 0x00001074
_0800AF84_lit_7:
	.word 0x00000FF2
_0800AF84_lit_8:
	.word 0x00001054
_0800AF84_lit_9:
	.word 0x00000FBC
_0800B054:
	cmp r0, #2
	beq _0800B084
	cmp r0, #7
	bne _0800B088
	ldr r2, _0800AF84_lit_10
	adds r0, r4, r2
	ldrh r0, [r0, #0]
	cmp r0, #2
	bne _0800B088
_0800B066:
	bl 0x0800BD40
	b _0800B088
	.align 2, 0
_0800AF84_lit_10:
	.word 0x00001078
_0800B070:
	ldr r3, _0800AF84_lit_11
	adds r0, r4, r3
	ldrh r0, [r0, #0]
	cmp r0, #2
	bne _0800B088
	bl 0x0800BE74
	b _0800B088
	.align 2, 0
_0800AF84_lit_11:
	.word 0x000010FC
_0800B084:
	bl 0x0800BE20
_0800B088:
	ldr r0, _0800AF84_lit_12
	ldr r1, _0800AF84_lit_13
	adds r4, r0, r1
	movs r2, #0
	ldrsh r0, [r4, r2]
	cmp r0, #0
	bne _0800B09A
	bl _0800AD84
_0800B09A:
	bl _0800AA40
	movs r3, #0
	ldrsh r0, [r4, r3]
	cmp r0, #0
	bne _0800B0AA
	bl _0800AE78
_0800B0AA:
	bl _0800AA20
_0800B0AE:
	pop {r4}
	pop {r1}
	bx r1
	.align 2, 0
_0800AF84_lit_12:
	.word 0x03001780
_0800AF84_lit_13:
	.word 0x00000FBC
