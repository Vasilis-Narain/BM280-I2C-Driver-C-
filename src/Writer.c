#include <type_alias.h>
#include "Writer.h"

#ifndef WRITER_MAX_BUFFER_SIZE
#define WRITER_MAX_BUFFER_SIZE 1024
#endif

//Error codes
#define WRITER_STRING_TOO_BIG -1
#define WRITER_BUFFER_WAS_FLUSHED 1

static u16 spread16(u8 num);
static u32 spread32(u16 num);
static void copy16(char *out, u16 num);
static void copy32(char *out, u32 num);
static u8 generic_int_to_hex(int_type t, int_union num, char *out);
static u8 get_int_size(int_type t);
static b32 is_signed(int_type t);
static i32 sign_extend(int_type t, int_union num);
static u32 i32_to_string(i32 num, char *buff);
static u32 u32_to_string(u32 num, char *buff);
static b32 _32_to_hex(int_type t, int_union num, char out[HEX_LEN(0, 32)]);
static b32 _16_to_hex(int_type t, int_union num, char out[HEX_LEN(0, 16)]);
static b32 _8_to_hex(int_type t, int_union num, char out[HEX_LEN(0, 8)]);
static i32 print_int_hex(Writer *writer, int_type t, int_union num);
static i32 print_int_dec(Writer *writer, int_type t, int_union num);
static i32 print_int(Writer *writer, int_type t, int_union num, fmt_int fmt);
static int_type get_type(signedness sign, size s);
static char next(const char **fmt, const char *end);

extern __attribute__((noreturn)) void _DEFAULT_Handler();
#define ERROR_HANDLER() _DEFAULT_Handler();

void flush(Writer *writer) {
    if (writer && writer->_flush) {
        writer->_flush(writer);
    }
}

i32 writer_write(Writer *writer, const char *str, u32 length) {
    i32 ret = 0;

    if (length > WRITER_MAX_BUFFER_SIZE) {
        ret = WRITER_STRING_TOO_BIG;
        return ret;
    }

    u32 current_index = writer->current_size;
    if (current_index + length > WRITER_MAX_BUFFER_SIZE) {
        flush(writer);
        current_index = writer->current_size;
    }

    for (u32 i = 0; i < length; i++) {
        writer->buf[current_index++] = str[i];
        ret++;
    }
    writer->current_size = current_index;

    return ret;
}

i32 writer_print(Writer *writer, const char *fmt, u32 length, ...) {
    if (length >= WRITER_MAX_BUFFER_SIZE) {
        return WRITER_STRING_TOO_BIG;
    }

    const char *end = fmt + length;
    va_list args;
    i32 bytes_printed = 0;

    va_start(args, length);

    while (fmt < end) {
        char c = *fmt;

        if (c == '{') {
            c = next(&fmt, end);

            if (c == 's') {
                if (next(&fmt, end) != '}') {
                    ERROR_HANDLER();
                }

                char *str = va_arg(args, char *);
                i32 bytes = writer_write(writer, str, sizeof(str));
                if (bytes < 0) {
                    ERROR_HANDLER();
                }
                bytes_printed += bytes;

            } else {
                fmt_int fmt_type;
                if (c == 'x') {
                    fmt_type = FMT_HEX;

                } else if (c == 'd') {
                    fmt_type = FMT_DEC;
                } else {
                    ERROR_HANDLER();
                }

                if (next(&fmt, end) != ':') {
                    ERROR_HANDLER();
                }

                signedness sign;
                c = next(&fmt, end);
                if (c == 'u') {
                    sign = UNSIGN;
                } else if (c == 'i') {
                    sign = SIGN;
                } else {
                    ERROR_HANDLER();
                }

                size width;
                c = next(&fmt, end);
                if (c == 'b') {
                    width = byte;
                } else if (c == 's') {
                    width = bytebyte;
                } else if (c == 'w') {
                    width = word;
                } else {
                    ERROR_HANDLER();
                }

                if (next(&fmt, end) != '}') {
                    ERROR_HANDLER();
                }

                u32 num = va_arg(args, u32);
                i32 bytes = print_int(writer, get_type(sign, width), (int_union)num, fmt_type);
                if (bytes < 0) {
                    ERROR_HANDLER();
                }
                bytes_printed += bytes;
            }
        } else {
            writer_write_char(writer, c);
            bytes_printed++;
        }
        fmt++;
    }
    va_end(args);
    return bytes_printed;
}

static char next(const char **fmt, const char *end) {
    (*fmt)++;
    return (*fmt < end) ? **fmt : '\0';
}

static i32 print_int(Writer *writer, int_type t, int_union num, fmt_int fmt) {
    switch (fmt) {
    case FMT_HEX:
        return print_int_hex(writer, t, num);
    case FMT_DEC:
        return print_int_dec(writer, t, num);
    }
    return 0;
}

static i32 print_int_dec(Writer *writer, int_type t, int_union num) {
    char buff[25];
    u32 size = 0;
    if (is_signed(t)) {
        i32 sign_extended = sign_extend(t, num);
        size = i32_to_string(sign_extended, buff);
    } else {
        u32 value = 0;
        switch (t) {
        case uint8:
            value = (u32)num.uint8;
            break;
        case uint16:
            value = (u32)num.uint16;
            break;
        case uint32:
            value = num.uint32;
            break;
        default:
            break;
        }
        size = u32_to_string(value, buff);
    }
    return writer_write(writer, (const char *)buff, size);
}

static i32 print_int_hex(Writer *writer, int_type t, int_union num) {
    char buff[12];
    u8 size = generic_int_to_hex(t, num, buff);
    return writer_write(writer, (const char *)buff, size);
}

static int_type get_type(signedness sign, size s) {
    i32 offset = 0;
    if (sign == SIGN) {
        offset = 3;
    }

    return offset + s;
}

void writer_write_char(Writer *writer, char c) {
    if (writer->current_size + 1 >= WRITER_MAX_BUFFER_SIZE) {
        flush(writer);
    }
    writer->buf[writer->current_size] = c;
    writer->current_size++;
}

static const char char_table[201] = {
    "00010203040506070809"
    "10111213141516171819"
    "20212223242526272829"
    "30313233343536373839"
    "40414243444546474849"
    "50515253545556575859"
    "60616263646566676869"
    "70717273747576777879"
    "80818283848586878889"
    "90919293949596979899",
};

static const u32 pow_of_10_table[10] = {
    1u,
    10u,
    100u,
    1000u,
    10000u,
    100000u,
    1000000u,
    10000000u,
    100000000u,
    1000000000u,
};

static u32 count_digits_u32(u32 num) {
    u32 result = 0;

    while (num >= pow_of_10_table[result]) {
        result++;
    }

    return result;
}

static u32 u32_to_string(u32 num, char *buff) {
    if (num == 0) {
        buff[0] = '0';
        return 1;
    }

    u32 initial_length = count_digits_u32(num);
    u32 length = initial_length;

    u32 i = 0;
    while (length >= 2) {
        length -= 2;
        u32 digits = num / pow_of_10_table[length];
        u32 index = digits * 2;
        buff[i] = char_table[index];
        buff[i + 1] = char_table[index + 1];
        num -= digits * pow_of_10_table[length];
        i += 2;
    }
    if (length == 1) {
        buff[i] = '0' + num;
    }

    return initial_length;
}

static u32 i32_to_string(i32 num, char *buff) {
    u32 size = 0;
    u32 mag;

    if (num < 0) {
        *buff++ = '-';
        size++;
        mag = 0u - (u32)num;
    } else {
        mag = (u32)num;
    }
    size += u32_to_string(mag, buff);
    return size;
}

static i32 sign_extend(int_type t, int_union num) {
    i32 x = num.int32;
    i32 shift_amount = 0;
    switch (t) {
    case int8:
        shift_amount = 32 - 8;
        break;
    case int16:
        shift_amount = 32 - 16;
        break;
    case int32:
        break;
    default:
        break;
    }
    x = x << shift_amount;
    x = x >> shift_amount;
    return x;
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

// Following functions are to convert a uint*_t to a hex string.
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
