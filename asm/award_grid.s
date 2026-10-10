@ GT Advance 3 - collection award threshold helper
@ Region: file offset 0x00B9F0-0x00BA3C (VMA 0x0800B9F0-0x0800BA3C).
@ Counts completed award-grid cells and grants four threshold cars.

.thumb

sub_0800B9F0:
_0800B9F0:
	push {r4, r5, lr}
	movs r5, #0
	movs r4, #0
_0800B9F6:
	adds r0, r4, #0
	bl 0x08025E1C
	lsls r0, r0, #24
	lsrs r0, r0, #24
	cmp r0, #3
	bne.n _0800BA06
	adds r5, #1
_0800BA06:
	adds r4, #1
	cmp r4, #31
	ble.n _0800B9F6
	cmp r5, #7
	ble.n _0800BA16
	movs r0, #51
	bl 0x0800B990
_0800BA16:
	cmp r5, #15
	ble.n _0800BA20
	movs r0, #78
	bl 0x0800B990
_0800BA20:
	cmp r5, #23
	ble.n _0800BA2A
	movs r0, #89
	bl 0x0800B990
_0800BA2A:
	cmp r5, #31
	ble.n _0800BA34
	movs r0, #26
	bl 0x0800B990
_0800BA34:
	pop {r4, r5}
	pop {r0}
	bx r0
	.short 0x0000
award_grid_end:
