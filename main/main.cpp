#include <print>
#include <thread>
#include <future>
#include "driver/gpio.h"

#include "dht22.cpp"
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
    auto bus_result = i2c::I2cBus::init_master(GPIO_NUM_3, GPIO_NUM_4);
    if (!std::holds_alternative<i2c::I2cBus>(bus_result)) {
        std::println("i2c bus init error");
        return;
    }

    auto bus = std::get<i2c::I2cBus>(bus_result);

    auto dev_result = i2c::I2cDevice::init(bus, 0x76);
    if (!std::holds_alternative<i2c::I2cDevice>(dev_result)) {
        std::println("i2c bus init error");
        return;
    }

    auto dev = std::get<i2c::I2cDevice>(dev_result);

    auto bmp_result = bmp280::Bmp280::from_i2c(dev);
    if (!std::holds_alternative<bmp280::Bmp280>(bmp_result)) {
        std::println("bmp280 init error");
        return;
    }

    auto bmp = std::get<bmp280::Bmp280>(bmp_result);

    while (1) {
        auto m = bmp.measure();
        std::println("temp: {}, pressure: {}", m.temperature, m.pressure);
        std::this_thread::sleep_for(std::chrono::milliseconds(125));
    }
}

auto dht() -> void {
    auto dh = dht22::Dht22::from_gpio(GPIO_NUM_5);

    while (1) {
        auto m = dh.measure();
        std::println("dht22 temp: {}, humidity: {}", m.temperature, m.humidity);
        std::this_thread::sleep_for(std::chrono::milliseconds(125));
    }
}

auto test() -> void {
    while (1) {
        std::println("second ok");
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

extern "C" void app_main(void) {
    auto fut_led = std::async(std::launch::async, led_blink);
    //auto fut_test = std::async(std::launch::async, test);
    auto fut_bmp = std::async(std::launch::async, bmp);
    auto fut_dht = std::async(std::launch::async, dht);

    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
