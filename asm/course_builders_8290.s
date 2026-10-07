@ GT Advance 3 - course record-builder cluster
@ Region: file offset 0x008290-0x008AAC (VMA 0x08008290-0x08008AAC).
@ Continuation in course_dispatch_8aac.s at 0x8AAC.
@ Pure Thumb; the `sub_08008290` marker (course_load.s callee) and its
@ five successors: record-area initializer/filler, two high-register
@ 32-row/16-row record builders, a type-dispatch fill, and three
@ high-register 8-record emitters (table-driven, 8-byte records).
@
@ Function map:
@   sub_08008290 - record-area init: 32 x {value+0x2A8 header sum} stride-2
@                  on the 9th record set, then 11 x 2 x CpuSet(32 B) copies
@                  from the two ROM tables into the sp frame.
@   sub_08008300 - 32-row record builder (u8 select -> 7/8), CpuSet row
@                  fills with lane masks + 0xD000 base, stride 22 rows.
@   sub_080083B8 - 16-row record builder (0/2/3), template 0x0805F624,
@                  CpuSet fills with masks, stride 30 rows.
@   sub_08008480 - mode/type dispatch fill: s8 mode selector, switches on
@                  `s16[0x03001780+0x10FC]` {0,2,5} -> sub_08008300/B8
@                  variants, then fills record fields (award/part/stream
@                  ids via sub_080261B0/F8) + 5 x sub_08007538/7EC4/7ABC
@                  lane writes.
@   sub_0800861C/0x8768/0x88B0 - table-driven 8-record emitters: base value
@                  via sub_0802D97C/978 (udiv-style ratio), types
@                  {0x5A,0x62,0x6A,0x72,0x7A,0x82,0x8A,0x92} etc., packed
@                  u16 cell {x + offset, y<<12} from the global record
@                  [+0x34]/[+0x36] fields; 0x8768 uses immediate 0x8000
@                  base, 0x88B0 pool base 0x0000808F.
@
@ Transcribed from baserom.gba via objdump; byte-exact (make SHA gate).
@ Unconverted targets keep numeric `bl 0x0800XXXX` form exactly like
@ neighboring files. Pools at original offsets.

.thumb

@ ----------------------------------------------------------------------------
.type sub_08008290, %function
sub_08008290:
_08008290:
  push {r4, r5, r6, lr}
  sub sp, #64
  ldr r4, _080082F0
  ldr r2, _080082F4
  adds r0, r4, #0
  movs r1, #9
  bl sub_080078EC
  adds r0, r4, #0
  movs r1, #9
  bl sub_08007924
  adds r2, r0, #0
  movs r0, #170
  lsls r0, r0, #2
  adds r3, r0, #0
  mov r1, sp
  movs r4, #31
_080082B4:
  ldrb r5, [r2, #0]
  adds r0, r3, r5
  strh r0, [r1, #0]
  adds r1, #2
  subs r4, #1
  cmp r4, #0
  bge _080082B4
  ldr r6, _080082F8
  ldr r5, _080082FC
  movs r4, #11
_080082C8:
  mov r0, sp
  adds r1, r5, #0
  movs r2, #32
  bl sub_0802D974
  mov r0, sp
  adds r1, r6, #0
  movs r2, #32
  bl sub_0802D974
  adds r6, #64
  adds r5, #64
  subs r4, #1
  cmp r4, #0
  bge _080082C8
  add sp, #64
  pop {r4, r5, r6}
  pop {r0}
  bx r0
  .align 2, 0
_080082F0: .4byte 0x0828FF08
_080082F4: .4byte 0x0600D500
_080082F8: .4byte 0x0600FA00
_080082FC: .4byte 0x0600F200

@ ----------------------------------------------------------------------------
.type sub_08008300, %function
sub_08008300:
_08008300:
  push {r4, r5, r6, r7, lr}
  mov r7, sl
  mov r6, r9
  mov r5, r8
  push {r5, r6, r7}
  sub sp, #68
  lsls r0, r0, #24
  movs r5, #8
  cmp r0, #0
  beq _08008316
  movs r5, #7
_08008316:
  ldr r4, _080083A4
  ldr r2, _080083A8
  adds r0, r4, #0
  adds r1, r5, #0
  bl sub_080078EC
  adds r0, r4, #0
  adds r1, r5, #0
  movs r2, #13
  bl sub_08007938
  adds r0, r4, #0
  adds r1, r5, #0
  bl sub_08007924
  str r0, [sp, #64]
  movs r4, #0
  ldr r0, _080083AC
  mov sl, r0
  movs r1, #192
  lsls r1, r1, #4
  mov r9, r1
  ldr r0, _080083B0
  mov r8, r0
  movs r1, #208
  lsls r1, r1, #8
  adds r7, r1, #0
_0800834C:
  movs r0, #22
  muls r0, r4
  ldr r1, [sp, #64]
  adds r0, r1, r0
  mov r1, sp
  movs r2, #11
  bl sub_0802D974
  mov r3, sp
  adds r5, r4, #0
  adds r5, #14
  adds r6, r4, #1
  movs r4, #31
_08008366:
  ldrh r2, [r3, #0]
  mov r1, sl
  ands r1, r2
  mov r0, r9
  ands r0, r2
  add r1, r8
  orrs r0, r7
  orrs r1, r0
  strh r1, [r3, #0]
  subs r4, #1
  adds r3, #2
  cmp r4, #0
  bge _08008366
  lsls r1, r5, #6
  ldr r0, _080083B4
  adds r1, r1, r0
  mov r0, sp
  movs r2, #11
  bl sub_0802D974
  adds r4, r6, #0
  cmp r4, #5
  ble _0800834C
  add sp, #68
  pop {r3, r4, r5}
  mov r8, r3
  mov r9, r4
  mov sl, r5
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .align 2, 0
_080083A4: .4byte 0x0828FF08
_080083A8: .4byte 0x0600D520
_080083AC: .4byte 0x000003FF
_080083B0: .4byte 0x000002A9
_080083B4: .4byte 0x0600F026

@ ----------------------------------------------------------------------------
.type sub_080083B8, %function
sub_080083B8:
_080083B8:
  push {r4, r5, r6, r7, lr}
  mov r7, sl
  mov r6, r9
  mov r5, r8
  push {r5, r6, r7}
  sub sp, #84
  add r3, sp, #64
  adds r2, r3, #0
  ldr r1, _08008468
  ldmia r1!, {r4, r5, r6}
  stmia r2!, {r4, r5, r6}
  ldr r1, [r1, #0]
  str r1, [r2, #0]
  lsls r0, r0, #2
  adds r3, r3, r0
  ldr r5, [r3, #0]
  ldr r4, _0800846C
  ldr r2, _08008470
  adds r0, r4, #0
  adds r1, r5, #0
  bl sub_080078EC
  adds r0, r4, #0
  adds r1, r5, #0
  movs r2, #12
  bl sub_08007938
  adds r0, r4, #0
  adds r1, r5, #0
  bl sub_08007924
  str r0, [sp, #80]
  movs r4, #0
  ldr r0, _08008474
  mov sl, r0
  movs r1, #192
  lsls r1, r1, #4
  mov r9, r1
  ldr r5, _08008478
  mov r8, r5
  movs r6, #192
  lsls r6, r6, #8
  adds r7, r6, #0
_0800840E:
  lsls r0, r4, #4
  subs r0, r0, r4
  lsls r0, r0, #1
  ldr r1, [sp, #80]
  adds r0, r1, r0
  mov r1, sp
  movs r2, #15
  bl sub_0802D974
  mov r3, sp
  adds r5, r4, #0
  adds r5, #18
  adds r6, r4, #1
  movs r4, #31
_0800842A:
  ldrh r2, [r3, #0]
  mov r1, sl
  ands r1, r2
  mov r0, r9
  ands r0, r2
  add r1, r8
  orrs r0, r7
  orrs r1, r0
  strh r1, [r3, #0]
  subs r4, #1
  adds r3, #2
  cmp r4, #0
  bge _0800842A
  lsls r1, r5, #6
  ldr r4, _0800847C
  adds r1, r1, r4
  mov r0, sp
  movs r2, #15
  bl sub_0802D974
  adds r4, r6, #0
  cmp r4, #1
  ble _0800840E
  add sp, #84
  pop {r3, r4, r5}
  mov r8, r3
  mov r9, r4
  mov sl, r5
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .align 2, 0
_08008468: .4byte 0x0805F624
_0800846C: .4byte 0x0828FF08
_08008470: .4byte 0x0600DB60
_08008474: .4byte 0x000003FF
_08008478: .4byte 0x000002DB
_0800847C: .4byte 0x0600F000

@ ----------------------------------------------------------------------------
.type sub_08008480, %function
sub_08008480:
_08008480:
  push {r4, r5, lr}
  sub sp, #4
  adds r3, r0, #0
  lsls r4, r1, #24
  lsrs r4, r4, #24
  ldr r0, _080084AC
  str r3, [r0, #0]
  movs r0, #0
  str r0, [sp, #0]
  ldr r2, _080084B0
  mov r0, sp
  adds r1, r3, #0
  bl sub_0802D974
  lsls r4, r4, #24
  asrs r4, r4, #24
  cmp r4, #0
  beq _080084B4
  cmp r4, #1
  beq _08008502
  b _0800850E
  .align 2, 0
_080084AC: .4byte 0x030003E0
_080084B0: .4byte 0x0500001A
_080084B4:
  movs r0, #1
  bl sub_08008300
  ldr r0, _080084D4
  ldr r1, _080084D8
  adds r0, r0, r1
  movs r1, #0
  ldrsh r0, [r0, r1]
  cmp r0, #2
  beq _080084EA
  cmp r0, #2
  bgt _080084DC
  cmp r0, #0
  beq _080084E2
  b _080084FA
  .align 2, 0
_080084D4: .4byte 0x03001780
_080084D8: .4byte 0x000010FC
_080084DC:
  cmp r0, #5
  beq _080084F2
  b _080084FA
_080084E2:
  movs r0, #0
  bl sub_080083B8
  b _0800850E
_080084EA:
  movs r0, #2
  bl sub_080083B8
  b _0800850E
_080084F2:
  movs r0, #3
  bl sub_080083B8
  b _0800850E
_080084FA:
  movs r0, #1
  bl sub_080083B8
  b _0800850E
_08008502:
  movs r0, #0
  bl sub_08008300
  movs r0, #0
  bl sub_080083B8
_0800850E:
  movs r0, #9
  bl sub_080261B0
  ldr r4, _08008608
  ldr r1, [r4, #0]
  strh r0, [r1, #36]
  movs r0, #9
  bl 0x080261F8
  ldr r1, [r4, #0]
  strh r0, [r1, #38]
  movs r0, #9
  bl sub_080261B0
  ldr r1, [r4, #0]
  adds r0, #43
  strh r0, [r1, #48]
  movs r0, #9
  bl sub_080261B0
  ldr r1, [r4, #0]
  adds r0, #47
  strh r0, [r1, #50]
  movs r0, #15
  bl sub_080261B0
  ldr r1, [r4, #0]
  strh r0, [r1, #52]
  movs r0, #15
  bl 0x080261F8
  ldr r1, [r4, #0]
  strh r0, [r1, #54]
  ldr r5, _0800860C
  adds r1, #88
  adds r0, r5, #0
  bl sub_0800798C
  ldr r1, [r4, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #36]
  bl sub_08007A04
  ldr r0, [r4, #0]
  ldrh r2, [r0, #38]
  adds r0, r5, #0
  movs r1, #2
  bl sub_080075E8
  ldr r0, [r4, #0]
  adds r0, #77
  movs r1, #1
  strb r1, [r0, #0]
  ldr r0, [r4, #0]
  adds r0, #76
  strb r1, [r0, #0]
  ldr r0, [r4, #0]
  adds r0, #88
  movs r1, #10
  bl sub_08007EC4
  adds r1, r0, #0
  ldr r0, [r4, #0]
  strh r1, [r0, #44]
  adds r0, #88
  movs r1, #11
  bl sub_08007EC4
  ldr r1, [r4, #0]
  strh r0, [r1, #46]
  ldr r0, _08008610
  ldr r1, _08008614
  adds r0, r0, r1
  ldrh r0, [r0, #0]
  cmp r0, #5
  bne _08008600
  movs r0, #3
  bl sub_080261B0
  ldr r2, [r4, #0]
  strh r0, [r2, #56]
  adds r1, r0, #0
  adds r1, #32
  strh r1, [r2, #60]
  adds r1, #32
  strh r1, [r2, #40]
  adds r0, #104
  strh r0, [r2, #42]
  movs r0, #2
  bl sub_080261B0
  ldr r1, [r4, #0]
  strh r0, [r1, #62]
  movs r0, #3
  bl 0x080261F8
  ldr r1, [r4, #0]
  strh r0, [r1, #58]
  ldr r0, _08008618
  adds r1, #96
  bl sub_0800798C
  ldr r1, [r4, #0]
  ldr r0, [r1, #100]
  ldrh r2, [r1, #60]
  movs r1, #10
  bl sub_08007ABC
  ldr r0, [r4, #0]
  ldrh r2, [r0, #40]
  adds r0, r5, #0
  movs r1, #0
  bl sub_08007538
  ldr r0, [r4, #0]
  ldrh r2, [r0, #42]
  adds r0, r5, #0
  movs r1, #1
  bl sub_08007538
_08008600:
  add sp, #4
  pop {r4, r5}
  pop {r0}
  bx r0
  .align 2, 0
_08008608: .4byte 0x030003E0
_0800860C: .4byte 0x0828FF08
_08008610: .4byte 0x03001780
_08008614: .4byte 0x000010FC
_08008618: .4byte 0x0828C420

@ ----------------------------------------------------------------------------
.type sub_0800861C, %function
sub_0800861C:
_0800861C:
  push {r4, r5, r6, lr}
  mov r6, sl
  mov r5, r9
  mov r4, r8
  push {r4, r5, r6}
  adds r4, r0, #0
  adds r5, r1, #0
  movs r0, #150
  lsls r0, r0, #1
  mov r8, r0
  adds r0, r5, #0
  mov r1, r8
  bl sub_0802D97C
  movs r1, #3
  bl sub_0802D978
  mov sl, r0
  ldr r6, _0800875C
  adds r0, r5, #0
  adds r1, r6, #0
  bl sub_0802D97C
  mov r1, r8
  bl sub_0802D978
  mov r9, r0
  adds r0, r5, #0
  adds r1, r6, #0
  bl sub_0802D978
  mov r8, r0
  ldr r1, _08008760
  adds r5, r1, #0
  strh r5, [r4, #0]
  movs r0, #90
  strh r0, [r4, #2]
  mov r0, r8
  movs r1, #10
  bl sub_0802D978
  ldr r6, _08008764
  ldr r1, [r6, #0]
  lsls r0, r0, #1
  ldrh r2, [r1, #52]
  adds r0, r2, r0
  ldrh r1, [r1, #54]
  lsls r1, r1, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #98
  strh r0, [r4, #2]
  mov r0, r8
  movs r1, #10
  bl sub_0802D97C
  ldr r2, [r6, #0]
  lsls r0, r0, #1
  ldrh r3, [r2, #52]
  adds r0, r3, r0
  ldrh r3, [r2, #54]
  lsls r1, r3, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #106
  strh r0, [r4, #2]
  ldrh r0, [r2, #52]
  adds r0, #20
  ldrh r2, [r2, #54]
  lsls r1, r2, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #114
  strh r0, [r4, #2]
  mov r0, r9
  movs r1, #10
  bl sub_0802D978
  ldr r1, [r6, #0]
  lsls r0, r0, #1
  ldrh r2, [r1, #52]
  adds r0, r2, r0
  ldrh r1, [r1, #54]
  lsls r1, r1, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #122
  strh r0, [r4, #2]
  mov r0, r9
  movs r1, #10
  bl sub_0802D97C
  ldr r2, [r6, #0]
  lsls r0, r0, #1
  ldrh r3, [r2, #52]
  adds r0, r3, r0
  ldrh r3, [r2, #54]
  lsls r1, r3, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #130
  strh r0, [r4, #2]
  ldrh r0, [r2, #52]
  adds r0, #22
  ldrh r2, [r2, #54]
  lsls r1, r2, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #138
  strh r0, [r4, #2]
  mov r0, sl
  movs r1, #10
  bl sub_0802D978
  ldr r1, [r6, #0]
  lsls r0, r0, #1
  ldrh r2, [r1, #52]
  adds r0, r2, r0
  ldrh r1, [r1, #54]
  lsls r1, r1, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #146
  strh r0, [r4, #2]
  mov r0, sl
  movs r1, #10
  bl sub_0802D97C
  ldr r1, [r6, #0]
  lsls r0, r0, #1
  ldrh r3, [r1, #52]
  adds r0, r3, r0
  ldrh r1, [r1, #54]
  lsls r1, r1, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  adds r0, r4, #0
  pop {r3, r4, r5}
  mov r8, r3
  mov r9, r4
  mov sl, r5
  pop {r4, r5, r6}
  pop {r1}
  bx r1
  .align 2, 0
_0800875C: .4byte 0x00004650
_08008760: .4byte 0x0000803C
_08008764: .4byte 0x030003E0

@ ----------------------------------------------------------------------------
.type sub_08008768, %function
sub_08008768:
_08008768:
  push {r4, r5, r6, lr}
  mov r6, sl
  mov r5, r9
  mov r4, r8
  push {r4, r5, r6}
  adds r4, r0, #0
  adds r5, r1, #0
  movs r0, #150
  lsls r0, r0, #1
  mov r8, r0
  adds r0, r5, #0
  mov r1, r8
  bl sub_0802D97C
  movs r1, #3
  bl sub_0802D978
  mov sl, r0
  ldr r6, _080088A8
  adds r0, r5, #0
  adds r1, r6, #0
  bl sub_0802D97C
  mov r1, r8
  bl sub_0802D978
  mov r9, r0
  adds r0, r5, #0
  adds r1, r6, #0
  bl sub_0802D978
  mov r8, r0
  movs r1, #128
  lsls r1, r1, #8
  adds r5, r1, #0
  strh r5, [r4, #0]
  movs r0, #56
  strh r0, [r4, #2]
  mov r0, r8
  movs r1, #10
  bl sub_0802D978
  ldr r6, _080088AC
  ldr r1, [r6, #0]
  lsls r0, r0, #1
  ldrh r2, [r1, #52]
  adds r0, r2, r0
  ldrh r1, [r1, #54]
  lsls r1, r1, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #64
  strh r0, [r4, #2]
  mov r0, r8
  movs r1, #10
  bl sub_0802D97C
  ldr r2, [r6, #0]
  lsls r0, r0, #1
  ldrh r3, [r2, #52]
  adds r0, r3, r0
  ldrh r3, [r2, #54]
  lsls r1, r3, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #70
  strh r0, [r4, #2]
  ldrh r0, [r2, #52]
  adds r0, #20
  ldrh r2, [r2, #54]
  lsls r1, r2, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #77
  strh r0, [r4, #2]
  mov r0, r9
  movs r1, #10
  bl sub_0802D978
  ldr r1, [r6, #0]
  lsls r0, r0, #1
  ldrh r2, [r1, #52]
  adds r0, r2, r0
  ldrh r1, [r1, #54]
  lsls r1, r1, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #85
  strh r0, [r4, #2]
  mov r0, r9
  movs r1, #10
  bl sub_0802D97C
  ldr r2, [r6, #0]
  lsls r0, r0, #1
  ldrh r3, [r2, #52]
  adds r0, r3, r0
  ldrh r3, [r2, #54]
  lsls r1, r3, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #91
  strh r0, [r4, #2]
  ldrh r0, [r2, #52]
  adds r0, #22
  ldrh r2, [r2, #54]
  lsls r1, r2, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #98
  strh r0, [r4, #2]
  mov r0, sl
  movs r1, #10
  bl sub_0802D978
  ldr r1, [r6, #0]
  lsls r0, r0, #1
  ldrh r2, [r1, #52]
  adds r0, r2, r0
  ldrh r1, [r1, #54]
  lsls r1, r1, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #106
  strh r0, [r4, #2]
  mov r0, sl
  movs r1, #10
  bl sub_0802D97C
  ldr r1, [r6, #0]
  lsls r0, r0, #1
  ldrh r3, [r1, #52]
  adds r0, r3, r0
  ldrh r1, [r1, #54]
  lsls r1, r1, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  adds r0, r4, #0
  pop {r3, r4, r5}
  mov r8, r3
  mov r9, r4
  mov sl, r5
  pop {r4, r5, r6}
  pop {r1}
  bx r1
  .align 2, 0
_080088A8: .4byte 0x00004650
_080088AC: .4byte 0x030003E0

@ ----------------------------------------------------------------------------
.type sub_080088B0, %function
sub_080088B0:
_080088B0:
  push {r4, r5, r6, lr}
  mov r6, sl
  mov r5, r9
  mov r4, r8
  push {r4, r5, r6}
  adds r4, r0, #0
  adds r5, r1, #0
  movs r0, #150
  lsls r0, r0, #1
  mov r8, r0
  adds r0, r5, #0
  mov r1, r8
  bl sub_0802D97C
  movs r1, #3
  bl sub_0802D978
  mov sl, r0
  ldr r6, _080089F0
  adds r0, r5, #0
  adds r1, r6, #0
  bl sub_0802D97C
  mov r1, r8
  bl sub_0802D978
  mov r9, r0
  adds r0, r5, #0
  adds r1, r6, #0
  bl sub_0802D978
  mov r8, r0
  ldr r1, _080089F4
  adds r5, r1, #0
  strh r5, [r4, #0]
  movs r0, #56
  strh r0, [r4, #2]
  mov r0, r8
  movs r1, #10
  bl sub_0802D978
  ldr r6, _080089F8
  ldr r1, [r6, #0]
  lsls r0, r0, #1
  ldrh r2, [r1, #52]
  adds r0, r2, r0
  ldrh r1, [r1, #54]
  lsls r1, r1, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #64
  strh r0, [r4, #2]
  mov r0, r8
  movs r1, #10
  bl sub_0802D97C
  ldr r2, [r6, #0]
  lsls r0, r0, #1
  ldrh r3, [r2, #52]
  adds r0, r3, r0
  ldrh r3, [r2, #54]
  lsls r1, r3, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #70
  strh r0, [r4, #2]
  ldrh r0, [r2, #52]
  adds r0, #20
  ldrh r2, [r2, #54]
  lsls r1, r2, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #77
  strh r0, [r4, #2]
  mov r0, r9
  movs r1, #10
  bl sub_0802D978
  ldr r1, [r6, #0]
  lsls r0, r0, #1
  ldrh r2, [r1, #52]
  adds r0, r2, r0
  ldrh r1, [r1, #54]
  lsls r1, r1, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #85
  strh r0, [r4, #2]
  mov r0, r9
  movs r1, #10
  bl sub_0802D97C
  ldr r2, [r6, #0]
  lsls r0, r0, #1
  ldrh r3, [r2, #52]
  adds r0, r3, r0
  ldrh r3, [r2, #54]
  lsls r1, r3, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #91
  strh r0, [r4, #2]
  ldrh r0, [r2, #52]
  adds r0, #22
  ldrh r2, [r2, #54]
  lsls r1, r2, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #98
  strh r0, [r4, #2]
  mov r0, sl
  movs r1, #10
  bl sub_0802D978
  ldr r1, [r6, #0]
  lsls r0, r0, #1
  ldrh r2, [r1, #52]
  adds r0, r2, r0
  ldrh r1, [r1, #54]
  lsls r1, r1, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  strh r5, [r4, #0]
  movs r0, #106
  strh r0, [r4, #2]
  mov r0, sl
  movs r1, #10
  bl sub_0802D97C
  ldr r1, [r6, #0]
  lsls r0, r0, #1
  ldrh r3, [r1, #52]
  adds r0, r3, r0
  ldrh r1, [r1, #54]
  lsls r1, r1, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
  adds r0, r4, #0
  pop {r3, r4, r5}
  mov r8, r3
  mov r9, r4
  mov sl, r5
  pop {r4, r5, r6}
  pop {r1}
  bx r1
  .align 2, 0
_080089F0: .4byte 0x00004650
_080089F4: .4byte 0x0000808F
_080089F8: .4byte 0x030003E0

@ ----------------------------------------------------------------------------
.type sub_080089FC, %function
sub_080089FC:
_080089FC:
  push {r4, r5, lr}
  adds r2, r0, #0
  ldr r4, _08008A70
  ldr r3, [r4, #0]
  movs r1, #16
  ldrsh r0, [r3, r1]
  cmp r0, #1
  ble _08008A84
  ldr r0, [r3, #84]
  movs r1, #255
  ands r0, r1
  movs r5, #128
  lsls r5, r5, #7
  adds r1, r5, #0
  orrs r0, r1
  strh r0, [r2, #0]
  ldr r0, [r3, #80]
  adds r0, #12
  ldr r5, _08008A74
  adds r1, r5, #0
  ands r0, r1
  ldr r5, _08008A78
  adds r1, r5, #0
  orrs r0, r1
  strh r0, [r2, #2]
  ldrh r1, [r3, #58]
  lsls r0, r1, #12
  ldrh r5, [r3, #60]
  orrs r0, r5
  strh r0, [r2, #4]
  adds r2, #8
  movs r1, #16
  ldrsh r0, [r3, r1]
  cmp r0, #9
  ble _08008A5A
  movs r0, #74
  strh r0, [r2, #0]
  ldr r5, _08008A7C
  adds r0, r5, #0
  strh r0, [r2, #2]
  ldrh r0, [r3, #56]
  adds r0, #16
  ldrh r3, [r3, #58]
  lsls r1, r3, #12
  orrs r0, r1
  strh r0, [r2, #4]
  adds r2, #8
_08008A5A:
  movs r0, #74
  strh r0, [r2, #0]
  ldr r1, _08008A80
  adds r0, r1, #0
  strh r0, [r2, #2]
  ldr r1, [r4, #0]
  ldrh r3, [r1, #58]
  lsls r0, r3, #12
  ldrh r1, [r1, #56]
  orrs r0, r1
  b _08008A98
  .align 2, 0
_08008A70: .4byte 0x030003E0
_08008A74: .4byte 0x000001FF
_08008A78: .4byte 0xFFFFC000
_08008A7C: .4byte 0x00008094
_08008A80: .4byte 0x000080A8
_08008A84:
  ldr r5, _08008AA4
  adds r0, r5, #0
  strh r0, [r2, #0]
  ldr r1, _08008AA8
  adds r0, r1, #0
  strh r0, [r2, #2]
  ldrh r5, [r3, #58]
  lsls r0, r5, #12
  ldrh r3, [r3, #56]
  orrs r0, r3
_08008A98:
  strh r0, [r2, #4]
  adds r2, #8
  adds r0, r2, #0
  pop {r4, r5}
  pop {r1}
  bx r1
  .align 2, 0
_08008AA4: .4byte 0x0000404A
_08008AA8: .4byte 0x0000C0A8
