#pragma once
#include <type_alias.h>

#ifndef RTT_WRITE_CHANNEL
#define RTT_WRITE_CHANNEL 0
#endif
#ifndef RTT_READ_CHANNEL
#define RTT_READ_CHANNEL 0
#endif

#define RTT_HEX_U32_LEN 10 // "0x" + 8 digits
#define RTT_HEX_U16_LEN 6  // "0x" + 4 digits
#define RTT_HEX_U8_LEN 4   // "0x" + 2 digits
#define RTT_HEX_I32_LEN 11 // "-0x" + 8 digits
#define RTT_HEX_I16_LEN 7  // "-0x" + 4 digits
#define RTT_HEX_I8_LEN 5   // "-0x" + 2 digits

#define GET_SIGN_0 U
#define GET_SIGN_1 I

#define GET_SIGN_8 U  // u8
#define GET_SIGN_9 I  // i8
#define GET_SIGN_16 U // u16
#define GET_SIGN_17 I // i16
#define GET_SIGN_32 U // u32
#define GET_SIGN_33 I // i32

#define CONCAT_4_HIDDEN(a, b, c, d) a##b##c##d
#define CONCAT_4(a, b, c, d) CONCAT_4_HIDDEN(a, b, c, d)

#define HEX_LEN(is_signed, size) CONCAT_4(RTT_HEX_, GET_SIGN_##is_signed, size, _LEN)

#ifndef RTT_MAX_NUM_UP_BUFFERS
#define RTT_MAX_NUM_UP_BUFFERS (1)
#endif

#ifndef RTT_MAX_NUM_DOWN_BUFFERS
#define RTT_MAX_NUM_DOWN_BUFFERS (1)
#endif

#ifndef RTT_BUFFER_SIZE_UP
#define RTT_BUFFER_SIZE_UP (1024)
#endif

#ifndef RTT_BUFFER_SIZE_DOWN
#define RTT_BUFFER_SIZE_DOWN (32)
#endif

#define _CORE_HAS_RTT_ASM_SUPPORT (1)
#define _CORE_NEEDS_DMB (1)
#define RTT_DMB() __asm__ volatile("dmb sy\n" : : :)

#ifndef RTT_CPU_CACHE_LINE_SIZE
#define RTT_CPU_CACHE_LINE_SIZE (0)
#endif

#if RTT_CPU_CACHE_LINE_SIZE
#define RTT_ROUND_UP_2_CACHE_LINE_SIZE(NumBytes) (((NumBytes + RTTRTT_CPU_CACHE_LINE_SIZE - 1) / RTTRTT_CPU_CACHE_LINE_SIZE) * RTT_CPU_CACHE_LINE_SIZE
#else
#define RTT_ROUND_UP_2_CACHE_LINE_SIZE(NumBytes) (NumBytes)
#endif

#define RTT_CB_SIZE (16 + 4 + 4 + (RTT_MAX_NUM_UP_BUFFERS * 24) + (RTT_MAX_NUM_DOWN_BUFFERS * 24))
#define RTT_CB_PADDING (RTT_ROUND_UP_2_CACHE_LINE_SIZE(RTT_CB_SIZE) - RTT_CB_SIZE)

// Up ring buffer (T -> H)
typedef struct {
    const char *sName;
    char *pBuffer;
    u32 SizeOfBuffer;
    u32 WrOff;
    volatile u32 RdOff;
    u32 Flags;
} rtt_buffer_up_t;

// Down ring buffer (H -> T)
typedef struct {
    const char *sName;
    char *pBuffer;
    u32 SizeOfBuffer;
    volatile u32 WrOff;
    u32 RdOff;
    u32 Flags;
} rtt_buffer_down_t;

// RTT control block
typedef struct {
    char acID[16]; // Initialized to "SEGGER RTT"
    i32 MaxNumUpBufers;
    i32 MaxNumDownBufers;
    rtt_buffer_up_t aUp[RTT_MAX_NUM_UP_BUFFERS];
    rtt_buffer_down_t aDown[RTT_MAX_NUM_DOWN_BUFFERS];
#if RTT_CB_PADDING
    u8 aDummy[RTT_CB_PADDING];
#endif
} rtt_ctrl_block_t;

// Global data
extern rtt_ctrl_block_t _SEGGER_RTT;

b32 rtt_write(const char *str, u32 len, u8 channel);
u32 rtt_read(char *buf, u32 max, u8 channel);
b32 rtt_print_hex_u32(u32 num, u8 channel);
b32 rtt_print_hex_u16(u16 num, u8 channel);
b32 rtt_print_hex_u8(u8 num, u8 channel);
void u32_to_hex(u32 num, char out[HEX_LEN(0, 32)]);
void u16_to_hex(u16 num, char out[HEX_LEN(0, 16)]);
void u8_to_hex(u8 num, char out[HEX_LEN(0, 8)]);

#define rtt_err(s) rtt_write("ERR::" s, sizeof("ERR::" s) - 1, RTT_WRITE_CHANNEL)

typedef enum {
    uint8,
    uint16,
    uint32,

    int8,
    int16,
    int32,
} int_type;

typedef union {
    u8 uint8;
    u16 uint16;
    u32 uint32;
    i8 int8;
    i16 int16;
    i32 int32;
} int_union;

typedef struct {
    char buf[RTT_BUFFER_SIZE_UP];
    u32 current_size;
} rtt_writer;

i32 rtt_print_hex(rtt_writer *writer, int_type t, int_union num);
i32 rtt_print(rtt_writer *writer, const char *str, u32 length);
void rtt_flush(rtt_writer *writer);
#define rtt_writeAll(writer, s) rtt_print(writer, "" s, sizeof("" s) - 1)
