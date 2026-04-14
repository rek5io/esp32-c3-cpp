#pragma once

#include "driver/i2c_master.h"

namespace i2c {
    class I2cBus {
        private:
            i2c_master_bus_handle_t bus_handle;
            I2cBus() {}

        public:
            static auto init_master() -> I2cBus {
                i2c_master_bus_handle_t bus_handle;
                i2c_master_bus_config_t i2c_mst_config = {};
                i2c_mst_config.clk_source = I2C_CLK_SRC_DEFAULT;
                i2c_mst_config.i2c_port = I2C_NUM_0;
                i2c_mst_config.scl_io_num = GPIO_NUM_3;
                i2c_mst_config.sda_io_num = GPIO_NUM_4;
                i2c_mst_config.glitch_ignore_cnt = 7;
                i2c_mst_config.flags = {};
                i2c_mst_config.flags.enable_internal_pullup = true;

                ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));
    
                I2cBus bus;
                bus.bus_handle = bus_handle;
                return bus;
            }

            auto get_handle() -> i2c_master_bus_handle_t {
                return this->bus_handle;
            } 
    };

    class I2cDevice {
        private:
            i2c_master_dev_handle_t dev_handle;

            I2cDevice() {}

        public:
            static auto init(I2cBus &bus, uint8_t address) -> I2cDevice {
                I2cDevice device;

                i2c_device_config_t dev_cfg = {};
                dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
                dev_cfg.device_address = address;
                dev_cfg.scl_speed_hz = 100000;
                dev_cfg.scl_wait_us = 0;
                dev_cfg.flags = {};
                
                ESP_ERROR_CHECK(i2c_master_bus_add_device(bus.get_handle(), &dev_cfg, &device.dev_handle));
                return device;
            }

            auto read(uint8_t reg) -> uint8_t {
                uint8_t value = 0;

                ESP_ERROR_CHECK(i2c_master_transmit_receive(
                    this->dev_handle,
                    &reg,
                    1,
                    &value,
                    1,
                    -1
                ));

                return value;
            }

            auto read_n(uint8_t reg, uint32_t len, uint8_t *data) -> int {
                ESP_ERROR_CHECK(i2c_master_transmit_receive(
                    this->dev_handle,
                    &reg,
                    1,
                    data,
                    len,
                    -1
                ));

                return 0;
            }
    };


}
