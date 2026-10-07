@ GT Advance 3 - save/timer queue helpers
@ Region: file offset 0x005A58-0x005AE4 (VMA 0x08005A58-0x08005AE4).
@ Pure Thumb; byte-exact transcription.

.thumb
.type sub_08005A58, %function
sub_08005A58:
_08005A58:
	ldr r1, _08005A60
	ldr r0, _08005A64
	str r0, [r1, #0]
	bx lr
_08005A60: .word 0x030003D4
_08005A64: .word 0x030003B0

.type sub_08005A68, %function
sub_08005A68:
_08005A68:
	push {lr}
	sub sp, #4
	movs r0, #0
	str r0, [sp, #0]
	ldr r0, _08005A84
	ldr r1, [r0, #0]
	ldr r2, _08005A88
	mov r0, sp
	bl 0x0802D974
	add sp, #4
	pop {r0}
	bx r0
	.short 0
_08005A84: .word 0x030003D4
_08005A88: .word 0x05000009

.type sub_08005A8C, %function
sub_08005A8C:
_08005A8C:
	movs r1, #0
	ldr r0, _08005AA0
	ldr r0, [r0, #0]
	movs r2, #32
	ldrsh r0, [r0, r2]
	cmp r0, #0
	bne.n _08005A9C
	movs r1, #1
_08005A9C:
	adds r0, r1, #0
	bx lr
_08005AA0: .word 0x030003D4

.type sub_08005AA4, %function
sub_08005AA4:
_08005AA4:
	push {r4, r5, r6, lr}
	lsls r0, r0, #16
	lsrs r4, r0, #16
	lsls r1, r1, #24
	lsrs r1, r1, #24
	ldr r0, _08005AD0
	ldr r2, [r0, #0]
	movs r3, #0
	movs r5, #1
	adds r6, r0, #0
_08005AB8:
	ldrb r0, [r2, #0]
	cmp r0, #0
	bne.n _08005AD4
	strb r5, [r2, #0]
	strh r4, [r2, #2]
	strb r1, [r2, #1]
	ldr r1, [r6, #0]
	ldrh r0, [r1, #32]
	adds r0, #1
	strh r0, [r1, #32]
	b.n _08005ADC
	.short 0
_08005AD0: .word 0x030003D4
_08005AD4:
	adds r3, #1
	adds r2, #4
	cmp r3, #3
	ble.n _08005AB8
_08005ADC:
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.short 0

@ ----------------------------------------------------------------------------
@ `_08005AA4` is the LAST typed entry in this file, so it has no following
@ `.type` to use as a span end marker, and `tools/promotion_screen.py` refuses
@ it with "no end marker in save_timer_queue.s and no save_timer_queue_end: to
@ fall back on". This anchor emits NO bytes, so it cannot change the byte
@ stream; it only gives the last body a truthful bound. The file is
@ self-terminated with no `.include`, so EOF here is the region's real end: the
@ trailing `.short 0` above occupies 0x08005AE2-0x08005AE3 and the region
@ header puts the end at 0x08005AE4. `make independent-slice` and
@ `make ownership-map` must run in the same change that touches a file, to
@ refresh the pinned sha256.
save_timer_queue_end:
