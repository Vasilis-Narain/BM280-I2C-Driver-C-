#include <type_alias.h>
#include <hardware/address_mapped.h>
#include <hardware/structs/resets.h>
#include <hardware/structs/io_bank0.h>
#include <hardware/structs/pads_bank0.h>
#include <hardware/structs/sio.h>

#define PIN25 25
#define CYCLES 1000000

void main() {
    const u32 reset_mask = RESETS_RESET_IO_BANK0_BITS | RESETS_RESET_PADS_BANK0_BITS;

    hw_clear_bits(&resets_hw->reset, reset_mask);
    while ((resets_hw->reset_done & reset_mask) != reset_mask) {}

    //io_bank0_hw
    io_bank0_hw->io[PIN25].ctrl = GPIO_FUNC_SIO;

    // Pads bank
    hw_clear_bits(&pads_bank0_hw->io[PIN25], PADS_BANK0_GPIO25_ISO_BITS);

    sio_hw->gpio_oe_set = 1 << PIN25;

    for (;;) {
        sio_hw->gpio_togl = 1 << PIN25;
        for (volatile u32 i = CYCLES; i > 0; i--) {}
    }
}
