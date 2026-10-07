@ GT Advance 3 - course stream cursor reset
@ Region: file offset 0x006B20-0x006B30 (VMA 0x08006B20-0x08006B30).
@ Exact Thumb leaf and literal pool.

.thumb
.type sub_08006B20, %function
sub_08006B20:
_08006B20:
  ldr r1, _08006B2C
  movs r2, #1
  negs r2, r2
  adds r0, r2, #0
  strh r0, [r1, #0]
  bx lr
_08006B2C: .word 0x0203F874
@ Region end (VMA 0x08006B30). Emits no bytes; gives the promotion screen an
@ end marker for the last function in this region instead of guessing.
course_cursor_reset_end:
