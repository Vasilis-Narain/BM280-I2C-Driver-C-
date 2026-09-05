#include <type_alias.h>
void *memcpy(void *dest, const void *src, i32 n) {
    i32 i;

    for (i = 0; i < n; i++) {
        ((unsigned char *)dest)[i] = ((unsigned char *)src)[i];
    }

    return dest;
}

void *memset(void *blk, int c, i32 n) {
    i32 i;

    for (i = 0; i < n; i++) {
        ((unsigned char *)blk)[i] = c;
    }
    return blk;
}
