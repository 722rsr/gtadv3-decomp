@ GT Advance 3 - collection-line bit writer
@ Region: file offset 0x025248-0x025264 (VMA 0x08025248-0x08025264).
@ Pure Thumb; byte-exact transcription.

.thumb
.type sub_08025248, %function
sub_08025248:
_08025248:
	push {lr}
	sub sp, #4
	adds r1, r0, #0
	movs r0, #0
	str r0, [sp, #0]
	ldr r2, _08025260
	mov r0, sp
	bl 0x0802D974
	add sp, #4
	pop {r0}
	bx r0
_08025260: .word 0x05000006

@ The span ends where the next file begins: this region is exactly
@ 0x025248-0x025264 and asm/passthrough.inc:535 opens the next one with
@ `.type sub_08025264, %function`, so the owning file cannot spell the next
@ VMA's labels and the splicer has no textual end anchor to bound a replacement
@ of 0x08025248. Naming the region's end is the documented remedy (cf.
@ menu_dispatch.s / `menu_dispatch_end:`). It emits no bytes, so the reference
@ ROM hash is unchanged, but it is NOT hash-neutral: docs/data/independent_slice.json
@ and docs/data/code_data_ownership.json pin this file's SHA-256 and size, so
@ `make independent-slice` and `make ownership-map` must run in the same change.
ai_line_write_end:
