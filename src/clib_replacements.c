#include <type_alias.h>
void *memcpy(void *dest, const void *src, usize n) {
    u32 i;

    for (i = 0; i < n; i++) {
        ((unsigned char *)dest)[i] = ((unsigned char *)src)[i];
    }

    return dest;
}

void *memset(void *blk, i32 c, usize n) {
    u32 i;

    for (i = 0; i < n; i++) {
        ((unsigned char *)blk)[i] = c;
    }
    return blk;
}
