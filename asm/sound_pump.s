@ GT Advance 3 - MTO sound driver: VBlank Direct-Sound DMA re-arm pump
@ Region: file offset 0x02BE78-0x02BEB4 (VMA 0x0802BE78-0x0802BEB4),
@ including the private literal pool {0x040000BC, 0x84400004} at
@ 0x02BEAC/0x02BEB0. Disassembled via objdump from baserom.gba;
@ byte-exact.
@ Companion: asm/sound_api.s
@
@ Root struct reached through IWRAM pointer cell 0x03007FF0:
@   +0x00 u32 "Smsh" re-entrancy lock (0x68736D53, or magic+1 while a
@         walker/mixer/tick holds it)
@   +0x04 u8  frame countdown (refill gate)
@   +0x0B u8  reload value (refill period, frames)
@ DMA1 registers (base 0x040000BC): [+8] = DMA1CNT word (count|control),
@ [+10] = DMA1CNT_H control halfword (0x040000C6).

	.thumb

@ ----------------------------------------------------------------------------
@ sub_0802BE78 — VBlank sample pump. Sole caller: _0802B07C in
@ sound_api.s (@0x02B08C), runs each VBlank while the driver is enabled.
@   * gate: root lock must be magic or magic+1, else return untouched;
@   * decrement u8[root+4]; while still positive, done;
@   * on underflow reload u8[root+11] into [root+4], then re-arm
@     Direct Sound A on DMA1:
@       - if word[0x040000C4] & 1 (enabled): write 0x84400004
@         (count = 4 words, dest fixed, 32-bit, immediate => one-shot
@         16-byte FIFO-A prime from the sample buffer),
@       - CNT_H <- 0x0400 (disable), then CNT_H <- 0xB600
@         (enable | repeat | 32-bit | timing 11 = DS-A refill on
@         FIFO-empty) => continuous hardware-driven refill.
@ Sample-rate timer/DAD programming lives in bank init (unconverted).
sub_0802BE78:
	ldr r0, lit_0802C114        @ =0x03007FF0 root-ptr cell (pool 0x2C114)
	ldr r0, [r0, #0]
	ldr r2, lit_0802C118        @ =0x68736D53 "Smsh" (pool 0x2C118)
	ldr r3, [r0, #0]
	subs r3, r3, r2
	cmp r3, #1
	bhi _0802BEAA               @ lock not {magic, magic+1} -> done
	ldrb r1, [r0, #4]
	subs r1, #1
	strb r1, [r0, #4]
	bgt _0802BEAA               @ countdown still running
	ldrb r1, [r0, #11]
	strb r1, [r0, #4]           @ reload countdown
	ldr r2, _0802BE78_lit_dma   @ =0x040000BC DMA1 base
	ldr r1, [r2, #8]            @ word[0x040000C4] DMA1CNT
	lsls r1, r1, #7             @ test enable bit (bit0)
	bcc _0802BE9E               @ disabled -> skip prime
	ldr r1, _0802BE78_lit_prime @ =0x84400004
	str r1, [r2, #8]            @ FIFO-A prime: 4 words, dst fixed
_0802BE9E:
	movs r1, #4
	lsls r1, r1, #8             @ 0x0400
	strh r1, [r2, #10]          @ CNT_H: disable (keep 32-bit flag)
	movs r1, #182
	lsls r1, r1, #8             @ 0xB600
	strh r1, [r2, #10]          @ CNT_H: enable|repeat|32-bit|DS-A refill
_0802BEAA:
	bx lr

_0802BE78_lit_dma:
	.word 0x040000BC
_0802BE78_lit_prime:
	.word 0x84400004
