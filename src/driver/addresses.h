#pragma once

#ifndef _u
#ifdef __ASSEMBLER__
#define _u(x) x
#else
#define _u(x) x##u
#endif
#endif

#define BME280_I2C_ADDR _u(0x76)
#define BME280_ALTERNATE_I2C_ADDR _u(0x77)
