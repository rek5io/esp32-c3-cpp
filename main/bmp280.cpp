#pragma once

#include "i2c.cpp"

namespace bmp280 {
    class Bmp280 {
        private:
            i2c::I2cDevice dev;
            uint16_t t1;
            int16_t t2;
            int16_t t3;
            int32_t t_fine;

            Bmp280(i2c::I2cDevice dev) : dev(dev) {}

        public:
            static auto from_i2c(i2c::I2cDevice dev) -> Bmp280 {
                auto self = Bmp280(dev);
                
                self.dev.write_u8(0xf4, 0x57);  // temp x2, pressure x16, normal mode
                self.dev.write_u8(0xf5, 0x40);  // standby 125ms, filter off
                self.t1 = self.dev.read_u16(0x88);
                self.t2 = (int16_t)self.dev.read_u16(0x8A);
                self.t3 = (int16_t)self.dev.read_u16(0x8C);

                return self;
            }

            auto read_temperature() -> float {
                uint8_t data[3];
                this->dev.read_n(0xfa, 3, data);

                int32_t adc_T =
                    ((int32_t)data[0] << 12) |
                    ((int32_t)data[1] << 4)  |
                    ((int32_t)data[2] >> 4);

                int32_t var1 =
                    ((((adc_T >> 3) - ((int32_t)t1 << 1))) * (int32_t)t2) >> 11;

                int32_t var2 =
                    (((((adc_T >> 4) - (int32_t)t1) *
                    ((adc_T >> 4) - (int32_t)t1)) >> 12) * (int32_t)t3) >> 14;

                t_fine = var1 + var2;
                int32_t temp = (t_fine * 5 + 128) >> 8;

                return temp / 100.0f;
            }
    };
}
