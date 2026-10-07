@ GT Advance 3 - packed collection-grid accessors
@ Region: file offset 0x025CF4-0x025DBC (VMA 0x08025CF4-0x08025DBC).
@ Pure Thumb; packed 2-bit collection grid and row-completion counters.

.thumb
.type sub_08025CF4, %function
sub_08025CF4:
_08025CF4:
  push {r4, r5, r6, lr}
  sub sp, #4
  adds r4, r0, #0
  adds r6, r1, #0
  adds r5, r2, #0
  ldr r1, _08025D14
  mov r0, sp
  movs r2, #4
  bl 0x0802E0A4
  cmp r5, #10
  bgt.n _08025D10
  cmp r5, #0
  bge.n _08025D18
_08025D10:
  movs r0, #0
  b.n _08025D56
_08025D14: .word 0x08060D48
_08025D18:
  movs r0, #44
  muls r4, r0
  movs r0, #11
  muls r0, r6
  adds r4, r4, r0
  adds r4, r4, r5
  adds r0, r4, #0
  movs r1, #4
  bl 0x0802D97C
  adds r5, r0, #0
  adds r0, r4, #0
  movs r1, #4
  bl 0x0802D97C
  adds r2, r0, #0
  ldr r1, _08025D60
  cmp r4, #0
  bge.n _08025D40
  adds r4, #3
_08025D40:
  asrs r0, r4, #2
  adds r0, r0, r1
  mov r3, sp
  adds r1, r3, r5
  ldrb r0, [r0, #0]
  ldrb r1, [r1, #0]
  ands r0, r1
  lsls r1, r2, #1
  asrs r0, r1
  lsls r0, r0, #24
  lsrs r0, r0, #24
_08025D56:
  add sp, #4
  pop {r4, r5, r6}
  pop {r1}
  bx r1
  .short 0x0000
_08025D60: .word 0x03001780

.type sub_08025D64, %function
sub_08025D64:
_08025D64:
  push {r4, r5, r6, r7, lr}
  adds r7, r0, #0
  adds r6, r1, #0
  movs r5, #0
  movs r4, #0
_08025D6E:
  cmp r4, #10
  bgt.n _08025D88
  adds r0, r7, #0
  adds r1, r6, #0
  adds r2, r4, #0
  bl 0x08025CF4
  lsls r0, r0, #24
  cmp r0, #0
  beq.n _08025D88
  adds r5, #1
  adds r4, #1
  b.n _08025D6E
_08025D88:
  adds r0, r5, #0
  pop {r4, r5, r6, r7}
  pop {r1}
  bx r1

.type sub_08025D90, %function
sub_08025D90:
_08025D90:
  push {r4, r5, r6, lr}
  adds r6, r0, #0
  movs r5, #0
  movs r4, #0
_08025D98:
  cmp r4, #3
  bgt.n _08025DAE
  adds r0, r6, #0
  adds r1, r4, #0
  bl 0x08025D64
  cmp r0, #11
  bne.n _08025DAE
  adds r5, #1
  adds r4, #1
  b.n _08025D98
_08025DAE:
  cmp r5, #3
  ble.n _08025DB4
  movs r5, #3
_08025DB4:
  adds r0, r5, #0
  pop {r4, r5, r6}
  pop {r1}
  bx r1

@ Region end. The last body (_08025D90) runs to exactly here, and this file is
@ self-terminated with no.include, so this anchor is its precise end marker.
ai_grid_end:
