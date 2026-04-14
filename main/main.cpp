#include <print>
#include <thread>
#include <future>
#include "driver/gpio.h"

#include "i2c.cpp"
#include "bmp280.cpp"

#define LED_GPIO GPIO_NUM_8

auto led_blink() -> void {
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while (1) {
        std::println("led on");
        gpio_set_level(LED_GPIO, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::println("led off");
        gpio_set_level(LED_GPIO, 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

auto bmp() -> void {
    auto bus = i2c::I2cBus::init_master();
    auto dev_0 = i2c::I2cDevice::init(bus, 0x76);
    auto bmp = bmp280::Bmp280::from_i2c(dev_0);

    while (1) {
        auto m = bmp.measure();
        std::println("temp: {}, pressure: {}", m.temperature, m.pressure);
        std::this_thread::sleep_for(std::chrono::milliseconds(125));
    }
}

auto test() -> void {
    while (1) {
        std::println("second ok");
        std::this_thread::sleep_for(std::chrono::milliseconds(125));
    }
}

extern "C" void app_main(void) {
    auto fut_led = std::async(std::launch::async, led_blink);
    auto fut_test = std::async(std::launch::async, test);
   // std::future<void> fut_bmp = std::async(std::launch::async, bmp);

    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
