#pragma once
#include <type_alias.h>
#include <hardware/structs/pads_bank0.h>
#include <hardware/regs/intctrl.h>
#include <hardware/structs/i2c.h>
#include <hardware/structs/m33.h>
#include "addresses.h"
#include "structs.h"

#define I2C_RESTART_READ_MASK (I2C_IC_DATA_CMD_CMD_BITS | I2C_IC_DATA_CMD_RESTART_BITS)
#define I2C_RESTART_READ_STOP_MASK (I2C_IC_DATA_CMD_CMD_BITS | I2C_IC_DATA_CMD_RESTART_BITS | I2C_IC_DATA_CMD_STOP_BITS)

#define BME280_LEN_TEMP_PRESS_CALIB 24

#ifndef SDA_PIN
#define SDA_PIN 14
#endif

#ifndef SCL_PIN
#define SCL_PIN 15
#endif

#ifndef I2C1_ENABLE
#define I2C1_ENABLE
#endif

#ifdef I2C1_ENABLE
#define i2c_hw i2c1_hw
#endif
#ifdef I2C0_ENABLE
#define i2c_hw i2c0_hw
#endif

#if !defined(I2C1_ENABLE)
#if !defined(I2C0_ENABLE)
#error Must define either `I2C0_ENABLE` or `I2C1_ENABLE`
#endif
#endif

#ifdef I2C1_ENABLE
#define i2c_hw i2c1_hw
#endif
#ifdef I2C0_ENABLE
#define i2c_hw i2c0_hw
#endif

#define I2C_INIT_SET (I2C_IC_CON_MASTER_MODE_VALUE_ENABLED |                    \
                      I2C_IC_CON_SPEED_VALUE_STANDARD << I2C_IC_CON_SPEED_LSB | \
                      I2C_IC_CON_IC_RESTART_EN_BITS |                           \
                      I2C_IC_CON_IC_SLAVE_DISABLE_BITS)

#define PADS_I2C_CLEAR (PADS_BANK0_GPIO0_ISO_BITS | \
                        PADS_BANK0_GPIO0_PDE_BITS | \
                        PADS_BANK0_GPIO0_PUE_BITS)

#define PADS_I2C_SET (PADS_BANK0_GPIO0_IE_BITS)

void i2c_init_master();

b32 get_tp_params(bme280_calib_tp *tp_params);
b32 get_hum_params(bme280_calib_hum *hum_params);

b32 i2c_blocking_bulk_read_command(u8 start_address, u8 *buffer, u32 length);
void i2c_blocking_read_command(u8 address, u8 *byte);
