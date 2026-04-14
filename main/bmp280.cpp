#pragma once

#include "i2c.cpp"

namespace bmp280 {
    class Bmp280 {
        private:
            i2c::I2cDevice dev;
            uint16_t t1;
            int16_t t2;
            int16_t t3;

            Bmp280(i2c::I2cDevice dev) : dev(dev) {}

        public:
            static auto from_i2c(i2c::I2cDevice dev) -> Bmp280 {
                auto self = Bmp280(dev);
                
                self.t1 = self.read_u16(0x88);
                self.t2 = self.read_i16(0x8a);
                self.t3 = self.read_i16(0x8c);

                return self;
            }

            auto read_u16(uint8_t address) -> uint16_t {
                uint8_t data[2];
                this->dev.read_n(address, 2, data);
                uint16_t value = (((uint16_t)data[1]<<8) | (uint16_t)data[0]);
                return value;
            }

            auto read_i16(uint8_t address) -> uint16_t {
                uint16_t uvalue = this->read_u16(address);
                int16_t value = *((int16_t*)&uvalue);
                return value;
            }

            auto read_temperature() -> float {
                uint8_t data[3];
                this->dev.read_n(0xfa, 3, data);

                int32_t adc_T =
                    ((int32_t)data[0] << 12) |
                    ((int32_t)data[1] << 4)  |
                    ((int32_t)data[2] >> 4);

                int32_t var1 = ((((adc_T >> 3) - ((int32_t)t1 << 1))) * t2) >> 11;
                int32_t var2 = (((((adc_T >> 4) - t1) *
                                  ((adc_T >> 4) - t1)) >> 12) * t3) >> 14;

                int32_t t_fine = var1 + var2;
                int32_t temp = (t_fine * 5 + 128) >> 8;

                return temp / 100.0f;
            }
    };
}
