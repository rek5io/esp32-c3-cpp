#pragma once

#include "i2c.cpp"
#include "result.hpp"
#include <format>
#include <iterator>
#include <cstddef>

using namespace result;

namespace oled {
    const uint8_t font8x8_basic[36][8] = {
        [0] = {0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00}, // '0'
        [1] = {0x18, 0x78, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, // '1'
        [2] = {0x38, 0x44, 0x04, 0x08, 0x10, 0x20, 0x7E, 0x00}, // '2'
        [3] = {0x3C, 0x42, 0x02, 0x1C, 0x02, 0x42, 0x3C, 0x00}, // '3'
        [4] = {0x04, 0x0C, 0x14, 0x24, 0x7E, 0x04, 0x04, 0x00}, // '4'
        [5] = {0x7E, 0x40, 0x40, 0x7C, 0x02, 0x02, 0x7C, 0x00}, // '5'
        [6] = {0x3C, 0x42, 0x40, 0x7C, 0x42, 0x42, 0x3C, 0x00}, // '6'
        [7] = {0x7E, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00}, // '7'
        [8] = {0x3C, 0x42, 0x42, 0x3C, 0x42, 0x42, 0x3C, 0x00}, // '8'
        [9] = {0x3C, 0x42, 0x42, 0x3C, 0x02, 0x42, 0x3C, 0x00}, // '9'
        [10] = {0x7E, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00}, // 'A'
        [11] = {0x7C, 0x42, 0x42, 0x7C, 0x42, 0x42, 0x7C, 0x00}, // 'B'
        [12] = {0x3E, 0x40, 0x40, 0x40, 0x40, 0x40, 0x3E, 0x00}, // 'C'
        [13] = {0x7C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7C, 0x00}, // 'D'
        [14] = {0x7E, 0x40, 0x40, 0x7E, 0x40, 0x40, 0x7E, 0x00}, // 'E'
        [15] = {0x7E, 0x40, 0x40, 0x7E, 0x40, 0x40, 0x40, 0x00}, // 'F'
        [16] = {0x3E, 0x40, 0x40, 0x4C, 0x42, 0x42, 0x7E, 0x00}, // 'G'
        [17] = {0x42, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00}, // 'H'
        [18] = {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, // 'I'
        [19] = {0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x4C, 0x38, 0x00}, // 'J'
        [20] = {0x41, 0x42, 0x44, 0x78, 0x44, 0x42, 0x41, 0x00}, // 'K'
        [21] = {0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7E, 0x00}, // 'L'
        [22] = {0x42, 0x66, 0x5A, 0x42, 0x42, 0x42, 0x42, 0x00}, // 'M'
        [23] = {0x42, 0x62, 0x52, 0x4A, 0x46, 0x42, 0x42, 0x00}, // 'N'
        [24] = {0x7E, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7E, 0x00}, // 'O'
        [25] = {0x7E, 0x42, 0x42, 0x7E, 0x40, 0x40, 0x40, 0x00}, // 'P'
        [26] = {0x3C, 0x42, 0x42, 0x42, 0x4A, 0x44, 0x3A, 0x00}, // 'Q'
        [27] = {0x7E, 0x42, 0x42, 0x7E, 0x48, 0x44, 0x42, 0x00}, // 'R'
        [28] = {0x3E, 0x40, 0x40, 0x1E, 0x02, 0x02, 0x7C, 0x00}, // 'S'
        [29] = {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // 'T'
        [30] = {0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00}, // 'U'
        [31] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 'V'
        [32] = {0x42, 0x42, 0x42, 0x42, 0x5A, 0x66, 0x42, 0x00}, // 'W'
        [33] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 'X'
        [34] = {0x81, 0x42, 0x24, 0x18, 0x18, 0x18, 0x18, 0x00}, // 'Y'
        [35] = {0x7E, 0x02, 0x04, 0x08, 0x10, 0x20, 0x7E, 0x00}, // 'Z'
    };

    const uint8_t font8x8_basic_symbols[7][8] = {
        [0] = {0x0C, 0x12, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00}, // 'stopien'([)
        [1] = {0x00, 0x62, 0x64, 0x08, 0x10, 0x26, 0x46, 0x00}, // '%' (\)
        [2] = {0x18, 0x18, 0x24, 0x42, 0x81, 0x81, 0x42, 0x3C}, // 'kropelka (])
        [3] = {0x38, 0x44, 0x54, 0x54, 0x54, 0x54, 0x7C, 0x38}, // 'termometr?' (^)
        [4] = {0x3C, 0x52, 0x93, 0x95, 0x99, 0x81, 0x42, 0x3C}, // 'zegar?' (_)
        [5] ={0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x00}, // 'kropka ' (`)
        [6] ={0x00, 0x60, 0x60, 0x00, 0x00, 0x60, 0x00, 0x00}, // 'dwukropek- przyda sie' ()
    };

    class OledError {};
    class Oled {
       class FramebufferWriter {
            public:
                using iterator_category = std::output_iterator_tag;
                using value_type        = char;
                using difference_type   = std::ptrdiff_t;

                FramebufferWriter(Oled* oled, bool color)
                    : oled(oled), color(color) {}

                FramebufferWriter& operator=(char const& c) {
                    oled->putc(c, color);
                    return *this;
                }

                FramebufferWriter& operator*() { return *this; }
                FramebufferWriter& operator++() { return *this; }
                FramebufferWriter& operator++(int) { return *this; }

            private:
                Oled* oled;
                bool color;
        };

        private:
            i2c::I2cDevice dev;
            uint8_t *framebuffer = nullptr;
            uint8_t x_size = 130;
            uint8_t y_size = 64;
            uint8_t coursor_x = 4;
            uint8_t coursor_y = 0;

            int16_t encode = -2;

            Oled(i2c::I2cDevice dev) : 
                dev(dev)
            {
            }

            void oled_cmd(uint8_t cmd) {
                this->dev.write_u8(0x00, cmd);
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
        public:
            void draw_symbol(uint8_t x, uint8_t y, const uint8_t *symbol, uint8_t width, uint8_t height, bool color) {
                for (uint8_t j = 0; j < height; j++) {
                    for (uint8_t i = 0; i < width; i++) {
                        bool pixel_on = (symbol[j] >> (7 - (i % 8))) & 1;
                        set_pixel(x + i, y + j, pixel_on^color);
                    }
                }
            }
        
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

            void draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool color=0) {
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

            void draw_circle(int x0, int y0, int radius, bool color=0) {
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

            void putc(char c, bool color = false) {
                if (c >= '0' && c <= '9') {
                    uint8_t num = (c - '0');

                    if (encode == -1) {
                        encode = num * 10;
                    } else if (encode != -2) {
                        encode += num;
                        draw_symbol(coursor_x, coursor_y, font8x8_basic_symbols[encode], 8, 8, color);
                        encode = -2;
                    } else {
                        draw_symbol(coursor_x, coursor_y, font8x8_basic[num], 8, 8, color);
                    }
                } else if (c >= 'A' && c <= 'Z') {
                    draw_symbol(coursor_x, coursor_y, font8x8_basic[c - 'A' + 10], 8, 8, color);
                } else if (c >= 'a' && c <= 'z') {
                    draw_symbol(coursor_x, coursor_y, font8x8_basic[c - 'a' + 10], 8, 8, color);
                } else {
                    switch (c) {
                    case ',':
                    case '.':
                        draw_symbol(coursor_x, coursor_y, font8x8_basic_symbols[5], 8, 8, color);
                        break;
                    case '\n':
                        coursor_x = 4;
                        coursor_y += 8;
                        if (coursor_y + 8 > y_size) {
                            coursor_y = 0;
                        }
                        break;
                    case '%':
                        encode = -1;
                        break;
                    default:
                        return; // skip unknown char
                    }
                }

                coursor_x += 8;
            }

            template <typename... Args>
            void print(std::format_string<Args...> fmt, Args&&... args) {
                FramebufferWriter w(this, false);
                std::format_to(w, fmt, std::forward<Args>(args)...);
            }

            template <typename... Args>
            void println(std::format_string<Args...> fmt, Args&&... args) {
                print(fmt, std::forward<Args>(args)...);
                putc('\n');
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

            void free() {
                if (framebuffer) {
                    delete[] framebuffer;
                    framebuffer = nullptr;
                }
            }
    };
}
