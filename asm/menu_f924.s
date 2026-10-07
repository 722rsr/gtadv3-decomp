@ GT Advance 3 - menu record helpers X
@ Region: file offset 0x00F924-0x00FA24 (VMA 0x0800F924-0x0800FA24).
@ Pure Thumb, ARMCC; continuation of the menu span (menu_f8b4.s ends at
@ 0xF924; next raw resumes at 0xFA24).
@
@ Function map:
@   sub_0800F924 - record selector: clears rec+0xD8, when
@                  s16[0x03001780+0xFBC]==3 clears rec+0x58 and
@                  (ldrb 0x03001780+0x10C3==0) -> 0x080056F4(rec,1,1)
@                  with rec+0xD8=1; then sub_08004B68[+2] selects
@                  rec+0x54 = 5 (==35) / 1 (39..40) / 6 (default).
@   sub_0800F9A0 - result-scatter: writes rec+0xAC/0xC0/0xBC+0xBE
@                  (via sub_0800F700)/0xB0/0xC8/0xC6/0xDC fields into
@                  the 0x03001780+0x576/0xFE4/0xFE6/0xFEA cell set,
@                  then when rec+0xDC==1 -> sub_0800F7C0.
@
@ Transcribed from baserom.gba via objdump; byte-exact (make SHA gate).
@ Unconverted targets keep numeric `bl 0x0800XXXX` form. Pools at
@ original offsets.

.thumb

@ ----------------------------------------------------------------------------
.type sub_0800F924, %function
sub_0800F924:
_0800F924:
  push {r4, r5, lr}
  adds r4, r1, #0
  adds r5, r0, #0
  adds r5, #216            @ 0xd8
  movs r0, #0
  strh r0, [r5, #0]
  ldr r2, _0800F970
  ldr r1, _0800F974
  adds r0, r2, r1
  ldrh r0, [r0, #0]
  cmp r0, #3
  bne _0800F95C
  adds r1, r4, #0
  adds r1, #88             @ 0x58
  movs r0, #0
  strb r0, [r1, #0]
  ldr r1, _0800F978
  adds r0, r2, r1
  ldrb r0, [r0, #0]
  cmp r0, #0
  bne _0800F95C
  adds r0, r4, #0
  movs r1, #1
  movs r2, #1
  bl 0x080056F4
  movs r0, #1
  strh r0, [r5, #0]
_0800F95C:
  bl sub_08004B68
  movs r1, #2
  ldrsh r0, [r0, r1]
  cmp r0, #35             @ 0x23
  bne _0800F97C
  adds r1, r4, #0
  adds r1, #84             @ 0x54
  movs r0, #5
  b _0800F996
  .align 2, 0
_0800F970: .4byte 0x03001780
_0800F974: .4byte 0x00000FBC
_0800F978: .4byte 0x000010C3
_0800F97C:
  cmp r0, #35             @ 0x23
  blt _0800F990
  cmp r0, #40             @ 0x28
  bgt _0800F990
  cmp r0, #39             @ 0x27
  blt _0800F990
  adds r1, r4, #0
  adds r1, #84             @ 0x54
  movs r0, #1
  b _0800F996
_0800F990:
  adds r1, r4, #0
  adds r1, #84             @ 0x54
  movs r0, #6
_0800F996:
  strh r0, [r1, #0]
  pop {r4, r5}
  pop {r0}
  bx r0
  .2byte 0x0000            @ pad @ 0xF99E

@ ----------------------------------------------------------------------------
.type sub_0800F9A0, %function
sub_0800F9A0:
_0800F9A0:
  push {r4, r5, lr}
  adds r5, r0, #0
  ldr r4, _0800FA10
  adds r0, #172            @ 0xac
  ldrh r1, [r0, #0]
  ldr r2, _0800FA14
  adds r0, r4, r2
  strh r1, [r0, #0]
  adds r0, r5, #0
  adds r0, #192            @ 0xc0
  ldrh r1, [r0, #0]
  ldr r2, _0800FA18
  adds r0, r4, r2
  strh r1, [r0, #0]
  adds r0, r5, #0
  adds r0, #188            @ 0xbc
  movs r1, #0
  ldrsh r0, [r0, r1]
  adds r1, r5, #0
  adds r1, #190            @ 0xbe
  movs r2, #0
  ldrsh r1, [r1, r2]
  bl sub_0800F700
  ldr r2, _0800FA1C
  adds r1, r4, r2
  strh r0, [r1, #0]
  adds r0, r5, #0
  adds r0, #176            @ 0xb0
  ldrh r1, [r0, #0]
  adds r2, #6
  adds r0, r4, r2
  strh r1, [r0, #0]
  adds r0, r5, #0
  adds r0, #200            @ 0xc8
  ldrh r1, [r0, #0]
  subs r2, #4
  adds r0, r4, r2
  strh r1, [r0, #0]
  adds r0, r5, #0
  adds r0, #198            @ 0xc6
  ldrh r0, [r0, #0]
  ldr r1, _0800FA20
  adds r4, r4, r1
  strh r0, [r4, #0]
  adds r0, r5, #0
  adds r0, #220            @ 0xdc
  ldrh r0, [r0, #0]
  cmp r0, #1
  bne _0800FA0A
  adds r0, r5, #0
  bl sub_0800F7C0
_0800FA0A:
  pop {r4, r5}
  pop {r0}
  bx r0
  .align 2, 0
_0800FA10: .4byte 0x03001780
_0800FA14: .4byte 0x00000576
_0800FA18: .4byte 0x00000FE4
_0800FA1C: .4byte 0x00000FE6
_0800FA20: .4byte 0x00000FEA
