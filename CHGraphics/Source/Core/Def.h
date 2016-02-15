#pragma once
#include <stdint.h>

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef int32_t int32;


#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (uint32)(sizeof(x) / sizeof(x[0]))
#endif

#ifndef NULL
#define NULL 0
#endif

