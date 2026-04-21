#pragma once

#include "i2c.cpp"
#include "result.hpp"
using namespace result;

namespace oled {
    class OledError {};

    class Oled {
        private:
            i2c::I2cDevice dev;
            uint8_t *framebuffer = nullptr;

            Oled(i2c::I2cDevice dev) : dev(dev) {}

            void oled_cmd(uint8_t cmd) {
                this->dev.write_u8(0x00, cmd);
            }

            void oled_data(const uint8_t* data, size_t len) {
                this->dev.write_n(0x40, data, len);
            }

            void oled_init() {
                // 1. Display OFF
                oled_cmd(0xAE);

                // 2. Set memory addressing mode
                oled_cmd(0x20);
                oled_cmd(0x00); // Horizontal addressing mode

                // 3. Set display start line
                oled_cmd(0x40);

                // 4. Set segment remap (mirror horizontally)
                oled_cmd(0xA1);

                // 5. Set COM output scan direction (flip vertically)
                oled_cmd(0xC8);

                // 6. Set multiplex ratio
                oled_cmd(0xA8);
                oled_cmd(0x3F); // 1/64 duty

                // 7. Set display offset
                oled_cmd(0xD3);
                oled_cmd(0x00);

                // 8. Set display clock divide ratio / oscillator
                oled_cmd(0xD5);
                oled_cmd(0x80);

                // 9. Set pre-charge period
                oled_cmd(0xD9);
                oled_cmd(0xF1);

                // 10. Set COM pins hardware configuration
                oled_cmd(0xDA);
                oled_cmd(0x12);

                // 11. Set VCOMH deselect level
                oled_cmd(0xDB);
                oled_cmd(0x40);

                // 12. Enable charge pump
                oled_cmd(0x8D);
                oled_cmd(0x14);

                // 13. Set contrast
                oled_cmd(0x81);
                oled_cmd(0x7F);

                // 14. Entire display ON (resume RAM content display)
                oled_cmd(0xA4);

                // 15. Normal display mode (not inverted)
                oled_cmd(0xA6);

                // 17. Display ON
                oled_cmd(0xAF);
            }
        public:
            static auto from_i2c(i2c::I2cDevice dev) -> Result<Oled, OledError> {
                auto self = Oled(dev);
                self.framebuffer = new uint8_t[1024];
                self.oled_init();
                return Result<Oled, OledError>::Ok(self);
            }

            void update() {
                for (uint8_t page = 0; page < 8; page++) {
                    oled_cmd(0xB0 + page);
                    oled_cmd(0x00);
                    oled_cmd(0x10);

                    dev.write_n(
                        0x40,
                        framebuffer + page * 128,
                        128
                    );
                }
            }

            void set_pixel(int x, int y, bool on) {
                if (x < 0 || x >= 128 || y < 0 || y >= 64) {
                    return;
                }

                int page = y >> 3;
                int index = page * 128 + x;
                uint8_t bit = 1 << (y & 7);

                if (on) {
                    framebuffer[index] |= bit;
                } else {
                    framebuffer[index] &= ~bit;
                }
            }

            void clear() {
                for (int i = 0; i < 128 * 64 / 8; i++) {
                    framebuffer[i] = 0x00;
                }
            }
    };
}
