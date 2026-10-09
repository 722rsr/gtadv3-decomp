@ GT Advance 3 - packed collection-grid halfword update
@ Region: file offset 0x025EC0-0x025F20 (VMA 0x08025EC0-0x08025F20).
@ Exact Thumb leaf with collection-grid and mask-table pools.

.thumb
.type sub_08025EC0, %function
sub_08025EC0:
_08025EC0:
  push {r4, r5, r6, lr}
  sub sp, #4
  adds r4, r0, #0
  adds r6, r1, #0
  ldr r1, _08025F18
  mov r0, sp
  movs r2, #4
  bl 0x0802E0A4
  movs r0, #7
  ands r4, r0
  adds r0, r4, #0
  movs r1, #4
  bl 0x0802D97C
  adds r2, r0, #0
  ldr r1, _08025F1C
  adds r0, r4, #0
  asrs r0, r0, #2
  lsls r0, r0, #1
  movs r3, #174
  lsls r3, r3, #3
  adds r1, r1, r3
  adds r5, r0, r1
  mov r1, sp
  adds r0, r1, r2
  ldrh r3, [r5, #0]
  ldrb r0, [r0, #0]
  bics r3, r0
  adds r0, r3, #0
  strh r0, [r5, #0]
  adds r0, r4, #0
  movs r1, #4
  bl 0x0802D97C
  lsls r0, r0, #1
  lsls r6, r0
  ldrh r0, [r5, #0]
  orrs r6, r0
  strh r6, [r5, #0]
  add sp, #4
  pop {r4, r5, r6}
  pop {r0}
  bx r0
_08025F18: .word 0x08060D48
_08025F1C: .word 0x03001780

@ Byte-neutral end anchor for the splicer. This file declares exactly one
@ `@ Region:` (0x025EC0-0x025F20) and has no `.include`; the body at
@ 0x08025EC0 runs to 0x025F20, exactly that boundary, so the anchor is safe.
@ Without it the body is refused as 'no end marker in ai_grid_more4.s'.
ai_grid_more4_end:
