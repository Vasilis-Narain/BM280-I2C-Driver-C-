#include <type_alias.h>
#include <hardware/address_mapped.h>
#include <hardware/structs/resets.h>
#include <hardware/structs/io_bank0.h>
#include <hardware/structs/pads_bank0.h>
#include <hardware/structs/sio.h>
#include <hardware/structs/i2c.h>
#include <hardware/structs/systick.h>
#include <hardware/structs/ticks.h>
#include <hardware/structs/m33.h>

#include "rtt.h"
#include "driver/i2c_state_machine.h"

#define SYST_CYCLES 12
#define PIN25 25
#define SDA_PIN 14
#define SCL_PIN 15

#define RESETS_CLEAR (RESETS_RESET_IO_BANK0_BITS | RESETS_RESET_PADS_BANK0_BITS | RESETS_RESET_I2C1_BITS)

#define SYSTICK_FREQ_HZ 1000
#define EXT_CLK_FREQ_HZ 1000000
#define SYSTICK_TOP (EXT_CLK_FREQ_HZ / SYSTICK_FREQ_HZ - 1)

/* clk_sys must already be configured. Usually done in `crt0`*/
void configure_systick(u8 cycles) {
    ticks_hw->ticks[TICK_PROC0].cycles = cycles;
    ticks_hw->ticks[TICK_PROC0].ctrl = TICKS_PROC0_CTRL_ENABLE_BITS;
    while (!(ticks_hw->ticks[TICK_PROC0].ctrl & TICKS_PROC0_CTRL_RUNNING_BITS)) {}

    // SysTick Control and Status Register
    // 0x00010000 [16]    COUNTFLAG    (0) Returns 1 if timer counted to 0 since last time this was read
    // 0x00000004 [2]     CLKSOURCE    (0) SysTick clock source
    // 0x00000002 [1]     TICKINT      (0) Enables SysTick exception request: +
    // 0x00000001 [0]     ENABLE       (0) Enable SysTick counter: +
    m33_hw->syst_rvr = SYSTICK_TOP;
    m33_hw->syst_cvr = 0;
    m33_hw->syst_csr = M33_SYST_CSR_TICKINT_BITS | M33_SYST_CSR_ENABLE_BITS;
}

volatile u32 ms = 0;
volatile u32 next = 500;
void SYSTICK_Handler() {
    ms++;
    if ((i32)(ms - next) >= 0) {
        next += 500;
        sio_hw->gpio_togl = 1 << PIN25;
    }
}

void resets_clear(u32 mask) {
    hw_clear_bits(&resets_hw->reset, mask);
    while ((resets_hw->reset_done & mask) != mask) {}
}

void main() {

    Writer writer = {
        ._flush = rtt_flush,
    };

    // Always first clear reset bits for desired functionalities.
    // In this case: iobank, padsbank, i2c
    resets_clear(RESETS_CLEAR);

    // clk_sys must be configured before calling this function.
    configure_systick(SYST_CYCLES);

    //io_bank0_hw -> gpio function selection
    io_bank0_hw->io[PIN25].ctrl = GPIO_FUNC_SIO;
    io_bank0_hw->io[SDA_PIN].ctrl = GPIO_FUNC_I2C;
    io_bank0_hw->io[SCL_PIN].ctrl = GPIO_FUNC_I2C;

    // Pads bank -> configure pads for led
    hw_clear_bits(&pads_bank0_hw->io[PIN25], PADS_BANK0_GPIO0_ISO_BITS);

    write_all(&writer, "\nRTT OK\n");
    i2c_init_master();

    bme280_calib_tp tp_params;
    get_tp_params(&tp_params);

    write_all(&writer, "\n...printing tp_params:\n");
    u16 *tmp = (u16 *)&tp_params;
    for (u8 i = 0; i < 12; i++) {
        if (i == 0 || i == 3) {
            print(&writer, "We're printing a uint16: {u:xs}\n", *tmp++);
        } else {
            print(&writer, "We're printing a int16: {d:s}\n", *tmp++);
        }
    }

    bme280_calib_hum hum_params;
    get_hum_params(&hum_params);

    write_all(&writer, "\n...printing hum_params:\n");
    tmp = (u16 *)&hum_params;

    // NOTE(vasilis): this doesnt actually select the 'correct' bytes...
    // its here to test the printing API itself.
    for (u32 i = 0; i < 6; i++) {
        if (i == 0 || i == 2) {
            print(&writer, "We're printing a uint8: {u:xb}\n", *tmp++);
        } else if (i == 5) {
            print(&writer, "We're printing a int8: {d:xb}\n", *tmp++);
        } else {
            print(&writer, "We're printing a int16: {d:xs}\n", *tmp++);
        }
    }

    flush(&writer);

    sio_hw->gpio_oe_set = 1 << PIN25; // output enable SIO reg. Special atomic registers for SIO

    for (;;) {
        __asm__ volatile("WFI");
    }
}
