#include "bme280.h"
static i32 get_tp_params(bme280_calib_t *calib_params);
static i32 get_hum_params(bme280_calib_t *calib_params);

// blocking function
i32 get_calib_params(bme280_calib_t *calib_params) {
    i32 tp_err = get_tp_params(calib_params);
    if (tp_err != 0) {
        return tp_err;
    }
    i32 hum_err = get_hum_params(calib_params);
    if (hum_err != 0) {
        return hum_err;
    }
    return 0;
}

static i32 get_tp_params(bme280_calib_t *calib_params) {
    if (i2c_start_bulk_read_async(BME280_REG_TEMP_PRESS_CALIB_DATA_START, (u8 *)calib_params, BME280_LEN_TEMP_PRESS_CALIB_DATA) == 0) {
        while (i2c1_state == I2C_READING) {
            WFI;
        }
        BARRIER;

        if (i2c1_state != I2C_DONE) {
            return (i32)i2c_get_fault();
        }
        i2c1_state = I2C_IDLE;
    } else {
        return I2C_BUS_BUSY;
    }
    return 0;
}

static i32 get_hum_params(bme280_calib_t *calib_params) {
    u8 buf[BME280_LEN_HUMIDITY_CALIB_DATA];

    if (i2c_start_bulk_read_async(BME280_REG_HUMIDITY_CALIB_DATA, buf, BME280_LEN_HUMIDITY_CALIB_DATA) == 0) {
        while (i2c1_state == I2C_READING) {
            WFI;
        }
        BARRIER;

        if (i2c1_state != I2C_DONE) {
            return (i32)i2c_get_fault();
        }
        i2c1_state = I2C_IDLE;

        calib_params->dig_h2 = COMBINE_I16(buf[0], buf[1]);
        calib_params->dig_h3 = buf[2];

        calib_params->dig_h4 = ((i16)(i8)buf[3] * 16) | (buf[4] & 0x0F);
        calib_params->dig_h5 = ((i16)(i8)buf[5] * 16) | (buf[4] >> 4);

        calib_params->dig_h6 = (i8)buf[6];
    } else {
        return I2C_BUS_BUSY;
    }
    return 0;
}
