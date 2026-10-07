#include "gba/types.h"
// Freestanding helpers (no hosted lib)
void *memset(void *s, int c, unsigned n){
    u8 *p=(u8*)s;
    while(n--) *p++=(u8)c;
    return s;
}
void *memcpy(void *dst, const void *src, unsigned n){
    u8 *d=(u8*)dst; const u8 *s=(const u8*)src;
    while(n--) *d++=*s++;
    return dst;
}
int memcmp(const void *a,const void *b,unsigned n){
    const u8 *p=(const u8*)a, *q=(const u8*)b;
    while(n--){ if(*p!=*q) return (int)*p-(int)*q; p++; q++; }
    return 0;
}
