#include <type_alias.h>
#include "rtt.h"

static u16 spread16(u8 num);
static u32 spread32(u16 num);
static void copy16(char *out, u16 num);
static void copy32(char *out, u32 num);

char rtt_buffer_up[RTT_BUFFER_SIZE_UP];
char rtt_buffer_down[RTT_BUFFER_SIZE_DOWN];

// Global data
rtt_ctrl_block_t __attribute__((used, section(".rtt_cb"))) rtt_ctrl_block = {
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

b32 rtt_write(const char *str, u32 len, u8 channel) {
    u32 wr_idx = rtt_ctrl_block.aUp[channel].WrOff;
    u32 rd_idx = (u32)rtt_ctrl_block.aUp[channel].RdOff;
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

    rtt_ctrl_block.aUp[channel].WrOff = wr_idx;
    return success;
}

u32 rtt_read(char *buf, u32 max, u8 channel) {
    u32 wr_idx = (u32)rtt_ctrl_block.aDown[channel].WrOff;
    u32 rd_idx = rtt_ctrl_block.aDown[channel].RdOff;
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

    rtt_ctrl_block.aDown[channel].RdOff = rd_idx;
    return num_bytes_processed;
}

b32 rtt_print_hex_u32(u32 num, u8 channel) {
    char buf[RTT_HEX32_LEN + 1];
    u32_to_hex(num, buf);
    buf[RTT_HEX32_LEN] = '\n';
    b32 res = rtt_write(buf, sizeof(buf), channel);
    return res;
}

b32 rtt_print_hex_u16(u16 num, u8 channel) {
    char buf[RTT_HEX16_LEN + 1];
    u16_to_hex(num, buf);
    buf[RTT_HEX16_LEN] = '\n';
    return rtt_write(buf, sizeof(buf), channel);
}

b32 rtt_print_hex_u8(u8 num, u8 channel) {
    char buf[RTT_HEX8_LEN + 1];
    u8_to_hex(num, buf);
    buf[RTT_HEX8_LEN] = '\n';
    return rtt_write(buf, sizeof(buf), channel);
}

// Following functions are to convert a uint*_t to a hex string.
//
// ! Does not add a null terminator !
//
// No efort has been done to make this safe. It's probably not.
//
// Modified from https://johnnylee-sde.github.io/Fast-unsigned-integer-to-hex-string/
//
void u32_to_hex(u32 num, char out[RTT_HEX32_LEN]) {
    out[0] = '0';
    out[1] = 'x';
    copy32(out + 2, spread32((u16)(num >> 16)));
    copy32(out + 6, spread32((u16)(num)));
}

void u16_to_hex(u16 num, char out[RTT_HEX16_LEN]) {
    out[0] = '0';
    out[1] = 'x';
    copy32(out + 2, spread32(num));
}

void u8_to_hex(u8 num, char out[RTT_HEX8_LEN]) {
    out[0] = '0';
    out[1] = 'x';
    copy16(out + 2, spread16(num));
}

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
