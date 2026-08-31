#pragma once
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64; // The Cortex-M33 FPU is single-precision only, so even f64 is software-emulated.

_Static_assert(sizeof(u8) == 1 && sizeof(u16) == 2 && sizeof(u32) == 4 && sizeof(u64) == 8, "unsigned widths");
_Static_assert(sizeof(i8) == 1 && sizeof(i16) == 2 && sizeof(i32) == 4 && sizeof(i64) == 8, "signed widths");
_Static_assert((i8)-1 < 0 && (i32)-1 < 0, "signed types must be signed");
_Static_assert(sizeof(f32) == 4 && sizeof(f64) == 8, "float widths");
