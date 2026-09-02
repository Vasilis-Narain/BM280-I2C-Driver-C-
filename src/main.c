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

#include "driver/addresses.h"
#include "rtt.h"

#define SYST_CYCLES 12
#define PIN25 25
#define SDA_PIN 14
#define SCL_PIN 15

// I2C Control Register
// 0x00000400 [10]    STOP_DET_IF_MASTER_ACTIVE (0) Master issues the STOP_DET interrupt irrespective of...
// 0x00000200 [9]     RX_FIFO_FULL_HLD_CTRL (0) This bit controls whether DW_apb_i2c should hold the bus...
// 0x00000100 [8]     TX_EMPTY_CTRL (0) This bit controls the generation of the TX_EMPTY...
// 0x00000080 [7]     STOP_DET_IFADDRESSED (0) In slave mode: - 1'b1:  issues the STOP_DET interrupt...
// 0x00000040 [6]     IC_SLAVE_DISABLE (1) This bit controls whether I2C has its slave disabled,...
// 0x00000020 [5]     IC_RESTART_EN (1) Determines whether RESTART conditions may be sent when...
// 0x00000010 [4]     IC_10BITADDR_MASTER (0) Controls whether the DW_apb_i2c starts its transfers in...
// 0x00000008 [3]     IC_10BITADDR_SLAVE (0) When acting as a slave, this bit controls whether the...
// 0x00000006 [2:1]   SPEED        (0x2) These bits control at which speed the DW_apb_i2c...
// 0x00000001 [0]     MASTER_MODE  (1) This bit controls whether the DW_apb_i2c master is enabled
#define I2C_INIT_SET (I2C_IC_CON_MASTER_MODE_VALUE_ENABLED |                    \
                      I2C_IC_CON_SPEED_VALUE_STANDARD << I2C_IC_CON_SPEED_LSB | \
                      I2C_IC_CON_IC_RESTART_EN_BITS |                           \
                      I2C_IC_CON_IC_SLAVE_DISABLE_BITS)

// 0x00000100 [8]     ISO          (1) Pad isolation control
// 0x00000080 [7]     OD           (0) Output disable
// 0x00000040 [6]     IE           (0) Input enable
// 0x00000030 [5:4]   DRIVE        (0x1) Drive strength
// 0x00000008 [3]     PUE          (0) Pull up enable
// 0x00000004 [2]     PDE          (1) Pull down enable
// 0x00000002 [1]     SCHMITT      (1) Enable schmitt trigger
// 0x00000001 [0]     SLEWFAST     (0) Slew rate control
#define PADS_I2C_CLEAR (PADS_BANK0_GPIO0_ISO_BITS | \
                        PADS_BANK0_GPIO0_PDE_BITS | \
                        PADS_BANK0_GPIO0_PUE_BITS)

#define PADS_I2C_SET (PADS_BANK0_GPIO0_IE_BITS)

#define RESETS_CLEAR (RESETS_RESET_IO_BANK0_BITS | RESETS_RESET_PADS_BANK0_BITS | RESETS_RESET_I2C1_BITS)

#define SYSTICK_FREQ_HZ 1000
#define EXT_CLK_FREQ_HZ 1000000
#define SYSTICK_TOP (EXT_CLK_FREQ_HZ / SYSTICK_FREQ_HZ - 1)

// clk_sys must already be configured. Usually done in `crt0`
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

    // Always first clear reset bits for desired functionalities.
    // In this case: iobank, padsbank, i2c
    resets_clear(RESETS_CLEAR);

    // clk_sys must be configured before calling this function.
    configure_systick(SYST_CYCLES);

    //io_bank0_hw -> gpio function selection
    io_bank0_hw->io[PIN25].ctrl = GPIO_FUNC_SIO;
    io_bank0_hw->io[SDA_PIN].ctrl = GPIO_FUNC_I2C;
    io_bank0_hw->io[SCL_PIN].ctrl = GPIO_FUNC_I2C;

    // Pads bank -> configure pads
    hw_clear_bits(&pads_bank0_hw->io[PIN25], PADS_BANK0_GPIO0_ISO_BITS);

    hw_clear_bits(&pads_bank0_hw->io[SDA_PIN], PADS_I2C_CLEAR);
    hw_clear_bits(&pads_bank0_hw->io[SCL_PIN], PADS_I2C_CLEAR);
    hw_set_bits(&pads_bank0_hw->io[SDA_PIN], PADS_I2C_SET);
    hw_set_bits(&pads_bank0_hw->io[SCL_PIN], PADS_I2C_SET);

    // Config I2C1 as master
    i2c1_hw->enable = 0;
    while (i2c1_hw->enable_status & I2C_IC_ENABLE_STATUS_IC_EN_BITS) {}

    i2c1_hw->con = I2C_INIT_SET;
    i2c1_hw->tar = BME280_I2C_ADDR_PRIM;

    // The following numbers calculated from specification formulas for 12Mhz clk_sys
    // lcnt and hcnt are mandatory ic_clk settings
    i2c1_hw->ss_scl_hcnt = 48;
    i2c1_hw->ss_scl_lcnt = 72;
    i2c1_hw->fs_spklen = 1;
    i2c1_hw->sda_hold = 4;

    i2c1_hw->enable = 1;

    // I2C Rx/Tx Data Buffer and Command Register
    // 0x00000800 [11]    FIRST_DATA_BYTE (0) Indicates the first data byte received after the address...
    // 0x00000400 [10]    RESTART      (0) This bit controls whether a RESTART is issued before the...
    // 0x00000200 [9]     STOP         (0) This bit controls whether a STOP is issued after the...
    // 0x00000100 [8]     CMD          (0: write) This bit controls whether a read or a write is performed
    // 0x000000ff [7:0]   DAT          (0x00) This register contains the data to be transmitted or...

    i2c1_hw->data_cmd = 0xD0; // write command querying 0xD0 register
    i2c1_hw->data_cmd = I2C_IC_DATA_CMD_CMD_BITS |
                        I2C_IC_DATA_CMD_RESTART_BITS |
                        I2C_IC_DATA_CMD_STOP_BITS; // read command

    while (i2c1_hw->rxflr == 0) {}
    u8 chip_id = i2c1_hw->data_cmd & I2C_IC_DATA_CMD_DAT_BITS;
    rtt_print_hex_u8(chip_id, RTT_WRITE_CHANNEL);

    sio_hw->gpio_oe_set = 1 << PIN25; // output enable SIO reg. Atomic set.
    for (;;) {
        // systick accumulated by interrupt handler
        __asm__ volatile("WFI");
    }
}
