@ GT Advance 3 - menu record helpers VIII
@ Region: file offset 0x00F810-0x00F8B4 (VMA 0x0800F810-0x0800F8B4).
@ Pure Thumb, ARMCC; continuation of the menu span (menu_f6d0.s ends at
@ 0xF810; next raw resumes at 0xF8B4).
@
@ Function map:
@   sub_0800F810 - record field refresh: switches on s16(rec+0xA8);
@                  case 0: sub_080258B8(rec+0xAC) (calendar cup id)
@                  * 2 -> 0x080CB588 table value stored to rec+0x100,
@                  then sub_08007ABC(rec[+20], rec+0xFC);
@                  case 1: sub_0800F700(rec+0xBC, rec+0xBE) * 2 +
@                  0x080CB588+10 -> rec+0x100, then the same bind;
@                  case 2: s16(rec+0xC0) * 2 + 0x080CB588+20 ->
@                  rec+0x100, then the same bind.
@
@ Transcribed from baserom.gba via objdump; byte-exact (make SHA gate).
@ Unconverted targets keep numeric `bl 0x0800XXXX` form. Pools at
@ original offsets.

.thumb

@ ----------------------------------------------------------------------------
.type sub_0800F810, %function
sub_0800F810:
_0800F810:
  push {r4, lr}
  adds r4, r0, #0
  adds r0, #168            @ 0xa8
  movs r1, #0
  ldrsh r0, [r0, r1]
  cmp r0, #1
  beq _0800F84C
  cmp r0, #1
  bgt _0800F828
  cmp r0, #0
  beq _0800F82E
  b _0800F8AA
_0800F828:
  cmp r0, #2
  beq _0800F884
  b _0800F8AA
_0800F82E:
  adds r0, r4, #0
  adds r0, #172            @ 0xac
  movs r2, #0
  ldrsh r0, [r0, r2]
  bl _080258B8
  movs r3, #128            @ 0x80
  lsls r3, r3, #1
  adds r2, r4, r3
  ldr r1, _0800F848
  lsls r0, r0, #16
  asrs r0, r0, #15
  b _0800F86C
  .align 2, 0
_0800F848: .4byte 0x080CB588
_0800F84C:
  adds r0, r4, #0
  adds r0, #188            @ 0xbc
  movs r1, #0
  ldrsh r0, [r0, r1]
  adds r1, r4, #0
  adds r1, #190            @ 0xbe
  movs r2, #0
  ldrsh r1, [r1, r2]
  bl sub_0800F700
  movs r3, #128            @ 0x80
  lsls r3, r3, #1
  adds r2, r4, r3
  ldr r1, _0800F880
  lsls r0, r0, #1
  adds r1, #10
_0800F86C:
  adds r0, r0, r1
  ldrh r1, [r0, #0]
  str r1, [r2, #0]
  ldr r0, [r4, #20]
  adds r2, r4, #0
  adds r2, #252            @ 0xfc
  ldr r2, [r2, #0]
  bl sub_08007ABC
  b _0800F8AA
  .align 2, 0
_0800F880: .4byte 0x080CB588
_0800F884:
  movs r0, #128            @ 0x80
  lsls r0, r0, #1
  adds r2, r4, r0
  ldr r1, _0800F8B0
  adds r0, r4, #0
  adds r0, #192            @ 0xc0
  movs r3, #0
  ldrsh r0, [r0, r3]
  lsls r0, r0, #1
  adds r1, #20
  adds r0, r0, r1
  ldrh r1, [r0, #0]
  str r1, [r2, #0]
  ldr r0, [r4, #20]
  adds r2, r4, #0
  adds r2, #252            @ 0xfc
  ldr r2, [r2, #0]
  bl sub_08007ABC
_0800F8AA:
  pop {r4}
  pop {r0}
  bx r0
  .align 2, 0
_0800F8B0: .4byte 0x080CB588
