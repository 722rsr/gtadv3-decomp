/*
 * Link shims for mGBA 0.10.5 Python bindings built WITHOUT FFmpeg.
 *
 * Upstream bug: src/gba/cart/ereader.c defines the EReaderScan /
 * EReaderAnchorList / EReaderBlockList APIs only under
 * `#ifdef USE_FFMPEG`, but their declarations in
 * include/mgba/internal/gba/cart/ereader.h are unconditional, so the
 * cffi builder (_builder.py scans headers without feature defines)
 * still emits bindings that reference the symbols -> dlopen fails with
 * "symbol not found in flat namespace".
 *
 * None of this API is reachable during headless emulation (it scans
 * physical e-Reader card bitmaps), so aborting stubs are safe.
 */
#include <stdio.h>
#include <stdlib.h>

#define STUB(name)                                        \
	void name(void);                                  \
	void name(void) {                                 \
		fprintf(stderr,                           \
			"mgba-pylib shim: %s needs an "   \
			"FFmpeg-enabled build\n",         \
			#name);                           \
		abort();                                  \
	}

STUB(EReaderAnchorListAppend)
STUB(EReaderAnchorListClear)
STUB(EReaderAnchorListCopy)
STUB(EReaderAnchorListDeinit)
STUB(EReaderAnchorListEnsureCapacity)
STUB(EReaderAnchorListGetConstPointer)
STUB(EReaderAnchorListGetPointer)
STUB(EReaderAnchorListIndex)
STUB(EReaderAnchorListInit)
STUB(EReaderAnchorListResize)
STUB(EReaderAnchorListShift)
STUB(EReaderAnchorListSize)
STUB(EReaderAnchorListUnshift)
STUB(EReaderBlockListAppend)
STUB(EReaderBlockListClear)
STUB(EReaderBlockListCopy)
STUB(EReaderBlockListDeinit)
STUB(EReaderBlockListEnsureCapacity)
STUB(EReaderBlockListGetConstPointer)
STUB(EReaderBlockListGetPointer)
STUB(EReaderBlockListIndex)
STUB(EReaderBlockListInit)
STUB(EReaderBlockListResize)
STUB(EReaderBlockListShift)
STUB(EReaderBlockListSize)
STUB(EReaderBlockListUnshift)
STUB(EReaderScanCard)
STUB(EReaderScanConnectAnchors)
STUB(EReaderScanCreate)
STUB(EReaderScanCreateBlocks)
STUB(EReaderScanDestroy)
STUB(EReaderScanDetectAnchors)
STUB(EReaderScanDetectBlockThreshold)
STUB(EReaderScanDetectParams)
STUB(EReaderScanFilterAnchors)
STUB(EReaderScanLoadImage)
STUB(EReaderScanLoadImage8)
STUB(EReaderScanLoadImageA)
STUB(EReaderScanLoadImagePNG)
STUB(EReaderScanOutputBitmap)
STUB(EReaderScanRecalibrateBlock)
STUB(EReaderScanSaveRaw)
STUB(EReaderScanScanBlock)
