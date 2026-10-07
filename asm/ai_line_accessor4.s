@ GT Advance 3 - collection-line fourth byte accessor
@ Region: file offset 0x025530-0x025548 (VMA 0x08025530-0x08025548).
@ Pure Thumb; byte-exact transcription.

.thumb
.type sub_08025530, %function
sub_08025530:
_08025530:
	ldr r2, _08025544
	lsls r0, r0, #16
	asrs r0, r0, #16
	lsls r1, r0, #2
	adds r1, r1, r0
	lsls r1, r1, #2
	adds r1, r1, r2
	movs r0, #12
	ldrsb r0, [r1, r0]
	bx lr
_08025544: .word 0x080CCEEC

@ Region end 0x025548. The _08025530 span runs to here exactly, and this
@ file has no.include, so the marker bounds it truthfully.
ai_line_accessor4_end:
