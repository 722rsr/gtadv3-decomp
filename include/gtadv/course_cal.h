#ifndef GTADV_COURSE_CAL_H
#define GTADV_COURSE_CAL_H
#include "gba/types.h"

// Championship calendar at ROM 0x08060124, 49 rows x 12 bytes.
// Fields: +0 cup, +2 song, +4 course_default, +6 course_variant, +8 seqnum, +10 param
// All s16; stride 12. Original asm does idx*12 via (idx*3)*4.
#define COURSE_CALENDAR_BASE ((const u8 *)0x08060124)
#define COURSE_CALENDAR_ROWS 49
#define COURSE_CALENDAR_STRIDE 12

s16 Course_GetCup(int idx);            // _080258B8 field +0
s16 Course_GetSeqnum(int idx);         // _080258CC field +8
s16 Course_GetSong(int idx);           // _080258E0 field +2
s16 Course_GetCourseDefault(int idx);  // _080258F4 field +4
s16 Course_GetCourseVariant(int idx);  // _08025908 field +6
s16 Course_GetParam(int idx);          // _0802591C field +10

#endif
