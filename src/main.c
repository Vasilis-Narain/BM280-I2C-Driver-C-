#include <type_alias.h>
#include <hardware/address_mapped.h>
#include <hardware/structs/resets.h>
#include <hardware/structs/io_bank0.h>
#include <hardware/structs/pads_bank0.h>
#include <hardware/structs/sio.h>
#include <hardware/structs/i2c.h>

#include "driver/addresses.h"

#define CYCLES 1000000
#define PIN25 25
#define SDA 14
#define SCL 15

// Going to avoid making abstractions as much as possible to have this as a reference of
// order of operations. However, using the provided `*.h` instead of handcoding
// as irl I'd mostly be taking definitions from manufacturer (svd).
void main() {
    const u32 reset_mask = RESETS_RESET_IO_BANK0_BITS | RESETS_RESET_PADS_BANK0_BITS;

    hw_clear_bits(&resets_hw->reset, reset_mask);
    while ((resets_hw->reset_done & reset_mask) != reset_mask) {}

    //io_bank0_hw
    io_bank0_hw->io[PIN25].ctrl = GPIO_FUNC_SIO;
    io_bank0_hw->io[SDA].ctrl = GPIO_FUNC_I2C;
    io_bank0_hw->io[SCL].ctrl = GPIO_FUNC_I2C;

    // Pads bank

    // 0x00000100 [8]     ISO          (1) Pad isolation control
    // 0x00000080 [7]     OD           (0) Output disable
    // 0x00000040 [6]     IE           (0) Input enable
    // 0x00000030 [5:4]   DRIVE        (0x1) Drive strength
    // 0x00000008 [3]     PUE          (0) Pull up enable
    // 0x00000004 [2]     PDE          (1) Pull down enable
    // 0x00000002 [1]     SCHMITT      (1) Enable schmitt trigger
    // 0x00000001 [0]     SLEWFAST     (0) Slew rate control
    hw_clear_bits(&pads_bank0_hw->io[PIN25], PADS_BANK0_GPIO25_ISO_BITS);

    const u32 i2c_mask = (1 << 6) | (1 << 1);
    hw_set_bits(&pads_bank0_hw->io[SDA], i2c_mask);
    hw_set_bits(&pads_bank0_hw->io[SCL], i2c_mask);

    // Config I2C1 as master
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
    i2c1_hw->enable = 0;
    i2c1_hw->con = 1 | (1 << 1) | (1 << 5) | (1 << 6);
    i2c1_hw->tar = BME280_I2C_ADDR;
    i2c1_hw->enable = 1;

    sio_hw->gpio_oe_set = 1 << PIN25;

    for (;;) {
        sio_hw->gpio_togl = 1 << PIN25;
        for (volatile u32 i = CYCLES; i > 0; i--) {}
    }
}
