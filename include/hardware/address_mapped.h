#pragma once
#include <assert.h>
#include <type_alias.h>
#include <hardware/regs/addressmap.h>
typedef volatile u32 io_rw_32;
typedef const volatile u32 io_ro_32;
typedef volatile u32 io_wo_32;

typedef volatile u16 io_rw_16;
typedef const volatile u16 io_ro_16;
typedef volatile u16 io_wo_16;

typedef volatile u8 io_rw_8;
typedef const volatile u8 io_ro_8;
typedef volatile u8 io_wo_8;
#define _REG_(x)
