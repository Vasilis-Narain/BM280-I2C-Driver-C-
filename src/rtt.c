#include <type_alias.h>
#include "rtt.h"

static u16 spread16(u8 num);
static u32 spread32(u16 num);
static void copy16(char *out, u16 num);
static void copy32(char *out, u32 num);
static u8 generic_int_to_hex(int_type t, int_union num, char *out);
static u8 get_int_size(int_type t);
static b32 is_signed(int_type t);

char rtt_buffer_up[RTT_BUFFER_SIZE_UP];
char rtt_buffer_down[RTT_BUFFER_SIZE_DOWN];

// Global data
rtt_ctrl_block_t __attribute__((used, section(".rtt_cb"))) _SEGGER_RTT = {
    .acID = "SEGGER RTT\0\0\0\0\0\0",
    .MaxNumUpBufers = RTT_MAX_NUM_UP_BUFFERS,
    .MaxNumDownBufers = RTT_MAX_NUM_DOWN_BUFFERS,
    .aUp = {
        {
            .sName = "Terminal",
            .pBuffer = rtt_buffer_up,
            .SizeOfBuffer = RTT_BUFFER_SIZE_UP,
            .WrOff = 0,
            .RdOff = 0,
            .Flags = 0,
        },
    },
    .aDown = {
        {
            .sName = ">",
            .pBuffer = rtt_buffer_down,
            .SizeOfBuffer = RTT_BUFFER_SIZE_DOWN,
            .WrOff = 0,
            .RdOff = 0,
            .Flags = 0,
        },
    },
};

#define BUFFER_WAS_FLUSHED -1
#define BUFFER_NOT_LARGE_ENOUGH_FOR_STRING -2

void rtt_flush(rtt_writer *writer) {
    rtt_write((const char *)(writer->buf), writer->current_size, RTT_WRITE_CHANNEL);
    writer->current_size = 0;
}

i32 rtt_print(rtt_writer *writer, const char *str, u32 length) {
    i32 ret = 0;

    if (length > RTT_BUFFER_SIZE_UP) {
        ret = BUFFER_NOT_LARGE_ENOUGH_FOR_STRING;
        return ret;
    }

    u32 current_index = writer->current_size;
    if (current_index + length > RTT_BUFFER_SIZE_UP) {
        rtt_flush(writer);
        ret = BUFFER_WAS_FLUSHED;
    }

    for (u32 i = 0; i < length; i++) {
        writer->buf[current_index++] = str[i];
    }
    writer->current_size = current_index;

    return ret;
}

b32 rtt_write(const char *str, u32 len, u8 channel) {
    u32 wr_idx = _SEGGER_RTT.aUp[channel].WrOff;
    u32 rd_idx = (u32)_SEGGER_RTT.aUp[channel].RdOff;
    b32 success = 1;

    for (u32 i = 0; i < len; i++) {
        u32 wr_next = wr_idx + 1;
        if (wr_next >= RTT_BUFFER_SIZE_UP - 1) {
            wr_next = 1;
        }

        if (wr_next == rd_idx) {
            success = 0;
            break;
        }

        rtt_buffer_up[wr_idx] = *str++;
        wr_idx = wr_next;
    }

#if _CORE_NEEDS_DMB
    RTT_DMB();
#endif

    _SEGGER_RTT.aUp[channel].WrOff = wr_idx;
    return success;
}

u32 rtt_read(char *buf, u32 max, u8 channel) {
    u32 wr_idx = (u32)_SEGGER_RTT.aDown[channel].WrOff;
    u32 rd_idx = _SEGGER_RTT.aDown[channel].RdOff;
    char *tmp = buf;
    u32 num_bytes_processed = 0;

    while ((rd_idx != wr_idx) && (num_bytes_processed <= max)) {
        u32 rd_next = rd_idx + 1;

        if (rd_next >= RTT_BUFFER_SIZE_DOWN) {
            rd_next = 0;
        }

        *tmp++ = rtt_buffer_down[rd_idx];
        rd_idx = rd_next;
        num_bytes_processed++;
    }

#if _CORE_NEEDS_DMB
    RTT_DMB();
#endif

    _SEGGER_RTT.aDown[channel].RdOff = rd_idx;
    return num_bytes_processed;
}

i32 rtt_print_hex(rtt_writer *writer, int_type t, int_union num) {
    char buf[32];
    u8 size = generic_int_to_hex(t, num, buf);
    buf[size] = '\n';
    return rtt_print(writer, (const char *)buf, size + 1);
}

static u8 get_int_size(int_type t) {
    u8 res = 0;
    switch (t) {
    case uint8:
        res = HEX_LEN(0, 8);
        break;
    case uint16:
        res = HEX_LEN(0, 16);
        break;
    case uint32:
        res = HEX_LEN(0, 32);
        break;

    case int8:
        res = HEX_LEN(1, 8);
        break;
    case int16:
        res = HEX_LEN(1, 16);
        break;
    case int32:
        res = HEX_LEN(1, 32);
        break;
    }
    return res;
}

static b32 _32_to_hex(int_type t, int_union num, char out[HEX_LEN(0, 32)]) {
    out[0] = '0';
    out[1] = 'x';

    u32 bits;
    if (t == int32) {
        bits = (num.int32 < 0) ? (0u - (u32)num.int32) : (u32)num.int32;

    } else if (t == uint32) {
        bits = num.uint32;
    } else {
        return -1;
    }

    copy32(out + 2, spread32((u16)(bits >> 16)));
    copy32(out + 6, spread32((u16)(bits)));
    return 0;
}

static b32 _16_to_hex(int_type t, int_union num, char out[HEX_LEN(0, 16)]) {
    out[0] = '0';
    out[1] = 'x';

    if (t == int16) {
        u16 mag = (num.int16 < 0) ? (u16)(0u - (u32)(u16)num.int16) : (u16)num.int16;
        copy32(out + 2, spread32(mag));
    } else if (t == uint16) {
        copy32(out + 2, spread32(num.uint16));
    } else {
        return -1;
    }
    return 0;
}

static b32 _8_to_hex(int_type t, int_union num, char out[HEX_LEN(0, 8)]) {
    out[0] = '0';
    out[1] = 'x';

    if (t == int8) {
        u8 mag = (num.int8 < 0) ? (u8)(0u - (u32)(u8)num.int8) : (u8)num.int8;
        copy16(out + 2, spread16(mag));
    } else if (t == uint8) {
        copy16(out + 2, spread16(num.uint8));
    } else {
        return -1;
    }
    return 0;
}

static u8 generic_int_to_hex(int_type t, int_union num, char *out) {
    char *start = out;

    switch (t) {
    case uint8:
    case uint16:
    case uint32:
        break;

    case int8:
        if (num.int8 < 0) {
            *out++ = '-';
        }
        break;
    case int16:
        if (num.int16 < 0) {
            *out++ = '-';
        }
        break;
    case int32:
        if (num.int32 < 0) {
            *out++ = '-';
        }
        break;
    }

    switch (t) {
    case uint8:
    case int8:
        _8_to_hex(t, num, out);
        break;
    case uint16:
    case int16:
        _16_to_hex(t, num, out);
        break;
    case uint32:
    case int32:
        _32_to_hex(t, num, out);
        break;
    }

    // Unsigned lengths: the sign char, when emitted, is already counted by
    // (out - start), so this yields the exact number of chars written.
    return (u8)((out - start) + get_int_size(t) - (is_signed(t) ? 1 : 0));
}

static b32 is_signed(int_type t) {
    return (t == int8) || (t == int16) || (t == int32);
}

static void copy32(char *out, u32 num) {
    out[0] = (char)(num >> 0);
    out[1] = (char)(num >> 8);
    out[2] = (char)(num >> 16);
    out[3] = (char)(num >> 24);
}

static void copy16(char *out, u16 num) {
    out[0] = (char)(num >> 0);
    out[1] = (char)(num >> 8);
}

// Following functions are to convert a uint*_t to a hex string.
//
// ! Does not add a null terminator !
//
// No efort has been done to make this safe. It's probably not.
//
// Modified from https://johnnylee-sde.github.io/Fast-unsigned-integer-to-hex-string/
//
static u32 spread32(u16 num) {
    u32 x = num;
    x = ((x & 0x00FF) << 16) | ((x & 0xFF00) >> 8);
    x = ((x & 0x00F000F0) >> 4) | ((x & 0x000F000F) << 8);

    u32 m = ((x + 0x06060606) >> 4) & 0x01010101;
    return x + 0x30303030 + m * 39;
}

static u16 spread16(u8 num) {
    u16 x = num;
    x = ((x & 0xF) << 8) | ((x & 0xF0) >> 4);

    u32 m = ((x + 0x0606) >> 4) & 0x0101;
    return x + 0x3030 + m * 39;
}
