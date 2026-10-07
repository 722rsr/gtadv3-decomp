@ GT Advance 3 - menu record handlers II
@ Region: file offset 0x00E650-0x00EBD8 (VMA 0x0800E650-0x0800EBD8).
@ Pure Thumb, ARMCC; continuation of the menu span (menu_dbe8.s ends at
@ 0xE650; next raw resumes at 0xEBD8).
@
@ Function map:
@   sub_0800E650 - record selector: clears rec+0x10A halfword, then
@                  dispatches on s16([sub_08004B68+2] - 16) via the
@                  34-entry jump table @0xE69C (base word 0x0800E69C
@                  @0xE698); cases write 5 (0xE724), 6 (0xE790 default)
@                  or 1 (0xE788/0xE74C) into rec+0x54. 0xE72C gates the
@                  5-write on s16[0x03001780+0xFBC] == 0; 0xE74C gates
@                  the 1-write on the packed grid cell at
@                  0x03001780 + s16*2 + s16*8 + 0xFD0.
@   sub_0800E7A0 - bx lr stub (unreferenced).
@   sub_0800E7A4 - bx lr stub (called from sub_0800E7CC tail).
@   sub_0800E7A8 - tiny: rec+0xE8/+0xEA <- 1, then bl 0x08002124
@                  (block B, param 0x1393), rec+0xE8 <- 2.
@   sub_0800E7CC - sl/r9/r8 menu record initializer: sound 0x32 via
@                  sub_0802B214, state cell 0x03001780+0xFBC -> rec+0xE4,
@                  sub_08007770 pairs, sub_0800DAB8 twin init, three
@                  sub_08007614 lanes, sub_080075E8 rows, three
@                  sub_0800572C allocs -> rec+0x10C/+0x118/+0x124
@                  + sub_08007ABC binds, sub_0800798C copies from
@                  resources 0x082C1268/0x082B7410/0x082A798C/
@                  0x082C4228/0x082C4458/0x082C5040/0x082D0DC0,
@                  ten sub_0800D77C field clears, music select
@                  (0x03001780+0xFBC == 7 ? +0xFC2 : +0x574) ->
@                  rec+0xFA via sub_080022E4, sub_080024D74/24E34
@                  track cells, 0x082D0DC0+idx*12+0x30/+0x31
@                  class/rank bytes, four sub_080024C3C/24C58/
@                  24D5C record binds, sub_0800D9A4 setup, then
@                  state switch on rec+0xE4 -> sub_0800E7A8 (==1) /
@                  sub_0800E7A4 (==2).
@
@ Transcribed from baserom.gba via objdump; byte-exact (make SHA gate).
@ Unconverted targets keep numeric `bl 0x0800XXXX` form. Pools at
@ original offsets.

.thumb

@ ----------------------------------------------------------------------------
.type sub_0800E650, %function
sub_0800E650:
_0800E650:
  push {r4, lr}
  adds r4, r1, #0
  movs r2, #133
  lsls r2, r2, #1
  adds r1, r0, r2
  movs r0, #0
  strh r0, [r1, #0]
  ldr r0, _0800E690
  ldr r3, _0800E694
  adds r0, r0, r3
  ldrh r0, [r0, #0]
  cmp r0, #3
  bne _0800E672
  adds r1, r4, #0
  adds r1, #88
  movs r0, #0
  strb r0, [r1, #0]
_0800E672:
  bl sub_08004B68
  ldrh r0, [r0, #2]
  subs r0, #16
  lsls r0, r0, #16
  asrs r0, r0, #16
  cmp r0, #33
  bls _0800E684
  b _0800E790
_0800E684:
  lsls r0, r0, #2
  ldr r1, _0800E698
  adds r0, r0, r1
  ldr r0, [r0, #0]
  mov pc, r0
  .align 2, 0
_0800E690: .4byte 0x03001780
_0800E694: .4byte 0x00000FBC
_0800E698: .4byte 0x0800E69C   @ jump table base (points at first entry)
@ 34-entry jump table: idx = s16(value - 16)
_0800E69C: .4byte _0800E724
_0800E6A0: .4byte _0800E724
_0800E6A4: .4byte _0800E724
_0800E6A8: .4byte _0800E724
_0800E6AC: .4byte _0800E724
_0800E6B0: .4byte _0800E72C
_0800E6B4: .4byte _0800E724
_0800E6B8: .4byte _0800E790
_0800E6BC: .4byte _0800E790
_0800E6C0: .4byte _0800E790
_0800E6C4: .4byte _0800E790
_0800E6C8: .4byte _0800E788
_0800E6CC: .4byte _0800E788
_0800E6D0: .4byte _0800E788
_0800E6D4: .4byte _0800E788
_0800E6D8: .4byte _0800E74C
_0800E6DC: .4byte _0800E788
_0800E6E0: .4byte _0800E724
_0800E6E4: .4byte _0800E790
_0800E6E8: .4byte _0800E724
_0800E6EC: .4byte _0800E724
_0800E6F0: .4byte _0800E790
_0800E6F4: .4byte _0800E790
_0800E6F8: .4byte _0800E788
_0800E6FC: .4byte _0800E790
_0800E700: .4byte _0800E790
_0800E704: .4byte _0800E790
_0800E708: .4byte _0800E790
_0800E70C: .4byte _0800E790
_0800E710: .4byte _0800E790
_0800E714: .4byte _0800E790
_0800E718: .4byte _0800E790
_0800E71C: .4byte _0800E790
_0800E720: .4byte _0800E788
@ case {0..4,6,17,19,20} -> store 5
_0800E724:
  adds r1, r4, #0
  adds r1, #84
  movs r0, #5
  b _0800E796
@ case 5: gate on s16[0x03001780+0xFBC] == 0
_0800E72C:
  ldr r0, _0800E744
  ldr r1, _0800E748
  adds r0, r0, r1
  movs r2, #0
  ldrsh r0, [r0, r2]
  cmp r0, #0
  beq _0800E790
  adds r1, r4, #0
  adds r1, #84
  movs r0, #5
  b _0800E796
  .align 2, 0
_0800E744: .4byte 0x03001780
_0800E748: .4byte 0x00000FBC
@ case 15: gate on packed grid cell @ 0x03001780 + a*2 + b*8 + 0xFD0
_0800E74C:
  ldr r2, _0800E77C
  ldr r3, _0800E780
  adds r0, r2, r3
  movs r3, #0
  ldrsh r1, [r0, r3]
  lsls r1, r1, #1
  ldr r3, _0800E784
  adds r0, r2, r3
  movs r3, #0
  ldrsh r0, [r0, r3]
  lsls r0, r0, #3
  adds r1, r1, r0
  movs r0, #253
  lsls r0, r0, #4
  adds r2, r2, r0
  adds r1, r1, r2
  movs r2, #0
  ldrsh r0, [r1, r2]
  cmp r0, #0
  bne _0800E790
  adds r1, r4, #0
  adds r1, #84
  movs r0, #1
  b _0800E796
  .align 2, 0
_0800E77C: .4byte 0x03001780
_0800E780: .4byte 0x00000FF6
_0800E784: .4byte 0x00000FF2
@ case {11..14,16,23,33} -> store 1
_0800E788:
  adds r1, r4, #0
  adds r1, #84
  movs r0, #1
  b _0800E796
@ default / all others -> store 6
_0800E790:
  adds r1, r4, #0
  adds r1, #84
  movs r0, #6
_0800E796:
  strh r0, [r1, #0]
  pop {r4}
  pop {r0}
  bx r0
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_0800E7A0, %function
sub_0800E7A0:
_0800E7A0:
  bx lr
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_0800E7A4, %function
sub_0800E7A4:
_0800E7A4:
  bx lr
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_0800E7A8, %function
sub_0800E7A8:
_0800E7A8:
  push {r4, lr}
  adds r4, r0, #0
  adds r4, #232
  movs r1, #1
  strh r1, [r4, #0]
  adds r0, #234
  strh r1, [r0, #0]
  ldr r0, _0800E7C8
  bl 0x08002124
  movs r0, #2
  strh r0, [r4, #0]
  pop {r4}
  pop {r0}
  bx r0
  .align 2, 0
_0800E7C8: .4byte 0x00001393

@ ----------------------------------------------------------------------------
.type sub_0800E7CC, %function
sub_0800E7CC:
_0800E7CC:
  push {r4, r5, r6, r7, lr}
  mov r7, sl
  mov r6, r9
  mov r5, r8
  push {r5, r6, r7}
  sub sp, #8
  adds r7, r0, #0
  movs r0, #50
  bl sub_0802B214
  ldr r0, _0800E7FC
  ldr r1, _0800E800
  adds r0, r0, r1
  movs r2, #0
  ldrsh r0, [r0, r2]
  cmp r0, #3
  beq _0800E804
  cmp r0, #7
  beq _0800E80C
  adds r1, r7, #0
  adds r1, #228
  movs r0, #0
  b _0800E812
  .align 2, 0
_0800E7FC: .4byte 0x03001780
_0800E800: .4byte 0x00000FBC
_0800E804:
  adds r1, r7, #0
  adds r1, #228
  movs r0, #1
  b _0800E812
_0800E80C:
  adds r1, r7, #0
  adds r1, #228
  movs r0, #2
_0800E812:
  strh r0, [r1, #0]
  mov r9, r1
  mov r3, r9
  movs r0, #0
  ldrsh r2, [r3, r0]
  cmp r2, #1
  beq _0800E848
  cmp r2, #1
  bgt _0800E82A
  cmp r2, #0
  beq _0800E82E
  b _0800E86E
_0800E82A:
  cmp r2, #2
  bne _0800E86E
_0800E82E:
  ldr r1, _0800E844
  movs r0, #4
  str r0, [sp, #0]
  movs r0, #1
  str r0, [sp, #4]
  movs r0, #0
  movs r2, #1
  movs r3, #0
  bl sub_08007770
  b _0800E86E
  .align 2, 0
_0800E844: .4byte 0x082C1268
_0800E848:
  ldr r1, _0800E9E8
  movs r0, #4
  str r0, [sp, #0]
  str r2, [sp, #4]
  movs r0, #0
  movs r2, #2
  movs r3, #0
  bl sub_08007770
  ldr r1, _0800E9EC
  movs r0, #0
  str r0, [sp, #0]
  movs r0, #3
  str r0, [sp, #4]
  movs r0, #1
  movs r2, #2
  movs r3, #0
  bl sub_08007770
_0800E86E:
  movs r1, #6
  mov sl, r1
  str r1, [r7, #112]
  movs r0, #3
  str r0, [r7, #124]
  adds r0, r7, #0
  adds r0, #136
  movs r2, #1
  mov r8, r2
  str r2, [r0, #0]
  adds r6, r7, #0
  adds r6, #40
  adds r0, r6, #0
  bl sub_0800DAB8
  ldr r4, _0800E9F0
  adds r0, r4, #0
  movs r1, #1
  movs r2, #0
  movs r3, #3
  bl sub_08007614
  adds r0, r4, #0
  movs r1, #1
  movs r2, #1
  movs r3, #4
  bl sub_08007614
  adds r0, r4, #0
  movs r1, #1
  movs r2, #2
  movs r3, #5
  bl sub_08007614
  adds r0, r4, #0
  movs r1, #0
  movs r2, #6
  bl sub_080075E8
  adds r0, r4, #0
  movs r1, #2
  movs r2, #7
  bl sub_080075E8
  ldr r0, _0800E9F4
  movs r1, #0
  movs r2, #8
  bl sub_080075E8
  ldr r0, _0800E9F8
  adds r1, r7, #0
  adds r1, #8
  bl sub_0800798C
  movs r0, #7
  bl sub_0800572C
  movs r3, #134
  lsls r3, r3, #1
  adds r1, r7, r3
  str r0, [r1, #0]
  movs r2, #136
  lsls r2, r2, #1
  adds r0, r7, r2
  movs r5, #2
  str r5, [r0, #0]
  ldr r0, [r7, #12]
  ldr r2, [r1, #0]
  movs r1, #2
  bl sub_08007ABC
  ldr r0, _0800E9FC
  adds r1, r7, #0
  adds r1, #16
  bl sub_0800798C
  movs r0, #14
  bl sub_0800572C
  movs r3, #140
  lsls r3, r3, #1
  adds r2, r7, r3
  str r0, [r2, #0]
  movs r0, #142
  lsls r0, r0, #1
  adds r1, r7, r0
  movs r0, #5
  str r0, [r1, #0]
  ldr r0, [r7, #20]
  ldr r2, [r2, #0]
  movs r1, #5
  bl sub_08007ABC
  ldr r0, _0800EA00
  adds r1, r7, #0
  adds r1, #24
  bl sub_0800798C
  movs r0, #7
  bl sub_0800572C
  movs r1, #146
  lsls r1, r1, #1
  adds r2, r7, r1
  str r0, [r2, #0]
  movs r3, #148
  lsls r3, r3, #1
  adds r1, r7, r3
  movs r0, #50
  str r0, [r1, #0]
  ldr r0, [r7, #28]
  ldr r2, [r2, #0]
  movs r1, #50
  bl sub_08007ABC
  ldr r4, _0800E9E8
  adds r0, r4, #0
  adds r1, r7, #0
  bl sub_0800798C
  adds r0, r7, #0
  bl sub_08007A58
  adds r0, r4, #0
  movs r1, #0
  movs r2, #9
  bl sub_080075E8
  ldr r4, _0800E9EC
  adds r1, r7, #0
  adds r1, #32
  adds r0, r4, #0
  bl sub_0800798C
  adds r0, r4, #0
  movs r1, #0
  movs r2, #10
  bl sub_080075E8
  movs r0, #152
  lsls r0, r0, #1
  adds r4, r7, r0
  adds r0, r4, #0
  movs r1, #5
  bl 0x08025BC8
  ldr r0, [r7, #36]
  movs r2, #154
  lsls r2, r2, #1
  adds r1, r7, r2
  ldr r1, [r1, #0]
  ldr r2, [r4, #0]
  bl sub_08007ABC
  movs r3, #130
  lsls r3, r3, #1
  adds r1, r7, r3
  str r5, [r1, #0]
  adds r0, r7, #0
  adds r0, #152
  str r1, [r0, #0]
  adds r0, #4
  str r6, [r0, #0]
  movs r0, #0
  str r0, [r7, #40]
  mov r0, r8
  strh r0, [r7, #44]
  adds r0, r7, #0
  adds r0, #60
  movs r2, #32
  negs r2, r2
  movs r1, #0
  bl sub_0800D77C
  adds r0, r7, #0
  adds r0, #52
  movs r1, #0
  movs r2, #160
  bl sub_0800D77C
  ldr r0, _0800EA04
  ldr r1, _0800EA08
  adds r0, r0, r1
  ldrh r0, [r0, #0]
  cmp r0, #1
  bne _0800EA0C
  movs r0, #8
  strh r0, [r7, #48]
  b _0800EA10
  .align 2, 0
_0800E9E8: .4byte 0x082C1268
_0800E9EC: .4byte 0x082B7410
_0800E9F0: .4byte 0x082A798C
_0800E9F4: .4byte 0x082C4228
_0800E9F8: .4byte 0x082C4458
_0800E9FC: .4byte 0x082C5040
_0800EA00: .4byte 0x082D0DC0
_0800EA04: .4byte 0x03001780
_0800EA08: .4byte 0x00000FBC
_0800EA0C:
  mov r2, sl
  strh r2, [r7, #48]
_0800EA10:
  movs r2, #0
  movs r0, #5
  strh r0, [r7, #50]
  movs r3, #133
  lsls r3, r3, #1
  adds r0, r7, r3
  strh r2, [r0, #0]
  adds r1, r7, #0
  adds r1, #224
  strh r2, [r1, #0]
  ldr r2, _0800EA38
  ldr r3, _0800EA3C
  adds r0, r2, r3
  adds r6, r1, #0
  ldrh r0, [r0, #0]
  cmp r0, #7
  bne _0800EA44
  ldr r1, _0800EA40
  adds r0, r2, r1
  b _0800EA48
  .align 2, 0
_0800EA38: .4byte 0x03001780
_0800EA3C: .4byte 0x00000FBC
_0800EA40: .4byte 0x00000FC2
_0800EA44:
  ldr r3, _0800EB9C
  adds r0, r2, r3
_0800EA48:
  ldrh r1, [r0, #0]
  adds r0, r7, #0
  adds r0, #250
  strh r1, [r0, #0]
  adds r5, r0, #0
  movs r1, #0
  ldrsh r0, [r5, r1]
  bl sub_080022E4
  adds r1, r7, #0
  adds r1, #252
  movs r3, #0
  strh r0, [r1, #0]
  ldr r2, _0800EBA0
  movs r0, #0
  ldrsh r1, [r5, r0]
  lsls r0, r1, #1
  adds r0, r0, r1
  lsls r0, r0, #2
  adds r0, r0, r2
  adds r0, #49
  ldrb r0, [r0, #0]
  lsls r0, r0, #24
  asrs r0, r0, #24
  adds r4, r7, #0
  adds r4, #248
  strh r0, [r4, #0]
  movs r0, #0
  ldrsh r1, [r5, r0]
  lsls r0, r1, #1
  adds r0, r0, r1
  lsls r0, r0, #2
  adds r0, r0, r2
  adds r0, #48
  movs r1, #0
  ldrsb r1, [r0, r1]
  movs r2, #132
  lsls r2, r2, #1
  adds r0, r7, r2
  strh r1, [r0, #0]
  strh r3, [r6, #0]
  bl 0x08024D74
  adds r1, r7, #0
  adds r1, #246
  strh r0, [r1, #0]
  bl 0x08024E34
  adds r1, r7, #0
  adds r1, #244
  strh r0, [r1, #0]
  adds r0, r7, #0
  adds r0, #160
  movs r1, #168
  movs r2, #40
  bl sub_0800D77C
  adds r0, r7, #0
  adds r0, #168
  movs r1, #168
  movs r2, #64
  bl sub_0800D77C
  adds r0, r7, #0
  adds r0, #176
  movs r1, #168
  movs r2, #88
  bl sub_0800D77C
  adds r0, r7, #0
  adds r0, #184
  movs r1, #96
  movs r2, #120
  bl sub_0800D77C
  adds r0, r7, #0
  adds r0, #192
  movs r1, #152
  movs r2, #120
  bl sub_0800D77C
  adds r0, r7, #0
  adds r0, #200
  movs r1, #176
  movs r2, #48
  bl sub_0800D77C
  adds r0, r7, #0
  adds r0, #208
  movs r1, #176
  movs r2, #72
  bl sub_0800D77C
  adds r0, r7, #0
  adds r0, #216
  movs r1, #176
  movs r2, #96
  bl sub_0800D77C
  movs r3, #158
  lsls r3, r3, #1
  adds r0, r7, r3
  movs r2, #0
  ldrsh r1, [r5, r2]
  movs r3, #0
  ldrsh r2, [r4, r3]
  bl sub_0800D9A4
  movs r1, #0
  ldrsh r0, [r5, r1]
  bl 0x08024C3C
  adds r1, r0, #0
  movs r2, #142
  lsls r2, r2, #1
  adds r0, r7, r2
  str r1, [r0, #0]
  ldr r0, [r7, #20]
  movs r3, #140
  lsls r3, r3, #1
  adds r2, r7, r3
  ldr r2, [r2, #0]
  bl sub_08007ABC
  movs r1, #0
  ldrsh r0, [r5, r1]
  bl 0x08024D5C
  adds r1, r0, #0
  movs r2, #136
  lsls r2, r2, #1
  adds r0, r7, r2
  str r1, [r0, #0]
  ldr r0, [r7, #12]
  movs r3, #134
  lsls r3, r3, #1
  adds r2, r7, r3
  ldr r2, [r2, #0]
  bl sub_08007ABC
  movs r1, #0
  ldrsh r0, [r5, r1]
  bl 0x08024C58
  adds r1, r0, #0
  movs r2, #148
  lsls r2, r2, #1
  adds r0, r7, r2
  str r1, [r0, #0]
  ldr r0, [r7, #28]
  movs r3, #146
  lsls r3, r3, #1
  adds r2, r7, r3
  ldr r2, [r2, #0]
  bl sub_08007ABC
  adds r1, r7, #0
  adds r1, #236
  movs r0, #2
  strh r0, [r1, #0]
  mov r0, r9
  movs r2, #0
  ldrsh r1, [r0, r2]
  cmp r1, #1
  beq _0800EBB0
  cmp r1, #1
  bgt _0800EBA4
  cmp r1, #0
  beq _0800EBA8
  b _0800EBB6
  .align 2, 0
_0800EB9C: .4byte 0x00000574
_0800EBA0: .4byte 0x03001780
_0800EBA4:
  cmp r1, #2
  bne _0800EBB6
_0800EBA8:
  adds r0, r7, #0
  bl sub_0800E7A4
  b _0800EBB6
_0800EBB0:
  adds r0, r7, #0
  bl sub_0800E7A8
_0800EBB6:
  movs r3, #128
  lsls r3, r3, #1
  adds r0, r7, r3
  movs r1, #0
  strh r1, [r0, #0]
  movs r2, #129
  lsls r2, r2, #1
  adds r0, r7, r2
  strh r1, [r0, #0]
  add sp, #8
  pop {r3, r4, r5}
  mov r8, r3
  mov r9, r4
  mov sl, r5
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .align 2, 0
