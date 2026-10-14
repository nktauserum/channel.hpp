#include <vector>
#include <thread>
#include <print>

#include "../src/channel.hpp"

int main(void) {
    const uint32_t num_threads = 100;
    Channel<int> ch(num_threads);

    std::vector<std::thread> pool;
    for (uint32_t i = 0; i < num_threads; ++i)
        pool.emplace_back([&ch, i](){
            std::println("Worker #{} started", i); 

            auto queue = ch.acquire();

            for (;;) {
                auto [item, status] = queue->read();
                if (status == -EMPTY) {
                    queue->wait();
                    continue;
                } else if (status == -CLOSED) break;

                queue->pop();

                std::println("Worker #{} got {}", i, *item);
            }
        });

    for (int i = 0; i < 1000; ++i)
        ch.push(i+1);

    ch.close();

    for (uint32_t i = 0; i < num_threads; ++i)
        pool[i].join();

    return 0;
}

