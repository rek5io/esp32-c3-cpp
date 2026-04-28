#pragma once

namespace mutex {
    template<typename T>
    class Mutex;

    template<typename T>
    class MutexGuard {
        friend class Mutex<T>;

        private:
            Mutex<T> &m;
            
            MutexGuard(Mutex<T>& mut): m(mut) {}
        public:
            auto get() const -> T {
                return this->m.value;
            }

            auto get_ref() -> T& {
                return this->m.value;
            }

            ~MutexGuard() {
                xSemaphoreGive(this->m.sem);
            }
    };

    template<typename T>
    class Mutex {
        friend class MutexGuard<T>;

        private:
            SemaphoreHandle_t sem;
            T value;

            Mutex(T v) : value(v) {
                this->sem = xSemaphoreCreateMutex();
            }

        public:
            static auto init(T value) -> Mutex<T> {
                return Mutex(value);
            }

            auto lock() -> MutexGuard<T> {
                xSemaphoreTake(this->sem, portMAX_DELAY);
                return MutexGuard(*this);
            }

            ~Mutex() {
                vSemaphoreDelete(this->sem);
            }
    };
}
