@ GT Advance 3 - menu record helpers IX
@ Region: file offset 0x00F8B4-0x00F924 (VMA 0x0800F8B4-0x0800F924).
@ Pure Thumb, ARMCC; continuation of the menu span (menu_f810.s ends at
@ 0xF8B4; next raw resumes at 0xF924).
@
@ Function map:
@   sub_0800F8B4 - record line writer: copies the 6-byte template
@                  0x0805F99A to the stack, clamps u16 r1 to 0..2,
@                  sub_08007614(sp+s16 idx*2, 2, 6, 0x080CB588),
@                  writes 0x080CB57C[idx*4] to rec+0x148, then binds
@                  sub_08007ABC(rec[+36], rec+0x144).
@
@ Transcribed from baserom.gba via objdump; byte-exact (make SHA gate).
@ Unconverted targets keep numeric `bl 0x0800XXXX` form. Pools at
@ original offsets.

.thumb

@ ----------------------------------------------------------------------------
.type sub_0800F8B4, %function
sub_0800F8B4:
_0800F8B4:
  push {r4, r5, lr}
  sub sp, #8
  adds r5, r0, #0
  lsls r1, r1, #16
  lsrs r4, r1, #16
  ldr r1, _0800F918
  mov r0, sp
  movs r2, #6
  bl sub_0802E0A4
  lsls r0, r4, #16
  cmp r0, #0
  bgt _0800F8D0
  movs r4, #0
_0800F8D0:
  lsls r0, r4, #16
  asrs r0, r0, #16
  cmp r0, #1
  ble _0800F8DA
  movs r4, #2
_0800F8DA:
  ldr r0, _0800F91C
  lsls r4, r4, #16
  asrs r4, r4, #16
  lsls r1, r4, #1
  add r1, sp
  movs r3, #0
  ldrsh r2, [r1, r3]
  movs r1, #2
  movs r3, #6
  bl sub_08007614
  movs r0, #164            @ 0xa4
  lsls r0, r0, #1
  adds r2, r5, r0
  ldr r0, _0800F920
  lsls r4, r4, #2
  adds r4, r4, r0
  ldr r1, [r4, #0]
  str r1, [r2, #0]
  ldr r0, [r5, #36]        @ 0x24
  movs r3, #162            @ 0xa2
  lsls r3, r3, #1
  adds r2, r5, r3
  ldr r2, [r2, #0]
  bl sub_08007ABC
  add sp, #8
  pop {r4, r5}
  pop {r0}
  bx r0
  .align 2, 0
_0800F918: .4byte 0x0805F99A
_0800F91C: .4byte 0x082D7660
_0800F920: .4byte 0x080CB57C
