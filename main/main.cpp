#include <print>
#include <thread>
#include <future>
#include "driver/gpio.h"
#include "i2c_oled.cpp"
#include "dht22.cpp"
#include "i2c.cpp"
#include "bmp280.cpp"
#include "result.hpp"
#include "mutex.hpp"
using namespace result;

#define time 1000
#define LED_GPIO GPIO_NUM_8

struct Measurements {
    bmp280::Measurement bmp;
    dht22::Measurement dht;
};

mutex::Mutex<Measurements> mea = mutex::Mutex<Measurements>::init(Measurements());

auto led_blink() -> void {
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while (1) {
        //std::println("led on");
        gpio_set_level(LED_GPIO, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        //std::println("led off");
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
            auto guard = mea.lock();
            guard.get_ref().bmp = m;
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }
}

auto dht() -> void {
    auto dh = dht22::Dht22::from_gpio(GPIO_NUM_9);

    while (1) {
        dh.measure().on_ok([](dht22::Measurement m) {
            auto guard = mea.lock();
            guard.get_ref().dht = m;
        });
        
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }
}

auto oled_task(i2c::I2cBus bus) -> void {
    auto dev_result = i2c::I2cDevice::init(bus, 0x3c);
    if (dev_result.is_err()) {
        std::println("i2c oled bus init error");
        return;
    }

    auto dev = dev_result.unwrap();
    auto oled = oled::Oled::from_i2c(dev).unwrap();
    oled.clear();
    oled.update();
    while (1) {
        //DEMO CODE
        oled.clear();
        oled.fill_chess(1);
        oled.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        oled.clear();
        oled.draw_line(20, 1, 10, 6, true);
        oled.draw_line(20, 1, 30, 6, true);
        oled.draw_line(10, 6, 20, 10, true);
        oled.draw_line(30, 6, 20, 10, true);
        oled.draw_line(10, 20, 10, 6, true);
        oled.draw_line(30, 20, 30, 6, true);
        oled.draw_line(20, 10, 20, 25, true);
        oled.draw_line(10, 20, 20, 25, true);
        oled.draw_line(30, 20, 20, 25, true);
        oled.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        oled.clear();
        oled.println("1110001010", true);
        oled.println("1010101010", false);
        oled.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        oled.clear();
        oled.draw_circle(64, 32, 30, true);
        oled.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        oled.clear();
        oled.fill_chess(4);
        oled.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }
}

extern "C" void app_main(void)  {
    auto bus_result = i2c::I2cBus::init_master(GPIO_NUM_0, GPIO_NUM_20);
    if (bus_result.is_err()) {
        std::println("i2c bus init error");
    }

    auto bus = bus_result.unwrap();
    auto fut_led = std::async(std::launch::async, led_blink);
    auto fut_bmp = std::async(std::launch::async, [&]() { bmp(bus); });
    auto fut_dht = std::async(std::launch::async, dht);
    auto fut_oled = std::async(std::launch::async, [&]() { oled_task(bus); });

    while (1) {
        {
            auto guard = mea.lock();
            
            std::println("Temperature: {}, Humidity: {}, Pressure: {}",
                (float)(((guard.get_ref().bmp.temperature * 10) + (guard.get_ref().dht.temperature * 10)) / 20),
                guard.get_ref().dht.humidity,
                guard.get_ref().bmp.pressure
            );
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    } 
}
