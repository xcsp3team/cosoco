#include <condition_variable>
#include <mutex>

#ifndef B_H
#define B_H

class Barrier {
   private:
    std::mutex              _mutex;
    std::condition_variable _cv;
    std::size_t             _count;

   public:
    explicit Barrier(std::size_t count) : _count(count) { }
    void wait() {
        std::unique_lock<std::mutex> lock {_mutex};
        if(--_count == 0) {
            _cv.notify_all();
        } else {
            _cv.wait(lock, [this] { return _count == 0; });
        }
    }
};
#endif
