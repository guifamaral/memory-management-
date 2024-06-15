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
            std::lock_guard<std::mutex> lock(mutex_);

            queue_.push(std::move(value));
            cond_var_.notify_one();
        }
};