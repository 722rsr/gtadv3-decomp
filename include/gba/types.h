#ifndef GBA_TYPES_H
#define GBA_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;

typedef volatile u8  vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile s8  vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;

#define IWRAM_DATA __attribute__((section(".iwram")))
#define EWRAM_DATA __attribute__((section(".ewram")))
#define IWRAM_CODE __attribute__((section(".iwram"), long_call))
#define EWRAM_CODE __attribute__((section(".ewram"), long_call))

#define ALIGN(n) __attribute__((aligned(n)))
#define PACKED __attribute__((packed))
#define NAKED __attribute__((naked))
#define UNUSED __attribute__((unused))

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#endif // GBA_TYPES_H
