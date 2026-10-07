@
@ asm/course_load.s — _0801A204 COURSE LOAD entry
@
@ VMA 0x0801A204–0x0801A293 incl. private literal pool 0x0801A27C–
@ 0x0801A293. Called EXACTLY twice per race entry, once per race-scene
@ event through two independent wrappers (see tools/track_dump.py):
@
@   scene ev 1 → _0801AAA0 ……………… bl _0801A204 @0x0801AB04   [pass A]
@   scene ev 5 → _0801D224 → _0801BB14 → _0801A294
@                → bl _0801A204 @0x0801A2B8                    [pass B]
@
@ Each pass resolves the raw course/event id and runs the full
@ orchestrator _08006138, whose tail decompresses the group-1 surface
@ map to register-built EWRAM 0x02000000 via BIOS LZ77UnCompWram
@ (swi wrapper sub_0802D988). This dual invocation is the runtime path
@ behind the observation of TWO full-map writes (~frame 2248
@ and ~2346; divergent cell 0x02008B74 flips on both).
@
@ Body (byte-exact transcription, objdump cross-checked):
@   sp+00/08 : two 6-byte selector templates copied from ROM 0x0805FB98
@              ({h{0,1,2}} identity tables)
@   sp+14    : variant cell staged here, then installed into EWRAM
@              0x0203F758 via _08006114/_080060EC
@   h[0x03002886] → template[(course sel)<<1] → cell +2   (twin select)
@   h[0x03002884] → template[(time sel)<<1]   → cell +0
@   _08006138(out=0x030039D0, s16[0x0300287E], base=NULL)
@   _08006618 sprite-table init; _08008290 sprite/OAM setup
@
	.syntax unified
	.cpu arm7tdmi
	.text
	.align 2

	.type _0801A204, %function
_0801A204:
	push	{r4, r5, r6, lr}
	sub	sp, #28
	ldr	r4, _0801A27C          @ = 0x0805FB98 selector templates
	mov	r0, sp
	adds	r1, r4, #0
	movs	r2, #6
	bl	sub_0802E0A4           @ copy template -> sp+00
	add	r6, sp, #8
	adds	r0, r6, #0
	adds	r1, r4, #0
	movs	r2, #6
	bl	sub_0802E0A4           @ copy template -> sp+08
	movs	r0, #0
	str	r0, [sp, #16]
	add	r0, sp, #16
	add	r5, sp, #20            @ variant staging cell
	ldr	r2, _0801A280          @ = 0x05000002 CpuSet fill ctrl
	adds	r1, r5, #0
	bl	sub_0802D974           @ zero 8 B at sp+14..1B
	ldr	r4, _0801A284          @ = 0x03001780 state region base
	ldr	r1, _0801A288          @ = 0x03001106 -> +0x1106 = 0x03002886
	adds	r0, r4, r1
	movs	r2, #0
	ldrsh	r0, [r0, r2]           @ course-selector s16
	lsls	r0, r0, #1
	add	r0, sp                 @ index template at sp+00
	ldrh	r0, [r0, #0]
	strh	r0, [r5, #2]           @ cell +2 = twin-select template
	subs	r1, #2                 @ 0x03001104 -> +0x1104 = 0x03002884
	adds	r0, r4, r1
	movs	r2, #0
	ldrsh	r0, [r0, r2]           @ time/weather-selector s16
	lsls	r0, r0, #1
	adds	r6, r6, r0             @ index second template at sp+08
	ldrh	r0, [r6, #0]
	strh	r0, [r5, #0]           @ cell +0
	bl	_08006114              @ zero variant cell 0x0203F758
	adds	r0, r5, #0
	bl	_080060EC              @ install staged pair -> 0x0203F758
	ldr	r0, _0801A28C          @ = 0x030039D0 orchestrator out
	ldr	r1, _0801A290          @ = 0x030010FE -> +0x10FE = 0x0300287E
	adds	r4, r4, r1
	movs	r2, #0
	ldrsh	r1, [r4, r2]           @ raw event/course index
	movs	r2, #0
	bl	_08006138              @ ORCHESTRATOR (-> surface load pass)
	bl	_08006618              @ sprite table init
	bl	sub_08008290           @ sprite/OAM setup (unconverted)
	add	sp, #28
	pop	{r4, r5, r6}
	pop	{r0}
	bx	r0

	.align 2
_0801A27C: .word 0x0805FB98     @ menu selector identity templates
_0801A280: .word 0x05000002     @ CpuSet fill control (2 words)
_0801A284: .word 0x03001780     @ save/state region base
_0801A288: .word 0x00001106     @ offset -> course selector 0x03002886
_0801A28C: .word 0x030039D0     @ orchestrator out struct
_0801A290: .word 0x000010FE     @ offset -> event index 0x0300287E
