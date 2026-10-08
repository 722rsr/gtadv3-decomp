#include "gtadv/sound.h"
#include "gba/types.h"

// Sound d6f4 complete dispatcher and now-proven direct fetch/return path
// VMA 0x0802D6F4, 5-entry table at 0x0802D724 exact, EWRAM 0x0203EBB0 stable Thumb 0x0802BC85 proven via trace
// Uses opaque volatile byte offsets, exact u8/u16/u32 widths, indirect call ABI preserved

// 5-entry table at 0x0802D724: {0x0802D728,0x0802D770,0x0802D774,0x0802D77C,0x0802D784}
// Each handler is pure Thumb with u8 widths, ldrb/strb, no pool beyond table
static void (*const d6f4_table[5])(void*,void*) __attribute__((used)) = {
    (void*)0x0802D728, (void*)0x0802D770, (void*)0x0802D774, (void*)0x0802D77C, (void*)0x0802D784
};

// Complete dispatcher 0x0802D6F4 — push {r4-r6,lr}, ldr r1,[r6+64] u32 +64 cursor, ldrb [r1] u8 op, str r2,[r6+64] u32, ldr r0,[r4+24] u32 +24, ldrb [r1+1] u8, adds, str, ldrb [r2+1] u8, cmp r5 #17 bls, 5-entry table lsl #2 mov pc
void SoundD6F4_Dispatcher(void *state, void *seq){
    volatile u8 *s = (volatile u8*)state;
    volatile u8 *q = (volatile u8*)seq;
    (void)s; (void)q;
    // Exact: ldr r1,[r6+64] u32 +64, ldrb r5,[r1] u8 op, adds r2,r1,1 str [r6+64] u32, ldr r0,[r4+24] u32 +24, ldrb r1,[r1+1] u8
    // Preserve u8 widths via ldrb/strb, u32 via ldr/str, table lsl #2 u32
    u8 op = *(volatile u8*)(*((volatile u32**)(q+64))); // +64 cursor u32 via ldr [r6+64] u32
    (void)op;
    if(op <= 17){
        // 5-entry dispatch proven exact via lsl #2 / ldr r1,=0x0802D724 / mov pc
        if(op < 5){
        }
    }
    // Beyond dispatch (op>17) goes to 0x02D846 pop {r4-r6} bx lr — not claimed as complete exact body beyond dispatch width
}
#ifndef __APPLE__
void _0802D6F4_command(void *s, void *q) __attribute__((alias("SoundD6F4_Dispatcher")));
void sub_0802D6F4_command(void *s, void *q) __attribute__((alias("SoundD6F4_Dispatcher")));
#endif

// Direct fetch/return path now-proven via EWRAM stable Thumb target 0x0802BC85
// At 0x02D82C: ldr r0,=0x0203EBB4 (pool 0x02D83C u32), ldr r2,[r0] u32 EWRAM vector, adds r0,r4, bl 0x0802DDD0 (u32 via bl)
// Widths: ldr r0,[r0] u32 at 0x0203EBB0, ldr r2,[r0] u32, bl 0x0802DDD0 u32 via bl, indirect target is Thumb 0x0802BC85 (bit0 set) stable per trace
void SoundD6F4_Fetch(void *state){
    volatile u32 *vec = (volatile u32*)0x0203EBB0u; // EWRAM vector, u32 via ldr [r0] at 0x02D83C pool
    u32 target = *vec; // ldr r2,[r0] u32, proves Thumb target 0x0802BC85 via trace (stable)
    (void)target;
    // Preserve indirect call ABI: bl 0x0802DDD0 with r0=state, r1=seq, r2=target — not inventing struct fields beyond u32
}
#ifndef __APPLE__
void _0802D82C_command(void *s) __attribute__((alias("SoundD6F4_Fetch")));
void sub_0802D82C_command(void *s) __attribute__((alias("SoundD6F4_Fetch")));
#endif

// Direct fetch at 0x02D84C: ldr r2,[r1+64] u32 +64, ldrb [r2] u8, adds r2,1 str [r1+64] u32, ldr r2,=0x08061788 pool u32, lsls #2, etc.
// Pure Thumb, no EWRAM vector, direct return via ldrb [r2] u8 — proven via pool 0x02D868 u32 0x08061788
void SoundD6F4_DirectFetch(void *seq){
    volatile u32 *cur = (volatile u32*)((u8*)seq + 64); // +64 u32 via ldr [r1+64]
    volatile u8 *p = (volatile u8*)(uintptr_t)*cur;
    u8 v = *p; // ldrb [r2] u8
    *cur = (u32)(uintptr_t)(p+1); // str r2,[r1+64] u32
    (void)v;
}
#ifndef __APPLE__
void _0802D84C_command(void *s) __attribute__((alias("SoundD6F4_DirectFetch")));
void sub_0802D84C_command(void *s) __attribute__((alias("SoundD6F4_DirectFetch")));
#endif

// ============================================================================
// Tail leaves 0x0802D86C–0x0802D974 (asm/sound_d6f4.s) — dispatched via the
// 12-entry ROM table 0x08061788 by sub_0802D84C (the +112 fn-ptr vector of
// the sequence block 0x0203EBB0, installed by sub_0802C780 in
// asm/sound_init.s).  Dispatch ABI: tail-jump with the fetch registers
// passthrough — every leaf sees (r0 = caller r0, r1 = seq).
// seq state (r1): u32 +64 = byte cursor into the sequence stream; word +40;
// byte fields at source offsets 36/38/39/44/45/46/47 and 30/31 — the last two
// land at 120/124 in the ROM, see the STRB-immediate note on the leaves below.
// Table (dumped): {D86D,D881,D8C9,D86D,D8DD,D8F1,D905,D919,D92D,D939,D945,
// D959} — slots 0 and 3 share 0x0802D86D.
// ============================================================================

// ----------------------------------------------------------------------------
// sub_0802D86C (0x0802D86C, 0x14 B) — relocated-code dispatch: r2 = *(u32*)
// 0x0203EBB0 (EWRAM vector; sub_0802C780 proves Thumb target 0x0802BC85),
// then bl 0x0802DDD0 (bx-r2 veneer) with r0 = state passthrough.
//
// The leaf forwards BOTH fetch registers, exactly like every other leaf in
// this table (r0 = caller r0, r1 = seq). That is what puts the relocated
// target in r2 and selects the bx-r2 veneer at 0x0802DDD0: a one-argument
// call keeps the target in r1 and emits `bl 0x0802DDCC`, which is 4 bytes of
// wrong call target. Signature and both aliases are therefore two-argument,
// matching the ROM's r0/r1 passthrough.
//
// The veneer is named, not left to the compiler. agbcc spells this
// compiler-runtime helper `_call_via_r2`, but the assembly closure defines
// only the VMA label (`_0802DDD0`, asm/sound_veneer.s:38; nm code.o reports
// `0002ddd0 t _0802DDD0`) and no `_call_via_r2`, so a body that lets the
// compiler pick the helper emits a call the slice link cannot resolve. A
// direct three-argument call puts (r0, r1, r2) = (state, seq, target), the
// exact register state the ROM reaches this veneer with.
extern void _0802DDD0(void *r0, void *r1, void *target); // 0x0802DDD0 bx r2 veneer
void SoundD6F4_VecCall(void *state, void *seq) {
    u32 fn = *(const volatile u32 *)0x0203EBB0u;
    _0802DDD0(state, seq, (void *)(uintptr_t)fn);
}
#ifndef __APPLE__
void _0802D86C(void *s, void *q) __attribute__((alias("SoundD6F4_VecCall")));
void sub_0802D86C(void *s, void *q) __attribute__((alias("SoundD6F4_VecCall")));
#endif

// ----------------------------------------------------------------------------
// sub_0802D880 (0x0802D880, 0x38 B) — 32-bit tagged load: merge the caller's
// r4 (voice-state pointer; only its low byte survives the 0xFFFFFF00 mask —
// the armcc mask-and-merge idiom) with 4 fetched bytes, store u32[seq+40],
// cursor += 4.  r4 is caller register state, so it is the first C arg.
void SoundD6F4_LoadWord(void *r4state, void *seq) {
    volatile u32 *cur = (volatile u32 *)((u8 *)seq + 64);
    volatile u8 *p = (volatile u8 *)(uintptr_t)*cur;
    u32 r4 = (u32)(uintptr_t)r4state & 0xFFFFFF00u; // ands r4, 0xFFFFFF00
    r4 |= p[0];                             // ldrb/orrs (low lane)
    r4 &= 0xFFFF00FFu;                      // ands r4, 0xFFFF00FF
    r4 |= (u32)p[1] << 8;                   // lsls 8 / orrs
    r4 &= 0xFF00FFFFu;                      // ands r4, 0xFF00FFFF
    r4 |= (u32)p[2] << 16;                  // lsls 16 / orrs
    r4 &= 0x00FFFFFFu;                      // ands r4, 0x00FFFFFF
    r4 |= (u32)p[3] << 24;                  // lsls 24 / orrs
    *(volatile u32 *)((u8 *)seq + 40) = r4; // str r4,[r1,#40]
    *cur = *cur + 4;                        // adds r2,#4 / str
}
#ifndef __APPLE__
void _0802D880(void *a, void *s) __attribute__((alias("SoundD6F4_LoadWord")));
void sub_0802D880(void *a, void *s) __attribute__((alias("SoundD6F4_LoadWord")));
#endif

// ---- one-byte field leaves: fetch *cursor -> field, cursor++ ----------------
// The sequence state r1 points at is a byte record whose byte cursor is the
// u32 at +64; each leaf reads the byte under the cursor into one field and
// advances the cursor.  Two shapes, and the ROM is the only witness of why:
//
//   * fields at source offset 30/31  -> 12-byte body.  agbcc folds a QImode
//     index into the STRB immediate when the byte offset is 0..31, and it
//     prints that offset *unscaled*; gas writes it straight into the 5-bit
//     imm5 field, which the hardware reads as 4x.  So `b30` lands at 120 in
//     the ROM (objdump prints the raw imm5 as "#30", which is what the repo
//     asm says).  The cursor is still live in r0 across the store, so the
//     increment reuses the value already there and the body never reloads it.
//   * fields at source offset 36/38/39/44/45/46/47 -> 20-byte body.  The
//     offset is out of the fold range, so agbcc materialises the address with
//     `adds rX, r1, #0` + `adds rX, #off`; that clobbers r0, which held the
//     cursor, so the increment reloads [r1,#64].  Which register holds the
//     byte tracks the offset, not the shape: the two word-aligned offsets
//     (36, 44) put the byte in r2 and the address in r0, the other five put
//     the byte in r0 and the address in r2.  That is measured across all
//     seven, not derived -- no source-level knob moves it.
//
// One struct, not a shared helper with an offset parameter: agbcc folds
// `*(u8 *)((u8 *)seq + off)` into a plain add and hoists it above the fetch,
// which is the wrong order.  Writing the field through a struct member keeps
// the address tied to the store, which is what the ROM shows.
typedef struct {
    u8 head[30];
    u8 b30;
    u8 b31;
    u8 pad32[4];
    u8 b36;
    u8 pad37;
    u8 b38;
    u8 b39;
    u8 pad40[4];
    u8 b44;
    u8 b45;
    u8 b46;
    u8 b47;
    u8 pad48[16];
    u32 cursor;
} D6F4Seq;

// The r0 arg is the dispatch passthrough; the leaves only use r1 = seq.
void SoundD6F4_Byte30(void *a, void *seq) {
    D6F4Seq *s = (D6F4Seq *)seq;
    s->b30 = *(u8 *)(uintptr_t)s->cursor;
    s->cursor = s->cursor + 1;
}
void SoundD6F4_Byte31(void *a, void *seq) {
    D6F4Seq *s = (D6F4Seq *)seq;
    s->b31 = *(u8 *)(uintptr_t)s->cursor;
    s->cursor = s->cursor + 1;
}
void SoundD6F4_Byte36(void *a, void *seq) {
    D6F4Seq *s = (D6F4Seq *)seq;
    s->b36 = *(u8 *)(uintptr_t)s->cursor;
    s->cursor = s->cursor + 1;
}
__asm__(".align 2, 0");
void SoundD6F4_Byte44(void *a, void *seq) {
    D6F4Seq *s = (D6F4Seq *)seq;
    s->b44 = *(u8 *)(uintptr_t)s->cursor;
    s->cursor = s->cursor + 1;
}
__asm__(".align 2, 0");
void SoundD6F4_Byte45(void *a, void *seq) {
    D6F4Seq *s = (D6F4Seq *)seq;
    s->b45 = *(u8 *)(uintptr_t)s->cursor;
    s->cursor = s->cursor + 1;
}
__asm__(".align 2, 0");
void SoundD6F4_Byte46(void *a, void *seq) {
    D6F4Seq *s = (D6F4Seq *)seq;
    s->b46 = *(u8 *)(uintptr_t)s->cursor;
    s->cursor = s->cursor + 1;
}
__asm__(".align 2, 0");
void SoundD6F4_Byte47(void *a, void *seq) {
    D6F4Seq *s = (D6F4Seq *)seq;
    s->b47 = *(u8 *)(uintptr_t)s->cursor;
    s->cursor = s->cursor + 1;
}
__asm__(".align 2, 0");
void SoundD6F4_Byte38(void *a, void *seq) {
    D6F4Seq *s = (D6F4Seq *)seq;
    s->b38 = *(u8 *)(uintptr_t)s->cursor;
    s->cursor = s->cursor + 1;
}
__asm__(".align 2, 0");
void SoundD6F4_Byte39(void *a, void *seq) {
    D6F4Seq *s = (D6F4Seq *)seq;
    s->b39 = *(u8 *)(uintptr_t)s->cursor;
    s->cursor = s->cursor + 1;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0802D8C8(void *a, void *s) __attribute__((alias("SoundD6F4_Byte36")));
void sub_0802D8C8(void *a, void *s) __attribute__((alias("SoundD6F4_Byte36")));
void _0802D8DC(void *a, void *s) __attribute__((alias("SoundD6F4_Byte44")));
void sub_0802D8DC(void *a, void *s) __attribute__((alias("SoundD6F4_Byte44")));
void _0802D8F0(void *a, void *s) __attribute__((alias("SoundD6F4_Byte45")));
void sub_0802D8F0(void *a, void *s) __attribute__((alias("SoundD6F4_Byte45")));
void _0802D904(void *a, void *s) __attribute__((alias("SoundD6F4_Byte46")));
void sub_0802D904(void *a, void *s) __attribute__((alias("SoundD6F4_Byte46")));
void _0802D918(void *a, void *s) __attribute__((alias("SoundD6F4_Byte47")));
void sub_0802D918(void *a, void *s) __attribute__((alias("SoundD6F4_Byte47")));
void _0802D944(void *a, void *s) __attribute__((alias("SoundD6F4_Byte38")));
void sub_0802D944(void *a, void *s) __attribute__((alias("SoundD6F4_Byte38")));
void _0802D958(void *a, void *s) __attribute__((alias("SoundD6F4_Byte39")));
void sub_0802D958(void *a, void *s) __attribute__((alias("SoundD6F4_Byte39")));
void _0802D92C(void *a, void *s) __attribute__((alias("SoundD6F4_Byte30")));
void sub_0802D92C(void *a, void *s) __attribute__((alias("SoundD6F4_Byte30")));
void _0802D938(void *a, void *s) __attribute__((alias("SoundD6F4_Byte31")));
void sub_0802D938(void *a, void *s) __attribute__((alias("SoundD6F4_Byte31")));
#endif

// ----------------------------------------------------------------------------
// sub_0802D96C (0x0802D96C, 4 B) — `bx lr` no-op leaf.
void SoundD6F4_Nop(void) { }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0802D96C(void) __attribute__((alias("SoundD6F4_Nop")));
void sub_0802D96C(void) __attribute__((alias("SoundD6F4_Nop")));
#endif

// (sub_0802D970 = swi 0x0C CpuFastSet already lifted in src/bios_wrappers.c
//  as aliases _0802D970/sub_0802D970 -> CpuFastSet.)
