/**
* Copyright (c) 2020 Bosch Sensortec GmbH. All rights reserved.
*
* BSD-3-Clause
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its
*    contributors may be used to endorse or promote products derived from
*    this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* @file       bme280_defs.h
* @date       2020-12-17
* @version    v3.5.1
*
*/

#pragma once

#ifndef _u
#ifdef __ASSEMBLER__
#define _u(x) x
#else
#define _u(x) x##u
#endif
#endif

#if !defined(UINT8_C) && !defined(INT8_C)
#define INT8_C(x) S8_C(x)
#define UINT8_C(x) U8_C(x)
#endif

#if !defined(UINT16_C) && !defined(INT16_C)
#define INT16_C(x) S16_C(x)
#define UINT16_C(x) U16_C(x)
#endif

#if !defined(INT32_C) && !defined(UINT32_C)
#define INT32_C(x) S32_C(x)
#define UINT32_C(x) U32_C(x)
#endif

#if !defined(INT64_C) && !defined(UINT64_C)
#define INT64_C(x) S64_C(x)
#define UINT64_C(x) U64_C(x)
#endif

/**@}*/
/**\name C standard macros */
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

/******************************************************************************/
/*! @name        General Macro Definitions                */
/******************************************************************************/
#ifndef TRUE
#define TRUE UINT8_C(1)
#endif
#ifndef FALSE
#define FALSE UINT8_C(0)
#endif

/*!
 * BME280_INTF_RET_TYPE is the read/write interface return type which can be overwritten by the build system.
 */
#ifndef BME280_INTF_RET_TYPE
#define BME280_INTF_RET_TYPE int8_t
#endif

/*!
 * The last error code from read/write interface is stored in the device structure as intf_rslt.
 */
#ifndef BME280_INTF_RET_SUCCESS
#define BME280_INTF_RET_SUCCESS INT8_C(0)
#endif

/*! @name API success code */
#define BME280_OK INT8_C(0)

/*! @name API error codes */
#define BME280_E_NULL_PTR INT8_C(-1)
#define BME280_E_COMM_FAIL INT8_C(-2)
#define BME280_E_INVALID_LEN INT8_C(-3)
#define BME280_E_DEV_NOT_FOUND INT8_C(-4)
#define BME280_E_SLEEP_MODE_FAIL INT8_C(-5)
#define BME280_E_NVM_COPY_FAILED INT8_C(-6)

/*! @name API warning codes */
#define BME280_W_INVALID_OSR_MACRO INT8_C(1)

/*! @name BME280 chip identifier */
#define BME280_CHIP_ID UINT8_C(0x60)

/*! @name I2C addresses */
#define BME280_I2C_ADDR_PRIM UINT8_C(0x76)
#define BME280_I2C_ADDR_SEC UINT8_C(0x77)

/*! @name Register Address */
#define BME280_REG_CHIP_ID UINT8_C(0xD0)
#define BME280_REG_RESET UINT8_C(0xE0)
#define BME280_REG_TEMP_PRESS_CALIB_DATA_START UINT8_C(0x88)
#define BME280_REG_HUMIDITY_CALIB_DATA UINT8_C(0xE1)
#define BME280_REG_CTRL_HUM UINT8_C(0xF2)
#define BME280_REG_STATUS UINT8_C(0xF3)
#define BME280_REG_PWR_CTRL UINT8_C(0xF4)
#define BME280_REG_CTRL_MEAS UINT8_C(0xF4)
#define BME280_REG_CONFIG UINT8_C(0xF5)
#define BME280_REG_DATA UINT8_C(0xF7)

/*! @name Macros related to size */
#define BME280_LEN_TEMP_PRESS_CALIB_DATA UINT8_C(26)
#define BME280_LEN_HUMIDITY_CALIB_DATA UINT8_C(7)
#define BME280_LEN_P_T_H_DATA UINT8_C(8)

/*! @name Sensor power modes */
#define BME280_POWERMODE_SLEEP UINT8_C(0x00)
#define BME280_POWERMODE_FORCED UINT8_C(0x01)
#define BME280_POWERMODE_NORMAL UINT8_C(0x03)

#define BME280_SENSOR_MODE_MSK UINT8_C(0x03)
#define BME280_SENSOR_MODE_POS UINT8_C(0x00)

/*! @name Soft reset command */
#define BME280_SOFT_RESET_COMMAND UINT8_C(0xB6)

#define BME280_STATUS_IM_UPDATE UINT8_C(0x01)
#define BME280_STATUS_MEAS_DONE UINT8_C(0x08)

/*! @name Sensor component selection macros
 * These values are internal for API implementation. Don't relate this to
 * data sheet.
 */
#define BME280_PRESS UINT8_C(1)
#define BME280_TEMP UINT8_C(1 << 1)
#define BME280_HUM UINT8_C(1 << 2)
#define BME280_ALL UINT8_C(0x07)

/*! @name Settings selection macros */
#define BME280_SEL_OSR_PRESS UINT8_C(1)
#define BME280_SEL_OSR_TEMP UINT8_C(1 << 1)
#define BME280_SEL_OSR_HUM UINT8_C(1 << 2)
#define BME280_SEL_FILTER UINT8_C(1 << 3)
#define BME280_SEL_STANDBY UINT8_C(1 << 4)
#define BME280_SEL_ALL_SETTINGS UINT8_C(0x1F)

/*! @name Oversampling macros */
#define BME280_NO_OVERSAMPLING UINT8_C(0x00)
#define BME280_OVERSAMPLING_1X UINT8_C(0x01)
#define BME280_OVERSAMPLING_2X UINT8_C(0x02)
#define BME280_OVERSAMPLING_4X UINT8_C(0x03)
#define BME280_OVERSAMPLING_8X UINT8_C(0x04)
#define BME280_OVERSAMPLING_16X UINT8_C(0x05)
#define BME280_OVERSAMPLING_MAX UINT8_C(16)

#define BME280_CTRL_HUM_MSK UINT8_C(0x07)
#define BME280_CTRL_HUM_POS UINT8_C(0x00)
#define BME280_CTRL_PRESS_MSK UINT8_C(0x1C)
#define BME280_CTRL_PRESS_POS UINT8_C(0x02)
#define BME280_CTRL_TEMP_MSK UINT8_C(0xE0)
#define BME280_CTRL_TEMP_POS UINT8_C(0x05)

/*! @name Measurement delay calculation macros  */
#define BME280_MEAS_OFFSET UINT16_C(1250)
#define BME280_MEAS_DUR UINT16_C(2300)
#define BME280_PRES_HUM_MEAS_OFFSET UINT16_C(575)
#define BME280_MEAS_SCALING_FACTOR UINT16_C(1000)
#define BME280_STARTUP_DELAY UINT16_C(2000)

/*! @name Length macros */
#define BME280_MAX_LEN UINT8_C(10)

/*! @name Standby duration selection macros */
#define BME280_STANDBY_TIME_0_5_MS (0x00)
#define BME280_STANDBY_TIME_62_5_MS (0x01)
#define BME280_STANDBY_TIME_125_MS (0x02)
#define BME280_STANDBY_TIME_250_MS (0x03)
#define BME280_STANDBY_TIME_500_MS (0x04)
#define BME280_STANDBY_TIME_1000_MS (0x05)
#define BME280_STANDBY_TIME_10_MS (0x06)
#define BME280_STANDBY_TIME_20_MS (0x07)

#define BME280_STANDBY_MSK UINT8_C(0xE0)
#define BME280_STANDBY_POS UINT8_C(0x05)

/*! @name Bit shift macros */
#define BME280_12_BIT_SHIFT UINT8_C(12)
#define BME280_8_BIT_SHIFT UINT8_C(8)
#define BME280_4_BIT_SHIFT UINT8_C(4)

/*! @name Filter coefficient selection macros */
#define BME280_FILTER_COEFF_OFF (0x00)
#define BME280_FILTER_COEFF_2 (0x01)
#define BME280_FILTER_COEFF_4 (0x02)
#define BME280_FILTER_COEFF_8 (0x03)
#define BME280_FILTER_COEFF_16 (0x04)

#define BME280_FILTER_MSK UINT8_C(0x1C)
#define BME280_FILTER_POS UINT8_C(0x02)

/*! @name Macro to combine two 8 bit data's to form a 16 bit data */
#define BME280_CONCAT_BYTES(msb, lsb) (((uint16_t)msb << 8) | (uint16_t)lsb)

/*! @name Macro to SET and GET BITS of a register */
#define BME280_SET_BITS(reg_data, bitname, data) \
    ((reg_data & ~(bitname##_MSK)) |             \
        ((data << bitname##_POS) & bitname##_MSK))

#define BME280_SET_BITS_POS_0(reg_data, bitname, data) \
    ((reg_data & ~(bitname##_MSK)) |                   \
        (data & bitname##_MSK))

#define BME280_GET_BITS(reg_data, bitname) ((reg_data & (bitname##_MSK)) >> \
                                            (bitname##_POS))
#define BME280_GET_BITS_POS_0(reg_data, bitname) (reg_data & (bitname##_MSK))
