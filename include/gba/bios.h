#ifndef GBA_BIOS_H
#define GBA_BIOS_H

#include "types.h"

// SWI wrappers — matching Nintendo SDK signatures.
// Implemented in src/bios.s or via inline swi.

void CpuFastSet(const void *src, void *dst, u32 mode);
void CpuSet(const void *src, void *dst, u32 mode);
void VBlankIntrWait(void);
void SoftReset(u32 flags);
int Div(int num, int den);
unsigned DivMod(unsigned num, unsigned den, unsigned *rem);
int Sqrt(u32 num);
void BitUnPack(const void *src, void *dst, const void *tbl);
void LZ77UnCompWram(const void *src, void *dst);
void LZ77UnCompVram(const void *src, void *dst);
void HuffUnComp(const void *src, void *dst);
void BgAffineSet(const void *src, void *dst, int num);
void ObjAffineSet(const void *src, void *dst, int num, int offset);

#endif // GBA_BIOS_H
