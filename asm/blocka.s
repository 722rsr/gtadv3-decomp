@ GT Advance 3 - state block A machinery (global mode state machine)
@ Region: file offset 0x0015F4-0x001988 (VMA 0x080015F4-0x08001988)
@
@ Disassembled via gbadisasm/objdump from baserom.gba; byte-exact.
@
@ Block A lives at IWRAM 0x03000008; its pointer slot is IWRAM
@ 0x030000E4. Field map (+ = block A offset):
@   +0x04 u16  pending "param" halfword (set by _080016D0, consumed by
@              mode-commit case 1 -> written as 0x03EB, read by _080021EC)
@   +0x06 u16  SIOCNT bits 4-5 snapshot (written every frame by _08001834)
@   +0x08 u16  scratch set on game-session enter transition
@   +0x10 u16  current mode
@   +0x12 u16  requested mode
@   +0x18 s16  last-min result mirror          +0x1C s16 current min
@   +0x1E u16  seconds-in-min counter (wraps at 30)
@   +0x24 u32  arg word for mode-8 hook       +0x28 u32 arg for mode-11
@   +0x70..    record area: 16-byte records selected by table word
@
@ Record selection uses ROM descriptor table 0x0802E190:
@   entry = 0x0802E190 + idx*4 + sel*16   (sel = st[+6] or st[+6]^1)
@   record_offset_in_blockA = [entry] * 16

.thumb

@ ----------------------------------------------------------------------------
@ _080015F4(idx) -> u16 — record getter (normal select)
@ entry word picks a 16-byte record; returns its halfword #idx*2... here
@ the record base itself: returns u16 at st+0x70+[entry]*16 (offset arg
@ is baked by caller via idx<<2 only).
@ Callers: _08001870 (loop), 0x8001AE0, 0x800206E
_080015F4:
	push {r4, lr}
	ldr r1, _08001618 @ =0x030000E4
	ldr r2, [r1]
	ldr r3, _0800161C @ =0x0802E190
	lsls r0, r0, #2
	ldrh r4, [r2, #6]
	lsls r1, r4, #4
	adds r0, r0, r1
	adds r0, r0, r3
	ldr r0, [r0]
	lsls r0, r0, #4
	adds r2, #0x70
	adds r2, r2, r0
	ldrh r0, [r2]
	pop {r4}
	pop {r1}
	bx r1
	movs r0, r0
	.align 2, 0
_08001618: .4byte 0x030000E4
_0800161C: .4byte 0x0802E190

@ ----------------------------------------------------------------------------
@ _08001620(idx0, idx1) -> u16 — record getter with halfword offset
@ entry = tbl[idx0]; off = (idx1+1)*2; return u16[st+0x70+[entry]*16+off]
@ Caller: _08001898 (loop in _08001854)
_08001620:
	push {r4, r5, lr}
	ldr r2, _08001648 @ =0x030000E4
	ldr r3, [r2]
	adds r1, #1
	lsls r1, r1, #1
	ldr r4, _0800164C @ =0x0802E190
	lsls r0, r0, #2
	ldrh r5, [r3, #6]
	lsls r2, r5, #4
	adds r0, r0, r2
	adds r0, r0, r4
	ldr r0, [r0]
	lsls r0, r0, #4
	adds r1, r1, r0
	adds r3, #0x70
	adds r3, r3, r1
	ldrh r0, [r3]
	pop {r4, r5}
	pop {r1}
	bx r1
	.align 2, 0
_08001648: .4byte 0x030000E4
_0800164C: .4byte 0x0802E190

@ ----------------------------------------------------------------------------
@ _08001650 -> u16 — first halfword of normal-select record
@ Callers: 0x800198A, 0x8001AA2, 0x8001B52
	.type _08001650, %function
_08001650:
	ldr r0, _0800165C @ =0x030000E4
	ldr r0, [r0]
	adds r0, #0x70
	ldrh r0, [r0]
	bx lr
	movs r0, r0
	.align 2, 0
_0800165C: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001660(halfword_idx) -> u16 — indexed halfword, normal select
@ Callers: 0x8001998, 0x80019B2, 0x80019C0
	.type _08001660, %function
_08001660:
	ldr r1, _08001670 @ =0x030000E4
	ldr r1, [r1]
	adds r0, #1
	lsls r0, r0, #1
	adds r1, #0x70
	adds r1, r1, r0
	ldrh r0, [r1]
	bx lr
	.align 2, 0
_08001670: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001674 -> address of normal-select record (st+0x70)
@ NOTE: returns a pointer, not a value.
	.type _08001674, %function
_08001674:
	ldr r0, _0800167C @ =0x030000E4
	ldr r0, [r0]
	adds r0, #0x70
	bx lr
	.align 2, 0
_0800167C: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001680 -> u16 — first halfword of INVERTED-select record
@ sel = st[+6] ^ 1. Callers: 0x800198A, 0x8001AA2, 0x8001B52
_08001680:
	ldr r0, _08001694 @ =0x030000E4
	ldr r1, [r0]
	movs r0, #1
	ldrh r2, [r1, #6]
	eors r0, r2
	lsls r0, r0, #4
	adds r1, #0x70
	adds r1, r1, r0
	ldrh r0, [r1]
	bx lr
	.align 2, 0
_08001694: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001698(halfword_idx) -> u16 — indexed halfword, inverted select
@ Callers: 0x8001998, 0x80019B2, 0x80019C0
_08001698:
	ldr r1, _080016B4 @ =0x030000E4
	ldr r2, [r1]
	adds r0, #1
	lsls r0, r0, #1
	movs r1, #1
	ldrh r3, [r2, #6]
	eors r1, r3
	lsls r1, r1, #4
	adds r0, r0, r1
	adds r2, #0x70
	adds r2, r2, r0
	ldrh r0, [r2]
	bx lr
	movs r0, r0
	.align 2, 0
_080016B4: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _080016B8 -> address of inverted-select record
_080016B8:
	ldr r0, _080016CC @ =0x030000E4
	ldr r0, [r0]
	movs r1, #1
	ldrh r2, [r0, #6]
	eors r1, r2
	lsls r1, r1, #4
	adds r1, #0x70
	adds r0, r0, r1
	bx lr
	movs r0, r0
	.align 2, 0
_080016CC: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _080016D0(v) — write v to block A +0x04 (pending param halfword)
@ Callers: 0x8001904, 0x8001A36, 0x8001A78, 0x8001AB8, 0x8001B0E,
@          0x8001B42, and _08002200 (block B cluster)
_080016D0:
	ldr r1, _080016D8 @ =0x030000E4
	ldr r1, [r1]
	strh r0, [r1, #4]
	bx lr
	.align 2, 0
_080016D8: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _080016DC — clear +0x04 and +0x08 (no direct BL callers found;
@ likely reached via pointer/fallthrough from adjacent dispatch code)
	.type _080016DC, %function
_080016DC:
	ldr r0, _080016E8 @ =0x030000E4
	ldr r1, [r0]
	movs r0, #0
	strh r0, [r1, #4]
	strh r0, [r1, #8]
	bx lr
	.align 2, 0
_080016E8: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _080016EC — register block A: slot 0x030000E4 <- 0x03000008, then
@ zero-fill 220 bytes (55 words, CpuFastSet fill). Called once per frame
@ from AgbMain's loop (asm/agbmain.s 4.4).
_080016EC:
	push {lr}
	ldr r1, _080016FC @ =0x030000E4
	ldr r0, _08001700 @ =0x03000008
	str r0, [r1]
	bl _08001704
	pop {r0}
	bx r0
	.align 2, 0
_080016FC: .4byte 0x030000E4
_08001700: .4byte 0x03000008

@ --- helper: CpuFastSet fill of block A (src=stack zero, dst=block A,
@     ctrl 0x05000037 = fill|32bit|55 words = 220 bytes) ---
_08001704:
	push {lr}
	sub sp, #4
	movs r0, #0
	str r0, [sp]
	ldr r1, _0800171C @ =0x03000008
	ldr r2, _08001720 @ =0x05000037
	mov r0, sp
	bl sub_0802D974
	add sp, #4
	pop {r0}
	bx r0
	.align 2, 0
_0800171C: .4byte 0x03000008
_08001720: .4byte 0x05000037

@ ----------------------------------------------------------------------------
@ _08001724(mode) — set requested mode (block A +0x12)
@ Also called from outside the cluster (0x8001F9E, 0x8001FBA).
_08001724:
	ldr r1, _0800172C @ =0x030000E4
	ldr r1, [r1]
	strh r0, [r1, #0x12]
	bx lr
	.align 2, 0
_0800172C: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001730(mode) -> bool — true for modes {1..7} U {10} ("game-driven")
@ Callers: commit proc below; also 0x8001C8E, 0x8001CE4.
_08001730:
	movs r1, #0
	cmp r0, #1
	blt.n _08001740
	cmp r0, #7
	ble.n _0800173E
	cmp r0, #0xa
	bne.n _08001740
_0800173E:
	movs r1, #1
_08001740:
	adds r0, r1, #0
	bx lr

@ ----------------------------------------------------------------------------
@ _08001744(mode) -> bool — true for modes {8, 11} ("special")
@ Caller: 0x8001B86.
_08001744:
	movs r1, #0
	cmp r0, #8
	beq.n _0800174E
	cmp r0, #0xb
	bne.n _08001750
_0800174E:
	movs r1, #1
_08001750:
	adds r0, r1, #0
	bx lr

@ _08001754: empty stub (single bx lr; no BL callers found)
	.type _08001754, %function
_08001754:
	bx lr
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _08001758 — mode-change processor (once per frame, from _08001834).
@ Pseudocode (see asm/blocka.s 3):
@   cur=+0x10; req=+0x12; if equal, done
@   oldA=A(cur); newA=A(req)
@   !oldA&&newA: session-enter 0x80005C0(0); +0x08=req
@    oldA&&!newA: session-exit  0x80006BC; +0x01=0
@   switch(req): 2..6,10 -> [[slot]]+0xD0 word = 0
@                1      -> +0x04 = 0x03EB
@                8      -> sub_08000BF0([[slot]][+0x24], 1)
@                11     -> sub_08000BF0([[slot]][+0x28], 0)
@   +0x10 = req
_08001758:
	push {r4, r5, r6, r7, lr}
	ldr r7, _0800178C @ =0x030000E4
	ldr r0, [r7]
	ldrh r1, [r0, #0x10]
	ldrh r5, [r0, #0x12]
	cmp r5, r1
	beq.n _0800180E
	adds r0, r1, #0
	bl _08001730
	adds r6, r0, #0
	adds r0, r5, #0
	bl _08001730
	adds r4, r0, #0
	cmp r6, #0
	bne.n _08001790
	cmp r4, #0
	beq.n _0800179C
	movs r0, #0
	bl _080005C0
	ldr r0, [r7]
	strh r6, [r0, #8]
	b.n _0800179C
	movs r0, r0
	.align 2, 0
_0800178C: .4byte 0x030000E4
_08001790:
	cmp r4, #0
	bne.n _0800179C
	bl _080006BC
	ldr r0, [r7]
	strb r4, [r0, #1]
_0800179C:
	cmp r5, #6
	beq.n _080017B2
	cmp r5, #6
	bgt.n _080017AE
	cmp r5, #3
	bgt.n _080017BC
	cmp r5, #2
	blt.n _080017BC
	b.n _080017B2
_080017AE:
	cmp r5, #0xa
	bne.n _080017BC
_080017B2:
	ldr r0, _080017CC @ =0x030000E4
	ldr r0, [r0]
	adds r0, #0xd0
	movs r1, #0
	str r1, [r0]
_080017BC:
	cmp r5, #8
	beq.n _080017E8
	cmp r5, #8
	bgt.n _080017D0
	cmp r5, #1
	beq.n _080017D6
	b.n _08001808
	movs r0, r0
	.align 2, 0
_080017CC: .4byte 0x030000E4
_080017D0:
	cmp r5, #0xb
	beq.n _080017FC
	b.n _08001808
_080017D6:
	ldr r0, _080017E0 @ =0x030000E4
	ldr r1, [r0]
	ldr r0, _080017E4 @ =0x000003EB
	strh r0, [r1, #4]
	b.n _08001808
	.align 2, 0
_080017E0: .4byte 0x030000E4
_080017E4: .4byte 0x000003EB
_080017E8:
	ldr r0, _080017F8 @ =0x030000E4
	ldr r0, [r0]
	ldr r1, [r0, #0x24]
	movs r0, #1
	bl sub_08000BF0
	b.n _08001808
	movs r0, r0
	.align 2, 0
_080017F8: .4byte 0x030000E4
_080017FC:
	ldr r0, _08001814 @ =0x030000E4
	ldr r0, [r0]
	ldr r1, [r0, #0x28]
	movs r0, #0
	bl sub_08000BF0
_08001808:
	ldr r0, _08001814 @ =0x030000E4
	ldr r0, [r0]
	strh r5, [r0, #0x10]
_0800180E:
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.align 2, 0
_08001814: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001818 / _08001824 — requested-mode wrappers (set 1 / set 0).
@ _08001818 callers: _08002132 plus engine ctors (0x80028736+, relocated).
	.type _08001818, %function
_08001818:
	push {lr}
	movs r0, #1
	bl _08001724
	pop {r0}
	bx r0

	.type _08001824, %function
_08001824:
	push {lr}
	movs r0, #0
	bl _08001724
	pop {r0}
	bx r0

@ _08001830: empty stub (single bx lr; no BL callers found)
	.type _08001830, %function
_08001830:
	bx lr
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _08001834 — per-frame mode sync, called from AgbMain idle path.
@ Snapshot: block A +0x06 = SIOCNT (0x04000128) bits 4-5 (link-port
@ status lines), then commit.
_08001834:
	push {lr}
	ldr r0, _0800184C @ =0x030000E4
	ldr r1, [r0]
	ldr r0, _08001850 @ =0x04000128
	ldr r0, [r0]
	lsls r0, r0, #0x1a
	lsrs r0, r0, #0x1e
	strh r0, [r1, #6]
	bl _08001758
	pop {r0}
	bx r0
	.align 2, 0
_0800184C: .4byte 0x030000E4
_08001850: .4byte 0x04000128

@ ----------------------------------------------------------------------------
@ _08001854 — per-frame clock/record updater (caller 0x8001C18).
@ Walks records 1..st[+0xA]: min over getters, tracks a 30-frame
@ countdown that commits "current minute" fields, writes mode-ish
@ halfwords when on the last record, then drives three calls into the
@ helper at 0x8001E48 and a final CpuFastSet copy of a 6-word record.
_08001854:
	push {r4, r5, r6, r7, lr}
	mov r7, r8
	push {r7}
	movs r6, #0
	ldr r5, _0800188C @ =0x00007FFF
	movs r7, #1
	movs r4, #1
	ldr r0, _08001890 @ =0x030000E4
	ldr r1, [r0]
	mov r8, r0
	ldrh r1, [r1, #0xa]
	cmp r7, r1
	bge.n _080018B2
_0800186E:
	adds r0, r4, #0
	bl _080015F4
	lsls r0, r0, #0x10
	lsrs r1, r0, #0x10
	movs r0, #0xfc
	lsls r0, r0, #2
	cmp r1, r0
	beq.n _08001894
	adds r0, #1
	cmp r1, r0
	bne.n _080018A6
	adds r6, #1
	b.n _080018A6
	movs r0, r0
	.align 2, 0
_0800188C: .4byte 0x00007FFF
_08001890: .4byte 0x030000E4
_08001894:
	adds r0, r4, #0
	movs r1, #0
	bl _08001620
	lsls r0, r0, #0x10
	lsrs r0, r0, #0x10
	cmp r0, r5
	bge.n _080018A6
	adds r5, r0, #0
_080018A6:
	adds r4, #1
	ldr r0, _080018D8 @ =0x030000E4
	ldr r0, [r0]
	ldrh r0, [r0, #0xa]
	cmp r4, r0
	blt.n _0800186E
_080018B2:
	mov r0, r8
	ldr r1, [r0]
	movs r2, #0x1c
	ldrsh r0, [r1, r2]
	cmp r0, r5
	bne.n _080018DC
	ldrh r0, [r1, #0x1e]
	adds r0, #1
	strh r0, [r1, #0x1e]
	lsls r0, r0, #0x10
	asrs r0, r0, #0x10
	cmp r0, #30
	ble.n _080018E2
	movs r0, #0
	strh r0, [r1, #0x1e]
	strh r5, [r1, #0x18]
	movs r7, #0
	b.n _080018E2
	movs r0, r0
	.align 2, 0
_080018D8: .4byte 0x030000E4
_080018DC:
	movs r0, #0
	strh r0, [r1, #0x1e]
	strh r5, [r1, #0x1c]
_080018E2:
	ldr r4, _08001930 @ =0x030000E4
	ldr r1, [r4]
	ldrh r0, [r1, #0xa]
	subs r0, #1
	cmp r6, r0
	bne.n _080018F6
	movs r0, #4
	strh r0, [r1, #0x12]
	movs r0, #0xa
	strh r0, [r1, #0x14]
_080018F6:
	cmp r7, #0
	beq.n _08001902
	ldr r1, [r4]
	ldrh r0, [r1, #0x18]
	adds r0, #1
	strh r0, [r1, #0x18]
_08001902:
	ldr r0, _08001934 @ =0x000003ED
	bl _080016D0
	ldr r0, [r4]
	ldrh r1, [r0, #0x18]
	movs r0, #0
	bl _08001E48
	ldr r2, [r4]
	movs r3, #0x18
	ldrsh r0, [r2, r3]
	cmp r0, #0
	bne.n _08001938
	ldrh r1, [r2, #0x2c]
	movs r0, #1
	bl _08001E48
	ldr r0, [r4]
	ldrh r1, [r0, #0x16]
	movs r0, #2
	bl _08001E48
	b.n _08001952
	.align 2, 0
_08001930: .4byte 0x030000E4
_08001934: .4byte 0x000003ED
_08001938:
	movs r1, #0x18
	ldrsh r0, [r2, r1]
	subs r0, #1
	lsls r1, r0, #1
	adds r1, r1, r0
	lsls r1, r1, #2
	ldr r0, [r2, #0x24]
	adds r0, r0, r1
	adds r1, r2, #0
	adds r1, #0xc4
	movs r2, #6
	bl sub_0802D974
_08001952:
	ldr r0, _08001984 @ =0x030000E4
	ldr r2, [r0]
	movs r3, #0x18
	ldrsh r0, [r2, r3]
	movs r3, #0x16
	ldrsh r1, [r2, r3]
	cmp r0, r1
	ble.n _08001964
	adds r0, r1, #0
_08001964:
	adds r4, r2, #0
	adds r4, #0xd0
	@ 0x0300 is the Thumb LSL-immediate encoding, not pool residue:
	@ `000 00 imm5=12 Rm=0 Rd=0`.  The dividend really is scaled here.
	lsls r0, r0, #12
	bl sub_0802DE04
	str r0, [r4]
	cmp r0, #0
	bge.n _08001978
	movs r0, #0
	str r0, [r4]
_08001978:
	pop {r3}
	mov r8, r3
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_08001984: .4byte 0x030000E4
