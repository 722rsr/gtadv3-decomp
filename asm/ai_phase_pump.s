@ GT Advance 3 - race presentation/countdown phase pump
@ Region: file offset 0x023958-0x023A34 (VMA 0x08023958-0x08023A34).
@ Exact Thumb function; private IWRAM-base pool at the span end.

.thumb
.type sub_08023958, %function
sub_08023958:
_08023958:
  push {r4, r5, r6, r7, lr}
  adds r5, r0, #0
  lsls r2, r2, #16
  lsrs r6, r2, #16
  adds r4, r5, #0
  adds r4, #158
  ldrh r0, [r4, #0]
  subs r0, #5
  lsls r0, r0, #16
  lsrs r0, r0, #16
  cmp r0, #3
  bls.n _08023978
  ldrh r1, [r4, #0]
  movs r0, #4
  bl 0x08002158
_08023978:
  adds r7, r5, #0
  adds r7, #164
  ldrh r4, [r4, #0]
  cmp r4, #1
  bne.n _080239CA
  cmp r6, #2
  bne.n _0802399C
  movs r0, #4
  bl 0x0802B368
  adds r1, r5, #0
  adds r1, #166
  movs r2, #0
  ldrsh r0, [r1, r2]
  cmp r0, #0
  bne.n _0802399A
  strh r6, [r1, #0]
_0802399A:
  strh r6, [r7, #0]
_0802399C:
  cmp r6, #1
  bne.n _080239CA
  adds r0, r5, #0
  adds r0, #166
  movs r2, #0
  ldrsh r1, [r0, r2]
  adds r4, r0, #0
  cmp r1, #0
  bne.n _080239B2
  movs r0, #2
  strh r0, [r4, #0]
_080239B2:
  ldrh r0, [r4, #0]
  cmp r0, #1
  bne.n _080239C0
  movs r0, #1
  bl 0x0802B368
  b.n _080239C6
_080239C0:
  movs r0, #4
  bl 0x0802B368
_080239C6:
  ldrh r0, [r4, #0]
  strh r0, [r7, #0]
_080239CA:
  adds r0, r5, #0
  adds r0, #158
  movs r1, #0
  ldrsh r0, [r0, r1]
  cmp r0, #3
  beq.n _080239E2
  cmp r0, #10
  beq.n _080239E2
  adds r2, r5, #0
  adds r2, #162
  cmp r0, #11
  bne.n _080239FE
_080239E2:
  subs r0, r6, #1
  lsls r0, r0, #16
  lsrs r0, r0, #16
  adds r2, r5, #0
  adds r2, #162
  cmp r0, #1
  bhi.n _080239FE
  ldr r0, _08023A2C
  ldr r1, _08023A30
  adds r0, r0, r1
  ldrb r0, [r0, #0]
  cmp r0, #1
  bne.n _080239FE
  strh r0, [r2, #0]
_080239FE:
  cmp r6, #32
  bne.n _08023A0A
  adds r1, r5, #0
  adds r1, #166
  movs r0, #2
  strh r0, [r1, #0]
_08023A0A:
  cmp r6, #16
  bne.n _08023A16
  adds r1, r5, #0
  adds r1, #166
  movs r0, #1
  strh r0, [r1, #0]
_08023A16:
  ldrh r1, [r2, #0]
  movs r0, #6
  bl 0x08002158
  ldrh r1, [r7, #0]
  movs r0, #5
  bl 0x08002158
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
_08023A2C: .word 0x03001780
_08023A30: .word 0x000010C3

@ Byte-neutral end anchor for the splicer. This file declares exactly one
@ `@ Region:` (0x023958-0x023A34) and has no `.include`; the promoted body at
@ 0x08023958 ends on 0x023A34, exactly that boundary, so the anchor is safe.
@ Without it the body is refused as 'no end marker in ai_phase_pump.s'.
ai_phase_pump_end:
