#pragma once
#include <type_alias.h>
#include <hardware/structs/pads_bank0.h>
#include <hardware/structs/i2c.h>
#include "addresses.h"

#ifndef SDA_PIN
#define SDA_PIN 14
#endif

#ifndef SCL_PIN
#define SCL_PIN 15
#endif

#ifndef I2C1_ENABLE
#define I2C1_ENABLE
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

void i2c_init_master();
