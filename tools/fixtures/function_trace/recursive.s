/* Original diagnostic ROM; no game assets. candidate(2) returns 3.
 * Three recursive calls share an inner return address but have distinct SPs.
 */
.syntax unified
.cpu arm7tdmi
.section .text
.arm
.global _start
_start:
    b boot
.org 0xc0
boot:
    ldr sp, =0x03007f00
    ldr r0, =main + 1
    bx r0
.thumb
main:
    movs r0, #2
    bl candidate
    ldr r1, =0x02000000
    str r0, [r1]
stop:
    b stop
.balign 4
.global candidate
.thumb_func
candidate:
    ldr r2, =0x02000004
    ldr r3, [r2]
    adds r3, #1
    str r3, [r2]
    push {lr}
    cmp r0, #0
    beq base
    subs r0, #1
    bl candidate
    adds r0, #1
    b done
base:
    movs r0, #1
done:
    pop {r1}
    bx r1
.pool
