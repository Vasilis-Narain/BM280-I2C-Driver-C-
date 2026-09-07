A driver implementation for BME280 I2C humidity/pressure/temperature sensor written in C.

In order to get to the driver some things were implemented first:
- RTT implementation necessary for print debugging 
- Writer implementation to achieve functionality simlar to printf (int to string and int to hexstring implementations). Inspired by Zig std: aims to reduce I/O calls (in this case RTT buffer loads) as much as possible.

Driver implementation (work in progress...)
- memory map from supplier (Bosch)
- single and burst __blocking__ read functions for setup (reading compensation coefficients)
