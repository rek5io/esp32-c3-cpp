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

void print_heap_info() {
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);

    std::println("mem info: free: {} KB allocated: {} KB", info.total_free_bytes / 1024, info.total_allocated_bytes / 1024);
}

struct Measurements {
    bmp280::Measurement bmp;
    dht22::Measurement dht;
};

mutex::Mutex<Measurements> mea = mutex::Mutex<Measurements>::init(Measurements());

auto led_task() -> void {
    gpio_reset_pin(GPIO_NUM_8);
    gpio_set_direction(GPIO_NUM_8, GPIO_MODE_OUTPUT);

    while (1) {
        //std::println("led on");
        gpio_set_level(GPIO_NUM_8, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        //std::println("led off");
        gpio_set_level(GPIO_NUM_8, 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

auto sensors_task(i2c::I2cBus bus) -> void { 
    auto dev_result = i2c::I2cDevice::init(bus, 0x76);
    if (dev_result.is_err()) {
        std::println("bmp280 i2c dev init error");
        return;
    }

    auto dev = dev_result.unwrap();

    auto bmp_result = bmp280::Bmp280::from_i2c(dev);
    if (bmp_result.is_err()) {
        std::println("bmp280 init error");
        return;
    }
    
    auto bmp = bmp_result.unwrap();
    auto dh = dht22::Dht22::from_gpio(GPIO_NUM_9);

    while (1) {
        bmp.measure().on_ok([](bmp280::Measurement m) {
            auto guard = mea.lock();
            guard.get_ref().bmp = m;
        });

        dh.measure().on_ok([](dht22::Measurement m) {
            auto guard = mea.lock();
            guard.get_ref().dht = m;
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(125));
    }
}

auto oled_task(i2c::I2cBus bus) -> void {
    auto dev_result = i2c::I2cDevice::init(bus, 0x3c);
    if (dev_result.is_err()) {
        std::println("oled i2c dev init error");
        return;
    }

    auto dev = dev_result.unwrap();
    auto oled = oled::Oled::from_i2c(dev).unwrap();
    float hum;
    uint32_t press;
    float temper;
    oled.clear();
    oled.update();
    while (1) {
        oled.clear();
        {
            auto guard = mea.lock();
            
            temper = ((guard.get_ref().bmp.temperature * 10) + (guard.get_ref().dht.temperature * 10)) / 20;
            hum = guard.get_ref().dht.humidity;
            press = guard.get_ref().bmp.pressure;
        }

        oled.println("^ {}[C", temper);
        oled.println("] {}\\", hum);
        oled.println("_ {}Pa", press);
        oled.update();

        std::this_thread::sleep_for(std::chrono::milliseconds(5*time));
        oled.clear();
        oled.draw_symbol(5, 5,  oled::font8x8_basic[38], 8, 8, true);
        oled.draw_symbol(13, 5, oled::font8x8_basic[39], 8, 8, true);
        oled.draw_symbol(21, 5, oled::font8x8_basic[40], 8, 8, true);
        oled.draw_symbol(29, 5, oled::font8x8_basic[41], 8, 8, true);
        oled.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        /*
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
        oled.println("01234567890", true);
        oled.println("01234567890", false);
        oled.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(10*time));
        oled.clear();
        oled.draw_circle(64, 32, 30, true);
        oled.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        oled.clear();
        oled.fill_chess(4);
        oled.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(time));*/
    }

    oled.free();
}

extern "C" void app_main(void)  {
    auto bus_result = i2c::I2cBus::init_master(GPIO_NUM_0, GPIO_NUM_20);
    if (bus_result.is_err()) {
        std::println("i2c bus init error");
    }

    auto bus = bus_result.unwrap();

    auto fut_led = std::async(std::launch::async, led_task);
    auto fut_sensors = std::async(std::launch::async, [&]() { sensors_task(bus); });
    auto fut_oled = std::async(std::launch::async, [&]() { oled_task(bus); });

    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        print_heap_info();
    } 
}
