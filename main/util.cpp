#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace util::time {
    class Duration {
        private:
            uint64_t ms = 0;
            Duration() {}

        public:
            static auto from_millis(uint32_t ms) -> Duration {
                Duration d;
                d.ms = ms;
                return d;
            }

            static auto from_secs(uint32_t secs) -> Duration {
                return Duration::from_millis(secs * 1000);
            }

            inline auto get_millis() -> uint64_t {
                return this->ms;
            }
    };

    auto delay(Duration duration) -> void {
        vTaskDelay(duration.get_millis() / portTICK_PERIOD_MS);
    }
}
