@ GT Advance 3 - menu record handlers IV
@ Region: file offset 0x00F040-0x00F22C (VMA 0x0800F040-0x0800F22C).
@ Pure Thumb, ARMCC; continuation of the menu span (menu_ebd8.s ends at
@ 0xF040; next raw resumes at 0xF22C).
@
@ Function map:
@   sub_0800F040 - 8-iteration record-bind loop (sl/r9/r8, sp frame
@                  16 B): for v==2 binds sub_08007B18(rec+0x4C, 7, i*8,
@                  r7, {3,1,1,0}) and v==3 with {4,1,1,0}, stepping the
@                  cursor by +8 per pass.
@   sub_0800F0BC - record-flag stepper: reads s16 table
@                  0x080CB548[rec+0xE0] (u16), switches on rec+0x104:
@                  {1,2} -> sub_08007B18(rec, r4, r5, {3,1,1,0}),
@                  {3}   -> sub_08007B18(rec, r4, r5, {4,1,1,0}).
@   sub_0800F134 - award-grid builder (r8): gates on s16(rec+0xF2)==0,
@                  switches on rec+0x104 == 2/3; lane-0 writes
@                  sub_08007B18(rec+0x4C, {10|9}, 0, r8+9,...) with
@                  rec+0xE0 steering for row/col/tier, lane-2 uses
@                  {3,1,1,0} sets; all calls share the {tier,1,1,0}
@                  sp-frame shape.
@
@ Transcribed from baserom.gba via objdump; byte-exact (make SHA gate).
@ Unconverted targets keep numeric `bl 0x0800XXXX` form. Pools at
@ original offsets.

.thumb

@ ----------------------------------------------------------------------------
.type sub_0800F040, %function
sub_0800F040:
_0800F040:
  push {r4, r5, r6, r7, lr}
  mov r7, sl
  mov r6, r9
  mov r5, r8
  push {r5, r6, r7}
  sub sp, #16
  mov r8, r0
  adds r7, r1, #0
  adds r6, r2, #0
  movs r5, #1
  movs r0, #0
  mov sl, r0
  movs r4, #0
  movs r0, #8
  mov r9, r0
_0800F05E:
  cmp r6, #2
  beq _0800F068
  cmp r6, #3
  beq _0800F084
  b _0800F09E
_0800F068:
  movs r0, #3
  str r0, [sp, #0]
  str r5, [sp, #4]
  str r5, [sp, #8]
  mov r0, sl
  str r0, [sp, #12]
  mov r0, r8
  adds r0, #76             @ 0x4c
  movs r1, #7
  adds r2, r4, #0
  adds r3, r7, #0
  bl sub_08007B18
  b _0800F09E
_0800F084:
  movs r0, #4
  str r0, [sp, #0]
  str r5, [sp, #4]
  str r5, [sp, #8]
  mov r0, sl
  str r0, [sp, #12]
  mov r0, r8
  adds r0, #76             @ 0x4c
  movs r1, #7
  adds r2, r4, #0
  adds r3, r7, #0
  bl sub_08007B18
_0800F09E:
  adds r4, #8
  movs r0, #1
  negs r0, r0
  add r9, r0
  mov r0, r9
  cmp r0, #0
  bge _0800F05E
  add sp, #16
  pop {r3, r4, r5}
  mov r8, r3
  mov r9, r4
  mov sl, r5
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0

@ ----------------------------------------------------------------------------
.type sub_0800F0BC, %function
sub_0800F0BC:
_0800F0BC:
  push {r4, r5, lr}
  sub sp, #16
  adds r3, r0, #0
  adds r4, r1, #0
  adds r5, r2, #0
  ldr r1, _0800F0EC
  adds r0, #224            @ 0xe0
  movs r2, #0
  ldrsh r0, [r0, r2]
  lsls r0, r0, #1
  adds r0, r0, r1
  ldrh r1, [r0, #0]
  movs r2, #130            @ 0x82
  lsls r2, r2, #1
  adds r0, r3, r2
  ldr r0, [r0, #0]
  cmp r0, #1
  beq _0800F0F8
  cmp r0, #1
  bgt _0800F0F0
  cmp r0, #0
  beq _0800F112
  b _0800F12A
  .2byte 0x0000            @ pad @ 0xF0EA
_0800F0EC: .4byte 0x080CB548
_0800F0F0:
  cmp r0, #2
  beq _0800F112
  cmp r0, #3
  bne _0800F12A
_0800F0F8:
  movs r0, #4
  str r0, [sp, #0]
  movs r0, #1
  str r0, [sp, #4]
  str r0, [sp, #8]
  movs r0, #0
  str r0, [sp, #12]
  adds r0, r3, #0
  adds r2, r4, #0
  adds r3, r5, #0
  bl sub_08007B18
  b _0800F12A
_0800F112:
  movs r0, #3
  str r0, [sp, #0]
  movs r0, #1
  str r0, [sp, #4]
  str r0, [sp, #8]
  movs r0, #0
  str r0, [sp, #12]
  adds r0, r3, #0
  adds r2, r4, #0
  adds r3, r5, #0
  bl sub_08007B18
_0800F12A:
  add sp, #16
  pop {r4, r5}
  pop {r0}
  bx r0
  .2byte 0x0000            @ pad @ 0xF132

@ ----------------------------------------------------------------------------
.type sub_0800F134, %function
sub_0800F134:
_0800F134:
  push {r4, r5, r6, r7, lr}
  mov r7, r8
  push {r7}
  sub sp, #16
  adds r3, r0, #0
  mov r8, r2
  adds r0, #242            @ 0xf2
  movs r1, #0
  ldrsh r7, [r0, r1]
  cmp r7, #0
  bne _0800F220
  movs r2, #130            @ 0x82
  lsls r2, r2, #1
  adds r0, r3, r2
  ldr r0, [r0, #0]
  cmp r0, #2
  beq _0800F1A8
  cmp r0, #3
  bne _0800F220
  adds r0, r3, #0
  adds r0, #224            @ 0xe0
  movs r1, #0
  ldrsh r0, [r0, r1]
  cmp r0, #0
  bne _0800F172
  adds r0, r3, #0
  adds r0, #76             @ 0x4c
  mov r3, r8
  adds r3, #9
  movs r1, #4
  b _0800F1BC
_0800F172:
  cmp r0, #3
  bne _0800F17E
  adds r0, r3, #0
  adds r0, #76             @ 0x4c
  movs r1, #4
  b _0800F1D8
_0800F17E:
  adds r6, r3, #0
  adds r6, #76             @ 0x4c
  mov r3, r8
  adds r3, #9
  movs r5, #4
  str r5, [sp, #0]
  movs r4, #1
  str r4, [sp, #4]
  str r4, [sp, #8]
  str r7, [sp, #12]
  adds r0, r6, #0
  movs r1, #10
  movs r2, #0
  bl sub_08007B18
  str r5, [sp, #0]
  str r4, [sp, #4]
  str r4, [sp, #8]
  str r7, [sp, #12]
  adds r0, r6, #0
  b _0800F1E2
_0800F1A8:
  adds r0, r3, #0
  adds r0, #224            @ 0xe0
  movs r2, #0
  ldrsh r1, [r0, r2]
  cmp r1, #0
  bne _0800F1D0
  subs r0, #148            @ 0x94
  mov r3, r8
  adds r3, #9
  movs r1, #3
_0800F1BC:
  str r1, [sp, #0]
  movs r1, #1
  str r1, [sp, #4]
  str r1, [sp, #8]
  str r7, [sp, #12]
  movs r1, #10
  movs r2, #0
  bl sub_08007B18
  b _0800F220
_0800F1D0:
  cmp r1, #3
  bne _0800F1EE
  adds r0, r3, #0
  adds r0, #76             @ 0x4c
_0800F1D8:
  str r1, [sp, #0]
  movs r1, #1
  str r1, [sp, #4]
  str r1, [sp, #8]
  str r7, [sp, #12]
_0800F1E2:
  movs r1, #9
  movs r2, #0
  mov r3, r8
  bl sub_08007B18
  b _0800F220
_0800F1EE:
  adds r6, r3, #0
  adds r6, #76             @ 0x4c
  mov r3, r8
  adds r3, #9
  movs r5, #3
  str r5, [sp, #0]
  movs r4, #1
  str r4, [sp, #4]
  str r4, [sp, #8]
  str r7, [sp, #12]
  adds r0, r6, #0
  movs r1, #10
  movs r2, #0
  bl sub_08007B18
  str r5, [sp, #0]
  str r4, [sp, #4]
  str r4, [sp, #8]
  str r7, [sp, #12]
  adds r0, r6, #0
  movs r1, #9
  movs r2, #0
  mov r3, r8
  bl sub_08007B18
_0800F220:
  add sp, #16
  pop {r3}
  mov r8, r3
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0

@ Region end 0x0800F22C. The next function's VMA labels live in the next
@ region file, so this file cannot spell them; the anchor gives
@ tools/match_c_slice.py a unique end marker when this body is C-owned.
@ Emits no bytes.
menu_f040_end:
