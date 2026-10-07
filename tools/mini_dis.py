#!/usr/bin/env python3
"""Minimal Thumb decoder for a ROM slice (works around local objdump
invocation flakiness). Decodes common instructions well enough to
follow control flow; unknown halfwords print as .hword with raw bits.

Usage: python3 tools/mini_dis.py 0x080019500 0x080019560
"""
import struct
import sys

rom = open('baserom.gba', 'rb').read()
MASK = 0x00FFFFFF


def rd2(a):
    o = a & MASK
    return struct.unpack('<H', rom[o:o + 2])[0]


def reg(n):
    return f"r{n}"


def dec_one(a):
    hw = rd2(a)
    op = hw >> 12
    if (hw >> 11) == 0b11110:
        off = rd2(a + 2)
        disp = ((hw & 0x7FF) << 11) | (off & 0x7FF)
        if disp & 0x100000:
            disp -= 0x200000
        return 4, f"bl {a + 4 + disp * 2:#x}"
    if (hw & 0xFF87) == 0x4700:
        rm = (hw >> 3) & 0xF
        return 2, f"bx r{rm}"
    if (hw & 0xFF00) == 0x4600:
        rd = (hw >> 0) & 7 | ((hw >> 4) & 8)
        rm = (hw >> 3) & 0xF
        return 2, f"mov r{rd}, r{rm}"
    if (hw & 0xFE00) == 0x1800 or (hw & 0xE000) == 0x1000:
        # data-processing register
        ops = {0: 'lsls', 1: 'lsrs', 2: 'asrs', 3: 'adds?', }
        return 2, f"alu {hw:04x}"
    if op == 0x2:
        rd = (hw >> 8) & 7
        imm = hw & 0xFF
        return 2, f"movs r{rd}, #{imm:#x}"
    if op == 0x3:
        rd = (hw >> 8) & 7
        imm = hw & 0xFF
        kind = (hw >> 11) & 3
        nm = {0: 'cmp', 1: 'adds', 2: 'subs'}.get(kind, 'op')
        return 2, f"{nm} r{rd}, #{imm:#x}"
    if op == 0x4 and (hw >> 10) != 0b010001:
        return 2, f"aluhi {hw:04x}"
    if op == 0x5:
        rd = hw & 7
        rn = (hw >> 3) & 7
        rm = (hw >> 6) & 7
        kind = (hw >> 9) & 3
        nm = ['str', 'strh', 'strb', 'ldrsb'][kind]
        return 2, f"{nm} r{rd}, [r{rn}, r{rm}]"
    if op in (6, 7, 8):
        rd = hw & 7
        rn = (hw >> 3) & 7
        imm = (hw >> 6) & 0x1F
        ld = (hw >> 11) & 1
        if op == 6:
            nm, sc = ('ldr' if ld else 'str'), 4
        elif op == 7:
            nm, sc = ('ldrb' if ld else 'strb'), 1
        else:
            nm, sc = ('ldrh' if ld else 'strh'), 2
        return 2, f"{nm} r{rd}, [r{rn}, #{imm * sc}]"
    if op == 0x9:
        rd = (hw >> 8) & 7
        imm = hw & 0xFF
        ld = (hw >> 11) & 1
        if ld:
            return 2, f"ldr r{rd}, [pc, #{imm * 4}]"
        return 2, f"str r{rd}, [sp, #{imm * 4}]"
    if op == 0xA:
        rd = (hw >> 8) & 7
        imm = hw & 0xFF
        sp = (hw >> 11) & 1
        return 2, f"{'add' if sp else 'add'} r{rd}, {'sp' if sp else 'pc'}, #{imm * 4}"
    if op == 0xB:
        kind = (hw >> 8) & 0xF
        if kind == 5:
            return 2, f"push {{{hw:04x}}}"
        if kind in (0xC, 0xD):
            return 2, f"pop {hw:04x}"
        return 2, f"misc{kind:x} {hw:04x}"
    if op == 0xC:
        return 2, f"ldmia/stmia {hw:04x}"
    if op == 0xD:
        cond = (hw >> 8) & 0xF
        if cond == 0xE:
            return 2, f"swi {hw:04x}"
        off = hw & 0xFF
        if off & 0x80:
            off -= 0x100
        names = ['beq', 'bne', 'bcs', 'bcc', 'bmi', 'bpl', 'bvs', 'bvc',
                 'bls', 'bhi', 'bge', 'blt', 'bgt', 'ble', 'bal']
        nm = names[cond] if cond < len(names) else f"b{cond:x}"
        return 2, f"{nm} {a + 4 + off * 2:#x}"
    if op == 0xE and (hw >> 11) == 0b11100:
        off = hw & 0x7FF
        if off & 0x400:
            off -= 0x800
        return 2, f"b {a + 4 + off * 2:#x}"
    if op == 0xF:
        return 4, f".word {hw:04x}"
    return 2, f".hword {hw:04x}"


def main():
    start = int(sys.argv[1], 0)
    end = int(sys.argv[2], 0)
    a = start
    while a < end:
        n, s = dec_one(a)
        print(f"{a:08x}: {s}")
        a += n


if __name__ == "__main__":
    main()
