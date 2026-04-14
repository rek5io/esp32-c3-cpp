#pragma once

#include "i2c.cpp"
#include <stdexcept>

namespace bmp280 {
    struct Measurement {
        float temperature;
        float pressure;
    };

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

        public:
            static auto from_i2c(i2c::I2cDevice dev) -> Bmp280 {
                auto self = Bmp280(dev);

                if (dev.read_u8(0xD0) != 0x58) {
                    throw std::runtime_error("not an bmp280");
                }
                
                self.dev.write_u8(0xf4, 0x57);  // temp x2, pressure x16, normal mode
                self.dev.write_u8(0xf5, 0x40);  // standby 125ms, filter off
                self.t1 = self.dev.read_u16(0x88);
                self.t2 = (int16_t)self.dev.read_u16(0x8a);
                self.t3 = (int16_t)self.dev.read_u16(0x8c);
                self.p1 = self.dev.read_u16(0x8e);
                self.p2 = (int16_t)self.dev.read_u16(0x90);
                self.p3 = (int16_t)self.dev.read_u16(0x92);
                self.p4 = (int16_t)self.dev.read_u16(0x94);
                self.p5 = (int16_t)self.dev.read_u16(0x96);
                self.p6 = (int16_t)self.dev.read_u16(0x98);
                self.p7 = (int16_t)self.dev.read_u16(0x9a);
                self.p8 = (int16_t)self.dev.read_u16(0x9c);
                self.p9 = (int16_t)self.dev.read_u16(0x9e);

                return self;
            }

            auto measure() -> Measurement {
                Measurement m;

                uint8_t buf[6];
                dev.read_n(0xf7, 6, buf);

                int32_t adc_P =
                    ((int32_t)buf[0] << 12) |
                    ((int32_t)buf[1] << 4)  |
                    ((int32_t)buf[2] >> 4);

                int32_t adc_T =
                    ((int32_t)buf[3] << 12) |
                    ((int32_t)buf[4] << 4)  |
                    ((int32_t)buf[5] >> 4);

                int32_t vart1 =
                    ((((adc_T >> 3) - ((int32_t)t1 << 1))) * (int32_t)t2) >> 11;

                int32_t vart2 =
                    (((((adc_T >> 4) - (int32_t)t1) *
                    ((adc_T >> 4) - (int32_t)t1)) >> 12) * (int32_t)t3) >> 14;

                t_fine = vart1 + vart2;
                m.temperature = ((t_fine * 5 + 128) >> 8) / 100.0f;

                int64_t var1 = ((int64_t)t_fine) - 128000;
                int64_t var2 = var1 * var1 * (int64_t)p6;
                var2 = var2 + ((var1 * (int64_t)p5) << 17);
                var2 = var2 + (((int64_t)p4) << 35);
                var1 = ((var1 * var1 * (int64_t)p3) >> 8) +
                       ((var1 * (int64_t)p2) << 12);
                var1 = (((((int64_t)1) << 47) + var1)) * (int64_t)p1 >> 33;

                if (var1 == 0) {
                    m.pressure = 0;
                } else {
                    int64_t p = 1048576 - adc_P;
                    p = (((p << 31) - var2) * 3125) / var1;
                    var1 = ((int64_t)p9 * (p >> 13) * (p >> 13)) >> 25;
                    var2 = ((int64_t)p8 * p) >> 19;
                    p = ((p + var1 + var2) >> 8) + (((int64_t)p7) << 4);

                    m.pressure = (float)p / 256.0f;
                }
                
                return m;
            }
    };
}
