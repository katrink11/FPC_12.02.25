#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

#define TIMER_START(id) \
    auto start_##id = std::chrono::high_resolution_clock::now();

#define TIMER_END(id) \
    auto end_##id = std::chrono::high_resolution_clock::now();

#define TIMER_RESULT(id) \
    std::chrono::duration_cast<std::chrono::microseconds>(end_##id - start_##id).count()

constexpr int ITERATIONS = 50000;
int shared_value = 0;
std::mutex mtx;
std::atomic<int> atomic_value = 0;

void increment(bool use_lock)
{
    for (int i = 0; i < ITERATIONS; i++)
    {
        if (use_lock)
        {
            mtx.lock();
        }
        shared_value++;
        if (use_lock)
        {
            mtx.unlock();
        }
    }
}

void decrement(bool use_lock)
{
    for (int i = 0; i < ITERATIONS; i++)
    {
        if (use_lock)
        {
            mtx.lock();
        }
        shared_value--;
        if (use_lock)
        {
            mtx.unlock();
        }
    }
}

void atomic_increment()
{
    for (int i = 0; i < ITERATIONS; i++)
    {
        atomic_value.fetch_add(1, std::memory_order_relaxed);
    }
}

void atomic_decrement()
{
    for (int i = 0; i < ITERATIONS; i++)
    {
        atomic_value.fetch_sub(1, std::memory_order_relaxed);
    }
}

int main()
{
    std::thread t1(increment, false);
    std::thread t2(decrement, false);
    t1.join();
    t2.join();
    std::cout << "NO LOCK: " << shared_value << std::endl;

    shared_value = 0;
    TIMER_START(mutex);
    std::thread t3(increment, true);
    std::thread t4(decrement, true);
    t3.join();
    t4.join();
    TIMER_END(mutex);
    std::cout << "LOCK: " << shared_value << " " << TIMER_RESULT(mutex) << "mcs" << std::endl;

    TIMER_START(atomic);
    std::thread t5(atomic_increment);
    std::thread t6(atomic_decrement);
    t5.join();
    t6.join();
    TIMER_END(atomic);
    std::cout << "ATOMIC: " << atomic_value << " " << TIMER_RESULT(atomic) << "mcs" << std::endl;

    return 0;
}
