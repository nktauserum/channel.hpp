#pragma once

#include <atomic>
#include <utility>

#include "futex.hpp"

#define unlikely(x) __builtin_expect(!!(x), 0)

#define QUEUE_SIZE 64

enum Status : int {
    OK,
    EMPTY,
    CLOSED,
};

template <typename T>
struct Queue {
private:
    alignas(64) std::atomic<uint32_t> head = 0;
    alignas(64) std::atomic<uint32_t> tail = 0;
    alignas(64) uint32_t tail_cache = 0;

    alignas(64) std::atomic<bool> closed = false;
   
    T data[QUEUE_SIZE]; // size must be a power of two
                        
public:
    std::atomic<uint32_t> waiting;

    T* acquire() {
        if (head.load(std::memory_order_relaxed) - tail_cache == QUEUE_SIZE) {
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

    std::pair<T*, int> read() {
        if (tail.load(std::memory_order_relaxed) == head.load(std::memory_order_acquire)) { 
            if (unlikely(closed.load(std::memory_order_acquire)))
                return std::make_pair(nullptr, -CLOSED);
            return std::make_pair(nullptr, -EMPTY);
        }
        return std::make_pair(&data[tail % QUEUE_SIZE], OK);
    }

    void pop() {
        tail.fetch_add(1, std::memory_order_release);
    }

    void wait() {
        waiting.store(1, std::memory_order_relaxed);
        futex_wait(&waiting, 1);
    }

    void close() {
        futex_wake(&waiting);
        closed.store(true, std::memory_order_release);
    }
};

