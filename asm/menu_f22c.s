@ GT Advance 3 - menu record handlers V
@ Region: file offset 0x00F22C-0x00F5A0 (VMA 0x0800F22C-0x0800F5A0).
@ Pure Thumb, ARMCC; continuation of the menu span (menu_f040.s ends at
@ 0xF22C; next raw resumes at 0xF5A0).
@
@ Function map:
@   sub_0800F22C - large menu record setup/bind (sl/r9/r8, sp frame
@                  24 B): initial sub_08007B18(rec+0x4C, 7, 0x48, 0x58,
@                  {7,3,1,0}), rec+0x13C via sub_08026A2C(0x78, 0x38),
@                  class-byte gate on 0x080CCEEC[rec+0xFA] (+8 == 1)
@                  -> sub_08007B18(rec, 11, 0x80, 0x68, {9,1,1,0}),
@                  rec+0xE4 halfword dispatch driving sub_08007B18
@                  fills {0x48,0x58}/{0x80,0x68}/{0x98,0xA8}/... via
@                  0x080CB508/0x080CB548 cursor tables, then a
@                  rec+0x108 halfword switch {0 -> sub_08007B18(rec,
@                  9, 0x60, 0x80), 1 -> sub_08007B18(rec, 10, 0x78,
@                  0x80)} with {tier,1,1,0} frames, then four
@                  sub_08007BFC record binds at rec+8/+16/+24/+32
@                  with {0x8C,0x8E}/{0x90,0x92}/{0x94,0x96}/...
@                  slot pairs, two sub_0800399C lane writes (rec+0xF4/
@                  0xF6), rec+0x10A==1 gate -> sub_08007C68(rec+0x20,
@                  {0x98,0x9A}, {0x70,0xA,1,0,0}), sub_0800D97C(rec+0xF0,
@                  15) then sub_0800DBE8(rec+0x28), tail jumps into
@                  sub_0800F59C (bx lr stub).
@   sub_0800F59C - bx lr stub (tail target of sub_0800F22C).
@
@ Transcribed from baserom.gba via objdump; byte-exact (make SHA gate).
@ Unconverted targets keep numeric `bl 0x0800XXXX` form. Pools at
@ original offsets.

.thumb

@ ----------------------------------------------------------------------------
.type sub_0800F22C, %function
sub_0800F22C:
_0800F22C:
  push {r4, r5, r6, r7, lr}
  mov r7, sl
  mov r6, r9
  mov r5, r8
  push {r5, r6, r7}
  sub sp, #24
  adds r7, r0, #0
  adds r0, #76             @ 0x4c
  movs r1, #7
  str r1, [sp, #0]
  movs r1, #3
  str r1, [sp, #4]
  movs r1, #1
  str r1, [sp, #8]
  movs r5, #0
  str r5, [sp, #12]
  movs r1, #8
  movs r2, #72             @ 0x48
  movs r3, #88             @ 0x58
  bl sub_08007B18
  movs r1, #158            @ 0x9e
  lsls r1, r1, #1
  adds r0, r7, r1
  ldr r0, [r0, #0]
  movs r1, #120            @ 0x78
  movs r2, #56             @ 0x38
  bl 0x08026A2C
  ldr r2, _0800F42C
  adds r0, r7, #0
  adds r0, #250            @ 0xfa
  movs r3, #0
  ldrsh r1, [r0, r3]
  lsls r0, r1, #2
  adds r0, r0, r1
  lsls r0, r0, #2
  adds r0, r0, r2
  movs r1, #8
  ldrsb r1, [r0, r1]
  cmp r1, #1
  bne _0800F296
  movs r0, #9
  str r0, [sp, #0]
  str r1, [sp, #4]
  str r1, [sp, #8]
  str r5, [sp, #12]
  adds r0, r7, #0
  movs r1, #11
  movs r2, #128            @ 0x80
  movs r3, #104            @ 0x68
  bl sub_08007B18
_0800F296:
  adds r0, r7, #0
  adds r0, #228            @ 0xe4
  adds r6, r0, #0
  ldrh r4, [r6, #0]
  cmp r4, #2
  beq _0800F348
  ldr r0, _0800F430
  ldr r2, [r0, #8]
  ldr r0, [r0, #12]
  mov r8, r0
  ldr r0, _0800F434
  ldrh r1, [r0, #2]
  ldr r0, _0800F438
  ldr r3, _0800F43C
  adds r0, r0, r3
  ldrh r0, [r0, #0]
  cmp r0, #1
  bne _0800F2D8
  adds r0, r7, #0
  adds r0, #242            @ 0xf2
  movs r3, #0
  ldrsh r4, [r0, r3]
  cmp r4, #1
  bne _0800F2D8
  movs r0, #5
  str r0, [sp, #0]
  str r4, [sp, #4]
  str r4, [sp, #8]
  str r5, [sp, #12]
  adds r0, r7, #0
  mov r3, r8
  bl sub_08007B18
_0800F2D8:
  ldr r0, _0800F430
  ldr r2, [r0, #16]
  ldr r3, [r0, #20]
  ldr r0, _0800F434
  ldrh r1, [r0, #4]
  ldr r0, _0800F438
  ldr r4, _0800F440
  adds r0, r0, r4
  ldrh r0, [r0, #0]
  cmp r0, #1
  bne _0800F30C
  adds r0, r7, #0
  adds r0, #242            @ 0xf2
  movs r5, #0
  ldrsh r4, [r0, r5]
  cmp r4, #1
  bne _0800F30C
  movs r0, #5
  str r0, [sp, #0]
  str r4, [sp, #4]
  str r4, [sp, #8]
  movs r0, #0
  str r0, [sp, #12]
  adds r0, r7, #0
  bl sub_08007B18
_0800F30C:
  ldrh r6, [r6, #0]
  cmp r6, #1
  beq _0800F348
  ldr r0, _0800F430
  ldr r5, [r0, #24]
  ldr r6, [r0, #28]
  ldr r0, _0800F434
  ldrh r4, [r0, #6]
  bl sub_080027EC
  cmp r0, #1
  bne _0800F348
  adds r0, r7, #0
  adds r0, #242            @ 0xf2
  movs r2, #0
  ldrsh r1, [r0, r2]
  cmp r1, #1
  bne _0800F348
  movs r0, #5
  str r0, [sp, #0]
  str r1, [sp, #4]
  str r1, [sp, #8]
  movs r0, #0
  str r0, [sp, #12]
  adds r0, r7, #0
  adds r1, r4, #0
  adds r2, r5, #0
  adds r3, r6, #0
  bl sub_08007B18
_0800F348:
  ldr r2, _0800F430
  adds r0, r7, #0
  adds r0, #224            @ 0xe0
  movs r3, #0
  ldrsh r0, [r0, r3]
  lsls r0, r0, #3
  adds r1, r0, r2
  ldr r6, [r1, #0]
  adds r2, #4
  adds r0, r0, r2
  ldr r5, [r0, #0]
  movs r0, #130            @ 0x82
  lsls r0, r0, #1
  adds r4, r7, r0
  ldr r2, [r4, #0]
  adds r0, r7, #0
  adds r1, r5, #0
  bl sub_0800F040
  ldr r3, [r4, #0]
  adds r0, r7, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0800F0BC
  adds r0, r7, #0
  adds r0, #160            @ 0xa0
  ldr r2, [r0, #0]
  adds r0, #4
  ldr r3, [r0, #0]
  movs r1, #6
  mov r8, r1
  str r1, [sp, #0]
  movs r6, #1
  str r6, [sp, #4]
  str r6, [sp, #8]
  movs r4, #0
  mov r9, r4
  str r4, [sp, #12]
  adds r0, r7, #0
  movs r1, #8
  bl sub_08007B18
  adds r0, r7, #0
  adds r0, #168            @ 0xa8
  ldr r2, [r0, #0]
  adds r0, #4
  ldr r3, [r0, #0]
  mov r5, r8
  str r5, [sp, #0]
  str r6, [sp, #4]
  str r6, [sp, #8]
  str r4, [sp, #12]
  adds r0, r7, #0
  movs r1, #8
  bl sub_08007B18
  adds r0, r7, #0
  adds r0, #176            @ 0xb0
  ldr r2, [r0, #0]
  adds r0, #4
  ldr r3, [r0, #0]
  str r5, [sp, #0]
  str r6, [sp, #4]
  str r6, [sp, #8]
  str r4, [sp, #12]
  adds r0, r7, #0
  movs r1, #8
  bl sub_08007B18
  adds r5, r7, #0
  adds r5, #192            @ 0xc0
  ldr r2, [r5, #0]
  adds r4, r7, #0
  adds r4, #196            @ 0xc4
  ldr r3, [r4, #0]
  mov r0, r8
  str r0, [sp, #0]
  str r6, [sp, #4]
  str r6, [sp, #8]
  mov r1, r9
  str r1, [sp, #12]
  adds r0, r7, #0
  movs r1, #6
  bl sub_08007B18
  adds r0, r7, #0
  adds r0, #184            @ 0xb8
  ldr r2, [r0, #0]
  adds r0, #4
  ldr r3, [r0, #0]
  mov r0, r8
  str r0, [sp, #0]
  str r6, [sp, #4]
  str r6, [sp, #8]
  mov r1, r9
  str r1, [sp, #12]
  adds r0, r7, #0
  movs r1, #7
  bl sub_08007B18
  movs r2, #132            @ 0x84
  lsls r2, r2, #1
  adds r0, r7, r2
  movs r3, #0
  ldrsh r0, [r0, r3]
  mov sl, r5
  str r4, [sp, #20]
  cmp r0, #0
  beq _0800F444
  cmp r0, #1
  beq _0800F45E
  b _0800F476
  .align 2, 0
_0800F42C: .4byte 0x080CCEEC
_0800F430: .4byte 0x080CB508
_0800F434: .4byte 0x080CB548
_0800F438: .4byte 0x03001780
_0800F43C: .4byte 0x00001058
_0800F440: .4byte 0x0000105A
_0800F444:
  mov r4, r8
  str r4, [sp, #0]
  str r6, [sp, #4]
  str r6, [sp, #8]
  mov r5, r9
  str r5, [sp, #12]
  adds r0, r7, #0
  movs r1, #9
  movs r2, #96             @ 0x60
  movs r3, #128            @ 0x80
  bl sub_08007B18
  b _0800F476
_0800F45E:
  mov r1, r8
  str r1, [sp, #0]
  str r0, [sp, #4]
  str r0, [sp, #8]
  mov r2, r9
  str r2, [sp, #12]
  adds r0, r7, #0
  movs r1, #10
  movs r2, #120            @ 0x78
  movs r3, #128            @ 0x80
  bl sub_08007B18
_0800F476:
  adds r0, r7, #0
  adds r0, #8
  movs r3, #134            @ 0x86
  lsls r3, r3, #1
  adds r1, r7, r3
  ldr r1, [r1, #0]
  movs r4, #136            @ 0x88
  lsls r4, r4, #1
  adds r2, r7, r4
  ldr r2, [r2, #0]
  adds r3, r7, #0
  adds r3, #200            @ 0xc8
  ldr r3, [r3, #0]
  adds r4, r7, #0
  adds r4, #204            @ 0xcc
  ldr r4, [r4, #0]
  str r4, [sp, #0]
  movs r6, #8
  str r6, [sp, #4]
  movs r5, #1
  str r5, [sp, #8]
  str r5, [sp, #12]
  movs r4, #0
  mov r8, r4
  str r4, [sp, #16]
  bl sub_08007BFC
  adds r0, r7, #0
  adds r0, #16
  movs r2, #140            @ 0x8c
  lsls r2, r2, #1
  adds r1, r7, r2
  ldr r1, [r1, #0]
  movs r3, #142            @ 0x8e
  lsls r3, r3, #1
  adds r2, r7, r3
  ldr r2, [r2, #0]
  adds r3, r7, #0
  adds r3, #208            @ 0xd0
  ldr r3, [r3, #0]
  adds r4, r7, #0
  adds r4, #212            @ 0xd4
  ldr r4, [r4, #0]
  str r4, [sp, #0]
  str r6, [sp, #4]
  str r5, [sp, #8]
  str r5, [sp, #12]
  mov r4, r8
  str r4, [sp, #16]
  bl sub_08007BFC
  adds r0, r7, #0
  adds r0, #24
  movs r2, #146            @ 0x92
  lsls r2, r2, #1
  adds r1, r7, r2
  ldr r1, [r1, #0]
  movs r3, #148            @ 0x94
  lsls r3, r3, #1
  adds r2, r7, r3
  ldr r2, [r2, #0]
  adds r3, r7, #0
  adds r3, #216            @ 0xd8
  ldr r3, [r3, #0]
  adds r4, r7, #0
  adds r4, #220            @ 0xdc
  ldr r4, [r4, #0]
  str r4, [sp, #0]
  str r6, [sp, #4]
  str r5, [sp, #8]
  str r5, [sp, #12]
  mov r4, r8
  str r4, [sp, #16]
  bl sub_08007BFC
  mov r5, sl
  ldr r0, [r5, #0]
  adds r0, #24
  ldr r2, [sp, #20]
  ldr r1, [r2, #0]
  adds r1, #8
  adds r2, r7, #0
  adds r2, #244            @ 0xf4
  movs r3, #0
  ldrsh r2, [r2, r3]
  bl 0x0800399C
  ldr r0, [r5, #0]
  adds r0, #48             @ 0x30
  ldr r4, [sp, #20]
  ldr r1, [r4, #0]
  adds r1, #8
  adds r2, r7, #0
  adds r2, #246            @ 0xf6
  movs r5, #0
  ldrsh r2, [r2, r5]
  bl 0x0800399C
  movs r1, #133            @ 0x85
  lsls r1, r1, #1
  adds r0, r7, r1
  movs r2, #0
  ldrsh r4, [r0, r2]
  cmp r4, #1
  bne _0800F572
  adds r0, r7, #0
  adds r0, #32
  movs r3, #152            @ 0x98
  lsls r3, r3, #1
  adds r1, r7, r3
  ldr r1, [r1, #0]
  movs r5, #154            @ 0x9a
  lsls r5, r5, #1
  adds r2, r7, r5
  ldr r2, [r2, #0]
  movs r3, #112            @ 0x70
  str r3, [sp, #0]
  movs r3, #10
  str r3, [sp, #4]
  str r4, [sp, #8]
  mov r3, r8
  str r3, [sp, #12]
  str r3, [sp, #16]
  movs r3, #20
  bl sub_08007C68
_0800F572:
  adds r0, r7, #0
  adds r0, #240            @ 0xf0
  movs r1, #15
  bl sub_0800D97C
  adds r0, r7, #0
  adds r0, #40             @ 0x28
  bl sub_0800DBE8
  adds r0, r7, #0
  bl sub_0800F59C
  add sp, #24
  pop {r3, r4, r5}
  mov r8, r3
  mov r9, r4
  mov sl, r5
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .2byte 0x0000            @ pad @ 0xF59A

@ ----------------------------------------------------------------------------
.type sub_0800F59C, %function
sub_0800F59C:
_0800F59C:
  bx lr
  .2byte 0x0000            @ pad @ 0xF59E

menu_f22c_end:
