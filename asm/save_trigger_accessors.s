@ GT Advance 3 - save/timer trigger table accessors
@ Region: file offset 0x005F2C-0x005F8C (VMA 0x08005F2C-0x08005F8C).
@ Pure Thumb; four identical signed-halfword table lookups with private pools.

.thumb
.type sub_08005F2C, %function
sub_08005F2C:
_08005F2C:
	ldr r2, _08005F3C
	ldr r1, _08005F40
	ands r1, r0
	adds r1, r1, r2
	movs r2, #0
	ldrsh r0, [r1, r2]
	bx lr
	.short 0
_08005F3C: .word 0x0805CAF0
_08005F40: .word 0x00000FFE

.type sub_08005F44, %function
sub_08005F44:
_08005F44:
	ldr r2, _08005F54
	ldr r1, _08005F58
	ands r1, r0
	adds r1, r1, r2
	movs r2, #0
	ldrsh r0, [r1, r2]
	bx lr
	.short 0
_08005F54: .word 0x0805BAF0
_08005F58: .word 0x00000FFE

.type sub_08005F5C, %function
sub_08005F5C:
_08005F5C:
	ldr r2, _08005F6C
	ldr r1, _08005F70
	ands r1, r0
	adds r1, r1, r2
	movs r2, #0
	ldrsh r0, [r1, r2]
	bx lr
	.short 0
_08005F6C: .word 0x0805CAF0
_08005F70: .word 0x00000FFE

.type sub_08005F74, %function
sub_08005F74:
_08005F74:
	ldr r2, _08005F84
	ldr r1, _08005F88
	ands r1, r0
	adds r1, r1, r2
	movs r2, #0
	ldrsh r0, [r1, r2]
	bx lr
	.short 0
_08005F84: .word 0x0805BAF0
_08005F88: .word 0x00000FFE
save_trigger_accessors_end:
