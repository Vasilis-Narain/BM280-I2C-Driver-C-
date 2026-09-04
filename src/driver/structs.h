#include <type_alias.h>
#include "addresses.h"

#define COMBINE_U16(lsb, msb) ((u16)((msb << 8) | lsb))
#define COMBINE_I16(lsb, msb) ((i16)((msb << 8) | lsb))

typedef struct {
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
} bme280_calib_tp;

typedef struct {
    u8 dig_h1;
    i16 dig_h2;
    u8 dig_h3;
    i16 dig_h4;
    i16 dig_h5;
    i8 dig_h6;

} bme280_calib_hum;
