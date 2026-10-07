@ GT Advance 3 (A2GE) — crt0 + IntrMain + IRQ hook plumbing
@ Region: file offset 0x0000C0–0x0002C4 (VMA 0x080000C0–0x080002C4)
@
@ Disassembled by gbadisasm (jiangzhengwenjz) from baserom.gba and verified
@ byte-identical against the original region. Full annotated trace:
@ asm/agbmain.s
@
@ Labels follow gbadisasm's auto style (_08000XXX = VMA). Named symbols:
@   crt0      0x080000C0  stack setup, install IntrMain, bx AgbMain (ARM)
@   IntrMain  0x08000108  stock Nintendo IRQ dispatcher          (ARM)

    .text

    .global crt0
crt0: @ 0x080000C0
	mov r0, #0x12            @ CPSR = IRQ mode
	msr cpsr_fc, r0
	ldr sp, _080000F8 @ =0x03007FA0     @ sp_irq
	mov r0, #0x1f            @ CPSR = System mode
	msr cpsr_fc, r0
	ldr sp, _080000F4 @ =0x03007F00     @ sp_sys
	ldr r1, _080000FC @ =0x03007FFC     @ IRQ handler slot
	ldr r0, _08000100 @ =_08000108      @ IntrMain
	str r0, [r1]
	ldr r1, _08000104 @ =0x080002C5     @ AgbMain (Thumb bit set)
	mov lr, pc
	bx r1                    @ -> AgbMain
	b crt0                   @ AgbMain returned: restart crt0
	.align 2, 0
_080000F4: .4byte 0x03007F00
_080000F8: .4byte 0x03007FA0
_080000FC: .4byte 0x03007FFC
_08000100: .4byte IntrMain
_08000104: .4byte 0x080002C5

IntrMain: @ 0x08000108 — stock Nintendo IRQ dispatcher
	mov r3, #0x4000000       @ r3 = 0x04000200 (IE)
	add r3, r3, #0x200
	ldr r2, [r3]             @ r2 = (IF<<16)|IE
	ldrh r1, [r3, #8]        @ r1 = IME
	mrs r0, spsr
	push {r0, r1, r2, r3, lr}
	mov r0, #1
	strh r0, [r3, #8]        @ IME = 1 (nest during dispatch)
	and r1, r2, r2, lsr #16  @ r1 = pending = IF & IE
	mov ip, #0               @ ip = handler-table byte index
	ands r0, r1, #0xc0       @ slot 0: Timer3+Serial combo
	bne _080001B4
	add ip, ip, #4
	ands r0, r1, #1          @ slot 1: VBlank
	bne _080001B4
	add ip, ip, #4
	ands r0, r1, #4          @ slot 2: VCounter
	bne _080001B4
	add ip, ip, #4
	ands r0, r1, #2          @ slot 3: HBlank
	bne _080001B4
	add ip, ip, #4
	ands r0, r1, #0x100      @ slot 4: DMA0
	bne _080001B4
	add ip, ip, #4
	ands r0, r1, #0x200      @ slot 5: DMA1
	bne _080001B4
	add ip, ip, #4
	ands r0, r1, #0x400      @ slot 6: DMA2
	bne _080001B4
	add ip, ip, #4
	ands r0, r1, #0x800      @ slot 7: DMA3
	bne _080001B4
	add ip, ip, #4
	ands r0, r1, #0x1000     @ slot 8: Keypad
	bne _080001B4
	add ip, ip, #4
	ands r0, r1, #8          @ slot 9: Timer0
	bne _080001B4
	add ip, ip, #4
	ands r0, r1, #0x2000     @ GamePak: store + spin forever
	strbne r0, [r3, #-0x17c] @ *(u8*)0x04000084 = r0
_080001B0:
	bne _080001B0
_080001B4:
	strh r0, [r3, #2]        @ IF = pending (ack)
	mov r1, #0x20c0
	bic r2, r2, r0           @ IE &= ~pending
	and r1, r1, r2           @ IE &= 0x20C0 mask
	strh r1, [r3]
	mrs r3, apsr             @ switch to System mode
	bic r3, r3, #0xdf
	orr r3, r3, #0x1f
	msr cpsr_fc, r3
	ldr r1, _08000218 @ =0x0203F170     @ handler table base (EWRAM)
	add r1, r1, ip
	ldr r0, [r1]
	stmdb sp!, {lr}
	add lr, pc, #0x0 @ =0x080001F0      @ return address
	bx r0                    @ -> handler (ARM or Thumb)
	ldm sp!, {lr}
	mrs r3, apsr             @ back to IRQ mode
	bic r3, r3, #0xdf
	orr r3, r3, #0x92
	msr cpsr_fc, r3
	pop {r0, r1, r2, r3, lr}
	strh r2, [r3]            @ restore IE / IF
	strh r1, [r3, #8]        @ restore IME
	msr spsr_fc, r0
	bx lr
	.align 2, 0
_08000218: .4byte 0x0203F170

_0800021C: @ ARM session RX-poll helper (decoded,): IME-guarded
           @ swap EF90+0x28 <-> +0x2C (parked RX matrix <-> record-table
           @ slot), RETURNS OLD BYTE EF90[+5] in r0 (burst-complete flag
           @ raised by ISR blob A at [+0x18]==9) and clears it. Never BL'd
           @ from ROM: enter hook _080005C0 copies these 16 words to
           @ EWRAM 0x0203F110, where sub_08008F4 calls it through the
           @ bx-r0 veneer pool and gates verification on the result.
           @ See asm/agbmain.s / asm/handlers.s
	push {r8, sb, sl, fp}
	mov ip, #0x4000000
	ldr fp, _08000258 @ =0x0203EF90
	add sl, fp, #0x28
	mov sb, #1
	mov r8, #0
	strb r8, [ip, #0x208]    @ IME = 0
	ldm sl, {r0, r1}         @ swap [0x28] <-> [0x2C]
	stm sl!, {r1}
	stm sl!, {r0}
	ldrb r0, [fp, #5]
	strb r8, [fp, #5]        @ clear flag byte +5
	strb sb, [ip, #0x208]    @ IME = 1
	pop {r8, sb, sl, fp}
	bx lr
	.align 2, 0
_08000258: .4byte 0x0203EF90

	.thumb

	.type _0800025C, %function
_0800025C: @ IRQ slot 2 (VCounter): sound sequencer tick
	push {lr}
	bl sub_0802B098
	pop {r0}
	bx r0
	.align 2, 0

	.type _08000268, %function
_08000268: @ IRQ slot 1 (VBlank): soft-IRQ pump + sound tick + BIOS flag
	push {lr}
	bl sub_08000A60
	bl sub_0802B07C
	ldr r3, _08000288 @ =0x04000208
	movs r0, #0
	strh r0, [r3]            @ IME = 0
	ldr r2, _0800028C @ =0x03007FF8
	ldrh r0, [r2]
	movs r1, #1
	orrs r0, r1
	strh r0, [r2]            @ VBlank flag for SWI VBlankIntrWait
	strh r1, [r3]            @ IME = 1
	pop {r0}
	bx r0
	.align 2, 0
_08000288: .4byte 0x04000208
_0800028C: .4byte 0x03007FF8

_08000290: @ install IRQ hooks into IntrMain table (called once by AgbMain)
	push {lr}
	bl sub_08002A68          @ reset slot 0 -> default handler
	ldr r1, _080002B8 @ =0x0203EE71
	movs r0, #0              @ slot 0 <- sound state block + 0x21 (?)
	bl sub_08002A3C
	ldr r1, _080002BC @ =0x08000269
	movs r0, #1              @ slot 1 <- VBlank handler (Thumb)
	bl sub_08002A3C
	ldr r1, _080002C0 @ =0x0800025D
	movs r0, #2              @ slot 2 <- VCounter handler (Thumb)
	bl sub_08002A3C
	bl sub_08002A80          @ bookkeeping table @ IWRAM 0x03001768
	pop {r0}
	bx r0
	.align 2, 0
_080002B8: .4byte 0x0203EE71
_080002BC: .4byte 0x08000269
_080002C0: .4byte 0x0800025D

@ End-of-region anchor for the splicer. This file declares exactly one
@ `@ Region:` (0x080000C0-0x080002C4) and has no `.include`; the promoted body at
@ 0x08000290 ends on 0x080002C4, exactly that boundary, so the anchor is safe.
@ Without it the body is refused as 'no end marker in boot.s'.
boot_end:
