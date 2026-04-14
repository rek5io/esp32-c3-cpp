#pragma once

#include "i2c.cpp"

namespace bmp280 {
    class Bmp280 {
        private:
            i2c::I2cDevice dev;

            Bmp280(i2c::I2cDevice dev) : dev(dev) {}

        public:
            static auto from_i2c(i2c::I2cDevice dev) -> Bmp280 {
                return Bmp280(dev);
            }

            auto read_temp() -> float {
                uint8_t data[3];
                this->dev.read_n(0xfa, 3, data);

                int32_t adc_T =
                    ((int32_t)data[0] << 12) |
                    ((int32_t)data[1] << 4)  |
                    ((int32_t)data[2] >> 4);

                int32_t dig_T1 = 27504;
                int32_t dig_T2 = 26435;
                int32_t dig_T3 = -1000;

                int32_t var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * dig_T2) >> 11;
                int32_t var2 = (((((adc_T >> 4) - dig_T1) *
                                  ((adc_T >> 4) - dig_T1)) >> 12) * dig_T3) >> 14;

                int32_t t_fine = var1 + var2;
                int32_t temp = (t_fine * 5 + 128) >> 8;

                return temp / 100.0f;
        }
    };
}
