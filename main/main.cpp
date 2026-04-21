#include <print>
#include <thread>
#include <future>
#include "driver/gpio.h"

#include "dht22.cpp"
#include "i2c.cpp"
#include "bmp280.cpp"
#include "result.hpp"
using namespace result;

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

auto bmp(i2c::I2cBus bus) -> void {
    auto dev_result = i2c::I2cDevice::init(bus, 0x76);
    if (dev_result.is_err()) {
        std::println("i2c bus init error");
        return;
    }

    auto dev = dev_result.unwrap();

    auto bmp_result = bmp280::Bmp280::from_i2c(dev);
    if (bmp_result.is_err()) {
        std::println("bmp280 init error");
        return;
    }
    
    auto bmp = bmp_result.unwrap();

    while (1) {
        auto m = bmp.measure(); 
        m.on_ok([](bmp280::Measurement m) {
            std::println("bmp280 temp: {}, pressure: {}", m.temperature, m.pressure);
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

auto dht() -> void {
    auto dh = dht22::Dht22::from_gpio(GPIO_NUM_9);

    while (1) {
        dh.measure().on_ok([](dht22::Measurement m) {
            std::println("dht22 temp: {}, humidity: {}", m.temperature, m.humidity); 
        });
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

extern "C" void app_main(void)  {
    auto bus_result = i2c::I2cBus::init_master(GPIO_NUM_20, GPIO_NUM_0);
    if (bus_result.is_err()) {
        std::println("i2c bus init error");
    }

    auto bus = bus_result.unwrap();

    auto fut_led = std::async(std::launch::async, led_blink);
    auto fut_bmp = std::async(std::launch::async, [&]() { bmp(bus); });
    auto fut_dht = std::async(std::launch::async, dht);

    while (1) {
        std::this_thread::sleep_for(std::chrono::seconds(500));
    } 
}
