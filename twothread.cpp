#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <random>

template<typename T>
class threadsafequeue 
{
    public:
        void push(T value)
        {
            std::lock_guard<std::mutex> lock(mutex);

            queue.push(std::move(value));
            condvar.notify_one();
        }

        T pop()
        {
            std::unique_lock<std::mutex> lock(mutex);
            condvar.wait(lock, [this] { return !queue.empty(); });
            T value = std::move(queue.front());
            queue.pop();
            return value;
        }

    private:
        std::queue<T> queue;
        std::mutex mutex;
        std::condition_variable condvar;
};