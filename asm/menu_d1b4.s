@ GT Advance 3 - garage cursor box handlers
@ Region: file offset 0x00D1B4-0x00D280 (VMA 0x0800D1B4-0x0800D280).
@ Pure Thumb; five units closing the menu tranche between _0800D13C and
@ sub_0800D280. Event dispatcher shape matches _0800D17C (events 1/2/6/7):
@   ev1 -> reset cursor box to 120x60, ev2 -> no-op stub,
@   ev6 -> redraw frame via _080038A4, ev7 -> dpad delta mover.
@ Runtime dispatch note: a descriptor table at ROM 0x0CB3F8 carries the
@ Thumb pointer 0x0800D245 for sub_0800D244 ({fn,size,data} records).

.thumb
.type sub_0800D1B4, %function
sub_0800D1B4:
_0800D1B4:
  movs r1, #120
  strh r1, [r0, #4]
  movs r1, #60
  strh r1, [r0, #6]
  bx lr
  .short 0x0000

.type sub_0800D1C0, %function
sub_0800D1C0:
_0800D1C0:
  push {r4, lr}
  adds r4, r0, #0
  ldr r2, _0800D1E4
  movs r0, #60
  movs r1, #60
  bl 0x080038A4
  movs r1, #4
  ldrsh r0, [r4, r1]
  movs r2, #6
  ldrsh r1, [r4, r2]
  ldr r2, _0800D1E8
  bl 0x080038A4
  pop {r4}
  pop {r0}
  bx r0
  .short 0x0000
_0800D1E4: .word 0x0805F8A4
_0800D1E8: .word 0x0805F8AC

.type sub_0800D1EC, %function
sub_0800D1EC:
_0800D1EC:
  push {r4, r5, lr}
  adds r3, r0, #0
  adds r5, r2, #0
  movs r4, #0
  movs r2, #0
  movs r0, #32
  ands r0, r1
  cmp r0, #0
  beq.n _0800D200
  subs r4, #1
_0800D200:
  movs r0, #16
  ands r0, r1
  cmp r0, #0
  beq.n _0800D20A
  adds r4, #1
_0800D20A:
  movs r0, #64
  ands r0, r1
  cmp r0, #0
  beq.n _0800D214
  subs r2, #1
_0800D214:
  movs r0, #128
  ands r0, r1
  cmp r0, #0
  beq.n _0800D21E
  adds r2, #1
_0800D21E:
  ldrh r1, [r3, #4]
  adds r0, r1, r4
  strh r0, [r3, #4]
  ldrh r1, [r3, #6]
  adds r0, r1, r2
  strh r0, [r3, #6]
  movs r0, #2
  ands r0, r5
  cmp r0, #0
  beq.n _0800D238
  movs r0, #1
  bl 0x08004EA8
_0800D238:
  pop {r4, r5}
  pop {r0}
  bx r0
  .short 0x0000

.type sub_0800D240, %function
sub_0800D240:
_0800D240:
  bx lr
  .short 0x0000

.type sub_0800D244, %function
sub_0800D244:
_0800D244:
  push {lr}
  cmp r0, #2
  beq.n _0800D25E
  cmp r0, #2
  bhi.n _0800D254
  cmp r0, #1
  beq.n _0800D266
  b.n _0800D27C
_0800D254:
  cmp r0, #6
  beq.n _0800D276
  cmp r0, #7
  beq.n _0800D26E
  b.n _0800D27C
_0800D25E:
  adds r0, r3, #0
  bl 0x0800D240
  b.n _0800D27C
_0800D266:
  adds r0, r3, #0
  bl 0x0800D1B4
  b.n _0800D27C
_0800D26E:
  adds r0, r3, #0
  bl 0x0800D1C0
  b.n _0800D27C
_0800D276:
  adds r0, r3, #0
  bl 0x0800D1EC
_0800D27C:
  pop {r0}
  bx r0
@ Region-final synthetic end label (cf. boot_end:, idle_end:, code_57d0_end:).
@ Byte-neutral: emits no instruction. The region header fixes the span at
@ 0x0800D1B4-0x0800D280 and sub_0800D244 is the last.type in this file, so the
@ manifest (which has no cross-file end-marker precedent) needs a same-file one.
menu_d1b4_end:
