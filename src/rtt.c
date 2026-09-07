#include <type_alias.h>
#include "rtt.h"

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

void rtt_flush(Writer *writer) {
    rtt_write((const char *)(writer->buf), writer->current_size, RTT_WRITE_CHANNEL);
    writer->current_size = 0;
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

void writer_error(const char *str, u32 len) {
    rtt_write(str, len, RTT_WRITE_CHANNEL);
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
