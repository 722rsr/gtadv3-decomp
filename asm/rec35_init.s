@ GT Advance 3 - rec35 scene initializer
@ Region: file offset 0x015CB4-0x015E28 (VMA 0x08015CB4-0x08015E28).
@ Pure Thumb with armcc high-register prologue/epilogue and private pools;
@ byte-exact.

.thumb
.type sub_08015CB4, %function
sub_08015CB4:
_08015CB4:
	push {r4, r5, r6, r7, lr}
	mov r7, r9
	mov r6, r8
	push {r6, r7}
	sub sp, #8
	adds r7, r0, #0
	bl 0x0802B234
	ldr r1, _08015D08
	movs r0, #4
	str r0, [sp, #0]
	movs r0, #2
	str r0, [sp, #4]
	movs r0, #0
	movs r2, #1
	movs r3, #0
	bl 0x08007770
	ldr r1, _08015D0C
	movs r4, #0
	str r4, [sp, #0]
	movs r0, #3
	str r0, [sp, #4]
	movs r0, #1
	movs r2, #1
	movs r3, #0
	bl 0x08007770
	movs r0, #6
	str r0, [r7, #88]
	bl 0x08004B68
	movs r1, #0
	ldrsh r0, [r0, r1]
	cmp r0, #36
	beq.n _08015D10
	cmp r0, #37
	beq.n _08015D1E
	movs r2, #142
	adds r2, r2, r7
	mov r9, r2
	b.n _08015D2C
	_08015D08: .word 0x082FCF04
	_08015D0C: .word 0x082B7410
_08015D10:
	movs r0, #14
	str r0, [r7, #100]
	adds r0, r7, #0
	adds r0, #142
	strh r4, [r0, #0]
	mov r9, r0
	b.n _08015D2C
_08015D1E:
	movs r0, #15
	str r0, [r7, #100]
	adds r1, r7, #0
	adds r1, #142
	movs r0, #1
	strh r0, [r1, #0]
	mov r9, r1
_08015D2C:
	movs r0, #1
	mov r8, r0
	str r0, [r7, #112]
	adds r6, r7, #0
	adds r6, #16
	adds r0, r6, #0
	bl 0x0800DAB8
	ldr r4, _08015DF8
	adds r0, r4, #0
	movs r1, #1
	movs r2, #0
	movs r3, #4
	bl 0x08007614
	adds r0, r4, #0
	movs r1, #1
	movs r2, #1
	movs r3, #5
	bl 0x08007614
	ldr r4, _08015DFC
	adds r0, r4, #0
	adds r1, r7, #0
	bl 0x0800798C
	adds r0, r7, #0
	bl 0x08007A58
	adds r0, r4, #0
	movs r1, #0
	movs r2, #3
	bl 0x080075E8
	ldr r4, _08015E00
	adds r1, r7, #0
	adds r1, #8
	adds r0, r4, #0
	bl 0x0800798C
	adds r0, r4, #0
	movs r1, #0
	movs r2, #6
	bl 0x080075E8
	adds r4, r7, #0
	adds r4, #160
	adds r0, r4, #0
	movs r1, #0
	bl 0x08025BC8
	ldr r0, [r7, #12]
	adds r1, r7, #0
	adds r1, #164
	ldr r1, [r1, #0]
	ldr r2, [r4, #0]
	bl 0x08007ABC
	adds r0, r7, #0
	adds r0, #128
	adds r5, r7, #0
	adds r5, #152
	str r5, [r0, #0]
	adds r0, #4
	str r6, [r0, #0]
	movs r4, #0
	str r4, [r7, #16]
	mov r1, r8
	strh r1, [r7, #20]
	subs r0, #96
	movs r2, #32
	negs r2, r2
	movs r1, #0
	bl 0x0800D77C
	adds r0, r7, #0
	adds r0, #28
	movs r1, #0
	movs r2, #160
	bl 0x0800D77C
	movs r0, #6
	strh r0, [r7, #24]
	movs r0, #5
	strh r0, [r7, #26]
	movs r0, #2
	str r0, [r5, #0]
	adds r0, r7, #0
	adds r0, #144
	strh r4, [r0, #0]
	adds r0, #4
	strh r4, [r0, #0]
	adds r0, #2
	strh r4, [r0, #0]
	mov r2, r9
	movs r0, #0
	ldrsh r1, [r2, r0]
	cmp r1, #0
	beq.n _08015E04
	cmp r1, #1
	beq.n _08015E12
	b.n _08015E18
	_08015DF8: .word 0x082A798C
	_08015DFC: .word 0x082FCF04
	_08015E00: .word 0x082B7410
_08015E04:
	adds r0, r7, #0
	adds r0, #140
	strh r1, [r0, #0]
	movs r0, #0
	bl 0x0800279C
	b.n _08015E18
_08015E12:
	adds r0, r7, #0
	adds r0, #140
	strh r1, [r0, #0]
_08015E18:
	add sp, #8
	pop {r3, r4}
	mov r8, r3
	mov r9, r4
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.short 0
