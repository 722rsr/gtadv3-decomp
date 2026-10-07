@ GT Advance 3 - packed collection-grid read accessor
@ Region: file offset 0x025F20-0x025F78 (VMA 0x08025F20-0x08025F78).
@ Exact Thumb leaf with mask-table and IWRAM-base pools.

.thumb
.type sub_08025F20, %function
sub_08025F20:
_08025F20:
  push {r4, r5, lr}
  sub sp, #4
  adds r4, r0, #0
  ldr r1, _08025F70
  mov r0, sp
  movs r2, #4
  bl 0x0802E0A4
  movs r0, #7
  ands r4, r0
  adds r0, r4, #0
  movs r1, #4
  bl 0x0802D97C
  adds r5, r0, #0
  adds r0, r4, #0
  movs r1, #4
  bl 0x0802D97C
  adds r2, r0, #0
  ldr r1, _08025F74
  asrs r0, r4, #2
  lsls r0, r0, #1
  movs r3, #174
  lsls r3, r3, #3
  adds r1, r1, r3
  adds r0, r0, r1
  movs r1, #0
  ldrsh r0, [r0, r1]
  mov r3, sp
  adds r1, r3, r5
  ldrb r1, [r1, #0]
  ands r0, r1
  lsls r1, r2, #1
  asrs r0, r1
  add sp, #4
  pop {r4, r5}
  pop {r1}
  bx r1
  .short 0x0000
_08025F70: .word 0x08060D48
_08025F74: .word 0x03001780
