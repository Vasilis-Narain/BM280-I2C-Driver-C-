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

// Atomic bit manipulation via the RP2350 register aliases: writing to
// base+0x2000/0x3000/0x1000 sets/clears/xors the given bits in one bus
// transaction, with no read-modify-write race against an interrupt.
#define hw_set_alias_untyped(addr)   ((void *)(REG_ALIAS_SET_BITS + (u32)(addr)))
#define hw_clear_alias_untyped(addr) ((void *)(REG_ALIAS_CLR_BITS + (u32)(addr)))
#define hw_xor_alias_untyped(addr)   ((void *)(REG_ALIAS_XOR_BITS + (u32)(addr)))

static inline void hw_set_bits(io_rw_32 *addr, u32 mask) {
    *(io_rw_32 *)hw_set_alias_untyped((volatile void *)addr) = mask;
}
static inline void hw_clear_bits(io_rw_32 *addr, u32 mask) {
    *(io_rw_32 *)hw_clear_alias_untyped((volatile void *)addr) = mask;
}
static inline void hw_xor_bits(io_rw_32 *addr, u32 mask) {
    *(io_rw_32 *)hw_xor_alias_untyped((volatile void *)addr) = mask;
}
