#pragma once

#include <cstdint>
#include "i2c.cpp"
#include "result.hpp"

using namespace result;

namespace ens160 {
    constexpr uint8_t DEFAULT_ADDRESS = 0x53; // zmienić ma 0x52 jak nie będzie działać

    constexpr uint8_t REG_PART_ID     = 0x00;
    constexpr uint8_t REG_OPMODE      = 0x10;
    constexpr uint8_t REG_DEVICE_STATUS = 0x20;
    constexpr uint8_t REG_DATA_AQI    = 0x21;
    constexpr uint8_t REG_DATA_TVOC   = 0x22;
    constexpr uint8_t REG_DATA_ECO2   = 0x24;

    constexpr uint16_t EXPECTED_PART_ID = 0x0160;

    constexpr uint8_t OPMODE_IDLE     = 0x01;
    constexpr uint8_t OPMODE_STANDARD = 0x02;

    class Ens160Error {};

    struct Measurement {
        uint8_t aqi;
        uint16_t tvoc;
        uint16_t eco2;
        bool valid;
    };

    class Ens160 {
    private:
        i2c::I2cDevice dev;

        Ens160(i2c::I2cDevice dev) : dev(dev) {}

    public:
        static auto from_i2c(i2c::I2cDevice dev) -> Result<Ens160, Ens160Error> {
            auto self = Ens160(dev);

            auto part_id_result = self.dev.read_u16(REG_PART_ID);

            if (part_id_result.is_err()) {
                return Result<Ens160, Ens160Error>::Err(Ens160Error());
            }

            if (part_id_result.unwrap() != EXPECTED_PART_ID) {
                return Result<Ens160, Ens160Error>::Err(Ens160Error());
            }

            if (self.dev.write_u8(REG_OPMODE, OPMODE_IDLE).is_err()) {
                return Result<Ens160, Ens160Error>::Err(Ens160Error());
            }

            if (self.dev.write_u8(REG_OPMODE, OPMODE_STANDARD).is_err()) {
                return Result<Ens160, Ens160Error>::Err(Ens160Error());
            }

            return Result<Ens160, Ens160Error>::Ok(self);
        }

        auto measure() -> Result<Measurement, Ens160Error> {
            uint8_t status = 0;
            uint8_t aqi = 0;
            uint8_t tvoc_data[2] = {0, 0};
            uint8_t eco2_data[2] = {0, 0};

            if (this->dev.read_n(REG_DEVICE_STATUS, 1, &status).is_err()) {
                return Result<Measurement, Ens160Error>::Err(Ens160Error());
            }

            if (this->dev.read_n(REG_DATA_AQI, 1, &aqi).is_err()) {
                return Result<Measurement, Ens160Error>::Err(Ens160Error());
            }

            if (this->dev.read_n(REG_DATA_TVOC, 2, tvoc_data).is_err()) {
                return Result<Measurement, Ens160Error>::Err(Ens160Error());
            }

            if (this->dev.read_n(REG_DATA_ECO2, 2, eco2_data).is_err()) {
                return Result<Measurement, Ens160Error>::Err(Ens160Error());
            }

            uint16_t tvoc = (uint16_t)tvoc_data[0] | ((uint16_t)tvoc_data[1] << 8);
            uint16_t eco2 = (uint16_t)eco2_data[0] | ((uint16_t)eco2_data[1] << 8);

            Measurement measurement = {
                .aqi = aqi,
                .tvoc = tvoc,
                .eco2 = eco2,
                .valid = (status & 0x02) != 0
            };

            return Result<Measurement, Ens160Error>::Ok(measurement);
        }
    };
}
