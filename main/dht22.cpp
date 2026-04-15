#pragma once

#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_rom_sys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace dht22 {
    struct Measurement {
        float temperature;
        float humidity;
    };

    class Dht22 {
        private:
            Dht22() {}

            gpio_num_t pin;

            auto wait_level(uint8_t level, uint32_t timeout_us) -> esp_err_t {
                for (uint32_t t = 0; t < timeout_us; t += 2) {
                    esp_rom_delay_us(2);
                    if (gpio_get_level(pin) == level) {
                        return ESP_OK;
                    }
                }

                return ESP_ERR_TIMEOUT;
            }

            auto read_raw(uint8_t data[5]) -> esp_err_t {
                gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
                gpio_set_level(pin, 0);
                esp_rom_delay_us(20000);
                gpio_set_level(pin, 1);
                esp_rom_delay_us(30);

                gpio_set_direction(pin, GPIO_MODE_INPUT);

                if (wait_level(0, 80) != ESP_OK) {
                    return ESP_ERR_TIMEOUT;
                }

                if (wait_level(1, 90) != ESP_OK) {
                    return ESP_ERR_TIMEOUT;
                }

                if (wait_level(0, 90) != ESP_OK) {
                    return ESP_ERR_TIMEOUT;
                }

                for (int i = 0; i < 40; i++) {
                    if (wait_level(1, 70) != ESP_OK)
                        return ESP_ERR_TIMEOUT;

                    uint32_t t = 0;
                    while (gpio_get_level(pin) == 1) {
                        esp_rom_delay_us(1);
                        t++;
                        if (t > 100) {
                            break;
                        }
                    }

                    int byte_idx = i / 8;
                    int bit_idx  = 7 - (i % 8);

                    if (t > 40) {
                        data[byte_idx] |= (1 << bit_idx);
                    }
                }

                return ESP_OK;
            }
        public:
            static auto from_gpio(gpio_num_t pin) -> Dht22 {
                auto d = Dht22();
                gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
                gpio_set_level(pin, 1);
                d.pin = pin;
                return d;
            }

            auto measure() -> Measurement {
                uint8_t data[5] = {0};

                esp_err_t err = read_raw(data);
                if (err != ESP_OK) {
                    return {-1, -1};
                }

                uint8_t sum = data[0] + data[1] + data[2] + data[3];
                if (data[4] != sum) {
                    return {-1, -1};
                }

                int16_t raw_h = (data[0] << 8) | data[1];
                int16_t raw_t = (data[2] << 8) | data[3];

                float humidity = raw_h / 10.0f;
                float temperature = 0.0;

                if (raw_t & 0x8000) {
                    raw_t &= 0x7FFF;
                    temperature = -raw_t / 10.0f;
                } else {
                    temperature = raw_t / 10.0f;
                }

                return {temperature, humidity};
            }
    };
}
