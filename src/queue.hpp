#pragma once

#include <atomic>

#include "futex.h"

#define unlikely(x) __builtin_expect(!!(x), 0)

#define QUEUE_SIZE 64

template <typename T>
struct Queue {
private:
    alignas(64) std::atomic<uint32_t> head = 0;
    alignas(64) std::atomic<uint32_t> tail_cache = 0;
    alignas(64) std::atomic<uint32_t> tail = 0;
   
    T data[QUEUE_SIZE]; // size must be a power of two
                        
public:
    std::atomic<uint32_t> waiting;

    T* acquire() {
        if (head - tail_cache == QUEUE_SIZE) {
            tail_cache = tail.load(std::memory_order_consume);
            if (unlikely(head - tail_cache == QUEUE_SIZE)) {
                return nullptr;
            }
        }

        return &data[head % QUEUE_SIZE];
    }

    void push() {
        head.fetch_add(1, std::memory_order_release);
    }

    T* read() {
        if (tail == head.load(std::memory_order_acquire)) { 
            return nullptr;
        }
        return &data[tail % QUEUE_SIZE];
    }

    void pop() {
        tail.fetch_add(1, std::memory_order_release);
    }

    void wait() {
        waiting.store(1, std::memory_order_relaxed);
        futex_wait(&waiting, 1);
    }
};

