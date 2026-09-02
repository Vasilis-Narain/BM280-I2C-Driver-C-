#include "state_machine.h"
#include "../rtt.h"

// Make sure to `#define SDA_PIN` and `#define SCL_PIN` to be used by pico.
// Defaults are 14 and 15 respectively.
void i2c_init_master() {
    // Setup pads
    hw_clear_bits(&pads_bank0_hw->io[SDA_PIN], PADS_I2C_CLEAR);
    hw_clear_bits(&pads_bank0_hw->io[SCL_PIN], PADS_I2C_CLEAR);
    hw_set_bits(&pads_bank0_hw->io[SDA_PIN], PADS_I2C_SET);
    hw_set_bits(&pads_bank0_hw->io[SCL_PIN], PADS_I2C_SET);

    // Most i2c config requires enable = 0.
    i2c1_hw->enable = 0;
    while (i2c1_hw->enable_status & I2C_IC_ENABLE_STATUS_IC_EN_BITS) {}

    // Config I2C1 as master
    i2c1_hw->con = I2C_INIT_SET;

    // Specify target address
    i2c1_hw->tar = BME280_I2C_ADDR;

    // The following numbers calculated from specification formulas for 12Mhz clk_sys
    // lcnt and hcnt ar ic_clk settings
    //
    // TODO(vasilis): calculate these at comptime
    //
    i2c1_hw->ss_scl_hcnt = 48;
    i2c1_hw->ss_scl_lcnt = 72;
    i2c1_hw->fs_spklen = 1;
    i2c1_hw->sda_hold = 4;

    i2c1_hw->enable = 1;
}

void EXAMPLE_READ_PROTOCOL_CMD() {
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

    // Long way to print hex (needs second rtt call to add newline)
    /*
    char str_buf[10];
    u32_to_hex(chip_id, str_buf);
    rtt_write(str_buf, 10, 0);
    rtt_print("\n", 0);
    */
    rtt_print_hex_u8(chip_id, RTT_WRITE_CHANNEL);
}
