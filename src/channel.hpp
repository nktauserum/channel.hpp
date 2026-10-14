#pragma once

// #include <thread>
#include <atomic>

#include "queue.hpp"
#include "futex.hpp"

template <typename T>
struct Channel {
private:
    Queue<T> *workers;
    uint32_t size;
    alignas(64) std::atomic<uint32_t> count = 0;
    alignas(64) std::atomic<uint32_t> next = 0;

public:
    Queue<T>* acquire() {
        return &workers[count.fetch_add(1, std::memory_order_relaxed)];
    }

    void push(T item) {
        while (1) {
            int idx = (next.fetch_add(1, std::memory_order_relaxed)) % size;
            Queue<T>& worker = workers[idx];

            auto [buf, status] = worker.acquire();
            if (status < 0) {
                continue;
            }
            *buf = item;
            worker.push();

            if (worker.waiting.exchange(0, std::memory_order_acq_rel))
                futex_wake(&worker.waiting);

            break;
        }
    }

    void close() {
        for (uint32_t i = 0; i < size; ++i) {
            workers[i].close(); 
        }
    }

    Channel(uint32_t size) : size{size}, workers{new Queue<T>[size]} {}
    ~Channel() {
        close();
        delete[] workers;
    }
};

