@ GT Advance 3 - collection-grid record write/read accessors
@ Region: file offset 0x025E70-0x025EC0 (VMA 0x08025E70-0x08025EC0).
@ Exact Thumb leaves with the shared collection-grid base pool.

.thumb
.type sub_08025E70, %function
sub_08025E70:
_08025E70:
  push {r4, r5, lr}
  ldr r5, _08025E94
  lsls r4, r1, #1
  adds r4, r4, r1
  adds r4, r4, r2
  lsls r4, r4, #2
  lsls r1, r0, #1
  adds r1, r1, r0
  lsls r1, r1, #4
  adds r4, r4, r1
  movs r0, #176
  lsls r0, r0, #3
  adds r5, r5, r0
  adds r4, r4, r5
  str r3, [r4, #0]
  pop {r4, r5}
  pop {r0}
  bx r0
_08025E94: .word 0x03001780

.type sub_08025E98, %function
sub_08025E98:
_08025E98:
  push {r4, lr}
  ldr r4, _08025EBC
  lsls r3, r1, #1
  adds r3, r3, r1
  adds r3, r3, r2
  lsls r3, r3, #2
  lsls r1, r0, #1
  adds r1, r1, r0
  lsls r1, r1, #4
  adds r3, r3, r1
  movs r0, #176
  lsls r0, r0, #3
  adds r4, r4, r0
  adds r3, r3, r4
  ldr r0, [r3, #0]
  pop {r4}
  pop {r1}
  bx r1
_08025EBC: .word 0x03001780

ai_grid_more3_end:
