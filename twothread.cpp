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

void producer(threadsafequeue<std::shared_ptr<int>>& queue, int id, int nitems)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1,100);

    for(int i = 0; i < nitems; i++)
    {
        auto value = std::make_shared<int>(dis(gen));
        queue.push(value);
        std::cout<< "producer " << id << " produced value " << *value << std::endl;
    }

    queue.push(std::make_shared<int>(-1));
}

void processor(threadsafequeue<std::shared_ptr<int>>& input_queue, threadsafequeue<std::shared_ptr<int>>& output_queue, int id)
{
    while(true)
    {
        auto value = input_queue.pop();
        if(*value == -1)
        {
            output_queue.push(value);
            break;
        }
        *value *= 2;
        std::cout << "Processor " << id << " processed value to " << *value << std::endl;
        output_queue.push(value);
    }
}

void consumer(threadsafequeue<std::shared_ptr<int>>& queue, int id)
{
    while (true)
    {
        auto value = queue.pop();
        if(*value == -1) break;
        std::cout << "Consumer " << id << " consumed value " << *value << std::endl;
    }
}

int main()
{
    const int nproducers = 3;
    const int nprocessors = 2;
    const int nconsumers = 3;
    const int item_per_producer = 5;

    //thread queues

    threadsafequeue<std::shared_ptr<int>> prod_queue;
    threadsafequeue<std::shared_ptr<int>> process_queue;

    //create and launch producer threads, processor threads and consumer threads

    std::vector<std::thread> prod_threads;
    for(int i = 0; i < nproducers; i++)
    {
        prod_threads.emplace_back(producer, std::ref(prod_queue), i, item_per_producer);
    }

    std::vector<std::thread> process_threads;
    for(int i = 0; i < nprocessors; i++)
    {
        process_threads.emplace_back(processor, std::ref(prod_queue), std::ref(process_queue), i);
    }

    std::vector<std::thread> consumer_threads;
    for(int i = 0; i < nconsumers; i++)
    {
        consumer_threads.emplace_back(consumer, std::ref(process_queue), i);
    }

    //join all producer, process and consumer threads 

    for(auto& thread : prod_threads)
    {
        thread.join();
    }

    for(auto& thread : process_threads)
    {
        thread.join();
    }

    for(int i = 0; i < nconsumers - nprocessors; i++)
    {
        process_queue.push(std::make_shared<int>(-1));
    }

    for(auto& thread : consumer_threads)
    {
        thread.join();
    }

    std::cout << "All producers, processosr and consumers have finished processing.";
    return 0;
}