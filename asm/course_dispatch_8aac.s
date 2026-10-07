@ GT Advance 3 - course record dispatcher + big phase machine
@ Region: file offset 0x008AAC-0x0095F0 (VMA 0x08008AAC-0x080095F0).
@ Pure Thumb; continuation of the record-builder cluster
@ (course_builders_8290.s ends at 0x8AAC; the small leaf cluster resumes at
@ 0x95F0). Three functions:
@
@   sub_08008AAC - 2/3-record emit: two s16 randoms (sub_08002BE8), table
@                  offsets via s16[rec+22] -> u16 table 0x080CB110/0x080CB132
@                  -> sub_08002C48 pan/vol, then 1-3 packed 8-byte records
@                  from rec[+0x34]/[+0x36] cells (y<<12 | x), calling
@                  sub_08008768 / sub_080088B0 for the variant rows.
@   sub_08008BB8 - 3-record emit variant: same random/table setup but
@                  packed cells from rec[+0x38]/[+0x3A], pool 0x030003E0,
@                  masks 0x01FF/0xFFFFC000/0xFFFF8000.
@   sub_08008CA0 - BIG record-49 phase machine (0x8CA0-0x95E8): builds a
@                  stacked course summary via sub_08003130/sub_08003D18
@                  (course count -> 4 x 8-byte heading records + u8 lane
@                  grid), fills four EWRAM/ROM table pairs, then switches on
@                  `s16[0x03001780+0x10FC]` via the 9-entry jump table
@                  @0x08008EB0 {0:done, 1:records, 2:sub_08009278,
@                  3:sub_080090A4, 4:records, 5:sub_0800928C, 6:done,
@                  7:done, 8:records}; each case emits packed records via
@                  sub_08008768/0x88B0/0x89FC/0x8AAC/0x8BB8 and writes
@                  stream rows via sub_08007B18 (11+ identical per-record
@                  x/y column writes), decrements countdown s16[rec+4] and
@                  checks `s16[rec+22]`/`[+24]` variants.
@
@ Transcribed from baserom.gba via objdump; byte-exact (make SHA gate).
@ Unconverted targets keep numeric `bl 0x0800XXXX` form. Pools + the
@ 9-entry jump table at original offsets.

.thumb

@ ----------------------------------------------------------------------------
.type sub_08008AAC, %function
sub_08008AAC:
_08008AAC:
  push {r4, r5, r6, r7, lr}
  adds r4, r0, #0
  bl 0x08002BE8
  lsls r0, r0, #16
  asrs r5, r0, #16
  bl 0x08002BE8
  lsls r0, r0, #16
  asrs r6, r0, #16
  ldr r1, _08008B6C
  ldr r7, _08008B70
  ldr r0, [r7, #0]
  movs r2, #22
  ldrsh r0, [r0, r2]
  lsls r0, r0, #1
  adds r0, r0, r1
  ldrh r1, [r0, #0]
  adds r0, r5, #0
  bl 0x08002C48
  ldr r1, _08008B74
  ldr r0, [r7, #0]
  movs r3, #22
  ldrsh r0, [r0, r3]
  lsls r0, r0, #1
  adds r0, r0, r1
  ldrh r1, [r0, #0]
  adds r0, r6, #0
  bl 0x08002C48
  ldr r2, [r7, #0]
  movs r1, #16
  ldrsh r0, [r2, r1]
  cmp r0, #1
  ble _08008B8C
  ldr r0, [r2, #84]
  subs r0, #16
  movs r1, #255
  ands r0, r1
  movs r3, #134
  lsls r3, r3, #7
  adds r1, r3, #0
  orrs r0, r1
  strh r0, [r4, #0]
  ldr r0, [r2, #80]
  subs r0, #20
  ldr r3, _08008B78
  adds r1, r3, #0
  ands r0, r1
  ldr r3, _08008B7C
  adds r1, r3, #0
  orrs r0, r1
  lsls r1, r6, #9
  orrs r0, r1
  strh r0, [r4, #2]
  ldrh r1, [r2, #58]
  lsls r0, r1, #12
  ldrh r3, [r2, #60]
  orrs r0, r3
  strh r0, [r4, #4]
  adds r4, #8
  movs r1, #16
  ldrsh r0, [r2, r1]
  lsls r3, r5, #9
  cmp r0, #9
  ble _08008B50
  ldr r1, _08008B80
  adds r0, r1, #0
  strh r0, [r4, #0]
  ldr r0, _08008B84
  adds r1, r0, #0
  adds r0, r3, #0
  orrs r0, r1
  strh r0, [r4, #2]
  ldrh r0, [r2, #56]
  adds r0, #16
  ldrh r2, [r2, #58]
  lsls r1, r2, #12
  orrs r0, r1
  strh r0, [r4, #4]
  adds r4, #8
_08008B50:
  ldr r1, _08008B80
  adds r0, r1, #0
  strh r0, [r4, #0]
  ldr r2, _08008B88
  adds r0, r2, #0
  orrs r3, r0
  strh r3, [r4, #2]
  ldr r1, [r7, #0]
  ldrh r3, [r1, #58]
  lsls r0, r3, #12
  ldrh r1, [r1, #56]
  orrs r0, r1
  b _08008BA4
  .align 2, 0
_08008B6C: .4byte 0x080CB110
_08008B70: .4byte 0x030003E0
_08008B74: .4byte 0x080CB132
_08008B78: .4byte 0x000001FF
_08008B7C: .4byte 0xFFFFC000
_08008B80: .4byte 0x0000033A
_08008B84: .4byte 0xFFFF8084
_08008B88: .4byte 0xFFFF8098
_08008B8C:
  ldr r1, _08008BB0
  adds r0, r1, #0
  strh r0, [r4, #0]
  lsls r0, r5, #9
  ldr r3, _08008BB4
  adds r1, r3, #0
  orrs r0, r1
  strh r0, [r4, #2]
  ldrh r1, [r2, #58]
  lsls r0, r1, #12
  ldrh r2, [r2, #56]
  orrs r0, r2
_08008BA4:
  strh r0, [r4, #4]
  adds r4, #8
  adds r0, r4, #0
  pop {r4, r5, r6, r7}
  pop {r1}
  bx r1
  .align 2, 0
_08008BB0: .4byte 0x0000433A
_08008BB4: .4byte 0xFFFFC088


@ ----------------------------------------------------------------------------
.type sub_08008BB8, %function
sub_08008BB8:
_08008BB8:
  push {r4, r5, r6, r7, lr}
  mov r7, r8
  push {r7}
  adds r3, r0, #0
  ldr r0, _08008C50
  mov ip, r0
  ldr r2, [r0, #0]
  movs r1, #16
  ldrsh r0, [r2, r1]
  cmp r0, #1
  ble _08008C60
  ldr r5, [r2, #84]
  movs r4, #255
  mov r8, r4
  adds r0, r5, #0
  ands r0, r4
  movs r7, #128
  lsls r7, r7, #7
  adds r1, r7, #0
  orrs r0, r1
  strh r0, [r3, #0]
  ldr r4, [r2, #80]
  adds r0, r4, #0
  adds r0, #12
  ldr r1, _08008C54
  adds r6, r1, #0
  ands r0, r6
  ldr r7, _08008C58
  adds r1, r7, #0
  orrs r0, r1
  strh r0, [r3, #2]
  ldrh r1, [r2, #58]
  lsls r0, r1, #12
  ldrh r7, [r2, #60]
  orrs r0, r7
  strh r0, [r3, #4]
  adds r3, #8
  movs r1, #16
  ldrsh r0, [r2, r1]
  cmp r0, #9
  ble _08008C2E
  subs r0, r5, #6
  mov r7, r8
  ands r0, r7
  strh r0, [r3, #0]
  adds r0, r4, #0
  subs r0, #20
  ands r0, r6
  ldr r4, _08008C5C
  adds r1, r4, #0
  orrs r0, r1
  strh r0, [r3, #2]
  ldrh r0, [r2, #56]
  adds r0, #16
  ldrh r2, [r2, #58]
  lsls r1, r2, #12
  orrs r0, r1
  strh r0, [r3, #4]
  adds r3, #8
_08008C2E:
  mov r7, ip
  ldr r2, [r7, #0]
  ldr r0, [r2, #84]
  subs r0, #6
  mov r1, r8
  ands r0, r1
  strh r0, [r3, #0]
  ldr r0, [r2, #80]
  ands r0, r6
  ldr r4, _08008C5C
  adds r1, r4, #0
  orrs r0, r1
  strh r0, [r3, #2]
  ldrh r7, [r2, #58]
  lsls r0, r7, #12
  b _08008C84
  .align 2, 0
_08008C50: .4byte 0x030003E0
_08008C54: .4byte 0x000001FF
_08008C58: .4byte 0xFFFFC000
_08008C5C: .4byte 0xFFFF8000
_08008C60:
  ldr r0, [r2, #84]
  subs r0, #6
  movs r1, #255
  ands r0, r1
  movs r4, #128
  lsls r4, r4, #7
  adds r1, r4, #0
  orrs r0, r1
  strh r0, [r3, #0]
  ldr r0, [r2, #80]
  ldr r1, _08008C98
  ands r0, r1
  ldr r7, _08008C9C
  adds r1, r7, #0
  orrs r0, r1
  strh r0, [r3, #2]
  ldrh r1, [r2, #58]
  lsls r0, r1, #12
_08008C84:
  ldrh r2, [r2, #56]
  orrs r0, r2
  strh r0, [r3, #4]
  adds r3, #8
  adds r0, r3, #0
  pop {r3}
  mov r8, r3
  pop {r4, r5, r6, r7}
  pop {r1}
  bx r1
  .align 2, 0
_08008C98: .4byte 0x000001FF
_08008C9C: .4byte 0xFFFFC000

@ ----------------------------------------------------------------------------
.type sub_08008CA0, %function
sub_08008CA0:
_08008CA0:
  push {r4, r5, r6, r7, lr}
  mov r7, sl
  mov r6, r9
  mov r5, r8
  push {r5, r6, r7}
  sub sp, #80
  bl 0x08003130
  adds r7, r0, #0
  ldr r4, _08008E8C
  ldr r0, [r4, #0]
  movs r2, #2
  ldrsh r1, [r0, r2]
  movs r0, #100
  muls r0, r1
  movs r1, #160
  bl 0x0802DE04
  add r1, sp, #20
  bl 0x08003D18
  adds r2, r0, #0
  add r3, sp, #36
  mov sl, r3
  mov r5, sp
  adds r5, #44
  str r5, [sp, #68]
  add r6, sp, #52
  mov r8, r6
  add r0, sp, #60
  mov r9, r0
  cmp r2, #0
  ble _08008D24
  lsls r1, r2, #3
  mov ip, r4
  mov r0, sp
  adds r0, #35
  subs r4, r0, r2
  subs r1, r2, r1
  adds r3, r1, #0
  adds r3, #175
  ldr r1, _08008E90
  adds r5, r1, #0
_08008CF6:
  strh r5, [r7, #0]
  ldr r6, _08008E94
  adds r1, r6, #0
  adds r0, r3, #0
  ands r0, r1
  strh r0, [r7, #2]
  mov r0, ip
  ldr r1, [r0, #0]
  ldrb r0, [r4, #0]
  subs r0, #48
  lsls r0, r0, #1
  ldrh r6, [r1, #52]
  adds r0, r6, r0
  ldrh r1, [r1, #54]
  lsls r1, r1, #12
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
  adds r4, #1
  adds r3, #7
  subs r2, #1
  cmp r2, #0
  bne _08008CF6
_08008D24:
  ldr r1, _08008E90
  adds r0, r1, #0
  strh r0, [r7, #0]
  movs r0, #202
  strh r0, [r7, #2]
  ldr r3, _08008E8C
  ldr r2, [r3, #0]
  ldrh r4, [r2, #6]
  lsls r0, r4, #1
  ldrh r5, [r2, #52]
  adds r0, r5, r0
  ldrh r6, [r2, #54]
  lsls r1, r6, #12
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
  ldr r0, _08008E98
  ldr r1, [r0, #4]
  ldr r0, [r0, #0]
  str r0, [sp, #36]
  str r1, [sp, #40]
  ldr r0, _08008E9C
  ldr r1, [r0, #4]
  ldr r0, [r0, #0]
  str r0, [sp, #44]
  str r1, [sp, #48]
  ldr r0, _08008EA0
  ldr r1, [r0, #4]
  ldr r0, [r0, #0]
  str r0, [sp, #52]
  str r1, [sp, #56]
  ldr r0, _08008EA4
  ldr r1, [r0, #4]
  ldr r0, [r0, #0]
  str r0, [sp, #60]
  str r1, [sp, #64]
  movs r1, #0
  ldrsh r0, [r2, r1]
  cmp r0, #0
  bge _08008D76
  adds r0, #3
_08008D76:
  lsls r0, r0, #14
  lsrs r4, r0, #16
  asrs r0, r0, #16
  cmp r0, #111
  bgt _08008D82
  movs r4, #112
_08008D82:
  lsls r4, r4, #20
  lsrs r4, r4, #20
  mov r0, sl
  adds r1, r4, #0
  bl 0x08005BA8
  ldr r0, [sp, #68]
  adds r1, r4, #0
  bl 0x08005BA8
  mov r0, r8
  adds r1, r4, #0
  bl 0x08005BA8
  mov r0, r9
  adds r1, r4, #0
  bl 0x08005BA8
  ldr r6, [sp, #52]
  adds r6, #199
  lsls r6, r6, #16
  lsrs r6, r6, #16
  str r6, [sp, #72]
  mov r2, r8
  ldr r4, [r2, #4]
  adds r4, #134
  lsls r4, r4, #16
  lsrs r4, r4, #16
  ldr r3, [sp, #60]
  mov r8, r3
  movs r5, #203
  add r8, r5
  mov r6, r8
  lsls r6, r6, #16
  lsrs r6, r6, #16
  mov r8, r6
  mov r0, r9
  ldr r5, [r0, #4]
  adds r5, #138
  lsls r5, r5, #16
  lsrs r5, r5, #16
  str r5, [sp, #76]
  bl 0x08002BE8
  mov r9, r0
  mov r1, r9
  lsls r1, r1, #16
  lsrs r1, r1, #16
  mov r9, r1
  ldr r1, [sp, #36]
  lsls r1, r1, #16
  asrs r1, r1, #16
  mov r3, sl
  movs r5, #4
  ldrsh r2, [r3, r5]
  ldr r3, [sp, #44]
  lsls r3, r3, #16
  asrs r3, r3, #16
  ldr r6, [sp, #68]
  movs r5, #4
  ldrsh r0, [r6, r5]
  str r0, [sp, #0]
  mov r0, r9
  bl 0x08002D98
  lsls r4, r4, #16
  asrs r4, r4, #16
  movs r6, #255
  mov sl, r6
  ands r4, r6
  movs r0, #128
  lsls r0, r0, #1
  adds r3, r0, #0
  orrs r4, r3
  strh r4, [r7, #0]
  ldr r1, [sp, #72]
  lsls r6, r1, #16
  asrs r6, r6, #16
  ldr r2, _08008E94
  ands r6, r2
  mov r4, r9
  lsls r4, r4, #9
  mov r9, r4
  movs r5, #128
  lsls r5, r5, #7
  adds r1, r5, #0
  mov r0, r9
  orrs r0, r1
  orrs r6, r0
  strh r6, [r7, #2]
  ldr r6, _08008E8C
  ldr r1, [r6, #0]
  ldrh r4, [r1, #38]
  lsls r0, r4, #12
  ldrh r1, [r1, #44]
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
  ldr r6, [sp, #76]
  lsls r5, r6, #16
  asrs r5, r5, #16
  mov r0, sl
  ands r5, r0
  orrs r5, r3
  strh r5, [r7, #0]
  mov r1, r8
  lsls r1, r1, #16
  asrs r1, r1, #16
  ands r1, r2
  mov r2, r9
  orrs r1, r2
  strh r1, [r7, #2]
  ldr r3, _08008E8C
  ldr r1, [r3, #0]
  ldrh r4, [r1, #38]
  lsls r0, r4, #12
  ldrh r1, [r1, #46]
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
  ldr r0, _08008EA8
  ldr r5, _08008EAC
  adds r0, r0, r5
  movs r6, #0
  ldrsh r0, [r0, r6]
  cmp r0, #8
  bls _08008E82
  b _080095D4
_08008E82:
  lsls r0, r0, #2
  ldr r1, _08008EB0
  adds r0, r0, r1
  ldr r0, [r0, #0]
  mov pc, r0
  .align 2, 0
_08008E8C: .4byte 0x030003E0
_08008E90: .4byte 0x0000808F
_08008E94: .4byte 0x000001FF
_08008E98: .4byte 0x0805F634
_08008E9C: .4byte 0x0805F63C
_08008EA0: .4byte 0x0805F644
_08008EA4: .4byte 0x0805F64C
_08008EA8: .4byte 0x03001780
_08008EAC: .4byte 0x000010FC
_08008EB0: .4byte 0x08008EB4

@ jump table (9 entries)
_08008EB4: .4byte 0x080095D4
_08008EB8: .4byte 0x08008ED8
_08008EBC: .4byte 0x08009278
_08008EC0: .4byte 0x080090A4
_08008EC4: .4byte 0x08008ED8
_08008EC8: .4byte 0x0800928C
_08008ECC: .4byte 0x080095D4
_08008ED0: .4byte 0x080095D4
_08008ED4: .4byte 0x08008ED8

@ case 1/4/8: records
_08008ED8:
  ldr r6, _08009098
  ldr r0, [r6, #0]
  ldr r1, [r0, #64]
  adds r0, r7, #0
  bl sub_08008768
  adds r7, r0, #0
  ldr r0, [r6, #0]
  ldr r1, [r0, #68]
  adds r0, r7, #0
  bl sub_080088B0
  adds r7, r0, #0
  movs r2, #2
  strh r2, [r7, #0]
  ldr r1, _0800909C
  adds r0, r1, #0
  strh r0, [r7, #2]
  ldr r1, [r6, #0]
  ldrh r3, [r1, #38]
  lsls r0, r3, #12
  ldrh r1, [r1, #50]
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
  strh r2, [r7, #0]
  ldr r4, _080090A0
  adds r0, r4, #0
  strh r0, [r7, #2]
  ldr r1, [r6, #0]
  ldrh r5, [r1, #38]
  lsls r0, r5, #12
  ldrh r1, [r1, #48]
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
  movs r0, #128
  lsls r0, r0, #8
  adds r2, r0, #0
  strh r2, [r7, #0]
  movs r0, #220
  strh r0, [r7, #2]
  ldr r1, [r6, #0]
  ldrh r0, [r1, #52]
  adds r0, #24
  ldrh r1, [r1, #54]
  lsls r1, r1, #12
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
  strh r2, [r7, #0]
  movs r0, #227
  strh r0, [r7, #2]
  ldr r2, [r6, #0]
  ldrh r1, [r2, #10]
  lsls r0, r1, #1
  ldrh r3, [r2, #52]
  adds r0, r3, r0
  ldrh r4, [r2, #54]
  lsls r1, r4, #12
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
  adds r0, r2, #0
  adds r0, #88
  ldrh r1, [r2, #38]
  str r1, [sp, #0]
  movs r5, #1
  str r5, [sp, #4]
  movs r4, #0
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #12
  movs r2, #0
  movs r3, #0
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #15
  movs r2, #64
  movs r3, #4
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #15
  movs r2, #80
  movs r3, #4
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #15
  movs r2, #96
  movs r3, #4
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #16
  movs r2, #104
  movs r3, #4
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #13
  movs r2, #120
  movs r3, #0
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #16
  movs r2, #168
  movs r3, #4
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #14
  movs r2, #184
  movs r3, #0
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #15
  movs r2, #208
  movs r3, #4
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #16
  movs r2, #224
  movs r3, #4
  bl sub_08007B18
  ldr r1, [r6, #0]
  ldrh r2, [r1, #4]
  movs r5, #4
  ldrsh r0, [r1, r5]
  cmp r0, #0
  bgt _0800906A
  b _080095D4
_0800906A:
  subs r0, r2, #1
  strh r0, [r1, #4]
  lsls r0, r0, #16
  asrs r0, r0, #16
  cmp r0, #63
  ble _0800908C
  movs r2, #4
  ldrsh r0, [r1, r2]
  movs r1, #8
  bl 0x0802D978
  movs r1, #2
  bl 0x0802D97C
  cmp r0, #0
  bne _0800908C
  b _080095D4
_0800908C:
  ldr r0, [r6, #0]
  ldr r1, [r0, #72]
  adds r0, r7, #0
  bl sub_0800861C
  b _080095D2
  .align 2, 0
_08009098: .4byte 0x030003E0
_0800909C: .4byte 0x000040A7
_080090A0: .4byte 0x000040CD

@ case 3: sub_080090A4
_080090A4:
  ldr r4, _080090FC
  ldr r0, [r4, #0]
  adds r0, #78
  ldrb r0, [r0, #0]
  cmp r0, #0
  beq _080090C8
  ldr r0, _08009100
  ldr r3, _08009104
  adds r0, r0, r3
  ldr r0, [r0, #0]
  movs r1, #3
  bl 0x0802D978
  movs r1, #2
  bl 0x0802D97C
  cmp r0, #0
  beq _080090D4
_080090C8:
  ldr r0, [r4, #0]
  ldr r1, [r0, #64]
  adds r0, r7, #0
  bl sub_08008768
  adds r7, r0, #0
_080090D4:
  ldr r4, _080090FC
  ldr r1, [r4, #0]
  ldrh r0, [r1, #34]
  cmp r0, #0
  beq _08009108
  subs r0, #1
  strh r0, [r1, #34]
  ldrh r0, [r1, #34]
  movs r1, #3
  bl 0x0802D978
  movs r1, #2
  bl 0x0802D97C
  cmp r0, #0
  beq _08009112
  ldr r0, [r4, #0]
  ldr r1, [r0, #72]
  b _0800910A
  .align 2, 0
_080090FC: .4byte 0x030003E0
_08009100: .4byte 0x03001780
_08009104: .4byte 0x000010D8
_08009108:
  ldr r1, [r1, #68]
_0800910A:
  adds r0, r7, #0
  bl sub_080088B0
  adds r7, r0, #0
_08009112:
  movs r0, #2
  strh r0, [r7, #0]
  ldr r4, _08009270
  adds r0, r4, #0
  strh r0, [r7, #2]
  ldr r6, _08009274
  ldr r1, [r6, #0]
  ldrh r5, [r1, #38]
  lsls r0, r5, #12
  ldrh r1, [r1, #48]
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
  movs r0, #128
  lsls r0, r0, #8
  adds r2, r0, #0
  strh r2, [r7, #0]
  movs r0, #220
  strh r0, [r7, #2]
  ldr r1, [r6, #0]
  ldrh r0, [r1, #52]
  adds r0, #24
  ldrh r1, [r1, #54]
  lsls r1, r1, #12
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
  strh r2, [r7, #0]
  movs r0, #227
  strh r0, [r7, #2]
  ldr r2, [r6, #0]
  ldrh r1, [r2, #10]
  lsls r0, r1, #1
  ldrh r3, [r2, #52]
  adds r0, r3, r0
  ldrh r4, [r2, #54]
  lsls r1, r4, #12
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
  adds r0, r2, #0
  adds r0, #88
  ldrh r1, [r2, #38]
  str r1, [sp, #0]
  movs r5, #1
  str r5, [sp, #4]
  movs r4, #0
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #12
  movs r2, #0
  movs r3, #0
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #15
  movs r2, #64
  movs r3, #4
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #15
  movs r2, #80
  movs r3, #4
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #15
  movs r2, #96
  movs r3, #4
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #16
  movs r2, #104
  movs r3, #4
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #14
  movs r2, #184
  movs r3, #0
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #15
  movs r2, #208
  movs r3, #4
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #16
  movs r2, #224
  movs r3, #4
  bl sub_08007B18
  ldr r1, [r6, #0]
  ldrh r2, [r1, #4]
  movs r5, #4
  ldrsh r0, [r1, r5]
  cmp r0, #0
  bgt _08009242
  b _080095D4
_08009242:
  subs r0, r2, #1
  strh r0, [r1, #4]
  lsls r0, r0, #16
  asrs r0, r0, #16
  cmp r0, #63
  ble _08009264
  movs r2, #4
  ldrsh r0, [r1, r2]
  movs r1, #8
  bl 0x0802D978
  movs r1, #2
  bl 0x0802D97C
  cmp r0, #0
  bne _08009264
  b _080095D4
_08009264:
  ldr r0, [r6, #0]
  ldr r1, [r0, #72]
  adds r0, r7, #0
  bl sub_0800861C
  b _080095D2
  .align 2, 0
_08009270: .4byte 0x000040CD
_08009274: .4byte 0x030003E0

@ case 2: sub_08009278
_08009278:
  ldr r0, _08009288
  ldr r0, [r0, #0]
  ldr r1, [r0, #68]
  adds r0, r7, #0
  bl sub_080088B0
  b _080095D2
  .align 2, 0
_08009288: .4byte 0x030003E0

@ case 5: sub_0800928C
_0800928C:
  movs r0, #2
  strh r0, [r7, #0]
  ldr r3, _080093B8
  adds r0, r3, #0
  strh r0, [r7, #2]
  ldr r6, _080093BC
  ldr r1, [r6, #0]
  ldrh r4, [r1, #38]
  lsls r0, r4, #12
  ldrh r1, [r1, #48]
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
  movs r5, #128
  lsls r5, r5, #8
  adds r2, r5, #0
  strh r2, [r7, #0]
  movs r0, #220
  strh r0, [r7, #2]
  ldr r1, [r6, #0]
  ldrh r0, [r1, #52]
  adds r0, #24
  ldrh r1, [r1, #54]
  lsls r1, r1, #12
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
  strh r2, [r7, #0]
  movs r0, #227
  strh r0, [r7, #2]
  ldr r2, [r6, #0]
  ldrh r1, [r2, #10]
  lsls r0, r1, #1
  ldrh r3, [r2, #52]
  adds r0, r3, r0
  ldrh r4, [r2, #54]
  lsls r1, r4, #12
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
  adds r0, r2, #0
  adds r0, #88
  ldrh r1, [r2, #38]
  str r1, [sp, #0]
  movs r5, #1
  str r5, [sp, #4]
  movs r4, #0
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #14
  movs r2, #184
  movs r3, #0
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #15
  movs r2, #208
  movs r3, #4
  bl sub_08007B18
  ldr r1, [r6, #0]
  adds r0, r1, #0
  adds r0, #88
  ldrh r1, [r1, #38]
  str r1, [sp, #0]
  str r5, [sp, #4]
  str r4, [sp, #8]
  str r4, [sp, #12]
  movs r1, #16
  movs r2, #224
  movs r3, #4
  bl sub_08007B18
  ldr r1, [r6, #0]
  movs r5, #26
  ldrsh r0, [r1, r5]
  cmp r0, #0
  ble _080093D0
  bl 0x08002BE8
  ldr r2, [r6, #0]
  ldrh r1, [r2, #26]
  subs r1, #1
  strh r1, [r2, #26]
  lsls r0, r0, #16
  asrs r5, r0, #16
  ldr r2, _080093C0
  movs r0, #15
  ands r1, r0
  lsls r1, r1, #1
  adds r1, r1, r2
  ldrh r1, [r1, #0]
  adds r0, r5, #0
  bl 0x08002C48
  ldr r0, [r6, #0]
  movs r1, #18
  ldrsh r0, [r0, r1]
  cmp r0, #9
  ble _08009390
  ldr r2, _080093C4
  adds r0, r2, #0
  strh r0, [r7, #0]
  lsls r0, r5, #9
  ldr r3, _080093C8
  adds r1, r3, #0
  orrs r0, r1
  strh r0, [r7, #2]
  ldr r4, [r6, #0]
  movs r1, #18
  ldrsh r0, [r4, r1]
  movs r1, #10
  bl 0x0802DE04
  lsls r0, r0, #16
  asrs r0, r0, #14
  ldrh r2, [r4, #40]
  adds r0, r2, r0
  ldrh r4, [r4, #38]
  lsls r1, r4, #12
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
_08009390:
  ldr r3, _080093C4
  adds r0, r3, #0
  strh r0, [r7, #0]
  lsls r0, r5, #9
  ldr r4, _080093CC
  adds r1, r4, #0
  orrs r0, r1
  strh r0, [r7, #2]
  ldr r4, [r6, #0]
  movs r5, #18
  ldrsh r0, [r4, r5]
  movs r1, #10
  bl 0x0802DE9C
  lsls r0, r0, #16
  asrs r0, r0, #14
  ldrh r6, [r4, #40]
  adds r0, r6, r0
  b _0800941E
  .align 2, 0
_080093B8: .4byte 0x000040CD
_080093BC: .4byte 0x030003E0
_080093C0: .4byte 0x080CB154
_080093C4: .4byte 0x00000386
_080093C8: .4byte 0x00004044
_080093CC: .4byte 0x00004050
_080093D0:
  movs r2, #18
  ldrsh r0, [r1, r2]
  cmp r0, #9
  ble _08009400
  movs r0, #142
  strh r0, [r7, #0]
  ldr r3, _08009514
  adds r0, r3, #0
  strh r0, [r7, #2]
  ldr r4, [r6, #0]
  movs r5, #18
  ldrsh r0, [r4, r5]
  movs r1, #10
  bl 0x0802DE04
  lsls r0, r0, #16
  asrs r0, r0, #14
  ldrh r1, [r4, #40]
  adds r0, r1, r0
  ldrh r4, [r4, #38]
  lsls r1, r4, #12
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
_08009400:
  movs r0, #142
  strh r0, [r7, #0]
  ldr r2, _08009518
  adds r0, r2, #0
  strh r0, [r7, #2]
  ldr r4, [r6, #0]
  movs r3, #18
  ldrsh r0, [r4, r3]
  movs r1, #10
  bl 0x0802DE9C
  lsls r0, r0, #16
  asrs r0, r0, #14
  ldrh r5, [r4, #40]
  adds r0, r5, r0
_0800941E:
  ldrh r4, [r4, #38]
  lsls r1, r4, #12
  orrs r0, r1
  strh r0, [r7, #4]
  adds r7, #8
  ldr r2, _0800951C
  ldr r0, [r2, #0]
  ldrh r0, [r0, #32]
  cmp r0, #0
  beq _0800946A
  cmp r0, #16
  bhi _0800946A
  movs r6, #144
  lsls r6, r6, #3
  adds r0, r6, #0
  strh r0, [r7, #0]
  ldr r1, _08009520
  adds r0, r1, #0
  strh r0, [r7, #2]
  ldr r2, [r2, #0]
  ldrh r3, [r2, #38]
  lsls r0, r3, #12
  ldrh r4, [r2, #42]
  orrs r0, r4
  strh r0, [r7, #4]
  adds r7, #8
  ldr r1, _08009524
  movs r5, #232
  lsls r5, r5, #3
  adds r0, r5, #0
  strh r0, [r1, #0]
  ldr r3, _08009528
  ldrh r1, [r2, #32]
  movs r0, #16
  subs r0, r0, r1
  lsls r0, r0, #8
  orrs r0, r1
  strh r0, [r3, #0]
_0800946A:
  ldr r4, _0800951C
  ldr r1, [r4, #0]
  ldrh r2, [r1, #28]
  movs r6, #28
  ldrsh r0, [r1, r6]
  cmp r0, #0
  ble _080094F4
  subs r0, r2, #1
  movs r2, #0
  mov r9, r2
  strh r0, [r1, #28]
  adds r1, #79
  ldrb r0, [r1, #0]
  adds r0, #1
  strb r0, [r1, #0]
  ldr r0, [r4, #0]
  adds r1, r0, #0
  adds r1, #79
  ldrb r3, [r1, #0]
  cmp r3, #15
  bls _08009498
  movs r0, #1
  strb r0, [r1, #0]
_08009498:
  ldr r0, _0800952C
  ldr r1, [r4, #0]
  adds r2, r1, #0
  adds r2, #79
  ldrb r2, [r2, #0]
  ldrh r3, [r1, #58]
  movs r1, #1
  bl sub_08007614
  ldr r4, [r4, #0]
  mov r8, r4
  mov r0, r8
  adds r0, #96
  ldrh r1, [r4, #62]
  ldr r2, _08009530
  movs r5, #30
  ldrsh r4, [r4, r5]
  lsls r4, r4, #3
  adds r4, r4, r2
  movs r6, #4
  ldrsh r2, [r4, r6]
  movs r5, #0
  ldrsh r3, [r4, r5]
  movs r5, #2
  ldrsh r6, [r4, r5]
  mov sl, r6
  mov r6, r8
  movs r4, #28
  ldrsh r5, [r6, r4]
  movs r4, #60
  subs r4, r4, r5
  lsrs r5, r4, #31
  adds r4, r4, r5
  asrs r4, r4, #1
  mov r5, sl
  subs r6, r5, r4
  str r6, [sp, #0]
  mov r6, r8
  ldrh r4, [r6, #58]
  str r4, [sp, #4]
  mov r4, r9
  str r4, [sp, #8]
  str r4, [sp, #12]
  str r4, [sp, #16]
  bl sub_08007BFC
_080094F4:
  ldr r2, _0800951C
  ldr r4, [r2, #0]
  movs r0, #168
  str r0, [r4, #80]
  movs r5, #80
  str r5, [r4, #84]
  movs r6, #24
  ldrsh r0, [r4, r6]
  cmp r0, #2
  beq _080095A0
  cmp r0, #2
  bgt _08009534
  cmp r0, #1
  beq _0800953A
  b _080095D4
  .align 2, 0
_08009514: .4byte 0x0000404C
_08009518: .4byte 0x00004058
_0800951C: .4byte 0x030003E0
_08009520: .4byte 0x000040A0
_08009524: .4byte 0x04000050
_08009528: .4byte 0x04000052
_0800952C: .4byte 0x0828A35C
_08009530: .4byte 0x080CB074
_08009534:
  cmp r0, #3
  beq _0800956C
  b _080095D4
_0800953A:
  ldrh r1, [r4, #22]
  subs r1, #1
  strh r1, [r4, #22]
  ldr r2, _08009568
  movs r3, #22
  ldrsh r0, [r4, r3]
  lsls r0, r0, #1
  adds r0, r0, r2
  movs r6, #0
  ldrsh r0, [r0, r6]
  adds r0, #168
  str r0, [r4, #80]
  str r5, [r4, #84]
  lsls r1, r1, #16
  cmp r1, #0
  bgt _0800955E
  movs r0, #2
  strh r0, [r4, #24]
_0800955E:
  adds r0, r7, #0
  bl sub_08008AAC
  b _080095D2
  .align 2, 0
_08009568: .4byte 0x080CB0CC
_0800956C:
  ldrh r2, [r4, #22]
  subs r2, #1
  strh r2, [r4, #22]
  ldr r3, _0800959C
  movs r0, #22
  ldrsh r1, [r4, r0]
  movs r0, #16
  subs r0, r0, r1
  lsls r0, r0, #1
  adds r0, r0, r3
  movs r1, #0
  ldrsh r0, [r0, r1]
  adds r0, #168
  str r0, [r4, #80]
  str r5, [r4, #84]
  lsls r2, r2, #16
  cmp r2, #0
  bgt _08009594
  movs r0, #0
  strh r0, [r4, #24]
_08009594:
  adds r0, r7, #0
  bl sub_08008BB8
  b _080095D2
  .align 2, 0
_0800959C: .4byte 0x080CB0EE
_080095A0:
  adds r0, r4, #0
  adds r0, #79
  ldrb r1, [r0, #0]
  adds r1, #1
  strb r1, [r0, #0]
  ldr r0, [r2, #0]
  adds r1, r0, #0
  adds r1, #79
  ldrb r3, [r1, #0]
  cmp r3, #15
  bls _080095BA
  movs r0, #1
  strb r0, [r1, #0]
_080095BA:
  ldr r0, _080095EC
  ldr r1, [r2, #0]
  adds r2, r1, #0
  adds r2, #79
  ldrb r2, [r2, #0]
  ldrh r3, [r1, #58]
  movs r1, #1
  bl sub_08007614
  adds r0, r7, #0
  bl sub_080089FC
_080095D2:
  adds r7, r0, #0
_080095D4:
  adds r0, r7, #0
  bl 0x0800313C
  add sp, #80
  pop {r3, r4, r5}
  mov r8, r3
  mov r9, r4
  mov sl, r5
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .align 2, 0
_080095EC: .4byte 0x0828A35C
