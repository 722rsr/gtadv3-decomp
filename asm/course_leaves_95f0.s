@ GT Advance 3 - course leaf helpers + small state machines
@ Region: file offset 0x0095F0-0x009B60 (VMA 0x080095F0-0x08009B60).
@ Pure Thumb; continuation of the record-49 phase machine
@ (course_dispatch_8aac.s ends at 0x95F0; menu_pkt.s resumes at 0x9B60).
@
@ Function map:
@   sub_080095F0/0x961C - two flag-gated field writers: read rec[+0x50]
@                  (resp. [+0x48]) and rec[+0xC] (resp. [+0x8]) * 4, then
@                  bl sub_08007570 with {base, rec, field, 4, sp0=4};
@                  gated on rec[+0x4D] (resp. [+0x4C]) flags by
@                  sub_08009648.
@   sub_08009648 - flag-gated dispatcher: if rec[+0x4D] != 0 -> 0x95F0;
@                  if rec[+0x4C] != 0 -> 0x961C.
@   sub_08009674/0x9680 - tiny getter/setter pair over u32[0x030003E4]
@                  (read [r0][0], write [r1][0]).
@   sub_0800968C - s16 fetch: *(u32[0x030003E4] + 20 + (r0>>15)).
@   sub_080096A4 - per-course header init: stores rec ptr at 0x030003E4,
@                  zero-fills [sp] via CpuSet (0x0500000B), fills
@                  [+0xE]/[+0x10]/[+0x12] award ids (sub_080261B0/F8),
@                  template copy 0x0805F604 -> [+0x24], record id x
@                  rows via sub_0800798C/sub_08007A04, two lane writes
@                  sub_08007614, [+0x14]=1, [+0x16]/[+0x18] from
@                  s16 tables 0x03001780+0x10C6/0x10C8.
@   sub_08009748 - countdown stepper on u16[0x030003E4]+0x14/16/18:
@                  value 32 -> +1 step, 16 -> +1, then switch on
@                  [rec+4] {0:+step, 1:+step, 2:+step} clamped to table
@                  0x03001780+0x10CA bounds.
@   sub_080097E8/0x9830/0x98C8 - input event handlers: key bits
@                  {1,2,8,0x40,0x80,0x100,0x200,0x300,0x400} mapped to
@                  counter adjustments on u32[0x030003E4] + a 0..5
@                  return tier; 0x98C8 dispatches 0x97E8/0x9830 on
@                  [rec+0xC] phase.
@   sub_08009900/0x99D0 - course-lap HUD writes: template strings
@                  0x0805F6A4/0x0805F6AA via runtime memcpy, lap label
@                  writes via sub_08007B18, 8 x sub_080038C8/sub_080039C0
@                  color/text rows, 3 x sub_08025500/18/30 grid cells.
@   sub_08009B28 - tiny dispatcher: [rec+0xC] phase 0/1 -> 0x9900/0x99D0.
@   sub_08009B50 - passthrough to sub_08002494 (keypad), masked u16.
@
@ Transcribed from baserom.gba via objdump; byte-exact (make SHA gate).
@ Unconverted targets keep numeric `bl 0x0800XXXX` form. Pools at
@ original offsets.

.thumb

@ ----------------------------------------------------------------------------
.type sub_080095F0, %function
sub_080095F0:
_080095F0:
  push {r4, lr}
  sub sp, #4
  ldr r0, _08009614
  ldr r1, _08009618
  ldr r1, [r1, #0]
  ldrh r2, [r1, #50]
  movs r4, #12
  ldrsh r3, [r1, r4]
  lsls r3, r3, #2
  movs r1, #4
  str r1, [sp, #0]
  movs r1, #0
  bl sub_08007570
  add sp, #4
  pop {r4}
  pop {r0}
  bx r0
  .align 2, 0
_08009614: .4byte 0x0828FF08
_08009618: .4byte 0x030003E0

@ ----------------------------------------------------------------------------
.type sub_0800961C, %function
sub_0800961C:
_0800961C:
  push {r4, lr}
  sub sp, #4
  ldr r0, _08009640
  ldr r1, _08009644
  ldr r1, [r1, #0]
  ldrh r2, [r1, #48]
  movs r4, #8
  ldrsh r3, [r1, r4]
  lsls r3, r3, #2
  movs r1, #4
  str r1, [sp, #0]
  movs r1, #0
  bl sub_08007570
  add sp, #4
  pop {r4}
  pop {r0}
  bx r0
  .align 2, 0
_08009640: .4byte 0x0828FF08
_08009644: .4byte 0x030003E0

@ ----------------------------------------------------------------------------
.type sub_08009648, %function
sub_08009648:
_08009648:
  push {r4, lr}
  ldr r4, _08009670
  ldr r0, [r4, #0]
  adds r0, #77
  ldrb r0, [r0, #0]
  cmp r0, #0
  beq _0800965A
  bl sub_080095F0
_0800965A:
  ldr r0, [r4, #0]
  adds r0, #76
  ldrb r0, [r0, #0]
  cmp r0, #0
  beq _08009668
  bl sub_0800961C
_08009668:
  pop {r4}
  pop {r0}
  bx r0
  .align 2, 0
_08009670: .4byte 0x030003E0

@ ----------------------------------------------------------------------------
.type sub_08009674, %function
sub_08009674:
_08009674:
  ldr r0, _0800967C
  ldr r0, [r0, #0]
  ldr r0, [r0, #0]
  bx lr
  .align 2, 0
_0800967C: .4byte 0x030003E4

@ ----------------------------------------------------------------------------
.type sub_08009680, %function
sub_08009680:
_08009680:
  ldr r1, _08009688
  ldr r1, [r1, #0]
  str r0, [r1, #0]
  bx lr
  .align 2, 0
_08009688: .4byte 0x030003E4

@ ----------------------------------------------------------------------------
.type sub_0800968C, %function
sub_0800968C:
_0800968C:
  ldr r1, _080096A0
  ldr r1, [r1, #0]
  lsls r0, r0, #16
  asrs r0, r0, #15
  adds r1, #20
  adds r1, r1, r0
  movs r2, #0
  ldrsh r0, [r1, r2]
  bx lr
  .align 2, 0
_080096A0: .4byte 0x030003E4

@ ----------------------------------------------------------------------------
.type sub_080096A4, %function
sub_080096A4:
_080096A4:
  push {r4, r5, lr}
  sub sp, #4
  adds r1, r0, #0
  ldr r4, _08009730
  str r1, [r4, #0]
  movs r0, #0
  str r0, [sp, #0]
  ldr r2, _08009734
  mov r0, sp
  bl sub_0802D974
  movs r0, #10
  bl sub_080261B0
  ldr r1, [r4, #0]
  strh r0, [r1, #14]
  movs r0, #10
  bl 0x080261F8
  ldr r1, [r4, #0]
  strh r0, [r1, #16]
  movs r0, #11
  bl 0x080261F8
  ldr r1, [r4, #0]
  strh r0, [r1, #18]
  ldr r5, _08009738
  adds r1, #36
  adds r0, r5, #0
  bl sub_0800798C
  ldr r1, [r4, #0]
  adds r0, r1, #0
  adds r0, #36
  movs r2, #14
  ldrsh r1, [r1, r2]
  bl sub_08007A04
  ldr r0, [r4, #0]
  movs r1, #16
  ldrsh r3, [r0, r1]
  adds r0, r5, #0
  movs r1, #0
  movs r2, #1
  bl sub_08007614
  ldr r0, [r4, #0]
  movs r2, #18
  ldrsh r3, [r0, r2]
  adds r0, r5, #0
  movs r1, #0
  movs r2, #0
  bl sub_08007614
  ldr r2, [r4, #0]
  movs r0, #1
  strh r0, [r2, #20]
  ldr r0, _0800973C
  ldr r3, _08009740
  adds r1, r0, r3
  ldrh r1, [r1, #0]
  strh r1, [r2, #22]
  ldr r1, _08009744
  adds r0, r0, r1
  ldrh r0, [r0, #0]
  strh r0, [r2, #24]
  add sp, #4
  pop {r4, r5}
  pop {r0}
  bx r0
  .align 2, 0
_08009730: .4byte 0x030003E4
_08009734: .4byte 0x0500000B
_08009738: .4byte 0x08291F7C
_0800973C: .4byte 0x03001780
_08009740: .4byte 0x000010C6
_08009744: .4byte 0x000010C8

@ ----------------------------------------------------------------------------
.type sub_08009748, %function
sub_08009748:
_08009748:
  push {r4, lr}
  lsls r1, r1, #16
  lsrs r1, r1, #16
  movs r2, #0
  cmp r1, #32
  bne _0800975C
  ldr r2, _08009758
  b _08009762
  .align 2, 0
_08009758: .4byte 0x0000FFFF
_0800975C:
  cmp r1, #16
  bne _08009762
  movs r2, #1
_08009762:
  ldr r0, _08009778
  ldr r1, [r0, #0]
  ldr r0, [r1, #4]
  cmp r0, #1
  beq _080097B8
  cmp r0, #1
  bgt _0800977C
  cmp r0, #0
  beq _08009782
  b _080097E2
  .align 2, 0
_08009778: .4byte 0x030003E4
_0800977C:
  cmp r0, #2
  beq _080097C4
  b _080097E2
_08009782:
  lsls r0, r2, #16
  asrs r0, r0, #16
  ldrh r2, [r1, #20]
  adds r0, r2, r0
  strh r0, [r1, #20]
  lsls r0, r0, #16
  asrs r3, r0, #16
  cmp r3, #0
  bgt _0800979A
  movs r0, #1
  strh r0, [r1, #20]
  b _080097E2
_0800979A:
  ldr r0, _080097B0
  ldr r4, _080097B4
  adds r0, r0, r4
  ldrh r2, [r0, #0]
  movs r4, #0
  ldrsh r0, [r0, r4]
  cmp r3, r0
  ble _080097E2
  strh r2, [r1, #20]
  b _080097E2
  .align 2, 0
_080097B0: .4byte 0x03001780
_080097B4: .4byte 0x000010CA
_080097B8:
  lsls r0, r2, #16
  asrs r0, r0, #16
  ldrh r2, [r1, #22]
  adds r0, r2, r0
  strh r0, [r1, #22]
  b _080097E2
_080097C4:
  lsls r0, r2, #16
  asrs r0, r0, #16
  ldrh r4, [r1, #24]
  adds r0, r4, r0
  strh r0, [r1, #24]
  lsls r0, r0, #16
  asrs r0, r0, #16
  cmp r0, #0
  bgt _080097DA
  movs r0, #1
  b _080097E0
_080097DA:
  cmp r0, #3
  ble _080097E2
  movs r0, #3
_080097E0:
  strh r0, [r1, #24]
_080097E2:
  pop {r4}
  pop {r0}
  bx r0

@ ----------------------------------------------------------------------------
.type sub_080097E8, %function
sub_080097E8:
_080097E8:
  lsls r1, r1, #16
  lsrs r1, r1, #16
  movs r3, #0
  cmp r1, #8
  bne _080097FA
  ldr r0, _0800982C
  ldr r0, [r0, #0]
  str r3, [r0, #0]
  movs r3, #2
_080097FA:
  cmp r1, #1
  bne _08009800
  movs r3, #2
_08009800:
  cmp r1, #64
  bne _08009814
  ldr r0, _0800982C
  ldr r2, [r0, #0]
  ldr r0, [r2, #0]
  cmp r0, #0
  ble _08009814
  movs r3, #1
  subs r0, #1
  str r0, [r2, #0]
_08009814:
  cmp r1, #128
  bne _08009828
  ldr r0, _0800982C
  ldr r1, [r0, #0]
  ldr r0, [r1, #0]
  cmp r0, #1
  bgt _08009828
  movs r3, #1
  adds r0, #1
  str r0, [r1, #0]
_08009828:
  adds r0, r3, #0
  bx lr
  .align 2, 0
_0800982C: .4byte 0x030003E4

@ ----------------------------------------------------------------------------
.type sub_08009830, %function
sub_08009830:
_08009830:
  push {r4, r5, lr}
  lsls r0, r0, #16
  lsrs r3, r0, #16
  lsls r1, r1, #16
  lsrs r4, r1, #16
  movs r5, #0
  cmp r4, #8
  bne _08009848
  ldr r0, _0800985C
  ldr r0, [r0, #0]
  str r5, [r0, #0]
  movs r5, #2
_08009848:
  cmp r4, #1
  bne _08009870
  ldr r1, _0800985C
  ldr r0, [r1, #0]
  ldr r2, [r0, #4]
  cmp r2, #0
  bne _08009860
  movs r5, #3
  b _08009866
  .align 2, 0
_0800985C: .4byte 0x030003E4
_08009860:
  cmp r2, #1
  bne _08009866
  movs r5, #4
_08009866:
  ldr r0, [r1, #0]
  ldr r0, [r0, #4]
  cmp r0, #2
  bne _08009870
  movs r5, #5
_08009870:
  cmp r4, #64
  bne _08009884
  ldr r0, _080098C4
  ldr r1, [r0, #0]
  ldr r0, [r1, #4]
  cmp r0, #0
  ble _08009884
  subs r0, #1
  str r0, [r1, #4]
  movs r5, #1
_08009884:
  cmp r4, #128
  bne _08009898
  ldr r0, _080098C4
  ldr r1, [r0, #0]
  ldr r0, [r1, #4]
  cmp r0, #1
  bgt _08009898
  adds r0, #1
  str r0, [r1, #4]
  movs r5, #1
_08009898:
  movs r0, #48
  ands r0, r4
  cmp r0, #0
  beq _080098A8
  adds r0, r3, #0
  adds r1, r4, #0
  bl sub_08009748
_080098A8:
  movs r0, #128
  lsls r0, r0, #1
  cmp r4, r0
  bne _080098BA
  ldr r0, _080098C4
  ldr r1, [r0, #0]
  movs r0, #0
  strh r0, [r1, #12]
  movs r5, #1
_080098BA:
  adds r0, r5, #0
  pop {r4, r5}
  pop {r1}
  bx r1
  .align 2, 0
_080098C4: .4byte 0x030003E4

@ ----------------------------------------------------------------------------
.type sub_080098C8, %function
sub_080098C8:
_080098C8:
  push {lr}
  lsls r0, r0, #16
  lsrs r2, r0, #16
  lsls r1, r1, #16
  lsrs r1, r1, #16
  ldr r0, _080098E8
  ldr r0, [r0, #0]
  movs r3, #12
  ldrsh r0, [r0, r3]
  cmp r0, #0
  beq _080098EC
  cmp r0, #1
  beq _080098F4
  movs r0, #0
  b _080098FA
  .align 2, 0
_080098E8: .4byte 0x030003E4
_080098EC:
  adds r0, r2, #0
  bl sub_080097E8
  b _080098FA
_080098F4:
  adds r0, r2, #0
  bl sub_08009830
_080098FA:
  pop {r1}
  bx r1
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_08009900, %function
sub_08009900:
_08009900:
  push {r4, r5, r6, r7, lr}
  mov r7, r9
  mov r6, r8
  push {r6, r7}
  sub sp, #32
  ldr r1, _08009980
  add r0, sp, #16
  movs r2, #6
  bl sub_0802E0A4
  add r4, sp, #24
  ldr r1, _08009984
  adds r0, r4, #0
  movs r2, #6
  bl sub_0802E0A4
  ldr r0, _08009988
  ldr r1, [r0, #0]
  adds r0, r1, #0
  adds r0, #36
  movs r2, #16
  ldrsh r1, [r1, r2]
  str r1, [sp, #0]
  movs r1, #0
  str r1, [sp, #4]
  str r1, [sp, #8]
  str r1, [sp, #12]
  movs r1, #1
  movs r2, #88
  movs r3, #28
  bl sub_08007B18
  movs r0, #0
  mov r8, r0
  movs r5, #0
  movs r7, #64
  adds r6, r4, #0
  add r4, sp, #16
_0800994C:
  ldr r0, _08009988
  ldr r3, [r0, #0]
  ldr r0, [r3, #0]
  cmp r0, r8
  bne _0800998C
  adds r0, r3, #0
  adds r0, #36
  movs r2, #0
  ldrsh r1, [r4, r2]
  mov r9, r1
  movs r2, #0
  ldrsh r1, [r6, r2]
  mov ip, r1
  movs r1, #16
  ldrsh r3, [r3, r1]
  str r3, [sp, #0]
  str r5, [sp, #4]
  str r5, [sp, #8]
  str r5, [sp, #12]
  mov r1, r9
  mov r2, ip
  adds r3, r7, #0
  bl sub_08007B18
  b _080099B2
  .align 2, 0
_08009980: .4byte 0x0805F6A4
_08009984: .4byte 0x0805F6AA
_08009988: .4byte 0x030003E4
_0800998C:
  adds r0, r3, #0
  adds r0, #36
  movs r1, #0
  ldrsh r2, [r4, r1]
  mov r9, r2
  movs r1, #0
  ldrsh r2, [r6, r1]
  mov ip, r2
  movs r2, #18
  ldrsh r3, [r3, r2]
  str r3, [sp, #0]
  str r5, [sp, #4]
  str r5, [sp, #8]
  str r5, [sp, #12]
  mov r1, r9
  mov r2, ip
  adds r3, r7, #0
  bl sub_08007B18
_080099B2:
  adds r7, #24
  adds r6, #2
  adds r4, #2
  movs r0, #1
  add r8, r0
  mov r1, r8
  cmp r1, #2
  ble _0800994C
  add sp, #32
  pop {r3, r4}
  mov r8, r3
  mov r9, r4
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0

@ ----------------------------------------------------------------------------
.type sub_080099D0, %function
sub_080099D0:
_080099D0:
  push {r4, r5, r6, r7, lr}
  mov r7, r8
  push {r7}
  sub sp, #68
  ldr r0, _08009B04
  ldr r0, [r0, #0]
  ldr r1, [r0, #4]
  lsls r1, r1, #4
  adds r1, #16
  ldr r2, _08009B08
  movs r0, #0
  bl 0x080038C8
  movs r4, #0
  add r7, sp, #52
  ldr r6, _08009B0C
  movs r5, #16
_080099F2:
  ldmia r6!, {r2}
  movs r0, #20
  adds r1, r5, #0
  bl 0x080038C8
  ldr r0, _08009B04
  ldr r0, [r0, #0]
  lsls r1, r4, #1
  adds r0, #20
  adds r0, r0, r1
  movs r1, #0
  ldrsh r2, [r0, r1]
  movs r0, #120
  adds r1, r5, #0
  bl 0x080039C0
  adds r5, #16
  adds r4, #1
  cmp r4, #2
  ble _080099F2
  ldr r4, _08009B10
  ldr r2, _08009B14
  adds r4, r4, r2
  movs r1, #0
  ldrsh r0, [r4, r1]
  bl sub_08025500
  strh r0, [r7, #0]
  movs r2, #0
  ldrsh r0, [r4, r2]
  bl sub_08025518
  strh r0, [r7, #2]
  movs r1, #0
  ldrsh r0, [r4, r1]
  bl sub_08025530
  strh r0, [r7, #4]
  movs r4, #0
  adds r6, r7, #0
  movs r5, #80
  ldr r2, _08009B18
  mov r8, r2
_08009A48:
  mov r0, r8
  adds r0, #4
  mov r8, r0
  subs r0, #4
  ldmia r0!, {r2}
  movs r0, #20
  adds r1, r5, #0
  bl 0x080038C8
  movs r1, #0
  ldrsh r2, [r6, r1]
  movs r0, #120
  adds r1, r5, #0
  bl 0x080039C0
  adds r6, #2
  adds r5, #16
  adds r4, #1
  cmp r4, #2
  ble _08009A48
  ldr r1, _08009B10
  ldr r2, _08009B1C
  adds r0, r1, r2
  ldrb r0, [r0, #0]
  lsls r0, r0, #24
  asrs r0, r0, #24
  strh r0, [r7, #0]
  adds r2, #4
  adds r0, r1, r2
  ldrb r0, [r0, #0]
  lsls r0, r0, #24
  asrs r0, r0, #24
  strh r0, [r7, #2]
  subs r2, #2
  adds r0, r1, r2
  ldrb r0, [r0, #0]
  lsls r0, r0, #24
  asrs r0, r0, #24
  strh r0, [r7, #4]
  adds r2, #3
  adds r0, r1, r2
  ldrb r0, [r0, #0]
  lsls r0, r0, #24
  asrs r0, r0, #24
  strh r0, [r7, #6]
  subs r2, #4
  adds r0, r1, r2
  ldrb r0, [r0, #0]
  lsls r0, r0, #24
  asrs r0, r0, #24
  strh r0, [r7, #8]
  adds r2, #5
  adds r0, r1, r2
  ldrb r0, [r0, #0]
  lsls r0, r0, #24
  asrs r0, r0, #24
  strh r0, [r7, #10]
  adds r2, #1
  adds r0, r1, r2
  ldrb r0, [r0, #0]
  lsls r0, r0, #24
  asrs r0, r0, #24
  strh r0, [r7, #12]
  ldr r0, _08009B20
  adds r1, r1, r0
  movs r0, #0
  ldrsb r0, [r1, r0]
  strh r0, [r7, #14]
  adds r6, r7, #0
  ldr r7, _08009B24
  movs r5, #16
  movs r4, #7
_08009AD8:
  ldmia r7!, {r2}
  movs r0, #130
  adds r1, r5, #0
  bl 0x080038C8
  movs r1, #0
  ldrsh r2, [r6, r1]
  movs r0, #220
  adds r1, r5, #0
  bl 0x080039C0
  adds r6, #2
  adds r5, #16
  subs r4, #1
  cmp r4, #0
  bge _08009AD8
  add sp, #68
  pop {r3}
  mov r8, r3
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .align 2, 0
_08009B04: .4byte 0x030003E4
_08009B08: .4byte 0x0805F6B0
_08009B0C: .4byte 0x080CB190
_08009B10: .4byte 0x03001780
_08009B14: .4byte 0x00001114
_08009B18: .4byte 0x080CB19C
_08009B1C: .4byte 0x0000111A
_08009B20: .4byte 0x0000111D
_08009B24: .4byte 0x080CB1A8

@ ----------------------------------------------------------------------------
.type sub_08009B28, %function
sub_08009B28:
_08009B28:
  push {lr}
  ldr r0, _08009B3C
  ldr r0, [r0, #0]
  movs r1, #12
  ldrsh r0, [r0, r1]
  cmp r0, #0
  beq _08009B40
  cmp r0, #1
  beq _08009B46
  b _08009B4A
  .align 2, 0
_08009B3C: .4byte 0x030003E4
_08009B40:
  bl sub_08009900
  b _08009B4A
_08009B46:
  bl sub_080099D0
_08009B4A:
  pop {r0}
  bx r0
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_08009B50, %function
sub_08009B50:
_08009B50:
  push {lr}
  bl _08002494
  lsls r0, r0, #16
  lsrs r0, r0, #16
  pop {r1}
  bx r1
  .align 2, 0

@ End-of-region anchor for the splicer. This file declares exactly one
@ `@ Region:` (0x080095f0-0x08009b60), has no `.include`, and ends at
@ 0x08009b60 -- the same address the promoted body at 0x08009b50 ends on
@ (0x08009b60). The boundary is therefore unambiguous and the anchor is safe.
@ Without it promotion_screen refuses the body with "no end marker in course_leaves_95f0.s".
course_leaves_95f0_end:
