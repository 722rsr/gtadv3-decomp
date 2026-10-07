@ GT Advance 3 - menu record handlers
@ Region: file offset 0x00DBE8-0x00E650 (VMA 0x0800DBE8-0x0800E650).
@ Pure Thumb, ARMCC; first slice of the big raw menu span
@ (menu_dab8.s ends at 0xDBE8; next raw resumes at 0xE650).
@
@ Function map:
@   sub_0800DBE8 - menu record field setup: pushes 5 field packets via
@                  sub_08007B8C (ids 6/5) and sub_08007C68 (0x58/0xB0/0)
@                  over sp-frame {w,h,tp,pr,..} read from record fields.
@   sub_0800DC7C - high-register (r8) twin: same writes, takes extra
@                  r1 param stored to [sp+8]/[sp+12] slots.
@   sub_0800DD20 - sl/r9/r8 triple: 30-iteration loop (r9=29 down to 0)
@                  stepping r4 += 8, calls sub_08007B18 with mode 3/4
@                  on r6 (type) 2/3.
@   sub_0800DD9C - dispatch on u16[rec+0x58+...] via sub_08004B68:
@                  s16(value-15) -> 28-entry jump table @0xDDC4 (base
@                  word 0x0800DDC4 @0xDDC0) -> 0xDE34 (write 5) or
@                  0xDE3C (write 1) into rec+0x54.
@   sub_0800DE4C - tiny bx lr stub (2 bytes + pad).
@   sub_0800DE50 - sl/r9/r8 record initializer: sound 0x33 via
@                  sub_0802B214, state cells @0x03001780+0xFBC,
@                  resource records 0x082B37D8/0x082B7410/0x082A798C
@                  via sub_08007770/sub_0800798C/sub_08007A58/
@                  sub_080075E8/sub_08007614, sub_0800DAB8 twin init,
@                  0x08025C08/0x08026068 AI queries, two
@                  sub_0800D77C fills, rec[+0x18] flag select.
@   sub_0800DFE0 - tiny record init: [+0x20]=10,[+0x40]=10,
@                  [+0x3C]=1,[+0x1C]=1 (unreferenced leaf).
@   sub_0800DFF0 - input handler: two sub_08002178 key reads ->
@                  state byte @0x03001780+0x10C3 (r6/r7 latches),
@                  sound 1/4/10 via sub_0802B368, music select
@                  from u16 table 0x080CB4EA indexed by rec[+0x18],
@                  mode switch on r7 (1->sub_0800DFF0 path / 2->10).
@   sub_0800E0BC - grid sweep: 4x11 cell clears via 0x08025C84,
@                  sub_080241C8 refresh, 3x sub_08025FF0 grants,
@                  32x sub_08025DBC clears.
@   sub_0800E114 - 8x sub_08025EC0 award bit sets (tiers 3/3/3/1..).
@   sub_0800E15C - r8/r9 key/param handler: sound + rec[+0x18] index
@                  adjust (0x080CB4EA table), sub_08002158 broadcasts,
@                  sub_0800DFF0 / sub_0800E0BC paths, 0x08002780 gate.
@   sub_0800E2E4 - 0x08002618 command + rec[+0x14] clear (guarded).
@   sub_0800E310 - r8 field writer: mode 2/3 (rec[+0x1C]) x type
@                  {0,8,other} -> sub_08007B18 8-byte packets.
@   sub_0800E418 - menu item renderer: 0x080CB448 car-record array
@                  indexed by rec[+0x18], 0x080CB4D8 twin table,
@                  sub_0800DD20/sub_0800E310/sub_0800D97C/sub_0800DBE8
@                  pipeline, sub_08007B18/sub_08007C68 writes.
@   sub_0800E598 - bx lr stub (called from 0xE418 tail).
@   sub_0800E59C - 12-entry command dispatcher (cmd-1): 1->sub_0800DE50,
@                  2->sub_0800DD9C, 5->sub_0800D854/sub_0800D8E4,
@                  6->sub_0800E15C/sub_0800E2E4, 7->sub_0800E418,
@                  12->sub_0800DE4C; jump table @0xE5B8 (base 0x0800E5B8
@                  @0xE5B4).
@
@ Transcribed from baserom.gba via objdump; byte-exact (make SHA gate).
@ Unconverted targets keep numeric `bl 0x0800XXXX` form. Pools at
@ original offsets.

.thumb

@ ----------------------------------------------------------------------------
.type sub_0800DBE8, %function
sub_0800DBE8:
_0800DBE8:
  push {r4, r5, r6, r7, lr}
  sub sp, #20
  adds r7, r0, #0
  adds r6, r7, #0
  adds r6, #36
  ldr r3, [r7, #24]
  movs r5, #1
  str r5, [sp, #0]
  str r5, [sp, #4]
  movs r4, #0
  str r4, [sp, #8]
  str r4, [sp, #12]
  adds r0, r6, #0
  movs r1, #6
  movs r2, #0
  bl sub_08007B8C
  ldr r3, [r7, #24]
  str r5, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  adds r0, r6, #0
  movs r1, #5
  movs r2, #88
  bl sub_08007B8C
  adds r0, r7, #0
  adds r0, #60
  ldr r1, [r7, #80]
  ldr r2, [r7, #84]
  ldr r3, [r7, #24]
  adds r3, #16
  str r3, [sp, #0]
  str r5, [sp, #4]
  str r5, [sp, #8]
  str r4, [sp, #12]
  str r4, [sp, #16]
  movs r3, #88
  bl sub_08007C68
  adds r0, r7, #0
  adds r0, #52
  ldr r1, [r7, #68]
  ldr r2, [r7, #72]
  ldr r3, [r7, #24]
  adds r3, #16
  str r3, [sp, #0]
  movs r6, #2
  str r6, [sp, #4]
  str r5, [sp, #8]
  str r4, [sp, #12]
  str r4, [sp, #16]
  movs r3, #176
  bl sub_08007C68
  adds r0, r7, #0
  adds r0, #44
  ldr r1, [r7, #92]
  ldr r2, [r7, #96]
  ldr r3, [r7, #16]
  str r3, [sp, #0]
  str r6, [sp, #4]
  str r5, [sp, #8]
  str r4, [sp, #12]
  str r4, [sp, #16]
  movs r3, #0
  bl sub_08007C68
  add sp, #20
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_0800DC7C, %function
sub_0800DC7C:
_0800DC7C:
  push {r4, r5, r6, r7, lr}
  mov r7, r8
  push {r7}
  sub sp, #20
  adds r7, r0, #0
  mov r8, r1
  adds r6, r7, #0
  adds r6, #36
  ldr r3, [r7, #24]
  movs r4, #1
  str r4, [sp, #0]
  str r4, [sp, #4]
  str r1, [sp, #8]
  movs r5, #0
  str r5, [sp, #12]
  adds r0, r6, #0
  movs r1, #6
  movs r2, #0
  bl sub_08007B8C
  ldr r3, [r7, #24]
  str r4, [sp, #0]
  str r4, [sp, #4]
  mov r0, r8
  str r0, [sp, #8]
  str r5, [sp, #12]
  adds r0, r6, #0
  movs r1, #5
  movs r2, #88
  bl sub_08007B8C
  adds r0, r7, #0
  adds r0, #60
  ldr r1, [r7, #80]
  ldr r2, [r7, #84]
  ldr r3, [r7, #24]
  adds r3, #16
  str r3, [sp, #0]
  str r4, [sp, #4]
  str r4, [sp, #8]
  mov r3, r8
  str r3, [sp, #12]
  str r5, [sp, #16]
  movs r3, #88
  bl sub_08007C68
  adds r0, r7, #0
  adds r0, #52
  ldr r1, [r7, #68]
  ldr r2, [r7, #72]
  ldr r3, [r7, #24]
  adds r3, #16
  str r3, [sp, #0]
  movs r6, #2
  str r6, [sp, #4]
  str r4, [sp, #8]
  mov r3, r8
  str r3, [sp, #12]
  str r5, [sp, #16]
  movs r3, #176
  bl sub_08007C68
  adds r0, r7, #0
  adds r0, #44
  ldr r1, [r7, #92]
  ldr r2, [r7, #96]
  ldr r3, [r7, #16]
  str r3, [sp, #0]
  str r6, [sp, #4]
  str r4, [sp, #8]
  mov r3, r8
  str r3, [sp, #12]
  str r5, [sp, #16]
  movs r3, #0
  bl sub_08007C68
  add sp, #20
  pop {r3}
  mov r8, r3
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_0800DD20, %function
sub_0800DD20:
_0800DD20:
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
  movs r0, #29
  mov r9, r0
_0800DD3E:
  cmp r6, #2
  beq _0800DD48
  cmp r6, #3
  beq _0800DD64
  b _0800DD7E
_0800DD48:
  movs r0, #3
  str r0, [sp, #0]
  str r5, [sp, #4]
  str r5, [sp, #8]
  mov r0, sl
  str r0, [sp, #12]
  mov r0, r8
  adds r0, #68
  movs r1, #7
  adds r2, r4, #0
  adds r3, r7, #0
  bl sub_08007B18
  b _0800DD7E
_0800DD64:
  movs r0, #4
  str r0, [sp, #0]
  str r5, [sp, #4]
  str r5, [sp, #8]
  mov r0, sl
  str r0, [sp, #12]
  mov r0, r8
  adds r0, #68
  movs r1, #7
  adds r2, r4, #0
  adds r3, r7, #0
  bl sub_08007B18
_0800DD7E:
  adds r4, #8
  movs r0, #1
  negs r0, r0
  add r9, r0
  mov r0, r9
  cmp r0, #0
  bge _0800DD3E
  add sp, #16
  pop {r3, r4, r5}
  mov r8, r3
  mov r9, r4
  mov sl, r5
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_0800DD9C, %function
sub_0800DD9C:
_0800DD9C:
  push {r4, lr}
  adds r4, r1, #0
  adds r1, #88
  movs r0, #1
  strb r0, [r1, #0]
  bl sub_08004B68
  ldrh r0, [r0, #2]
  subs r0, #15
  lsls r0, r0, #16
  asrs r0, r0, #16
  cmp r0, #27
  bhi _0800DE3C
  lsls r0, r0, #2
  ldr r1, _0800DDC0
  adds r0, r0, r1
  ldr r0, [r0, #0]
  mov pc, r0
  .align 2, 0
_0800DDC0: .4byte 0x0800DDC4   @ jump table base (points at first entry)
@ 28-entry jump table: idx = s16(value - 15); 0xDE34 -> store 5, 0xDE3C -> 1
_0800DDC4: .4byte _0800DE34
_0800DDC8: .4byte _0800DE3C
_0800DDCC: .4byte _0800DE34
_0800DDD0: .4byte _0800DE3C
_0800DDD4: .4byte _0800DE3C
_0800DDD8: .4byte _0800DE34
_0800DDDC: .4byte _0800DE3C
_0800DDE0: .4byte _0800DE3C
_0800DDE4: .4byte _0800DE34
_0800DDE8: .4byte _0800DE3C
_0800DDEC: .4byte _0800DE3C
_0800DDF0: .4byte _0800DE3C
_0800DDF4: .4byte _0800DE3C
_0800DDF8: .4byte _0800DE3C
_0800DDFC: .4byte _0800DE3C
_0800DE00: .4byte _0800DE3C
_0800DE04: .4byte _0800DE34
_0800DE08: .4byte _0800DE3C
_0800DE0C: .4byte _0800DE3C
_0800DE10: .4byte _0800DE3C
_0800DE14: .4byte _0800DE3C
_0800DE18: .4byte _0800DE34
_0800DE1C: .4byte _0800DE34
_0800DE20: .4byte _0800DE3C
_0800DE24: .4byte _0800DE3C
_0800DE28: .4byte _0800DE3C
_0800DE2C: .4byte _0800DE34
_0800DE30: .4byte _0800DE34
@ case: value-15 == {0,2,5,8,16,21,22,26,27} -> store 5
_0800DE34:
  adds r1, r4, #0
  adds r1, #84
  movs r0, #5
  b _0800DE42
@ default / all others -> store 1
_0800DE3C:
  adds r1, r4, #0
  adds r1, #84
  movs r0, #1
_0800DE42:
  strh r0, [r1, #0]
  pop {r4}
  pop {r0}
  bx r0
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_0800DE4C, %function
sub_0800DE4C:
_0800DE4C:
  bx lr
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_0800DE50, %function
sub_0800DE50:
_0800DE50:
  push {r4, r5, r6, r7, lr}
  mov r7, sl
  mov r6, r9
  mov r5, r8
  push {r5, r6, r7}
  sub sp, #8
  adds r7, r0, #0
  bl sub_08004B68
  movs r0, #51
  bl sub_0802B214
  ldr r0, _0800DF9C
  ldr r1, _0800DFA0
  adds r0, r0, r1
  movs r1, #0
  mov r9, r1
  movs r4, #9
  strh r4, [r0, #0]
  ldr r5, _0800DFA4
  movs r0, #4
  str r0, [sp, #0]
  movs r0, #1
  mov r8, r0
  str r0, [sp, #4]
  movs r0, #0
  adds r1, r5, #0
  movs r2, #5
  movs r3, #0
  bl sub_08007770
  ldr r6, _0800DFA8
  mov r1, r9
  str r1, [sp, #0]
  movs r0, #3
  str r0, [sp, #4]
  movs r0, #1
  adds r1, r6, #0
  movs r2, #1
  movs r3, #0
  bl sub_08007770
  str r4, [r7, #104]
  movs r0, #22
  str r0, [r7, #116]
  adds r0, r7, #0
  adds r0, #128
  mov r1, r8
  str r1, [r0, #0]
  movs r0, #32
  adds r0, r0, r7
  mov sl, r0
  bl sub_0800DAB8
  ldr r4, _0800DFAC
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
  adds r0, r5, #0
  adds r1, r7, #0
  bl sub_0800798C
  adds r0, r7, #0
  bl sub_08007A58
  adds r0, r5, #0
  movs r1, #4
  movs r2, #7
  bl sub_080075E8
  adds r0, r5, #0
  movs r1, #3
  movs r2, #6
  bl sub_080075E8
  adds r1, r7, #0
  adds r1, #8
  adds r0, r6, #0
  bl sub_0800798C
  adds r0, r6, #0
  movs r1, #0
  movs r2, #8
  bl sub_080075E8
  adds r4, r7, #0
  adds r4, #156
  adds r0, r4, #0
  movs r1, #20
  bl 0x08025C08
  ldr r0, [r7, #12]
  adds r1, r7, #0
  adds r1, #160
  ldr r1, [r1, #0]
  ldr r2, [r4, #0]
  bl sub_08007ABC
  adds r1, r7, #0
  adds r1, #144
  adds r0, r7, #0
  adds r0, #28
  str r0, [r1, #0]
  adds r0, #120
  mov r1, sl
  str r1, [r0, #0]
  mov r0, r9
  str r0, [r7, #32]
  mov r1, r8
  strh r1, [r7, #36]
  adds r0, r7, #0
  adds r0, #52
  movs r2, #32
  negs r2, r2
  movs r1, #0
  bl sub_0800D77C
  adds r0, r7, #0
  adds r0, #44
  movs r1, #0
  movs r2, #160
  bl sub_0800D77C
  movs r0, #6
  strh r0, [r7, #40]
  mov r0, r8
  strh r0, [r7, #42]
  movs r0, #0
  bl 0x08004CA8
  str r0, [r7, #24]
  movs r0, #2
  str r0, [r7, #28]
  mov r1, r9
  strh r1, [r7, #20]
  movs r5, #0
  movs r4, #0
_0800DF80:
  adds r0, r4, #0
  bl 0x08026068
  cmp r0, #1
  bne _0800DF8C
  adds r5, #1
_0800DF8C:
  adds r4, #1
  cmp r4, #2
  ble _0800DF80
  cmp r5, #0
  ble _0800DFB0
  movs r0, #1
  b _0800DFB2
  .align 2, 0
_0800DF9C: .4byte 0x03001780
_0800DFA0: .4byte 0x00000FBC
_0800DFA4: .4byte 0x082B37D8
_0800DFA8: .4byte 0x082B7410
_0800DFAC: .4byte 0x082A798C
_0800DFB0:
  movs r0, #0
_0800DFB2:
  strh r0, [r7, #18]
  ldr r0, _0800DFD4
  bl 0x08002124
  ldr r0, _0800DFD8
  ldr r1, _0800DFDC
  adds r0, r0, r1
  movs r1, #0
  strb r1, [r0, #0]
  add sp, #8
  pop {r3, r4, r5}
  mov r8, r3
  mov r9, r4
  mov sl, r5
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .align 2, 0
_0800DFD4: .4byte 0x0000138C
_0800DFD8: .4byte 0x03001780
_0800DFDC: .4byte 0x000010C3

@ ----------------------------------------------------------------------------
@ tiny record init: [+0x20]=10, [+0x40]=10, [+0x3C]=1, [+0x1C]=1
.type sub_0800DFE0, %function
sub_0800DFE0:
_0800DFE0:
  movs r1, #10
  str r1, [r0, #32]
  str r1, [r0, #64]
  movs r1, #1
  str r1, [r0, #60]
  str r1, [r0, #28]
  bx lr
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_0800DFF0, %function
sub_0800DFF0:
_0800DFF0:
  push {r4, r5, r6, r7, lr}
  adds r4, r0, #0
  movs r6, #0
  movs r7, #0
  movs r0, #0
  movs r1, #1
  bl 0x08002178
  lsls r0, r0, #16
  lsrs r5, r0, #16
  movs r0, #1
  movs r1, #1
  bl 0x08002178
  lsls r0, r0, #16
  lsrs r0, r0, #16
  cmp r5, #1
  bne _0800E02C
  cmp r0, #1
  beq _0800E044
  movs r6, #1
  ldr r0, _0800E024
  ldr r1, _0800E028
  adds r0, r0, r1
  strb r6, [r0, #0]
  b _0800E04A
  .align 2, 0
_0800E024: .4byte 0x03001780
_0800E028: .4byte 0x000010C3
_0800E02C:
  cmp r0, #1
  bne _0800E04A
  ldr r0, _0800E03C
  ldr r2, _0800E040
  adds r0, r0, r2
  strb r6, [r0, #0]
  movs r6, #1
  b _0800E04A
  .align 2, 0
_0800E03C: .4byte 0x03001780
_0800E040: .4byte 0x000010C3
_0800E044:
  movs r0, #10
  bl sub_0802B368
_0800E04A:
  cmp r5, #2
  bne _0800E052
  movs r7, #1
  strh r5, [r4, #16]
_0800E052:
  cmp r6, #0
  beq _0800E098
  movs r0, #1
  bl sub_0802B368
  movs r0, #0
  strh r0, [r4, #36]
  adds r1, r4, #0
  adds r1, #140
  movs r0, #1
  strh r0, [r1, #0]
  ldr r0, [r4, #24]
  bl 0x08004BFC
  ldr r1, [r4, #24]
  movs r0, #0
  bl 0x08004C84
  ldr r1, _0800E08C
  ldr r2, _0800E090
  ldr r0, [r4, #24]
  lsls r0, r0, #1
  adds r0, r0, r2
  ldrh r0, [r0, #0]
  ldr r2, _0800E094
  adds r1, r1, r2
  strh r0, [r1, #0]
  b _0800E0B4
  .align 2, 0
_0800E08C: .4byte 0x03001780
_0800E090: .4byte 0x080CB4EA
_0800E094: .4byte 0x00000FBC
_0800E098:
  cmp r7, #0
  beq _0800E0B4
  movs r0, #4
  bl sub_0802B368
  movs r0, #10
  str r0, [r4, #32]
  strh r6, [r4, #36]
  str r0, [r4, #64]
  str r6, [r4, #60]
  adds r1, r4, #0
  adds r1, #168
  movs r0, #1
  strb r0, [r1, #0]
_0800E0B4:
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_0800E0BC, %function
sub_0800E0BC:
_0800E0BC:
  push {r4, r5, r6, lr}
  movs r5, #0
_0800E0C0:
  movs r4, #0
  adds r6, r5, #1
_0800E0C4:
  movs r0, #0
  adds r1, r5, #0
  adds r2, r4, #0
  movs r3, #3
  bl 0x08025C84
  movs r0, #1
  adds r1, r5, #0
  adds r2, r4, #0
  movs r3, #3
  bl 0x08025C84
  adds r4, #1
  cmp r4, #10
  ble _0800E0C4
  adds r5, r6, #0
  cmp r5, #3
  ble _0800E0C0
  bl sub_080241C8
  movs r0, #0
  bl sub_08025FF0
  movs r0, #1
  bl sub_08025FF0
  movs r0, #2
  bl sub_08025FF0
  movs r4, #0
_0800E100:
  adds r0, r4, #0
  movs r1, #3
  bl sub_08025DBC
  adds r4, #1
  cmp r4, #31
  ble _0800E100
  pop {r4, r5, r6}
  pop {r0}
  bx r0
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_0800E114, %function
sub_0800E114:
_0800E114:
  push {lr}
  movs r0, #0
  movs r1, #3
  bl sub_08025EC0
  movs r0, #1
  movs r1, #3
  bl sub_08025EC0
  movs r0, #2
  movs r1, #3
  bl sub_08025EC0
  movs r0, #3
  movs r1, #1
  bl sub_08025EC0
  movs r0, #4
  movs r1, #1
  bl sub_08025EC0
  movs r0, #5
  movs r1, #1
  bl sub_08025EC0
  movs r0, #6
  movs r1, #1
  bl sub_08025EC0
  movs r0, #7
  movs r1, #1
  bl sub_08025EC0
  pop {r0}
  bx r0
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_0800E15C, %function
sub_0800E15C:
_0800E15C:
  push {r4, r5, r6, r7, lr}
  mov r7, r9
  mov r6, r8
  push {r6, r7}
  adds r4, r0, #0
  lsls r1, r1, #16
  lsrs r1, r1, #16
  mov r9, r1
  lsls r2, r2, #16
  lsrs r7, r2, #16
  bl sub_08004B68
  movs r6, #1
  bl 0x08002140
  cmp r0, #1
  ble _0800E1B6
  movs r5, #0
  movs r0, #168
  adds r0, r0, r4
  mov r8, r0
  b _0800E1AC
_0800E188:
  adds r0, r5, #0
  movs r1, #2
  bl 0x08002178
  lsls r0, r0, #16
  lsrs r0, r0, #16
  cmp r0, #6
  beq _0800E19A
  movs r6, #0
_0800E19A:
  adds r0, r5, #0
  movs r1, #3
  bl 0x08002178
  lsls r0, r0, #16
  cmp r0, #0
  beq _0800E1AA
  movs r6, #0
_0800E1AA:
  adds r5, #1
_0800E1AC:
  bl 0x08002140
  cmp r5, r0
  blt _0800E188
  b _0800E1BE
_0800E1B6:
  movs r6, #0
  movs r1, #168
  adds r1, r1, r4
  mov r8, r1
_0800E1BE:
  cmp r6, #0
  beq _0800E1DA
  ldrh r1, [r4, #16]
  movs r2, #16
  ldrsh r0, [r4, r2]
  cmp r0, #0
  ble _0800E1D2
  subs r0, r1, #1
  strh r0, [r4, #16]
  b _0800E278
_0800E1D2:
  adds r0, r4, #0
  bl sub_0800DFF0
  b _0800E278
_0800E1DA:
  cmp r7, #1
  bne _0800E25C
  ldr r1, [r4, #24]
  cmp r1, #6
  beq _0800E24C
  ldrh r0, [r4, #18]
  cmp r0, #0
  bne _0800E1EE
  cmp r1, #5
  beq _0800E1FC
_0800E1EE:
  bl 0x08002780
  cmp r0, #1
  bne _0800E204
  ldr r0, [r4, #24]
  cmp r0, #7
  bne _0800E204
_0800E1FC:
  movs r0, #10
  bl sub_0802B368
  b _0800E25C
_0800E204:
  movs r0, #1
  bl sub_0802B368
  movs r0, #0
  strh r0, [r4, #36]
  adds r1, r4, #0
  adds r1, #140
  movs r0, #1
  strh r0, [r1, #0]
  ldr r0, [r4, #24]
  bl 0x08004BFC
  ldr r1, [r4, #24]
  movs r0, #0
  bl 0x08004C84
  ldr r1, _0800E240
  ldr r2, _0800E244
  ldr r3, [r4, #24]
  lsls r0, r3, #1
  adds r0, r0, r2
  ldrh r0, [r0, #0]
  ldr r2, _0800E248
  adds r1, r1, r2
  strh r0, [r1, #0]
  cmp r3, #7
  bne _0800E25C
  movs r0, #8
  strh r0, [r4, #40]
  b _0800E25C
  .align 2, 0
_0800E240: .4byte 0x03001780
_0800E244: .4byte 0x080CB4EA
_0800E248: .4byte 0x00000FBC
_0800E24C:
  movs r0, #10
  bl sub_0802B368
  movs r0, #1
  movs r1, #1
  bl 0x08002618
  strh r7, [r4, #20]
_0800E25C:
  cmp r7, #2
  bne _0800E278
  movs r0, #4
  bl sub_0802B368
  movs r1, #10
  str r1, [r4, #32]
  movs r0, #0
  strh r0, [r4, #36]
  str r1, [r4, #64]
  str r0, [r4, #60]
  movs r0, #1
  mov r1, r8
_0800E276:
  strb r0, [r1, #0]
_0800E278:
  cmp r7, #64
  beq _0800E280
  cmp r7, #32
  bne _0800E28C
_0800E280:
  movs r0, #2
  bl sub_0802B368
  ldr r0, [r4, #24]
  subs r0, #1
  str r0, [r4, #24]
_0800E28C:
  cmp r7, #128
  beq _0800E294
  cmp r7, #16
  bne _0800E2A0
_0800E294:
  movs r0, #2
  bl sub_0802B368
  ldr r0, [r4, #24]
  adds r0, #1
  str r0, [r4, #24]
_0800E2A0:
  ldr r0, [r4, #24]
  cmp r0, #0
  bge _0800E2AA
  movs r0, #8
  str r0, [r4, #24]
_0800E2AA:
  ldr r0, [r4, #24]
  cmp r0, #8
  ble _0800E2B4
  movs r0, #0
  str r0, [r4, #24]
_0800E2B4:
  movs r0, #0
  mov r1, r9
  bl 0x08002158
  movs r0, #1
  adds r1, r7, #0
  bl 0x08002158
  ldrh r1, [r4, #24]
  movs r0, #2
  bl 0x08002158
  mov r2, r8
  ldrb r1, [r2, #0]
  movs r0, #3
  bl 0x08002158
  pop {r3, r4}
  mov r8, r3
  mov r9, r4
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_0800E2E4, %function
sub_0800E2E4:
_0800E2E4:
  push {r4, lr}
  adds r4, r0, #0
  lsls r2, r2, #16
  ldr r0, _0800E30C
  adds r2, r2, r0
  lsrs r2, r2, #16
  cmp r2, #1
  bhi _0800E306
  movs r0, #4
  bl sub_0802B368
  movs r0, #1
  movs r1, #0
  bl 0x08002618
  movs r0, #0
  strh r0, [r4, #20]
_0800E306:
  pop {r4}
  pop {r0}
  bx r0
  .align 2, 0
_0800E30C: .4byte 0xFFFF0000

@ ----------------------------------------------------------------------------
.type sub_0800E310, %function
sub_0800E310:
_0800E310:
  push {r4, r5, r6, r7, lr}
  mov r7, r8
  push {r7}
  sub sp, #16
  adds r7, r2, #0
  adds r1, r0, #0
  adds r1, #154
  movs r3, #0
  ldrsh r2, [r1, r3]
  mov r8, r2
  cmp r2, #0
  bne _0800E40C
  ldr r6, [r0, #28]
  cmp r6, #2
  beq _0800E39A
  cmp r6, #3
  bne _0800E40C
  ldr r1, [r0, #24]
  cmp r1, #0
  bne _0800E34E
  adds r0, #68
  adds r3, r7, #0
  adds r3, #9
  movs r1, #2
  str r1, [sp, #0]
  movs r1, #1
  str r1, [sp, #4]
  str r1, [sp, #8]
  str r2, [sp, #12]
  movs r1, #10
  b _0800E3CE
_0800E34E:
  cmp r1, #8
  bne _0800E368
  adds r0, #68
  adds r3, r7, #0
  subs r3, #9
  movs r1, #2
  str r1, [sp, #0]
  movs r1, #1
  str r1, [sp, #4]
  str r1, [sp, #8]
  mov r1, r8
  str r1, [sp, #12]
  b _0800E3CC
_0800E368:
  adds r6, r0, #0
  adds r6, #68
  adds r3, r7, #0
  adds r3, #9
  movs r5, #2
  str r5, [sp, #0]
  movs r4, #1
  str r4, [sp, #4]
  str r4, [sp, #8]
  mov r2, r8
  str r2, [sp, #12]
  adds r0, r6, #0
  movs r1, #10
  movs r2, #0
  bl sub_08007B18
  adds r3, r7, #0
  subs r3, #9
  str r5, [sp, #0]
  str r4, [sp, #4]
  str r4, [sp, #8]
  mov r0, r8
  str r0, [sp, #12]
  adds r0, r6, #0
  b _0800E3CC
_0800E39A:
  ldr r1, [r0, #24]
  cmp r1, #0
  bne _0800E3B6
  adds r0, #68
  adds r3, r7, #0
  adds r3, #9
  str r6, [sp, #0]
  movs r1, #1
  str r1, [sp, #4]
  str r1, [sp, #8]
  mov r1, r8
  str r1, [sp, #12]
  movs r1, #10
  b _0800E3CE
_0800E3B6:
  cmp r1, #8
  bne _0800E3D6
  adds r0, #68
  adds r3, r7, #0
  subs r3, #9
  str r6, [sp, #0]
  movs r1, #1
  str r1, [sp, #4]
  str r1, [sp, #8]
  mov r2, r8
  str r2, [sp, #12]
_0800E3CC:
  movs r1, #9
_0800E3CE:
  movs r2, #0
  bl sub_08007B18
  b _0800E40C
_0800E3D6:
  adds r5, r0, #0
  adds r5, #68
  adds r3, r7, #0
  adds r3, #9
  str r6, [sp, #0]
  movs r4, #1
  str r4, [sp, #4]
  str r4, [sp, #8]
  mov r0, r8
  str r0, [sp, #12]
  adds r0, r5, #0
  movs r1, #10
  movs r2, #0
  bl sub_08007B18
  adds r3, r7, #0
  subs r3, #9
  str r6, [sp, #0]
  str r4, [sp, #4]
  str r4, [sp, #8]
  mov r1, r8
  str r1, [sp, #12]
  adds r0, r5, #0
  movs r1, #9
  movs r2, #0
  bl sub_08007B18
_0800E40C:
  add sp, #16
  pop {r3}
  mov r8, r3
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_0800E418, %function
sub_0800E418:
_0800E418:
  push {r4, r5, r6, lr}
  sub sp, #20
  adds r4, r0, #0
  ldrh r1, [r4, #18]
  cmp r1, #1
  bne _0800E440
  ldr r0, _0800E4DC
  ldr r2, [r0, #40]
  adds r2, #8
  ldr r3, [r0, #44]
  movs r0, #6
  str r0, [sp, #0]
  str r1, [sp, #4]
  str r1, [sp, #8]
  movs r0, #0
  str r0, [sp, #12]
  adds r0, r4, #0
  movs r1, #17
  bl sub_08007B18
_0800E440:
  ldr r0, _0800E4E0
  ldr r1, _0800E4E4
  adds r0, r0, r1
  ldrh r0, [r0, #0]
  cmp r0, #1
  bne _0800E476
  ldr r0, _0800E4DC
  ldr r2, [r0, #40]
  ldr r3, [r0, #44]
  ldr r0, _0800E4E8
  ldrh r5, [r0, #10]
  adds r0, r4, #0
  adds r0, #154
  movs r6, #0
  ldrsh r1, [r0, r6]
  cmp r1, #1
  bne _0800E476
  movs r0, #5
  str r0, [sp, #0]
  str r1, [sp, #4]
  str r1, [sp, #8]
  movs r0, #0
  str r0, [sp, #12]
  adds r0, r4, #0
  adds r1, r5, #0
  bl sub_08007B18
_0800E476:
  bl sub_080027EC
  cmp r0, #1
  bne _0800E4A8
  ldr r0, _0800E4DC
  ldr r2, [r0, #56]
  ldr r3, [r0, #60]
  ldr r0, _0800E4E8
  ldrh r5, [r0, #14]
  adds r0, r4, #0
  adds r0, #154
  movs r6, #0
  ldrsh r1, [r0, r6]
  cmp r1, #1
  bne _0800E4A8
  movs r0, #5
  str r0, [sp, #0]
  str r1, [sp, #4]
  str r1, [sp, #8]
  movs r0, #0
  str r0, [sp, #12]
  adds r0, r4, #0
  adds r1, r5, #0
  bl sub_08007B18
_0800E4A8:
  ldr r2, _0800E4DC
  ldr r0, [r4, #24]
  lsls r0, r0, #3
  adds r1, r0, r2
  ldr r6, [r1, #0]
  adds r2, #4
  adds r0, r0, r2
  ldr r5, [r0, #0]
  ldr r2, [r4, #28]
  adds r0, r4, #0
  adds r1, r5, #0
  bl sub_0800DD20
  ldr r1, _0800E4E8
  ldr r0, [r4, #24]
  lsls r0, r0, #1
  adds r0, r0, r1
  ldrh r1, [r0, #0]
  ldr r2, [r4, #28]
  cmp r2, #1
  beq _0800E4F6
  cmp r2, #1
  bgt _0800E4EC
  cmp r2, #0
  beq _0800E500
  b _0800E540
  .align 2, 0
_0800E4DC: .4byte 0x080CB448
_0800E4E0: .4byte 0x03001780
_0800E4E4: .4byte 0x00001076
_0800E4E8: .4byte 0x080CB4D8
_0800E4EC:
  cmp r2, #2
  beq _0800E528
  cmp r2, #3
  beq _0800E50E
  b _0800E540
_0800E4F6:
  movs r0, #4
  str r0, [sp, #0]
  str r2, [sp, #4]
  str r2, [sp, #8]
  b _0800E518
_0800E500:
  movs r0, #3
  str r0, [sp, #0]
  movs r0, #1
  str r0, [sp, #4]
  str r0, [sp, #8]
  str r2, [sp, #12]
  b _0800E51C
_0800E50E:
  movs r0, #4
  str r0, [sp, #0]
  movs r0, #1
  str r0, [sp, #4]
  str r0, [sp, #8]
_0800E518:
  movs r0, #0
  str r0, [sp, #12]
_0800E51C:
  adds r0, r4, #0
  adds r2, r6, #0
  adds r3, r5, #0
  bl sub_08007B18
  b _0800E540
_0800E528:
  movs r0, #3
  str r0, [sp, #0]
  movs r0, #1
  str r0, [sp, #4]
  str r0, [sp, #8]
  movs r0, #0
  str r0, [sp, #12]
  adds r0, r4, #0
  adds r2, r6, #0
  adds r3, r5, #0
  bl sub_08007B18
_0800E540:
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0800E310
  movs r0, #20
  ldrsh r5, [r4, r0]
  cmp r5, #1
  bne _0800E578
  adds r0, r4, #0
  adds r0, #8
  adds r1, r4, #0
  adds r1, #156
  ldr r1, [r1, #0]
  adds r2, r4, #0
  adds r2, #160
  ldr r2, [r2, #0]
  movs r3, #56
  str r3, [sp, #0]
  movs r3, #8
  str r3, [sp, #4]
  str r5, [sp, #8]
  movs r3, #0
  str r3, [sp, #12]
  str r3, [sp, #16]
  movs r3, #12
  bl sub_08007C68
_0800E578:
  adds r0, r4, #0
  adds r0, #152
  movs r1, #15
  bl sub_0800D97C
  adds r0, r4, #0
  adds r0, #32
  bl sub_0800DBE8
  adds r0, r4, #0
  bl sub_0800E598
  add sp, #20
  pop {r4, r5, r6}
  pop {r0}
  bx r0
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_0800E598, %function
sub_0800E598:
_0800E598:
  bx lr
  .align 2, 0

@ ----------------------------------------------------------------------------
.type sub_0800E59C, %function
sub_0800E59C:
_0800E59C:
  push {r4, r5, lr}
  adds r5, r1, #0
  adds r4, r3, #0
  subs r0, #1
  cmp r0, #11
  bhi _0800E648
  lsls r0, r0, #2
  ldr r1, _0800E5B4
  adds r0, r0, r1
  ldr r0, [r0, #0]
  mov pc, r0
  .align 2, 0
_0800E5B4: .4byte 0x0800E5B8   @ jump table base (points at first entry)
@ 12-entry jump table: idx = cmd - 1
_0800E5B8: .4byte _0800E63A
_0800E5BC: .4byte _0800E5E8
_0800E5C0: .4byte _0800E648
_0800E5C4: .4byte _0800E648
_0800E5C8: .4byte _0800E5F2
_0800E5CC: .4byte _0800E60C
_0800E5D0: .4byte _0800E604
_0800E5D4: .4byte _0800E648
_0800E5D8: .4byte _0800E648
_0800E5DC: .4byte _0800E648
_0800E5E0: .4byte _0800E648
_0800E5E4: .4byte _0800E642
@ case 2: dispatch via sub_0800DD9C
_0800E5E8:
  adds r0, r4, #0
  adds r1, r5, #0
  bl sub_0800DD9C
  b _0800E648
@ case 5: sub_0800D854 + sub_0800D8E4 refresh pair
_0800E5F2:
  adds r0, r4, #0
  adds r0, #32
  bl sub_0800D854
  adds r0, r4, #0
  adds r0, #136
  bl sub_0800D8E4
  b _0800E648
@ case 7: item renderer
_0800E604:
  adds r0, r4, #0
  bl sub_0800E418
  b _0800E648
@ case 6: key/param handler (sub_0800E15C / sub_0800E2E4)
_0800E60C:
  ldrh r0, [r4, #36]
  cmp r0, #0
  beq _0800E648
  movs r1, #20
  ldrsh r0, [r4, r1]
  cmp r0, #0
  bne _0800E62A
  lsls r1, r5, #16
  lsrs r1, r1, #16
  lsls r2, r2, #16
  lsrs r2, r2, #16
  adds r0, r4, #0
  bl sub_0800E15C
  b _0800E648
_0800E62A:
  lsls r1, r5, #16
  lsrs r1, r1, #16
  lsls r2, r2, #16
  lsrs r2, r2, #16
  adds r0, r4, #0
  bl sub_0800E2E4
  b _0800E648
@ case 1: record initializer
_0800E63A:
  adds r0, r4, #0
  bl sub_0800DE50
  b _0800E648
@ case 12: tiny stub
_0800E642:
  adds r0, r4, #0
  bl sub_0800DE4C
_0800E648:
  pop {r4, r5}
  pop {r0}
  bx r0
  .align 2, 0
