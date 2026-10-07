@ GT Advance 3 - collection-line byte accessors
@ Region: file offset 0x0254E8-0x025530 (VMA 0x080254E8-0x08025530).
@ Pure Thumb; byte-exact transcription of three independent leaves.

.thumb
.type sub_080254E8, %function
sub_080254E8:
_080254E8:
	ldr r2, _080254FC
	lsls r0, r0, #16
	asrs r0, r0, #16
	lsls r1, r0, #2
	adds r1, r1, r0
	lsls r1, r1, #2
	adds r1, r1, r2
	movs r0, #0
	ldrsb r0, [r1, r0]
	bx lr
_080254FC: .word 0x080CCEEC

.type sub_08025500, %function
sub_08025500:
_08025500:
	ldr r2, _08025514
	lsls r0, r0, #16
	asrs r0, r0, #16
	lsls r1, r0, #2
	adds r1, r1, r0
	lsls r1, r1, #2
	adds r1, r1, r2
	movs r0, #14
	ldrsb r0, [r1, r0]
	bx lr
_08025514: .word 0x080CCEEC

.type sub_08025518, %function
sub_08025518:
_08025518:
	ldr r2, _0802552C
	lsls r0, r0, #16
	asrs r0, r0, #16
	lsls r1, r0, #2
	adds r1, r1, r0
	lsls r1, r1, #2
	adds r1, r1, r2
	movs r0, #13
	ldrsb r0, [r1, r0]
	bx lr
_0802552C: .word 0x080CCEEC

@ Region end 0x025530. The _08025518 span runs to here exactly, and this
@ file has no.include, so the marker bounds it truthfully.
ai_line_accessors_end:
