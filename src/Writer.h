/* Print/Fmt API for strings and integers (signed and unsigned) of sizes upto 32 bits */

#pragma once
#include <type_alias.h>
#include <stdarg.h>

#ifndef WRITER_MAX_BUFFER_SIZE
#define WRITER_MAX_BUFFER_SIZE 256
#endif

#define HEX_U32_LEN 10 // "0x" + 8 digits
#define HEX_U16_LEN 6  // "0x" + 4 digits
#define HEX_U8_LEN 4   // "0x" + 2 digits
#define HEX_I32_LEN 11 // "-0x" + 8 digits
#define HEX_I16_LEN 7  // "-0x" + 4 digits
#define HEX_I8_LEN 5   // "-0x" + 2 digits

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

#define HEX_LEN(is_signed, size) CONCAT_4(HEX_, GET_SIGN_##is_signed, size, _LEN)

typedef struct Writer Writer;

// Flush function must be provided. It depends on target architecture.
// In this case it calls RTT functions (copies bytes to the dedicated memory block)
struct Writer {
    char buf[WRITER_MAX_BUFFER_SIZE];
    u32 current_size;
    void (*_flush)(Writer *);
};

typedef enum {
    uint8 = 0,
    uint16 = 1,
    uint32 = 2,

    int8 = 3,
    int16 = 4,
    int32 = 5,
} int_type;

typedef enum {
    UNSIGNED,
    SIGNED,
} signedness;

typedef enum {
    byte,
    bytebyte,
    word,
} size;

typedef union {
    u8 uint8;
    u16 uint16;
    u32 uint32;
    i8 int8;
    i16 int16;
    i32 int32;
} int_union;

typedef enum {
    FMT_HEX,
    FMT_DEC,
} fmt_int;

// provide an optional writer_error printing function that bypasses the
// Writer struct. The Writer struct needs to be flushed to actually
// print (to reduce io calls), but logging/errors usually want
// on-time printing.
extern void writer_error(const char *str, u32 length);
#define ERROR(s) writer_error("ERR::" s, sizeof("ERR::" s) - 1)

// Always remember to flush!
void flush(Writer *writer);

i32 writer_write(Writer *writer, const char *str, u32 length);
void writer_write_char(Writer *writer, char c);

// Use this when length != fmt string length
i32 writer_print(Writer *writer, const char *fmt, u32 length, ...);

// Use these for string literals
#define print(writer, s, ...) writer_print(writer, "" s, sizeof(s) - 1, ##__VA_ARGS__)
#define write_all(writer, s) writer_write(writer, "" s, sizeof(s) - 1)
