#ifndef GTADV_COURSE_STREAM_H
#define GTADV_COURSE_STREAM_H
#include "gba/types.h"

// EWRAM session block anchors (tools/track_dump.py)
#define EWRAM_CAM_X_ADDR      0x0203F6B0
#define EWRAM_CAM_Y_ADDR      0x0203F6B4
#define EWRAM_COL_SELECT_ADDR 0x0203F728
#define EWRAM_ROOT_PTR_ADDR   0x0203F760
#define EWRAM_DEDUP_CACHE     0x0203F770
#define EWRAM_COLLECT_CNT     0x0203F870
#define EWRAM_STREAM_CURSOR   0x0203F8A4
#define EWRAM_TILE_DX         0x0203F8B0
#define EWRAM_TILE_DY         0x0203F8B4
#define EWRAM_REC_X_ARRAY     0x0203F8C0
#define EWRAM_REC_Y_ARRAY     0x0203F8C4
#define EWRAM_CEIL_X          0x0203F8E0
#define EWRAM_CEIL_Y          0x0203F8E4
#define EWRAM_SURFACE_MAP     0x02000000
#define VRAM_TILE_BASE        0x06004000
#define SURFACE_HEADER_W_OFF  8
#define SURFACE_HEADER_H_OFF  10

void Course_StreamDispatcher(void); // _08006650
void Course_StreamRow(void);        // _08006678
void Course_CollectProximity(void); // _080068D4
// Tail helpers in same pipeline
void Course_StreamInit(void);       // _08006618 (64x {0x80,0x80} + clear counters)
int  Course_ScanHelper(void *outPair, void *inPair); // _08006050 returns 12 or 0
void Course_CursorReset(void);      // _08006B20
// Proximity iterator exposed by course_stream.s
int Course_CollectNearby(int camX, int camY); // _08006A50 wrapper
void *Course_NextRecord(void);      // _08006AEC returns ptr or 0 and bumps cursor

#endif
