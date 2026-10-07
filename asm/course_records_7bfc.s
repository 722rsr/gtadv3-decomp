@ GT Advance 3 - course record iteration + course state leaves
@ Region: file offset 0x007BFC-0x008290 (VMA 0x08007BFC-0x08008290).
@ Pure Thumb; continuation of the course resource family (preceding slice
@ course_resource_leaf_more2.s ends at 0x7BFC; the next function begins at
@ 0x8290 = sub_08008290 marker in passthrough.inc).
@
@ Three groups of functions:
@   1. Record iterators (0x7BFC/0x7C68/0x7CD0/0x7D4C/0x7DB4/0x7E14): resolve
@      the course record array via sub_08007498/sub_0800748C, loop over the
@      u8 count at [+7] and per-record 4-byte rows at [+8], and call the
@      generic placement/emit helper (sub_08002ED0 / sub_08003004 /
@      sub_08002F68 / sub_08002DB8) with an sp-built argument block.
@   2. Math leaves (0x7EC4/0x7EE0/0x7F08/0x7FC0): record u16 sums, an
@      abs/round-average helper, a sqrt+udiv distance routine, and a
@      heading-table interpolator (tables 0x080C9064 / 0x080CA064 /
@      0x080CB064).
@   3. Course state leaves (0x8014 onward): tick a per-course state block
@      behind the global pointer slot 0x030003E0 (right before the subsystem
@      instance array at 0x030003E8), copy a 32-byte template from
@      0x0805F604, and write small fields (u16/u8) of that block; plus the
@      course-id handler sub_08008164 (div-by-10 id tables 0x080CB17C /
@      0x080CB074) and the timed reset helper sub_0800821C.
@
@ Transcribed from baserom.gba via objdump (+gbadisasm cross-check for the
@ first function); byte-exact (make SHA gate). Unconverted targets keep
@ numeric `bl 0x0800XXXX` form exactly like neighboring files.

.thumb

@ ----------------------------------------------------------------------------
.type sub_08007BFC, %function
sub_08007BFC:
_08007BFC:
  push {r4, r5, r6, r7, lr}
  mov r7, r9
  mov r6, r8
  push {r6, r7}
  sub sp, #24
  mov r9, r1
  adds r1, r2, #0
  mov r8, r3
  ldr r0, [r0, #4]
  bl sub_08007498
  bl sub_0800748C
  adds r6, r0, #0
  movs r5, #0
  ldrb r0, [r6, #7]
  cmp r5, r0
  bge _08007C58
  movs r7, #1
  adds r4, r6, #0
  adds r4, #8
_08007C26:
  ldrb r0, [r4, #2]
  add r0, r8
  ldrb r2, [r4, #3]
  ldr r3, [sp, #52]
  adds r1, r2, r3
  ldrb r2, [r4, #0]
  add r2, r9
  ldr r3, [sp, #60]
  str r3, [sp, #0]
  ldrb r3, [r4, #1]
  str r3, [sp, #4]
  str r7, [sp, #8]
  ldr r3, [sp, #64]
  str r3, [sp, #12]
  ldr r3, [sp, #68]
  str r3, [sp, #16]
  str r7, [sp, #20]
  ldr r3, [sp, #56]
  bl 0x08002ED0
  adds r4, #4
  adds r5, #1
  ldrb r0, [r6, #7]
  cmp r5, r0
  blt _08007C26
_08007C58:
  add sp, #24
  pop {r3, r4}
  mov r8, r3
  mov r9, r4
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .short 0

@ ----------------------------------------------------------------------------
.type sub_08007C68, %function
sub_08007C68:
_08007C68:
  push {r4, r5, r6, r7, lr}
  mov r7, r8
  push {r7}
  sub sp, #24
  mov r8, r1
  adds r1, r2, #0
  adds r7, r3, #0
  ldr r0, [r0, #4]
  bl sub_08007498
  bl sub_0800748C
  adds r6, r0, #0
  movs r5, #0
  ldrb r0, [r6, #7]
  cmp r5, r0
  bge _08007CC4
  adds r4, r6, #0
  adds r4, #8
_08007C8E:
  ldrb r1, [r4, #2]
  adds r0, r1, r7
  ldrb r2, [r4, #3]
  ldr r3, [sp, #48]
  adds r1, r2, r3
  ldrb r2, [r4, #0]
  add r2, r8
  ldr r3, [sp, #56]
  str r3, [sp, #0]
  ldrb r3, [r4, #1]
  str r3, [sp, #4]
  movs r3, #1
  str r3, [sp, #8]
  ldr r3, [sp, #60]
  str r3, [sp, #12]
  ldr r3, [sp, #64]
  str r3, [sp, #16]
  movs r3, #0
  str r3, [sp, #20]
  ldr r3, [sp, #52]
  bl 0x08002ED0
  adds r4, #4
  adds r5, #1
  ldrb r0, [r6, #7]
  cmp r5, r0
  blt _08007C8E
_08007CC4:
  add sp, #24
  pop {r3}
  mov r8, r3
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0

@ ----------------------------------------------------------------------------
.type sub_08007CD0, %function
sub_08007CD0:
_08007CD0:
  push {r4, r5, r6, r7, lr}
  mov r7, sl
  mov r6, r9
  mov r5, r8
  push {r5, r6, r7}
  sub sp, #24
  mov sl, r1
  adds r1, r2, #0
  mov r9, r3
  ldr r0, [r0, #4]
  bl sub_08007498
  bl sub_0800748C
  adds r6, r0, #0
  movs r0, #240
  ldrb r1, [r6, #4]
  subs r0, r0, r1
  lsrs r1, r0, #31
  adds r0, r0, r1
  asrs r0, r0, #1
  mov r8, r0
  movs r5, #0
  ldrb r2, [r6, #7]
  cmp r5, r2
  bge _08007D3A
  movs r7, #1
  adds r4, r6, #0
  adds r4, #8
_08007D0A:
  ldrb r0, [r4, #2]
  add r0, r8
  ldrb r1, [r4, #3]
  add r1, r9
  ldrb r2, [r4, #0]
  add r2, sl
  ldr r3, [sp, #60]
  str r3, [sp, #0]
  ldrb r3, [r4, #1]
  str r3, [sp, #4]
  str r7, [sp, #8]
  movs r3, #0
  str r3, [sp, #12]
  ldr r3, [sp, #64]
  str r3, [sp, #16]
  str r7, [sp, #20]
  ldr r3, [sp, #56]
  bl 0x08002ED0
  adds r4, #4
  adds r5, #1
  ldrb r0, [r6, #7]
  cmp r5, r0
  blt _08007D0A
_08007D3A:
  add sp, #24
  pop {r3, r4, r5}
  mov r8, r3
  mov r9, r4
  mov sl, r5
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .short 0

@ ----------------------------------------------------------------------------
.type sub_08007D4C, %function
sub_08007D4C:
_08007D4C:
  push {r4, r5, r6, r7, lr}
  mov r7, r9
  mov r6, r8
  push {r6, r7}
  sub sp, #16
  adds r4, r0, #0
  mov r9, r2
  mov r8, r3
  ldr r0, [r4, #4]
  bl sub_08007498
  bl sub_0800748C
  adds r6, r0, #0
  ldrh r0, [r6, #2]
  ldrh r4, [r4, #0]
  adds r7, r0, r4
  movs r5, #0
  ldrb r3, [r6, #7]
  cmp r5, r3
  bge _08007DA6
  adds r4, r6, #0
  adds r4, #8
_08007D7A:
  ldrb r0, [r4, #2]
  add r0, r9
  ldrb r1, [r4, #3]
  add r1, r8
  ldrb r3, [r4, #0]
  adds r2, r3, r7
  ldr r3, [sp, #56]
  str r3, [sp, #0]
  ldrb r3, [r4, #1]
  str r3, [sp, #4]
  ldr r3, [sp, #44]
  str r3, [sp, #8]
  ldr r3, [sp, #48]
  str r3, [sp, #12]
  ldr r3, [sp, #52]
  bl 0x08003004
  adds r4, #4
  adds r5, #1
  ldrb r0, [r6, #7]
  cmp r5, r0
  blt _08007D7A
_08007DA6:
  add sp, #16
  pop {r3, r4}
  mov r8, r3
  mov r9, r4
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0

@ ----------------------------------------------------------------------------
.type sub_08007DB4, %function
sub_08007DB4:
_08007DB4:
  push {r4, r5, r6, r7, lr}
  mov r7, r8
  push {r7}
  sub sp, #16
  mov r8, r1
  adds r1, r2, #0
  adds r7, r3, #0
  ldr r0, [r0, #4]
  bl sub_08007498
  bl sub_0800748C
  adds r6, r0, #0
  movs r5, #0
  ldrb r0, [r6, #7]
  cmp r5, r0
  bge _08007E08
  adds r4, r6, #0
  adds r4, #8
_08007DDA:
  ldrb r1, [r4, #2]
  adds r0, r1, r7
  ldrb r2, [r4, #3]
  ldr r3, [sp, #40]
  adds r1, r2, r3
  ldrb r2, [r4, #0]
  add r2, r8
  ldr r3, [sp, #56]
  str r3, [sp, #0]
  ldrb r3, [r4, #1]
  str r3, [sp, #4]
  ldr r3, [sp, #44]
  str r3, [sp, #8]
  ldr r3, [sp, #48]
  str r3, [sp, #12]
  ldr r3, [sp, #52]
  bl 0x08003004
  adds r4, #4
  adds r5, #1
  ldrb r0, [r6, #7]
  cmp r5, r0
  blt _08007DDA
_08007E08:
  add sp, #16
  pop {r3}
  mov r8, r3
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0

@ ----------------------------------------------------------------------------
.type sub_08007E14, %function
sub_08007E14:
_08007E14:
  push {r4, r5, r6, r7, lr}
  mov r7, sl
  mov r6, r9
  mov r5, r8
  push {r5, r6, r7}
  sub sp, #16
  mov sl, r1
  adds r1, r2, #0
  mov r9, r3
  ldr r2, [sp, #60]
  lsls r2, r2, #24
  lsrs r2, r2, #24
  str r2, [sp, #12]
  ldr r0, [r0, #4]
  bl sub_08007498
  bl sub_0800748C
  mov r8, r0
  movs r5, #0
  movs r6, #0
  movs r7, #0
  ldrb r0, [r0, #7]
  cmp r7, r0
  bge _08007EB4
_08007E46:
  mov r4, r8
  adds r4, #8
_08007E4A:
  ldrb r1, [r4, #0]
  cmp r1, r5
  blt _08007E54
  subs r0, r1, r5
  b _08007E5C
_08007E54:
  movs r2, #128
  lsls r2, r2, #1
  adds r0, r1, r2
  subs r0, r0, r5
_08007E5C:
  adds r5, r1, #0
  adds r6, r6, r0
  ldr r3, [sp, #12]
  cmp r3, #0
  beq _08007E88
  ldrb r0, [r4, #2]
  add r0, r9
  ldrb r2, [r4, #3]
  ldr r3, [sp, #48]
  adds r1, r2, r3
  mov r3, sl
  adds r2, r3, r6
  ldr r3, [sp, #56]
  str r3, [sp, #0]
  ldrb r3, [r4, #1]
  str r3, [sp, #4]
  movs r3, #1
  str r3, [sp, #8]
  ldr r3, [sp, #52]
  bl 0x08002F68
  b _08007EA8
_08007E88:
  ldrb r0, [r4, #2]
  add r0, r9
  ldrb r2, [r4, #3]
  ldr r3, [sp, #48]
  adds r1, r2, r3
  mov r3, sl
  adds r2, r3, r6
  ldr r3, [sp, #56]
  str r3, [sp, #0]
  ldrb r3, [r4, #1]
  str r3, [sp, #4]
  movs r3, #1
  str r3, [sp, #8]
  ldr r3, [sp, #52]
  bl 0x08002DB8
_08007EA8:
  adds r4, #4
  adds r7, #1
  mov r0, r8
  ldrb r0, [r0, #7]
  cmp r7, r0
  blt _08007E4A
_08007EB4:
  add sp, #16
  pop {r3, r4, r5}
  mov r8, r3
  mov r9, r4
  mov sl, r5
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0

@ ----------------------------------------------------------------------------
.type sub_08007EC4, %function
sub_08007EC4:
_08007EC4:
  push {r4, lr}
  adds r4, r0, #0
  ldr r0, [r4, #4]
  bl sub_08007498
  bl sub_0800748C
  ldrh r0, [r0, #2]
  ldrh r4, [r4, #0]
  adds r0, r0, r4
  pop {r4}
  pop {r1}
  bx r1
  .short 0

@ ----------------------------------------------------------------------------
@ abs(a) + abs(b), rounded average: (a+b) - ((5*min)>>3)
.type sub_08007EE0, %function
sub_08007EE0:
_08007EE0:
  adds r3, r0, #0
  adds r2, r1, #0
  cmp r3, #0
  bge _08007EEA
  negs r3, r3
_08007EEA:
  cmp r2, #0
  bge _08007EF0
  negs r2, r2
_08007EF0:
  cmp r2, r3
  bgt _08007EFC
  adds r0, r2, r3
  lsls r1, r2, #2
  adds r1, r1, r2
  b _08007F02
_08007EFC:
  adds r0, r3, r2
  lsls r1, r3, #2
  adds r1, r1, r3
_08007F02:
  asrs r1, r1, #3
  subs r0, r0, r1
  bx lr

@ ----------------------------------------------------------------------------
.type sub_08007F08, %function
sub_08007F08:
_08007F08:
  push {r4, r5, r6, r7, lr}
  mov r7, r8
  push {r7}
  adds r7, r1, #0
  adds r5, r2, #0
  adds r6, r3, #0
  subs r5, r5, r0
  cmp r5, #0
  bge _08007F22
  negs r5, r5
  movs r0, #0
  mov r8, r0
  b _08007F26
_08007F22:
  movs r1, #1
  mov r8, r1
_08007F26:
  subs r6, r6, r7
  cmp r6, #0
  bge _08007F32
  negs r6, r6
  movs r7, #0
  b _08007F34
_08007F32:
  movs r7, #1
_08007F34:
  cmp r5, #0
  bne _08007F40
  cmp r6, #0
  bne _08007F40
  movs r0, #0
  b _08007FB4
_08007F40:
  adds r0, r5, #0
  muls r0, r5
  adds r1, r6, #0
  muls r1, r6
  adds r0, r0, r1
  bl sub_0802D9AC          @ BIOS sqrt (swi 8)
  adds r4, r0, #0
  lsls r4, r4, #16
  lsrs r4, r4, #16
  lsls r0, r5, #11
  adds r1, r4, #0
  bl sub_0802DF6C          @ u32 divide
  adds r5, r0, #0
  lsls r0, r6, #11
  adds r1, r4, #0
  bl sub_0802DF6C          @ u32 divide
  adds r6, r0, #0
  cmp r5, r6
  bgt _08007F78
  ldr r1, _08007F74
  lsls r0, r5, #1
  b _08007F7C
  .align 2, 0
_08007F74: .4byte 0x080C9064
_08007F78:
  ldr r1, _08007F94
  lsls r0, r6, #1
_08007F7C:
  adds r0, r0, r1
  movs r1, #0
  ldrsh r5, [r0, r1]
  mov r0, r8
  cmp r0, #0
  bne _08007FA0
  cmp r7, #0
  beq _08007F98
  movs r1, #128
  lsls r1, r1, #3
  adds r5, r5, r1
  b _08007FB2
  .align 2, 0
_08007F94: .4byte 0x080CA064
_08007F98:
  movs r0, #128
  lsls r0, r0, #3
  subs r5, r0, r5
  b _08007FB2
_08007FA0:
  cmp r7, #0
  beq _08007FAC
  movs r0, #192
  lsls r0, r0, #4
  subs r5, r0, r5
  b _08007FB2
_08007FAC:
  movs r0, #192
  lsls r0, r0, #4
  adds r5, r5, r0
_08007FB2:
  adds r0, r5, #0
_08007FB4:
  pop {r3}
  mov r8, r3
  pop {r4, r5, r6, r7}
  pop {r1}
  bx r1
  .short 0

@ ----------------------------------------------------------------------------
.type sub_08007FC0, %function
sub_08007FC0:
_08007FC0:
  push {r4, r5, r6, lr}
  adds r5, r3, #0
  lsls r0, r0, #16
  lsls r1, r1, #16
  lsrs r3, r1, #16
  lsls r2, r2, #16
  lsrs r1, r2, #16
  movs r6, #0
  lsrs r4, r0, #16
  asrs r0, r0, #16
  cmp r0, #7
  ble _08007FDC
  movs r4, #7
  movs r6, #1
_08007FDC:
  lsls r1, r1, #16
  asrs r1, r1, #16
  lsls r0, r3, #16
  asrs r3, r0, #16
  subs r1, r1, r3
  ldr r2, _0800800C
  lsls r0, r4, #16
  asrs r0, r0, #15
  adds r0, r0, r2
  movs r2, #0
  ldrsh r0, [r0, r2]
  muls r0, r1
  cmp r0, #0
  bge _08007FFC
  ldr r1, _08008010
  adds r0, r0, r1
_08007FFC:
  asrs r0, r0, #12
  adds r0, r3, r0
  strh r0, [r5, #0]
  adds r0, r6, #0
  pop {r4, r5, r6}
  pop {r1}
  bx r1
  .short 0
  .align 2, 0
_0800800C: .4byte 0x080CB064
_08008010: .4byte 0x00000FFF

@ ----------------------------------------------------------------------------
.type sub_08008014, %function
sub_08008014:
_08008014:
  push {r4, r5, r6, lr}
  sub sp, #32
  mov r1, sp
  ldr r0, _08008064
  ldmia r0!, {r2, r3, r4}
  stmia r1!, {r2, r3, r4}
  ldmia r0!, {r2, r3, r4}
  stmia r1!, {r2, r3, r4}
  ldmia r0!, {r2, r3}
  stmia r1!, {r2, r3}
  ldr r0, _08008068
  movs r4, #135
  lsls r4, r4, #5
  adds r5, r0, r4
  ldr r1, _0800806C
  adds r0, r0, r1
  ldr r6, [r0, #0]
  movs r0, #7
  ands r0, r6
  lsls r0, r0, #2
  add r0, sp
  ldr r4, [r5, #0]
  ldr r0, [r0, #0]
  adds r4, r4, r0
  ldr r0, _08008070
  ldr r0, [r0, #0]
  adds r4, r4, r0
  str r4, [r5, #0]
  adds r0, r4, #0
  movs r1, #23
  bl sub_0802DF6C          @ u32 divide
  adds r4, r4, r0
  adds r4, r4, r6
  str r4, [r5, #0]
  adds r0, r4, #0
  add sp, #32
  pop {r4, r5, r6}
  pop {r1}
  bx r1
  .align 2, 0
_08008064: .4byte 0x0805F604
_08008068: .4byte 0x03001780
_0800806C: .4byte 0x000010D8
_08008070: .4byte 0x04000100

@ ----------------------------------------------------------------------------
@ Small state leaves: write fields of the block behind global pointer 0x030003E0.
.type sub_08008074, %function
sub_08008074:
_08008074:
  ldr r1, _0800807C
  ldr r1, [r1, #0]
  strh r0, [r1, #0]
  bx lr
  .align 2, 0
_0800807C: .4byte 0x030003E0

.type sub_08008080, %function
sub_08008080:
_08008080:
  ldr r1, _08008088
  ldr r1, [r1, #0]
  strh r0, [r1, #14]
  bx lr
  .align 2, 0
_08008088: .4byte 0x030003E0

.type sub_0800808C, %function
sub_0800808C:
_0800808C:
  ldr r1, _08008094
  ldr r1, [r1, #0]
  strh r0, [r1, #6]
  bx lr
  .align 2, 0
_08008094: .4byte 0x030003E0

.type sub_08008098, %function
sub_08008098:
_08008098:
  ldr r1, _080080A0
  ldr r1, [r1, #0]
  strh r0, [r1, #10]
  bx lr
  .align 2, 0
_080080A0: .4byte 0x030003E0

@ Signed clamp to 0x3E7, store at [+2]
.type sub_080080A4, %function
sub_080080A4:
_080080A4:
  lsls r0, r0, #16
  lsrs r1, r0, #16
  cmp r0, #0
  bge _080080AE
  movs r1, #0
_080080AE:
  lsls r0, r1, #16
  asrs r0, r0, #16
  ldr r2, _080080C4
  cmp r0, r2
  ble _080080BA
  adds r1, r2, #0
_080080BA:
  ldr r0, _080080C8
  ldr r0, [r0, #0]
  strh r1, [r0, #2]
  bx lr
  .short 0
  .align 2, 0
_080080C4: .4byte 0x000003E7
_080080C8: .4byte 0x030003E0

.type sub_080080CC, %function
sub_080080CC:
_080080CC:
  ldr r1, _080080D4
  ldr r1, [r1, #0]
  str r0, [r1, #64]
  bx lr
  .align 2, 0
_080080D4: .4byte 0x030003E0

.type sub_080080D8, %function
sub_080080D8:
_080080D8:
  ldr r1, _080080E0
  ldr r1, [r1, #0]
  str r0, [r1, #68]
  bx lr
  .align 2, 0
_080080E0: .4byte 0x030003E0

.type sub_080080E4, %function
sub_080080E4:
_080080E4:
  ldr r1, _080080F0
  ldr r1, [r1, #0]
  str r0, [r1, #72]
  movs r0, #104
  strh r0, [r1, #4]
  bx lr
  .align 2, 0
_080080F0: .4byte 0x030003E0

.type sub_080080F4, %function
sub_080080F4:
_080080F4:
  ldr r1, _08008100
  ldr r1, [r1, #0]
  str r0, [r1, #72]
  movs r0, #150
  strh r0, [r1, #34]
  bx lr
  .align 2, 0
_08008100: .4byte 0x030003E0

.type sub_08008104, %function
sub_08008104:
_08008104:
  ldr r0, _08008110
  ldr r0, [r0, #0]
  adds r0, #78
  movs r1, #1
  strb r1, [r0, #0]
  bx lr
  .align 2, 0
_08008110: .4byte 0x030003E0

@ If u16[+12] != value, flag byte at [+77]; store value at [+12]
.type sub_08008114, %function
sub_08008114:
_08008114:
  lsls r0, r0, #16
  lsrs r0, r0, #16
  adds r2, r0, #0
  ldr r3, _08008130
  ldr r1, [r3, #0]
  ldrh r0, [r1, #12]
  cmp r0, r2
  beq _0800812A
  adds r1, #77
  movs r0, #1
  strb r0, [r1, #0]
_0800812A:
  ldr r0, [r3, #0]
  strh r2, [r0, #12]
  bx lr
  .align 2, 0
_08008130: .4byte 0x030003E0

@ If u16[+10] > value, keep [+10]; if u16[+8] != chosen, flag at [+76];
@ store chosen at [+8]
.type sub_08008134, %function
sub_08008134:
_08008134:
  push {r4, r5, lr}
  lsls r0, r0, #16
  ldr r4, _08008160
  ldr r2, [r4, #0]
  lsrs r3, r0, #16
  ldrh r5, [r2, #10]
  lsls r1, r5, #16
  cmp r0, r1
  ble _08008148
  ldrh r3, [r2, #10]
_08008148:
  ldrh r0, [r2, #8]
  cmp r0, r3
  beq _08008156
  adds r1, r2, #0
  adds r1, #76
  movs r0, #1
  strb r0, [r1, #0]
_08008156:
  ldr r0, [r4, #0]
  strh r3, [r0, #8]
  pop {r4, r5}
  pop {r0}
  bx r0
  .align 2, 0
_08008160: .4byte 0x030003E0

@ ----------------------------------------------------------------------------
@ Course-id handler: store id at [+16], clear [+28], div-by-10 table pick,
@ then the shared label/emit helpers.
.type sub_08008164, %function
sub_08008164:
_08008164:
  push {r4, r5, r6, r7, lr}
  adds r6, r0, #0
  ldr r7, _080081B8
  ldr r1, [r7, #0]
  movs r0, #0
  strh r6, [r1, #16]
  strh r0, [r1, #28]
  cmp r6, #1
  ble _080081C4
  cmp r6, #9
  ble _08008198
  ldr r5, _080081BC
  ldr r4, _080081C0
  adds r0, r6, #0
  movs r1, #10
  bl sub_0802D978          @ BIOS div (swi 6)
  lsls r0, r0, #1
  adds r0, r0, r4
  ldrh r1, [r0, #0]
  ldr r0, [r7, #0]
  ldrh r2, [r0, #56]
  adds r2, #16
  adds r0, r5, #0
  bl 0x08007538
_08008198:
  ldr r5, _080081BC
  ldr r4, _080081C0
  adds r0, r6, #0
  movs r1, #10
  bl sub_0802D97C          @ BIOS div (swi 6)
  lsls r0, r0, #1
  adds r0, r0, r4
  ldrh r1, [r0, #0]
  ldr r0, [r7, #0]
  ldrh r2, [r0, #56]
  adds r0, r5, #0
  bl 0x08007538
  b _080081CE
  .short 0
  .align 2, 0
_080081B8: .4byte 0x030003E0
_080081BC: .4byte 0x0828A35C
_080081C0: .4byte 0x080CB17C
_080081C4:
  ldr r0, _080081EC
  ldrh r2, [r1, #56]
  movs r1, #11
  bl 0x08007538
_080081CE:
  ldr r0, _080081F0
  ldr r4, _080081F4
  ldr r1, [r4, #0]
  ldrh r3, [r1, #58]
  movs r1, #1
  movs r2, #0
  bl 0x08007614
  ldr r0, [r4, #0]
  adds r0, #79
  movs r1, #1
  strb r1, [r0, #0]
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .align 2, 0
_080081EC: .4byte 0x0828C420
_080081F0: .4byte 0x0828A35C
_080081F4: .4byte 0x030003E0

.type sub_080081F8, %function
sub_080081F8:
_080081F8:
  ldr r1, _08008204
  ldr r1, [r1, #0]
  strh r0, [r1, #18]
  movs r0, #48
  strh r0, [r1, #26]
  bx lr
  .align 2, 0
_08008204: .4byte 0x030003E0

.type sub_08008208, %function
sub_08008208:
_08008208:
  ldr r0, _08008218
  ldr r1, [r0, #0]
  movs r0, #16
  strh r0, [r1, #22]
  movs r0, #1
  strh r0, [r1, #24]
  bx lr
  .short 0
  .align 2, 0
_08008218: .4byte 0x030003E0

@ ----------------------------------------------------------------------------
@ Timed reset helper: arm timing fields, sdiv by 5 for the slot index, and a
@ gated course-reset call.
.type sub_0800821C, %function
sub_0800821C:
_0800821C:
  push {r4, r5, r6, lr}
  ldr r5, _0800827C
  ldr r1, [r5, #0]
  movs r0, #16
  strh r0, [r1, #22]
  movs r0, #3
  strh r0, [r1, #24]
  adds r1, #79
  movs r6, #1
  strb r6, [r1, #0]
  ldr r4, [r5, #0]
  movs r1, #16
  ldrsh r0, [r4, r1]
  movs r1, #5
  bl sub_0802DE04          @ s32 divide
  strh r0, [r4, #30]
  lsls r0, r0, #16
  asrs r0, r0, #16
  cmp r0, #10
  ble _0800824A
  movs r0, #10
  strh r0, [r4, #30]
_0800824A:
  ldr r1, _08008280
  ldr r2, [r5, #0]
  movs r3, #30
  ldrsh r0, [r2, r3]
  lsls r0, r0, #3
  adds r3, r0, r1
  movs r4, #4
  ldrsh r1, [r3, r4]
  movs r0, #1
  negs r0, r0
  cmp r1, r0
  beq _08008274
  ldr r0, [r2, #100]
  ldrh r2, [r2, #62]
  bl sub_08007ABC
  ldr r0, [r5, #0]
  movs r1, #60
  strh r1, [r0, #28]
  adds r0, #79
  strb r6, [r0, #0]
_08008274:
  pop {r4, r5, r6}
  pop {r0}
  bx r0
  .short 0
  .align 2, 0
_0800827C: .4byte 0x030003E0
_08008280: .4byte 0x080CB074

.type sub_08008284, %function
sub_08008284:
_08008284:
  ldr r1, _0800828C
  ldr r1, [r1, #0]
  strh r0, [r1, #32]
  bx lr
  .align 2, 0
_0800828C: .4byte 0x030003E0

@ End-of-region anchor for the splicer. This file declares exactly one
@ `@ Region:` (0x08007BFC-0x08008290) and has no `.include`; the body at
@ 0x08008284 is 8 bytes of code plus its 4-byte pool word and ends on
@ 0x08008290, exactly that boundary. Verified against baserom.gba: the span
@ reads 4901 6809 8408 4770 | 03e0 0300. Unambiguous, so the anchor is safe.
@ Without it the body is refused as 'no end marker'.
course_records_7bfc_end:
