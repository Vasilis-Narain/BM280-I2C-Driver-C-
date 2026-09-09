/* Print/Fmt API for strings and integers (signed and unsigned) of sizes upto 32 bits */

#pragma once
#include <type_alias.h>
#include <stdarg.h>

#ifndef WRITER_MAX_BUFFER_SIZE
#define WRITER_MAX_BUFFER_SIZE 256
#endif

typedef struct Writer Writer;

// Flush function must be provided. It depends on target architecture.
// In this case it calls RTT functions (copies bytes to the dedicated memory block)
struct Writer {
    char buf[WRITER_MAX_BUFFER_SIZE];
    u32 current_size;
    void (*_flush)(Writer *);
};

typedef enum {
    UNSIGNED,
    SIGNED,
} signedness;

typedef enum {
    byte,
    half,
    word,
} int_size;

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
