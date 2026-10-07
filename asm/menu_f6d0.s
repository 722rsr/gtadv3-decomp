@ GT Advance 3 - menu record helpers VII
@ Region: file offset 0x00F6D0-0x00F810 (VMA 0x0800F6D0-0x0800F810).
@ Pure Thumb, ARMCC; continuation of the menu span (menu_f5a0.s ends at
@ 0xF6D0; next raw resumes at 0xF810).
@
@ Function map:
@   sub_0800F6D0 - table cell read: copies the 80-byte template
@                  0x0805F940 to the stack via sub_0802E0A4, clamps
@                  r0 to 0..4, returns sp[clamp*16].
@   sub_0800F700 - table cell read: copies 0x0805F940, clamps r0 to
@                  0..4 and r1 to 0..2, returns sp[4 + r1*4 + r0*16].
@   sub_0800F744 - lane read: zero-fills 10 bytes at sp via
@                  sub_0802E104, writes u16 1 at sp+2, clamps r0 to
@                  0..4, returns s16(sp + r0*2).
@   sub_0800F778 - two-arg leaf: r2 != 0 ? bl 0x08003BC0 :
@                  bl 0x08003838 with r2 = 0x0805F990.
@   sub_0800F794 - record-new check: bl 0x08024BF0; when [ret] != 0
@                  and [ret+2] == rec+0xAC, sets rec+0x48 = 1.
@   sub_0800F7C0 - record-new check (inverted): sets rec+0x48 = 1
@                  when [ret] == 0 or [ret+2] != rec+0xAC.
@   sub_0800F7EC - award index lookup: sub_0800F700(s16 r0, s16 r1)
@                  -> s16(0x080CB55C + 10 + r0*2).
@
@ Transcribed from baserom.gba via objdump; byte-exact (make SHA gate).
@ Unconverted targets keep numeric `bl 0x0800XXXX` form. Pools at
@ original offsets.

.thumb

@ ----------------------------------------------------------------------------
.type sub_0800F6D0, %function
sub_0800F6D0:
_0800F6D0:
  push {r4, lr}
  sub sp, #80              @ 0x50
  adds r4, r0, #0
  ldr r1, _0800F6FC
  mov r0, sp
  movs r2, #80             @ 0x50
  bl sub_0802E0A4
  cmp r4, #0
  bgt _0800F6E6
  movs r4, #0
_0800F6E6:
  cmp r4, #3
  ble _0800F6EC
  movs r4, #4
_0800F6EC:
  lsls r0, r4, #4
  add r0, sp
  ldr r0, [r0, #0]
  add sp, #80              @ 0x50
  pop {r4}
  pop {r1}
  bx r1
  .align 2, 0
_0800F6FC: .4byte 0x0805F940

@ ----------------------------------------------------------------------------
.type sub_0800F700, %function
sub_0800F700:
_0800F700:
  push {r4, r5, lr}
  sub sp, #80              @ 0x50
  adds r5, r0, #0
  adds r4, r1, #0
  ldr r1, _0800F740
  mov r0, sp
  movs r2, #80             @ 0x50
  bl sub_0802E0A4
  cmp r5, #0
  bgt _0800F718
  movs r5, #0
_0800F718:
  cmp r5, #3
  ble _0800F71E
  movs r5, #4
_0800F71E:
  cmp r4, #0
  bgt _0800F724
  movs r4, #0
_0800F724:
  cmp r4, #1
  ble _0800F72A
  movs r4, #2
_0800F72A:
  lsls r1, r4, #2
  lsls r0, r5, #4
  adds r1, r1, r0
  add r0, sp, #4
  adds r0, r0, r1
  ldr r0, [r0, #0]
  add sp, #80              @ 0x50
  pop {r4, r5}
  pop {r1}
  bx r1
  .align 2, 0
_0800F740: .4byte 0x0805F940

@ ----------------------------------------------------------------------------
.type sub_0800F744, %function
sub_0800F744:
_0800F744:
  push {r4, r5, lr}
  sub sp, #12
  adds r5, r0, #0
  mov r4, sp
  mov r0, sp
  movs r1, #0
  movs r2, #10
  bl sub_0802E104
  movs r0, #1
  strh r0, [r4, #2]
  cmp r5, #0
  bgt _0800F760
  movs r5, #0
_0800F760:
  cmp r5, #3
  ble _0800F766
  movs r5, #4
_0800F766:
  lsls r0, r5, #1
  add r0, sp
  movs r1, #0
  ldrsh r0, [r0, r1]
  add sp, #12
  pop {r4, r5}
  pop {r1}
  bx r1
  .2byte 0x0000            @ pad @ 0xF776

@ ----------------------------------------------------------------------------
.type sub_0800F778, %function
sub_0800F778:
_0800F778:
  push {lr}
  cmp r2, #0
  beq _0800F784
  bl 0x08003BC0
  b _0800F78A
_0800F784:
  ldr r2, _0800F790
  bl 0x08003838
_0800F78A:
  pop {r0}
  bx r0
  .align 2, 0
_0800F790: .4byte 0x0805F990

@ ----------------------------------------------------------------------------
.type sub_0800F794, %function
sub_0800F794:
_0800F794:
  push {r4, lr}
  adds r4, r0, #0
  bl 0x08024BF0
  adds r1, r0, #0
  ldrh r0, [r1, #0]
  cmp r0, #0
  beq _0800F7B8
  adds r0, r4, #0
  adds r0, #172            @ 0xac
  ldrh r1, [r1, #2]
  ldrh r0, [r0, #0]
  cmp r1, r0
  bne _0800F7B8
  adds r1, r4, #0
  adds r1, #72             @ 0x48
  movs r0, #1
  strh r0, [r1, #0]
_0800F7B8:
  pop {r4}
  pop {r0}
  bx r0
  .2byte 0x0000            @ pad @ 0xF7BE

@ ----------------------------------------------------------------------------
.type sub_0800F7C0, %function
sub_0800F7C0:
_0800F7C0:
  push {r4, lr}
  adds r4, r0, #0
  bl 0x08024BF0
  adds r1, r0, #0
  ldrh r0, [r1, #0]
  cmp r0, #0
  beq _0800F7DC
  adds r0, r4, #0
  adds r0, #172            @ 0xac
  ldrh r1, [r1, #2]
  ldrh r0, [r0, #0]
  cmp r1, r0
  beq _0800F7E4
_0800F7DC:
  adds r1, r4, #0
  adds r1, #72             @ 0x48
  movs r0, #1
  strh r0, [r1, #0]
_0800F7E4:
  pop {r4}
  pop {r0}
  bx r0
  .2byte 0x0000            @ pad @ 0xF7EA

@ ----------------------------------------------------------------------------
.type sub_0800F7EC, %function
sub_0800F7EC:
_0800F7EC:
  push {r4, lr}
  ldr r4, _0800F80C
  lsls r0, r0, #16
  asrs r0, r0, #16
  lsls r1, r1, #16
  asrs r1, r1, #16
  bl sub_0800F700
  lsls r0, r0, #1
  adds r4, #10
  adds r0, r0, r4
  movs r1, #0
  ldrsh r0, [r0, r1]
  pop {r4}
  pop {r1}
  bx r1
  .align 2, 0
_0800F80C: .4byte 0x080CB55C
@ Region end (VMA 0x0800F810). Emits no bytes; gives the promotion screen an
@ end marker for the last function in this region instead of guessing.
menu_f6d0_end:
