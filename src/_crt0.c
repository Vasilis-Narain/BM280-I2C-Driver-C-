#include <type_alias.h>

extern u8 __data_start;
extern u8 __data_end;
extern u8 __data_lma;
extern u8 __bss_start;
extern u8 __bss_end;

// Functions provided
static void _config_ref_clock();
static void _config_sys_clock();

// Functions needed
extern void main();

void _crt0() {
    _config_sys_clock();
    _config_ref_clock();

    // Copy data segment
    u8 *dest = &__data_start;
    const u8 *src = &__data_lma;
    while (dest < &__data_end) {
        *dest++ = *src++;
    }

    // Clear bss segment
    u8 *bss = &__bss_start;
    while (bss < &__bss_end) {
        *bss++ = 0;
    }

    main();

    // in case main was made to return
    __asm__("CPSID I");
    while (1) {
        __asm__("WFI");
    }
}

static void _config_ref_clock() {
}
static void _config_sys_clock() {}
