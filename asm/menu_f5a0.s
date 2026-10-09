@ GT Advance 3 - menu record handlers VI
@ Region: file offset 0x00F5A0-0x00F6D0 (VMA 0x0800F5A0-0x0800F6D0).
@ Pure Thumb, ARMCC; continuation of the menu span (menu_f22c.s ends at
@ 0xF5A0; next raw resumes at 0xF6D0).
@
@ Function map:
@   sub_0800F5A0 - ticker: when s16[0x03001780+0xFBC]==3, reads
@                  _08002140 (block B) and either clears rec+0x144
@                  (result==2) or increments rec+0x144, broadcasting
@                  event 21 (sub_08004D4C) past the 180-count cap.
@   sub_0800F5EC - record dispatch (12-entry jump table @0xF608,
@                  base pool 0x0800F608 @0xF604, arg-1 indexed):
@                  v1 -> sub_0800E7CC, v2 -> sub_0800E650,
@                  v5 -> {sub_0800D854(rec+0x28), sub_0800D8E4(rec+0x90),
@                        sub_0800EF54}, v6 -> sub_0800F5A0 then the
@                  rec+0xE4 switch {0 -> sub_0800EBD8, 2 -> sub_0800ECAC,
@                  1 -> sub_0800EE00} with u16 args, v7 ->
@                  sub_0800F22C, v12 -> sub_0800E7A0.
@
@ Transcribed from baserom.gba via objdump; byte-exact (make SHA gate).
@ Unconverted targets keep numeric `bl 0x0800XXXX` form. Pools at
@ original offsets.

.thumb

@ ----------------------------------------------------------------------------
.type sub_0800F5A0, %function
sub_0800F5A0:
_0800F5A0:
  push {r4, lr}
  adds r4, r0, #0
  ldr r0, _0800F5D4
  ldr r1, _0800F5D8
  adds r0, r0, r1
  ldrh r0, [r0, #0]
  cmp r0, #3
  bne _0800F5E6
  bl 0x08002140
  cmp r0, #2
  beq _0800F5DC
  movs r0, #162            @ 0xa2
  lsls r0, r0, #1
  adds r1, r4, r0
  ldr r0, [r1, #0]
  adds r0, #1
  str r0, [r1, #0]
  cmp r0, #180            @ 0xb4
  ble _0800F5E6
  movs r0, #21
  movs r1, #0
  movs r2, #0
  bl sub_08004D4C
  b _0800F5E6
  .align 2, 0
_0800F5D4: .4byte 0x03001780
_0800F5D8: .4byte 0x00000FBC
_0800F5DC:
  movs r0, #162            @ 0xa2
  lsls r0, r0, #1
  adds r1, r4, r0
  movs r0, #0
  str r0, [r1, #0]
_0800F5E6:
  pop {r4}
  pop {r0}
  bx r0

@ ----------------------------------------------------------------------------
.type sub_0800F5EC, %function
sub_0800F5EC:
_0800F5EC:
  push {r4, r5, r6, lr}
  adds r5, r1, #0
  adds r6, r2, #0
  adds r4, r3, #0
  subs r0, #1
  cmp r0, #11
  bhi _0800F6C8
  lsls r0, r0, #2
  ldr r1, _0800F604
  adds r0, r0, r1
  ldr r0, [r0, #0]
  mov pc, r0
  .align 2, 0
_0800F604: .4byte 0x0800F608   @ jump table base (points at first entry)
_0800F608: .4byte _0800F6BA
_0800F60C: .4byte _0800F638
_0800F610: .4byte _0800F6C8
_0800F614: .4byte _0800F6C8
_0800F618: .4byte _0800F642
_0800F61C: .4byte _0800F662
_0800F620: .4byte _0800F65A
_0800F624: .4byte _0800F6C8
_0800F628: .4byte _0800F6C8
_0800F62C: .4byte _0800F6C8
_0800F630: .4byte _0800F6C8
_0800F634: .4byte _0800F6C2
_0800F638:
  adds r0, r4, #0
  adds r1, r5, #0
  bl sub_0800E650
  b _0800F6C8
_0800F642:
  adds r0, r4, #0
  adds r0, #40             @ 0x28
  bl sub_0800D854
  adds r0, r4, #0
  adds r0, #144            @ 0x90
  bl sub_0800D8E4
  adds r0, r4, #0
  bl sub_0800EF54
  b _0800F6C8
_0800F65A:
  adds r0, r4, #0
  bl sub_0800F22C
  b _0800F6C8
_0800F662:
  adds r0, r4, #0
  bl sub_0800F5A0
  ldrh r0, [r4, #44]       @ 0x2c
  cmp r0, #0
  beq _0800F6C8
  adds r0, r4, #0
  adds r0, #228            @ 0xe4
  movs r1, #0
  ldrsh r0, [r0, r1]
  cmp r0, #1
  beq _0800F6AA
  cmp r0, #1
  bgt _0800F684
  cmp r0, #0
  beq _0800F68A
  b _0800F6C8
_0800F684:
  cmp r0, #2
  beq _0800F69A
  b _0800F6C8
_0800F68A:
  lsls r1, r5, #16
  lsrs r1, r1, #16
  lsls r2, r6, #16
  lsrs r2, r2, #16
  adds r0, r4, #0
  bl sub_0800EBD8
  b _0800F6C8
_0800F69A:
  lsls r1, r5, #16
  lsrs r1, r1, #16
  lsls r2, r6, #16
  lsrs r2, r2, #16
  adds r0, r4, #0
  bl sub_0800ECAC
  b _0800F6C8
_0800F6AA:
  lsls r1, r5, #16
  lsrs r1, r1, #16
  lsls r2, r6, #16
  lsrs r2, r2, #16
  adds r0, r4, #0
  bl sub_0800EE00
  b _0800F6C8
_0800F6BA:
  adds r0, r4, #0
  bl sub_0800E7CC
  b _0800F6C8
_0800F6C2:
  adds r0, r4, #0
  bl sub_0800E7A0
_0800F6C8:
  pop {r4, r5, r6}
  pop {r0}
  bx r0
  .2byte 0x0000            @ pad @ 0xF6CE
menu_f5a0_end:
