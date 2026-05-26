#pragma once

#include <optional>

namespace cyclic_buffor {
    template<typename T, const size_t CAP>
    class CyclicBuffor {
        private:
            std::optional<T> buffor[CAP];
            size_t read_head = 0;
            size_t write_head = 0;
            size_t size_ = 0;

        public:
            CyclicBuffor() {}

            auto size() -> size_t {
                return size_;
            }

            auto capacity() -> size_t {
                return CAP;
            }

            auto read() -> std::optional<T> {
                if (size_ == 0) {
                    return std::nullopt;
                }

                std::optional<T> tmp = std::nullopt;
                buffor[read_head].swap(tmp);
                read_head = (read_head + 1) % CAP;
                size_--;

                return std::move(tmp);
            }

            auto write(T value) -> std::optional<T> {
                if (size_ < CAP) {
                    size_++;
                }

                std::optional<T> tmp = std::move(value);
                buffor[write_head].swap(tmp);
                write_head = (write_head + 1) % CAP;

                return std::move(tmp);
            }
    };

    auto test() -> void {
        CyclicBuffor<int, 32> buffor;

        std::println("{}", buffor.read().has_value());
        std::println("{}", buffor.write(67).has_value());
        std::println("{}", buffor.write(42).has_value());
        std::println("{}", buffor.read().value());
        std::println("{}", buffor.read().value());
        std::println("{}", buffor.read().has_value());
    }
}
