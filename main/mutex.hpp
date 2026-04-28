#pragma once

namespace mutex {
    class Mutex {
        private:
            SemaphoreHandle_t sem;
    }
}
