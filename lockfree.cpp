#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

template<typename T>
class lockfreestack
{
    public:
    private:
        struct Node
        {
            T value;
            Node* next;
            Node(T val) : value(std::move(val), next(nullptr)) {}
        };
        std::atomic<Node*> head{nullptr};
        
}