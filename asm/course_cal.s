@ GT Advance 3 — championship-calendar accessors (course-variant path)
@ Region: file offset 0x0258B8-0x025930 (VMA 0x080258B8-0x08025930)
@
@ Disassembled via objdump from baserom.gba; byte-exact (make SHA gate).
@
@ All six are identical s16 getters over the stride-12 event table at
@ ROM 0x08060124 (49 rows, ends exactly at the MTO directory 0x08060378):
@   return (s16) [0x08060124 + idx*12 + OFF]
@ Row fields (payload-relative):
@   +00 cup id (0..4)            — _080258B8
@   +02 cup theme-song id        — _080258E0 (53..56; started via _0802B1E4)
@   +04 course id, default       — _080258F4 (orchestrator default branch)
@   +06 course id, variant twin  — _08025908 (orchestrator when sel==2)
@   +08 per-row sequence number  — _080258CC
@   +10 per-row signed param     — _0802591C (-4..+2; race-setup descriptor)

.thumb

@ ----------------------------------------------------------------------------
@ _080258B8(idx) — row.cup_id (field +0)
_080258B8:
	ldr r2, _080258C8
	lsls r1, r0, #1
	adds r1, r1, r0
	lsls r1, r1, #2
	adds r1, r1, r2
	movs r2, #0
	ldrsh r0, [r1, r2]
	bx lr
	.align 2, 0
_080258C8: .4byte 0x08060124

@ ----------------------------------------------------------------------------
@ _080258CC(idx) — row.seqnum (field +8)
_080258CC:
	ldr r2, _080258DC
	lsls r1, r0, #1
	adds r1, r1, r0
	lsls r1, r1, #2
	adds r1, r1, r2
	movs r2, #8
	ldrsh r0, [r1, r2]
	bx lr
	.align 2, 0
_080258DC: .4byte 0x08060124

@ ----------------------------------------------------------------------------
@ _080258E0(idx) — row.song_id (field +2)
_080258E0:
	ldr r2, _080258F0
	lsls r1, r0, #1
	adds r1, r1, r0
	lsls r1, r1, #2
	adds r1, r1, r2
	movs r2, #2
	ldrsh r0, [r1, r2]
	bx lr
	.align 2, 0
_080258F0: .4byte 0x08060124

@ ----------------------------------------------------------------------------
@ _080258F4(idx) — row.course_default (field +4)
_080258F4:
	ldr r2, _08025904
	lsls r1, r0, #1
	adds r1, r1, r0
	lsls r1, r1, #2
	adds r1, r1, r2
	movs r2, #4
	ldrsh r0, [r1, r2]
	bx lr
	.align 2, 0
_08025904: .4byte 0x08060124

@ ----------------------------------------------------------------------------
@ _08025908(idx) — row.course_variant (field +6)
_08025908:
	ldr r2, _08025918
	lsls r1, r0, #1
	adds r1, r1, r0
	lsls r1, r1, #2
	adds r1, r1, r2
	movs r2, #6
	ldrsh r0, [r1, r2]
	bx lr
	.align 2, 0
_08025918: .4byte 0x08060124

@ ----------------------------------------------------------------------------
@ _0802591C(idx) — row.param (field +10)
_0802591C:
	ldr r2, _0802592C
	lsls r1, r0, #1
	adds r1, r1, r0
	lsls r1, r1, #2
	adds r1, r1, r2
	movs r2, #10
	ldrsh r0, [r1, r2]
	bx lr
	.align 2, 0
_0802592C: .4byte 0x08060124
course_cal_end:
