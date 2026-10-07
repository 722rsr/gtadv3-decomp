@ menu mode-record initializer.
@ VMA 0x0800C7F4-0x0800C814 (32 bytes), pure Thumb with one private pool.

	.thumb
	.type sub_0800C7F4, %function
sub_0800C7F4:
	push {r4, lr}
	adds r4, r0, #0
	movs r0, #0
	bl 0x08004CA8
	strh r0, [r4]
	bl 0x0800254C
	ldr r0, .L_c810
	bl 0x08002124
	pop {r4}
	pop {r0}
	bx r0
.L_c810:
	.word 0x0000138A

@ Unique end marker so the body owned by this file can be C-owned.
@ Emits no bytes.
menu_c7f4_end:
