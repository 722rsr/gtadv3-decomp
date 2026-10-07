#ifndef GTADV_CALLEE_H
#define GTADV_CALLEE_H

// Call-site split for a callee that has two spellings.
//
// The idiom was re-derived file-locally in 13 places across 12 files, and each
// re-derivation got a guard wrong at least once: course_orch.c, ai_award_leaves.c,
// garage.c and menus.c all produced a call-site-split failure within one day. A
// duplicated idiom is a defect that recurs. Use this one.
//
//     #include "gtadv/callee.h"
//     #ifndef __APPLE__
//     #define MY_CALLEE(friendly, closure) CALLEE(friendly, closure)
//     extern void sub_08025F78(int id);     // closure spelling, per-body
//     #else
//     #define MY_CALLEE(friendly, closure) CALLEE(friendly, closure)
//     extern void Ai_AwardSetBit(int id);   // friendly name, per-body
//     #endif
//
// The per-file macro stays because the `extern` block between the two branches
// IS the per-file part; only the selection rule is shared.

#ifndef __APPLE__
#define CALLEE(friendly, closure) closure
#else
#define CALLEE(friendly, closure) friendly
#endif

#endif // GTADV_CALLEE_H
