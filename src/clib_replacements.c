#include <type_alias.h>

void *memcpy(void *dest, const void *src, usize n) {
    u32 i;

    for (i = 0; i < n; i++) {
        ((u8 *)dest)[i] = ((u8 *)src)[i];
    }

    return dest;
}

void *memset(void *blk, i32 c, usize n) {
    u32 i;

    for (i = 0; i < n; i++) {
        ((u8 *)blk)[i] = c;
    }
    return blk;
}

usize strlen(const char *str) {
    usize length = 0;
    while (*str != '\0') {
        str++;
        length++;
    }
    return length;
}
