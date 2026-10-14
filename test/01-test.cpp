#include <vector>
#include <thread>
#include <print>

#include "../src/channel.hpp"

int main(void) {
    const uint32_t num_threads = 4;
    Channel<int> ch(num_threads);

    std::vector<std::thread> pool;
    for (uint32_t i = 0; i < num_threads; ++i)
        pool.emplace_back([&ch, i](){
            std::println("Worker #{} started", i); 

            auto queue = ch.acquire();

            for (;;) {
                int* item = queue->read();
                if (!item) {
                    queue->wait();
                    continue;
                }
                queue->pop();

                std::println("Worker #{} got {}", i, *item);
            }
        });

    for (int i = 0; i < 10; ++i)
        ch.push(i+1);

    for (uint32_t i = 0; i < num_threads; ++i)
        pool[i].join();

    return 0;
}
