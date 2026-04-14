#include <print>
#include "driver/gpio.h"

#include "util.cpp"
#include "i2c.cpp"
#include "bmp280.cpp"

#define LED_GPIO GPIO_NUM_8

extern "C" void app_main(void) {
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    i2c::init_master();
    auto dev_0 = i2c::I2cDevice::init(0x76);
    auto bmp = bmp280::Bmp280::from_i2c(dev_0);

    while (1) {
        std::println("led on");
        gpio_set_level(LED_GPIO, 1);
        util::time::delay(util::time::Duration::from_millis(500));
        std::println("led off");
        gpio_set_level(LED_GPIO, 0);
        util::time::delay(util::time::Duration::from_millis(500));
        std::println("chip id: {}", bmp.read_temp());
    }
}
