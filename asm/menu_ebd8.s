@ GT Advance 3 - menu record handlers III
@ Region: file offset 0x00EBD8-0x00F040 (VMA 0x0800EBD8-0x0800F040).
@ Pure Thumb, ARMCC; continuation of the menu span (menu_e650.s ends at
@ 0xEBD8; next raw resumes at 0xF040).
@
@ Function map:
@   sub_0800EBD8 - menu record tick (r8): on v==2 sets sound 4 + the
@                  rec+0x28/0x2C/0x48/0x44 field set; on v==1 gates
@                  sound 10 on sub_08002780==1 && rec+0xE0==3, else
@                  sound 1 + rec+0x2C=0/+0x94=1/+0x44=1 + sub_08004BFC
@                  (rec+0x30=8 when rec+0xE0==3) and a 6-check against
@                  s16[0x03001780+0xFBC] (rec+0x30=1 when ==0); v==0x40
@                  decrements rec+0xE0, v==0x80 increments it (clamped
@                  0..3); sound 2 when the clamped value changed.
@   sub_0800ECAC - twin of sub_0800EBD8 without the +0xFBC 6-check.
@   sub_0800ED68 - reset leaf: _08002618(1,0), rec+0x10A=0, sound 4,
@                  rec+0x28=10/+0x2C=0/+0x48=10/+0x44=0, rec+0x32=5.
@   sub_0800ED98 - select leaf: _08002618(1,0), rec+0x10A=0, sound 1,
@                  rec+0x2C=0/+0x94=1/+0x44=1, sub_08004BFC(rec+0xE0),
@                  rec+0x30=6, then _08002178(1,4)/(1,5) -> s16
@                  0x03001780+0xFC4 / +0xFC6.
@   sub_0800EE00 - broadcast tick (r8/r9): six _08002158 event writes
@                  (1=v, 0=v, 2=rec+0xE0, 3=rec+0xE8, 4=rec+0xFA,
@                  5=u16 ldrsb @ 0x03001780 + rec+0xFA*12 + 49,
@                  6=sub_08004B68 result), rec+0xE0/0xE8/0xEC steering
@                  on v==2/==1 (with _08002618(1,1) empty-lane vs
@                  _08002618(1,0) busy-lane setup, sound 1,
@                  sub_08004BFC, rec+0x30=8 when rec+0xE0==3),
@                  0x40/0x80 value-stepping (clamp 0..2), sound 2 on
@                  change.
@   sub_0800EF54 - state switch on s16(rec+0xEC): 1 -> if
@                  _08002178(0,3)==4 && _08002178(1,3)==4 then
@                  sub_0800ED98; 2 -> when ldrb 0x03001780+0x10C3==1
@                  and both _08002178(0,3)/(1,3)==5 then sub_0800ED68,
@                  then rec+0xE8=2 when _08002178(1,6) !=
@                  sub_08004B68[0]; else rec+0xE8=5 (+ re-check of
@                  the (0,3)/(1,3)==5 pair -> sub_0800ED68).
@
@ Transcribed from baserom.gba via objdump; byte-exact (make SHA gate).
@ Unconverted targets keep numeric `bl 0x0800XXXX` form. Pools at
@ original offsets.

.thumb

@ ----------------------------------------------------------------------------
.type sub_0800EBD8, %function
sub_0800EBD8:
_0800EBD8:
  push {r4, r5, r6, r7, lr}
  mov r7, r8
  push {r7}
  adds r5, r0, #0
  lsls r2, r2, #16
  lsrs r6, r2, #16
  adds r4, r5, #0
  adds r4, #224            @ 0xe0
  movs r1, #0
  ldrsh r0, [r4, r1]
  mov r8, r0
  cmp r6, #2
  bne _0800EC04
  movs r0, #4
  bl sub_0802B368
  movs r1, #10
  str r1, [r5, #40]        @ 0x28
  movs r0, #0
  strh r0, [r5, #44]       @ 0x2c
  str r1, [r5, #72]        @ 0x48
  str r0, [r5, #68]        @ 0x44
_0800EC04:
  cmp r6, #1
  bne _0800EC5E
  bl 0x08002780
  cmp r0, #1
  bne _0800EC1E
  ldrh r2, [r4, #0]
  cmp r2, #3
  bne _0800EC1E
  movs r0, #10
  bl sub_0802B368
  b _0800EC5E
_0800EC1E:
  movs r0, #1
  bl sub_0802B368
  movs r0, #0
  strh r0, [r5, #44]       @ 0x2c
  adds r0, r5, #0
  adds r0, #148            @ 0x94
  movs r7, #1
  strh r7, [r0, #0]
  str r7, [r5, #68]        @ 0x44
  adds r4, r5, #0
  adds r4, #224            @ 0xe0
  movs r1, #0
  ldrsh r0, [r4, r1]
  bl 0x08004BFC
  ldrh r2, [r4, #0]
  cmp r2, #3
  bne _0800EC48
  movs r0, #8
  strh r0, [r5, #48]       @ 0x30
_0800EC48:
  ldr r0, _0800ECA4
  ldr r1, _0800ECA8
  adds r0, r0, r1
  ldrh r0, [r0, #0]
  cmp r0, #6
  bne _0800EC5E
  movs r2, #0
  ldrsh r0, [r4, r2]
  cmp r0, #0
  bne _0800EC5E
  strh r7, [r5, #48]       @ 0x30
_0800EC5E:
  cmp r6, #64              @ 0x40
  bne _0800EC68
  ldrh r0, [r4, #0]
  subs r0, #1
  strh r0, [r4, #0]
_0800EC68:
  cmp r6, #128             @ 0x80
  bne _0800EC72
  ldrh r0, [r4, #0]
  adds r0, #1
  strh r0, [r4, #0]
_0800EC72:
  adds r1, r4, #0
  movs r2, #0
  ldrsh r0, [r1, r2]
  cmp r0, #0
  bgt _0800EC80
  movs r0, #0
  strh r0, [r1, #0]
_0800EC80:
  movs r2, #0
  ldrsh r0, [r1, r2]
  cmp r0, #2
  ble _0800EC8C
  movs r0, #3
  strh r0, [r1, #0]
_0800EC8C:
  movs r1, #0
  ldrsh r0, [r4, r1]
  cmp r8, r0
  beq _0800EC9A
  movs r0, #2
  bl sub_0802B368
_0800EC9A:
  pop {r3}
  mov r8, r3
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .align 2, 0
_0800ECA4: .4byte 0x03001780
_0800ECA8: .4byte 0x00000FBC

@ ----------------------------------------------------------------------------
.type sub_0800ECAC, %function
sub_0800ECAC:
_0800ECAC:
  push {r4, r5, r6, r7, lr}
  adds r4, r0, #0
  lsls r2, r2, #16
  lsrs r6, r2, #16
  adds r5, r4, #0
  adds r5, #224            @ 0xe0
  movs r0, #0
  ldrsh r7, [r5, r0]
  cmp r6, #2
  bne _0800ECD2
  movs r0, #4
  bl sub_0802B368
  movs r1, #10
  str r1, [r4, #40]        @ 0x28
  movs r0, #0
  strh r0, [r4, #44]       @ 0x2c
  str r1, [r4, #72]        @ 0x48
  str r0, [r4, #68]        @ 0x44
_0800ECD2:
  cmp r6, #1
  bne _0800ED26
  bl 0x08002780
  cmp r0, #1
  bne _0800ECEC
  ldrh r1, [r5, #0]
  cmp r1, #3
  bne _0800ECEC
  movs r0, #10
  bl sub_0802B368
  b _0800ED26
_0800ECEC:
  adds r0, r4, #0
  adds r0, #224            @ 0xe0
  adds r5, r0, #0
  ldrh r2, [r5, #0]
  cmp r2, #2
  beq _0800ED20
  movs r0, #1
  bl sub_0802B368
  movs r0, #0
  strh r0, [r4, #44]       @ 0x2c
  adds r1, r4, #0
  adds r1, #148            @ 0x94
  movs r0, #1
  strh r0, [r1, #0]
  str r0, [r4, #68]        @ 0x44
  movs r1, #0
  ldrsh r0, [r5, r1]
  bl 0x08004BFC
  ldrh r2, [r5, #0]
  cmp r2, #3
  bne _0800ED26
  movs r0, #8
  strh r0, [r4, #48]       @ 0x30
  b _0800ED26
_0800ED20:
  movs r0, #10
  bl sub_0802B368
_0800ED26:
  cmp r6, #64              @ 0x40
  bne _0800ED30
  ldrh r0, [r5, #0]
  subs r0, #1
  strh r0, [r5, #0]
_0800ED30:
  cmp r6, #128             @ 0x80
  bne _0800ED3A
  ldrh r0, [r5, #0]
  adds r0, #1
  strh r0, [r5, #0]
_0800ED3A:
  adds r1, r5, #0
  movs r2, #0
  ldrsh r0, [r1, r2]
  cmp r0, #0
  bgt _0800ED48
  movs r0, #0
  strh r0, [r1, #0]
_0800ED48:
  movs r2, #0
  ldrsh r0, [r1, r2]
  cmp r0, #2
  ble _0800ED54
  movs r0, #3
  strh r0, [r1, #0]
_0800ED54:
  movs r1, #0
  ldrsh r0, [r5, r1]
  cmp r7, r0
  beq _0800ED62
  movs r0, #2
  bl sub_0802B368
_0800ED62:
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0

@ ----------------------------------------------------------------------------
.type sub_0800ED68, %function
sub_0800ED68:
_0800ED68:
  push {r4, r5, lr}
  adds r5, r0, #0
  movs r0, #1
  movs r1, #0
  bl 0x08002618
  movs r1, #133            @ 0x85
  lsls r1, r1, #1
  adds r0, r5, r1
  movs r4, #0
  strh r4, [r0, #0]
  movs r0, #4
  bl sub_0802B368
  movs r0, #10
  str r0, [r5, #40]        @ 0x28
  strh r4, [r5, #44]       @ 0x2c
  str r0, [r5, #72]        @ 0x48
  str r4, [r5, #68]        @ 0x44
  movs r0, #5
  strh r0, [r5, #50]       @ 0x32
  pop {r4, r5}
  pop {r0}
  bx r0

@ ----------------------------------------------------------------------------
.type sub_0800ED98, %function
sub_0800ED98:
_0800ED98:
  push {r4, r5, lr}
  adds r5, r0, #0
  movs r0, #1
  movs r1, #0
  bl 0x08002618
  movs r1, #133            @ 0x85
  lsls r1, r1, #1
  adds r0, r5, r1
  movs r4, #0
  strh r4, [r0, #0]
  movs r0, #1
  bl sub_0802B368
  strh r4, [r5, #44]       @ 0x2c
  adds r1, r5, #0
  adds r1, #148            @ 0x94
  movs r0, #1
  strh r0, [r1, #0]
  str r0, [r5, #68]        @ 0x44
  adds r0, r5, #0
  adds r0, #224            @ 0xe0
  movs r2, #0
  ldrsh r0, [r0, r2]
  bl 0x08004BFC
  movs r0, #6
  strh r0, [r5, #48]       @ 0x30
  movs r0, #1
  movs r1, #4
  bl 0x08002178
  ldr r4, _0800EDF4
  ldr r2, _0800EDF8
  adds r1, r4, r2
  strh r0, [r1, #0]
  movs r0, #1
  movs r1, #5
  bl 0x08002178
  ldr r1, _0800EDFC
  adds r4, r4, r1
  strh r0, [r4, #0]
  pop {r4, r5}
  pop {r0}
  bx r0
  .align 2, 0
_0800EDF4: .4byte 0x03001780
_0800EDF8: .4byte 0x00000FC4
_0800EDFC: .4byte 0x00000FC6

@ ----------------------------------------------------------------------------
.type sub_0800EE00, %function
sub_0800EE00:
_0800EE00:
  push {r4, r5, r6, r7, lr}
  mov r7, r9
  mov r6, r8
  push {r6, r7}
  adds r5, r0, #0
  lsls r2, r2, #16
  lsrs r7, r2, #16
  adds r4, r5, #0
  adds r4, #224            @ 0xe0
  movs r1, #0
  ldrsh r0, [r4, r1]
  mov r9, r0
  movs r2, #236            @ 0xec
  adds r2, r2, r5
  mov r8, r2
  ldrh r1, [r2, #0]
  movs r0, #1
  bl 0x08002158
  movs r0, #0
  adds r1, r7, #0
  bl 0x08002158
  ldrh r1, [r4, #0]
  movs r0, #2
  bl 0x08002158
  adds r6, r5, #0
  adds r6, #232            @ 0xe8
  ldrh r1, [r6, #0]
  movs r0, #3
  bl 0x08002158
  adds r4, #26
  ldrh r1, [r4, #0]
  movs r0, #4
  bl 0x08002158
  ldr r2, _0800EEBC
  movs r0, #0
  ldrsh r1, [r4, r0]
  lsls r0, r1, #1
  adds r0, r0, r1
  lsls r0, r0, #2
  adds r0, r0, r2
  adds r0, #49             @ 0x31
  movs r1, #0
  ldrsb r1, [r0, r1]
  lsls r1, r1, #16
  lsrs r1, r1, #16
  movs r0, #5
  bl 0x08002158
  bl sub_08004B68
  ldrh r1, [r0, #0]
  movs r0, #6
  bl 0x08002158
  cmp r7, #2
  bne _0800EE88
  ldrh r1, [r6, #0]
  cmp r1, #4
  beq _0800EE88
  movs r0, #5
  strh r0, [r6, #0]
  mov r2, r8
  strh r7, [r2, #0]
_0800EE88:
  adds r6, r5, #0
  adds r6, #224            @ 0xe0
  movs r0, #236            @ 0xec
  adds r0, r0, r5
  mov r8, r0
  cmp r7, #1
  bne _0800EEF8
  movs r1, #0
  ldrsh r0, [r6, r1]
  cmp r0, #0
  bne _0800EEC0
  movs r0, #1
  movs r1, #1
  bl 0x08002618
  movs r2, #133            @ 0x85
  lsls r2, r2, #1
  adds r0, r5, r2
  strh r7, [r0, #0]
  adds r1, r5, #0
  adds r1, #232            @ 0xe8
  movs r0, #4
  strh r0, [r1, #0]
  mov r0, r8
  strh r7, [r0, #0]
  b _0800EEF8
  .align 2, 0
_0800EEBC: .4byte 0x03001780
_0800EEC0:
  movs r0, #1
  movs r1, #0
  bl 0x08002618
  movs r1, #133            @ 0x85
  lsls r1, r1, #1
  adds r0, r5, r1
  movs r4, #0
  strh r4, [r0, #0]
  movs r0, #1
  bl sub_0802B368
  strh r4, [r5, #44]       @ 0x2c
  adds r0, r5, #0
  adds r0, #148            @ 0x94
  strh r7, [r0, #0]
  str r7, [r5, #68]        @ 0x44
  adds r0, #84             @ 0x54
  movs r4, #8
  strh r4, [r0, #0]
  movs r2, #0
  ldrsh r0, [r6, r2]
  bl 0x08004BFC
  ldrh r0, [r6, #0]
  cmp r0, #3
  bne _0800EEF8
  strh r4, [r5, #48]       @ 0x30
_0800EEF8:
  mov r1, r8
  ldrh r1, [r1, #0]
  cmp r1, #1
  beq _0800EF20
  cmp r7, #64              @ 0x40
  bne _0800EF10
  movs r0, #2
  bl sub_0802B368
  ldrh r0, [r6, #0]
  subs r0, #1
  strh r0, [r6, #0]
_0800EF10:
  cmp r7, #128             @ 0x80
  bne _0800EF20
  movs r0, #2
  bl sub_0802B368
  ldrh r0, [r6, #0]
  adds r0, #1
  strh r0, [r6, #0]
_0800EF20:
  adds r1, r6, #0
  movs r2, #0
  ldrsh r0, [r1, r2]
  cmp r0, #0
  bgt _0800EF2E
  movs r0, #0
  strh r0, [r1, #0]
_0800EF2E:
  movs r2, #0
  ldrsh r0, [r1, r2]
  cmp r0, #1
  ble _0800EF3A
  movs r0, #2
  strh r0, [r1, #0]
_0800EF3A:
  movs r1, #0
  ldrsh r0, [r6, r1]
  cmp r9, r0
  beq _0800EF48
  movs r0, #2
  bl sub_0802B368
_0800EF48:
  pop {r3, r4}
  mov r8, r3
  mov r9, r4
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0

@ ----------------------------------------------------------------------------
.type sub_0800EF54, %function
sub_0800EF54:
_0800EF54:
  push {r4, r5, lr}
  adds r5, r0, #0
  adds r1, r5, #0
  adds r1, #236            @ 0xec
  movs r2, #0
  ldrsh r0, [r1, r2]
  cmp r0, #1
  beq _0800EF74
  cmp r0, #1
  bgt _0800EF6E
  cmp r0, #0
  beq _0800EFFC
  b _0800F03A
_0800EF6E:
  cmp r0, #2
  beq _0800EF9C
  b _0800F03A
_0800EF74:
  movs r0, #0
  movs r1, #3
  bl 0x08002178
  lsls r0, r0, #16
  lsrs r0, r0, #16
  cmp r0, #4
  bne _0800F03A
  movs r0, #1
  movs r1, #3
  bl 0x08002178
  lsls r0, r0, #16
  lsrs r0, r0, #16
  cmp r0, #4
  bne _0800F03A
  adds r0, r5, #0
  bl sub_0800ED98
  b _0800F03A
_0800EF9C:
  ldr r0, _0800F004
  ldr r1, _0800F008
  adds r0, r0, r1
  ldrb r0, [r0, #0]
  cmp r0, #1
  bne _0800F00C
  movs r0, #0
  movs r1, #3
  bl 0x08002178
  lsls r0, r0, #16
  lsrs r0, r0, #16
  cmp r0, #5
  bne _0800EFCE
  movs r0, #1
  movs r1, #3
  bl 0x08002178
  lsls r0, r0, #16
  lsrs r0, r0, #16
  cmp r0, #5
  bne _0800EFCE
  adds r0, r5, #0
  bl sub_0800ED68
_0800EFCE:
  movs r0, #1
  movs r1, #6
  bl 0x08002178
  adds r4, r0, #0
  lsls r4, r4, #16
  lsrs r4, r4, #16
  bl sub_08004B68
  movs r2, #0
  ldrsh r0, [r0, r2]
  cmp r4, r0
  bne _0800EFF8
  movs r0, #1
  movs r1, #3
  bl 0x08002178
  lsls r0, r0, #16
  lsrs r0, r0, #16
  cmp r0, #4
  bne _0800F03A
_0800EFF8:
  adds r1, r5, #0
  adds r1, #232            @ 0xe8
_0800EFFC:
  movs r0, #2
  strh r0, [r1, #0]
  b _0800F03A
  .align 2, 0
_0800F004: .4byte 0x03001780
_0800F008: .4byte 0x000010C3
_0800F00C:
  adds r1, r5, #0
  adds r1, #232            @ 0xe8
  movs r0, #5
  strh r0, [r1, #0]
  movs r0, #0
  movs r1, #3
  bl 0x08002178
  lsls r0, r0, #16
  lsrs r0, r0, #16
  cmp r0, #5
  bne _0800F03A
  movs r0, #1
  movs r1, #3
  bl 0x08002178
  lsls r0, r0, #16
  lsrs r0, r0, #16
  cmp r0, #5
  bne _0800F03A
  adds r0, r5, #0
  bl sub_0800ED68
_0800F03A:
  pop {r4, r5}
  pop {r0}
  bx r0
