#pragma once

#include "i2c.cpp"
#include "result.hpp"
using namespace result;

namespace oled {
    const uint8_t font8x8_basic[2][8] = {
        [0] = {0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00}, // '0'
        [1] = {0xAA, 0x55, 0xAA, 0x55,0xAA, 0x55,0xAA, 0x55}, // 'szachownica'
    };
    class OledError {};
    class Oled {
        private:
            i2c::I2cDevice dev;
            uint8_t *framebuffer = nullptr;
            uint8_t x_size = 130;
            uint8_t y_size = 64;
            uint8_t coursor_x = 0;
            uint8_t coursor_y = 0;
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
            void set_pixel(int x, int y, bool on) {
                if (x < 0 || x >= x_size || y < 0 || y >= y_size) {
                    return;
                }

                int page = y >> 3;
                int index = page * x_size + x;
                uint8_t bit = 1 << (y & 7);

                if (on) {
                    framebuffer[index] |= bit;
                } else {
                    framebuffer[index] &= ~bit;
                }
            }
            void draw_symbol(uint8_t x, uint8_t y, const uint8_t *symbol, uint8_t width, uint8_t height, bool color) {
                for (uint8_t j = 0; j < height; j++) {
                    for (uint8_t i = 0; i < width; i++) {
                        bool pixel_on = (symbol[j] >> (7 - (i % 8))) & 1;
                        set_pixel(x + i, y + j, pixel_on^color);
                    }
                }
            }
        public:
            static auto from_i2c(i2c::I2cDevice dev) -> Result<Oled, OledError> {
                auto self = Oled(dev);
                self.framebuffer = new uint8_t[self.x_size * self.y_size / 8]();
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
                        framebuffer + page * x_size,
                        x_size
                    );
                }
            }

            void draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool color) {
                int dx = std::abs(x2 - x1);
                int dy = std::abs(y2 - y1);
                int sx = (x1 < x2) ? 1 : -1;
                int sy = (y1 < y2) ? 1 : -1;
                int err = dx - dy;

                while (true) {
                    set_pixel(x1, y1, color);

                    if (x1 == x2 && y1 == y2) break;
                    int err2 = err * 2;
                    if (err2 > -dy) {
                        err -= dy;
                        x1 += sx;
                    }
                    if (err2 < dx) {
                        err += dx;
                        y1 += sy;
                    }
                }
            }
            void draw_circle(int x0, int y0, int radius, bool color) {
                int x = radius;
                int y = 0;
                int err = 0;

                while (x >= y) {
                    set_pixel(x0 + x, y0 + y, color);
                    set_pixel(x0 + y, y0 + x, color);
                    set_pixel(x0 - y, y0 + x, color);
                    set_pixel(x0 - x, y0 + y, color);
                    set_pixel(x0 - x, y0 - y, color);
                    set_pixel(x0 - y, y0 - x, color);
                    set_pixel(x0 + y, y0 - x, color);
                    set_pixel(x0 + x, y0 - y, color);

                    if (err <= 0) {
                        y += 1;
                        err += 2*y + 1;
                    }
                    if (err > 0) {
                        x -= 1;
                        err -= 2*x + 1;
                    }
                }
            }
            void print(const char* str, bool color) {
                while (*str) {
                    if(*str>='0' && *str<='9') {
                        draw_symbol(coursor_x, coursor_y, font8x8_basic[*str - '0'], 8, 8, color);
                    } else if(*str>'A' && *str<='Z' ) {
                        draw_symbol(coursor_x, coursor_y, font8x8_basic[*str-'A'+10], 8, 8, color);
                    } else if(*str>'a' && *str<='z'){
                        draw_symbol(coursor_x, coursor_y, font8x8_basic[*str-'a'+10], 8, 8, color);
                    }
                    coursor_x += 8;
                    str++;
                }
            }
            void println(const char* str, bool color) {
                print(str, color);
                coursor_x = 0;
                coursor_y += 8;
                if (coursor_y + 8 > y_size) {
                    coursor_y = 0;
                }
            }
            void fill_chess(uint8_t size){
                for (uint8_t y = 0; y < y_size; y++) {
                    for (uint8_t x = 0; x < x_size; x++) {
                        set_pixel(x, y, !((x/size + y/size)%2));
                    }
                }
            }
            void clear() {
                set_coursor(0, 0);
                for (int i = 0; i < x_size * y_size / 8; i++) {
                    framebuffer[i] = 0x00;
                }
            }
            uint8_t get_x_size() const {
                return x_size;
            }
            uint8_t get_y_size() const {
                return y_size;
            }
            void set_coursor(uint8_t x, uint8_t y) {
                coursor_x = x;
                coursor_y = y;
            }
    };
}
