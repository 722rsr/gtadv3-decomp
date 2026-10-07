#ifndef GTADV_COURSE_COLLISION_H
#define GTADV_COURSE_COLLISION_H
#include "gba/types.h"

// Collision / proximity helpers (ARMCC high-register Thumb)

// 0x08006E70: candidate segment test, returns bool, writes corrected pos via out ptr
int Course_CollisionTest(void *a, void *b, void *c, void *out, void *extra);
// 0x08006FD4 continuation (4 args: sl,a,b,c as per push {r4-r7,lr} frame), 0x07110 loop (3 args)
int Course_CollisionMore(void *sl, void *a, void *b, void *c);
int Course_CollisionLoop(void *a, void *b, void *c);
// Proximity search 0x08007210-0x08007368
int Course_ProximitySearch(void *query, void *candidate);
// Proximity more 0x07368+ (4 args per high-reg frame)
int Course_ProximityMore(void *a, void *b, void *c, void *d);
// Scan helper 0x08006050
int Course_Scan(void *outPair, void *inPair);
// Stream cursor helpers
void Course_StreamTail(void);

#endif
