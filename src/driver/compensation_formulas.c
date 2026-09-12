/* 32 bit Compensation formulas from Bosch BME280 Datasheet */
#include <type_alias.h>
#include "bme280.h"

// t_fine carries fine temperature as a global value
i32 t_fine;

// returns temperature in DegC, resolution is 0.01 DegC.
// Output value of "5123" equals 51.23 DegC
i32 bme280_compensate_t(bme280_calib_t *calib, i32 adc_temp) {
    i32 var1, var2, temp;

    var1 = ((((adc_temp >> 3) - ((i32)calib->dig_t1 << 1))) * ((i32)calib->dig_t2)) >> 11;

    var2 = (((((adc_temp >> 4) - ((i32)calib->dig_t1)) * ((adc_temp >> 4) - ((i32)calib->dig_t1))) >> 12) * ((i32)calib->dig_t3)) >> 14;

    t_fine = var1 + var2;
    temp = (t_fine * 5 + 128) >> 8;
    return temp;
}

// returns pressure in Pa as unsigned 32 bit integer.
// Output value of "96386" equals 96386 Pa = 963.86 hPa
u32 bme280_compensate_p(bme280_calib_t *calib, i32 adc_p) {
    i32 var1, var2;
    u32 p;

    var1 = (((i32)t_fine) >> 1) - (i32)64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((i32)calib->dig_p6);
    var2 += ((var1 * ((i32)calib->dig_p5)) << 1);
    var2 = (var2 >> 2) + (((i32)calib->dig_p4) << 16);
    var1 = (((calib->dig_p3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((i32)calib->dig_p2) * var1) >> 1)) >> 18;
    var1 = ((((32768 + var1)) * ((i32)calib->dig_p1)) >> 15);

    if (var1 == 0) {
        // avoid exception caused by division by 0
        return 0;
    }

    p = (((u32)(((i32)1048576) - adc_p) - (var2 >> 12))) * 3125;

    if (p < 0x80000000) {
        p = (p << 1) / ((u32)var1);
    } else {
        p = (p / (u32)var1) * 2;
    }

    var1 = (((i32)calib->dig_p9) * ((i32)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
    var2 = (((i32)(p >> 2)) * ((i32)calib->dig_p8)) >> 13;
    p = (u32)((i32)p + ((var1 + var2 + calib->dig_p7) >> 4));

    return p;
}

// returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
// Output value of "47445" represents 47445/1024 = 46.333 %RH
u32 bme280_compensate_h(bme280_calib_t *calib, i32 adc_h) {
    i32 v_x1_u32r;

    v_x1_u32r = (t_fine - ((i32)76800));

    v_x1_u32r = (((((adc_h << 14) - (((i32)calib->dig_h4) << 20) - (((i32)calib->dig_h5) * v_x1_u32r)) + ((i32)16384)) >> 15) * (((((((v_x1_u32r * ((i32)calib->dig_h6)) >> 10) * (((v_x1_u32r * ((i32)calib->dig_h3)) >> 11) + ((i32)32768))) >> 10) + ((i32)2097162)) * ((i32)calib->dig_h2) + 1892) >> 14));

    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((i32)calib->dig_h1)) >> 4));

    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);

    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

    return (u32)(v_x1_u32r >> 12);
}
