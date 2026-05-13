#pragma once

#include "driver/uart.h"
#include "result.hpp"
#include "vector"
using namespace result;

namespace uart {
    class UartError {

    };

    class Uart {
        private:
            uart_port_t num;
            Uart(uart_port_t n) : num(n) {}

        public:
            static auto init(uart_port_t uart_num) -> Result<Uart, UartError> {
                uart_config_t uart_config = {};

                uart_config.baud_rate = 19200;
                uart_config.data_bits = UART_DATA_8_BITS;
                uart_config.parity = UART_PARITY_DISABLE;
                uart_config.stop_bits = UART_STOP_BITS_1;
                uart_config.flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS;
                uart_config.rx_flow_ctrl_thresh = 122;

                if (uart_driver_install(uart_num, 1024, 0, 0, NULL, 0) != ESP_OK) {
                    return Result<Uart, UartError>::Err(UartError());
                }

                if (uart_param_config(uart_num, &uart_config) != ESP_OK) {
                    return Result<Uart, UartError>::Err(UartError());
                }

                if (uart_set_pin(uart_num, 7, 8, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
                    return Result<Uart, UartError>::Err(UartError());
                }

                return Result<Uart, UartError>::Ok(Uart(uart_num));
            }

            auto read() -> std::vector<uint8_t> {
                size_t len = 0;
                ESP_ERROR_CHECK(uart_get_buffered_data_len(num, &len));

                std::vector<uint8_t> vec(len, 0);

                if (len != 0) {
                    uart_read_bytes(num, vec.data(), len, 100);
                }

                return vec;
            }

            auto write(std::span<uint8_t> slice) -> void {
                uart_write_bytes(num, slice.data(), slice.size());
                ESP_ERROR_CHECK(uart_wait_tx_done(num, 100));
            }
    };
}
