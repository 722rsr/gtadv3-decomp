@ GT Advance 3 - course streaming dispatcher + surface row-streamer + proximity collector
@ Region: file offset 0x006650-0x006A50 (VMA 0x08006650-0x08006A50).
@ Pure Thumb; the only external BL caller is raw code @ 0x01D112.
@ Companion: tools/track_dump.py (resource package layout).
@
@ Three functions:
@   sub_08006650  - dispatcher: if s16[0x0203F8A4] < s16[0x0203F870]
@                   stream one row group (sub_08006678), else re-collect
@                   the nearby-course set (sub_080068D4).
@   sub_08006678  - surface row-streamer: 16 unrolled CpuSet copies from the
@                   EWRAM surface map (base 0x02000000) into VRAM at 0x06004000
@                   (row stride 0x80, src += width), or a zero-fill path that
@                   pushes zero halfwords through the stack; then bumps the
@                   stream cursor s16[0x0203F8A4].
@   sub_080068D4  - proximity collector: camera pos u32[0x0203F6B0/4], per-row
@                   origin table 0x0805DBF4, entry table 0x0805DCF4 (200 B /
@                   column of {s8 dx, s8 dy,...} 4-byte records); collects up
@                   to 4 non-duplicate nearby records into the u32 pair arrays
@                   at 0x0203F8C0 / 0x0203F8C4 (8-byte stride), dedup cache at
@                   0x0203F770, and resets s16[0x0203F870] + s16[0x0203F8A4].
@
@ EWRAM cells (all in the 0x0203Fxxx session block):
@   0x0203F6B0/4  camera x/y (u32)
@   0x0203F728    camera-halfword source for the column selector
@   0x0203F760    pointer to surface header {u16 w, u16 h at +8/+10}
@   0x0203F770    byte dedup cache (64 x {s8 x, s8 y}, idx = ((y&7)<<3|(x&7))<<2)
@   0x0203F870    collected-record count (s16)
@   0x0203F8A4    stream cursor (s16)
@   0x0203F8B0/4  rounded dx/dy tiles (u32)
@   0x0203F8C0    record x array (u32, 8-byte stride)
@   0x0203F8C4    record y array (u32, 8-byte stride)
@   0x0203F8E0/4  ceil(camera/0x8000) x/y (u32)
@
@ Transcribed from baserom.gba via objdump + gbadisasm cross-check; byte-exact
@ (make SHA gate).

.thumb

@ ----------------------------------------------------------------------------
.type sub_08006650, %function
sub_08006650:
_08006650:
  push {lr}
  ldr r0, _08006668        @ =0x0203F8A4
  ldr r1, _0800666C        @ =0x0203F870
  movs r3, #0
  ldrsh r2, [r0, r3]       @ r2 = s16[0x0203F8A4] (stream cursor)
  movs r3, #0
  ldrsh r0, [r1, r3]       @ r0 = s16[0x0203F870] (target count)
  cmp r2, r0
  bge _08006670
  bl _08006678             @ stream one row group
  b _08006674
  .align 2, 0
_08006668: .4byte 0x0203F8A4
_0800666C: .4byte 0x0203F870
_08006670:
  bl _080068D4             @ re-collect the nearby-course set
_08006674:
  pop {r0}
  bx r0

@ ----------------------------------------------------------------------------
_08006678:
  push {r4, r5, r6, r7, lr}
  sub sp, #32
  ldr r2, _080067B4        @ =0x0203F8C0 (record x array)
  ldr r0, _080067B8        @ =0x0203F8A4 (stream cursor)
  movs r1, #0
  ldrsh r0, [r0, r1]       @ r0 = s16 cursor
  lsls r0, r0, #3          @ r0 = cursor * 8
  adds r1, r0, r2          @ r1 = 0x0203F8C0 + cursor*8
  ldr r1, [r1]             @ r1 = x cell
  lsls r3, r1, #4
  adds r2, #4              @ r2 = 0x0203F8C4
  adds r0, r0, r2          @ r0 = 0x0203F8C4 + cursor*8
  ldr r0, [r0]             @ r0 = y cell
  lsls r4, r0, #4
  movs r1, #112            @ 0x70
  adds r2, r4, #0
  ands r2, r1
  lsls r2, r2, #7          @ r2 = (y&0x70)<<7
  adds r0, r3, #0
  ands r0, r1              @ r0 = x&0x70
  ldr r1, _080067BC        @ =0x06004000 (VRAM tile base)
  adds r0, r0, r1
  adds r6, r2, r0          @ r6 = VRAM dest base
  ldr r0, _080067C0        @ =0x0203F760
  ldr r0, [r0]             @ r0 = u32[0x0203F760]
  ldr r0, [r0]             @ r0 = surface header
  ldrh r7, [r0, #8]        @ r7 = width
  ldrh r0, [r0, #10]       @ r0 = height
  cmp r3, #0
  bge _080066B6
  b _080067C8              @ x < 0 -> zero-fill
_080066B6:
  cmp r4, #0
  bge _080066BC
  b _080067C8              @ y < 0 -> zero-fill
_080066BC:
  cmp r7, r3
  bgt _080066C2
  b _080067C8              @ x >= width -> zero-fill
_080066C2:
  cmp r0, r4
  bgt _080066C8
  b _080067C8              @ y >= height -> zero-fill
_080066C8:
  muls r4, r7              @ r4 = y * width
  movs r1, #128
  lsls r1, r1, #18         @ r1 = 0x02000000 (surface map base)
  adds r0, r3, r1
  adds r4, r4, r0          @ r4 = src base
  ldr r5, _080067C4        @ =0x04000004 (CpuSet ctrl: 4 words, 32-bit)
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974          @ CpuSet(src, dst, ctrl)
  adds r4, r4, r7          @ src += width
  adds r6, #128            @ dst += 0x80
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r4, r4, r7
  adds r6, #128
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r4, r4, r7
  adds r6, #128
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r4, r4, r7
  adds r6, #128
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r4, r4, r7
  adds r6, #128
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r4, r4, r7
  adds r6, #128
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r4, r4, r7
  adds r6, #128
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r4, r4, r7
  adds r6, #128
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r4, r4, r7
  adds r6, #128
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r4, r4, r7
  adds r6, #128
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r4, r4, r7
  adds r6, #128
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r4, r4, r7
  adds r6, #128
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r4, r4, r7
  adds r6, #128
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r4, r4, r7
  adds r6, #128
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r4, r4, r7
  adds r6, #128
  adds r0, r4, #0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  b _080068BA
  .align 2, 0
_080067B4: .4byte 0x0203F8C0
_080067B8: .4byte 0x0203F8A4
_080067BC: .4byte 0x06004000
_080067C0: .4byte 0x0203F760
_080067C4: .4byte 0x04000004
_080067C8:                 @ zero-fill path: 16 rows of 8 zero halfwords
  mov r0, sp
  movs r4, #0
  strh r4, [r0]            @ sp[0] = 0
  ldr r5, _080068CC        @ =0x01000008
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r6, #128
  mov r0, sp
  adds r0, #2
  strh r4, [r0]            @ sp[2] = 0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r6, #128
  add r0, sp, #4
  strh r4, [r0]            @ sp[4] = 0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r6, #128
  mov r0, sp
  adds r0, #6
  strh r4, [r0]            @ sp[6] = 0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r6, #128
  add r0, sp, #8
  strh r4, [r0]            @ sp[8] = 0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r6, #128
  mov r0, sp
  adds r0, #10
  strh r4, [r0]            @ sp[10] = 0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r6, #128
  add r0, sp, #12
  strh r4, [r0]            @ sp[12] = 0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r6, #128
  mov r0, sp
  adds r0, #14
  strh r4, [r0]            @ sp[14] = 0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r6, #128
  add r0, sp, #16
  strh r4, [r0]            @ sp[16] = 0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r6, #128
  mov r0, sp
  adds r0, #18
  strh r4, [r0]            @ sp[18] = 0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r6, #128
  add r0, sp, #20
  strh r4, [r0]            @ sp[20] = 0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r6, #128
  mov r0, sp
  adds r0, #22
  strh r4, [r0]            @ sp[22] = 0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r6, #128
  add r0, sp, #24
  strh r4, [r0]            @ sp[24] = 0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r6, #128
  mov r0, sp
  adds r0, #26
  strh r4, [r0]            @ sp[26] = 0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r6, #128
  add r0, sp, #28
  strh r4, [r0]            @ sp[28] = 0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
  adds r6, #128
  mov r0, sp
  adds r0, #30
  strh r4, [r0]            @ sp[30] = 0
  adds r1, r6, #0
  adds r2, r5, #0
  bl sub_0802D974
_080068BA:
  ldr r1, _080068D0        @ =0x0203F8A4
  ldrh r0, [r1]
  adds r0, #1
  strh r0, [r1]            @ s16[0x0203F8A4]++
  add sp, #32
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .align 2, 0
_080068CC: .4byte 0x01000008
_080068D0: .4byte 0x0203F8A4

@ ----------------------------------------------------------------------------
_080068D4:
  push {r4, r5, r6, r7, lr}
  mov r7, sl
  mov r6, r9
  mov r5, r8
  push {r5, r6, r7}
  ldr r1, _08006948        @ =0x0203F728
  movs r2, #192
  lsls r2, r2, #4          @ r2 = 0xC00
  adds r0, r2, #0
  ldrh r1, [r1]            @ r1 = u16[0x0203F728]
  subs r0, r0, r1          @ r0 = 0xC00 - u16
  ldr r3, _0800694C        @ =0x00000FFF
  adds r1, r3, #0
  ands r0, r1              @ r0 &= 0xFFF
  adds r0, #64
  asrs r4, r0, #7          @ r4 = (v + 64) >> 7
  movs r0, #31
  ands r4, r0              @ r4 = column selector (0..31)
  ldr r2, _08006950        @ =0x0203F8E0
  ldr r1, _08006954        @ =0x0203F6B0 (camera)
  ldr r7, [r1]             @ r7 = cam_x
  mov ip, r7
  mov r3, ip
  asrs r0, r3, #11         @ r0 = cam_x >> 11
  cmp r0, #0
  bge _0800690A
  adds r0, #15
_0800690A:
  asrs r0, r0, #4          @ r0 = ceil(cam_x / 0x8000)
  str r0, [r2]             @ u32[0x0203F8E0] = r0
  ldr r7, [r1, #4]         @ r7 = cam_y
  asrs r0, r7, #11
  cmp r0, #0
  bge _08006918
  adds r0, #15
_08006918:
  asrs r0, r0, #4          @ r0 = ceil(cam_y / 0x8000)
  str r0, [r2, #4]         @ u32[0x0203F8E4] = r0
  ldr r2, _08006958        @ =0x0805DBF4 (row-origin table, 8 B/entry)
  lsls r4, r4, #16
  asrs r1, r4, #13         @ r1 = r4 * 8
  adds r0, r1, r2
  ldr r3, [r0]             @ r3 = origin_x[col]
  mov r0, ip
  subs r3, r0, r3          @ r3 = cam_x - origin_x
  adds r5, r3, #0
  adds r2, #4
  adds r1, r1, r2
  ldr r0, [r1]             @ r0 = origin_y[col]
  subs r6, r7, r0          @ r6 = cam_y - origin_y
  cmp r3, #0
  bge _0800695C
  asrs r0, r5, #11
  cmp r0, #0
  bge _08006940
  adds r0, #15
_08006940:
  asrs r0, r0, #4
  subs r5, r0, #1          @ r5 = floor(dx / 0x8000)
  b _08006966
  .align 2, 0
_08006948: .4byte 0x0203F728
_0800694C: .4byte 0x00000FFF
_08006950: .4byte 0x0203F8E0
_08006954: .4byte 0x0203F6B0
_08006958: .4byte 0x0805DBF4
_0800695C:
  asrs r0, r5, #11
  cmp r0, #0
  bge _08006964
  adds r0, #15
_08006964:
  asrs r5, r0, #4          @ r5 = ceil(dx / 0x8000)
_08006966:
  cmp r6, #0
  bge _08006978
  asrs r0, r6, #11
  cmp r0, #0
  bge _08006972
  adds r0, #15
_08006972:
  asrs r0, r0, #4
  subs r6, r0, #1          @ r6 = floor(dy / 0x8000)
  b _08006982
_08006978:
  asrs r0, r6, #11
  cmp r0, #0
  bge _08006980
  adds r0, #15
_08006980:
  asrs r6, r0, #4          @ r6 = ceil(dy / 0x8000)
_08006982:
  ldr r0, _08006A38        @ =0x0203F8B0
  str r5, [r0]             @ u32[0x0203F8B0] = dx tiles
  str r6, [r0, #4]         @ u32[0x0203F8B4] = dy tiles
  asrs r1, r4, #16         @ r1 = r4 (column)
  movs r0, #200
  muls r1, r0              @ r1 = col * 200
  ldr r0, _08006A3C        @ =0x0805DCF4 (per-column entry table)
  adds r1, r1, r0          @ r1 = entries + col*200
  mov ip, r1
  ldr r2, _08006A40        @ =0x0203F870
  movs r1, #0
  strh r1, [r2]            @ s16[0x0203F870] = 0
  ldr r0, _08006A44        @ =0x0203F8A4
  strh r1, [r0]            @ s16[0x0203F8A4] = 0
  mov r1, ip
  movs r0, #0
  ldrsb r0, [r1, r0]       @ r0 = s8[entry+0] (loop guard: >= 0 = valid)
  cmp r0, #0
  blt _08006A2A
  movs r3, #7
  mov r8, r3               @ r8 = 7
  ldr r7, _08006A48        @ =0x0203F8C0
  mov r9, r7               @ r9 = 0x0203F8C0
  movs r0, #4
  add r0, r9
  mov sl, r0               @ sl = 0x0203F8C4
  mov r4, ip               @ r4 = current entry
_080069B8:
  movs r0, #0
  ldrsb r0, [r4, r0]       @ r0 = s8[entry+0] (dx)
  adds r2, r0, r5          @ r2 = dx + tile_x
  movs r0, #1
  ldrsb r0, [r4, r0]       @ r0 = s8[entry+1] (dy)
  adds r3, r0, r6          @ r3 = dy + tile_y
  adds r1, r3, #0
  mov r7, r8
  ands r1, r7              @ r1 = y & 7
  lsls r1, r1, #3          @ r1 = (y&7)<<3
  adds r0, r2, #0
  ands r0, r7              @ r0 = x & 7
  orrs r1, r0              @ r1 = ((y&7)<<3) | (x&7)
  lsls r1, r1, #2          @ r1 = idx * 4
  ldr r0, _08006A4C        @ =0x0203F770 (dedup cache)
  adds r1, r1, r0
  movs r0, #0
  ldrsb r0, [r1, r0]       @ cached x
  cmp r0, r2
  bne _080069E8
  movs r0, #1
  ldrsb r0, [r1, r0]       @ cached y
  cmp r0, r3
  beq _08006A18            @ duplicate -> skip store
_080069E8:
  strb r2, [r1]            @ cache x
  strb r3, [r1, #1]        @ cache y
  ldr r7, _08006A40        @ =0x0203F870
  movs r0, #0
  ldrsh r1, [r7, r0]       @ r1 = count
  lsls r1, r1, #3          @ r1 = count*8
  add r1, r9               @ r1 = 0x0203F8C0 + count*8
  lsls r0, r2, #24
  asrs r0, r0, #24         @ sign-extend x
  str r0, [r1]             @ u32[0x0203F8C0+count*8] = x
  movs r2, #0
  ldrsh r1, [r7, r2]
  lsls r1, r1, #3
  add r1, sl               @ r1 = 0x0203F8C4 + count*8
  lsls r0, r3, #24
  asrs r0, r0, #24         @ sign-extend y
  str r0, [r1]             @ u32[0x0203F8C4+count*8] = y
  ldrh r0, [r7]
  adds r0, #1
  strh r0, [r7]            @ count++
  lsls r0, r0, #16
  asrs r0, r0, #16
  cmp r0, #3
  bgt _08006A2A            @ count > 3 -> stop
_08006A18:
  adds r4, #4              @ next entry
  mov r0, ip
  adds r0, #196            @ end = base + 0xC4
  cmp r4, r0
  bgt _08006A2A
  movs r0, #0
  ldrsb r0, [r4, r0]
  cmp r0, #0
  bge _080069B8            @ loop while s8[entry] >= 0
_08006A2A:
  pop {r3, r4, r5}
  mov r8, r3
  mov r9, r4
  mov sl, r5
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .align 2, 0
_08006A38: .4byte 0x0203F8B0
_08006A3C: .4byte 0x0805DCF4
_08006A40: .4byte 0x0203F870
_08006A44: .4byte 0x0203F8A4
_08006A48: .4byte 0x0203F8C0
_08006A4C: .4byte 0x0203F770
