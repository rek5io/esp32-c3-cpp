#pragma once

#include "i2c.cpp"
#include "result.hpp"
using namespace result;

namespace bmp280 {
    struct Measurement {
        float temperature;
        float pressure;
    };

    class BmpError {};

    class Bmp280 {
        private:
            i2c::I2cDevice dev;
            uint16_t t1;
            int16_t t2;
            int16_t t3;
            uint16_t p1;
            int16_t p2;
            int16_t p3;
            int16_t p4;
            int16_t p5;
            int16_t p6;
            int16_t p7;
            int16_t p8;
            int16_t p9;
            int32_t t_fine;

            Bmp280(i2c::I2cDevice dev) : dev(dev) {}

            auto compensate_T(int32_t adc_T) -> uint32_t {
                int32_t var1 = ((((adc_T >> 3) - ((int32_t)t1 << 1)))
                       * (int32_t)t2) >> 11;

                int32_t var2 = (((((adc_T >> 4) - (int32_t)t1)
                         * ((adc_T >> 4) - (int32_t)t1)) >> 12)
                       * (int32_t)t3) >> 14;

                this->t_fine = var1 + var2;

                int32_t t = (this->t_fine * 5 + 128) >> 8;
                return t;
            }

            auto compensate_P(int32_t adc_P) -> uint32_t {
                int32_t var1 = (((int32_t)this->t_fine) >> 1) - (int32_t)64000;
                int32_t var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * (int32_t)p6;
                var2 = var2 + ((var1 * (int32_t)p5) << 1);
                var2 = (var2 >> 2) + ((int32_t)p4 << 16);

                var1 = ((((int32_t)p3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3)
                      + (((int32_t)p2 * var1) >> 1)) >> 18;

                var1 = (((32768 + var1) * (int32_t)p1) >> 15);

                if (var1 == 0) {
                    return 0;
                }

                uint32_t p = (uint32_t)((((int32_t)1048576 - adc_P - (var2 >> 12)) * 3125));

                if (p < 0x80000000U) {
                    p = (p << 1) / (uint32_t)var1;
                } else {
                    p = (p / (uint32_t)var1) * 2;
                }

                var1 = ((int32_t)p9 *
                       (int32_t)((((p >> 3) * (p >> 3)) >> 13))) >> 12;

                var2 = ((int32_t)(p >> 2) * (int32_t)p8) >> 13;

                p = (uint32_t)((int32_t)p + ((var1 + var2 + (int32_t)p7) >> 4));
                return p;
            }

            static inline uint16_t le_u16(uint8_t lsb, uint8_t msb) {
                return (((uint16_t)msb) << 8 | (uint16_t)lsb);
            }

            static inline int16_t le_i16(uint8_t lsb, uint8_t msb) {
               return (((int16_t)msb) << 8 | (int16_t)lsb);
            }
        public:
            static auto from_i2c(i2c::I2cDevice dev) -> Result<Bmp280, BmpError> {
                auto self = Bmp280(dev);

                uint8_t id = 0;
                uint8_t buf[24] = {0};

                if (dev.read_n(0xD0, 1, &id).is_err() || id != 0x58 || self.dev.read_n(0x88, 24, buf).is_err()) {
                    return Result<Bmp280, BmpError>::Err(BmpError());
                }
                
                self.dev.write_u8(0xf4, 0x57);  // temp x2, pressure x16, normal mode
                self.dev.write_u8(0xf5, 0x40);  // standby 125ms, filter off
               
                self.t1 = le_u16(buf[0],  buf[1]);
                self.t2 = le_i16(buf[2],  buf[3]);
                self.t3 = le_i16(buf[4],  buf[5]);

                self.p1 = le_u16(buf[6],  buf[7]);
                self.p2 = le_i16(buf[8],  buf[9]);
                self.p3 = le_i16(buf[10], buf[11]);
                self.p4 = le_i16(buf[12], buf[13]);
                self.p5 = le_i16(buf[14], buf[15]);
                self.p6 = le_i16(buf[16], buf[17]);
                self.p7 = le_i16(buf[18], buf[19]);
                self.p8 = le_i16(buf[20], buf[21]);
                self.p9 = le_i16(buf[22], buf[23]);
         
                return Result<Bmp280, BmpError>::Ok(self);
            }

            auto measure() -> Result<Measurement, BmpError> {
                uint8_t buf[6] = {0};
                if (this->dev.read_n(0xf7, 6, buf).is_err()) {
                    return Result<Measurement, BmpError>::Err(BmpError()); 
                }

                int32_t adc_P =
                    ((int32_t)buf[0] << 12) |
                    ((int32_t)buf[1] << 4)  |
                    ((int32_t)buf[2] >> 4);

                int32_t adc_T =
                    ((int32_t)buf[3] << 12) |
                    ((int32_t)buf[4] << 4)  |
                    ((int32_t)buf[5] >> 4);

                float t = (float)this->compensate_T(adc_T) / 100.0;
                float p = this->compensate_P(adc_P);

                return Result<Measurement, BmpError>::Ok({t, p});
            }
    };
}
