@ GT Advance 3 - menu mode-6 state setter
@ Region: file offset 0x00CE70-0x00CE78 (VMA 0x0800CE70-0x0800CE78).
@ Pure Thumb; standalone leaf.

.thumb
.type sub_0800CE70, %function
sub_0800CE70:
_0800CE70:
	adds r1, #84
	movs r0, #6
	strh r0, [r1, #0]
	bx lr
menu_ce70_end:
