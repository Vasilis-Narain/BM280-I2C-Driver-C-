#include "i2c_state_machine.h"
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
    i2c_hw->enable = 0;
    while (i2c_hw->enable_status & I2C_IC_ENABLE_STATUS_IC_EN_BITS) {}

    // Config I2C1 as master
    i2c_hw->con = I2C_INIT_SET;

    // Specify target address
    i2c_hw->tar = BME280_I2C_ADDR_PRIM;

    // The following numbers calculated from specification formulas for 12Mhz clk_sys
    // lcnt and hcnt ar ic_clk settings
    //
    // TODO(vasilis): calculate these at comptime rather than hardcode
    //
    i2c_hw->ss_scl_hcnt = 48;
    i2c_hw->ss_scl_lcnt = 72;
    i2c_hw->fs_spklen = 1;
    i2c_hw->sda_hold = 4;

    i2c_hw->enable = 1;
}

b32 get_tp_params(bme280_calib_tp *tp_params) {
    u32 length = BME280_LEN_TEMP_PRESS_CALIB;
    u8 start_addr = 0x88;
    if (i2c_blocking_bulk_read_command(start_addr, (u8 *)tp_params, length) != 0) {
        return 1;
    }
    return 0;
}

b32 get_hum_params(bme280_calib_hum *hum_params) {
    i2c_blocking_read_command(0xa1, &hum_params->dig_h1);

    u8 buff[BME280_LEN_HUMIDITY_CALIB_DATA];

    if (i2c_blocking_bulk_read_command(0xe1, buff, sizeof(buff)) != 0) {
        return 1;
    }

    hum_params->dig_h2 = COMBINE_I16(buff[0], buff[1]);
    hum_params->dig_h3 = buff[2];

    hum_params->dig_h4 = ((i16)(i8)buff[3] * 16) | (buff[4] & 0x0F);
    hum_params->dig_h5 = ((i16)(i8)buff[5] * 16) | (buff[4] >> 4);

    hum_params->dig_h6 = (i8)buff[6];
    return 0;
}

void i2c_blocking_read_command(u8 address, u8 *byte) {
    while (!(i2c_hw->status & I2C_IC_STATUS_TFNF_BITS)) {}
    i2c_hw->data_cmd = address;

    while (!(i2c_hw->status & I2C_IC_STATUS_TFNF_BITS)) {}
    i2c_hw->data_cmd = I2C_IC_DATA_CMD_CMD_BITS | I2C_IC_DATA_CMD_RESTART_BITS | I2C_IC_DATA_CMD_STOP_BITS;

    while (i2c_hw->rxflr == 0) {}
    *byte = i2c_hw->data_cmd & I2C_IC_DATA_CMD_DAT_BITS;
}

b32 i2c_blocking_bulk_read_command(u8 start_address, u8 *buffer, u32 length) {
    while (!(i2c_hw->status & I2C_IC_STATUS_TFNF_BITS)) {}
    i2c_hw->data_cmd = start_address;

    u32 issued = 0;
    u32 received = 0;
    while (received < length) {
        if ((issued < length) && (i2c_hw->status & I2C_IC_STATUS_TFNF_BITS)) {
            u32 cmd = I2C_IC_DATA_CMD_CMD_BITS;
            if (issued == 0) {
                cmd |= I2C_IC_DATA_CMD_RESTART_BITS;
            }
            if (issued == length - 1) {
                cmd |= I2C_IC_DATA_CMD_STOP_BITS;
            }
            i2c_hw->data_cmd = cmd;
            issued++;
        }

        if (i2c_hw->status & I2C_IC_STATUS_RFNE_BITS) {
            buffer[received++] = (u8)(i2c_hw->data_cmd & I2C_IC_DATA_CMD_DAT_BITS);
        }

        if (i2c_hw->raw_intr_stat & I2C_IC_RAW_INTR_STAT_TX_ABRT_BITS) {
            (void)i2c_hw->clr_tx_abrt;
            return 1;
        }
    }
    return 0;
}
