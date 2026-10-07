@ GT Advance 3 - MTO sound PSG channel updater
@ Region: file offset 0x02D034-0x02D480 (VMA 0x0802D034-0x0802D480).
@ Pure Thumb armcc state machine. This listing preserves every decoded
@ instruction, literal pool, branch target, and armcc pad halfword byte-exactly.

.thumb
.type sub_0802D034, %function
sub_0802D034:
_0802D034:
    @ GLOBAL: src/sound_core.c takes this entry's ADDRESS into the dispatch
    @ table. A local `t` symbol cannot satisfy another object's undefined
    @ reference, so the hybrid link trampolined the reference to the ROM and
    @ build_c.py failed closed (exit 2) -- a bare numeric bl operand is
    @ discoverable but not referenceable, and taking an address needs the
    @ latter. The pair is exported so the C can bind to the real body.
    .global sub_0802D034
    .global _0802D034
	.hword 0xb5f0 @ 0x802D034: push {r4, r5, r6, r7, lr}
	.hword 0x4657 @ 0x802D036: mov r7, sl
	.hword 0x464e @ 0x802D038: mov r6, r9
	.hword 0x4645 @ 0x802D03A: mov r5, r8
	.hword 0xb4e0 @ 0x802D03C: push {r5, r6, r7}
	.hword 0xb087 @ 0x802D03E: sub sp, #28
	.hword 0x4804 @ 0x802D040: ldr r0, [pc, #16]; (0x802d054)
	.hword 0x6800 @ 0x802D042: ldr r0, [r0, #0]
	.hword 0x9001 @ 0x802D044: str r0, [sp, #4]
	.hword 0x7a80 @ 0x802D046: ldrb r0, [r0, #10]
	.hword 0x2800 @ 0x802D048: cmp r0, #0
	.hword 0xd005 @ 0x802D04A: beq.n 0x802d058
	.hword 0x3801 @ 0x802D04C: subs r0, #1
	.hword 0x9901 @ 0x802D04E: ldr r1, [sp, #4]
	.hword 0x7288 @ 0x802D050: strb r0, [r1, #10]
	.hword 0xe004 @ 0x802D052: b.n 0x802d05e
_080054: .word 0x03007ff0 @ pool
	.hword 0x200e @ 0x802D058: movs r0, #14
	.hword 0x9a01 @ 0x802D05A: ldr r2, [sp, #4]
	.hword 0x7290 @ 0x802D05C: strb r0, [r2, #10]
	.hword 0x2601 @ 0x802D05E: movs r6, #1
	.hword 0x9801 @ 0x802D060: ldr r0, [sp, #4]
	.hword 0x69c4 @ 0x802D062: ldr r4, [r0, #28]
	.hword 0x7821 @ 0x802D064: ldrb r1, [r4, #0]
	.hword 0x20c7 @ 0x802D066: movs r0, #199; 0xc7
	.hword 0x4008 @ 0x802D068: ands r0, r1
	.hword 0x1c72 @ 0x802D06A: adds r2, r6, #1
	.hword 0x4692 @ 0x802D06C: mov sl, r2
	.hword 0x2240 @ 0x802D06E: movs r2, #64; 0x40
	.hword 0x1912 @ 0x802D070: adds r2, r2, r4
	.hword 0x4691 @ 0x802D072: mov r9, r2
	.hword 0x2800 @ 0x802D074: cmp r0, #0
	.hword 0xd100 @ 0x802D076: bne.n 0x802d07a
	.hword 0xe1f4 @ 0x802D078: b.n 0x802d464
	.hword 0x2e02 @ 0x802D07A: cmp r6, #2
	.hword 0xd016 @ 0x802D07C: beq.n 0x802d0ac
	.hword 0x2e02 @ 0x802D07E: cmp r6, #2
	.hword 0xdc02 @ 0x802D080: bgt.n 0x802d088
	.hword 0x2e01 @ 0x802D082: cmp r6, #1
	.hword 0xd003 @ 0x802D084: beq.n 0x802d08e
	.hword 0xe02d @ 0x802D086: b.n 0x802d0e4
	.hword 0x2e03 @ 0x802D088: cmp r6, #3
	.hword 0xd01b @ 0x802D08A: beq.n 0x802d0c4
	.hword 0xe02a @ 0x802D08C: b.n 0x802d0e4
	.hword 0x4804 @ 0x802D08E: ldr r0, [pc, #16]; (0x802d0a0)
	.hword 0x9002 @ 0x802D090: str r0, [sp, #8]
	.hword 0x4f04 @ 0x802D092: ldr r7, [pc, #16]; (0x802d0a4)
	.hword 0x4a04 @ 0x802D094: ldr r2, [pc, #16]; (0x802d0a8)
	.hword 0x9203 @ 0x802D096: str r2, [sp, #12]
	.hword 0x3004 @ 0x802D098: adds r0, #4
	.hword 0x9004 @ 0x802D09A: str r0, [sp, #16]
	.hword 0x3202 @ 0x802D09C: adds r2, #2
	.hword 0xe029 @ 0x802D09E: b.n 0x802d0f4
_0800A0: .word 0x04000060 @ pool
_0800A4: .word 0x04000062 @ pool
_0800A8: .word 0x04000063 @ pool
	.hword 0x4802 @ 0x802D0AC: ldr r0, [pc, #8]; (0x802d0b8)
	.hword 0x9002 @ 0x802D0AE: str r0, [sp, #8]
	.hword 0x4f02 @ 0x802D0B0: ldr r7, [pc, #8]; (0x802d0bc)
	.hword 0x4a03 @ 0x802D0B2: ldr r2, [pc, #12]; (0x802d0c0)
	.hword 0xe01a @ 0x802D0B4: b.n 0x802d0ec
	.short 0 @ pad 0x802D0B6
_0800B8: .word 0x04000061 @ pool
_0800BC: .word 0x04000068 @ pool
_0800C0: .word 0x04000069 @ pool
	.hword 0x4804 @ 0x802D0C4: ldr r0, [pc, #16]; (0x802d0d8)
	.hword 0x9002 @ 0x802D0C6: str r0, [sp, #8]
	.hword 0x4f04 @ 0x802D0C8: ldr r7, [pc, #16]; (0x802d0dc)
	.hword 0x4a05 @ 0x802D0CA: ldr r2, [pc, #20]; (0x802d0e0)
	.hword 0x9203 @ 0x802D0CC: str r2, [sp, #12]
	.hword 0x3004 @ 0x802D0CE: adds r0, #4
	.hword 0x9004 @ 0x802D0D0: str r0, [sp, #16]
	.hword 0x3202 @ 0x802D0D2: adds r2, #2
	.hword 0xe00e @ 0x802D0D4: b.n 0x802d0f4
	.short 0 @ pad 0x802D0D6
_0800D8: .word 0x04000070 @ pool
_0800DC: .word 0x04000072 @ pool
_0800E0: .word 0x04000073 @ pool
	.hword 0x4817 @ 0x802D0E4: ldr r0, [pc, #92]; (0x802d144)
	.hword 0x9002 @ 0x802D0E6: str r0, [sp, #8]
	.hword 0x4f17 @ 0x802D0E8: ldr r7, [pc, #92]; (0x802d148)
	.hword 0x4a18 @ 0x802D0EA: ldr r2, [pc, #96]; (0x802d14c)
	.hword 0x9203 @ 0x802D0EC: str r2, [sp, #12]
	.hword 0x300b @ 0x802D0EE: adds r0, #11
	.hword 0x9004 @ 0x802D0F0: str r0, [sp, #16]
	.hword 0x3204 @ 0x802D0F2: adds r2, #4
	.hword 0x9205 @ 0x802D0F4: str r2, [sp, #20]
	.hword 0x9801 @ 0x802D0F6: ldr r0, [sp, #4]
	.hword 0x7a80 @ 0x802D0F8: ldrb r0, [r0, #10]
	.hword 0x9000 @ 0x802D0FA: str r0, [sp, #0]
	.hword 0x9a03 @ 0x802D0FC: ldr r2, [sp, #12]
	.hword 0x7810 @ 0x802D0FE: ldrb r0, [r2, #0]
	.hword 0x4680 @ 0x802D100: mov r8, r0
	.hword 0x1c0a @ 0x802D102: adds r2, r1, #0
	.hword 0x2080 @ 0x802D104: movs r0, #128; 0x80
	.hword 0x4010 @ 0x802D106: ands r0, r2
	.hword 0x2800 @ 0x802D108: cmp r0, #0
	.hword 0xd06e @ 0x802D10A: beq.n 0x802d1ea
	.hword 0x2340 @ 0x802D10C: movs r3, #64; 0x40
	.hword 0x1c18 @ 0x802D10E: adds r0, r3, #0
	.hword 0x4010 @ 0x802D110: ands r0, r2
	.hword 0x0600 @ 0x802D112: lsls r0, r0, #24
	.hword 0x0e05 @ 0x802D114: lsrs r5, r0, #24
	.hword 0x1c70 @ 0x802D116: adds r0, r6, #1
	.hword 0x4682 @ 0x802D118: mov sl, r0
	.hword 0x2140 @ 0x802D11A: movs r1, #64; 0x40
	.hword 0x1909 @ 0x802D11C: adds r1, r1, r4
	.hword 0x4689 @ 0x802D11E: mov r9, r1
	.hword 0x2d00 @ 0x802D120: cmp r5, #0
	.hword 0xd174 @ 0x802D122: bne.n 0x802d20e
	.hword 0x2003 @ 0x802D124: movs r0, #3
	.hword 0x7020 @ 0x802D126: strb r0, [r4, #0]
	.hword 0x7760 @ 0x802D128: strb r0, [r4, #29]
	.hword 0x1c20 @ 0x802D12A: adds r0, r4, #0
	.hword 0x9306 @ 0x802D12C: str r3, [sp, #24]
	.hword 0xf7ff @ 0x802D12E: bl 0x802cfcc
	.hword 0xff4d @ 0x802D12E +2: continuation
	.hword 0x9b06 @ 0x802D132: ldr r3, [sp, #24]
	.hword 0x2e02 @ 0x802D134: cmp r6, #2
	.hword 0xd011 @ 0x802D136: beq.n 0x802d15c
	.hword 0x2e02 @ 0x802D138: cmp r6, #2
	.hword 0xdc09 @ 0x802D13A: bgt.n 0x802d150
	.hword 0x2e01 @ 0x802D13C: cmp r6, #1
	.hword 0xd00a @ 0x802D13E: beq.n 0x802d156
	.hword 0xe036 @ 0x802D140: b.n 0x802d1b0
	.short 0 @ pad 0x802D142
_080144: .word 0x04000071 @ pool
_080148: .word 0x04000078 @ pool
_08014C: .word 0x04000079 @ pool
	.hword 0x2e03 @ 0x802D150: cmp r6, #3
	.hword 0xd009 @ 0x802D152: beq.n 0x802d168
	.hword 0xe02c @ 0x802D154: b.n 0x802d1b0
	.hword 0x7fe0 @ 0x802D156: ldrb r0, [r4, #31]
	.hword 0x9a02 @ 0x802D158: ldr r2, [sp, #8]
	.hword 0x7010 @ 0x802D15A: strb r0, [r2, #0]
	.hword 0x6a60 @ 0x802D15C: ldr r0, [r4, #36]; 0x24
	.hword 0x0180 @ 0x802D15E: lsls r0, r0, #6
	.hword 0x7fa1 @ 0x802D160: ldrb r1, [r4, #30]
	.hword 0x1808 @ 0x802D162: adds r0, r1, r0
	.hword 0x7038 @ 0x802D164: strb r0, [r7, #0]
	.hword 0xe029 @ 0x802D166: b.n 0x802d1bc
	.hword 0x6a61 @ 0x802D168: ldr r1, [r4, #36]; 0x24
	.hword 0x6aa0 @ 0x802D16A: ldr r0, [r4, #40]; 0x28
	.hword 0x4281 @ 0x802D16C: cmp r1, r0
	.hword 0xd00f @ 0x802D16E: beq.n 0x802d190
	.hword 0x9a02 @ 0x802D170: ldr r2, [sp, #8]
	.hword 0x7013 @ 0x802D172: strb r3, [r2, #0]
	.hword 0x490b @ 0x802D174: ldr r1, [pc, #44]; (0x802d1a4)
	.hword 0x6a62 @ 0x802D176: ldr r2, [r4, #36]; 0x24
	.hword 0x6810 @ 0x802D178: ldr r0, [r2, #0]
	.hword 0x6008 @ 0x802D17A: str r0, [r1, #0]
	.hword 0x3104 @ 0x802D17C: adds r1, #4
	.hword 0x6850 @ 0x802D17E: ldr r0, [r2, #4]
	.hword 0x6008 @ 0x802D180: str r0, [r1, #0]
	.hword 0x3104 @ 0x802D182: adds r1, #4
	.hword 0x6890 @ 0x802D184: ldr r0, [r2, #8]
	.hword 0x6008 @ 0x802D186: str r0, [r1, #0]
	.hword 0x3104 @ 0x802D188: adds r1, #4
	.hword 0x68d0 @ 0x802D18A: ldr r0, [r2, #12]
	.hword 0x6008 @ 0x802D18C: str r0, [r1, #0]
	.hword 0x62a2 @ 0x802D18E: str r2, [r4, #40]; 0x28
	.hword 0x9802 @ 0x802D190: ldr r0, [sp, #8]
	.hword 0x7005 @ 0x802D192: strb r5, [r0, #0]
	.hword 0x7fa0 @ 0x802D194: ldrb r0, [r4, #30]
	.hword 0x7038 @ 0x802D196: strb r0, [r7, #0]
	.hword 0x7fa0 @ 0x802D198: ldrb r0, [r4, #30]
	.hword 0x2800 @ 0x802D19A: cmp r0, #0
	.hword 0xd004 @ 0x802D19C: beq.n 0x802d1a8
	.hword 0x20c0 @ 0x802D19E: movs r0, #192; 0xc0
	.hword 0xe013 @ 0x802D1A0: b.n 0x802d1ca
	.short 0 @ pad 0x802D1A2
_0801A4: .word 0x04000090 @ pool
	.hword 0x2180 @ 0x802D1A8: movs r1, #128; 0x80
	.hword 0x4249 @ 0x802D1AA: negs r1, r1
	.hword 0x76a1 @ 0x802D1AC: strb r1, [r4, #26]
	.hword 0xe00d @ 0x802D1AE: b.n 0x802d1cc
	.hword 0x7fa0 @ 0x802D1B0: ldrb r0, [r4, #30]
	.hword 0x7038 @ 0x802D1B2: strb r0, [r7, #0]
	.hword 0x6a60 @ 0x802D1B4: ldr r0, [r4, #36]; 0x24
	.hword 0x00c0 @ 0x802D1B6: lsls r0, r0, #3
	.hword 0x9a04 @ 0x802D1B8: ldr r2, [sp, #16]
	.hword 0x7010 @ 0x802D1BA: strb r0, [r2, #0]
	.hword 0x7920 @ 0x802D1BC: ldrb r0, [r4, #4]
	.hword 0x3008 @ 0x802D1BE: adds r0, #8
	.hword 0x4680 @ 0x802D1C0: mov r8, r0
	.hword 0x7fa0 @ 0x802D1C2: ldrb r0, [r4, #30]
	.hword 0x2800 @ 0x802D1C4: cmp r0, #0
	.hword 0xd000 @ 0x802D1C6: beq.n 0x802d1ca
	.hword 0x2040 @ 0x802D1C8: movs r0, #64; 0x40
	.hword 0x76a0 @ 0x802D1CA: strb r0, [r4, #26]
	.hword 0x7921 @ 0x802D1CC: ldrb r1, [r4, #4]
	.hword 0x2200 @ 0x802D1CE: movs r2, #0
	.hword 0x72e1 @ 0x802D1D0: strb r1, [r4, #11]
	.hword 0x20ff @ 0x802D1D2: movs r0, #255; 0xff
	.hword 0x4008 @ 0x802D1D4: ands r0, r1
	.hword 0x1c71 @ 0x802D1D6: adds r1, r6, #1
	.hword 0x468a @ 0x802D1D8: mov sl, r1
	.hword 0x2140 @ 0x802D1DA: movs r1, #64; 0x40
	.hword 0x1909 @ 0x802D1DC: adds r1, r1, r4
	.hword 0x4689 @ 0x802D1DE: mov r9, r1
	.hword 0x2800 @ 0x802D1E0: cmp r0, #0
	.hword 0xd100 @ 0x802D1E2: bne.n 0x802d1e6
	.hword 0xe09d @ 0x802D1E4: b.n 0x802d322
	.hword 0x7262 @ 0x802D1E6: strb r2, [r4, #9]
	.hword 0xe0b2 @ 0x802D1E8: b.n 0x802d350
	.hword 0x2004 @ 0x802D1EA: movs r0, #4
	.hword 0x4010 @ 0x802D1EC: ands r0, r2
	.hword 0x2800 @ 0x802D1EE: cmp r0, #0
	.hword 0xd014 @ 0x802D1F0: beq.n 0x802d21c
	.hword 0x7b60 @ 0x802D1F2: ldrb r0, [r4, #13]
	.hword 0x3801 @ 0x802D1F4: subs r0, #1
	.hword 0x7360 @ 0x802D1F6: strb r0, [r4, #13]
	.hword 0x22ff @ 0x802D1F8: movs r2, #255; 0xff
	.hword 0x4010 @ 0x802D1FA: ands r0, r2
	.hword 0x0600 @ 0x802D1FC: lsls r0, r0, #24
	.hword 0x1c71 @ 0x802D1FE: adds r1, r6, #1
	.hword 0x468a @ 0x802D200: mov sl, r1
	.hword 0x2240 @ 0x802D202: movs r2, #64; 0x40
	.hword 0x1912 @ 0x802D204: adds r2, r2, r4
	.hword 0x4691 @ 0x802D206: mov r9, r2
	.hword 0x2800 @ 0x802D208: cmp r0, #0
	.hword 0xdd00 @ 0x802D20A: ble.n 0x802d20e
	.hword 0xe0a9 @ 0x802D20C: b.n 0x802d362
	.hword 0x0630 @ 0x802D20E: lsls r0, r6, #24
	.hword 0x0e00 @ 0x802D210: lsrs r0, r0, #24
	.hword 0xf7ff @ 0x802D212: bl 0x802cf7c
	.hword 0xfeb3 @ 0x802D212 +2: continuation
	.hword 0x2000 @ 0x802D216: movs r0, #0
	.hword 0x7020 @ 0x802D218: strb r0, [r4, #0]
	.hword 0xe121 @ 0x802D21A: b.n 0x802d460
	.hword 0x2040 @ 0x802D21C: movs r0, #64; 0x40
	.hword 0x4008 @ 0x802D21E: ands r0, r1
	.hword 0x1c72 @ 0x802D220: adds r2, r6, #1
	.hword 0x4692 @ 0x802D222: mov sl, r2
	.hword 0x2240 @ 0x802D224: movs r2, #64; 0x40
	.hword 0x1912 @ 0x802D226: adds r2, r2, r4
	.hword 0x4691 @ 0x802D228: mov r9, r2
	.hword 0x2800 @ 0x802D22A: cmp r0, #0
	.hword 0xd016 @ 0x802D22C: beq.n 0x802d25c
	.hword 0x2003 @ 0x802D22E: movs r0, #3
	.hword 0x4008 @ 0x802D230: ands r0, r1
	.hword 0x2800 @ 0x802D232: cmp r0, #0
	.hword 0xd012 @ 0x802D234: beq.n 0x802d25c
	.hword 0x20fc @ 0x802D236: movs r0, #252; 0xfc
	.hword 0x4008 @ 0x802D238: ands r0, r1
	.hword 0x2200 @ 0x802D23A: movs r2, #0
	.hword 0x7020 @ 0x802D23C: strb r0, [r4, #0]
	.hword 0x79e1 @ 0x802D23E: ldrb r1, [r4, #7]
	.hword 0x72e1 @ 0x802D240: strb r1, [r4, #11]
	.hword 0x20ff @ 0x802D242: movs r0, #255; 0xff
	.hword 0x4008 @ 0x802D244: ands r0, r1
	.hword 0x2800 @ 0x802D246: cmp r0, #0
	.hword 0xd021 @ 0x802D248: beq.n 0x802d28e
	.hword 0x2001 @ 0x802D24A: movs r0, #1
	.hword 0x7f61 @ 0x802D24C: ldrb r1, [r4, #29]
	.hword 0x4308 @ 0x802D24E: orrs r0, r1
	.hword 0x7760 @ 0x802D250: strb r0, [r4, #29]
	.hword 0x2e03 @ 0x802D252: cmp r6, #3
	.hword 0xd07c @ 0x802D254: beq.n 0x802d350
	.hword 0x79e2 @ 0x802D256: ldrb r2, [r4, #7]
	.hword 0x4690 @ 0x802D258: mov r8, r2
	.hword 0xe079 @ 0x802D25A: b.n 0x802d350
	.hword 0x7ae0 @ 0x802D25C: ldrb r0, [r4, #11]
	.hword 0x2800 @ 0x802D25E: cmp r0, #0
	.hword 0xd176 @ 0x802D260: bne.n 0x802d350
	.hword 0x2e03 @ 0x802D262: cmp r6, #3
	.hword 0xd103 @ 0x802D264: bne.n 0x802d26e
	.hword 0x2001 @ 0x802D266: movs r0, #1
	.hword 0x7f61 @ 0x802D268: ldrb r1, [r4, #29]
	.hword 0x4308 @ 0x802D26A: orrs r0, r1
	.hword 0x7760 @ 0x802D26C: strb r0, [r4, #29]
	.hword 0x1c20 @ 0x802D26E: adds r0, r4, #0
	.hword 0xf7ff @ 0x802D270: bl 0x802cfcc
	.hword 0xfeac @ 0x802D270 +2: continuation
	.hword 0x2003 @ 0x802D274: movs r0, #3
	.hword 0x7822 @ 0x802D276: ldrb r2, [r4, #0]
	.hword 0x4010 @ 0x802D278: ands r0, r2
	.hword 0x2800 @ 0x802D27A: cmp r0, #0
	.hword 0xd121 @ 0x802D27C: bne.n 0x802d2c2
	.hword 0x7a60 @ 0x802D27E: ldrb r0, [r4, #9]
	.hword 0x3801 @ 0x802D280: subs r0, #1
	.hword 0x7260 @ 0x802D282: strb r0, [r4, #9]
	.hword 0x21ff @ 0x802D284: movs r1, #255; 0xff
	.hword 0x4008 @ 0x802D286: ands r0, r1
	.hword 0x0600 @ 0x802D288: lsls r0, r0, #24
	.hword 0x2800 @ 0x802D28A: cmp r0, #0
	.hword 0xdc17 @ 0x802D28C: bgt.n 0x802d2be
	.hword 0x7b22 @ 0x802D28E: ldrb r2, [r4, #12]
	.hword 0x7aa1 @ 0x802D290: ldrb r1, [r4, #10]
	.hword 0x1c10 @ 0x802D292: adds r0, r2, #0
	.hword 0x4348 @ 0x802D294: muls r0, r1
	.hword 0x30ff @ 0x802D296: adds r0, #255; 0xff
	.hword 0x1200 @ 0x802D298: asrs r0, r0, #8
	.hword 0x2100 @ 0x802D29A: movs r1, #0
	.hword 0x7260 @ 0x802D29C: strb r0, [r4, #9]
	.hword 0x0600 @ 0x802D29E: lsls r0, r0, #24
	.hword 0x2800 @ 0x802D2A0: cmp r0, #0
	.hword 0xd0b4 @ 0x802D2A2: beq.n 0x802d20e
	.hword 0x2004 @ 0x802D2A4: movs r0, #4
	.hword 0x7822 @ 0x802D2A6: ldrb r2, [r4, #0]
	.hword 0x4310 @ 0x802D2A8: orrs r0, r2
	.hword 0x7020 @ 0x802D2AA: strb r0, [r4, #0]
	.hword 0x2001 @ 0x802D2AC: movs r0, #1
	.hword 0x7f61 @ 0x802D2AE: ldrb r1, [r4, #29]
	.hword 0x4308 @ 0x802D2B0: orrs r0, r1
	.hword 0x7760 @ 0x802D2B2: strb r0, [r4, #29]
	.hword 0x2e03 @ 0x802D2B4: cmp r6, #3
	.hword 0xd054 @ 0x802D2B6: beq.n 0x802d362
	.hword 0x2208 @ 0x802D2B8: movs r2, #8
	.hword 0x4690 @ 0x802D2BA: mov r8, r2
	.hword 0xe051 @ 0x802D2BC: b.n 0x802d362
	.hword 0x79e0 @ 0x802D2BE: ldrb r0, [r4, #7]
	.hword 0xe045 @ 0x802D2C0: b.n 0x802d34e
	.hword 0x2801 @ 0x802D2C2: cmp r0, #1
	.hword 0xd103 @ 0x802D2C4: bne.n 0x802d2ce
	.hword 0x7e60 @ 0x802D2C6: ldrb r0, [r4, #25]
	.hword 0x7260 @ 0x802D2C8: strb r0, [r4, #9]
	.hword 0x2007 @ 0x802D2CA: movs r0, #7
	.hword 0xe03f @ 0x802D2CC: b.n 0x802d34e
	.hword 0x2802 @ 0x802D2CE: cmp r0, #2
	.hword 0xd11f @ 0x802D2D0: bne.n 0x802d312
	.hword 0x7a60 @ 0x802D2D2: ldrb r0, [r4, #9]
	.hword 0x3801 @ 0x802D2D4: subs r0, #1
	.hword 0x7260 @ 0x802D2D6: strb r0, [r4, #9]
	.hword 0x21ff @ 0x802D2D8: movs r1, #255; 0xff
	.hword 0x4008 @ 0x802D2DA: ands r0, r1
	.hword 0x0600 @ 0x802D2DC: lsls r0, r0, #24
	.hword 0x7e62 @ 0x802D2DE: ldrb r2, [r4, #25]
	.hword 0x0611 @ 0x802D2E0: lsls r1, r2, #24
	.hword 0x4288 @ 0x802D2E2: cmp r0, r1
	.hword 0xdc13 @ 0x802D2E4: bgt.n 0x802d30e
	.hword 0x79a0 @ 0x802D2E6: ldrb r0, [r4, #6]
	.hword 0x2800 @ 0x802D2E8: cmp r0, #0
	.hword 0xd104 @ 0x802D2EA: bne.n 0x802d2f6
	.hword 0x20fc @ 0x802D2EC: movs r0, #252; 0xfc
	.hword 0x7821 @ 0x802D2EE: ldrb r1, [r4, #0]
	.hword 0x4008 @ 0x802D2F0: ands r0, r1
	.hword 0x7020 @ 0x802D2F2: strb r0, [r4, #0]
	.hword 0xe7cb @ 0x802D2F4: b.n 0x802d28e
	.hword 0x7820 @ 0x802D2F6: ldrb r0, [r4, #0]
	.hword 0x3801 @ 0x802D2F8: subs r0, #1
	.hword 0x7020 @ 0x802D2FA: strb r0, [r4, #0]
	.hword 0x2001 @ 0x802D2FC: movs r0, #1
	.hword 0x7f62 @ 0x802D2FE: ldrb r2, [r4, #29]
	.hword 0x4310 @ 0x802D300: orrs r0, r2
	.hword 0x7760 @ 0x802D302: strb r0, [r4, #29]
	.hword 0x2e03 @ 0x802D304: cmp r6, #3
	.hword 0xd0de @ 0x802D306: beq.n 0x802d2c6
	.hword 0x2008 @ 0x802D308: movs r0, #8
	.hword 0x4680 @ 0x802D30A: mov r8, r0
	.hword 0xe7db @ 0x802D30C: b.n 0x802d2c6
	.hword 0x7960 @ 0x802D30E: ldrb r0, [r4, #5]
	.hword 0xe01d @ 0x802D310: b.n 0x802d34e
	.hword 0x7a60 @ 0x802D312: ldrb r0, [r4, #9]
	.hword 0x3001 @ 0x802D314: adds r0, #1
	.hword 0x7260 @ 0x802D316: strb r0, [r4, #9]
	.hword 0x21ff @ 0x802D318: movs r1, #255; 0xff
	.hword 0x4008 @ 0x802D31A: ands r0, r1
	.hword 0x7aa2 @ 0x802D31C: ldrb r2, [r4, #10]
	.hword 0x4290 @ 0x802D31E: cmp r0, r2
	.hword 0xd314 @ 0x802D320: bcc.n 0x802d34c
	.hword 0x7820 @ 0x802D322: ldrb r0, [r4, #0]
	.hword 0x3801 @ 0x802D324: subs r0, #1
	.hword 0x2200 @ 0x802D326: movs r2, #0
	.hword 0x7020 @ 0x802D328: strb r0, [r4, #0]
	.hword 0x7961 @ 0x802D32A: ldrb r1, [r4, #5]
	.hword 0x72e1 @ 0x802D32C: strb r1, [r4, #11]
	.hword 0x20ff @ 0x802D32E: movs r0, #255; 0xff
	.hword 0x4008 @ 0x802D330: ands r0, r1
	.hword 0x2800 @ 0x802D332: cmp r0, #0
	.hword 0xd0d7 @ 0x802D334: beq.n 0x802d2e6
	.hword 0x2001 @ 0x802D336: movs r0, #1
	.hword 0x7f61 @ 0x802D338: ldrb r1, [r4, #29]
	.hword 0x4308 @ 0x802D33A: orrs r0, r1
	.hword 0x7760 @ 0x802D33C: strb r0, [r4, #29]
	.hword 0x7aa0 @ 0x802D33E: ldrb r0, [r4, #10]
	.hword 0x7260 @ 0x802D340: strb r0, [r4, #9]
	.hword 0x2e03 @ 0x802D342: cmp r6, #3
	.hword 0xd004 @ 0x802D344: beq.n 0x802d350
	.hword 0x7962 @ 0x802D346: ldrb r2, [r4, #5]
	.hword 0x4690 @ 0x802D348: mov r8, r2
	.hword 0xe001 @ 0x802D34A: b.n 0x802d350
	.hword 0x7920 @ 0x802D34C: ldrb r0, [r4, #4]
	.hword 0x72e0 @ 0x802D34E: strb r0, [r4, #11]
	.hword 0x7ae0 @ 0x802D350: ldrb r0, [r4, #11]
	.hword 0x3801 @ 0x802D352: subs r0, #1
	.hword 0x72e0 @ 0x802D354: strb r0, [r4, #11]
	.hword 0x9800 @ 0x802D356: ldr r0, [sp, #0]
	.hword 0x2800 @ 0x802D358: cmp r0, #0
	.hword 0xd102 @ 0x802D35A: bne.n 0x802d362
	.hword 0x3801 @ 0x802D35C: subs r0, #1
	.hword 0x9000 @ 0x802D35E: str r0, [sp, #0]
	.hword 0xe77c @ 0x802D360: b.n 0x802d25c
	.hword 0x2002 @ 0x802D362: movs r0, #2
	.hword 0x7f61 @ 0x802D364: ldrb r1, [r4, #29]
	.hword 0x4008 @ 0x802D366: ands r0, r1
	.hword 0x2800 @ 0x802D368: cmp r0, #0
	.hword 0xd036 @ 0x802D36A: beq.n 0x802d3da
	.hword 0x2e03 @ 0x802D36C: cmp r6, #3
	.hword 0xdc18 @ 0x802D36E: bgt.n 0x802d3a2
	.hword 0x2008 @ 0x802D370: movs r0, #8
	.hword 0x7862 @ 0x802D372: ldrb r2, [r4, #1]
	.hword 0x4010 @ 0x802D374: ands r0, r2
	.hword 0x2800 @ 0x802D376: cmp r0, #0
	.hword 0xd013 @ 0x802D378: beq.n 0x802d3a2
	.hword 0x4804 @ 0x802D37A: ldr r0, [pc, #16]; (0x802d38c)
	.hword 0x7800 @ 0x802D37C: ldrb r0, [r0, #0]
	.hword 0x283f @ 0x802D37E: cmp r0, #63; 0x3f
	.hword 0xdc08 @ 0x802D380: bgt.n 0x802d394
	.hword 0x6a20 @ 0x802D382: ldr r0, [r4, #32]
	.hword 0x3002 @ 0x802D384: adds r0, #2
	.hword 0x4902 @ 0x802D386: ldr r1, [pc, #8]; (0x802d390)
	.hword 0xe009 @ 0x802D388: b.n 0x802d39e
	.short 0 @ pad 0x802D38A
_08038C: .word 0x04000089 @ pool
_080390: .word 0x000007fc @ pool
	.hword 0x287f @ 0x802D394: cmp r0, #127; 0x7f
	.hword 0xdc04 @ 0x802D396: bgt.n 0x802d3a2
	.hword 0x6a20 @ 0x802D398: ldr r0, [r4, #32]
	.hword 0x3001 @ 0x802D39A: adds r0, #1
	.hword 0x4904 @ 0x802D39C: ldr r1, [pc, #16]; (0x802d3b0)
	.hword 0x4008 @ 0x802D39E: ands r0, r1
	.hword 0x6220 @ 0x802D3A0: str r0, [r4, #32]
	.hword 0x2e04 @ 0x802D3A2: cmp r6, #4
	.hword 0xd006 @ 0x802D3A4: beq.n 0x802d3b4
	.hword 0x6a20 @ 0x802D3A6: ldr r0, [r4, #32]
	.hword 0x9904 @ 0x802D3A8: ldr r1, [sp, #16]
	.hword 0x7008 @ 0x802D3AA: strb r0, [r1, #0]
	.hword 0xe009 @ 0x802D3AC: b.n 0x802d3c2
	.short 0 @ pad 0x802D3AE
_0803B0: .word 0x000007fe @ pool
	.hword 0x9a04 @ 0x802D3B4: ldr r2, [sp, #16]
	.hword 0x7810 @ 0x802D3B6: ldrb r0, [r2, #0]
	.hword 0x2108 @ 0x802D3B8: movs r1, #8
	.hword 0x4001 @ 0x802D3BA: ands r1, r0
	.hword 0x6a20 @ 0x802D3BC: ldr r0, [r4, #32]
	.hword 0x4308 @ 0x802D3BE: orrs r0, r1
	.hword 0x7010 @ 0x802D3C0: strb r0, [r2, #0]
	.hword 0x20c0 @ 0x802D3C2: movs r0, #192; 0xc0
	.hword 0x7ea1 @ 0x802D3C4: ldrb r1, [r4, #26]
	.hword 0x4008 @ 0x802D3C6: ands r0, r1
	.hword 0x1c21 @ 0x802D3C8: adds r1, r4, #0
	.hword 0x3121 @ 0x802D3CA: adds r1, #33; 0x21
	.hword 0x7809 @ 0x802D3CC: ldrb r1, [r1, #0]
	.hword 0x1808 @ 0x802D3CE: adds r0, r1, r0
	.hword 0x76a0 @ 0x802D3D0: strb r0, [r4, #26]
	.hword 0x22ff @ 0x802D3D2: movs r2, #255; 0xff
	.hword 0x4010 @ 0x802D3D4: ands r0, r2
	.hword 0x9905 @ 0x802D3D6: ldr r1, [sp, #20]
	.hword 0x7008 @ 0x802D3D8: strb r0, [r1, #0]
	.hword 0x2001 @ 0x802D3DA: movs r0, #1
	.hword 0x7f62 @ 0x802D3DC: ldrb r2, [r4, #29]
	.hword 0x4010 @ 0x802D3DE: ands r0, r2
	.hword 0x2800 @ 0x802D3E0: cmp r0, #0
	.hword 0xd03d @ 0x802D3E2: beq.n 0x802d460
	.hword 0x490f @ 0x802D3E4: ldr r1, [pc, #60]; (0x802d424)
	.hword 0x7808 @ 0x802D3E6: ldrb r0, [r1, #0]
	.hword 0x7f22 @ 0x802D3E8: ldrb r2, [r4, #28]
	.hword 0x4390 @ 0x802D3EA: bics r0, r2
	.hword 0x7ee2 @ 0x802D3EC: ldrb r2, [r4, #27]
	.hword 0x4310 @ 0x802D3EE: orrs r0, r2
	.hword 0x7008 @ 0x802D3F0: strb r0, [r1, #0]
	.hword 0x2e03 @ 0x802D3F2: cmp r6, #3
	.hword 0xd11a @ 0x802D3F4: bne.n 0x802d42c
	.hword 0x480c @ 0x802D3F6: ldr r0, [pc, #48]; (0x802d428)
	.hword 0x7a61 @ 0x802D3F8: ldrb r1, [r4, #9]
	.hword 0x1808 @ 0x802D3FA: adds r0, r1, r0
	.hword 0x7800 @ 0x802D3FC: ldrb r0, [r0, #0]
	.hword 0x9a03 @ 0x802D3FE: ldr r2, [sp, #12]
	.hword 0x7010 @ 0x802D400: strb r0, [r2, #0]
	.hword 0x2180 @ 0x802D402: movs r1, #128; 0x80
	.hword 0x1c08 @ 0x802D404: adds r0, r1, #0
	.hword 0x7ea2 @ 0x802D406: ldrb r2, [r4, #26]
	.hword 0x4010 @ 0x802D408: ands r0, r2
	.hword 0x2800 @ 0x802D40A: cmp r0, #0
	.hword 0xd028 @ 0x802D40C: beq.n 0x802d460
	.hword 0x9802 @ 0x802D40E: ldr r0, [sp, #8]
	.hword 0x7001 @ 0x802D410: strb r1, [r0, #0]
	.hword 0x7ea0 @ 0x802D412: ldrb r0, [r4, #26]
	.hword 0x9905 @ 0x802D414: ldr r1, [sp, #20]
	.hword 0x7008 @ 0x802D416: strb r0, [r1, #0]
	.hword 0x207f @ 0x802D418: movs r0, #127; 0x7f
	.hword 0x7ea2 @ 0x802D41A: ldrb r2, [r4, #26]
	.hword 0x4010 @ 0x802D41C: ands r0, r2
	.hword 0x76a0 @ 0x802D41E: strb r0, [r4, #26]
	.hword 0xe01e @ 0x802D420: b.n 0x802d460
	.short 0 @ pad 0x802D422
_080424: .word 0x04000081 @ pool
_080428: .word 0x08061744 @ pool
	.hword 0x200f @ 0x802D42C: movs r0, #15
	.hword 0x4641 @ 0x802D42E: mov r1, r8
	.hword 0x4001 @ 0x802D430: ands r1, r0
	.hword 0x4688 @ 0x802D432: mov r8, r1
	.hword 0x7a62 @ 0x802D434: ldrb r2, [r4, #9]
	.hword 0x0110 @ 0x802D436: lsls r0, r2, #4
	.hword 0x4440 @ 0x802D438: add r0, r8
	.hword 0x9903 @ 0x802D43A: ldr r1, [sp, #12]
	.hword 0x7008 @ 0x802D43C: strb r0, [r1, #0]
	.hword 0x2280 @ 0x802D43E: movs r2, #128; 0x80
	.hword 0x7ea0 @ 0x802D440: ldrb r0, [r4, #26]
	.hword 0x4310 @ 0x802D442: orrs r0, r2
	.hword 0x9905 @ 0x802D444: ldr r1, [sp, #20]
	.hword 0x7008 @ 0x802D446: strb r0, [r1, #0]
	.hword 0x2e01 @ 0x802D448: cmp r6, #1
	.hword 0xd109 @ 0x802D44A: bne.n 0x802d460
	.hword 0x9802 @ 0x802D44C: ldr r0, [sp, #8]
	.hword 0x7801 @ 0x802D44E: ldrb r1, [r0, #0]
	.hword 0x2008 @ 0x802D450: movs r0, #8
	.hword 0x4008 @ 0x802D452: ands r0, r1
	.hword 0x2800 @ 0x802D454: cmp r0, #0
	.hword 0xd103 @ 0x802D456: bne.n 0x802d460
	.hword 0x7ea0 @ 0x802D458: ldrb r0, [r4, #26]
	.hword 0x4310 @ 0x802D45A: orrs r0, r2
	.hword 0x9905 @ 0x802D45C: ldr r1, [sp, #20]
	.hword 0x7008 @ 0x802D45E: strb r0, [r1, #0]
	.hword 0x2000 @ 0x802D460: movs r0, #0
	.hword 0x7760 @ 0x802D462: strb r0, [r4, #29]
	.hword 0x4656 @ 0x802D464: mov r6, sl
	.hword 0x464c @ 0x802D466: mov r4, r9
	.hword 0x2e04 @ 0x802D468: cmp r6, #4
	.hword 0xdc00 @ 0x802D46A: bgt.n 0x802d46e
	.hword 0xe5fa @ 0x802D46C: b.n 0x802d064
	.hword 0xb007 @ 0x802D46E: add sp, #28
	.hword 0xbc38 @ 0x802D470: pop {r3, r4, r5}
	.hword 0x4698 @ 0x802D472: mov r8, r3
	.hword 0x46a1 @ 0x802D474: mov r9, r4
	.hword 0x46aa @ 0x802D476: mov sl, r5
	.hword 0xbcf0 @ 0x802D478: pop {r4, r5, r6, r7}
	.hword 0xbc01 @ 0x802D47A: pop {r0}
	.hword 0x4700 @ 0x802D47C: bx r0
	.short 0 @ pad 0x802D47E
