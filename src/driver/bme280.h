#pragma once
#include <type_alias.h>
#include "addresses.h"
#include "i2c_state_machine.h"

#define COMBINE_U16(lsb, msb) ((u16)((msb << 8) | lsb))
#define COMBINE_I16(lsb, msb) ((i16)((msb << 8) | lsb))

typedef struct {
    // bulk read 1: 26 bytes
    u16 dig_t1;
    i16 dig_t2;
    i16 dig_t3;

    u16 dig_p1;
    i16 dig_p2;
    i16 dig_p3;
    i16 dig_p4;
    i16 dig_p5;
    i16 dig_p6;
    i16 dig_p7;
    i16 dig_p8;
    i16 dig_p9;

    u8 _pad1;
    u8 dig_h1;

    // bulk read 2 : 7 bytes
    i16 dig_h2;
    u8 dig_h3;
    u8 _pad2;
    i16 dig_h4;
    i16 dig_h5;
    i8 dig_h6;
    u8 _pad3;
} bme280_calib_t;

i32 bme280_get_calib_params(bme280_calib_t *calib_params);

// returns temperature in DegC, resolution is 0.01 DegC.
// Output value of "5123" equals 51.23 DegC
i32 bme280_compensate_t(bme280_calib_t *calib, i32 adc_temp);

// returns pressure in Pa as unsigned 32 bit integer.
// Output value of "96386" equals 96386 Pa = 963.86 hPa
u32 bme280_compensate_p(bme280_calib_t *calib, i32 adc_pressure);

// returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
// Output value of "47445" represents 47445/1024 = 46.333 %RH
u32 bme280_compensate_h(bme280_calib_t *calib, i32 adc_humidity);
